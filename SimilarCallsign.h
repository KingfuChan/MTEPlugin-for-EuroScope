// CHNCallsign.hpp

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <set>
#include <map>
#include <list>

typedef std::map<CString, bool> CSMark; // Callsign Marker
typedef std::list<char> CharList;

// public functions
bool IsCallsignChinese(EuroScopePlugIn::CFlightPlan FlightPlan);
bool IsCallsignSimilar(CString callsign1, CString callsign2);
CSMark ParseSimilarCallsign(CSMark MarkerMap);

// private functions
CharList ExtractNumfromCallsign(const CString callsign);
bool CompareCallsignNum(CharList cs1, CharList cs2);