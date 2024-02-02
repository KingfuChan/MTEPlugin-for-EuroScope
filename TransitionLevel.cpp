// TransitionLevel.cpp

#include "pch.h"
#include "TransitionLevel.h"

TransitionLevel::TransitionLevel(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
	m_DefaultLevel = 0;
	m_MaxLevel = 0;
	q_Thread = std::jthread(&TransitionLevel::UpdateQueueThread, this, q_StopSrc.get_token());
}

TransitionLevel::~TransitionLevel(void)
{
	q_StopSrc.request_stop();
	q_Thread.join();
}

void TransitionLevel::LoadCSV(const std::string& filename)
{
	std::unique_lock dlock(data_mutex);
	m_AirportMap.clear();
	m_DefaultLevel = 0;
	m_MaxLevel = 0;
	int asteroid_level = 0;
	int asteroid_range = 0;

	// external file
	std::ifstream inFile;
	inFile.open(filename, std::ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		throw std::string("unable to open file");
	}
	std::string line;
	getline(inFile, line);
	if (line != "Ident,TransLevel,Elevation,IsQFE,Range,Boundary") { // confirm header
		inFile.close();
		throw std::string("invalid column names");
	}
	try {
		while (getline(inFile, line)) {
			std::istringstream ssin(line);
			std::string apid, lvlstr, elevstr, qfestr, rngstr, bndstr;
			getline(ssin, apid, ',');
			getline(ssin, lvlstr, ',');
			getline(ssin, elevstr, ',');
			getline(ssin, qfestr, ',');
			getline(ssin, rngstr, ',');
			getline(ssin, bndstr);
			int lvlft(0), elevft(0), isqfe(0), rngnm(0);
			if (sscanf_s(lvlstr.c_str(), "F%d", &lvlft)) {
				lvlft = lvlft * 100;
			}
			else if (sscanf_s(lvlstr.c_str(), "S%d", &lvlft)) {
				lvlft = MetricAlt::LvlMtoFeet(lvlft * 100);
			}
			sscanf_s(elevstr.c_str(), "%d", &elevft);
			sscanf_s(qfestr.c_str(), "%d", &isqfe);
			sscanf_s(rngstr.c_str(), "%d", &rngnm);
			std::istringstream bdin(bndstr);
			std::string ordstr;
			pos_vec bndvec;
			while (bdin >> ordstr) {
				double lon(0), lat(0);
				if (sscanf_s(ordstr.c_str(), "%lf/%lf", &lon, &lat) == 2) {
					EuroScopePlugIn::CPosition pos;
					pos.m_Longitude = lon;
					pos.m_Latitude = lat;
					bndvec.push_back(pos);
				}
			}

			// no checks for lvlft, need to check >0 when using it
			m_MaxLevel = max(lvlft, m_MaxLevel);
			if (apid == "*") { // for sector default
				asteroid_level = lvlft;
				asteroid_range = rngnm;
			}
			else if (!apid.size()) { // for run-time no match
				m_DefaultLevel = lvlft;
			}
			else {
				m_AirportMap[apid] = AirportData{
				lvlft,elevft,(bool)isqfe,rngnm,bndvec,false,EuroScopePlugIn::CPosition(),
				};
			}
		}
		inFile.close();
	}
	catch (...) {
		inFile.close();
		throw;
	}
	if (m_AirportMap.empty()) {
		throw std::string("no valid airport definitions");
	}

	// sector file
	for (auto se = m_PluginPtr->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT);
		se.IsValid();
		se = m_PluginPtr->SectorFileElementSelectNext(se, EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT)) {
		auto apd = m_AirportMap.find(se.GetName());
		EuroScopePlugIn::CPosition pos;
		se.GetPosition(&pos, 0);
		if (apd != m_AirportMap.end()) {
			apd->second.in_sector = true;
			apd->second.position = pos;
		}
		else { // sector default
			m_AirportMap.insert({ se.GetName(),
				AirportData{asteroid_level,0,false,asteroid_range,pos_vec{},true,pos,}
				});
		}
	}
}

void TransitionLevel::UpdateRadarPosition(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	if (!RadarTarget.IsValid()) return;
	// extract useful info and push to queue
	std::string sysID = RadarTarget.GetSystemID();
	auto pos = RadarTarget.GetPosition().GetPosition();
	// concerned airport
	std::string acls = "";
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (FlightPlan.IsValid()) {
		// check the closer one of origin/destination
		std::string adep = FlightPlan.GetFlightPlanData().GetOrigin();
		std::string aarr = FlightPlan.GetFlightPlanData().GetDestination();
		double ddep = FlightPlan.GetDistanceFromOrigin();
		double darr = FlightPlan.GetDistanceToDestination();
		std::string acls = ddep < darr ? adep : aarr;
	}
	{
		std::lock_guard qlock(queue_mutex);
		m_UpdateQueue.push({ sysID,pos,acls });
	}
	q_CondVar.notify_all();
}

void TransitionLevel::UpdateQueueThread(std::stop_token stoken)
{
	std::mutex t_mutex;
	typedef struct _DisAp {
		std::string airport;
		double distance;
	} DistanceToAirport;
	while (!stoken.stop_requested()) {
		std::unique_lock t_lock(t_mutex);
		q_CondVar.wait(t_lock, stoken, [&] {
			return !m_UpdateQueue.empty() || stoken.stop_requested();
			});
		while (true) {
			// updates radar cache
			QueueData qd;
			{
				std::lock_guard qlock(queue_mutex);
				if (m_UpdateQueue.empty()) {
					break;
				}
				else {
					qd = m_UpdateQueue.front();
					m_UpdateQueue.pop();
				}
			}
			std::shared_lock dlock(data_mutex);
			if (m_AirportMap.empty()) continue;
			bool updated = false;
			std::string sysID = qd.system_id;
			auto pos = qd.position;
			// look up in cache and check validity
			{
				std::shared_lock clock(cache_mutex);
				auto cached = m_RadarCache.find(sysID);
				if (cached != m_RadarCache.end()) {
					auto apitr = m_AirportMap.find(cached->second);
					if (apitr != m_AirportMap.end()) {
						if (IsinQNHBoundary(pos, apitr->second)) {
							updated = true;// no need to refresh
						}
					}
				}
			}
			if (updated) continue;
			// use concerned airport
			std::string acls = qd.concerned_airport;
			auto icls = m_AirportMap.find(acls);
			if (icls != m_AirportMap.end() && IsinQNHBoundary(pos, icls->second)) {
				std::unique_lock clock(cache_mutex);
				m_RadarCache[sysID] = acls;
				updated = true;
			}
			if (updated) continue;
			// sort closest airport and check boundary
			std::vector<DistanceToAirport> distance_airports;
			std::transform(m_AirportMap.begin(), m_AirportMap.end(), std::back_inserter(distance_airports),
				[pos](std::pair<const std::string, AirportData>& m) {
					double d = pos.DistanceTo(m.second.position);
					return DistanceToAirport({ m.first,d });
				});
			std::sort(distance_airports.begin(), distance_airports.end(),
				[](const auto& d1, const auto& d2) {
					return d1.distance < d2.distance;
				});
			auto daitr = std::find_if(distance_airports.begin(), distance_airports.end(),
				[&](const auto& d) {
					return IsinQNHBoundary(pos, m_AirportMap.find(d.airport)->second);
				});
			if (daitr != distance_airports.end()) {
				std::unique_lock clock(cache_mutex);
				m_RadarCache[sysID] = daitr->airport;
			}
			else { // no match
				std::unique_lock clock(cache_mutex);
				m_RadarCache.erase(sysID);
			}
		}
	}
}

int TransitionLevel::GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference)
{
	// returns radar display altitude and assign reference AltitudeReference::ALT_REF_xxx
	if (!RadarTarget.IsValid()) return 0;
	int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
	int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
	std::shared_lock dlock(data_mutex);
	if (m_AirportMap.empty()) {
		if (stdAlt >= m_PluginPtr->GetTransitionAltitude()) {
			reference = AltitudeReference::ALT_REF_QNE;
			return stdAlt;
		}
		else {
			reference = AltitudeReference::ALT_REF_QNH;
			return qnhAlt;
		}
	}
	else if (stdAlt >= m_MaxLevel) {
		reference = AltitudeReference::ALT_REF_QNE;
		return stdAlt;
	}
	else { // use cache only
		std::shared_lock clock(cache_mutex);
		auto cached = m_RadarCache.find(RadarTarget.GetSystemID());
		if (cached != m_RadarCache.end()) {
			auto apitr = m_AirportMap.find(cached->second);
			if (apitr != m_AirportMap.end()) {
				int trslvl = apitr->second.trans_level > 0 ? apitr->second.trans_level : m_PluginPtr->GetTransitionAltitude();
				if (stdAlt >= trslvl) {
					reference = AltitudeReference::ALT_REF_QNE;
					return stdAlt;
				}
				else if (apitr->second.is_QFE) {
					reference = AltitudeReference::ALT_REF_QFE;
					return qnhAlt - apitr->second.elevation;
				}
				else {
					reference = AltitudeReference::ALT_REF_QNH;
					return qnhAlt;
				}
			}
		}
	}
	// no boundary match
	if (stdAlt >= m_DefaultLevel) {
		reference = AltitudeReference::ALT_REF_QNE;
		return stdAlt;
	}
	else {
		reference = AltitudeReference::ALT_REF_QNH;
		return qnhAlt;
	}
}

std::string TransitionLevel::GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan, int& trans_level, int& elevation)
{
	// returns airport ident and sets trans_level, elevation. Elevation will be 0 if not QFE. Doesn't consider altitude.
	auto RadarTarget = FlightPlan.GetCorrelatedRadarTarget();
	std::shared_lock dlock(data_mutex);
	if (!RadarTarget.IsValid() || m_AirportMap.empty()) {
		trans_level = !m_AirportMap.empty() && m_DefaultLevel > 0 ? m_DefaultLevel : m_PluginPtr->GetTransitionAltitude();
		elevation = 0;
		return std::string();
	}
	else {
		std::shared_lock clock(cache_mutex);
		auto cached = m_RadarCache.find(RadarTarget.GetSystemID());
		if (cached != m_RadarCache.end()) {
			auto apitr = m_AirportMap.find(cached->second);
			if (apitr != m_AirportMap.end()) {
				trans_level = apitr->second.trans_level > 0 ? apitr->second.trans_level : m_PluginPtr->GetTransitionAltitude();
				elevation = apitr->second.is_QFE ? apitr->second.elevation : 0;
				return apitr->first;
			}
		}
	}
	// not found, default values
	trans_level = m_DefaultLevel;
	elevation = 0;
	return std::string();
}

bool TransitionLevel::SetAirportParam(const std::string& airport, const int trans_level, const int isQFE, const int range)
{
	// default to -1 for ignoring. isQFE=0 means QNH, trans_level in feet, range in nm
	std::unique_lock dlock(data_mutex);
	auto apitr = m_AirportMap.find(airport);
	if (apitr != m_AirportMap.end()) {
		if (trans_level > 0)
			apitr->second.trans_level = trans_level;
		if (isQFE >= 0)
			apitr->second.is_QFE = isQFE;
		if (range >= 0)
			apitr->second.range = range;
		return true;
	}
	return false;
}

bool TransitionLevel::IsinQNHBoundary(const EuroScopePlugIn::CPosition pos, const AirportData airport_data)
{
	// only considers lateral boundary, need to check iter's validity before calling
	// by range
	if (airport_data.in_sector && airport_data.range) {
		double distance = pos.DistanceTo(airport_data.position);
		if (distance < (double)airport_data.range) {
			return true;
		}
	}

	// by boundary check
	pos_vec boundary = airport_data.boundary;
	if (boundary.empty())
		return false;
	std::vector<double> directions;
	for (size_t i = 0; i < boundary.size(); directions.push_back(pos.DirectionTo(boundary[i++])));
	std::vector<double> angles;
	for (size_t i = 0; i < directions.size(); i++) {
		size_t j = i + 1 < directions.size() ? i + 1 : 0;
		double a = directions[j] - directions[i];
		a += a < -180.0 ? 360.0 : (a > 180.0 ? -360.0 : 0.0);
		angles.push_back(a);
	}
	double sum = 0.0;
	for (size_t i = 0; i < angles.size(); sum += angles[i++]);
	return abs(sum - 360.0) < 1.0;
}
