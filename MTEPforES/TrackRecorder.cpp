//TrackRecorder.cpp

#include "pch.h"
#include "TrackRecorder.h"

TrackRecorder::TrackRecorder(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
	sc_Thread = std::jthread(std::bind_front(&TrackRecorder::SimilarCallsignThread, this));
}

TrackRecorder::~TrackRecorder(void)
{
	sc_Thread.request_stop();
	sc_Thread.join();
}

void TrackRecorder::UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, const bool online)
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
		TrackedData trd{ FlightPlan.GetCorrelatedRadarTarget().GetSystemID(), AssignedData(FlightPlan), m_CoordFlagAssumed };
		m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
		mlock.unlock();
		RefreshSimilarCallsign();
	}
}

void TrackRecorder::UpdateFlight(EuroScopePlugIn::CRadarTarget RadarTarget)
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
		TrackedData trd{ RadarTarget.GetSystemID(), AssignedData(FlightPlan), m_CoordFlagAssumed };
		m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
		mlock.unlock();
		RefreshSimilarCallsign();
	}
}

bool TrackRecorder::GetCoordinationFlag(const std::string& callsign)
{
	std::shared_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CoordFlag;
	else
		return false;
}

void TrackRecorder::SetCoordinationFlag(const std::string& callsign)
{
	std::unique_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CoordFlag = !r->second.m_CoordFlag;
}

void TrackRecorder::SetCoordinationFlag(const bool& assumed)
{
	m_CoordFlagAssumed = assumed;
}

void TrackRecorder::HandleNewCfl(EuroScopePlugIn::CFlightPlan FlightPlan, const bool& acknowledge)
{
	if (!FlightPlan.IsValid()) return;
	int t = strlen(FlightPlan.GetTrackingControllerId()) ? (FlightPlan.GetTrackingControllerIsMe() ? 1 : -1) : 0;
	CflData cd(t, acknowledge);
	std::unique_lock mlock(cfl_Mutex);
	auto r = m_CflMap.find(FlightPlan.GetCallsign());
	if (r == m_CflMap.end()) {
		m_CflMap.insert({ FlightPlan.GetCallsign(), cd });
	}
	else {
		r->second = cd;
	}
}

bool TrackRecorder::IsCflAcknowledged(const std::string& callsign)
{
	// returns true if not found
	std::shared_lock mlock(cfl_Mutex);
	auto r = m_CflMap.find(callsign);
	if (r != m_CflMap.end())
		return r->second.m_Acknowledged;
	else
		return true;
}

void TrackRecorder::AcknowledgeCfl(const std::string& callsign, const bool confirmed)
{
	std::unique_lock mlock(cfl_Mutex);
	auto r = m_CflMap.find(callsign);
	if (r != m_CflMap.end())
		r->second.m_Acknowledged = confirmed;
}

int TrackRecorder::GetCflElapsedTime(const std::string& callsign)
{
	std::shared_lock mlock(cfl_Mutex);
	auto r = m_CflMap.find(callsign);
	if (r != m_CflMap.end()) {
		auto dura = std::chrono::steady_clock::now() - r->second.m_TimeStamp;
		auto secs = std::chrono::duration_cast<std::chrono::seconds>(dura);
		return (int)secs.count();
	}
	return -1;
}

bool TrackRecorder::IsForceFeet(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// either m_AltUnitCallsign or m_AltUnitSysID will reverse default unit
	if (!FlightPlan.IsValid()) return m_DefaultFeet;
	auto RadarTarget = FlightPlan.GetCorrelatedRadarTarget();
	std::shared_lock clock(ucals_Mutex), slock(usysi_Mutex), tlock(utemp_Mutex);
	bool revc = m_AltUnitCallsign.contains(FlightPlan.GetCallsign());
	bool revs = RadarTarget.IsValid() && m_AltUnitSysID.contains(RadarTarget.GetSystemID());
	bool revt = RadarTarget.IsValid() && m_AltUnitTempo.contains(RadarTarget.GetSystemID());
	return revt != ((revc || revs) != m_DefaultFeet);
}

bool TrackRecorder::IsForceFeet(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	// either m_AltUnitCallsign or m_AltUnitSysID will reverse default unit
	if (!RadarTarget.IsValid()) return m_DefaultFeet;
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	std::shared_lock clock(ucals_Mutex), slock(usysi_Mutex), tlock(utemp_Mutex);
	bool revc = FlightPlan.IsValid() && m_AltUnitCallsign.contains(FlightPlan.GetCallsign());
	bool revs = m_AltUnitSysID.contains(RadarTarget.GetSystemID());
	bool revt = m_AltUnitTempo.contains(RadarTarget.GetSystemID());
	return revt != ((revc || revs) != m_DefaultFeet);
}

void TrackRecorder::SetAltitudeUnit(EuroScopePlugIn::CFlightPlan FlightPlan, const bool& feet)
{
	if (!FlightPlan.IsValid() || IsForceFeet(FlightPlan) == feet) return; // no need to change
	std::unique_lock uclock(ucals_Mutex), uslock(usysi_Mutex);
	std::string callsign = FlightPlan.GetCallsign();
	std::string systemID = FlightPlan.GetCorrelatedRadarTarget().IsValid() ? FlightPlan.GetCorrelatedRadarTarget().GetSystemID() : "";
	if (feet != m_DefaultFeet) {
		m_AltUnitCallsign.insert(callsign);
		if (systemID.size()) {
			m_AltUnitSysID.insert(systemID);
		}
	}
	else {
		m_AltUnitCallsign.erase(callsign);
		m_AltUnitSysID.erase(systemID);
	}
}

void TrackRecorder::SetAltitudeUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const bool& feet)
{
	if (!RadarTarget.IsValid() || IsForceFeet(RadarTarget) == feet) return;
	std::unique_lock uclock(ucals_Mutex), uslock(usysi_Mutex);
	std::string systemID = RadarTarget.GetSystemID();
	std::string callsign = RadarTarget.GetCorrelatedFlightPlan().IsValid() ? RadarTarget.GetCorrelatedFlightPlan().GetCallsign() : "";
	if (feet != m_DefaultFeet) {
		m_AltUnitSysID.insert(systemID);
		if (callsign.size()) {
			m_AltUnitCallsign.insert(callsign);
		}
	}
	else {
		m_AltUnitSysID.erase(systemID);
		m_AltUnitCallsign.erase(callsign);
	}
}

void TrackRecorder::ResetAltitudeUnit(const bool& feet)
{
	m_DefaultFeet = feet;
	std::shared_lock clock(ucals_Mutex), slock(usysi_Mutex);
	m_AltUnitSysID.clear();
	m_AltUnitCallsign.clear();
}

bool TrackRecorder::IsSquawkDUPE(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	if (!RadarTarget.IsValid()) return false;
	if (!RadarTarget.GetPosition().GetTransponderC()) return false;
	bool correlated = RadarTarget.GetCorrelatedFlightPlan().IsValid();
	std::string sq = RadarTarget.GetPosition().GetSquawk();
	for (auto rt1 = m_PluginPtr->RadarTargetSelectFirst(); rt1.IsValid(); rt1 = m_PluginPtr->RadarTargetSelectNext(rt1)) {
		if (!strcmp(rt1.GetSystemID(), RadarTarget.GetSystemID())) continue; // skip itself
		if (rt1.GetPosition().GetTransponderC() && sq == rt1.GetPosition().GetSquawk()) {
			if (correlated || rt1.GetCorrelatedFlightPlan().IsValid()) { // at least one is correlated
				return true;
			}
		}
	}
	return false;
}

bool TrackRecorder::IsActive(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	std::shared_lock mlock(tr_Mutex);
	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r != m_TrackedMap.end())
		return !r->second.m_Offline;
	else
		return true;
}

bool TrackRecorder::IsActive(EuroScopePlugIn::CRadarTarget RadarTarget)
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

bool TrackRecorder::IsSimilarCallsign(const std::string& callsign)
{
	std::shared_lock lock(sc_Mutex);
	return m_SCSetMap.find(callsign) != m_SCSetMap.end();
}

std::unordered_set<std::string> TrackRecorder::GetSimilarCallsigns(const std::string& callsign)
{
	std::shared_lock lock(sc_Mutex);
	auto f = m_SCSetMap.find(callsign);
	if (f != m_SCSetMap.end())
		return f->second;
	else
		return std::unordered_set<std::string>();
}

bool TrackRecorder::SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan)
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

bool TrackRecorder::SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget)
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

bool TrackRecorder::ToggleAltitudeUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const int& duration)
{
	// set TR->ForceFeet for duration seconds (only accepts positive value), return true if toggle
	if (duration <= 0 || !RadarTarget.IsValid() || !(GetAsyncKeyState(VK_MENU) & 0x8000)) return false; // invalid duration or keyboard alt is not pressed
	std::unique_lock uslock(utemp_Mutex);
	std::unique_lock utlock(uthrd_Mutex);
	std::string systemid = RadarTarget.GetSystemID();
	auto ucitr = m_AltUnitTempo.find(systemid);
	if (ucitr != m_AltUnitTempo.end()) { // there is a previous toggle
		m_AltUnitTempo.erase(ucitr);
		uslock.unlock();
		m_TempUnitThread.erase(systemid); // implies stop request
		utlock.unlock();
	}
	else { // start new toggle
		// clean up previous thread
		std::erase_if(m_TempUnitThread, [this](const auto& thrd) {
			const auto& [i, t] = thrd;
			return !m_AltUnitTempo.contains(i);
			});
		m_AltUnitTempo.insert(systemid);
		uslock.unlock();
		auto jt = std::make_shared<std::jthread>([this, systemid, duration](std::stop_token stoken) {
			std::mutex t_mutex;
			std::unique_lock t_lock(t_mutex);
			std::condition_variable_any().wait_for(t_lock, stoken, std::chrono::seconds(duration), [] {
				return false;
				});
			if (!stoken.stop_requested()) {
				std::unique_lock uclock1(utemp_Mutex);
				m_AltUnitTempo.erase(systemid);
			}
			});
		m_TempUnitThread.insert({ systemid,jt });
		utlock.unlock();
	}
	return true;
}

bool TrackRecorder::IsForceKnot(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	std::shared_lock slock(speed_Mutex);
	if (RadarTarget.IsValid() && m_SpeedUnitSysID.contains(RadarTarget.GetSystemID())) {
		return !m_DefaultKnot;
	}
	return m_DefaultKnot;
}

void TrackRecorder::SetSpeedUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const bool& knot)
{
	if (!RadarTarget.IsValid()) return;
	std::unique_lock slock(speed_Mutex);
	std::string systemID = RadarTarget.GetSystemID();
	if (knot == m_DefaultKnot) {
		m_SpeedUnitSysID.erase(systemID);
	}
	else {
		m_SpeedUnitSysID.insert(systemID);
	}
}

void TrackRecorder::SetSpeedUnit(const bool& knot)
{
	std::unique_lock slock(speed_Mutex);
	m_SpeedUnitSysID.clear();
	m_DefaultKnot = knot;
}

bool TrackRecorder::IsDifferentUnitPUS(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	// PUS = Position Unit Setting
	if (!RadarTarget.IsValid()) return false;
	std::shared_lock aclock(ucals_Mutex), ailock(usysi_Mutex), slock(speed_Mutex);
	std::string callsign = RadarTarget.GetCorrelatedFlightPlan().IsValid() ? RadarTarget.GetCorrelatedFlightPlan().GetCallsign() : "";
	std::string systemID = RadarTarget.GetSystemID();
	bool revc = callsign.size() && m_AltUnitCallsign.contains(callsign);
	bool revi = m_AltUnitSysID.contains(systemID);
	bool revs = m_SpeedUnitSysID.contains(systemID);
	return revc || revi || revs;
}

bool TrackRecorder::IsDifferentUnitRFL(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid()) return false;
	int _meter;
	bool feetRequested = !MetricAlt::RflFeettoM(FlightPlan.GetFinalAltitude(), _meter);
	return m_DefaultFeet != feetRequested;
}

bool TrackRecorder::IsDisplayVerticalSpeed(const std::string& systemID)
{
	std::shared_lock v_lock(vsdsp_Mutex);
	if (systemID.size() && m_DisplayVS.contains(systemID)) {
		return !m_GlobalVS;
	}
	return m_GlobalVS;
}

void TrackRecorder::ToggleVerticalSpeed(const std::string& systemID)
{
	std::unique_lock v_lock(vsdsp_Mutex);
	if (!m_DisplayVS.erase(systemID)) {
		m_DisplayVS.insert(systemID);
	}
}

void TrackRecorder::ToggleVerticalSpeed(const bool& display)
{
	std::unique_lock v_lock(vsdsp_Mutex);
	m_DisplayVS.clear();
	m_GlobalVS = display;
}

std::unordered_map<std::string, TrackRecorder::TrackedData>::iterator TrackRecorder::GetTrackedDataBySystemID(const std::string& systemID)
{
	std::shared_lock mlock(tr_Mutex);
	return std::find_if(m_TrackedMap.begin(), m_TrackedMap.end(), [&systemID](const auto& d) { return d.second.m_SystemID == systemID; });
}

void TrackRecorder::RefreshSimilarCallsign(void)
{
	sc_NeedRefresh = true;
	sc_CondVar.notify_all();
}

void TrackRecorder::SimilarCallsignThread(std::stop_token stoken)
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
