// RouteChecker.h

#pragma once

#include "pch.h"
#include "MetricAlt.h"

namespace RouteCheckerConstants {
	const int NOT_FOUND = -1;
	const int INVALID = 0;
	const int PARTIAL_NO_LEVEL = 1;
	const int STRUCT_NO_LEVEL = 2;
	const int TEXT_NO_LEVEL = 3;
	const int PARTIAL_OK_LEVEL = 11;
	const int STRUCT_OK_LEVEL = 12;
	const int TEXT_OK_LEVEL = 13;
}

class RouteChecker
{
public:
	RouteChecker(EuroScopePlugIn::CPlugIn* plugin, std::string filename);
	~RouteChecker(void);

	std::vector<std::string> GetRouteInfo(std::string departure, std::string arrival); // for std::string display
	int CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan, bool refresh = false);
	void RemoveCache(EuroScopePlugIn::CFlightPlan FlightPlan);

private:
	typedef struct {
		std::string m_Name;
		std::string m_EvenO; // accepts a combination of SE SO FE FO. S/F - m/ft, E/O - Even/Odd. No need for seperation marks
		std::string m_FixAltStr; // accepts a combination of alt Sxxx(m*100) Fxxx(ft*100), seperated by '/'
		std::string m_MinAlt; // in feet
		std::string m_Route;
		std::string m_Remark;
	}RouteData;
	typedef struct { std::string via_; std::string to_; int cls_; }plan_point;
	typedef std::vector<plan_point> plan_vec;

	std::unordered_map<std::string, std::unordered_set<std::string>> m_SIDSTAR; // ICAO -> set <SID & STAR>
	std::unordered_map<std::string, std::vector<RouteData>> m_Data; // map std::string store: "ZSSSZGGG" OD pair
	std::unordered_map<std::string, int> m_Cache; // callsign -> check result

	bool IsRouteValid(std::string FProute, std::string DBroute);
	int IsRouteValid(EuroScopePlugIn::CFlightPlanExtractedRoute ExtractedRoute, std::string DBroute);
	bool IsLevelValid(int planalt, std::string evenodd, std::string fixalt, std::string minalt);
};