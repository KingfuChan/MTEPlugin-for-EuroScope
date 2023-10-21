//TrackedRecorder.h

#pragma once

#include "pch.h"
#include "SimilarCallsign.h"

class TrackedRecorder
{
public:
	TrackedRecorder(EuroScopePlugIn::CPlugIn* plugin);
	~TrackedRecorder(void);
	void UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, const bool online = true);
	void UpdateFlight(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool IsCommEstablished(const std::string& callsign);
	void SetCommEstablished(const std::string& callsign);
	bool IsCFLConfirmed(const std::string& callsign);
	void SetCFLConfirmed(const std::string& callsign, const bool confirmed = true);
	bool IsForceFeet(const std::string& callsign);
	void SetAltitudeUnit(const std::string& callsign, const bool& feet);
	void ResetAltitudeUnit(const bool& feet);
	bool IsSquawkDUPE(const std::string& callsign);
	bool IsActive(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool IsActive(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool IsSimilarCallsign(const std::string& callsign);
	std::unordered_set<std::string> GetSimilarCallsigns(const std::string& callsign);
	bool SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget);

private:
	bool m_DefaultFeet;

	typedef struct _AsD {
		// in the order of SDK
		std::string m_Squawk;
		int m_FinalAlt;
		int m_ClearedAlt;
		char m_CommType;
		std::string m_ScratchPad;
		int m_Speed;
		int m_Mach;
		int m_Rate;
		int m_Heading;
		std::string m_DCTName;

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
		std::string m_SystemID;
		bool m_Offline;
		bool m_CommEstbed;
		bool m_CFLConfirmed;
		bool m_ForceFeet;
		AssignedData m_AssignedData;

		_TkD(std::string _sID, AssignedData _asd, bool _fft) :
			m_SystemID(_sID),
			m_Offline(false), m_CommEstbed(false), m_CFLConfirmed(true),
			m_ForceFeet(_fft),
			m_AssignedData(_asd)
		{};
	}TrackedData;

	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	std::unordered_map<std::string, TrackedData> m_TrackedMap; // callsign
	std::unordered_map<std::string, std::unordered_set<std::string>> m_SCSetMap; // callsign
	std::mutex similar_callsign_lock;

	std::unordered_map<std::string, TrackedData>::iterator GetTrackedDataBySystemID(const std::string& systemID);
	void RefreshSimilarCallsign(void);
};
