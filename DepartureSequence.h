// DepartureSequence.cpp

#pragma once

#include "pch.h"
#include <string>
#include <list>
#include <map>

using namespace std;



class DepartureSequence
{
public:
	DepartureSequence(void);
	~DepartureSequence(void);
	void AddFlight(EuroScopePlugIn::CFlightPlan FlightPlan);
	int GetSequence(EuroScopePlugIn::CFlightPlan FlightPlan);
	void EditSequence(EuroScopePlugIn::CFlightPlan FlightPlan, int seq);

private:
	typedef struct {
		string callsign;
		bool active;
	}SeqData;
	typedef list<SeqData> seq_list;
	typedef struct {
		string callsign;
		string state;
		int sequence; // will be -1 if inactive, 0 if cleared or not exist
		seq_list::iterator iterator;
	}FlightSeqData;
	map<string, seq_list> m_SequenceListMap; // {state, SeqData list}
	FlightSeqData FindData(EuroScopePlugIn::CFlightPlan FlightPlan);
	string GetFPGroundState(EuroScopePlugIn::CFlightPlan FlightPlan);
};
