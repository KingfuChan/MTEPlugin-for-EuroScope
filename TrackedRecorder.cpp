//TrackedRecorder.cpp

#include "pch.h"
#include "TrackedRecorder.h"

TrackedRecorder::TrackedRecorder(EuroScopePlugIn::CPlugIn* plugin)
{
	m_PluginPtr = plugin;
	m_DefaultFeet = false;
}

TrackedRecorder::~TrackedRecorder(void)
{
}

void TrackedRecorder::UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, const bool online)
{
	// Pass online=false to deactivate when disconnecting.
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
			if (rfsc)
				RefreshSimilarCallsign();
		}
		else if (!r->second.m_Offline) {
			// recorded, not tracking, not offline. remove
			m_TrackedMap.erase(r);
			RefreshSimilarCallsign();
		}
	}
	else if (FlightPlan.GetTrackingControllerIsMe()) {
		// not recorded but tracking. add
		TrackedData trd{ FlightPlan.GetCorrelatedRadarTarget().GetSystemID(), AssignedData(FlightPlan), m_DefaultFeet };
		m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
		RefreshSimilarCallsign();
	}
}

void TrackedRecorder::UpdateFlight(EuroScopePlugIn::CRadarTarget RadarTarget)
{
	// Only useful if correlated.
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (!FlightPlan.IsValid())
		return;
	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r != m_TrackedMap.end()) {
		if (FlightPlan.GetTrackingControllerIsMe()) {
			// recorded, tracking. update
			r->second.m_SystemID = RadarTarget.GetSystemID();
		}
		else if (!r->second.m_Offline) {
			// recorded, not tracking, not offline. remove
			m_TrackedMap.erase(r);
			RefreshSimilarCallsign();
		}
	}
	else if (FlightPlan.GetTrackingControllerIsMe()) {
		// not recorded, tracking. add
		TrackedData trd{ RadarTarget.GetSystemID(), AssignedData(FlightPlan), m_DefaultFeet };
		m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
		RefreshSimilarCallsign();
	}
}

bool TrackedRecorder::IsCommEstablished(const std::string& callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CommEstbed;
	else
		return true;
}

void TrackedRecorder::SetCommEstablished(const std::string& callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CommEstbed = true;
}

bool TrackedRecorder::IsCFLConfirmed(const std::string& callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CFLConfirmed;
	else
		return true;
}

void TrackedRecorder::SetCFLConfirmed(const std::string& callsign, const bool confirmed)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CFLConfirmed = confirmed;
}

bool TrackedRecorder::IsForceFeet(const std::string& callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_ForceFeet;
	else
		return m_DefaultFeet;
}

void TrackedRecorder::SetAltitudeUnit(const std::string& callsign, const bool& feet)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_ForceFeet = feet;
}

void TrackedRecorder::ResetAltitudeUnit(const bool& feet)
{
	m_DefaultFeet = feet;
	for (auto& [c, d] : m_TrackedMap) {
		d.m_ForceFeet = feet;
	}
}

bool TrackedRecorder::IsSquawkDUPE(const std::string& callsign)
{
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
		return r != m_TrackedMap.end() ? !r->second.m_Offline : true;
	}
}

bool TrackedRecorder::IsSimilarCallsign(const std::string& callsign)
{
	std::lock_guard<std::mutex> lock(similar_callsign_lock);
	return m_SCSetMap.find(callsign) != m_SCSetMap.end();
}

std::unordered_set<std::string> TrackedRecorder::GetSimilarCallsigns(const std::string& callsign)
{
	std::lock_guard<std::mutex> lock(similar_callsign_lock);
	auto f = m_SCSetMap.find(callsign);
	if (f != m_SCSetMap.end())
		return f->second;
	else
		return std::unordered_set<std::string>();
}

bool TrackedRecorder::SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// returns true if success
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
	trd->second.m_Offline = true; // prevent following assigns removing
	AssignedData tad(trd->second.m_AssignedData);
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
	trd->second.m_Offline = false;
	RefreshSimilarCallsign();
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
		if (trp != m_TrackedMap.end()) {
			auto corrFP = m_PluginPtr->FlightPlanSelect(trp->first.c_str());
			if (corrFP.IsValid()) {
				RadarTarget.CorrelateWithFlightPlan(corrFP); // set data at next radar refresh
			}
		}
	}
	return false;
}

std::unordered_map<std::string, TrackedRecorder::TrackedData>::iterator TrackedRecorder::GetTrackedDataBySystemID(const std::string& systemID)
{
	return std::find_if(m_TrackedMap.begin(), m_TrackedMap.end(), [&systemID](const auto& d) { return d.second.m_SystemID == systemID; });
}

void TrackedRecorder::RefreshSimilarCallsign(void)
{
	std::thread threadRefresh([&] {
		std::lock_guard<std::mutex> lock(similar_callsign_lock);
		m_SCSetMap.clear();
		std::unordered_set<std::string> setENG, setCHN;
		for (const auto& [c, d] : m_TrackedMap) {
			if (d.m_Offline || d.m_AssignedData.m_CommType == 'T') continue;
			std::string cal = c.substr(0, 3);
			if (m_CHNCallsign.find(cal) != m_CHNCallsign.end()) {
				std::string scratch = d.m_AssignedData.m_ScratchPad;
				transform(scratch.begin(), scratch.end(), scratch.begin(), ::toupper);
				size_t epos = scratch.find("EN");
				if (epos != std::string::npos && epos > 0) {
					if (std::string("*/\\.").find(scratch[epos - 1]) != std::string::npos) {
						setCHN.insert(c);
						continue;
					}
				}
			}
			setENG.insert(c);
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
		});
	threadRefresh.detach();
}
