// TransitionLevel.cpp

#include "pch.h"
#include "TransitionLevel.h"

TransitionLevel::TransitionLevel(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
	m_DefaultLevel = m_PluginPtr->GetTransitionAltitude();
	m_MaxLevel = 0;
}

TransitionLevel::~TransitionLevel(void)
{
}

void TransitionLevel::LoadCSV(string filename)
{
	m_AirportMap.clear();
	m_DefaultLevel = m_PluginPtr->GetTransitionAltitude();
	m_MaxLevel = 0;
	int asteroid_level = 0;
	int asteroid_range = 0;

	// external file
	ifstream inFile;
	inFile.open(filename, ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		throw string("unable to open file");
	}
	string line;
	getline(inFile, line);
	if (line != "Ident,TransLevel,Elevation,IsQFE,Range,Boundary") { // confirm header
		inFile.close();
		throw string("invalid column names");
	}
	try {
		while (getline(inFile, line)) {
			istringstream ssin(line);
			string apid, lvlstr, elevstr, qfestr, rngstr, bndstr;
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
			istringstream bdin(bndstr);
			string ordstr;
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

int TransitionLevel::GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference)
{
	// returns radar display altitude and assign reference AltitudeReference::ALT_REF_xxx
	if (!RadarTarget.IsValid()) return 0;
	int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
	int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
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
	if (stdAlt >= m_MaxLevel) {
		reference = AltitudeReference::ALT_REF_QNE;
		return stdAlt;
	}
	apmap_iter apitr = GetTargetAirport(RadarTarget);
	if (apitr == m_AirportMap.end()) { // no boundary match
		if (stdAlt >= m_DefaultLevel) {
			reference = AltitudeReference::ALT_REF_QNE;
			return stdAlt;
		}
		else {
			reference = AltitudeReference::ALT_REF_QNH;
			return qnhAlt;
		}
	}
	// match boundary/range
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

string TransitionLevel::GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan, int& trans_level, int& elevation)
{
	// returns airport ident and sets trans_level, elevation. Elevation will be 0 if not QFE. Doesn't consider altitude.
	apmap_iter apitr = GetTargetAirport(FlightPlan);
	if (apitr != m_AirportMap.end()) {
		trans_level = apitr->second.trans_level > 0 ? apitr->second.trans_level : m_PluginPtr->GetTransitionAltitude();
		elevation = apitr->second.is_QFE ? apitr->second.elevation : 0;
		return apitr->first;
	}
	// not found, default values
	trans_level = m_DefaultLevel;
	elevation = 0;
	return string();
}

bool TransitionLevel::SetAirportParam(string airport, int trans_level, int isQFE, int range)
{
	// default to -1 for ignoring. isQFE=0 means QNH, trans_level in feet, range in nm
	apmap_iter apitr = m_AirportMap.find(airport);
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

TransitionLevel::apmap_iter TransitionLevel::GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	if (m_AirportMap.empty() || !FlightPlan.IsValid())
		return m_AirportMap.end();
	string callsign = FlightPlan.GetCallsign();
	auto curPos = FlightPlan.GetFPTrackPosition().GetPosition();
	// look up in cache
	auto ap_cached = m_Cache.find(callsign);
	if (ap_cached != m_Cache.end()) {
		auto apitr = m_AirportMap.find(ap_cached->second);
		if (apitr != m_AirportMap.end() && IsinQNHBoundary(curPos, apitr)) {
			return apitr;
		}
	}
	// determine by origin/destination
	string adep = FlightPlan.GetFlightPlanData().GetOrigin();
	string aarr = FlightPlan.GetFlightPlanData().GetDestination();
	double ddep = FlightPlan.GetDistanceFromOrigin();
	double darr = FlightPlan.GetDistanceToDestination();
	// closer
	string acls = ddep < darr ? adep : aarr;
	apmap_iter icls = m_AirportMap.find(acls);
	if (icls != m_AirportMap.end() && IsinQNHBoundary(curPos, icls)) {
		m_Cache[callsign] = acls;
		return icls;
	}
	else { // further
		string afar = ddep > darr ? adep : aarr;
		apmap_iter ifar = m_AirportMap.find(afar);
		if (ifar != m_AirportMap.end() && IsinQNHBoundary(curPos, ifar)) {
			m_Cache[callsign] = afar;
			return ifar;
		}
	}
	return GetTargetAirport(curPos, callsign);
}

TransitionLevel::apmap_iter TransitionLevel::GetTargetAirport(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	if (m_AirportMap.empty() || !RadarTarget.IsValid())
		return m_AirportMap.end();
	if (RadarTarget.GetCorrelatedFlightPlan().IsValid()) {
		return GetTargetAirport(RadarTarget.GetCorrelatedFlightPlan());
	}
	else {
		string sysID = RadarTarget.GetSystemID();
		auto pos = RadarTarget.GetPosition().GetPosition();
		// look up in cache
		auto ap_cached = m_Cache.find(sysID);
		if (ap_cached != m_Cache.end()) {
			apmap_iter apitr = m_AirportMap.find(ap_cached->second);
			if (apitr != m_AirportMap.end()) {
				if (IsinQNHBoundary(pos, apitr)) {
					return apitr;
				}
			}
		}
		return GetTargetAirport(pos, sysID);
	}
}

TransitionLevel::apmap_iter TransitionLevel::GetTargetAirport(EuroScopePlugIn::CPosition Position, string CacheID)
{
	// will not check cache
	map<double, string> distance_airports; // sorted by distance to reduce calculation
	for (const auto& ap : m_AirportMap) {
		double d = ap.second.position.DistanceTo(Position);
		distance_airports[d] = ap.first;
	}
	for (auto& dstap : distance_airports) {
		auto apitr = m_AirportMap.find(dstap.second);
		if (IsinQNHBoundary(Position, apitr)) {
			m_Cache[CacheID] = dstap.second;
			return apitr;
		}
	}
	m_Cache.erase(CacheID);
	return m_AirportMap.end();
}

bool TransitionLevel::IsinQNHBoundary(EuroScopePlugIn::CPosition pos, apmap_iter airport_iter)
{
	// only considers lateral boundary, need to check iter's validity before calling
	// by range
	if (airport_iter->second.in_sector && airport_iter->second.range) {
		double distance = pos.DistanceTo(airport_iter->second.position);
		if (distance < (double)airport_iter->second.range) {
			return true;
		}
	}

	// by boundary check
	pos_vec boundary = airport_iter->second.boundary;
	if (boundary.empty())
		return false;
	vector<double> directions;
	for (size_t i = 0; i < boundary.size(); directions.push_back(pos.DirectionTo(boundary[i++])));
	vector<double> angles;
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
