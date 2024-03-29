﻿// MTEPlugin.h

#pragma once

#include "pch.h"
#include "MTEPScreen.h"
#include "MetricAlt.h"
#include "ReCat.h"
#include "SimilarCallsign.h"
#include "RouteChecker.h"
#include "DepartureSequence.h"
#include "TrackedRecorder.h"
#include "TransitionLevel.h"

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
	std::stack<std::shared_ptr<CMTEPScreen>> m_ScreenStack; // for StartTagFunction
	std::unique_ptr<RouteChecker> m_RouteChecker;
	std::unique_ptr<DepartureSequence> m_DepartureSequence;
	std::unique_ptr<TrackedRecorder> m_TrackedRecorder;
	std::unique_ptr<TransitionLevel> m_TransitionLevel;
	bool m_CustomCursor;
	int m_AutoRetrack; // 0: off (default); 1: silent; 2: notified.
	std::string m_CustomNumMap; // 0-9
	std::atomic_int m_AmendCFL; // 0: off; 1: MTEP (default); 2: all.

	void CallItemFunction(const char* sCallsign, const int& FunctionId, const POINT& Pt, const RECT& Area); // overload for ES internal function
	void CallItemFunction(const char* sCallsign, const char* sItemPlugInName, int ItemCode, const char* sItemString, const char* sFunctionPlugInName, int FunctionId, POINT Pt, RECT Area);
	void GetColorDefinition(const char* setting, int* pColorCode, COLORREF* pRGB);
	void SetCustomCursor(void);
	void CancelCustomCursor(void);
	void LoadRouteChecker(const std::string& filename);
	void ResetDepartureSequence(void);
	void ResetTrackedRecorder(void);
	bool LoadTransitionLevel(const std::string& filename);
	bool LoadMetricAltitude(const std::string& filename);
	std::string DisplayRouteMessage(const std::string& departure, const std::string& arrival);
};
