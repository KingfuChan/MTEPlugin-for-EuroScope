// TransitionLevel.h

#pragma once

#include "pch.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
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

	void LoadCSV(string filename);
	int GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference);
	string GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan, int& trans_level, int& elevation);
	bool SetAirportParam(string airport, int trans_level = -1, int isQFE = -1, int range = -1);

private:
	typedef vector<EuroScopePlugIn::CPosition> pos_vec;
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
	unordered_map<string, AirportData> m_AirportMap;
	typedef unordered_map<string, AirportData>::iterator apmap_iter;
	int m_DefaultLevel, m_MaxLevel;
	apmap_iter GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan);
	apmap_iter GetTargetAirport(EuroScopePlugIn::CRadarTarget RadarTarget);
	apmap_iter GetTargetAirport(EuroScopePlugIn::CPosition Position);
	bool IsinQNHBoundary(EuroScopePlugIn::CPosition pos, apmap_iter airport_iter);
};
