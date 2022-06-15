// MTEPlugIn.cpp

#include "pch.h"
#include "RouteChecker.h"

RouteChecker::RouteChecker(EuroScopePlugIn::CPlugIn* plugin, string filename)
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
	// read sector file SID/STAR
	for (auto se = plugin->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_SIDS_STARS);
		se.IsValid();
		se = plugin->SectorFileElementSelectNext(se, EuroScopePlugIn::SECTOR_ELEMENT_SIDS_STARS)) {
		m_SIDSTAR[se.GetAirportName()].insert(se.GetName());
	}
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
	for (auto& rd : routes) {
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

int RouteChecker::CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan, bool refresh)
{
	/*
	see RouteCheckerConstants:: for returns
	:refresh = true will force refresh, otherwise will look up in cache
	*/

	if (!refresh) {
		auto r = m_Cache.find(FlightPlan.GetCallsign());
		if (r != m_Cache.end())
			return r->second;
	}
	int res;
	EuroScopePlugIn::CFlightPlanData fpd = FlightPlan.GetFlightPlanData();
	string od = string(fpd.GetOrigin()) + string(fpd.GetDestination());
	auto rteit = m_Data.find(od);
	if (rteit == m_Data.end()) {
		res = -1;
	}
	else {
		res = 0;
		for (auto& rd : rteit->second) {
			int rvalid = IsRouteValid(fpd.GetRoute(), rd.m_Route) ? 2 : IsRouteValid(FlightPlan.GetExtractedRoute(), rd.m_Route);
			bool lvalid = rvalid ? IsLevelValid(FlightPlan.GetFinalAltitude(), rd.m_EvenO, rd.m_FixAltStr, rd.m_MinAlt) : false;
			res = max(rvalid + (int)lvalid * 10, res); // corresponds RouteCheckerConstants
			if (res == RouteCheckerConstants::COMPLETE_OK_LEVEL)
				break;
		}
	}
	m_Cache[FlightPlan.GetCallsign()] = res;
	return res;
}

void RouteChecker::RemoveCache(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	m_Cache.erase(FlightPlan.GetCallsign());
}

bool RouteChecker::IsRouteValid(string FProute, string DBroute)
{
	string temproute = FProute;
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
	return newroute.find(" " + DBroute + " ") != string::npos;
}

int RouteChecker::IsRouteValid(EuroScopePlugIn::CFlightPlanExtractedRoute ExtractedRoute, string DBroute)
{
	// returns: 0-not valid, 1-partially valid, 2-valid
	bool partial = false;
	int i(1), m(ExtractedRoute.GetPointsNumber());
	int j(0), n(0);

	// load Extracted route
	typedef struct { string via_; string to_; int cls_; }_plnpoint;
	vector<_plnpoint> plnvec;
	for (i = 1; i < m; i++) {
		plnvec.push_back({ ExtractedRoute.GetPointAirwayName(i), ExtractedRoute.GetPointName(i), ExtractedRoute.GetPointAirwayClassification(i) });
	}
	m = plnvec.size();

	// parse database route
	vector<string> rtevec;
	istringstream ssrt(DBroute);
	string strt = "";
	while (ssrt >> strt) {
		rtevec.push_back(strt);
		n++;
	}

	// make point i the 1st waypoint of route (out of SID)
	auto setsid = m_SIDSTAR.find(ExtractedRoute.GetPointName(0));
	if (setsid != m_SIDSTAR.end()) {
		for (i = 0; i < m &&
			setsid->second.find(plnvec[i].via_) != setsid->second.end();
			i++);
		if (plnvec[i].cls_ != EuroScopePlugIn::AIRWAY_CLASS_NO_DATA_DIRECT)
			i--;
	}

	// compare vec to extracted route
	for (j = 0; j < n && rtevec[j] != plnvec[i].to_; j++);
	if (j == n) { // starting point not found in rtevec
		partial = true;
		i++;
		for (j = 0; j < n && rtevec[j] != plnvec[i].via_; j++); // locate starting airway
		if (j == n) // starting airway not found in rtevec
			return 0;
	}
	while (i < m && j < n) {
		if (rtevec[j] == plnvec[i].via_) {
			if (j + 1 < n && rtevec[j + 1] == plnvec[i].to_)
				j += 2;
			i++;
		}
		else if (rtevec[j] == plnvec[i].to_) {
			i++;
			j++;
		}
		else {
			break;
		}
	}
	if (j == n) {
		return partial ? 1 : 2;
	}
	else if (i == m) {
		return 1;
	}
	else {
		auto setstar = m_SIDSTAR.find(ExtractedRoute.GetPointName(m));// check STAR
		if (setstar != m_SIDSTAR.end() &&
			setstar->second.find(plnvec[i].via_) != setstar->second.end()) {
			return partial ? 1 : 2;
		}
	}
	return 0;
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
