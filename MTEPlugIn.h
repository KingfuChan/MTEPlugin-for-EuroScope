// MTEPlugin.h

#pragma once

#include "pch.h"
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
#include "TrackedRecorder.h"

using namespace std;
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
	virtual void OnRadarTargetPositionUpdate(CRadarTarget RadarTarget);
	virtual bool OnCompileCommand(const char* sCommandLine);

private:

	RouteChecker* m_RouteChecker;
	DepartureSequence* m_DepartureSequence;
	TrackedRecorder* m_TrackedRecorder;
	bool m_CustomCursor;
	int m_AutoRetrack;

	int GetRadarDisplayAltitude(CRadarTarget RadarTarget);
	void SetCustomCursor(void);
	void CancelCustomCursor(void);
	void LoadRouteChecker(string filename);
	void UnloadRouteChecker(void);
	void DeleteDepartureSequence(void);
	void ResetTrackedRecorder(void);
	string DisplayRouteMessage(string departure, string arrival);
};
