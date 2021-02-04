// MTEPforES.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MTEPlugIn.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_VERSION "1.3.0"
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
const float THRESHOLD_ACC_DEC = 2.5; // threshold (kph) to determin accel/decel

// SETTING NAMES
const char* SETTING_CUSTOM_CURSOR = "CustomCursor";

// CURSOR RELATED
// Note that cursor setting will only be effective with it's original file name (MTEPlugIn.dll)
bool initCursor = true; // if cursor is set to custom
bool customCursor = false; // if cursor need to be set to custom
WNDPROC gSourceProc;
HWND pluginWindow;
HCURSOR myCursor = nullptr;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#define CPCUR CopyCursor((HCURSOR)::LoadImage(GetModuleHandle("MTEPlugIn.dll"), MAKEINTRESOURCE(IDC_CURSOR1), IMAGE_CURSOR, 0, 0, LR_SHARED))
void SetCustomCursor(void);
void CancelCustomCursor(void);

using namespace EuroScopePlugIn;


CMTEPlugIn::CMTEPlugIn(void)
	: CPlugIn(COMPATIBILITY_CODE,
		PLUGIN_NAME,
		PLUGIN_VERSION,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	AddAlias(".mteplugin", GITHUB_LINK); // for testing and for fun
	RegisterTagItemType("GS(KPH) with indicator", TAG_ITEM_TYPE_GS_W_IND);
	RegisterTagItemType("RMK/STS indicator", TAG_ITEM_TYPE_RMK_IND);

	const char* setcc = GetDataFromSettings(SETTING_CUSTOM_CURSOR);
	customCursor = setcc == nullptr ? false : !strcmp(setcc, "1"); // 1 means true
	myCursor = CPCUR;
	//pluginWindow = GetActiveWindow();
}

CMTEPlugIn::~CMTEPlugIn(void)
{
	CancelCustomCursor();
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

void CMTEPlugIn::OnTimer(int Counter)
{
	// cursor
	if (initCursor && customCursor) SetCustomCursor();
}

bool CMTEPlugIn::OnCompileCommand(const char* sCommandLine) {
	CString cmd = sCommandLine;
	cmd.Trim();
	cmd.MakeUpper();

	if (cmd == ".MTEP CURSOR ON") {
		if (customCursor) { // reset
			CancelCustomCursor();
			SetCustomCursor();
		}
		else {
			customCursor = true;
			SetCustomCursor();
			SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", "1");
		}
		return true;
	}
	if (cmd == ".MTEP CURSOR OFF") {
		if (!customCursor) return true;
		customCursor = false;
		CancelCustomCursor();
		SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", "0");
		return true;
	}
	return false;
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SETCURSOR:
		SetCursor(myCursor);
		return true;
	default:
		return CallWindowProc(gSourceProc, hwnd, uMsg, wParam, lParam);
	}
}

void SetCustomCursor(void)
{
	// correlate cursor
	if (myCursor == nullptr) return;
	pluginWindow = GetActiveWindow();
	gSourceProc = (WNDPROC)SetWindowLong(pluginWindow, GWL_WNDPROC, (LONG)WindowProc);
	initCursor = false;
}

void CancelCustomCursor(void)
{
	// uncorrelate cursor
	if (myCursor == nullptr) return;
	SetWindowLong(pluginWindow, GWL_WNDPROC, (LONG)gSourceProc);
	initCursor = true;
}
