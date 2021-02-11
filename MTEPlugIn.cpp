// MTEPlugIn.cpp

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MTEPlugIn.h"
#include "MetricAlt.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_VERSION "1.6.1"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2021 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugIn-for-EuroScope"
#endif // !COPYRIGHTS


// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1; // GS(KPH) with indicator
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_FPM = 3; // V/S(fpm) in 4 digits
const int TAG_ITEM_TYPE_LVL_IND = 4; // Climb / Descend / Level indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // Actual altitude (m)
const int TAG_ITEM_TYPE_CFL_MTR = 6; // Cleared flight level (m)
const int TAG_ITEM_TYPE_RFL_MTR = 7; // Final flight level (m)
const int TAG_ITEM_TYPE_SC_IND = 8; // Similar callsign indicator

// GROUND SPEED TREND CHAR
const char CHR_GS_NON = ' ';
const char CHR_GS_ACC = 'A';
const char CHR_GS_DEC = 'L';

// COMPUTERISING RELATED
const float THRESHOLD_ACC_DEC = 2.5; // threshold (kph) to determin accel/decel
#define KN2KPH(int) 1.85184*(int) // 1 knot = 1.85184 kph
#define KPH2KN(int) (int)/1.85184

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
	RegisterTagItemType("VS(fpm) in 4 digits", TAG_ITEM_TYPE_VS_FPM);
	RegisterTagItemType("Level indicator", TAG_ITEM_TYPE_LVL_IND);
	RegisterTagItemType("Actual altitude (m)", TAG_ITEM_TYPE_AFL_MTR);
	RegisterTagItemType("Cleared flight level (m)", TAG_ITEM_TYPE_CFL_MTR);
	RegisterTagItemType("Final flight level (m)", TAG_ITEM_TYPE_RFL_MTR);
	RegisterTagItemType("Similar callsign indicator", TAG_ITEM_TYPE_SC_IND);

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
		CRadarTargetPositionData curpos = RadarTarget.GetPosition();
		CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
		int curgs = curpos.GetReportedGS(); // current GS
		int dspgs = round(KN2KPH(curgs) / 10.0); // display GS
		dspgs = dspgs ? dspgs : RadarTarget.GetGS(); // If reported GS is 0 then uses the calculated one.
		dspgs = dspgs > 999 ? 999 : dspgs; // in case of overflow
		int pregs = prepos.GetReportedGS(); // previous GS
		double diff = KN2KPH(curgs - pregs);
		char gsTrend;
		if (diff >= THRESHOLD_ACC_DEC)
			gsTrend = CHR_GS_ACC;
		else if (diff <= -THRESHOLD_ACC_DEC)
			gsTrend = CHR_GS_DEC;
		else
			gsTrend = CHR_GS_NON;

		sprintf_s(sItemString, 5, "%03d%c", dspgs, gsTrend);

		break; }
	case TAG_ITEM_TYPE_RMK_IND: {
		CString remarks;
		remarks = FlightPlan.GetFlightPlanData().GetRemarks();
		remarks.MakeUpper();
		if (remarks.Find("RMK/") != -1 || remarks.Find("STS/") != -1)
			sprintf_s(sItemString, 2, "*");
		else
			sprintf_s(sItemString, 2, " ");

		break; }
	case TAG_ITEM_TYPE_VS_FPM: {
		CRadarTargetPositionData curpos = RadarTarget.GetPosition();
		CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
		int curAlt = curpos.GetPressureAltitude();
		int preAlt = prepos.GetPressureAltitude();
		int preT = prepos.GetReceivedTime();
		int curT = curpos.GetReceivedTime();
		int deltaT = preT - curT;
		deltaT = deltaT ? deltaT : INFINITE;
		int vs = abs(round((curAlt - preAlt) / deltaT * 60));
		//int vs = abs(RadarTarget.GetVerticalSpeed()); // not accurate
		if (vs > 100) {
			vs = vs > 9999 ? 9999 : vs; // in case of overflow
			sprintf_s(sItemString, 5, "%04d", vs);
		}
		else // recogize as level flight
			sprintf_s(sItemString, 5, "    "); // place-holder

		break; }
	case TAG_ITEM_TYPE_LVL_IND: {
		int vs = RadarTarget.GetVerticalSpeed();
		if (vs > 100)
			sprintf_s(sItemString, 2, "^");
		else if (vs < -100)
			sprintf_s(sItemString, 2, "|");
		else
			sprintf_s(sItemString, 2, ">");

		break; }
	case TAG_ITEM_TYPE_AFL_MTR: {
		int trsAlt = GetTransitionAltitude();
		int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
		int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
		int dspAlt = qnhAlt > trsAlt ? stdAlt : qnhAlt;
		dspAlt = round(MetricAlt::FeettoM(dspAlt) / 10.0);
		dspAlt = dspAlt > 9999 ? 9999 : dspAlt; // in case of overflow
		sprintf_s(sItemString, 5, "%04d", dspAlt);

		break; }
	case TAG_ITEM_TYPE_CFL_MTR: {
		int cflAlt = FlightPlan.GetClearedAltitude();
		if (cflAlt != 0 && cflAlt != FlightPlan.GetFinalAltitude()) {
			cflAlt = MetricAlt::LvlFeettoM(cflAlt) / 10;
			cflAlt = cflAlt > 9999 ? 9999 : cflAlt; // in case of overflow
			sprintf_s(sItemString, 5, "%04d", cflAlt);
		}
		else
			sprintf_s(sItemString, 5, "    "); // place-holder

		break; }
	case TAG_ITEM_TYPE_RFL_MTR: {
		int rflAlt = MetricAlt::LvlFeettoM(FlightPlan.GetFinalAltitude()) / 10;
		rflAlt = rflAlt > 9999 ? 9999 : rflAlt; // in case of overflow
		sprintf_s(sItemString, 5, "%04d", rflAlt);

		break; }
	case TAG_ITEM_TYPE_SC_IND: {
		CString marker;
		StrMark::iterator item = m_similarMarker.find(RadarTarget.GetCallsign());
		if (item != m_similarMarker.end() && item->second) // item exist and is similar
			sprintf_s(sItemString, 2, "*");
		else
			sprintf_s(sItemString, 2, " ");

		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnTimer(int Counter)
{
	if (initCursor && customCursor) SetCustomCursor(); // cursor

	// deals with similar callsign stuff, scan every 3 seconds
	if (!(Counter % 3)) {
		m_similarMarker.clear(); // re-initialize map
		for (CRadarTarget rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt))
			if (rt.GetPosition().GetTransponderC()) // in-flight, ignores on ground
				m_similarMarker[rt.GetCallsign()] = false;
		ParseSimilarCallsign();
	}
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

void CMTEPlugIn::ParseSimilarCallsign(void)
{
	// parse m_similarMarker and set if a callsign is similar to others
	for (StrMark::iterator it1 = m_similarMarker.begin(); it1 != m_similarMarker.end(); it1++) {
		CharList cs1 = ExtractNumfromCallsign(it1->first);
		if (cs1.size() > 4) continue; // skip
		for (StrMark::iterator it2 = it1; it2 != m_similarMarker.end(); it2++) {
			if (it1 == it2) continue;
			CharList cs2 = ExtractNumfromCallsign(it2->first);
			if (cs2.size() > 4) continue; // skip

			// compare two callsign num
			CharList::iterator p1, p2;
			int same = 0; // same number on same position count
			CharList dn1, dn2; // different number on same position list
			for (p1 = cs1.begin(), p2 = cs2.begin(); p1 != cs1.end() && p2 != cs2.end(); p1++, p2++) {
				if (*p1 == *p2)
					same++;
				else {
					dn1.push_back(*p1);
					dn2.push_back(*p2);
				}
			}
			bool isSimilar = false;
			switch (same)
			{
			case 4:
			case 3:
				isSimilar = true;
				break;
			case 2:
			case 1: {
				dn1.sort();
				dn2.sort();
				isSimilar = dn1 == dn2;
				break;
			}
			default:
				isSimilar = false;
				break;
			}
			if (isSimilar) // same num count
				it1->second = it2->second = true;
		}
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

CharList ExtractNumfromCallsign(const CString callsign)
{
	// extract num from callsign, no less than 4 digits
	CharList csnum;
	bool numbegin = false;
	for (int i = 0; i < callsign.GetLength(); i++) {
		numbegin = numbegin || (callsign[i] >= '0' && callsign[i] <= '9');
		if (numbegin)
			csnum.push_back(callsign[i]);
	}
	int size = csnum.size();
	for (int i = 0; i < 4 - size; i++)
		csnum.push_back(' '); // make 4 digits
	return csnum;
}