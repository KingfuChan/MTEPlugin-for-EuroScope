//TrackedRecorder.cpp

#include "pch.h"
#include "TrackedRecorder.h"

TrackedRecorder::TrackedRecorder(void)
{
}

TrackedRecorder::~TrackedRecorder(void)
{
}

void TrackedRecorder::UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, bool online)
{
	// pass online=false to deactivate

	TRData trd{};
	trd.m_Reconnected = false;
	trd.m_CommEstbed = false;
	trd.m_CFLConfirmed = true;
	trd.m_ForceFeet = false;
	trd.m_CallsignType = FlightPlan.IsTextCommunication() ? -1 : IsChineseCallsign(FlightPlan);

	auto asd = FlightPlan.GetControllerAssignedData();
	trd.m_CommType = asd.GetCommunicationType();
	trd.m_Heading = asd.GetAssignedHeading();
	trd.m_ClearedAlt = asd.GetClearedAltitude();
	trd.m_FinalAlt = asd.GetFinalAltitude();
	trd.m_Speed = asd.GetAssignedSpeed() + asd.GetAssignedMach(); // speed:(100,inf), mach:(0:100]
	trd.m_Rate = asd.GetAssignedRate();
	trd.m_DCTName = asd.GetDirectToPointName();
	trd.m_Squawk = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetSquawk();
	trd.m_ScratchPad = asd.GetScratchPadString();

	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r == m_TrackedMap.end()) {
		if (FlightPlan.GetTrackingControllerIsMe()) {
			// tracking but not recorded
			m_TrackedMap.insert({ FlightPlan.GetCallsign(), trd });
			RefreshSimilarCallsign();
		}
	}
	else if (!r->second.m_Reconnected) {
		if (FlightPlan.GetTrackingControllerIsMe()) {
			// recorded, tracking, not reconnected
			bool rfsh = trd.m_CallsignType != r->second.m_CallsignType;
			trd.m_Reconnected = !online;
			trd.m_CommEstbed = r->second.m_CommEstbed;
			trd.m_CFLConfirmed = r->second.m_CFLConfirmed;
			trd.m_ForceFeet = r->second.m_ForceFeet;
			r->second = trd;
			if (rfsh)
				RefreshSimilarCallsign();
		}
		else {
			// recorded, not tracking, not reconnected
			m_TrackedMap.erase(r);
			RefreshSimilarCallsign();
		}
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
		return false;
}

void TrackedRecorder::SetAltitudeUnit(string callsign, bool feet)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		r->second.m_ForceFeet = feet;
}

bool TrackedRecorder::IsSquawkDUPE(string callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r == m_TrackedMap.end())
		return false;
	for (auto& r1 : m_TrackedMap) {
		if (!r1.second.m_Reconnected && r->first != r1.first && r->second.m_Squawk == r1.second.m_Squawk)
			return true;
	}
	return false;
}

bool TrackedRecorder::IsReconnected(string callsign)
{
	auto r = m_TrackedMap.find(callsign);
	if (r != m_TrackedMap.end())
		return r->second.m_Reconnected;
	else
		return false;
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

void TrackedRecorder::SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	auto r = m_TrackedMap.find(FlightPlan.GetCallsign());
	if (r == m_TrackedMap.end())
		return;
	if (r->second.m_Reconnected) {
		auto asd = FlightPlan.GetControllerAssignedData();
		asd.SetCommunicationType(r->second.m_CommType);
		if (r->second.m_Heading)
			asd.SetAssignedHeading(r->second.m_Heading);
		else if (r->second.m_DCTName.size())
			asd.SetDirectToPointName(r->second.m_DCTName.c_str());
		asd.SetClearedAltitude(r->second.m_ClearedAlt);
		asd.SetFinalAltitude(r->second.m_FinalAlt);
		if (r->second.m_Speed < 100)
			asd.SetAssignedMach(r->second.m_Speed);
		else
			asd.SetAssignedSpeed(r->second.m_Speed);
		asd.SetAssignedRate(r->second.m_Rate);
		asd.SetScratchPadString(r->second.m_ScratchPad.c_str());
		r->second.m_Reconnected = false;
		FlightPlan.StartTracking();
	}
}

void TrackedRecorder::RefreshSimilarCallsign(void)
{
	m_SCSetMap.clear();
	unordered_set<string> setENG, setCHN;
	for (auto& r : m_TrackedMap) {
		if (r.second.m_Reconnected) continue;
		if (r.second.m_CallsignType == 1)
			setCHN.insert(r.first);
		else if (r.second.m_CallsignType == 0)
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
