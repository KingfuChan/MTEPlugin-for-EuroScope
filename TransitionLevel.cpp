// TransitionLevel.cpp

#include "pch.h"
#include "TransitionLevel.h"

const int DISTANCE_THRESHOLD = 50; // nautical miles, for airport nearest RadarTarget

TransitionLevel::TransitionLevel(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
}

TransitionLevel::~TransitionLevel(void)
{
}

void TransitionLevel::LoadCSV(string filename)
{
	m_AirportMap.clear();
	// external file
	ifstream inFile;
	inFile.open(filename, ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		throw string("unable to open file");
	}
	string line;
	getline(inFile, line);
	if (line != "Ident,TransLevel,Elevation,QFERange") { // confirm header
		inFile.close();
		throw string("invalid column names");
	}
	while (getline(inFile, line)) {
		istringstream ssin(line);
		string apid, lvlstr, elevstr, rangestr;
		getline(ssin, apid, ',');
		getline(ssin, lvlstr, ',');
		getline(ssin, elevstr, ',');
		getline(ssin, rangestr);
		int lvlft(0), elevft(0), rangenm(0); // use 0 to re-initialize
		if (sscanf_s(lvlstr.c_str(), "F%d", &lvlft)) {
			m_AirportMap[apid].trans_level = lvlft * 100;
		}
		else if (sscanf_s(lvlstr.c_str(), "S%d", &lvlft)) {
			m_AirportMap[apid].trans_level = MetricAlt::LvlMtoFeet(lvlft * 100);
		}
		else {
			m_AirportMap[apid].trans_level = 0;
		}
		if (sscanf_s(elevstr.c_str(), "%d", &elevft)) {
			m_AirportMap[apid].elevation = elevft;
		}
		if (sscanf_s(rangestr.c_str(), "%d", &rangenm)) {
			m_AirportMap[apid].QFE_range = rangenm;
		}
	}
	inFile.close();
	// sector file
	for (auto se = m_PluginPtr->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT);
		se.IsValid();
		se = m_PluginPtr->SectorFileElementSelectNext(se, EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT)) {
		EuroScopePlugIn::CPosition pos;
		m_AirportMap[se.GetName()].in_sector = se.GetPosition(&pos, 0);
		m_AirportMap[se.GetName()].position = pos;
	}
}

int TransitionLevel::GetTransitionLevel(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	auto apitr = GetTargetAirport(FlightPlan);
	if (apitr != m_AirportMap.end()) {
		int trslvl = apitr->second.trans_level;
		if (trslvl)
			return trslvl;
	}
	return m_PluginPtr->GetTransitionAltitude();
}

int TransitionLevel::GetTransitionLevel(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	auto apitr = GetTargetAirport(RadarTarget);
	if (apitr != m_AirportMap.end()) {
		int trslvl = apitr->second.trans_level;
		if (trslvl)
			return trslvl;
	}
	return m_PluginPtr->GetTransitionAltitude();
}

int TransitionLevel::GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference)
{
	// returns radar display altitude and assign reference AltitudeReference::ALT_REF_xxx
	if (!RadarTarget.IsValid()) return 0;
	int translvl = GetTransitionLevel(RadarTarget);
	int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
	int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
	if (stdAlt >= translvl) {
		reference = AltitudeReference::ALT_REF_QNE;
		return stdAlt;
	}
	else {
		auto apitr = GetTargetAirport(RadarTarget);
		if (apitr != m_AirportMap.end() && apitr->second.in_sector) {
			double distance = apitr->second.position.DistanceTo(RadarTarget.GetPosition().GetPosition());
			if (distance <= apitr->second.QFE_range) {
				reference = AltitudeReference::ALT_REF_QFE;
				return qnhAlt - apitr->second.elevation;
			}
		}
		reference = AltitudeReference::ALT_REF_QNH;
		return qnhAlt;
	}
}

unordered_map<string, TransitionLevel::AirportData>::iterator TransitionLevel::GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	if (!m_AirportMap.empty() && FlightPlan.IsValid()) {
		string adep = FlightPlan.GetFlightPlanData().GetOrigin();
		string aarr = FlightPlan.GetFlightPlanData().GetDestination();
		auto itrdep = m_AirportMap.find(adep);
		auto itrarr = m_AirportMap.find(aarr);
		auto curPos = FlightPlan.GetFPTrackPosition().GetPosition();
		double ddep = itrdep != m_AirportMap.end() && itrdep->second.in_sector ? curPos.DistanceTo(itrdep->second.position) : FlightPlan.GetDistanceFromOrigin();
		double darr = itrarr != m_AirportMap.end() && itrarr->second.in_sector ? curPos.DistanceTo(itrarr->second.position) : FlightPlan.GetDistanceToDestination();
		return ddep < darr ? itrdep : itrarr;
	}
	return m_AirportMap.end();
}

unordered_map<string, TransitionLevel::AirportData>::iterator TransitionLevel::GetTargetAirport(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	auto res = m_AirportMap.end();
	if (RadarTarget.IsValid()) {
		auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
		if (FlightPlan.IsValid()) {
			return GetTargetAirport(FlightPlan);
		}
		else if (!m_AirportMap.empty()) {
			double minDist = DISTANCE_THRESHOLD;
			auto rtpos = RadarTarget.GetPosition().GetPosition();
			for (auto ap = m_AirportMap.begin(); ap != m_AirportMap.end(); ap++) {
				if (!ap->second.in_sector) continue;
				double d = rtpos.DistanceTo(ap->second.position);
				if (d < minDist) {
					minDist = d;
					res = ap;
				}
			}
		}
	}
	return res;
}
