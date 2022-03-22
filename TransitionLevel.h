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

class TransitionLevel
{
public:
	TransitionLevel(EuroScopePlugIn::CPlugIn* plugin);
	~TransitionLevel(void);

	void LoadCSV(string filename);
	int GetTransitionLevel(EuroScopePlugIn::CFlightPlan FlightPlan);
	int GetTransitionLevel(EuroScopePlugIn::CRadarTarget RadarTarget);

private:
	EuroScopePlugIn::CPlugIn* m_PluginPtr;
	unordered_map<string, EuroScopePlugIn::CPosition> m_AirportPosMap;
	unordered_map<string, int> m_TransLevelMap;
};
