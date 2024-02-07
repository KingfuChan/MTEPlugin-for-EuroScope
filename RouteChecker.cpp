// MTEPlugIn.cpp

#include "pch.h"
#include "RouteChecker.h"

RouteChecker::RouteChecker(EuroScopePlugIn::CPlugIn* plugin, const std::string& filename)
{
	std::ifstream inFile;
	inFile.open(filename, std::ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		throw std::string("unable to open file");
	}
	std::string line;
	getline(inFile, line);
	if (line != "Dep,Arr,Name,EvenOdd,AltList,MinAlt,Route,Remarks") { // confirm header
		inFile.close();
		throw std::string("invalid column names");
	}
	try {
		while (getline(inFile, line)) {
			std::istringstream ssin(line);
			RouteData rd;
			std::string dep, arr, listeo, listalt, minalt;
			std::string eo, la, ma;
			getline(ssin, dep, ',');
			getline(ssin, arr, ',');
			getline(ssin, rd.m_Name, ',');
			getline(ssin, rd.m_EvenO, ',');
			getline(ssin, rd.m_FixAltStr, ',');
			getline(ssin, rd.m_MinAlt, ',');
			getline(ssin, rd.m_Route, ',');
			getline(ssin, rd.m_Remark);

			// split departure and arrival airpots and assign route
			std::string d, a;
			for (std::istringstream ssdep(dep); getline(ssdep, d, '/');) {
				for (std::istringstream ssarr(arr); getline(ssarr, a, '/');) {
					m_Data[d + a].push_back(rd);
				}
			}
		}
		inFile.close();
	}
	catch (...) {
		inFile.close();
		throw;
	}
	// read sector file SID/STAR
	for (auto se = plugin->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_SIDS_STARS);
		se.IsValid();
		se = plugin->SectorFileElementSelectNext(se, EuroScopePlugIn::SECTOR_ELEMENT_SIDS_STARS)) {
		m_SIDSTAR[se.GetAirportName()].insert(se.GetName());
	}
	// finishing initialization
	m_PluginPtr = plugin;
	q_Thread = std::jthread(std::bind_front(&RouteChecker::UpdateQueueThread, this));
}

RouteChecker::~RouteChecker(void)
{
	q_Thread.request_stop();
	q_Thread.join();
}

std::vector<std::string> RouteChecker::GetRouteInfo(const std::string& departure, const std::string& arrival)
{
	std::vector<std::string> res;
	auto rditr = m_Data.find(departure + arrival);
	if (rditr == m_Data.end()) return res;
	for (auto& rd : rditr->second) {
		std::string info = rd.m_Route;
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

int RouteChecker::CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan, const bool& refresh, const bool& delayed)
{
	// see RouteCheckerConstants for returns. 
	// => refresh=true will force refresh, otherwise will look up in cache
	// => delayed=true will put the request in queue, and will return -1 (Not Found)
	std::string callsign = FlightPlan.GetCallsign();
	if (!refresh) {
		std::shared_lock clock(cache_mutex);
		auto r = m_Cache.find(callsign);
		if (r != m_Cache.end())
			return r->second;
	}
	if (!delayed) {
		return CheckFlightPlan(FlightPlan);
	}
	else { // add to queue and notify thread
		std::lock_guard lock(queue_mutex);
		m_UpdateQueue.push(callsign);
		q_CondVar.notify_all();
		return RouteCheckerConstants::NOT_FOUND;
	}
}

int RouteChecker::CheckFlightPlan(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	int res;
	EuroScopePlugIn::CFlightPlanData fpd = FlightPlan.GetFlightPlanData();
	std::string od = std::string(fpd.GetOrigin()) + std::string(fpd.GetDestination());
	auto rteit = m_Data.find(od);
	if (rteit == m_Data.end()) {
		res = -1;
	}
	else {
		res = 0;
		for (auto& rd : rteit->second) {
			int rvalid = IsRouteValid(fpd.GetRoute(), rd.m_Route) ? 3 : IsRouteValid(FlightPlan.GetExtractedRoute(), rd.m_Route);
			bool lvalid = rvalid ? IsLevelValid(FlightPlan.GetFinalAltitude(), rd.m_EvenO, rd.m_FixAltStr, rd.m_MinAlt) : false;
			res = max(rvalid + (int)lvalid * 10, res); // corresponds RouteCheckerConstants
			if (res == RouteCheckerConstants::TEXT_OK_LEVEL)
				break;
		}
	}
	{
		std::unique_lock clock(cache_mutex);
		m_Cache[FlightPlan.GetCallsign()] = res;
	}
	return res;
}

void RouteChecker::RemoveCache(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	std::unique_lock clock(cache_mutex);
	m_Cache.erase(FlightPlan.GetCallsign());
}

bool RouteChecker::IsRouteValid(const std::string& FProute, const std::string& DBroute)
{
	std::string temproute = FProute;
	// remove dep rwy and alt/spd restrictions
	// e.g. PIK81D/17L PIKAS -> PIK81D PIKAS
	// e.g. W107 SANKO/K0909S1130 A326 AKARA -> W107 SANKO A326 AKARA
	std::regex rex1("/(.+?)(\\s+?)");
	temproute = regex_replace(temproute, rex1, " ");
	// clear all DCTs
	std::regex rex2(" DCT "); // must have space, or points like _DCT_ will be replaced
	temproute = " " + temproute + " "; // for first and last DCT
	temproute = regex_replace(temproute, rex2, " ");
	// shrink route
	// e.g. N620 GUDOR/K0898F360 N620 NALEB/K0898F370 G494 -> N620 NALEB G494
	std::istringstream prss(temproute);
	std::string buf0, buf1, buf2, newroute(" ");
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
	return newroute.find(" " + DBroute + " ") != std::string::npos;
}

int RouteChecker::IsRouteValid(EuroScopePlugIn::CFlightPlanExtractedRoute ExtractedRoute, const std::string& DBroute)
{
	// returns: 0-not valid, 1-partially valid, 2-valid
	bool partial = false;
	std::string dep = ExtractedRoute.GetPointName(0);
	std::string arr = ExtractedRoute.GetPointName(ExtractedRoute.GetPointsNumber() - 1);

	// load Extracted route
	plan_vec plnvec;
	for (size_t i = 1; (int)i < ExtractedRoute.GetPointsNumber() - 1; i++) {
		plnvec.push_back({ ExtractedRoute.GetPointAirwayName(i), ExtractedRoute.GetPointName(i), ExtractedRoute.GetPointAirwayClassification(i) });
	}
	size_t i = 0;
	const size_t m = plnvec.size();

	// parse database route
	std::vector<std::string> rtevec;
	std::istringstream ssrt(DBroute);
	std::string strt = "";
	while (ssrt >> strt) {
		rtevec.push_back(strt);
	}
	size_t j = 0;
	const size_t n = rtevec.size();

	// prevent error caused by empty vec
	if (!m || !n)
		return 0;

	// make point i the 1st waypoint of route (last of SID)
	auto setsid = m_SIDSTAR.find(dep);
	if (setsid != m_SIDSTAR.end()) {
		for (i = 0; i < m &&
			setsid->second.find(plnvec[i].via_) != setsid->second.end();
			i++);
		if (i == m || plnvec[i].cls_ != EuroScopePlugIn::AIRWAY_CLASS_NO_DATA_DIRECT)
			i -= i > 0;
	}

	// compare vec to extracted route
	for (j = 0; j < n && rtevec[j] != plnvec[i].to_; j++);
	partial = j > 0 && plnvec[i].cls_ == EuroScopePlugIn::AIRWAY_CLASS_NO_DATA_DIRECT;
	if (j == n) { // starting point not found in rtevec
		i += (int)(i < m - 1); // make sure i<m
		for (j = 0; j < n && rtevec[j] != plnvec[i].via_; j++); // locate starting airway
		if (j == n) // starting airway not found in rtevec
			return 0;
	}
	while (i < m && j < n) {
		if (rtevec[j] == plnvec[i].via_) {
			if (j + 1 < n && rtevec[j + 1] == plnvec[i].to_) {
				j += 2;
			}
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
		auto setstar = m_SIDSTAR.find(arr);// check STAR
		if (setstar != m_SIDSTAR.end() &&
			setstar->second.find(plnvec[i].via_) != setstar->second.end()) {
			return partial ? 1 : 2;
		}
	}
	return 0;
}

bool RouteChecker::IsLevelValid(const int& planalt, const std::string& evenodd, const std::string& fixalt, const std::string& minalt)
{
	// considers even/odd, fixed altitudes and restrictions
	int malt = 0;
	sscanf_s(minalt.c_str(), "%d", &malt); // it's ok to be empty
	if (planalt < malt) return false;
	if (fixalt.empty()) {
		return !evenodd.size() || evenodd.find(MetricAlt::LvlFeetEvenOdd(planalt)) != std::string::npos;
	}
	else { // there is restrictions
		std::istringstream fass(fixalt);
		int falt = 0;
		for (std::string r; getline(fass, r, '/');) {
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

void RouteChecker::UpdateQueueThread(std::stop_token stoken)
{
	std::mutex t_mutex;
	while (!stoken.stop_requested()) {
		std::unique_lock t_lock(t_mutex);
		bool queueing = q_CondVar.wait(t_lock, stoken, [&] {
			std::lock_guard qlock(queue_mutex);
			return !m_UpdateQueue.empty() && !stoken.stop_requested();
			});
		if (!queueing) continue;
		// updates radar cache
		std::string callsign;
		{
			std::lock_guard qlock(queue_mutex);
			if (m_UpdateQueue.empty()) {
				break;
			}
			else {
				callsign = m_UpdateQueue.front();
				m_UpdateQueue.pop();
			}
		}
		auto fp = m_PluginPtr->FlightPlanSelect(callsign.c_str());
		if (fp.IsValid()) {
			CheckFlightPlan(fp);
		}
	}
}
