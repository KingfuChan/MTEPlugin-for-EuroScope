// MTEPforES.h: MTEPforES DLL 的主标头文件
//

#pragma once

#include "resource.h"		// 主符号
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

private:
	int GetRadarGS(CRadarTarget RadarTarget);
	char GetGSTrend(CRadarTarget RadarTarget);
};
