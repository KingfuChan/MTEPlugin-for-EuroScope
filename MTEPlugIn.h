// MTEPforES.h: MTEPforES DLL 的主标头文件
//

#pragma once
// 主符号
#include "pch.h"
#include "framework.h"
#include "resource.h"		
#include <EuroScopePlugIn.h>
#include <string>

using namespace EuroScopePlugIn;

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
	int GetRadarGS(CRadarTarget RadarTarget);
	char GetGSTrend(CRadarTarget RadarTarget);
};
