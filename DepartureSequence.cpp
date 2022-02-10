// DepartureSequence.cpp

#include "pch.h"
#include "DepartureSequence.h"

DepartureSequence::DepartureSequence(void)
{
}

DepartureSequence::~DepartureSequence(void)
{
}

void DepartureSequence::AddFlight(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	FlightSeqData fsd = FindData(FlightPlan);
	if (fsd.sequence < 0) {
		fsd.iterator->active = fsd.state == GetFPGroundState(FlightPlan);
	}
	else if (fsd.sequence == 0) {
		string state = GetFPGroundState(FlightPlan);
		if (state == "NSTS" && FlightPlan.GetClearenceFlag()) return;
		string callsign = FlightPlan.GetCallsign();
		m_SequenceListMap[state].push_back(SeqData{ callsign, true });
	}
}

int DepartureSequence::GetSequence(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	FlightSeqData fsd = FindData(FlightPlan);
	if (fsd.state == "NSTS" && FlightPlan.GetClearenceFlag()) {
		// delete cleared flights on NSTS
		m_SequenceListMap[fsd.state].erase(fsd.iterator);
		return 0;
	}
	if (fsd.state == "NONE" || fsd.state == GetFPGroundState(FlightPlan))
		return fsd.sequence;
	else // reconnected and state not restored
		return -1;
}

void DepartureSequence::EditSequence(EuroScopePlugIn::CFlightPlan FlightPlan, int seq)
{
	// if seq is -1, will deactivate flight; if seq is 0, will remove from list
	FlightSeqData fsd = FindData(FlightPlan);
	if (fsd.sequence == 0) return;
	else if (fsd.sequence < 0) {
		// activate reconnected flight
		fsd.iterator->active = GetFPGroundState(FlightPlan) == fsd.state;
		return;
	}
	if (seq < 0)
		fsd.iterator->active = false;
	else if (seq == 0)
		m_SequenceListMap[fsd.state].erase(fsd.iterator);
	else {
		m_SequenceListMap[fsd.state].erase(fsd.iterator);
		auto itm = m_SequenceListMap[fsd.state].begin();
		for (; itm != m_SequenceListMap[fsd.state].end() && seq > 1; seq -= itm->active, itm++);
		m_SequenceListMap[fsd.state].insert(itm, SeqData{ fsd.callsign, true });
	}
}

FlightSeqData DepartureSequence::FindData(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// sequence == -1: inactive, 0: not found
	for (auto& itm : m_SequenceListMap) {
		seq_list::iterator itl;
		int seq = 0;
		for (itl = itm.second.begin(); itl != itm.second.end(); itl++) {
			seq += itl->active;
			if (itl->callsign == FlightPlan.GetCallsign()) {
				seq = itl->active ? seq : -1;
				return FlightSeqData{ itl->callsign, itm.first, seq, itl };
			}
		}
	}
	return FlightSeqData{ "", "NONE", 0, seq_list::iterator() };
}

string DepartureSequence::GetFPGroundState(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	string s = FlightPlan.GetGroundState();
	s = s.length() ? s : "NSTS";
	return s;
}
