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
		std::string state = GetFPGroundState(FlightPlan);
		std::string callsign = FlightPlan.GetCallsign();
		m_SequenceListMap[state].push_back(SeqData{ callsign, true });
	}
}

int DepartureSequence::GetSequence(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	FlightSeqData fsd = FindData(FlightPlan);
	if (fsd.state == "NONE" || fsd.state == GetFPGroundState(FlightPlan))
		return fsd.sequence;
	else // reconnected and state not restored
		return -1;
}

void DepartureSequence::EditSequence(EuroScopePlugIn::CFlightPlan FlightPlan, const int& sequence)
{
	// if seq is -1, will deactivate flight; if seq is 0, will remove from list
	FlightSeqData fsd = FindData(FlightPlan);
	if (fsd.sequence == 0) return;
	else if (fsd.sequence < 0) {
		// activate reconnected flight
		fsd.iterator->active = GetFPGroundState(FlightPlan) == fsd.state;
		return;
	}
	int seq = sequence;
	if (seq < 0) {
		fsd.iterator->active = false;
	}
	else {
		auto& in_list = m_SequenceListMap[fsd.state];
		in_list.erase(fsd.iterator);
		if (seq != 0) {
			auto itm = std::find_if(in_list.begin(), in_list.end(), [&seq](const auto& s) {seq -= s.active; return seq < 1; });
			in_list.insert(itm, SeqData{ fsd.callsign, true });
		}
	}
}

DepartureSequence::FlightSeqData DepartureSequence::FindData(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	// sequence == -1: inactive, 0: not found
	for (auto& [state, state_list] : m_SequenceListMap) {
		int seq = 0;
		for (auto itl = state_list.begin(); itl != state_list.end(); itl++) {
			seq += itl->active;
			if (itl->callsign == FlightPlan.GetCallsign()) {
				seq = itl->active ? seq : -1;
				return FlightSeqData{ itl->callsign, state, seq, itl };
			}
		}
	}
	return FlightSeqData{ "", "NONE", 0, seq_list::iterator() };
}

std::string DepartureSequence::GetFPGroundState(EuroScopePlugIn::CFlightPlan FlightPlan)
{
	std::string s = FlightPlan.GetGroundState();
	s = s.length() ? s : FlightPlan.GetClearenceFlag() ? "CLRD" : "NSTS";
	return s;
}
