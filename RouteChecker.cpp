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
		throw string("unable to open file");
	}
	string line;
	getline(inFile, line);
	if (line != "Dep,Arr,Name,EvenOdd,AltList,MinAlt,Route,Remarks") { // confirm header
		inFile.close();
		throw string("invalid column names");
	}
	while (getline(inFile, line)) {
		istringstream ssin(line);
		RouteData rd;
		string dep, arr, listeo, listalt, minalt;
		string eo, la, ma;
		getline(ssin, dep, ',');
		getline(ssin, arr, ',');
		getline(ssin, rd.m_Name, ',');
		getline(ssin, rd.m_EvenO, ',');
		getline(ssin, rd.m_FixAltStr, ',');
		getline(ssin, rd.m_MinAlt, ',');
		getline(ssin, rd.m_Route, ',');
		getline(ssin, rd.m_Remark);

		// split departure and arrival airpots and assign route
		string d, a;
		for (istringstream ssdep(dep); getline(ssdep, d, '/');) {
			for (istringstream ssarr(arr); getline(ssarr, a, '/');) {
				m_Data[d + a].push_back(rd);
			}
		}
	}
	inFile.close();
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
		if (rd.m_Name.size())
			info = "(" + rd.m_Name + ")  " + info;
		if (rd.m_FixAltStr.size())
			info += "  @ " + rd.m_FixAltStr;
		if (rd.m_EvenO.size())
			info += "  @ " + rd.m_EvenO;
		if (rd.m_MinAlt.size())
			info += "  @ >= " + rd.m_MinAlt;
		if (rd.m_Remark.size())
			info += "  & RMK: " + rd.m_Remark;
		res.push_back(info);
	}
	return res;
}

char RouteChecker::CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// ? - no route for OD pair
	// Y - route and alt ok 
	// L - route ok, alt not
	// X - route not ok
	// space - clearance received flag set
	if (FlightPlan.GetClearenceFlag())
		return ' ';
	EuroScopePlugIn::CFlightPlanData fpd = FlightPlan.GetFlightPlanData();
	string od = string(fpd.GetOrigin()) + string(fpd.GetDestination());
	list<RouteData> routes;
	try {
		routes = m_Data.at(od);
	}
	catch (out_of_range e) {
		return '?';
	}
	char res = 'X';
	for (auto rd : routes) {
		if (IsRouteValid(fpd.GetRoute(), rd.m_Route)) {
			if (IsLevelValid(FlightPlan.GetFinalAltitude(), rd.m_EvenO, rd.m_FixAltStr, rd.m_MinAlt))
				return 'Y';
			else
				res = 'L';
		}
	}
	return res;
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
	// find route, note both newroute and realroute starts and ends with space
	return newroute.find(" " + realroute + " ") != string::npos;
}

bool RouteChecker::IsLevelValid(int planalt, string evenodd, string fixalt, string minalt)
{
	// considers even/odd, fixed altitudes and restrictions
	int malt = 0;
	sscanf_s(minalt.c_str(), "%d", &malt); // it's ok to be empty
	if (planalt < malt) return false;
	if (fixalt.empty()) {
		return !evenodd.size() || evenodd.find(MetricAlt::LvlFeetEvenOdd(planalt)) != string::npos;
	}
	else { // there is restrictions
		istringstream fass(fixalt);
		int falt = 0;
		for (string r; getline(fass, r, '/');) {
			if (sscanf_s(r.c_str(), "S%d", &falt))
				falt = MetricAlt::LvlMtoFeet(falt * 100);
			else if (sscanf_s(r.c_str(), "F%d", &falt)) {
				falt = falt * 100;
			}
			if (falt && falt == planalt)
				return true;
		}
	}
	return false;
}
