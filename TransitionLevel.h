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

	void LoadCSV(std::string filename);
	int GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference);
	std::string GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan, int& trans_level, int& elevation);
	bool SetAirportParam(std::string airport, int trans_level = -1, int isQFE = -1, int range = -1);

private:
	typedef std::vector<EuroScopePlugIn::CPosition> pos_vec;
	typedef struct {
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
	typedef std::unordered_map<std::string, AirportData>::iterator apmap_iter;
	std::unordered_map<std::string, std::string> m_Cache; // systemID/callsign -> target airport
	int m_DefaultLevel, m_MaxLevel;

	apmap_iter GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan);
	apmap_iter GetTargetAirport(EuroScopePlugIn::CRadarTarget RadarTarget);
	apmap_iter GetTargetAirport(EuroScopePlugIn::CPosition Position, std::string CacheID);
	bool IsinQNHBoundary(EuroScopePlugIn::CPosition pos, apmap_iter airport_iter);
};
