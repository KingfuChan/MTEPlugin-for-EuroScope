// MTEPlugin.cpp

#pragma once

#include "pch.h"
#include "Version.h"
#include "MTEPlugin.h"

#ifndef COPYRIGHTS
constexpr auto PLUGIN_NAME = "MTEPlugin";
constexpr auto PLUGIN_AUTHOR = "Kingfu Chan";
constexpr auto PLUGIN_COPYRIGHT = "MIT License, Copyright (c) 2022 Kingfu Chan";
constexpr auto GITHUB_LINK = "https://github.com/KingfuChan/MTEPlugin-for-EuroScope";
#endif // !COPYRIGHTS

// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1; // Ground speed (duplicate)
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_AHIDE = 3; // Vertical speed (FPM)
const int TAG_ITEM_TYPE_LVL_IND = 4; // Climb/Descend/Level indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // Actual altitude (m/ft)
const int TAG_ITEM_TYPE_CFL_FLX = 6; // Cleared flight level (m/FL)
const int TAG_ITEM_TYPE_RFL_ICAO = 7; // Final flight level (ICAO)
const int TAG_ITEM_TYPE_SC_IND = 8; // Similar callsign indicator
const int TAG_ITEM_TYPE_RFL_IND = 9; // RFL unit indicator
const int TAG_ITEM_TYPE_RVSM_IND = 10; // RVSM indicator
const int TAG_ITEM_TYPE_COMM_IND = 11; // COMM ESTB indicator
const int TAG_ITEM_TYPE_RECAT_BC = 12; // RECAT-CN (H-B/C)
const int TAG_ITEM_TYPE_RTE_CHECK = 13; // Route validity
const int TAG_ITEM_TYPE_SQ_DUPE = 14; // Tracked DUPE warning
const int TAG_ITEM_TYPE_DEP_SEQ = 15; // Departure sequence
const int TAG_ITEM_TYPE_RVEC_IND = 16; // Radar vector indicator
const int TAG_ITEM_TYPE_CFL_MTR = 17; // Cleared flight level (m)
const int TAG_ITEM_TYPE_RCNT_IND = 18; // Reconnected indicator
const int TAG_ITEM_TYPE_DEP_STS = 19; // Departure status
const int TAG_TIEM_TYPE_RECAT_WTC = 20; // RECAT-CN (LMCBJ)
const int TAG_ITEM_TYPE_ASPD_BND = 21; // Assigned speed bound (Topsky, +/-)
const int TAG_ITEM_TYPE_GS_CALC = 22; // Ground speed
const int TAG_ITEM_TYPE_UNIT_IND = 23; // Unit indicator
const int TAG_ITEM_TYPE_VS_TOGGL = 24; // Vertical speed (FPM, duplicate)

// TAG ITEM FUNCTION
const int TAG_ITEM_FUNCTION_COMM_ESTAB = 1; // Set COMM ESTB
const int TAG_ITEM_FUNCTION_RCNT_RST = 2; // Restore assigned data
const int TAG_ITEM_FUNCTION_VS_DISP = 3; // Toggle vertical speed display
const int TAG_ITEM_FUNCTION_CFL_SET_EDIT = 10; // Set CFL from edit (not registered)
const int TAG_ITEM_FUNCTION_CFL_MENU = 11; // Open CFL popup menu
const int TAG_ITEM_FUNCTION_CFL_EDIT = 12; // Open CFL popup edit
const int TAG_ITEM_FUNCTION_CFL_SET_MENU = 13; // Set CFL from menu (not registered)
const int TAG_ITEM_FUNCTION_CFL_TOPSKY = 14; // Confirm CFL / Open Topsky CFL menu
const int TAG_ITEM_FUNCTION_RFL_SET_EDIT = 20; // Set RFL from edit (not registered)
const int TAG_ITEM_FUNCTION_RFL_MENU = 21; // Open RFL popup menu
const int TAG_ITEM_FUNCTION_RFL_EDIT = 22; // Open RFL popup edit
const int TAG_ITEM_FUNCTION_RFL_SET_MENU = 23; // Set RFL from menu (not registered)
const int TAG_ITEM_FUNCTION_SC_LIST = 30; // Open similar callsign list
const int TAG_ITEM_FUNCTION_SC_SELECT = 31; // Select in similar callsign list (not registered)
const int TAG_ITEM_FUNCTION_RTE_INFO = 40; // Show route checker info
const int TAG_ITEM_FUNCTION_DSQ_MENU = 50; // Set departure sequence
const int TAG_ITEM_FUNCTION_DSQ_EDIT = 51; // Open departure sequence popup edit (not registered)
const int TAG_ITEM_FUNCTION_DSQ_STS = 52; // Set departure status
const int TAG_ITEM_FUNCTION_SPD_SET = 60; // Set assigned speed (not registered)
const int TAG_ITEM_FUNCTION_SPD_LIST = 61; // Open assigned speed popup list
const int TAG_ITEM_FUNCTION_UNIT_MENU = 70; // Open unit settings popup menu
const int TAG_ITEM_FUNCTION_UNIT_SET = 71; // Set unit from menu (not registered)

// COMPUTERISING RELATED
static constexpr double KN_KPH(const double& k) { return 1.85184 * k; } // 1 knot = 1.85184 kph
static constexpr double KPH_KN(const double& k) { return k / 1.85184; } // 1.85184 kph = 1 knot
static constexpr int OVRFLW2(const int& t) { return t > 99 || t < 0 ? 99 : t; } // overflow pre-process 2 digits
static constexpr int OVRFLW3(const int& t) { return t > 999 || t < 0 ? 999 : t; } // overflow pre-process 3 digits
static constexpr int OVRFLW4(const int& t) { return t > 9999 || t < 0 ? 9999 : t; }  // overflow pre-process 4 digits
inline std::string MakeUpper(const std::string& str);
inline bool IsCFLAssigned(CFlightPlan FlightPlan);
inline int GetLastRadarInterval(CRadarTargetPositionData pos1, CRadarTargetPositionData pos2);

// SETTING RELATED
inline std::string GetRealFileName(const std::string& path);
// NON-REALTIME READ SETTINGS, CHANGE BY COMMAND ONLY
constexpr auto SETTING_TRANS_LVL_CSV = "TransLevelCSV"; // file name
constexpr auto SETTING_ROUTE_CHECKER_CSV = "RteCheckerCSV"; // file name
constexpr auto SETTING_TRANS_MALT_TXT = "MetricAltitudeTXT"; // file name
constexpr auto SETTING_CUSTOM_CURSOR = "CustomCursor"; // bool, *0*
constexpr auto DEFAULT_CUSTOM_CURSOR = false;
constexpr auto SETTING_AUTO_RETRACK = "AutoRetrack"; // *0*: off, 1: silent, 2: notify
constexpr auto DEFAULT_AUTO_RETRACK = 0;
constexpr auto SETTING_AMEND_CFL = "AmendQFEinCFL"; // 0: off, *1*: MTEP, 2: all
constexpr auto DEFAULT_AMEND_CFL = 1;
// REALTIME READ SETTINGS, CHANGE BY LOAD SETTINGS
constexpr auto SETTING_CUSTOM_NUMBER_MAP = "CustomNumber0-9"; // char[10], *0123456789*
const std::string DEFAULT_CUSTOM_NUMBER_MAP = "0123456789"; // specify std::string for template function
// ALTITUDE
constexpr auto SETTING_ALT_FEET = "ALT/Feet"; // bool, *0*, include command
constexpr auto DEFAULT_ALT_FEET = false;
constexpr auto SETTING_ALT_TOGG = "ALT/NoToggle"; // bool, *0*
constexpr auto DEFAULT_ALT_TOGG = false;
// VERTICAL SPEED
constexpr auto SETTING_VS_MODE = "VS/Mode"; // *-1*: auto-hide, 0: hide, 1: show, include command.
constexpr auto DEFAULT_VS_MODE = -1; // Auto-hide disables VS toggle.
constexpr auto SETTING_VS_THLD = "VS/Threshold"; // positive int, *100*
constexpr auto DEFAULT_VS_THLD = 100;
constexpr auto SETTING_VS_RNDG = "VS/Rounding"; // positive int, *1*
constexpr auto DEFAULT_VS_RNDG = 1;
// GOUND SPEED
constexpr auto SETTING_GS_KNOT = "GS/Knot"; // bool, *0*, include command
constexpr auto DEFAULT_GS_KNOT = false;
constexpr auto SETTING_GS_MODE = "GS/ModeThreshold"; // int, *0*
constexpr auto DEFAULT_GS_MODE = 0;
constexpr auto SETTING_GS_TREND = "GS/TrendThreshold"; // positive int, *5*
constexpr auto DEFAULT_GS_TREND = 5;
constexpr auto SETTING_GS_INC = "GS/IncreaseMark"; // char, *NULL*
//constexpr auto DEFAULT_GS_INC = '\0';
constexpr auto SETTING_GS_STA = "GS/StableMark"; // char, *NULL*
//constexpr auto DEFAULT_GS_STA = '\0';
constexpr auto SETTING_GS_DEC = "GS/DecreaseMark"; // char, *NULL*
//constexpr auto DEFAULT_GS_DEC = '\0';
// COLOR DEFINITIONS (R:G:B)
constexpr auto SETTING_COLOR_CFL_CONFRM = "Color/CFLNeedConfirm";
constexpr auto SETTING_COLOR_CS_SIMILR = "Color/SimilarCallsign";
constexpr auto SETTING_COLOR_COMM_ESTAB = "Color/CommNoEstablish";
constexpr auto SETTING_COLOR_RC_INVALID = "Color/RouteInvalid";
constexpr auto SETTING_COLOR_RC_UNCERTN = "Color/RouteUncertain";
constexpr auto SETTING_COLOR_SQ_DUPE = "Color/SquawkDupe";
constexpr auto SETTING_COLOR_DS_NUMBR = "Color/DSRestore";
constexpr auto SETTING_COLOR_DS_STATE = "Color/DSNotCleared";
constexpr auto SETTING_COLOR_RDRV_IND = "Color/RadarVector";
constexpr auto SETTING_COLOR_RECONT_IND = "Color/Reconnected";
constexpr auto SETTING_COLOR_RVSM_IND = "Color/RVSMIndicator";

// WINAPI RELATED
WNDPROC prevWndFunc = nullptr;
HWND pluginWindow = nullptr;
HMODULE pluginModule = nullptr;
HCURSOR pluginCursor = nullptr;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

CMTEPlugIn::CMTEPlugIn(void)
	: CPlugIn(COMPATIBILITY_CODE,
		PLUGIN_NAME,
#ifdef DEBUG
		VERSION_FILE_STR " DEBUG",
#else
		VERSION_FILE_STR,
#endif // DEBUG
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	DWORD dwCurPID = GetCurrentProcessId();
	EnumWindows(EnumWindowsProc, (LPARAM)&dwCurPID); // to set pluginWindow
	pluginModule = AfxGetInstanceHandle();

	UINT dpiSys = GetDpiForWindow(GetDesktopWindow());
	UINT dpiWnd = GetDpiForWindow(pluginWindow);
	int curSize = (int)((float)(dpiSys < 96 ? 96 : dpiSys) / (float)(dpiWnd < 96 ? 96 : dpiWnd) * 32.0);
	pluginCursor = CopyCursor(LoadImage(pluginModule, MAKEINTRESOURCE(IDC_CURSORCROSS), IMAGE_CURSOR, curSize, curSize, LR_SHARED));

	if (GetPluginSetting(SETTING_CUSTOM_CURSOR, DEFAULT_CUSTOM_CURSOR))
		SetCustomCursor();

	ResetTrackedRecorder();
	LoadRouteChecker();
	LoadTransitionLevel();
	LoadMetricAltitude();

	AddAlias(".mteplugin", GITHUB_LINK); // for testing and for fun

	RegisterTagItemType("Ground speed (duplicate)", TAG_ITEM_TYPE_GS_W_IND);
	RegisterTagItemType("RMK/STS indicator", TAG_ITEM_TYPE_RMK_IND);
	RegisterTagItemType("Vertical speed (FPM)", TAG_ITEM_TYPE_VS_AHIDE);
	RegisterTagItemType("Climb/Descend/Level indicator", TAG_ITEM_TYPE_LVL_IND);
	RegisterTagItemType("Actual altitude (m/ft)", TAG_ITEM_TYPE_AFL_MTR);
	RegisterTagItemType("Cleared flight level (m/FL)", TAG_ITEM_TYPE_CFL_FLX);
	RegisterTagItemType("Final flight level (ICAO)", TAG_ITEM_TYPE_RFL_ICAO);
	RegisterTagItemType("Similar callsign indicator", TAG_ITEM_TYPE_SC_IND);
	RegisterTagItemType("RFL unit indicator", TAG_ITEM_TYPE_RFL_IND);
	RegisterTagItemType("RVSM indicator", TAG_ITEM_TYPE_RVSM_IND);
	RegisterTagItemType("COMM ESTB indicator", TAG_ITEM_TYPE_COMM_IND);
	RegisterTagItemType("RECAT-CN (H-B/C)", TAG_ITEM_TYPE_RECAT_BC);
	RegisterTagItemType("Route validity", TAG_ITEM_TYPE_RTE_CHECK);
	RegisterTagItemType("Tracked DUPE warning", TAG_ITEM_TYPE_SQ_DUPE);
	RegisterTagItemType("Departure sequence", TAG_ITEM_TYPE_DEP_SEQ);
	RegisterTagItemType("Radar vector indicator", TAG_ITEM_TYPE_RVEC_IND);
	RegisterTagItemType("Cleared flight level (m)", TAG_ITEM_TYPE_CFL_MTR);
	RegisterTagItemType("Reconnected indicator", TAG_ITEM_TYPE_RCNT_IND);
	RegisterTagItemType("Departure status", TAG_ITEM_TYPE_DEP_STS);
	RegisterTagItemType("RECAT-CN (LMCBJ)", TAG_TIEM_TYPE_RECAT_WTC);
	RegisterTagItemType("Assigned speed bound (Topsky, +/-)", TAG_ITEM_TYPE_ASPD_BND);
	RegisterTagItemType("Ground speed", TAG_ITEM_TYPE_GS_CALC);
	RegisterTagItemType("Unit indicator", TAG_ITEM_TYPE_UNIT_IND);
	RegisterTagItemType("Vertical speed (FPM, duplicate)", TAG_ITEM_TYPE_VS_TOGGL);

	RegisterTagItemFunction("Set COMM ESTB", TAG_ITEM_FUNCTION_COMM_ESTAB);
	RegisterTagItemFunction("Restore assigned data", TAG_ITEM_FUNCTION_RCNT_RST);
	RegisterTagItemFunction("Toggle vertical speed display", TAG_ITEM_FUNCTION_VS_DISP);
	RegisterTagItemFunction("Open CFL popup menu", TAG_ITEM_FUNCTION_CFL_MENU);
	RegisterTagItemFunction("Open CFL popup edit", TAG_ITEM_FUNCTION_CFL_EDIT);
	RegisterTagItemFunction("Confirm CFL / Open Topsky CFL menu", TAG_ITEM_FUNCTION_CFL_TOPSKY);
	RegisterTagItemFunction("Open RFL popup menu", TAG_ITEM_FUNCTION_RFL_MENU);
	RegisterTagItemFunction("Open RFL popup edit", TAG_ITEM_FUNCTION_RFL_EDIT);
	RegisterTagItemFunction("Open similar callsign list", TAG_ITEM_FUNCTION_SC_LIST);
	RegisterTagItemFunction("Show route checker info", TAG_ITEM_FUNCTION_RTE_INFO);
	RegisterTagItemFunction("Set departure sequence", TAG_ITEM_FUNCTION_DSQ_MENU);
	RegisterTagItemFunction("Set departure status", TAG_ITEM_FUNCTION_DSQ_STS);
	RegisterTagItemFunction("Open assigned speed popup list", TAG_ITEM_FUNCTION_SPD_LIST);
	RegisterTagItemFunction("Open unit settings popup menu", TAG_ITEM_FUNCTION_UNIT_MENU);

	DisplayUserMessage("MESSAGE", "MTEPlugin",
		(std::string("MTEPlugin loaded! For help please refer to ") + GITHUB_LINK).c_str(),
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
	if (!FlightPlan.IsValid() && !RadarTarget.IsValid())
		return;
	switch (ItemCode)
	{
	case TAG_ITEM_TYPE_GS_W_IND: {
		if (!RadarTarget.IsValid()) break;
		CRadarTargetPositionData curpos = RadarTarget.GetPosition();
		CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
		int curgs = curpos.GetReportedGS();
		int pregs = prepos.GetReportedGS();
		double diff = KN_KPH((double)(curgs - pregs));
		char gsTrend;
		if (diff >= 5)
			gsTrend = 'A';
		else if (diff <= -5)
			gsTrend = 'L';
		else
			gsTrend = ' ';
		int dspgs = curgs ? curgs : RadarTarget.GetGS();
		dspgs = (int)round(KN_KPH((double)dspgs) / 10.0);
		sprintf_s(sItemString, 5, "%03d%c", OVRFLW3(dspgs), gsTrend);
		break;
	}
	case TAG_ITEM_TYPE_GS_CALC: {
		if (!RadarTarget.IsValid()) break;
		// determine if using calculated or reported
		int threshold = GetPluginSetting(SETTING_GS_MODE, DEFAULT_GS_MODE); // knots, when the gap is smaller than this value, use reported, otherwise use calculated.
		CRadarTargetPositionData curpos = RadarTarget.GetPosition();
		double gsrpt = curpos.GetReportedGS(); // can be 0 but should not affect consequent selection
		CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
		double distance = prepos.GetPosition().DistanceTo(curpos.GetPosition()); // n miles
		double elapsed = GetLastRadarInterval(curpos, prepos);
		double gscal = abs(distance / elapsed * 3600.0); // knots
		double gskts = abs(gsrpt - gscal) < (double)threshold ? gsrpt : gscal;
		std::string strgs = "";
		if (m_TrackedRecorder->IsForceKnot(RadarTarget)) { // force knot
			strgs = gskts < 995 ? std::format("{:02d} ", (int)round(gskts / 10.0)) : "++ "; // due to rounding
		}
		else { //convert to kph
			double gskph = KN_KPH(gskts);
			strgs = gskph < 1995 ? std::format("{:03d}", (int)round(gskph / 10.0)) : "+++"; // due to rounding
		}
		strcpy_s(sItemString, strgs.size() + 1, strgs.c_str());
		break;
	}
	case TAG_ITEM_TYPE_RMK_IND: {
		if (!FlightPlan.IsValid()) break;
		std::string remarks;
		remarks = FlightPlan.GetFlightPlanData().GetRemarks();
		if (remarks.find("RMK/") != std::string::npos || remarks.find("STS/") != std::string::npos)
			sprintf_s(sItemString, 2, "*");
		else
			sprintf_s(sItemString, 2, " ");
		break;
	}
	case TAG_ITEM_TYPE_VS_AHIDE:
	case TAG_ITEM_TYPE_VS_TOGGL: {
		if (!RadarTarget.IsValid()) break;
		int mode = GetPluginSetting(SETTING_VS_MODE, DEFAULT_VS_MODE);
		int vs = abs(CalculateVerticalSpeed(RadarTarget, true));
		int thld = abs(GetPluginSetting(SETTING_VS_THLD, DEFAULT_VS_THLD));
		// determines whether to show
		if (mode == -1) {
			if (vs < thld) {
				break;
			}
		}
		else if (!m_TrackedRecorder->IsDisplayVerticalSpeed(RadarTarget.GetSystemID())) {
			break;
		}
		else {
			vs = vs >= thld ? vs : 0;
		}
		sprintf_s(sItemString, 5, "%04d", OVRFLW4(vs));
		break;
	}
	case TAG_ITEM_TYPE_LVL_IND: {
		if (!RadarTarget.IsValid()) break;
		int vs = CalculateVerticalSpeed(RadarTarget);
		int thld = abs(GetPluginSetting(SETTING_VS_THLD, DEFAULT_VS_THLD));
		if (vs >= thld)
			sprintf_s(sItemString, 2, "^");
		else if (vs <= -thld)
			sprintf_s(sItemString, 2, "|");
		else
			sprintf_s(sItemString, 2, ">");
		break;
	}
	case TAG_ITEM_TYPE_AFL_MTR: {
		if (!RadarTarget.IsValid()) break;
		int altref;
		int rdrAlt = m_TransitionLevel->GetRadarDisplayAltitude(RadarTarget, altref);
		int dspAlt;
		std::string tmpStr;
		if (!m_TrackedRecorder->IsForceFeet(RadarTarget)) {
			dspAlt = (int)round(MetricAlt::FeettoM(rdrAlt) / 10.0);
			tmpStr = std::format("{:04d}", OVRFLW4(dspAlt));
		}
		else {
			dspAlt = (int)round(rdrAlt / 100.0);
			tmpStr = std::format("{:03d}", OVRFLW3(dspAlt));
		}
		std::string dspStr = tmpStr;
		if (altref == AltitudeReference::ALT_REF_QNH) {
			// use custom number mapping
			std::string numMap = GetPluginSetting(SETTING_CUSTOM_NUMBER_MAP, DEFAULT_CUSTOM_NUMBER_MAP);
			if (numMap.size() == 10) {
				std::transform(dspStr.begin(), dspStr.end(), dspStr.begin(), [numMap](auto& c) {
					return numMap[int(c - '0')];
					});
			}
		}
		else if (altref == AltitudeReference::ALT_REF_QFE) {
			dspStr = "(" + dspStr + ")";
		}
		strcpy_s(sItemString, dspStr.size() + 1, dspStr.c_str());
		break;
	}
	case TAG_ITEM_TYPE_CFL_FLX: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			*pColorCode = TAG_COLOR_REDUNDANT;
			GetColorDefinition(SETTING_COLOR_CFL_CONFRM, pColorCode, pRGB);
		}
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		if (cflAlt == 2) { // cleared for visual approach
			sprintf_s(sItemString, 3, "VA");
		}
		else if (cflAlt == 1) { // cleared for ILS approach
			sprintf_s(sItemString, 4, "ILS");
		}
		else if (cflAlt <= 0 && !IsCFLAssigned(FlightPlan)) { // no cleared level or CFL==RFL
			// TODO: sometimes FlightPlan.GetControllerAssignedData().GetClearedAltitude() returns -1.
			sprintf_s(sItemString, 5, "    ");
		}
		else { // have a cleared level
			cflAlt = FlightPlan.GetClearedAltitude(); // difference: no ILS/VA, no CFL will show RFL
			int dspAlt;
			int trslvl, elev;
			bool isQFE = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev).size() && elev && cflAlt < trslvl;
			cflAlt -= isQFE ? elev : 0;
			bool isfeet = m_TrackedRecorder->IsForceFeet(FlightPlan);
			if (!isfeet) {
				if (MetricAlt::RflFeettoM(cflAlt, dspAlt)) { // is metric RVSM
					dspAlt = dspAlt / 10;
				}
				else if (cflAlt % 1000) { // not metric RVSM nor FL xx0, convert to metric
					dspAlt = MetricAlt::FeettoM(cflAlt) / 10;
				}
				else {
					isfeet = true;
				}
			}
			if (isfeet) {
				dspAlt = cflAlt / 100; // FL xx0, show FL in feet
				sprintf_s(sItemString, 4, "%03d", OVRFLW3(dspAlt));
			}
			else {
				sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt));
			}
			if (isQFE) {
				std::string qfeAltStr = "(" + std::string(sItemString) + ")";
				strcpy_s(sItemString, qfeAltStr.size() + 1, qfeAltStr.c_str());
			}
		}
		break;
	}
	case TAG_ITEM_TYPE_CFL_MTR: {
		if (!FlightPlan.IsValid()) break;
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int dspAlt = MetricAlt::LvlFeettoM(cflAlt);
		sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt / 10));
		break;
	}
	case TAG_ITEM_TYPE_RFL_ICAO: {
		if (!FlightPlan.IsValid()) break;
		int rflAlt = FlightPlan.GetFinalAltitude();
		int dspMtr;
		if (MetricAlt::RflFeettoM(rflAlt, dspMtr) && !m_TrackedRecorder->IsForceFeet(FlightPlan)) {
			// is metric RVSM and not forced feet
			char trsMrk = rflAlt >= GetTransitionAltitude() ? 'S' : 'M';
			sprintf_s(sItemString, 6, "%c%04d", trsMrk, OVRFLW4(dspMtr / 10));
		}
		else {
			rflAlt = (int)round(rflAlt / 100.0);
			sprintf_s(sItemString, 5, "F%03d", OVRFLW3(rflAlt));
		}
		break;
	}
	case TAG_ITEM_TYPE_SC_IND: {
		if (!FlightPlan.IsValid()) break;
		if (m_TrackedRecorder->IsSimilarCallsign(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 3, "SC");
			*pColorCode = TAG_COLOR_INFORMATION;
			GetColorDefinition(SETTING_COLOR_CS_SIMILR, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_RFL_IND: {
		if (!FlightPlan.IsValid()) break;
		bool plnfeet = m_TrackedRecorder->IsForceFeet(FlightPlan);
		int rflAlt = FlightPlan.GetFinalAltitude();
		int _meter;
		bool rflfeet = !MetricAlt::RflFeettoM(rflAlt, _meter);
		if (plnfeet != rflfeet)  // discrepancy
			strcpy_s(sItemString, 2, "#");
		break;
	}
	case TAG_ITEM_TYPE_RVSM_IND: {
		if (!FlightPlan.IsValid()) break;
		CFlightPlanData fpdata = FlightPlan.GetFlightPlanData();
		std::string acinf = fpdata.GetAircraftInfo();
		char ind = ' ';
		if (!strcmp(fpdata.GetPlanType(), "V"))
			ind = 'V';
		else if (acinf.size() <= 8) { // assume FAA format
			char capa = fpdata.GetCapibilities();
			if (std::string("HWJKLZ?").find(capa) == std::string::npos)
				ind = 'X';
		}
		else { // assume ICAO format
			std::string acet;
			if (acinf.find('(') != std::string::npos && acinf.find(')') != std::string::npos) { // () in string, for TopSky. e.g. A321/L (SDE2E3FGIJ1RWY/H)
				int lc = acinf.find('(');
				int rc = acinf.find(')');
				acet = acinf.substr(lc + 1, rc - lc);
			}
			else { // no () in string, for erroneous simbrief prefile. e.g. A333/H-SDE3GHIJ2J3J5M1RVWXY/LB2D1
				acet = acinf.substr(acinf.find('-') + 1);
			}
			if (acet.substr(0, acet.find('/')).find('W') == std::string::npos)
				ind = 'X';
		}
		sprintf_s(sItemString, 2, "%c", ind);
		GetColorDefinition(SETTING_COLOR_RVSM_IND, pColorCode, pRGB);
		break;
	}
	case TAG_ITEM_TYPE_COMM_IND: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackedRecorder->IsCommEstablished(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 2, "C");
			*pColorCode = TAG_COLOR_REDUNDANT;
			GetColorDefinition(SETTING_COLOR_COMM_ESTAB, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_RECAT_BC: {
		if (!FlightPlan.IsValid()) break;
		if (FlightPlan.GetFlightPlanData().GetAircraftWtc() == 'H') {
			auto rc = m_ReCatMap.find(FlightPlan.GetFlightPlanData().GetAircraftFPType());
			if (rc != m_ReCatMap.end() && rc->second != 'J')
				sprintf_s(sItemString, 3, "-%c", rc->second);
		}
		break;
	}
	case TAG_TIEM_TYPE_RECAT_WTC: {
		if (!FlightPlan.IsValid()) break;
		auto rc = m_ReCatMap.find(FlightPlan.GetFlightPlanData().GetAircraftFPType());
		sprintf_s(sItemString, 2, "%c",
			rc != m_ReCatMap.end() ? rc->second : FlightPlan.GetFlightPlanData().GetAircraftWtc());
		break;
	}
	case TAG_ITEM_TYPE_RTE_CHECK: {
		if (!m_RouteChecker ||
			!FlightPlan.IsValid() ||
			FlightPlan.GetClearenceFlag())
			break;
		switch (m_RouteChecker->CheckFlightPlan(FlightPlan, false, false))
		{
		case RouteCheckerConstants::NOT_FOUND:
			sprintf_s(sItemString, 3, "? ");
			break;
		case RouteCheckerConstants::INVALID:
			sprintf_s(sItemString, 3, "X ");
			*pColorCode = TAG_COLOR_INFORMATION;
			GetColorDefinition(SETTING_COLOR_RC_INVALID, pColorCode, pRGB);
			break;
		case RouteCheckerConstants::PARTIAL_NO_LEVEL:
			sprintf_s(sItemString, 3, "PL");
			*pColorCode = TAG_COLOR_REDUNDANT;
			GetColorDefinition(SETTING_COLOR_RC_UNCERTN, pColorCode, pRGB);
			break;
		case RouteCheckerConstants::STRUCT_NO_LEVEL:
			sprintf_s(sItemString, 3, "YL");
			*pColorCode = TAG_COLOR_REDUNDANT;
			GetColorDefinition(SETTING_COLOR_RC_UNCERTN, pColorCode, pRGB);
			break;
		case RouteCheckerConstants::TEXT_NO_LEVEL:
			sprintf_s(sItemString, 3, "YL");
			break;
		case RouteCheckerConstants::PARTIAL_OK_LEVEL:
			sprintf_s(sItemString, 3, "P ");
			*pColorCode = TAG_COLOR_REDUNDANT;
			GetColorDefinition(SETTING_COLOR_RC_UNCERTN, pColorCode, pRGB);
			break;
		case RouteCheckerConstants::STRUCT_OK_LEVEL:
			sprintf_s(sItemString, 3, "Y ");
			*pColorCode = TAG_COLOR_REDUNDANT;
			GetColorDefinition(SETTING_COLOR_RC_UNCERTN, pColorCode, pRGB);
			break;
		case RouteCheckerConstants::TEXT_OK_LEVEL:
			sprintf_s(sItemString, 3, "Y ");
			break;
		default:
			break;
		}
		break;
	}
	case TAG_ITEM_TYPE_SQ_DUPE: {
		if (!FlightPlan.IsValid()) break;
		if (m_TrackedRecorder->IsSquawkDUPE(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 5, "DUPE");
			*pColorCode = TAG_COLOR_INFORMATION;
			GetColorDefinition(SETTING_COLOR_SQ_DUPE, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_DEP_SEQ: {
		if (!FlightPlan.IsValid()) break;
		if (!m_DepartureSequence) m_DepartureSequence = std::make_unique<DepartureSequence>();
		int seq = m_DepartureSequence->GetSequence(FlightPlan);
		if (seq > 0)
			sprintf_s(sItemString, 3, "%02d", OVRFLW2(seq));
		else if (seq < 0) { // reconnected
			sprintf_s(sItemString, 3, "--");
			*pColorCode = TAG_COLOR_INFORMATION;
			GetColorDefinition(SETTING_COLOR_DS_NUMBR, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_DEP_STS: {
		if (!FlightPlan.IsValid()) break;
		std::string gsts = FlightPlan.GetGroundState();
		if (gsts.size()) {
			sprintf_s(sItemString, gsts.size() + 1, gsts.c_str());
			if (!FlightPlan.GetClearenceFlag()) {
				*pColorCode = TAG_COLOR_INFORMATION;
				GetColorDefinition(SETTING_COLOR_DS_STATE, pColorCode, pRGB);
			}
		}
		else if (FlightPlan.GetClearenceFlag()) {
			sprintf_s(sItemString, 5, "CLRD");
		}
		break;
	}
	case TAG_ITEM_TYPE_RVEC_IND: {
		if (FlightPlan.IsValid() && FlightPlan.GetTrackingControllerIsMe() && FlightPlan.GetControllerAssignedData().GetAssignedHeading()) {
			sprintf_s(sItemString, 3, "RV");
			*pColorCode = TAG_COLOR_INFORMATION;
			GetColorDefinition(SETTING_COLOR_RDRV_IND, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_RCNT_IND: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackedRecorder->IsActive(FlightPlan)) {
			sprintf_s(sItemString, 2, "r");
			*pColorCode = TAG_COLOR_INFORMATION;
			GetColorDefinition(SETTING_COLOR_RECONT_IND, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_ASPD_BND: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetControllerAssignedData().GetAssignedSpeed() &&
			!FlightPlan.GetControllerAssignedData().GetAssignedMach())
			break; // not assigned
		std::string strip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(7); // on (2, 3) of annotation
		if (strip.find("/s+/") != std::string::npos) {
			sprintf_s(sItemString, 2, "+");
		}
		else if (strip.find("/s-/") != std::string::npos) {
			sprintf_s(sItemString, 2, "-");
		}
		break;
	}
	case TAG_ITEM_TYPE_UNIT_IND: {
		if (!RadarTarget.IsValid()) break;
		if (!m_TrackedRecorder->IsDifferentUnit(RadarTarget)) {
			strcpy_s(sItemString, 2, " ");
		}
		break;
	}
	default:
		break;
	}
}

void CMTEPlugIn::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	CRadarTarget RadarTarget = RadarTargetSelectASEL();
	CFlightPlan FlightPlan = FlightPlanSelectASEL();
	if (!RadarTarget.IsValid() && !FlightPlan.IsValid()) return;

	switch (FunctionId)
	{
	case TAG_ITEM_FUNCTION_COMM_ESTAB: {
		if (!FlightPlan.IsValid()) break;
		m_TrackedRecorder->SetCommEstablished(FlightPlan.GetCallsign());
		break;
	}
	case TAG_ITEM_FUNCTION_RCNT_RST: {
		if (!FlightPlan.IsValid()) break;
		m_TrackedRecorder->SetTrackedData(FlightPlan);
		break;
	}
	case TAG_ITEM_FUNCTION_VS_DISP: {
		if (!RadarTarget.IsValid() || GetPluginSetting(SETTING_VS_MODE, DEFAULT_VS_MODE) == -1) break;
		m_TrackedRecorder->ToggleVerticalSpeed(std::string(RadarTarget.GetSystemID()));
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_SET_MENU: {
		if (!FlightPlan.IsValid()) break;
		int tgtAlt = MetricAlt::GetAltitudeFromMenuItem(sItemString, !m_TrackedRecorder->IsForceFeet(FlightPlan));
		if (tgtAlt > MetricAlt::ALT_MAP_NOT_FOUND) {
			if (tgtAlt == 1 || tgtAlt == 2) { // ILS or VA
				FlightPlan.GetControllerAssignedData().SetAssignedHeading(0);
			}
			else if (tgtAlt > 2) {
				if (GetPluginSetting(SETTING_AMEND_CFL, DEFAULT_AMEND_CFL) == 1) {
					int trslvl, elev;
					std::string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
					tgtAlt += aptgt.size() && tgtAlt < trslvl ? elev : 0; // convert QNH to QFE
				}
			}
			FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow
		}
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_SET_EDIT: {
		if (!FlightPlan.IsValid() || !strlen(sItemString)) break;
		std::string input = MakeUpper(sItemString);
		if (input == "F") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan, true);
			break;
		}
		else if (input == "M") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan, false);
			break;
		}
		int tgtAlt = -1;
		// use regular expressions to match input
		std::regex rxfd("^F([0-9]+)\\.$");
		std::regex rxf("^F([0-9]+)$");
		std::regex rxd("^([0-9]+)\\.$");
		std::regex rxn("^([0-9]+)$");
		std::smatch match;
		if (regex_match(input, match, rxfd)) {
			tgtAlt = stoi(match[1]);
		}
		else if (regex_match(input, match, rxf)) {
			tgtAlt = stoi(match[1]) * 100;
		}
		else if (regex_match(input, match, rxd)) {
			tgtAlt = MetricAlt::LvlMtoFeet(stoi(match[1]));
		}
		else if (regex_match(input, match, rxn)) {
			tgtAlt = MetricAlt::LvlMtoFeet(stoi(match[1]) * 100);
		}
		if (tgtAlt > 2 && GetPluginSetting(SETTING_AMEND_CFL, DEFAULT_AMEND_CFL) == 1) {
			int trslvl, elev;
			std::string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
			tgtAlt += aptgt.size() && tgtAlt < trslvl ? elev : 0; // convert QNH to QFE
		}
		FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_MENU: {
		if (m_TrackedRecorder->ToggleAltitudeUnit(RadarTarget)) break;
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		else if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			// confirm previous CFL first
			m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign());
			break;
		}
		OpenPopupList(Area, "CFL Menu", 1);

		int copxAlt = FlightPlan.GetExitCoordinationAltitude();
		std::string copxName = FlightPlan.GetExitCoordinationPointName();
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int ref;
		int rdrAlt = m_TransitionLevel->GetRadarDisplayAltitude(RadarTarget, ref);
		int trslvl, elev;
		m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
		auto m_alt = MetricAlt::GetMenuItems(!m_TrackedRecorder->IsForceFeet(FlightPlan), trslvl);
		if (elev) { // QFE in use
			cflAlt -= cflAlt > 2 && cflAlt < trslvl ? elev : 0;
			// remove unavailable altitudes from list
			std::erase_if(m_alt, [&](const auto& i) { return i.altitude < trslvl && i.altitude + elev >= trslvl; });
		}
		int cmpAlt = cflAlt <= 2 ? rdrAlt : cflAlt;
		if (copxAlt > 0 && copxAlt != FlightPlan.GetFinalAltitude()) {
			// known: COPX altitude is associated with a given route point
			auto ExtractedRoute = FlightPlan.GetExtractedRoute();
			int calIndex = ExtractedRoute.GetPointsCalculatedIndex();
			int assIndex = ExtractedRoute.GetPointsAssignedIndex();
			int nextIndex = assIndex > -1 ? assIndex : calIndex;
			if (nextIndex > -1 && nextIndex < ExtractedRoute.GetPointsNumber()) { // next point is valid
				double dtRun = 0;
				CPosition prevPos = FlightPlan.GetFPTrackPosition().GetPosition();
				int i = nextIndex;
				for (; i < ExtractedRoute.GetPointsNumber(); i++) {
					double nextD = prevPos.DistanceTo(ExtractedRoute.GetPointPosition(i));
					dtRun += nextD;
					prevPos = ExtractedRoute.GetPointPosition(i);
					std::string nextName = ExtractedRoute.GetPointName(i);
					int nextAlt = ExtractedRoute.GetPointCalculatedProfileAltitude(i);
					if (copxName == nextName && copxAlt == nextAlt) {
						double dtReq = abs((double)(rdrAlt - copxAlt)) * 0.004713;
						// altitude (feet) to distance (nautical miles), descent angle 2 deg
						// 1 / tan(2.0 / 180.0 * 3.141593) / 1000.0 * 0.164579 = 0.004713
						if (dtReq + 20 >= dtRun) { // 20 more nautical miles
							cmpAlt = copxAlt;
						}
						break;
					}
				}
			}
		}
		// pre-select altitude
		int minAlt = std::ranges::min_element(m_alt, {}, [&](const auto& a) { return abs(a.altitude - cmpAlt); })->altitude;
		for (auto it = m_alt.rbegin(); it != m_alt.rend(); it++) {
			if (it->altitude > 3) { // not NONE, ILS, VA, EDIT
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_CFL_SET_MENU, it->altitude == minAlt, POPUP_ELEMENT_NO_CHECKBOX, false, false);
			}
			else if (it->altitude == 3) { // EDIT
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_CFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
			}
			else { // ILS, VA, NONE
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_CFL_SET_MENU, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
			}
		}
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_EDIT: {
		if (m_TrackedRecorder->ToggleAltitudeUnit(RadarTarget)) break;
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		else if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			// confirm previous CFL first
			m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign());
			break;
		}
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_CFL_SET_EDIT, "");
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_TOPSKY: {
		if (m_TrackedRecorder->ToggleAltitudeUnit(RadarTarget)) break;
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			// confirm previous CFL first
			m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign());
			break;
		}
		CallItemFunction(FlightPlan.GetCallsign(), nullptr, 0, sItemString, "TopSky plugin", 12, Pt, Area);
		// 12 is learned from tag settings, that line may look like:
		//  TAGITEM:6::0:0:12:12:MTEPlugin:0:1:TopSky plugin:MTEPlugin:3
		// ________________^^__________________^^^^^^^^^^^^^____________
		break;
	}
	case TAG_ITEM_FUNCTION_RFL_SET_MENU: {
		if (!FlightPlan.IsValid()) break;
		int tgtAlt = MetricAlt::GetAltitudeFromMenuItem(sItemString, !m_TrackedRecorder->IsForceFeet(FlightPlan));
		if (tgtAlt > MetricAlt::ALT_MAP_NOT_FOUND) {
			FlightPlan.GetControllerAssignedData().SetFinalAltitude(tgtAlt);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_RFL_SET_EDIT: {
		if (!FlightPlan.IsValid() || !strlen(sItemString)) break;
		CFlightPlanControllerAssignedData ctrData = FlightPlan.GetControllerAssignedData();
		std::string input = MakeUpper(sItemString);
		if (input == "F") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan, true);
			break;
		}
		else if (input == "M") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan, false);
			break;
		}
		std::regex rxm("^([0-9]{1,3})$");
		std::regex rxf("^F([0-9]{1,3})$");
		std::smatch match;
		if (regex_match(input, match, rxf)) {
			ctrData.SetFinalAltitude(stoi(match[1]) * 100);
		}
		else if (regex_match(input, match, rxm)) {
			ctrData.SetFinalAltitude(MetricAlt::LvlMtoFeet(stoi(match[1]) * 100));
		}
		break;
	}
	case TAG_ITEM_FUNCTION_RFL_MENU: {
		if (m_TrackedRecorder->ToggleAltitudeUnit(RadarTarget)) break;
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupList(Area, "RFL Menu", 1);
		// pre-select altitude
		int rflAlt = FlightPlan.GetFinalAltitude();
		auto m_alt = MetricAlt::GetMenuItems(!m_TrackedRecorder->IsForceFeet(FlightPlan), 0);
		int minAlt = std::ranges::min_element(m_alt, {}, [&](const auto& a) { return abs(a.altitude - rflAlt); })->altitude;
		for (auto it = m_alt.rbegin(); it != m_alt.rend(); it++) {
			if (it->altitude > 3) {
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_RFL_SET_MENU, it->altitude == minAlt, POPUP_ELEMENT_NO_CHECKBOX, false, false);
			}
			else if (it->altitude == 3) { // EDIT
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_RFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
			}
		}
		break;
	}
	case TAG_ITEM_FUNCTION_RFL_EDIT: {
		if (m_TrackedRecorder->ToggleAltitudeUnit(RadarTarget)) break;
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_RFL_SET_EDIT, "");
		break;
	}
	case TAG_ITEM_FUNCTION_SC_LIST: {
		if (!FlightPlan.IsValid()) break;
		std::string cs = FlightPlan.GetCallsign();
		if (!m_TrackedRecorder->IsSimilarCallsign(cs)) // not a SC
			break;
		OpenPopupList(Area, "SC List", 1);
		AddPopupListElement(cs.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT, true);
		auto cset = m_TrackedRecorder->GetSimilarCallsigns(cs);
		for (auto& c : cset) {
			AddPopupListElement(c.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_SC_SELECT: {
		if (pluginWindow != nullptr) {
			HWND editWnd = FindWindowEx(pluginWindow, nullptr, "Edit", nullptr);
			if (editWnd != nullptr)
				SendMessage(editWnd, WM_SETTEXT, NULL, (LPARAM)(LPCSTR)".find ");
		}
		SetASELAircraft(FlightPlanSelect(sItemString));
		break;
	}
	case TAG_ITEM_FUNCTION_RTE_INFO: {
		if (!m_RouteChecker ||
			!FlightPlan.IsValid() ||
			FlightPlan.GetClearenceFlag())
			break;
		int rc = m_RouteChecker->CheckFlightPlan(FlightPlan, true, false); // force a refresh here to avoid error
		if (rc == RouteCheckerConstants::NOT_FOUND) break;
		DisplayRouteMessage(FlightPlan.GetFlightPlanData().GetOrigin(), FlightPlan.GetFlightPlanData().GetDestination());
		break;
	}
	case TAG_ITEM_FUNCTION_DSQ_MENU: {
		if (!FlightPlan.IsValid()) break;
		if (!m_DepartureSequence) break;
		int seq = m_DepartureSequence->GetSequence(FlightPlan);
		if (seq <= 0) { // reconnected or completely new
			m_DepartureSequence->AddFlight(FlightPlan);
		}
		else {
			OpenPopupEdit(Area, TAG_ITEM_FUNCTION_DSQ_EDIT, "");
		}
		break;
	}
	case TAG_ITEM_FUNCTION_DSQ_EDIT: {
		if (!FlightPlan.IsValid()) break;
		int seq;
		if (sscanf_s(sItemString, "%d", &seq) != 1) break;
		if (seq >= 0) {
			m_DepartureSequence->EditSequence(FlightPlan, seq);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_DSQ_STS: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetClearenceFlag()) {
			// set cleared flag
			CallItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_CLEARED_FLAG, Pt, Area);
		}
		else {
			// set status
			CallItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_GROUND_STATUS, Pt, Area);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_SPD_SET: {
		if (!FlightPlan.IsValid()) break;
		if (!strcmp(sItemString, "----"))
			FlightPlan.GetControllerAssignedData().SetAssignedSpeed(0);
		float m;
		int s;
		if (sscanf_s(sItemString, "M%f", &m) == 1) { // MACH
			FlightPlan.GetControllerAssignedData().SetAssignedMach((int)round(m * 100.0));
		}
		else if (sscanf_s(sItemString, "N%d", &s) == 1) { // IAS
			FlightPlan.GetControllerAssignedData().SetAssignedSpeed(s);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_SPD_LIST: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		int altref;
		int alt = m_TransitionLevel->GetRadarDisplayAltitude(RadarTarget, altref);
		if (MetricAlt::FeettoM(alt) <= 7530 || altref != AltitudeReference::ALT_REF_QNE) { // use IAS
			int aspd = FlightPlan.GetControllerAssignedData().GetAssignedSpeed();
			OpenPopupList(Area, "IAS", 2);
			for (int s = 320; s >= 160; s -= 10) {
				int k = (int)round(KN_KPH((double)s));
				char ias[5], kph[6];
				sprintf_s(ias, 5, "N%d", s);
				sprintf_s(kph, 6, " K%d", k);
				AddPopupListElement(ias, kph, TAG_ITEM_FUNCTION_SPD_SET, aspd ? aspd == s : s == 240);
			}
			AddPopupListElement("----", " ----", TAG_ITEM_FUNCTION_SPD_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		}
		else { // use MACH
			int amac = FlightPlan.GetControllerAssignedData().GetAssignedMach();
			OpenPopupList(Area, "MACH", 1);
			AddPopupListElement("M2.0", nullptr, TAG_ITEM_FUNCTION_SPD_SET);
			for (int m = 90; m >= 65; m -= 1) {
				char mc[5];
				sprintf_s(mc, 5, "M.%d", m);
				AddPopupListElement(mc, nullptr, TAG_ITEM_FUNCTION_SPD_SET, amac ? amac == m : m == 80);
			}
			AddPopupListElement("----", nullptr, TAG_ITEM_FUNCTION_SPD_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_UNIT_MENU: {
		bool isfeet = m_TrackedRecorder->IsForceFeet(FlightPlan) || m_TrackedRecorder->IsForceFeet(RadarTarget);
		bool isknot = m_TrackedRecorder->IsForceKnot(RadarTarget);
		bool suppvs = GetPluginSetting(SETTING_VS_MODE, DEFAULT_VS_MODE) == -1;
		bool showvs = m_TrackedRecorder->IsDisplayVerticalSpeed(RadarTarget.IsValid() ? RadarTarget.GetSystemID() : "");
		OpenPopupList(Area, "Units", 2);
		std::vector<std::string> unitvec = {
			std::format("{}{}","ALT:", isfeet ? "F" : "M"),
			std::format("{}{}","SPD:" , isknot ? "S" : "K"),
			std::format("{}{}","VS :" , suppvs ? "-" : (showvs ? "O" : "X")),
		}; // fix length=5
		for (const auto& s : unitvec) {
			AddPopupListElement(s.c_str(), "", TAG_ITEM_FUNCTION_UNIT_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		break;
	}
	case TAG_ITEM_FUNCTION_UNIT_SET: {
		std::string type = sItemString;
		char c = type.at(4);
		if (type.substr(0, 3) == "ALT") {
			if (RadarTarget.IsValid()) {
				m_TrackedRecorder->SetAltitudeUnit(RadarTarget, c != 'F');
			}
			else if (FlightPlan.IsValid()) {
				m_TrackedRecorder->SetAltitudeUnit(FlightPlan, c != 'F');
			}
		}
		else if (type.substr(0, 3) == "SPD") {
			if (RadarTarget.IsValid()) {
				m_TrackedRecorder->SetSpeedUnit(RadarTarget, c != 'S');
			}
		}
		else if (type.substr(0, 3) == "VS ") {
			if (RadarTarget.IsValid() && GetPluginSetting(SETTING_VS_MODE, DEFAULT_VS_MODE) != -1) {
				m_TrackedRecorder->ToggleVerticalSpeed(std::string(RadarTarget.GetSystemID()));
			}
		}
		break;
	}
	default:
		break;
	}
}

void CMTEPlugIn::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
	if (!FlightPlan.IsValid())
		return;
	if (DataType == CTR_DATA_TYPE_TEMPORARY_ALTITUDE) {
		if (GetPluginSetting(SETTING_AMEND_CFL, DEFAULT_AMEND_CFL) == 2) { // amend only when mode 2 (all) is set
			int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
			if (cflAlt > 2) { // exclude ILS, VA, NONE
				int trslvl, elev;
				std::string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
				if (aptgt.size() && cflAlt < trslvl && elev > 0) {
					static bool suppress = false;
					if (!suppress) {
						suppress = true;
						cflAlt += elev; // convert QNH to QFE
						FlightPlan.GetControllerAssignedData().SetClearedAltitude(cflAlt);
						suppress = false;
						return;
					}
				}
			}
		}
		if (FlightPlan.GetTrackingControllerIsMe()) {
			if (FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetTransponderC() &&
				(IsCFLAssigned(FlightPlan) || FlightPlan.GetControllerAssignedData().GetClearedAltitude())) {
				// initiate CFL to be confirmed
				m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign(), false);
			}
		}
	}
	if (m_RouteChecker &&
		(DataType == CTR_DATA_TYPE_FINAL_ALTITUDE && !FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetTransponderC())) {
		m_RouteChecker->CheckFlightPlan(FlightPlan, true, true);
	}
	if (m_DepartureSequence &&
		(DataType == CTR_DATA_TYPE_GROUND_STATE || DataType == CTR_DATA_TYPE_CLEARENCE_FLAG)) {
		m_DepartureSequence->EditSequence(FlightPlan, 0);
	}
	if (DataType == CTR_DATA_TYPE_GROUND_STATE &&
		FlightPlan.GetClearenceFlag() &&
		!strlen(FlightPlan.GetGroundState())) {
		CallItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_CLEARED_FLAG, POINT(), RECT());
	}
	m_TrackedRecorder->UpdateFlight(FlightPlan);
}

void CMTEPlugIn::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid())
		return;
	if (m_RouteChecker)
		m_RouteChecker->RemoveCache(FlightPlan);
	if (m_DepartureSequence)
		m_DepartureSequence->EditSequence(FlightPlan, -1);
	if (FlightPlan.GetTrackingControllerIsMe())
		m_TrackedRecorder->UpdateFlight(FlightPlan, false);
}

void CMTEPlugIn::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid())
		return;
	if (m_RouteChecker) {
		m_RouteChecker->CheckFlightPlan(FlightPlan, true, true);
	}
}

void CMTEPlugIn::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	if (!RadarTarget.IsValid())
		return;
	if (m_TrackedRecorder->IsActive(RadarTarget)) {
		m_TrackedRecorder->UpdateFlight(RadarTarget);
	}
	else {
		int retrack = GetPluginSetting(SETTING_AUTO_RETRACK, DEFAULT_AUTO_RETRACK);
		if (retrack == 1 || retrack == 2) {
			if (m_TrackedRecorder->SetTrackedData(RadarTarget) && retrack == 2) {
				std::string msg = std::string(RadarTarget.GetCallsign()) + " reconnected and is re-tracked.";
				DisplayUserMessage("MTEP-Recorder", "MTEPlugin", msg.c_str(), 1, 1, 0, 0, 0);
			}
		}
	}
	m_TransitionLevel->UpdateRadarPosition(RadarTarget);
}

CRadarScreen* CMTEPlugIn::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	auto screen = std::make_shared<CMTEPScreen>();
	m_ScreenStack.push(screen);
	return screen.get();
}

bool CMTEPlugIn::OnCompileCommand(const char* sCommandLine)
{
	std::string cmd = sCommandLine;
	std::smatch match; // all regular expressions will ignore cases

	// custom cursor
	std::regex rxcc("^.MTEP CURSOR (ON|OFF)$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxcc)) {
		CancelCustomCursor();
		bool cc = MakeUpper(match[1].str()) == "ON";
		if (cc)
			SetCustomCursor();
		SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", cc ? "1" : "0");
		return true;
	}

	// flightradar24 and variflight
	std::regex rxfr("^.MTEP (FR24|VARI) ([A-Z]{4})$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxfr)) {
		std::string airport = MakeUpper(match[2].str());
		CSectorElement se = SectorFileElementSelectFirst(SECTOR_ELEMENT_AIRPORT);
		for (; se.IsValid() && airport != se.GetName();
			se = SectorFileElementSelectNext(se, SECTOR_ELEMENT_AIRPORT));
		CPosition pos;
		if (se.GetPosition(&pos, 0)) {
			std::string url_full = MakeUpper(match[1].str()) == "FR24" ? \
				"https://www.flightradar24.com/" + std::to_string(pos.m_Latitude) + "," + std::to_string(pos.m_Longitude) + "/9":\
				"https://flightadsb.variflight.com/tracker/" + std::to_string(pos.m_Longitude) + "," + std::to_string(pos.m_Latitude) + "/9";
			ShellExecute(NULL, "open", url_full.c_str(), NULL, NULL, SW_SHOW);
			return true;
		}
	}

	// load route checker
	std::regex rxrc("^.MTEP RC RESET$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxrc)) {
		LoadRouteChecker();
		return true;
	}

	// route checker get route info
	std::regex rxrcod("^.MTEP RC ([A-Z]{4}) ([A-Z]{4})$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxrcod)) {
		std::string msg = DisplayRouteMessage(MakeUpper(match[1].str()), MakeUpper(match[2].str()));
		if (msg.size()) {
			// clipboard operation
			HGLOBAL hGlobal;
			size_t bSize = msg.size() + 1;
			hGlobal = GlobalAlloc(GPTR, bSize);
			memcpy_s(hGlobal, bSize, msg.c_str(), bSize);
			OpenClipboard(nullptr);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hGlobal);
			CloseClipboard();
		}
		return msg.size();
	}

	// delete departure sequence
	std::regex rxds("^.MTEP DS RESET$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxds)) {
		ResetDepartureSequence();
		return true;
	}

	// reset tracked recorder
	std::regex rxtr("^.MTEP TR RESET$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxtr)) {
		ResetTrackedRecorder();
		return true;
	}

	// set auto retrack
	std::regex rxtrrt("^.MTEP TR ([0-2])$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxtrrt)) {
		std::string res = match[1].str();
		const char* descr = "auto retrack mode";
		if (res == "1") {
			SaveDataToSettings(SETTING_AUTO_RETRACK, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack mode 1 (silent) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "2") {
			SaveDataToSettings(SETTING_AUTO_RETRACK, descr, "2");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack mode 2 (notify) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "0") {
			SaveDataToSettings(SETTING_AUTO_RETRACK, descr, "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack is off", 1, 0, 0, 0, 0);
		}
		return true;
	}

	// set force feet and force knot
	std::regex rxff("^.MTEP TR (F|M|S|K)$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxff)) {
		std::string res = MakeUpper(match[1].str());
		const char* descr = "force feet";
		if (res == "F") {
			m_TrackedRecorder->ResetAltitudeUnit(true);
			SaveDataToSettings(SETTING_ALT_FEET, "force feet", "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Altitude unit is set to feet", 1, 0, 0, 0, 0);
		}
		else if (res == "M") {
			m_TrackedRecorder->ResetAltitudeUnit(false);
			SaveDataToSettings(SETTING_ALT_FEET, "force feet", "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Altitude unit is set to meter", 1, 0, 0, 0, 0);
		}
		else if (res == "S") {
			m_TrackedRecorder->SetSpeedUnit(true);
			SaveDataToSettings(SETTING_GS_KNOT, "force knot", "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Speed unit is set to KTS", 1, 0, 0, 0, 0);
		}
		else if (res == "K") {
			m_TrackedRecorder->SetSpeedUnit(false);
			SaveDataToSettings(SETTING_GS_KNOT, "force knot", "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Speed unit is set to KPH", 1, 0, 0, 0, 0);
		}
		return true;
	}

	// set vertical speed display
	std::regex rxvs("^.MTEP VS (AUTO|ON|OFF)$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxvs)) {
		std::string res = MakeUpper(match[1].str());
		const char* descr = "display vertical speed";
		if (res == "AUTO") {
			m_TrackedRecorder->ToggleVerticalSpeed(false);
			SaveDataToSettings(SETTING_VS_MODE, descr, "-1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Global vertical speed display is AUTO", 1, 0, 0, 0, 0);
		}
		else if (res == "ON") {
			m_TrackedRecorder->ToggleVerticalSpeed(true);
			SaveDataToSettings(SETTING_VS_MODE, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Global vertical speed display is ON", 1, 0, 0, 0, 0);
		}
		else if (res == "OFF") {
			m_TrackedRecorder->ToggleVerticalSpeed(false);
			SaveDataToSettings(SETTING_VS_MODE, descr, "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Global vertical speed display is OFF", 1, 0, 0, 0, 0);
		}
		return true;
	}

	// load transition level
	std::regex rxtl("^.MTEP TL RESET$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxtl)) {
		LoadTransitionLevel();
		return true;
	}

	// set transition level for single airport
	std::regex rxtlt("^.MTEP ([A-Z]{4}) TL (S|F)(\\d+)$", std::regex_constants::icase);
	std::regex rxtlb("^.MTEP ([A-Z]{4}) (QNH|QFE)$", std::regex_constants::icase);
	std::regex rxtlr("^.MTEP ([A-Z]{4}) R (\\d+)$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxtlt)) {
		std::string airport = MakeUpper(match[1].str());
		int tl = stoi(MakeUpper(match[3].str())) * 100;
		if (MakeUpper(match[2].str()) == "S") {
			tl = MetricAlt::LvlMtoFeet(tl);
		}
		if (m_TransitionLevel->SetAirportParam(airport, tl, -1, -1)) {
			std::string msg = airport + " - transition level is set to " + std::to_string(tl) + " ft.";
			DisplayUserMessage("MESSAGE", "MTEPlugin", msg.c_str(), 1, 0, 0, 0, 0);
			return true;
		}
	}
	if (regex_match(cmd, match, rxtlb)) {
		std::string airport = MakeUpper(match[1].str());
		bool qfe = MakeUpper(match[2].str()) == "QFE";
		if (m_TransitionLevel->SetAirportParam(airport, -1, qfe, -1)) {
			std::string msg = airport + " - altitude reference is set to " + std::string(qfe ? "QFE." : "QNH.");
			DisplayUserMessage("MESSAGE", "MTEPlugin", msg.c_str(), 1, 0, 0, 0, 0);
			return true;
		}
	}
	if (regex_match(cmd, match, rxtlr)) {
		std::string airport = MakeUpper(match[1].str());
		int r = stoi(match[2].str());
		if (m_TransitionLevel->SetAirportParam(airport, -1, -1, r)) {
			std::string msg = airport + " - QNH/QFE range is set to " + match[2].str() + " miles.";
			DisplayUserMessage("MESSAGE", "MTEPlugin", msg.c_str(), 1, 0, 0, 0, 0);
			return true;
		}
	}

	// load MetricAlt settings
	std::regex rxma("^.MTEP MA RESET$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxma)) {
		LoadMetricAltitude();
		return true;
	}

	// set amend QFE in CFL
	std::regex rxac("^.MTEP QFE ([0-2])$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxac)) {
		std::string res = match[1].str();
		const char* descr = "amend QFE in CFL";
		if (res == "1") {
			SaveDataToSettings(SETTING_AMEND_CFL, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL mode 1 (MTEP) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "2") {
			SaveDataToSettings(SETTING_AMEND_CFL, descr, "2");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL mode 2 (all) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "0") {
			SaveDataToSettings(SETTING_AMEND_CFL, descr, "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL is off", 1, 0, 0, 0, 0);
		}
		return true;
	}

	return false;
}

template<typename T>
inline T CMTEPlugIn::GetPluginSetting(const char* setting, const T& fallback)
{
	// only accept 0/1 for bool
	T bufval = fallback;
	auto data = GetDataFromSettings(setting);
	if (data != nullptr) {
		std::stringstream ss(data);
		ss >> bufval;
		if (ss.fail()) {
			bufval = fallback;
		}
	}
	return bufval;
}

inline int CMTEPlugIn::CalculateVerticalSpeed(CRadarTarget RadarTarget, bool rounded)
{
	// if rounded = true, will use setting from SETTING_VS_ROUND, default is 1 (no rounding), will ensure >=1
	if (!RadarTarget.IsValid()) return 0;
	auto curpos = RadarTarget.GetPosition();
	auto prepos = RadarTarget.GetPreviousPosition(curpos);
	double deltaA = curpos.GetFlightLevel() - prepos.GetFlightLevel();
	double deltaT = GetLastRadarInterval(curpos, prepos);
	int vs = (int)round(deltaA / deltaT * 60.0);
	if (rounded) {
		int rnd = abs(GetPluginSetting(SETTING_VS_RNDG, DEFAULT_VS_RNDG));
		if (rnd > 1) {
			int rem1 = vs % rnd;
			int rem2 = rnd - rem1;
			vs += rem1 <= rem2 ? -rem1 : rem2;
		}
	}
	return vs;
}

void CMTEPlugIn::CallItemFunction(const char* sCallsign, const int& FunctionId, const POINT& Pt, const RECT& Area)
{
	return CallItemFunction(sCallsign, nullptr, 0, nullptr, nullptr, FunctionId, Pt, Area);
}

void CMTEPlugIn::CallItemFunction(const char* sCallsign, const char* sItemPlugInName, int ItemCode, const char* sItemString, const char* sFunctionPlugInName, int FunctionId, POINT Pt, RECT Area)
{
	while (!m_ScreenStack.empty()) {
		auto& s = m_ScreenStack.top();
		if (s->m_Opened)
			return s->StartTagFunction(sCallsign, sItemPlugInName, ItemCode, sItemString, sFunctionPlugInName, FunctionId, Pt, Area);
		else {
			s.reset();
			m_ScreenStack.pop();
		}
	}
}

void CMTEPlugIn::GetColorDefinition(const char* setting, int* pColorCode, COLORREF* pRGB)
{
	// If setting is not present or invalid, it will not touch anything
	unsigned int r(256), g(256), b(256);
	auto settingValue = GetDataFromSettings(setting);
	if (settingValue != nullptr && sscanf_s(settingValue, "%u:%u:%u", &r, &g, &b) != 3)
		return;
	if (r <= 255 && g <= 255 && b <= 255) {
		*pColorCode = TAG_COLOR_RGB_DEFINED;
		*pRGB = RGB(r, g, b);
	}
}

void CMTEPlugIn::SetCustomCursor(void)
{
	if (m_CustomCursor || pluginWindow == nullptr || pluginCursor == nullptr) return;
	prevWndFunc = (WNDPROC)SetWindowLong(pluginWindow, GWL_WNDPROC, (LONG)WindowProc);
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Cursor is set!", 1, 0, 0, 0, 0);
	m_CustomCursor = true;
}

void CMTEPlugIn::CancelCustomCursor(void)
{
	if (!m_CustomCursor || pluginWindow == nullptr || pluginCursor == nullptr) return;
	SetWindowLong(pluginWindow, GWL_WNDPROC, (LONG)prevWndFunc);
	m_CustomCursor = false;
}

void CMTEPlugIn::LoadRouteChecker(void)
{
	auto setfn = GetDataFromSettings(SETTING_ROUTE_CHECKER_CSV);
	if (setfn == nullptr) {
		m_RouteChecker.reset();
		return;
	}
	std::string fn = GetRealFileName(setfn);
	try {
		m_RouteChecker.reset(new RouteChecker(this, fn));
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route checker is loaded successfully. CSV file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route checker failed to load (" + e + "). CSV file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_RouteChecker.reset();
	}
	catch (std::exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route checker failed to load (" + std::string(e.what()) + "). CSV file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_RouteChecker.reset();
	}
}

void CMTEPlugIn::ResetDepartureSequence(void)
{
	m_DepartureSequence.reset();
}

void CMTEPlugIn::ResetTrackedRecorder(void)
{
	m_TrackedRecorder.reset(new TrackedRecorder(this));
	bool setff = GetPluginSetting(SETTING_ALT_FEET, DEFAULT_ALT_FEET);
	m_TrackedRecorder->ResetAltitudeUnit(setff);
	bool setfn = GetPluginSetting(SETTING_GS_KNOT, DEFAULT_GS_KNOT);
	m_TrackedRecorder->SetSpeedUnit(setfn);
	int setvs = GetPluginSetting(SETTING_VS_MODE, DEFAULT_VS_MODE);
	m_TrackedRecorder->ToggleVerticalSpeed(setvs == 1);
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Tracked recorder is ready!", 1, 0, 0, 0, 0);
}

void CMTEPlugIn::LoadTransitionLevel(void)
{
	m_TransitionLevel.reset(new TransitionLevel(this));
	auto setfn = GetDataFromSettings(SETTING_TRANS_LVL_CSV);
	if (setfn == nullptr) {
		return;
	}
	std::string fn = GetRealFileName(setfn);
	try {
		m_TransitionLevel->LoadCSV(fn);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Transition levels are loaded successfully. CSV file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Transition levels failed to load (" + e + "). CSV file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_TransitionLevel.reset(new TransitionLevel(this));
	}
	catch (std::exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Transition levels failed to load (" + std::string(e.what()) + "). CSV file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_TransitionLevel.reset(new TransitionLevel(this));
	}
}

void CMTEPlugIn::LoadMetricAltitude(void)
{
	auto setfn = GetDataFromSettings(SETTING_TRANS_MALT_TXT);
	std::string fn = GetRealFileName(setfn == nullptr ? "" : setfn);
	try {
		MetricAlt::LoadAltitudeDefinition(fn);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Altitude menu definitions are loaded successfully. TXT file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Altitude menu definitions failed to load (" + e + "). TXT file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Altitude menu definitions failed to load (" + std::string(e.what()) + "). TXT file name: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
}

std::string CMTEPlugIn::DisplayRouteMessage(const std::string& departure, const std::string& arrival)
{
	if (!m_RouteChecker) return "";
	auto rinfo = m_RouteChecker->GetRouteInfo(departure, arrival);
	if (!rinfo.size()) return "";
	std::string res = departure + "-" + arrival;
	DisplayUserMessage("MTEP-Route", res.c_str(), (std::to_string(rinfo.size()) + " route(s):").c_str(), 1, 1, 0, 0, 0);
	for (const auto& ri : rinfo) {
		DisplayUserMessage("MTEP-Route", nullptr, ri.c_str(), 1, 0, 0, 0, 0);
		res += "\n" + ri;
	}
	return res;
}

inline std::string MakeUpper(const std::string& str)
{
	std::string res;
	std::transform(str.begin(), str.end(), back_inserter(res), ::toupper);
	return res;
}

inline bool IsCFLAssigned(CFlightPlan FlightPlan)
{
	// tell when cleared altitude is 0, is CFL not assigned or assigned to RFL
	// true means CFL is assigned to RFL, false means no CFL
	if (FlightPlan.IsValid() && strlen(FlightPlan.GetTrackingControllerCallsign())) { // tracked by someone
		CRadarTarget RadarTarget = FlightPlan.GetCorrelatedRadarTarget();
		if (RadarTarget.IsValid()) {
			std::string squawk = RadarTarget.GetPosition().GetSquawk();
			if (!(squawk == "7700" || squawk == "7600" || squawk == "7500" || !RadarTarget.GetPosition().GetTransponderC()))
				return true;
		}
	}
	return false;
}

inline int GetLastRadarInterval(CRadarTargetPositionData pos1, CRadarTargetPositionData pos2)
{
	// work-around for 4/6 second interval and prevents divided by 0
	int t = abs(pos1.GetReceivedTime() - pos2.GetReceivedTime());
	if (t > 0) {
		if (t >= 4 && t <= 6) {
			t = 5;
		}
	}
	else {
		t = 1;
	}
	return t;
}

inline std::string GetRealFileName(const std::string& path)
{
	namespace fs = std::filesystem;
	if (path.empty()) {
		return path;
	}
	fs::path pFilename = path;
	if (!fs::is_regular_file(pFilename)) {
		// add DLL directory before relative path
		if (pluginModule != nullptr) {
			TCHAR pBuffer[MAX_PATH] = { 0 };
			GetModuleFileName(pluginModule, pBuffer, sizeof(pBuffer) / sizeof(TCHAR) - 1);
			std::filesystem::path dllPath = pBuffer;
			pFilename = dllPath.parent_path() / path;
		}
	}
	return pFilename.string();
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwTarProcessId = *((DWORD*)lParam);
	DWORD dwEnumProcessId = 0;
	GetWindowThreadProcessId(hwnd, &dwEnumProcessId);
	const char* tarWindowText = "EuroScope v3.2";
	char enumWindowtext[15] = {};
	GetWindowText(hwnd, enumWindowtext, 15);
	if (!strcmp(enumWindowtext, tarWindowText) && dwTarProcessId == dwEnumProcessId) {
		pluginWindow = hwnd;
		return false;
	}
	return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SETCURSOR: {
		SetCursor(pluginCursor);
		return true;
	}
	default:
		return CallWindowProc(prevWndFunc, hwnd, uMsg, wParam, lParam);
	}
}
