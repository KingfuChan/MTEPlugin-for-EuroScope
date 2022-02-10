// SimilarCallsign.h

#pragma once

#include "pch.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <unordered_set>
#include <list>

using namespace std;

bool IsChineseCallsign(EuroScopePlugIn::CFlightPlan FlightPlan);
bool CompareCallsign(string callsign1, string callsign2);
