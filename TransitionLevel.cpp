// TransitionLevel.cpp

#include "pch.h"
#include "TransitionLevel.h"

const int DISTANCE_THRESHOLD = 50; // nautical miles, for airport nearest RadarTarget

TransitionLevel::TransitionLevel(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
	m_MaxLevel = 0;
}

TransitionLevel::~TransitionLevel(void)
{
}

void TransitionLevel::LoadCSV(string filename)
{
	m_AirportMap.clear();
	m_MaxLevel = 0;

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
			int lvlft(m_PluginPtr->GetTransitionAltitude()), elevft(0), isqfe(0), rngnm(0);
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
			m_AirportMap[apid] = AirportData{
				lvlft,elevft,(bool)isqfe,rngnm,bndvec,false,EuroScopePlugIn::CPosition(),
			};
			m_MaxLevel = max(lvlft, m_MaxLevel);
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
		EuroScopePlugIn::CPosition pos;
		m_AirportMap[se.GetName()].in_sector = se.GetPosition(&pos, 0);
		m_AirportMap[se.GetName()].position = pos;
	}
}

int TransitionLevel::GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference)
{
	// returns radar display altitude and assign reference AltitudeReference::ALT_REF_xxx
	if (!RadarTarget.IsValid()) return 0;
	int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
	if (stdAlt >= m_MaxLevel) {
		reference = AltitudeReference::ALT_REF_QNE;
		return stdAlt;
	}
	apmap_iter apitr = GetTargetAirport(RadarTarget);
	if (apitr == m_AirportMap.end()) { // no boundary match
		reference = AltitudeReference::ALT_REF_QNE;
		return stdAlt;
	}
	// match boundary/range
	int translvl = apitr->second.trans_level;
	int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
	if (stdAlt >= translvl) {
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
		trans_level = apitr->second.trans_level;
		elevation = apitr->second.is_QFE ? apitr->second.elevation : 0;
		return apitr->first;
	}
	// not found, default values
	trans_level = m_PluginPtr->GetTransitionAltitude();
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
	if (!m_AirportMap.empty() && FlightPlan.IsValid()) {
		string callsign = FlightPlan.GetCallsign();
		auto curPos = FlightPlan.GetFPTrackPosition().GetPosition();
		string adep = FlightPlan.GetFlightPlanData().GetOrigin();
		string aarr = FlightPlan.GetFlightPlanData().GetDestination();
		double ddep = FlightPlan.GetDistanceFromOrigin();
		double darr = FlightPlan.GetDistanceToDestination();
		string acls = ddep < darr ? adep : aarr;
		string afar = ddep > darr ? adep : aarr;
		if (IsinQNHBoundary(curPos, m_AirportMap.find(acls))) {
			return m_AirportMap.find(acls);
		}
		else if (IsinQNHBoundary(curPos, m_AirportMap.find(afar))) {
			return m_AirportMap.find(afar);
		}
		else {
			return GetTargetAirport(curPos);
		}
	}
	return m_AirportMap.end();
}

TransitionLevel::apmap_iter TransitionLevel::GetTargetAirport(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	apmap_iter res = m_AirportMap.end();
	if (RadarTarget.IsValid()) {
		if (RadarTarget.GetCorrelatedFlightPlan().IsValid()) {
			return GetTargetAirport(RadarTarget.GetCorrelatedFlightPlan());
		}
		else {
			return GetTargetAirport(RadarTarget.GetPosition().GetPosition());
		}
	}
	return res;
}

TransitionLevel::apmap_iter TransitionLevel::GetTargetAirport(EuroScopePlugIn::CPosition Position)
{
	apmap_iter apitr = m_AirportMap.begin();
	for (; apitr != m_AirportMap.end() && !IsinQNHBoundary(Position, apitr);
		apitr++);
	return apitr;
}

bool TransitionLevel::IsinQNHBoundary(EuroScopePlugIn::CPosition pos, apmap_iter airport_iter)
{
	// lateral boundary only
	if (airport_iter == m_AirportMap.end())
		return false;

	// by range
	if (airport_iter->second.in_sector) {
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
