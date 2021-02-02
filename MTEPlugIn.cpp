// MTEPforES.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "MTEPlugIn.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_VERSION "1.2.0"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2021 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugIn-for-EuroScope"
#endif // !COPYRIGHTS


// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1;
const int TAG_ITEM_TYPE_RMK_IND = 2;

// GROUND SPEED TREND CHAR
const char CHR_GS_NON = ' ';
const char CHR_GS_ACC = 'A';
const char CHR_GS_DEC = 'L';

// RELATED TO COMPUTERISING
const float CONVERSION_KN_KPH = 1.85184; // 1 knot = 1.85184 kph
const float THRESHOLD_ACC_DEC = 2.5; // threshold to determin accel/decel

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
		int cgs = round(GetRadarGS(RadarTarget) * CONVERSION_KN_KPH / 10.0);// current GS
		cgs = cgs < 1000 ? cgs : 999; // in case of overflow
		char ad = GetGSTrend(RadarTarget);
		sprintf_s(sItemString, 5, "%03d%c", cgs, ad);

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

int CMTEPlugIn::GetRadarGS(CRadarTarget RadarTarget)
{
	int rpgs = RadarTarget.GetPosition().GetReportedGS();
	rpgs = rpgs ? rpgs : RadarTarget.GetGS();
	//If reported GS is 0 then uses the calculated one.
	return rpgs;
}

char CMTEPlugIn::GetGSTrend(CRadarTarget RadarTarget)
{
	CRadarTargetPositionData curpos = RadarTarget.GetPosition();
	CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
	int curgs = curpos.GetReportedGS();
	int pregs = prepos.GetReportedGS();
	float diff = (curgs - pregs) * CONVERSION_KN_KPH;
	if (diff >= THRESHOLD_ACC_DEC)
		return CHR_GS_ACC;
	else if (diff <= -THRESHOLD_ACC_DEC)
		return CHR_GS_DEC;
	else
		return CHR_GS_NON;
}