// MTEPforES.h: MTEPforES DLL 的主标头文件
//

#pragma once

#include "resource.h"		// 主符号
#include <EuroScopePlugIn.h>
#include <string>

using namespace EuroScopePlugIn;

struct TagData {
	CString m_callsign;
	int m_ias;
	bool m_active;
};

class CMTEPlugIn :
	public EuroScopePlugIn::CPlugIn
{
public:
	CMTEPlugIn(void);
	~CMTEPlugIn(void);
	virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnTimer(int Counter);

private:
	CArray<TagData, TagData&> m_TagDataArray;
	bool IsCallsignOnline(const char* callsign);
};
