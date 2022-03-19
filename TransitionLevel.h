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
	TransitionLevel(string filename);
	~TransitionLevel(void);

	int GetTransitionLevel(EuroScopePlugIn::CFlightPlan);
};

