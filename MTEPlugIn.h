// MTEPlugin.h

#pragma once

#include "pch.h"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <regex>
#include <atomic>
#include "MTEPScreen.h"
#include "MetricAlt.h"
#include "ReCat.h"
#include "SimilarCallsign.h"
#include "RouteChecker.h"
#include "DepartureSequence.h"
#include "TrackedRecorder.h"
#include "TransitionLevel.h"

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
	virtual void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	virtual void OnRadarTargetPositionUpdate(CRadarTarget RadarTarget);
	virtual CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);
	virtual bool OnCompileCommand(const char* sCommandLine);

private:
	stack<CMTEPScreen*> m_ScreenStack; // for StartTagFunction
	RouteChecker* m_RouteChecker;
	DepartureSequence* m_DepartureSequence;
	TrackedRecorder* m_TrackedRecorder;
	TransitionLevel* m_TransitionLevel;
	bool m_CustomCursor;
	int m_AutoRetrack; // 0: off (default); 1: silent; 2: notified.
	string m_CustomNumMap; // 0-9
	int m_AmendCFL; // 0: off; 1: MTEP (default); 2: all.

	void CallNativeItemFunction(const char* sCallsign, int FunctionId, POINT Pt, RECT Area);
	void SetCustomCursor(void);
	void CancelCustomCursor(void);
	void LoadRouteChecker(string filename);
	void UnloadRouteChecker(void);
	void DeleteDepartureSequence(void);
	void ResetTrackedRecorder(void);
	bool LoadTransitionLevel(string filename);
	bool LoadMetricAltitude(string filename);
	string DisplayRouteMessage(string departure, string arrival);
};
