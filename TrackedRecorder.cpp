//TrackedRecorder.cpp

#include "pch.h"
#include "TrackedRecorder.h"

TrackedRecorder::TrackedRecorder(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
	sc_Thread = std::jthread(std::bind_front(&TrackedRecorder::SimilarCallsignThread, this));
}

TrackedRecorder::~TrackedRecorder(void)
{
	sc_Thread.request_stop();
	sc_Thread.join();
}

void TrackedRecorder::UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, const bool online)
{
	// Pass online=false to deactivate when disconnecting.
	if (m_SuppressUpdate == true) return;
	std::unique_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r != m_TrackedMap.end()) {
		if (FlightPlan.GetTrackingControllerIsMe()) {
			// recorded, tracking. update
			auto tad = AssignedData(FlightPlan);
			bool rfsc = !(
				tad.m_ScratchPad == r->second.m_AssignedData.m_ScratchPad &&
				tad.m_CommType == r->second.m_AssignedData.m_CommType &&
				r->second.m_Offline != online
				);
			r->second.m_AssignedData = tad;
			r->second.m_Offline = !online;
			if (rfsc) {
				mlock.unlock();
				RefreshSimilarCallsign();
			}
		}
		else if (!r->second.m_Offline) {
			// recorded, not tracking, not offline. remove
			m_TrackedMap.erase(r);
			mlock.unlock();
			RefreshSimilarCallsign();
		}
	}
	else if (FlightPlan.GetTrackingControllerIsMe()) {
		// not recorded but tracking. add
		TrackedData trd{ FlightPlan.GetCorrelatedRadarTarget().GetSystemID(), AssignedData(FlightPlan), m_DefaultFeet };
		m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
		mlock.unlock();
		RefreshSimilarCallsign();
	}
}

void TrackedRecorder::UpdateFlight(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	// Only useful if correlated.
	if (m_SuppressUpdate == true) return;
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (!FlightPlan.IsValid())
		return;
	std::unique_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r != m_TrackedMap.end()) {
		if (FlightPlan.GetTrackingControllerIsMe()) {
			// recorded, tracking. update
			r->second.m_SystemID = RadarTarget.GetSystemID();
		}
		else if (!r->second.m_Offline) {
			// recorded, not tracking, not offline. remove
			m_TrackedMap.erase(r);
			mlock.unlock();
			RefreshSimilarCallsign();
		}
	}
	else if (FlightPlan.GetTrackingControllerIsMe()) {
		// not recorded, tracking. add
		TrackedData trd{ RadarTarget.GetSystemID(), AssignedData(FlightPlan), m_DefaultFeet };
		m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
		mlock.unlock();
		RefreshSimilarCallsign();
	}
}

bool TrackedRecorder::IsCommEstablished(const std::string& callsign)
{
	std::shared_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CommEstbed;
	else
		return true;
}

void TrackedRecorder::SetCommEstablished(const std::string& callsign)
{
	std::unique_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CommEstbed = true;
}

bool TrackedRecorder::IsCFLConfirmed(const std::string& callsign)
{
	std::shared_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CFLConfirmed;
	else
		return true;
}

void TrackedRecorder::SetCFLConfirmed(const std::string& callsign, const bool confirmed)
{
	std::unique_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CFLConfirmed = confirmed;
}

bool TrackedRecorder::IsForceFeet(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget)
{
	// priority: m_TempUnitSysID (SystemID) > m_TrackedMap (Callsign) >= m_TrackedMap (SystemID)
	bool isfeet = m_DefaultFeet;
	std::string callsign = FlightPlan.IsValid() ? FlightPlan.GetCallsign() : "";
	std::string systemid = RadarTarget.IsValid() ? RadarTarget.GetSystemID() : "";
	std::shared_lock mlock(tr_Mutex), ulock(usysi_Mutex);
	auto r = std::find_if(m_TrackedMap.begin(), m_TrackedMap.end(),
		[callsign, systemid](const auto& tr) {
			return tr.first == callsign || tr.second.m_SystemID == systemid;
		});
	std::string newid = systemid;
	if (r != m_TrackedMap.end()) {
		isfeet = r->second.m_ForceFeet;
		newid = r->second.m_SystemID;
	}
	return !newid.empty() && m_TempUnitSysID.contains(newid) ? !isfeet : isfeet;
}

void TrackedRecorder::SetAltitudeUnit(const std::string& callsign, const bool& feet)
{
	std::unique_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_ForceFeet = feet;
}

void TrackedRecorder::ResetAltitudeUnit(const bool& feet)
{
	m_DefaultFeet = feet;
	std::unique_lock mlock(tr_Mutex);
	for (auto& [c, d] : m_TrackedMap) {
		d.m_ForceFeet = feet;
	}
}

bool TrackedRecorder::IsSquawkDUPE(const std::string& callsign)
{
	std::shared_lock mlock(tr_Mutex);
	const auto r1 = m_TrackedMap.find(callsign);
	if (r1 == m_TrackedMap.end())
		return false;
	return std::any_of(m_TrackedMap.begin(), m_TrackedMap.end(), [&](const auto& r2) {
		return !r2.second.m_Offline &&
			r1->first != r2.first &&
			r1->second.m_AssignedData.m_Squawk == r2.second.m_AssignedData.m_Squawk;
		});
}

bool TrackedRecorder::IsActive(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	std::shared_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r != m_TrackedMap.end())
		return !r->second.m_Offline;
	else
		return true;
}

bool TrackedRecorder::IsActive(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (FlightPlan.IsValid())
		return IsActive(FlightPlan);
	else {
		auto r = GetTrackedDataBySystemID(RadarTarget.GetSystemID());
		std::shared_lock mlock(tr_Mutex);
		return r != m_TrackedMap.end() ? !r->second.m_Offline : true;
	}
}

bool TrackedRecorder::IsSimilarCallsign(const std::string& callsign)
{
	std::shared_lock lock(sc_Mutex);
	return m_SCSetMap.find(callsign) != m_SCSetMap.end();
}

std::unordered_set<std::string> TrackedRecorder::GetSimilarCallsigns(const std::string& callsign)
{
	std::shared_lock lock(sc_Mutex);
	auto f = m_SCSetMap.find(callsign);
	if (f != m_SCSetMap.end())
		return f->second;
	else
		return std::unordered_set<std::string>();
}

bool TrackedRecorder::SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// returns true if success
	std::unique_lock mlock(tr_Mutex);
	auto trd = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (trd == m_TrackedMap.end())
		return false;
	// radar target
	if (!FlightPlan.GetCorrelatedRadarTarget().IsValid() && trd->second.m_SystemID.size()) {
		auto corrRT = m_PluginPtr->RadarTargetSelect(trd->second.m_SystemID.c_str());
		if (corrRT.IsValid()) {
			FlightPlan.CorrelateWithRadarTarget(corrRT);
		}
	}
	// flight plan
	m_SuppressUpdate = true; // protect it from removing by following assigns
	TrackedData trd1 = trd->second;
	AssignedData tad(trd1.m_AssignedData);
	auto asd = FlightPlan.GetControllerAssignedData();
	asd.SetSquawk(tad.m_Squawk.c_str());
	asd.SetFinalAltitude(tad.m_FinalAlt);
	asd.SetClearedAltitude(tad.m_ClearedAlt);
	asd.SetCommunicationType(tad.m_CommType);
	asd.SetScratchPadString(tad.m_ScratchPad.c_str());
	if (tad.m_Speed)
		asd.SetAssignedSpeed(tad.m_Speed);
	else if (tad.m_Mach)
		asd.SetAssignedMach(tad.m_Mach);
	asd.SetAssignedRate(tad.m_Rate);
	if (tad.m_Heading)
		asd.SetAssignedHeading(tad.m_Heading);
	else if (tad.m_DCTName.size())
		asd.SetDirectToPointName(tad.m_DCTName.c_str());
	for (size_t i = 0; i < 9; i++) {
		asd.SetFlightStripAnnotation(i, tad.m_StripAnno[i].c_str());
	}
	trd1.m_Offline = false;
	trd->second = trd1;
	mlock.unlock();
	RefreshSimilarCallsign();
	m_SuppressUpdate = false;
	return FlightPlan.StartTracking();
}

bool TrackedRecorder::SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	// returns true if success
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (FlightPlan.IsValid()) {
		return SetTrackedData(FlightPlan);
	}
	else {
		auto trp = GetTrackedDataBySystemID(RadarTarget.GetSystemID());
		std::shared_lock mlock(tr_Mutex);
		if (trp != m_TrackedMap.end()) {
			auto corrFP = m_PluginPtr->FlightPlanSelect(trp->first.c_str());
			if (corrFP.IsValid()) {
				RadarTarget.CorrelateWithFlightPlan(corrFP); // set data at next radar refresh
			}
		}
	}
	return false;
}

bool TrackedRecorder::ToggleAltitudeUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const int duration)
{
	// set TR->ForceFeet for duration seconds, return true is keyboard alt is pressed
	if (!RadarTarget.IsValid() || !(GetAsyncKeyState(VK_MENU) & 0x8000)) return false; // keyboard alt is not pressed
	std::unique_lock uslock(usysi_Mutex);
	std::unique_lock utlock(uthrd_Mutex);
	std::string systemid = RadarTarget.GetSystemID();
	auto ucitr = m_TempUnitSysID.find(systemid);
	if (ucitr != m_TempUnitSysID.end()) { // there is a previous toggle
		m_TempUnitSysID.erase(ucitr);
		uslock.unlock();
		m_TempUnitThread.erase(systemid); // implies stop request
		utlock.unlock();
	}
	else { // start new toggle
		// clean up previous thread
		std::erase_if(m_TempUnitThread, [this](const auto& thrd) {
			const auto& [i, t] = thrd;
			return !m_TempUnitSysID.contains(i);
			});
		m_TempUnitSysID.insert(systemid);
		uslock.unlock();
		auto jt = std::make_shared<std::jthread>([this, systemid, duration](std::stop_token stoken) {
			std::mutex t_mutex;
			std::unique_lock t_lock(t_mutex);
			std::condition_variable_any().wait_for(t_lock, stoken, std::chrono::seconds(duration), [] {
				return false;
				});
			if (!stoken.stop_requested()) {
				std::unique_lock uclock1(usysi_Mutex);
				m_TempUnitSysID.erase(systemid);
			}
			});
		m_TempUnitThread.insert({ systemid,jt });
		utlock.unlock();
	}
	return true;
}

bool TrackedRecorder::IsDisplayVerticalSpeed(const std::string& systemID)
{
	std::shared_lock v_lock(vsdsp_Mutex);
	if (m_DisplayVS.contains(systemID)) {
		return !m_GlobalVS;
	}
	return m_GlobalVS;
}

void TrackedRecorder::ToggleVerticalSpeed(const std::string& systemID)
{
	std::unique_lock v_lock(vsdsp_Mutex);
	if (!m_DisplayVS.erase(systemID)) {
		m_DisplayVS.insert(systemID);
	}
}

void TrackedRecorder::ToggleVerticalSpeed(const bool& display)
{
	std::unique_lock v_lock(vsdsp_Mutex);
	m_DisplayVS.clear();
	m_GlobalVS = display;
}

std::unordered_map<std::string, TrackedRecorder::TrackedData>::iterator TrackedRecorder::GetTrackedDataBySystemID(const std::string& systemID)
{
	std::shared_lock mlock(tr_Mutex);
	return std::find_if(m_TrackedMap.begin(), m_TrackedMap.end(), [&systemID](const auto& d) { return d.second.m_SystemID == systemID; });
}

void TrackedRecorder::RefreshSimilarCallsign(void)
{
	sc_NeedRefresh = true;
	sc_CondVar.notify_all();
}

void TrackedRecorder::SimilarCallsignThread(std::stop_token stoken)
{
	std::mutex t_mutex;
	while (!stoken.stop_requested()) {
		std::unique_lock t_lock(t_mutex);
		bool refreshing = sc_CondVar.wait(t_lock, stoken, [this, stoken] {
			return sc_NeedRefresh && !stoken.stop_requested();
			});
		if (!refreshing) continue;
		sc_NeedRefresh = false;
		// go on to refresh
		std::unique_lock lock(sc_Mutex);
		m_SCSetMap.clear();
		std::unordered_set<std::string> setENG, setCHN;
		std::shared_lock mlock(tr_Mutex);
		for (const auto& [c, d] : m_TrackedMap) {
			if (d.m_Offline ||
				toupper(d.m_AssignedData.m_CommType) == 'T') {
				continue;
			}
			std::string cal = c.substr(0, 3);
			if (m_CHNCallsign.contains(cal)) {
				std::string scratch = d.m_AssignedData.m_ScratchPad;
				transform(scratch.begin(), scratch.end(), scratch.begin(), ::toupper);
				size_t epos = scratch.find("EN");
				if (epos != std::string::npos) {
					char delim_l = epos > 0 ? scratch[epos - 1] : '\0';
					char delim_r = epos + 2 < scratch.size() ? scratch[epos + 2] : '\0';
					if (m_EngDelimiter.find(delim_l) != std::string::npos ||
						m_EngDelimiter.find(delim_r) != std::string::npos) {
						setENG.insert(c);
					}
					else {
						setCHN.insert(c);
					}
				}
				else {
					setCHN.insert(c);
				}
			}
			else {
				setENG.insert(c);
			}
		}
		for (const auto& cset : { setENG,setCHN }) {
			for (const auto& c1 : cset) {
				for (const auto& c2 : cset) {
					if (c1 != c2 && CompareCallsign(c1, c2)) {
						m_SCSetMap[c1].insert(c2);
					}
				}
			}
		}
	}
}
