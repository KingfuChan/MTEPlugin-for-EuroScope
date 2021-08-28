// MTEPlugIn.cpp

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "RouteChecker.h"

RouteChecker::RouteChecker(string filename)
{
	ifstream inFile;
	inFile.open(filename, ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		m_IsValid = false;
		return;
	}
	string line;
	getline(inFile, line);
	if (line != "Dep,Arr,EvenOdd,LevelRestr,Route") { // confirm header
		inFile.close();
		m_IsValid = false;
		return;
	}
	while (getline(inFile, line)) {
		istringstream ssin(line);
		RouteData rd;
		string dep, arr;
		getline(ssin, dep, ',');
		getline(ssin, arr, ',');
		getline(ssin, rd.m_EvenO, ',');
		getline(ssin, rd.m_Restr, ',');
		getline(ssin, rd.m_Route);
		string d, a;
		// split departure and arrival airpots and assign route
		for (istringstream ssdep(dep); getline(ssdep, d, '/');) {
			for (istringstream ssarr(arr); getline(ssarr, a, '/');) {
				m_Data[d + a].push_back(rd);
			}
		}
	}
	inFile.close();
	m_IsValid = true;
}

RouteChecker::~RouteChecker(void)
{

}

list<string> RouteChecker::GetRouteInfo(string departure, string arrival)
{
	list<string> res;
	list<RouteData> routes;
	try {
		routes = m_Data.at(departure + arrival);
	}
	catch (out_of_range e) {
		return res;
	}
	for (auto rd : routes) {
		string info = rd.m_Route;
		if (rd.m_Restr.size())
			info += "  @ (FL)" + rd.m_Restr;
		if (rd.m_EvenO.size())
			info += "  @ " + rd.m_EvenO;
		res.push_back(info);
	}
	return res;
}

char RouteChecker::CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// space - no route for OD pair
	// Y - route and alt ok 
	// L - route ok, alt not
	// N - route not ok
	EuroScopePlugIn::CFlightPlanData fpd = FlightPlan.GetFlightPlanData();
	string od = string(fpd.GetOrigin()) + string(fpd.GetDestination());
	list<RouteData> routes;
	try {
		routes = m_Data.at(od);
	}
	catch (out_of_range e) {
		return ' ';
	}
	for (auto rd : routes) {
		bool lv = IsLevelValid(FlightPlan.GetFinalAltitude(), rd.m_EvenO, rd.m_Restr);
		bool rv = IsRouteValid(fpd.GetRoute(), rd.m_Route);
		if (lv && rv)
			return 'Y'; // passed
		else if (!lv && rv)
			return 'L'; // invalid RFL
	}
	return 'N'; // invalid route
}

bool RouteChecker::IsRouteValid(string planroute, string realroute)
{
	string temproute = planroute;
	// remove dep rwy and alt/spd restrictions
	// e.g. PIK81D/17L PIKAS -> PIK81D PIKAS
	// e.g. W107 SANKO/K0909S1130 A326 AKARA -> W107 SANKO A326 AKARA
	regex rex1("/(.+?)(\\s+?)");
	temproute = regex_replace(temproute, rex1, " ");
	// clear all DCTs
	regex rex2(" DCT "); // must have space, or points like _DCT_ will be replaced
	temproute = " " + temproute + " "; // for first and last DCT
	temproute = regex_replace(temproute, rex2, " ");
	// shrink route
	// e.g. N620 GUDOR/K0898F360 N620 NALEB/K0898F370 G494 -> N620 NALEB G494
	istringstream prss(temproute);
	string buf0, buf1, buf2, newroute(" ");
	while (prss >> buf0) {
		if (buf0 == buf2) {
			buf1 = "";
			buf2 = "";
		}
		if (buf2.size()) {
			newroute += buf2 + " ";
		}
		buf2 = buf1;
		buf1 = buf0;
	}
	newroute += buf2 + " " + buf1 + " ";
	// find route, not both newroute and realroute starts and ends with space
	return newroute.find(" " + realroute + " ") != string::npos;
}

bool RouteChecker::IsLevelValid(int level, string evenodd, string restriction)
{
	// considers even/odd and restriction. note RVSM airspace.
	if (restriction.empty()) { // no restrictions
		return evenodd.empty() || MetricAlt::LvlFeetEvenOdd(level) == evenodd;
	}
	else { // there is restrictions
		istringstream ress(restriction);
		try {
			for (string r; getline(ress, r, '/');) {
				if (level == stoi(r) * 100) {
					return true;
				}
			}
		}
		catch (exception e) {
			return false;
		}
	}
	return false;
}
