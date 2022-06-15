// RouteChecker.h

#pragma once

#include "pch.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "MetricAlt.h"

using namespace std;

namespace RouteCheckerConstants {
	const int NOT_FOUND = -1;
	const int INVALID = 0;
	const int PARTIAL_NO_LEVEL = 1;
	const int COMPLETE_NO_LEVEL = 2;
	const int PARTIAL_OK_LEVEL = 11;
	const int COMPLETE_OK_LEVEL = 12;
}

class RouteChecker
{
public:
	RouteChecker(EuroScopePlugIn::CPlugIn* plugin, string filename);
	~RouteChecker(void);

	list<string> GetRouteInfo(string departure, string arrival); // for string display
	int CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan, bool refresh = false);
	void RemoveCache(EuroScopePlugIn::CFlightPlan FlightPlan);

private:
	struct RouteData {
		string m_Name;
		string m_EvenO; // accepts a combination of SE SO FE FO. S/F - m/ft, E/O - Even/Odd. No need for seperation marks
		string m_FixAltStr; // accepts a combination of alt Sxxx(m*100) Fxxx(ft*100), seperated by '/'
		string m_MinAlt; // in feet
		string m_Route;
		string m_Remark;
	};

	unordered_map<string, unordered_set<string>> m_SIDSTAR; // ICAO -> set <SID/STAR>
	unordered_map<string, list<RouteData>> m_Data; // map string store: "ZSSSZGGG" OD pair
	unordered_map<string, int> m_Cache; // callsign -> check result

	bool IsRouteValid(string FProute, string DBroute);
	int IsRouteValid(EuroScopePlugIn::CFlightPlanExtractedRoute ExtractedRoute, string DBroute);
	bool IsLevelValid(int planalt, string evenodd, string fixalt, string minalt);
};