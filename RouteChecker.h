// RouteChecker.h

#pragma once

#include "pch.h"
#include "framework.h"
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
	string m_Route;
	string m_EvenO;
	string m_Restr;
};

class RouteChecker
{
public:
	bool m_IsValid; // if something goes wrong when initializing, this will be false

	RouteChecker(string filename);
	~RouteChecker(void);

	list<string> GetRouteInfo(string departure, string arrival); // for string display
	char CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan);

private:
	unordered_map<string, list<RouteData>> m_Data; // map string store: "ZSSSZGGG" OD pair

	bool IsRouteValid(string planroute, string realroute);
	bool IsLevelValid(int level, string evenodd, string restriction);
};