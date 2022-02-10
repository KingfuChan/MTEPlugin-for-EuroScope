// DepartureSequence.cpp

#pragma once

#include "pch.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <list>
#include <map>

using namespace std;

struct SeqData {
	string callsign;
	bool active;
};

typedef list<SeqData> seq_list;

struct FlightSeqData {
	string callsign;
	string state;
	int sequence; // will be -1 if inactive, 0 if cleared or not exist
	seq_list::iterator iterator;
};

class DepartureSequence
{
public:
	DepartureSequence(void);
	~DepartureSequence(void);
	void AddFlight(EuroScopePlugIn::CFlightPlan FlightPlan);
	int GetSequence(EuroScopePlugIn::CFlightPlan FlightPlan);
	void EditSequence(EuroScopePlugIn::CFlightPlan FlightPlan, int seq);

private:
	map<string, seq_list> m_SequenceListMap; // {state, SeqData list}
	FlightSeqData FindData(EuroScopePlugIn::CFlightPlan FlightPlan);
	string GetFPGroundState(EuroScopePlugIn::CFlightPlan FlightPlan);
};
