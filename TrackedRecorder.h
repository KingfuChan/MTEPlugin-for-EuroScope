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
	bool IsForceFeet(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget);
	void SetAltitudeUnit(const std::string& callsign, const bool& feet);
	void ResetAltitudeUnit(const bool& feet);
	bool IsSquawkDUPE(const std::string& callsign);
	bool IsActive(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool IsActive(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool IsSimilarCallsign(const std::string& callsign);
	std::unordered_set<std::string> GetSimilarCallsigns(const std::string& callsign);
	bool SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool ToggleAltitudeUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const int duration = 5);

private:
	bool m_DefaultFeet = false;
	bool m_SuppressUpdate = false;

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
		std::array<std::string, 9> m_StripAnno;

		_AsD(EuroScopePlugIn::CFlightPlan _fp) {
			auto a = _fp.GetControllerAssignedData();
			m_Squawk = a.GetSquawk();
			m_FinalAlt = a.GetFinalAltitude();
			m_ClearedAlt = a.GetClearedAltitude();
			m_CommType = a.GetCommunicationType();
			m_ScratchPad = a.GetScratchPadString();
			m_Speed = a.GetAssignedSpeed();
			m_Mach = a.GetAssignedMach();
			m_Rate = a.GetAssignedRate();
			m_Heading = a.GetAssignedHeading();
			m_DCTName = a.GetDirectToPointName();
			for (size_t i = 0; i < 9; i++) {
				m_StripAnno[i] = a.GetFlightStripAnnotation(i);
			}
		};
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
	// track map
	std::unordered_map<std::string, TrackedData> m_TrackedMap; // callsign -> TrackedData
	std::shared_mutex tr_Mutex;

	// for similar callsign
	std::unordered_map<std::string, std::unordered_set<std::string>> m_SCSetMap; // callsign -> set<callsign>
	std::shared_mutex sc_Mutex;
	// callsign threading
	std::jthread sc_Thread;
	std::condition_variable_any sc_CondVar;
	bool sc_NeedRefresh = false;

	// toggle altitude unit
	std::unordered_map<std::string, std::shared_ptr<std::jthread>> m_TempUnitThread; // systemID -> thread
	std::set<std::string> m_TempUnitSysID; // only stores those different from default
	std::shared_mutex uthrd_Mutex, usysi_Mutex;

	std::unordered_map<std::string, TrackedData>::iterator GetTrackedDataBySystemID(const std::string& systemID);
	void RefreshSimilarCallsign(void);
	void SimilarCallsignThread(std::stop_token stoken);
};
