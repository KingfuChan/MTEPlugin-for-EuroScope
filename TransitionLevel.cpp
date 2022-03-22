// TransitionLevel.cpp

#include "pch.h"
#include "TransitionLevel.h"

TransitionLevel::TransitionLevel(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
}

TransitionLevel::~TransitionLevel(void)
{
}

void TransitionLevel::LoadCSV(string filename)
{
	m_TransLevelMap.clear();
	m_AirportPosMap.clear();
	// external file
	ifstream inFile;
	inFile.open(filename, ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		throw string("unable to open file");
	}
	string line;
	getline(inFile, line);
	if (line != "Airport,Level") { // confirm header
		inFile.close();
		throw string("invalid column names");
	}
	while (getline(inFile, line)) {
		istringstream ssin(line);
		string aplist, lvlstr;
		getline(ssin, aplist, ',');
		getline(ssin, lvlstr);
		int lvlft;
		if (sscanf_s(lvlstr.c_str(), "F%d", &lvlft)) {
			lvlft = lvlft * 100;
		}
		else if (sscanf_s(lvlstr.c_str(), "S%d", &lvlft)) {
			lvlft = MetricAlt::LvlMtoFeet(lvlft * 100);
		}
		else {
			continue;
		}
		// split airpots and assign level
		string a;
		for (istringstream ssapl(aplist); getline(ssapl, a, '/');) {
			m_TransLevelMap[a] = lvlft;
		}
	}
	inFile.close();
	// sector file
	for (auto se = m_PluginPtr->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT);
		se.IsValid();
		se = m_PluginPtr->SectorFileElementSelectNext(se, EuroScopePlugIn::SECTOR_ELEMENT_AIRPORT)) {
		EuroScopePlugIn::CPosition pos;
		if (se.GetPosition(&pos, 0)) {
			m_AirportPosMap.insert({ se.GetName(), pos });
		}
	}
}

int TransitionLevel::GetTransitionLevel(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	if (!m_TransLevelMap.empty() && FlightPlan.IsValid()) {
		string adep = FlightPlan.GetFlightPlanData().GetOrigin();
		string aarr = FlightPlan.GetFlightPlanData().GetDestination();
		auto ldep = m_TransLevelMap.find(adep);
		auto larr = m_TransLevelMap.find(aarr);
		auto curPos = FlightPlan.GetFPTrackPosition().GetPosition();
		auto pdep = m_AirportPosMap.find(adep);
		auto parr = m_AirportPosMap.find(aarr);
		double ddep = pdep != m_AirportPosMap.end() ? curPos.DistanceTo(pdep->second) : FlightPlan.GetDistanceFromOrigin();
		double darr = parr != m_AirportPosMap.end() ? curPos.DistanceTo(parr->second) : FlightPlan.GetDistanceToDestination();
		auto& lres = ddep < darr ? ldep : larr;
		if (lres != m_TransLevelMap.end())
			return lres->second;
	}
	return m_PluginPtr->GetTransitionAltitude();
}

int TransitionLevel::GetTransitionLevel(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	if (RadarTarget.IsValid()) {
		auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
		if (FlightPlan.IsValid()) {
			return GetTransitionLevel(FlightPlan);
		}
		else if (!m_TransLevelMap.empty() && !m_AirportPosMap.empty()) {
			double minDist(50); // within 50 nautical miles
			int minL = m_PluginPtr->GetTransitionAltitude();
			for (auto& l : m_TransLevelMap) {
				auto p = m_AirportPosMap.find(l.first);
				if (p != m_AirportPosMap.end()) {
					double d = RadarTarget.GetPosition().GetPosition().DistanceTo(p->second);
					if (d < minDist) {
						minDist = d;
						minL = l.second;
					}
				}
			}
			return minL;
		}
	}
	return m_PluginPtr->GetTransitionAltitude();
}
