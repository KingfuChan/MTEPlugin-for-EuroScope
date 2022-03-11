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
	TrackedRecorder(EuroScopePlugIn::CPlugIn* plugin);
	~TrackedRecorder(void);
	void UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, bool online = true);
	void UpdateFlight(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool IsCommEstablished(string callsign);
	void SetCommEstablished(string callsign);
	bool IsCFLConfirmed(string callsign);
	void SetCFLConfirmed(string callsign, bool confirmed = true);
	bool IsForceFeet(string callsign);
	void SetAltitudeUnit(string callsign, bool feet);
	bool IsSquawkDUPE(string callsign);
	bool IsActive(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool IsActive(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool IsSimilarCallsign(string callsign);
	unordered_set<string> GetSimilarCallsigns(string callsign);
	void SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan);
	void SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget);

private:
	struct AssignedData {
		// in the order of SDK
		string m_Squawk;
		int m_FinalAlt;
		int m_ClearedAlt;
		char m_CommType;
		string m_ScratchPad;
		int m_Speed;
		int m_Mach;
		int m_Rate;
		int m_Heading;
		string m_DCTName;
	};
	struct TrackedData {
		string m_SystemID;
		bool m_Offline;
		bool m_CommEstbed;
		bool m_CFLConfirmed;
		bool m_ForceFeet;
		AssignedData m_AssignedData;
	};

	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	unordered_map<string, TrackedData> m_TrackedMap; // callsign
	unordered_map<string, unordered_set<string>> m_SCSetMap; // callsign

	AssignedData ExtractAssignedData(EuroScopePlugIn::CFlightPlan FlightPlan);
	unordered_map<string, TrackedData>::iterator GetTrackedDataBySystemID(string systemID);
	void RefreshSimilarCallsign(void);

};
