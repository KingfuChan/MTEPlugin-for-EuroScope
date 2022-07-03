// TransitionLevel.h

#pragma once

#include "pch.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <fstream>
#include <string>
#include <sstream>
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
	int GetTransitionLevel(EuroScopePlugIn::CFlightPlan FlightPlan);
	int GetTransitionLevel(EuroScopePlugIn::CRadarTarget RadarTarget);
	int GetRadarDisplayAltitude(EuroScopePlugIn::CRadarTarget RadarTarget, int& reference);

private:
	struct AirportData {
		int trans_level;
		int elevation;
		int QFE_range;
		bool in_sector;
		EuroScopePlugIn::CPosition position;
	};

	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	unordered_map<string, AirportData> m_AirportMap;
	unordered_map<string, AirportData>::iterator GetTargetAirport(EuroScopePlugIn::CFlightPlan FlightPlan);
	unordered_map<string, AirportData>::iterator GetTargetAirport(EuroScopePlugIn::CRadarTarget RadarTarget);
};
