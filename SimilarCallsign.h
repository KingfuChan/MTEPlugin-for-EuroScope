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

typedef std::map<CString, bool> StrMark;
typedef std::list<char> CharList;

// public functions
bool IsCallsignChinese(EuroScopePlugIn::CFlightPlan FlightPlan);
StrMark ParseSimilarCallsign(StrMark MarkerMap);

// private functions
CharList ExtractNumfromCallsign(const CString callsign);
bool CompareCallsignNum(CharList cs1, CharList cs2);