// CHNCallsign.hpp

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <unordered_set>
#include <list>

using namespace std;

typedef list<char> char_list;

// public functions
bool IsChineseCallsign(EuroScopePlugIn::CFlightPlan FlightPlan);
bool CompareCallsign(string callsign1, string callsign2);
unordered_set<string> ParseSimilarCallsignSet(unordered_set<string> callsigns);

// private functions
char_list ExtractNumfromCallsign(const string callsign);
bool CompareFlightNum(char_list cs1, char_list cs2);