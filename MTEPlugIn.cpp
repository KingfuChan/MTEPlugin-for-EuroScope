// MTEPlugIn.cpp


#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MTEPlugIn.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_VERSION "2.0.1"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2021 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugIn-for-EuroScope"
#endif // !COPYRIGHTS

using namespace EuroScopePlugIn;

// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1; // GS(KPH) with indicator
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_FPM = 3; // V/S(fpm) in 4 digits
const int TAG_ITEM_TYPE_LVL_IND = 4; // Climb / Descend / Level indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // Actual altitude (m)
const int TAG_ITEM_TYPE_CFL_MTR = 6; // Cleared flight level (m)
const int TAG_ITEM_TYPE_RFL_ICAO = 7; // Final flight level (m/FL)
const int TAG_ITEM_TYPE_SC_IND = 8; // Similar callsign indicator
const int TAG_ITEM_TYPE_RFL_IND = 9; // RFL unit indicator
const int TAG_ITEM_TYPE_RVSM_IND = 10; // RVSM indicator
const int TAG_ITEM_TYPE_COMM_IND = 11; // COMM ESTB indicator
const int TAG_ITEM_TYPE_ASSIGN_VS = 12; // Assigned V/S in 4 digits
const int TAG_ITEM_TYPE_RECAT = 13; // RECAT-CN

// TAG ITEM FUNCTION
const int TAG_ITEM_FUNCTION_COMM_ESTAB = 1; // Set COMM ESTB
const int TAG_ITEM_FUNCTION_RATE_W_CD = 2; // Assign V/S with +/-

// GROUND SPEED TREND CHAR
const char CHR_GS_NON = ' ';
const char CHR_GS_ACC = 'A';
const char CHR_GS_DEC = 'L';

// COMPUTERISING RELATED
const float THRESHOLD_ACC_DEC = 2.5; // threshold (kph) to determin accel/decel
#define KN2KPH(int) 1.85184*(int) // 1 knot = 1.85184 kph
#define KPH2KN(int) (int)/1.85184
int CalculateVerticalSpeed(CRadarTarget RadarTarget);

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
	RegisterTagItemType("Final flight level (m/FL)", TAG_ITEM_TYPE_RFL_ICAO);
	RegisterTagItemType("Similar callsign indicator", TAG_ITEM_TYPE_SC_IND);
	RegisterTagItemType("RFL unit indicator", TAG_ITEM_TYPE_RFL_IND);
	RegisterTagItemType("RVSM indicator", TAG_ITEM_TYPE_RVSM_IND);
	RegisterTagItemType("COMM ESTB indicator", TAG_ITEM_TYPE_COMM_IND);
	//RegisterTagItemType("Assigned V/S in 4 digits", TAG_ITEM_TYPE_ASSIGN_VS);
	RegisterTagItemType("RECAT-CN", TAG_ITEM_TYPE_RECAT);

	RegisterTagItemFunction("Set COMM ESTB", TAG_ITEM_FUNCTION_COMM_ESTAB);
	//RegisterTagItemFunction("Assign V/S with +/-", TAG_ITEM_FUNCTION_RATE_W_CD);

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
		// remarks.MakeUpper(); // could crash by some special characters in certain system environment
		if (remarks.Find("RMK/") != -1 || remarks.Find("STS/") != -1)
			sprintf_s(sItemString, 2, "*");
		else
			sprintf_s(sItemString, 2, " ");

		break; }
	case TAG_ITEM_TYPE_VS_FPM: {
		int vs = abs(CalculateVerticalSpeed(RadarTarget));
		if (vs > 100) {
			vs = vs > 9999 ? 9999 : vs; // in case of overflow
			sprintf_s(sItemString, 5, "%04d", vs);
		}
		//else // recogize as level flight
		//	sprintf_s(sItemString, 5, "    "); // place-holder

		break; }
	case TAG_ITEM_TYPE_LVL_IND: {
		int vs = CalculateVerticalSpeed(RadarTarget);
		if (vs > 100)
			sprintf_s(sItemString, 2, "^");
		else if (vs < -100)
			sprintf_s(sItemString, 2, "|");
		else
			sprintf_s(sItemString, 2, ">");

		break; }
	case TAG_ITEM_TYPE_AFL_MTR: {
		int trsAlt = GetTransitionAltitude(); // should be transition level
		int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
		int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
		int dspAlt = stdAlt >= trsAlt ? stdAlt : qnhAlt;
		dspAlt = round(MetricAlt::FeettoM(dspAlt) / 10.0);
		dspAlt = dspAlt > 9999 ? 9999 : dspAlt; // in case of overflow
		sprintf_s(sItemString, 5, "%04d", dspAlt);

		break; }
	case TAG_ITEM_TYPE_CFL_MTR: {
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		switch (cflAlt)
		{
		case 2: // cleared for visual approach
			sprintf_s(sItemString, 6, "V/APP");
			break;
		case 1: // cleared for ILS approach
			sprintf_s(sItemString, 5, "ILS ");
			break;
		case 0: // no cleared level
			sprintf_s(sItemString, 5, "    ");
			break;
		default: // have a cleared level
			cflAlt = MetricAlt::LvlFeettoM(cflAlt) / 10;
			cflAlt = cflAlt > 9999 ? 9999 : cflAlt; // in case of overflow
			sprintf_s(sItemString, 5, "%04d", cflAlt);
			break;
		}

		break; }
	case TAG_ITEM_TYPE_RFL_ICAO: {
		int rflCtr = FlightPlan.GetControllerAssignedData().GetFinalAltitude();
		int rflFpl = FlightPlan.GetFinalAltitude();
		int rflAlt = rflCtr ? rflCtr : rflFpl;
		int dspMtr;
		if (MetricAlt::RflFeettoM(rflAlt, dspMtr)) { // is metric RVSM
			char trsMrk = rflAlt >= GetTransitionAltitude() ? 'S' : 'M';
			dspMtr = dspMtr / 10 > 9999 ? 9999 : dspMtr / 10; // in case of overflow
			sprintf_s(sItemString, 6, "%c%04d", trsMrk, dspMtr);
		}
		else {
			rflAlt = round(rflAlt / 100.0);
			rflAlt = rflAlt > 999 ? 999 : rflAlt; // in case of overflow
			sprintf_s(sItemString, 5, "F%03d", rflAlt);
		}

		break; }
	case TAG_ITEM_TYPE_SC_IND: {
		CString marker;
		StrMark::iterator item = m_similarMarker.find(RadarTarget.GetCallsign());
		if (item != m_similarMarker.end() && item->second) { // item exist and is similar
			sprintf_s(sItemString, 3, "SC");
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 0);
		}

		break; }
	case TAG_ITEM_TYPE_RFL_IND: {
		int rflCtr = FlightPlan.GetControllerAssignedData().GetFinalAltitude();
		int rflFpl = FlightPlan.GetFinalAltitude();
		int rflAlt = rflCtr ? rflCtr : rflFpl;
		int _meter;
		if (!MetricAlt::RflFeettoM(rflAlt, _meter))  // not metric RVSM
			sprintf_s(sItemString, 2, "#");

		break; }
	case TAG_ITEM_TYPE_RVSM_IND: {
		CFlightPlanData fpdata = FlightPlan.GetFlightPlanData();
		CString acinf = fpdata.GetAircraftInfo();
		if (!strcmp(fpdata.GetPlanType(), "V"))
			sprintf_s(sItemString, 2, "V");
		else if (acinf.GetLength() <= 8) { // assume FAA format
			char capa = fpdata.GetCapibilities();
			if (capa == 'H' || capa == 'W' || capa == 'J' || capa == 'K' || capa == 'L' || capa == 'Z' || capa == '?')
				sprintf_s(sItemString, 2, " ");
			else {
				sprintf_s(sItemString, 2, "X");
			}
		}
		else { // assume ICAO format, no format check
			CString acet = acinf.Mid(acinf.Find('-') + 1);
			if (acet.Left(acet.Find('/')).Find('W') >= 0)
				sprintf_s(sItemString, 2, " ");
			else
				sprintf_s(sItemString, 2, "X");
		}

		break; }
	case TAG_ITEM_TYPE_COMM_IND: {
		// in marker, false means not established
		int state = FlightPlan.GetState();
		if (!
			(RadarTarget.GetPosition().GetTransponderC() &&
				(
					state == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED ||
					state == FLIGHT_PLAN_STATE_ASSUMED
					)
				)
			)
			break; // only valid for assumed flights
		CString marker;
		StrMark::iterator item = m_communMarker.find(RadarTarget.GetCallsign());
		if (item == m_communMarker.end() || !item->second) { // comm not established
			sprintf_s(sItemString, 2, "C");
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 255);
		}

		break; }
	case TAG_ITEM_TYPE_ASSIGN_VS: {
		//////////////////// NOT IN USE ////////////////////
		int rate = FlightPlan.GetControllerAssignedData().GetAssignedRate();
		char cd = rate > 0 ? '+' : '-';
		rate = rate > 9999 ? 9999 : rate; // in case of overflow
		sprintf_s(sItemString, 6, "%c%4d", cd, rate);
		//////////////////// NOT IN USE ////////////////////
		break; }
	case TAG_ITEM_TYPE_RECAT: {
		char categ = FlightPlan.GetFlightPlanData().GetAircraftWtc();
		CString acType = FlightPlan.GetFlightPlanData().GetAircraftFPType();
		if (categ == 'H' && m_ReCatMap.count(acType))
			sprintf_s(sItemString, 3, "-%c", m_ReCatMap.at(acType));

		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	CRadarTarget RadarTarget = RadarTargetSelectASEL();
	if (!RadarTarget.IsValid()) return;

	switch (FunctionId)
	{
	case TAG_ITEM_FUNCTION_COMM_ESTAB: {
		int state = RadarTarget.GetCorrelatedFlightPlan().GetState();
		if (RadarTarget.GetPosition().GetTransponderC() && // in-flight, ignores on ground
			( // consider assumed aircraft
				state == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED ||
				state == FLIGHT_PLAN_STATE_ASSUMED
				))
			m_communMarker[RadarTarget.GetCallsign()] = true;

		break; }
	case TAG_ITEM_FUNCTION_RATE_W_CD: {
		//////////////////// NOT IN USE ////////////////////
		break;	}
	default:
		break;
	}
}

void CMTEPlugIn::OnTimer(int Counter)
{
	if (initCursor && customCursor) SetCustomCursor(); // cursor

	// deals with similar callsign stuff, and communication establish stuff
	StrMark mkrCN, mkrEN;
	for (CRadarTarget rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt)) {
		CFlightPlan fp = rt.GetCorrelatedFlightPlan();
		int state = fp.GetState();
		if (rt.GetPosition().GetTransponderC() && // in-flight, ignores on ground
			( // consider assumed aircraft
				state == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED ||
				state == FLIGHT_PLAN_STATE_ASSUMED
				)
			)
		{ // used for similar callsign
			if (!fp.IsTextCommunication()) {
				if (IsCallsignChinese(fp))
					mkrCN[rt.GetCallsign()] = false;
				else
					mkrEN[rt.GetCallsign()] = false;
			}
			else
				OutputDebugString("Text aircraft!");
		}
		else { // used for communication marker
			m_communMarker[rt.GetCallsign()] = false;
		}
	}
	m_similarMarker.clear();
	m_similarMarker.merge(ParseSimilarCallsign(mkrCN));
	m_similarMarker.merge(ParseSimilarCallsign(mkrEN));
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

int CalculateVerticalSpeed(CRadarTarget RadarTarget)
{
	CRadarTargetPositionData curpos = RadarTarget.GetPosition();
	CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
	int curAlt = curpos.GetPressureAltitude();
	int preAlt = prepos.GetPressureAltitude();
	int preT = prepos.GetReceivedTime();
	int curT = curpos.GetReceivedTime();
	int deltaT = preT - curT;
	deltaT = deltaT ? deltaT : INFINITE;
	return round((curAlt - preAlt) / deltaT * 60);
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
