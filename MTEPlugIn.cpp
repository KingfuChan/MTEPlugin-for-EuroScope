// MTEPforES.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "MTEPlugIn.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_VERSION "1.1.0"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2021 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugIn-for-EuroScope"
#endif // !COPYRIGHTS


// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1;
const int TAG_ITEM_TYPE_RMK_IND = 2;

using namespace EuroScopePlugIn;


CMTEPlugIn::CMTEPlugIn(void)
	: CPlugIn(COMPATIBILITY_CODE,
		PLUGIN_NAME,
		PLUGIN_VERSION,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	AddAlias(".mtep", GITHUB_LINK); // for testing and for fun
	RegisterTagItemType("GS(KPH) with indicator", TAG_ITEM_TYPE_GS_W_IND);
	RegisterTagItemType("RMK/STS indicator", TAG_ITEM_TYPE_RMK_IND);
}

CMTEPlugIn::~CMTEPlugIn(void)
{

}

void CMTEPlugIn::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget,
	int ItemCode, int TagData, char sItemString[16],
	int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!(FlightPlan.IsValid() || RadarTarget.IsValid()))
		return;

	switch (ItemCode)
	{
	case TAG_ITEM_TYPE_GS_W_IND: {

		break; }
	case TAG_ITEM_TYPE_RMK_IND: {
		CString remarks;
		remarks = FlightPlan.GetFlightPlanData().GetRemarks();
		remarks.MakeUpper();
		if (remarks.Find("RMK/") != -1 || remarks.Find("STS/") != -1)
			sprintf_s(sItemString, 2, "*");

		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnTimer(int Counter)
{
	// update active information for all aircrafts
	int CustomRefreshIntv = 60;
	if (Counter % CustomRefreshIntv) // once every n seconds
		return;

	CRadarTarget rt;
	int idx;
	bool pre_ol, cur_ol;
	for (idx = 0; idx < m_TagDataArray.GetCount(); idx++) {
		pre_ol = m_TagDataArray[idx].m_active;
		cur_ol = IsCallsignOnline(m_TagDataArray[idx].m_callsign);
		m_TagDataArray[idx].m_active = cur_ol;
		if (!cur_ol) { // avoid potential errors
			TRACE("%s\tinactive\n", m_TagDataArray[idx].m_callsign);
			continue;
		}
		if (!pre_ol && !cur_ol) { // has been offline for 2 checks
			TRACE("%s\tremoved\n", m_TagDataArray[idx].m_callsign);
			m_TagDataArray.RemoveAt(idx);
		}
	}
}

bool CMTEPlugIn::IsCallsignOnline(const char* callsign) {
	CRadarTarget rt;
	for (rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt))
		if (!strcmp(callsign, rt.GetCallsign()))
			return true;
	return false;
}
