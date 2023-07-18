//TrackedRecorder.h

#pragma once

#include "pch.h"
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
	bool SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget);

private:
	typedef struct _AsD {
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

		_AsD(EuroScopePlugIn::CFlightPlan _fp) :
			m_Squawk(_fp.GetControllerAssignedData().GetSquawk()),
			m_FinalAlt(_fp.GetFinalAltitude()),
			m_ClearedAlt(_fp.GetControllerAssignedData().GetClearedAltitude()),
			m_CommType(_fp.IsTextCommunication() ? 'T' : _fp.GetControllerAssignedData().GetCommunicationType()),
			m_ScratchPad(_fp.GetControllerAssignedData().GetScratchPadString()),
			m_Speed(_fp.GetControllerAssignedData().GetAssignedSpeed()),
			m_Mach(_fp.GetControllerAssignedData().GetAssignedMach()),
			m_Rate(_fp.GetControllerAssignedData().GetAssignedRate()),
			m_Heading(_fp.GetControllerAssignedData().GetAssignedHeading()),
			m_DCTName(_fp.GetControllerAssignedData().GetDirectToPointName())
		{};
	}AssignedData;
	typedef struct _TkD {
		string m_SystemID;
		bool m_Offline;
		bool m_CommEstbed;
		bool m_CFLConfirmed;
		bool m_ForceFeet;
		AssignedData m_AssignedData;

		_TkD(string _sID, AssignedData _asd) :
			m_SystemID(_sID),
			m_Offline(false), m_CommEstbed(false), m_CFLConfirmed(true), m_ForceFeet(false),
			m_AssignedData(_asd)
		{};
	}TrackedData;

	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	unordered_map<string, TrackedData> m_TrackedMap; // callsign
	unordered_map<string, unordered_set<string>> m_SCSetMap; // callsign

	unordered_map<string, TrackedData>::iterator GetTrackedDataBySystemID(string systemID);
	void RefreshSimilarCallsign(void);
};
