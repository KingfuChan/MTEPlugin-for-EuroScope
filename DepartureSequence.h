// DepartureSequence.cpp

#pragma once

#include "pch.h"

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
		std::string callsign;
		bool active;
	}SeqData;
	typedef std::list<SeqData> seq_list;
	typedef struct {
		std::string callsign;
		std::string state;
		int sequence; // will be -1 if inactive, 0 if cleared or not exist
		seq_list::iterator iterator;
	}FlightSeqData;
	std::map<std::string, seq_list> m_SequenceListMap; // {state, SeqData list}
	FlightSeqData FindData(EuroScopePlugIn::CFlightPlan FlightPlan);
	std::string GetFPGroundState(EuroScopePlugIn::CFlightPlan FlightPlan);
};
