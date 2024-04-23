//TrackedRecorder.h

#pragma once

#include "pch.h"
#include "SimilarCallsign.h"
#include "MetricAlt.h"

class TrackedRecorder
{
public:
	TrackedRecorder(EuroScopePlugIn::CPlugIn* plugin);
	~TrackedRecorder(void);

	void UpdateFlight(EuroScopePlugIn::CFlightPlan FlightPlan, const bool online = true);
	void UpdateFlight(EuroScopePlugIn::CRadarTarget RadarTarget);

	bool GetCoordinationFlag(const std::string& callsign);
	void SetCoordinationFlag(const std::string& callsign);
	void SetCoordinationFlag(const bool& assumed);

	void HandleNewCfl(EuroScopePlugIn::CFlightPlan FlightPlan, const bool& acknowledge);
	bool IsCflAcknowledged(const std::string& callsign);
	void AcknowledgeCfl(const std::string& callsign, const bool confirmed = true);
	int GetCflElapsedTime(const std::string& callsign);

	bool IsForceFeet(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool IsForceFeet(EuroScopePlugIn::CRadarTarget RadarTarget);
	void SetAltitudeUnit(EuroScopePlugIn::CFlightPlan FlightPlan, const bool& feet);
	void SetAltitudeUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const bool& feet);
	void ResetAltitudeUnit(const bool& feet);
	bool ToggleAltitudeUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const int& duration);

	bool IsForceKnot(EuroScopePlugIn::CRadarTarget RadarTarget);
	void SetSpeedUnit(EuroScopePlugIn::CRadarTarget RadarTarget, const bool& knot);
	void SetSpeedUnit(const bool& knot);

	bool IsDifferentUnitPUS(EuroScopePlugIn::CRadarTarget RadarTarget);
	bool IsDifferentUnitRFL(EuroScopePlugIn::CFlightPlan FlightPlan);

	bool IsSquawkDUPE(EuroScopePlugIn::CRadarTarget RadarTarget);

	bool IsActive(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool IsActive(EuroScopePlugIn::CRadarTarget RadarTarget);

	bool IsSimilarCallsign(const std::string& callsign);
	std::unordered_set<std::string> GetSimilarCallsigns(const std::string& callsign);

	bool SetTrackedData(EuroScopePlugIn::CFlightPlan FlightPlan);
	bool SetTrackedData(EuroScopePlugIn::CRadarTarget RadarTarget);

	bool IsDisplayVerticalSpeed(const std::string& systemID);
	void ToggleVerticalSpeed(const std::string& systemID);
	void ToggleVerticalSpeed(const bool& display);

private:
	bool m_SuppressUpdate = false;
	bool m_CoordFlagAssumed = true;

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
		bool m_CoordFlag;
		AssignedData m_AssignedData;

		_TkD(std::string _sID, AssignedData _asd, bool _cFlag) :
			m_AssignedData(_asd)
		{
			m_SystemID = _sID;
			m_Offline = false;
			m_CoordFlag = _cFlag;
		};
	}TrackedData;

	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	// track map
	std::unordered_map<std::string, TrackedData> m_TrackedMap; // callsign -> TrackedData
	std::shared_mutex tr_Mutex;

	// CFL recorder
	typedef struct _CflD {
		int m_Tracked; // 0: no one tracking, 1: tracked by myself, -1: tracked by others
		bool m_Acknowledged;
		std::chrono::time_point<std::chrono::steady_clock> m_TimeStamp;

		_CflD(int _t, bool _ack) {
			m_Tracked = _t;
			m_Acknowledged = _ack;
			m_TimeStamp = std::chrono::steady_clock::now();
		}
	}CflData;
	std::unordered_map<std::string, CflData> m_CflMap; // callsign -> CflData
	std::shared_mutex cfl_Mutex;

	// for similar callsign
	std::unordered_map<std::string, std::unordered_set<std::string>> m_SCSetMap; // callsign -> set<callsign>
	std::shared_mutex sc_Mutex;
	// callsign threading
	std::jthread sc_Thread;
	std::condition_variable_any sc_CondVar;
	bool sc_NeedRefresh = false;

	// toggle altitude unit
	bool m_DefaultFeet = false; // true=default feet
	std::set<std::string> m_AltUnitSysID; // systemID, only stores those different from default
	std::set<std::string> m_AltUnitCallsign; // callsign, only stores those different from default
	std::set<std::string> m_AltUnitTempo; // systemID, only stores those being temporarily toggled
	std::unordered_map<std::string, std::shared_ptr<std::jthread>> m_TempUnitThread; // systemID -> thread
	std::shared_mutex uthrd_Mutex, usysi_Mutex, ucals_Mutex, utemp_Mutex;

	// speed unit
	bool m_DefaultKnot = false;
	std::set<std::string> m_SpeedUnitSysID; // systemID, only stores those different from default
	std::shared_mutex speed_Mutex;

	// toggle vertical speed display
	bool m_GlobalVS = false; // global vs display, true=display
	std::set<std::string> m_DisplayVS; // systemID, only stores those different from default
	std::shared_mutex vsdsp_Mutex;

	std::unordered_map<std::string, TrackedData>::iterator GetTrackedDataBySystemID(const std::string& systemID);
	void RefreshSimilarCallsign(void);
	void SimilarCallsignThread(std::stop_token stoken);
};
