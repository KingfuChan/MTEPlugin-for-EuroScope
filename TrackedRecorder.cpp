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

void TrackedRecorder::UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, bool online)
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

bool TrackedRecorder::IsCommEstablished(string callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CommEstbed;
	else
		return true;
}

void TrackedRecorder::SetCommEstablished(string callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CommEstbed = true;
}

bool TrackedRecorder::IsCFLConfirmed(string callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_CFLConfirmed;
	else
		return true;
}

void TrackedRecorder::SetCFLConfirmed(string callsign, bool confirmed)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_CFLConfirmed = confirmed;
}

bool TrackedRecorder::IsForceFeet(string callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_ForceFeet;
	else
		return m_DefaultFeet;
}

void TrackedRecorder::SetAltitudeUnit(string callsign, bool feet)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_ForceFeet = feet;
}

void TrackedRecorder::ResetAltitudeUnit(bool feet)
{
	m_DefaultFeet = feet;
	for (auto& r : m_TrackedMap) {
		r.second.m_ForceFeet = feet;
	}
}

bool TrackedRecorder::IsSquawkDUPE(string callsign)
{
	auto r1 = m_TrackedMap.find(callsign);
	if (r1 == m_TrackedMap.end())
		return false;
	for (auto& r2 : m_TrackedMap) {
		if (!r2.second.m_Offline &&
			r1->first != r2.first &&
			r1->second.m_AssignedData.m_Squawk == r2.second.m_AssignedData.m_Squawk)
			return true;
	}
	return false;
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

bool TrackedRecorder::IsSimilarCallsign(string callsign)
{
	return m_SCSetMap.find(callsign) != m_SCSetMap.end();
}

unordered_set<string> TrackedRecorder::GetSimilarCallsigns(string callsign)
{
	auto f = m_SCSetMap.find(callsign);
	if (f != m_SCSetMap.end())
		return f->second;
	else
		return unordered_set<string>();
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

unordered_map<string, TrackedRecorder::TrackedData>::iterator TrackedRecorder::GetTrackedDataBySystemID(string systemID)
{
	for (auto trd = m_TrackedMap.begin(); trd != m_TrackedMap.end(); trd++) {
		if (trd->second.m_SystemID == systemID)
			return trd;
	}
	return m_TrackedMap.end();
}

void TrackedRecorder::RefreshSimilarCallsign(void)
{
	m_SCSetMap.clear();
	unordered_set<string> setENG, setCHN;
	for (auto& r : m_TrackedMap) {
		if (r.second.m_Offline || r.second.m_AssignedData.m_CommType == 'T') continue; // offline or text
		string cal = r.first.substr(0, 3);
		if (m_CHNCallsign.find(cal) != m_CHNCallsign.end() && r.second.m_AssignedData.m_ScratchPad.find("*EN") == string::npos)
			setCHN.insert(r.first);
		else
			setENG.insert(r.first);
	}
	for (auto& cset : { setENG,setCHN }) {
		for (auto& c1 : cset) {
			for (auto& c2 : cset) {
				if (c1 != c2 && CompareCallsign(c1, c2)) {
					m_SCSetMap[c1].insert(c2);
				}
			}
		}
	}
}
