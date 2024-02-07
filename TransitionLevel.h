// TransitionLevel.h

#pragma once

#include "pch.h"
#include "MetricAlt.h"

namespace AltitudeReference {
	const int ALT_REF_QNE = 0;
	const int ALT_REF_QNH = 1;
	const int ALT_REF_QFE = 2;
}

class TransitionLevel
{
public:
	TransitionLevel(EuroScopePlugIn::CPlugIn* plugin);
	~TransitionLevel(void);

	void LoadCSV(const std::string& filename);
	void UpdateRadarPosition(EuroScopePlugIn::CRadarTarget RadarTarget);
	int GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference);
	std::string GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan, int& trans_level, int& elevation);
	bool SetAirportParam(const std::string& airport, const int trans_level = -1, const int isQFE = -1, const int range = -1);

private:
	typedef std::vector<EuroScopePlugIn::CPosition> pos_vec;
	typedef struct _ApD {
		int trans_level;
		int elevation;
		bool is_QFE;
		int range;
		pos_vec boundary;
		bool in_sector;
		EuroScopePlugIn::CPosition position;
	}AirportData;

	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	std::unordered_map<std::string, AirportData> m_AirportMap;
	std::unordered_map<std::string, std::string> m_RadarCache; // systemID -> target airport
	std::shared_mutex cache_mutex;
	std::shared_mutex data_mutex;
	int m_DefaultLevel; // logic: only used when m_AirportMap not empty, and defined by csv, and outside of all boundaries. 
	// If m_AirportMap is empty, use m_PluginPtr->GetTransitionAltitude() for all.
	// If not defined, a/c ouside of all boundaries will use 0 as trans_level.
	int m_MaxLevel;

	// for threading queue control
	typedef struct _QuD {
		std::string system_id;
		EuroScopePlugIn::CPosition position;
		std::string concerned_airport;
	}QueueData;
	std::queue<QueueData> m_UpdateQueue; // systemID -> position
	std::mutex queue_mutex;
	std::jthread q_Thread;
	std::condition_variable_any q_CondVar;
	void UpdateQueueThread(std::stop_token stoken);

	bool IsinQNHBoundary(const EuroScopePlugIn::CPosition pos, const AirportData airport_iter);
};
