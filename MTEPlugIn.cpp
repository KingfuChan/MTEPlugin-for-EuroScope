// MTEPforES.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MTEPlugIn.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_VERSION "1.4.1"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2021 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugIn-for-EuroScope"
#endif // !COPYRIGHTS


// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1; // GS(KPH) with indicator
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_FPM = 3; // V/S(fpm) in 4 digits
const int TAG_ITEM_TYPE_CDL_IND = 4; // TODO: Climb / Descend / Level indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // TODO: Acutal flight level (m)

// GROUND SPEED TREND CHAR
const char CHR_GS_NON = ' ';
const char CHR_GS_ACC = 'A';
const char CHR_GS_DEC = 'L';

// COMPUTERISING RELATED
const float CONVERSION_KN_KPH = 1.85184; // 1 knot = 1.85184 kph
const float THRESHOLD_ACC_DEC = 2.5; // threshold (kph) to determin accel/decel

// SETTING NAMES
const char* SETTING_CUSTOM_CURSOR = "CustomCursor";

// CURSOR RELATED
bool initCursor = true; // if cursor has not been set to custom / if using default cursor
bool customCursor = false; // if cursor need to be set to custom
WNDPROC gSourceProc;
HWND pluginWindow;
HCURSOR myCursor = nullptr;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#define CPCUR CopyCursor((HCURSOR)::LoadImage(GetModuleHandle("MTEPlugIn.dll"), MAKEINTRESOURCE(IDC_CURSOR1), IMAGE_CURSOR, 0, 0, LR_SHARED))
// Note that cursor setting will only be effective with it's original file name (MTEPlugIn.dll)

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
	RegisterTagItemType("V/S(fpm) in 4 digits", TAG_ITEM_TYPE_VS_FPM);

	const char* setcc = GetDataFromSettings(SETTING_CUSTOM_CURSOR);
	customCursor = setcc == nullptr ? false : !strcmp(setcc, "1"); // 1 means true
	myCursor = CPCUR;
	if (customCursor && myCursor == nullptr)
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			"Cursor will not be set! Use original dll file name (MTEPlugIn.dll) to enable custom cursor.",
			1, 0, 0, 0, 0);
	else if (customCursor && myCursor != nullptr)
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			"If custom cursor does not show on radar screen, please use \".mtep cursor on\" command.",
			1, 0, 0, 0, 0);
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
	case TAG_ITEM_TYPE_VS_FPM: {
		int vs = abs(RadarTarget.GetVerticalSpeed());
		if (vs > 100) {
			vs = vs < 10000 ? vs : 9999; // in case of overflow
			sprintf_s(sItemString, 5, "%04d", vs);
		}
		else // recogize as level flight
			sprintf_s(sItemString, 5, "    ");

		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnTimer(int Counter)
{
	if (initCursor && customCursor) SetCustomCursor(); // cursor
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

void CMTEPlugIn::SetCustomCursor(void)
{
	// correlate cursor
	if (myCursor == nullptr) return;
	pluginWindow = GetActiveWindow();
	gSourceProc = (WNDPROC)SetWindowLong(pluginWindow, GWL_WNDPROC, (LONG)WindowProc);
	initCursor = false;
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Cursor is set!", 1, 0, 0, 0, 0);
}

void CMTEPlugIn::CancelCustomCursor(void)
{
	// uncorrelate cursor
	if (myCursor == nullptr) return;
	SetWindowLong(pluginWindow, GWL_WNDPROC, (LONG)gSourceProc);
	initCursor = true;
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Cursor is reset!", 1, 0, 0, 0, 0);
}
