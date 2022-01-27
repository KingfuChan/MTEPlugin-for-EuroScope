// MTEPlugin.h

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include "MetricAlt.h"
#include "ReCat.h"
#include "SimilarCallsign.h"
#include "RouteChecker.h"
#include "DepartureSequence.h"

using namespace EuroScopePlugIn;

class CMTEPlugIn :
	public CPlugIn
{
public:
	CMTEPlugIn(void);
	~CMTEPlugIn(void);

	virtual void OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagMemData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	virtual void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);
	virtual void OnFlightPlanDisconnect(CFlightPlan FlightPlan);
	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char* sCommandLine);

private:
	unordered_set<string> m_SimilarCallsignSet; // similar callsign set
	unordered_set<string> m_SquawkDupeSet; // duplicated squawk set
	unordered_map<string, bool> m_ComEstbMap; // true means communication established
	unordered_map<string, bool> m_CFLConfirmMap; // true means CFL needs confirm
	RouteChecker* m_RouteChecker;
	DepartureSequence* m_DepartureSequence;

	int GetRadarDisplayAltitude(CRadarTarget RadarTarget);
	void SetCustomCursor(void);
	void CancelCustomCursor(void);
	void LoadRouteChecker(string filename);
	void UnloadRouteChecker(void);
	void DeleteDepartureSequence(void);
	string DisplayRouteMessage(string departure, string arrival);
};
