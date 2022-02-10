// RouteChecker.h

#pragma once

#include "pch.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <regex>
#include <unordered_map>
#include "MetricAlt.h"

using namespace std;

struct RouteData {
	string m_Name;
	string m_EvenO; // accepts a combination of SE SO FE FO. S/F - m/ft, E/O - Even/Odd. No need for seperation marks
	string m_FixAltStr; // accepts a combination of alt Sxxx(m*100) Fxxx(ft*100), seperated by '/'
	string m_MinAlt; // in feet
	string m_Route;
	string m_Remark;
};

class RouteChecker
{
public:
	RouteChecker(string filename);
	~RouteChecker(void);

	list<string> GetRouteInfo(string departure, string arrival); // for string display
	char CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan);

private:
	unordered_map<string, list<RouteData>> m_Data; // map string store: "ZSSSZGGG" OD pair

	bool IsRouteValid(string planroute, string realroute);
	bool IsLevelValid(int planalt, string evenodd, string fixalt, string minalt);
};