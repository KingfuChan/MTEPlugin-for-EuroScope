//TrackedRecorder.h

#pragma once

#include "pch.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "SimilarCallsign.h"

using namespace std;


class TrackedRecorder
{

public:
	TrackedRecorder(void);
	~TrackedRecorder(void);
	void UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, bool online = true);
	bool IsCommEstablished(string callsign);
	void SetCommEstablished(string callsign);
	bool IsCFLConfirmed(string callsign);
	void SetCFLConfirmed(string callsign, bool confirmed = true);
	bool IsForceFeet(string callsign);
	void SetAltitudeUnit(string callsign, bool feet);
	bool IsSquawkDUPE(string callsign);
	bool IsReconnected(string callsign);
	bool IsSimilarCallsign(string callsign);
	unordered_set<string> GetSimilarCallsigns(string callsign);
	void SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan);

private:
	struct TRData {
		bool m_Reconnected;
		bool m_CommEstbed;
		bool m_CFLConfirmed;
		bool m_ForceFeet;
		int m_CallsignType; // -1: Text, 0: ENG, 1: CHN
		// assigned data
		char m_CommType;
		int m_Heading;
		int m_ClearedAlt;
		int m_FinalAlt;
		int m_Speed;
		int m_Rate;
		string m_DCTName; // should only be valid if no heading
		string m_Squawk;
		string m_ScratchPad;
	};

	unordered_map<string, TRData> m_TrackedMap;
	unordered_map<string, unordered_set<string>> m_SCSetMap;

	void RefreshSimilarCallsign(void);

};
