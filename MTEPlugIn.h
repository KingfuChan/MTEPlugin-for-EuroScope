// MTEPlugIn.h

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <list>
#include "MetricAlt.h"

typedef std::map<CString, bool> StrMark;
typedef std::list<char> CharList;

CharList ExtractNumfromCallsign(const CString callsign);
bool CompareCallsignNum(CharList cs1, CharList cs2);

class CMTEPlugIn :
	public EuroScopePlugIn::CPlugIn
{
public:
	CMTEPlugIn(void);
	~CMTEPlugIn(void);
	virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagMemData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char* sCommandLine);

private:
	StrMark m_similarMarker;
	void ParseSimilarCallsign(void);
	void SetCustomCursor(void);
	void CancelCustomCursor(void);
};
