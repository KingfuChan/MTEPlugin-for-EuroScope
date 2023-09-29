// MTEPlugin.cpp

#include "pch.h"
#include "Version.h"
#include "MTEPlugin.h"

#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2022 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugin-for-EuroScope"
#endif // !COPYRIGHTS

// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1; // GS(KPH) with trend indicator
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_FPM = 3; // Vertical speed (4-digit FPM)
const int TAG_ITEM_TYPE_LVL_IND = 4; // Climb/Descend/Level indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // Actual altitude (m)
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
const int TAG_TIEM_TYPE_DEP_STS = 19; // Departure status
const int TAG_TIEM_TYPE_RECAT_WTC = 20; // RECAT-CN (LMCBJ)
const int TAG_ITEM_TYPE_ASPD_BND = 21; // Assigned speed bound (Topsky, +/-)

// TAG ITEM FUNCTION
const int TAG_ITEM_FUNCTION_COMM_ESTAB = 1; // Set COMM ESTB
const int TAG_ITEM_FUNCTION_RCNT_RST = 2; // Restore assigned data
const int TAG_ITEM_FUNCTION_CFL_SET_EDIT = 10; // Set CFL from edit (not registered)
const int TAG_ITEM_FUNCTION_CFL_MENU = 11; // Open CFL popup menu
const int TAG_ITEM_FUNCTION_CFL_EDIT = 12; // Open CFL popup edit
const int TAG_ITEM_FUNCTION_CFL_SET_MENU = 13; // Set CFL from menu (not registered)
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

// COMPUTERISING RELATED
constexpr double KN_KPH(double k) { return 1.85184 * k; } // 1 knot = 1.85184 kph
constexpr double KPH_KN(double k) { return k / 1.85184; } // 1.85184 kph = 1 knot
constexpr int OVRFLW2(int t) { return abs(t) > 99 ? 99 : abs(t); } // overflow pre-process 2 digits
constexpr int OVRFLW3(int t) { return abs(t) > 999 ? 999 : abs(t); } // overflow pre-process 3 digits
constexpr int OVRFLW4(int t) { return abs(t) > 9999 ? 9999 : abs(t); }  // overflow pre-process 4 digits
string MakeUpper(string str);
string GetAbsolutePath(string relativePath);
int CalculateVerticalSpeed(CRadarTarget RadarTarget);
bool IsCFLAssigned(CFlightPlan FlightPlan);

// SETTING NAMES
const char* SETTING_CUSTOM_CURSOR = "CustomCursor";
const char* SETTING_ROUTE_CHECKER_CSV = "RteCheckerCSV";
const char* SETTING_AUTO_RETRACK = "AutoRetrack";
const char* SETTING_CUSTOM_NUMBER_MAP = "CustomNumber0-9";
const char* SETTING_TRANS_LVL_CSV = "TransLevelCSV";
const char* SETTING_TRANS_MALT_TXT = "MetricAltitudeTXT";
const char* SETTING_AMEND_CFL = "AmendQFEinCFL";

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

	const char* setcc = GetDataFromSettings(SETTING_CUSTOM_CURSOR);
	UINT dpiSys = GetDpiForWindow(GetDesktopWindow());
	UINT dpiWnd = GetDpiForWindow(pluginWindow);
	int curSize = (int)((float)(dpiSys < 96 ? 96 : dpiSys) / (float)(dpiWnd < 96 ? 96 : dpiWnd) * 32.0);
	pluginCursor = CopyCursor(LoadImage(pluginModule, MAKEINTRESOURCE(IDC_CURSORCROSS), IMAGE_CURSOR, curSize, curSize, LR_SHARED));
	m_CustomCursor = false;
	if (setcc == nullptr ? false : stoi(setcc))
		SetCustomCursor();

	m_RouteChecker = nullptr;
	const char* setrc = GetDataFromSettings(SETTING_ROUTE_CHECKER_CSV);
	if (setrc != nullptr)
		LoadRouteChecker(setrc);

	m_DepartureSequence = nullptr;

	m_TrackedRecorder = new TrackedRecorder(this);
	const char* setar = GetDataFromSettings(SETTING_AUTO_RETRACK);
	m_AutoRetrack = setar == nullptr ? 0 : stoi(setar);

	m_TransitionLevel = new TransitionLevel(this);
	const char* settl = GetDataFromSettings(SETTING_TRANS_LVL_CSV);
	if (settl != nullptr)
		LoadTransitionLevel(settl);

	const char* setma = GetDataFromSettings(SETTING_TRANS_MALT_TXT);
	if (setma != nullptr)
		LoadMetricAltitude(setma);

	const char* setnm = GetDataFromSettings(SETTING_CUSTOM_NUMBER_MAP);
	m_CustomNumMap = "0123456789";
	if (setnm != nullptr) {
		if (strlen(setnm) == 10) {
			m_CustomNumMap = setnm;
			DisplayUserMessage("MESSAGE", "MTEPlugin", ("Numbers are mapped to (0-9): " + m_CustomNumMap).c_str(), 1, 0, 0, 0, 0);
		}
	}

	const char* setac = GetDataFromSettings(SETTING_AMEND_CFL);
	m_AmendCFL = setac == nullptr ? 1 : stoi(setac);

	AddAlias(".mteplugin", GITHUB_LINK); // for testing and for fun

	RegisterTagItemType("GS(KPH) with trend indicator", TAG_ITEM_TYPE_GS_W_IND);
	RegisterTagItemType("RMK/STS indicator", TAG_ITEM_TYPE_RMK_IND);
	RegisterTagItemType("Vertical speed (4-digit FPM)", TAG_ITEM_TYPE_VS_FPM);
	RegisterTagItemType("Climb/Descend/Level indicator", TAG_ITEM_TYPE_LVL_IND);
	RegisterTagItemType("Actual altitude (m)", TAG_ITEM_TYPE_AFL_MTR);
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
	RegisterTagItemType("Departure status", TAG_TIEM_TYPE_DEP_STS);
	RegisterTagItemType("RECAT-CN (LMCBJ)", TAG_TIEM_TYPE_RECAT_WTC);
	RegisterTagItemType("Assigned speed bound (Topsky, +/-)", TAG_ITEM_TYPE_ASPD_BND);

	RegisterTagItemFunction("Set COMM ESTB", TAG_ITEM_FUNCTION_COMM_ESTAB);
	RegisterTagItemFunction("Restore assigned data", TAG_ITEM_FUNCTION_RCNT_RST);
	RegisterTagItemFunction("Open CFL popup menu", TAG_ITEM_FUNCTION_CFL_MENU);
	RegisterTagItemFunction("Open CFL popup edit", TAG_ITEM_FUNCTION_CFL_EDIT);
	RegisterTagItemFunction("Open RFL popup menu", TAG_ITEM_FUNCTION_RFL_MENU);
	RegisterTagItemFunction("Open RFL popup edit", TAG_ITEM_FUNCTION_RFL_EDIT);
	RegisterTagItemFunction("Open similar callsign list", TAG_ITEM_FUNCTION_SC_LIST);
	RegisterTagItemFunction("Show route checker info", TAG_ITEM_FUNCTION_RTE_INFO);
	RegisterTagItemFunction("Set departure sequence", TAG_ITEM_FUNCTION_DSQ_MENU);
	RegisterTagItemFunction("Set departure status", TAG_ITEM_FUNCTION_DSQ_STS);
	RegisterTagItemFunction("Open assigned speed popup list", TAG_ITEM_FUNCTION_SPD_LIST);

	DisplayUserMessage("MESSAGE", "MTEPlugin",
		(string("MTEPlugin loaded! For help please refer to ") + GITHUB_LINK).c_str(),
		1, 0, 0, 0, 0);
}

CMTEPlugIn::~CMTEPlugIn(void)
{
	while (!m_ScreenStack.empty()) {
		delete m_ScreenStack.top();
		m_ScreenStack.pop();
	}
	delete m_TransitionLevel;
	delete m_TrackedRecorder;
	DeleteDepartureSequence();
	UnloadRouteChecker();
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

		break; }
	case TAG_ITEM_TYPE_RMK_IND: {
		if (!FlightPlan.IsValid()) break;
		string remarks;
		remarks = FlightPlan.GetFlightPlanData().GetRemarks();
		if (remarks.find("RMK/") != string::npos || remarks.find("STS/") != string::npos)
			sprintf_s(sItemString, 2, "*");
		else
			sprintf_s(sItemString, 2, " ");

		break; }
	case TAG_ITEM_TYPE_VS_FPM: {
		int vs = abs(CalculateVerticalSpeed(RadarTarget));
		if (vs > 100)
			sprintf_s(sItemString, 5, "%04d", OVRFLW4(vs));

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
		int altref;
		int rdrAlt = m_TransitionLevel->GetRadarDisplayAltitude(RadarTarget, altref);
		int dspAlt;
		char tmpStr[16];
		if (!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign())) {
			dspAlt = (int)round(MetricAlt::FeettoM(rdrAlt) / 10.0);
			sprintf_s(tmpStr, 5, "%04d", OVRFLW4(dspAlt));
		}
		else {
			dspAlt = (int)round(rdrAlt / 100.0);
			sprintf_s(tmpStr, 4, "%03d", OVRFLW3(dspAlt));
		}
		string dspStr = tmpStr;
		if (altref == AltitudeReference::ALT_REF_QNH) {
			for (auto& c : dspStr) {
				// use custom number mapping
				c = m_CustomNumMap[int(c - '0')];
			}
		}
		else if (altref == AltitudeReference::ALT_REF_QFE) {
			dspStr = "(" + dspStr + ")";
		}
		strcpy_s(sItemString, dspStr.size() + 1, dspStr.c_str());

		break; }
	case TAG_ITEM_TYPE_CFL_FLX: {
		if (!FlightPlan.IsValid()) break;
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		switch (cflAlt)
		{
		case 2: // cleared for visual approach
			sprintf_s(sItemString, 3, "VA");
			break;
		case 1: // cleared for ILS approach
			sprintf_s(sItemString, 4, "ILS");
			break;
		case 0: { // no cleared level or CFL==RFL
			if (!IsCFLAssigned(FlightPlan)) {
				sprintf_s(sItemString, 5, "    ");
				break;
			}
			else {
				cflAlt = FlightPlan.GetClearedAltitude(); // difference: no ILS/VA, no CFL will show RFL
			}
			[[fallthrough]];
		}
		default: {// have a cleared level
			int dspAlt;
			int trslvl, elev;
			bool isQFE = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev).size() && elev && cflAlt < trslvl;
			cflAlt -= isQFE ? elev : 0;
			bool isfeet = m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign());
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
				string qfeAltStr = "(" + string(sItemString) + ")";
				strcpy_s(sItemString, qfeAltStr.size() + 1, qfeAltStr.c_str());
			}
			break; }
		}
		if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			*pColorCode = TAG_COLOR_REDUNDANT;
		}
		break; }
	case TAG_ITEM_TYPE_CFL_MTR: {
		if (!FlightPlan.IsValid()) break;
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int dspAlt = MetricAlt::LvlFeettoM(cflAlt);
		sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt / 10));

		break; }
	case TAG_ITEM_TYPE_RFL_ICAO: {
		if (!FlightPlan.IsValid()) break;
		int rflAlt = FlightPlan.GetFinalAltitude();
		int dspMtr;
		if (MetricAlt::RflFeettoM(rflAlt, dspMtr) && !m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign())) {
			// is metric RVSM and not forced feet
			char trsMrk = rflAlt >= GetTransitionAltitude() ? 'S' : 'M';
			sprintf_s(sItemString, 6, "%c%04d", trsMrk, OVRFLW4(dspMtr / 10));
		}
		else {
			rflAlt = (int)round(rflAlt / 100.0);
			sprintf_s(sItemString, 5, "F%03d", OVRFLW3(rflAlt));
		}
		break; }
	case TAG_ITEM_TYPE_SC_IND: {
		if (!FlightPlan.IsValid()) break;
		if (m_TrackedRecorder->IsSimilarCallsign(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 3, "SC");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_RFL_IND: {
		if (!FlightPlan.IsValid() ||
			!FlightPlan.GetTrackingControllerIsMe() ||
			m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign()))
			break;
		int rflAlt = FlightPlan.GetFinalAltitude();
		int _meter;
		if (!MetricAlt::RflFeettoM(rflAlt, _meter))  // not metric RVSM
			sprintf_s(sItemString, 2, "#");

		break; }
	case TAG_ITEM_TYPE_RVSM_IND: {
		if (!FlightPlan.IsValid()) break;
		CFlightPlanData fpdata = FlightPlan.GetFlightPlanData();
		string acinf = fpdata.GetAircraftInfo();
		if (!strcmp(fpdata.GetPlanType(), "V"))
			sprintf_s(sItemString, 2, "V");
		else if (acinf.size() <= 8) { // assume FAA format
			char capa = fpdata.GetCapibilities();
			if (capa == 'H' || capa == 'W' || capa == 'J' || capa == 'K' || capa == 'L' || capa == 'Z' || capa == '?')
				sprintf_s(sItemString, 2, " ");
			else {
				sprintf_s(sItemString, 2, "X");
			}
		}
		else { // assume ICAO format
			string acet;
			if (acinf.find('(') != string::npos && acinf.find(')') != string::npos) { // () in string, for TopSky. e.g. A321/L (SDE2E3FGIJ1RWY/H)
				int lc = acinf.find('(');
				int rc = acinf.find(')');
				acet = acinf.substr(lc + 1, rc - lc);
			}
			else { // no () in string, for erroneous simbrief prefile. e.g. A333/H-SDE3GHIJ2J3J5M1RVWXY/LB2D1
				acet = acinf.substr(acinf.find('-') + 1);
			}
			if (acet.substr(0, acet.find('/')).find('W') != string::npos)
				sprintf_s(sItemString, 2, " ");
			else
				sprintf_s(sItemString, 2, "X");
		}
		break; }
	case TAG_ITEM_TYPE_COMM_IND: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackedRecorder->IsCommEstablished(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 2, "C");
			*pColorCode = TAG_COLOR_REDUNDANT;
		}
		break; }
	case TAG_ITEM_TYPE_RECAT_BC: {
		if (!FlightPlan.IsValid()) break;
		if (FlightPlan.GetFlightPlanData().GetAircraftWtc() == 'H') {
			auto rc = m_ReCatMap.find(FlightPlan.GetFlightPlanData().GetAircraftFPType());
			if (rc != m_ReCatMap.end() && rc->second != 'J')
				sprintf_s(sItemString, 3, "-%c", rc->second);
		}
		break; }
	case TAG_TIEM_TYPE_RECAT_WTC: {
		if (!FlightPlan.IsValid()) break;
		auto rc = m_ReCatMap.find(FlightPlan.GetFlightPlanData().GetAircraftFPType());
		sprintf_s(sItemString, 2, "%c",
			rc != m_ReCatMap.end() ? rc->second : FlightPlan.GetFlightPlanData().GetAircraftWtc());

		break; }
	case TAG_ITEM_TYPE_RTE_CHECK: {
		if (m_RouteChecker == nullptr ||
			!FlightPlan.IsValid() ||
			FlightPlan.GetClearenceFlag())
			break;
		switch (m_RouteChecker->CheckFlightPlan(FlightPlan))
		{
		case RouteCheckerConstants::NOT_FOUND:
			sprintf_s(sItemString, 3, "? ");
			break;
		case RouteCheckerConstants::INVALID:
			sprintf_s(sItemString, 3, "X ");
			*pColorCode = TAG_COLOR_INFORMATION;
			break;
		case RouteCheckerConstants::PARTIAL_NO_LEVEL:
			sprintf_s(sItemString, 3, "PL");
			*pColorCode = TAG_COLOR_REDUNDANT;
			break;
		case RouteCheckerConstants::STRUCT_NO_LEVEL:
			sprintf_s(sItemString, 3, "YL");
			*pColorCode = TAG_COLOR_REDUNDANT;
			break;
		case RouteCheckerConstants::TEXT_NO_LEVEL:
			sprintf_s(sItemString, 3, "YL");
			break;
		case RouteCheckerConstants::PARTIAL_OK_LEVEL:
			sprintf_s(sItemString, 3, "P ");
			*pColorCode = TAG_COLOR_REDUNDANT;
			break;
		case RouteCheckerConstants::STRUCT_OK_LEVEL:
			sprintf_s(sItemString, 3, "Y ");
			*pColorCode = TAG_COLOR_REDUNDANT;
			break;
		case RouteCheckerConstants::TEXT_OK_LEVEL:
			sprintf_s(sItemString, 3, "Y ");
			break;
		default:
			break;
		}
		break; }
	case TAG_ITEM_TYPE_SQ_DUPE: {
		if (!FlightPlan.IsValid()) break;
		if (m_TrackedRecorder->IsSquawkDUPE(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 5, "DUPE");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_DEP_SEQ: {
		if (!FlightPlan.IsValid()) break;
		if (m_DepartureSequence == nullptr) m_DepartureSequence = new DepartureSequence();
		int seq = m_DepartureSequence->GetSequence(FlightPlan);
		if (seq > 0)
			sprintf_s(sItemString, 3, "%02d", OVRFLW2(seq));
		else if (seq < 0) { // reconnected
			sprintf_s(sItemString, 3, "--");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_TIEM_TYPE_DEP_STS: {
		if (!FlightPlan.IsValid()) break;
		string gsts = FlightPlan.GetGroundState();
		if (gsts.size()) {
			sprintf_s(sItemString, gsts.size() + 1, gsts.c_str());
			if (!FlightPlan.GetClearenceFlag()) {
				*pColorCode = TAG_COLOR_INFORMATION;
			}
		}
		else if (FlightPlan.GetClearenceFlag()) {
			sprintf_s(sItemString, 5, "CLRD");
		}
		break; }
	case TAG_ITEM_TYPE_RVEC_IND: {
		if (FlightPlan.IsValid() && FlightPlan.GetTrackingControllerIsMe() && FlightPlan.GetControllerAssignedData().GetAssignedHeading()) {
			sprintf_s(sItemString, 3, "RV");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_RCNT_IND: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackedRecorder->IsActive(FlightPlan)) {
			sprintf_s(sItemString, 2, "r");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_ASPD_BND: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetControllerAssignedData().GetAssignedSpeed() &&
			!FlightPlan.GetControllerAssignedData().GetAssignedMach())
			break; // not assigned
		string strip = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(7); // on (2, 3) of annotation
		if (strip.find("/s+/") != string::npos) {
			sprintf_s(sItemString, 2, "+");
		}
		else if (strip.find("/s-/") != string::npos) {
			sprintf_s(sItemString, 2, "-");
		}
		break; }
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

		break; }
	case TAG_ITEM_FUNCTION_RCNT_RST: {
		if (!FlightPlan.IsValid()) break;
		m_TrackedRecorder->SetTrackedData(FlightPlan);

		break; }
	case TAG_ITEM_FUNCTION_CFL_SET_MENU: {
		if (!FlightPlan.IsValid()) break;
		int tgtAlt = MetricAlt::GetAltitudeFromMenuItem(sItemString, !m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign()));
		if (tgtAlt > MetricAlt::ALT_MAP_NOT_FOUND) {
			if (tgtAlt == 1 || tgtAlt == 2) { // ILS or VA
				FlightPlan.GetControllerAssignedData().SetAssignedHeading(0);
			}
			else if (tgtAlt > 2 && m_AmendCFL == 1) {
				int trslvl, elev;
				string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
				tgtAlt += aptgt.size() && tgtAlt < trslvl ? elev : 0; // convert QNH to QFE
			}
			FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow
		}
		break; }
	case TAG_ITEM_FUNCTION_CFL_SET_EDIT: {
		if (!FlightPlan.IsValid() || !strlen(sItemString)) break;
		string input = MakeUpper(sItemString);
		if (input == "F") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), true);
			break;
		}
		else if (input == "M") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), false);
			break;
		}
		int tgtAlt = -1;
		// use regular expressions to match input
		regex rxfd("^F([0-9]+)\\.$");
		regex rxf("^F([0-9]+)$");
		regex rxd("^([0-9]+)\\.$");
		regex rxn("^([0-9]+)$");
		smatch match;
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
		if (tgtAlt > 2 && m_AmendCFL == 1) {
			int trslvl, elev;
			string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
			tgtAlt += aptgt.size() && tgtAlt < trslvl ? elev : 0; // convert QNH to QFE
		}
		FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow

		break; }
	case TAG_ITEM_FUNCTION_CFL_MENU: {
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
		string copxName = FlightPlan.GetExitCoordinationPointName();
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int ref;
		int rdrAlt = m_TransitionLevel->GetRadarDisplayAltitude(RadarTarget, ref);
		int trslvl, elev;
		m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
		auto m_alt = MetricAlt::GetMenuItems(!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign()), trslvl);
		if (elev) { // QFE in use
			cflAlt -= cflAlt > 2 && cflAlt < trslvl ? elev : 0;
			// remove unavailable altitudes from list
			for (auto it = m_alt.begin(); it != m_alt.end();) {
				if (it->altitude < trslvl && it->altitude + elev >= trslvl) {
					it = m_alt.erase(it);
				}
				else {
					it++;
				}
			}
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
					string nextName = ExtractedRoute.GetPointName(i);
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
		int minDif(100000), minAlt(0); // a big enough number
		for (auto it = m_alt.begin(); it != m_alt.end(); it++) {
			int dif = abs(it->altitude - cmpAlt);
			if (dif < minDif) {
				minDif = dif;
				minAlt = it->altitude;
			}
		}
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
		break; }
	case TAG_ITEM_FUNCTION_CFL_EDIT: {
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

		break; }
	case TAG_ITEM_FUNCTION_RFL_SET_MENU: {
		if (!FlightPlan.IsValid()) break;
		int tgtAlt = MetricAlt::GetAltitudeFromMenuItem(sItemString, !m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign()));
		if (tgtAlt > MetricAlt::ALT_MAP_NOT_FOUND) {
			FlightPlan.GetControllerAssignedData().SetFinalAltitude(tgtAlt);
		}
		break; }
	case TAG_ITEM_FUNCTION_RFL_SET_EDIT: {
		if (!FlightPlan.IsValid() || !strlen(sItemString)) break;
		CFlightPlanControllerAssignedData ctrData = FlightPlan.GetControllerAssignedData();
		string input = MakeUpper(sItemString);
		if (input == "F") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), true);
			break;
		}
		else if (input == "M") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), false);
			break;
		}
		regex rxm("^([0-9]{1,3})$");
		regex rxf("^F([0-9]{1,3})$");
		smatch match;
		if (regex_match(input, match, rxf)) {
			ctrData.SetFinalAltitude(stoi(match[1]) * 100);
		}
		else if (regex_match(input, match, rxm)) {
			ctrData.SetFinalAltitude(MetricAlt::LvlMtoFeet(stoi(match[1]) * 100));
		}
		break; }
	case TAG_ITEM_FUNCTION_RFL_MENU: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupList(Area, "RFL Menu", 1);
		// pre-select altitude
		int rflAlt = FlightPlan.GetFinalAltitude();
		int minDif(100000), minAlt(0); // a big enough number
		auto m_alt = MetricAlt::GetMenuItems(!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign()), 0);
		for (auto it = m_alt.begin(); it != m_alt.end(); it++) {
			int dif = abs(it->altitude - rflAlt);
			if (dif < minDif) {
				minDif = dif;
				minAlt = it->altitude;
			}
		}
		for (auto it = m_alt.rbegin(); it != m_alt.rend(); it++) {
			if (it->altitude > 3) {
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_RFL_SET_MENU, it->altitude == minAlt, POPUP_ELEMENT_NO_CHECKBOX, false, false);
			}
			else if (it->altitude == 3) { // EDIT
				AddPopupListElement(it->entry.c_str(), nullptr, TAG_ITEM_FUNCTION_RFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
			}
		}
		break; }
	case TAG_ITEM_FUNCTION_RFL_EDIT: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_RFL_SET_EDIT, "");

		break; }
	case TAG_ITEM_FUNCTION_SC_LIST: {
		if (!FlightPlan.IsValid()) break;
		string cs = FlightPlan.GetCallsign();
		if (!m_TrackedRecorder->IsSimilarCallsign(cs)) // not a SC
			break;
		OpenPopupList(Area, "SC List", 1);
		AddPopupListElement(cs.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT, true);
		auto cset = m_TrackedRecorder->GetSimilarCallsigns(cs);
		for (auto& c : cset) {
			AddPopupListElement(c.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT);
		}

		break; }
	case TAG_ITEM_FUNCTION_SC_SELECT: {
		if (pluginWindow != nullptr) {
			HWND editWnd = FindWindowEx(pluginWindow, nullptr, "Edit", nullptr);
			if (editWnd != nullptr)
				SendMessage(editWnd, WM_SETTEXT, NULL, (LPARAM)(LPCSTR)".find ");
		}
		SetASELAircraft(FlightPlanSelect(sItemString));

		break; }
	case TAG_ITEM_FUNCTION_RTE_INFO: {
		if (m_RouteChecker == nullptr ||
			!FlightPlan.IsValid() ||
			FlightPlan.GetClearenceFlag())
			break;
		int rc = m_RouteChecker->CheckFlightPlan(FlightPlan, true); // force a refresh here to avoid error
		if (rc == RouteCheckerConstants::NOT_FOUND) break;
		DisplayRouteMessage(FlightPlan.GetFlightPlanData().GetOrigin(), FlightPlan.GetFlightPlanData().GetDestination());

		break; }
	case TAG_ITEM_FUNCTION_DSQ_MENU: {
		if (!FlightPlan.IsValid()) break;
		if (m_DepartureSequence == nullptr) break;
		int seq = m_DepartureSequence->GetSequence(FlightPlan);
		if (seq <= 0) { // reconnected or completely new
			m_DepartureSequence->AddFlight(FlightPlan);
		}
		else {
			OpenPopupEdit(Area, TAG_ITEM_FUNCTION_DSQ_EDIT, "");
		}
		break;	}
	case TAG_ITEM_FUNCTION_DSQ_EDIT: {
		if (!FlightPlan.IsValid()) break;
		int seq;
		if (sscanf_s(sItemString, "%d", &seq) != 1) break;
		if (seq >= 0) {
			m_DepartureSequence->EditSequence(FlightPlan, seq);
		}
		break;	}
	case TAG_ITEM_FUNCTION_DSQ_STS: {
		if (!FlightPlan.IsValid()) break;
		if (!FlightPlan.GetClearenceFlag()) {
			// set cleared flag
			CallNativeItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_CLEARED_FLAG, Pt, Area);
		}
		else {
			// set status
			CallNativeItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_GROUND_STATUS, Pt, Area);
		}
		break; }
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
		break; }
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
		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
	if (!FlightPlan.IsValid())
		return;
	m_TrackedRecorder->UpdateFlight(FlightPlan);
	if (DataType == CTR_DATA_TYPE_TEMPORARY_ALTITUDE) {
		if (m_AmendCFL == 2) { // amend only when mode 2 (all) is set
			int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
			if (cflAlt > 2) { // exclude ILS, VA, NONE
				int trslvl, elev;
				string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
				if (aptgt.size() && cflAlt < trslvl && elev > 0) {
					cflAlt += elev; // convert QNH to QFE
					m_AmendCFL = 0;
					FlightPlan.GetControllerAssignedData().SetClearedAltitude(cflAlt);
					m_AmendCFL = 2;
					return;
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
	if (m_RouteChecker != nullptr &&
		(DataType == CTR_DATA_TYPE_FINAL_ALTITUDE && !FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetTransponderC())) {
		m_RouteChecker->CheckFlightPlan(FlightPlan, true);
	}
	if (m_DepartureSequence != nullptr &&
		(DataType == CTR_DATA_TYPE_GROUND_STATE || DataType == CTR_DATA_TYPE_CLEARENCE_FLAG)) {
		m_DepartureSequence->EditSequence(FlightPlan, 0);
	}
	if (DataType == CTR_DATA_TYPE_GROUND_STATE &&
		FlightPlan.GetClearenceFlag() &&
		!strlen(FlightPlan.GetGroundState())) {
		CallNativeItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_CLEARED_FLAG, POINT(), RECT());
	}
}

void CMTEPlugIn::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid())
		return;
	if (m_RouteChecker != nullptr)
		m_RouteChecker->RemoveCache(FlightPlan);
	if (m_DepartureSequence != nullptr)
		m_DepartureSequence->EditSequence(FlightPlan, -1);
	if (FlightPlan.GetTrackingControllerIsMe())
		m_TrackedRecorder->UpdateFlight(FlightPlan, false);
}

void CMTEPlugIn::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid())
		return;
	if (m_RouteChecker != nullptr) {
		m_RouteChecker->CheckFlightPlan(FlightPlan, true);
	}
}

void CMTEPlugIn::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	if (!RadarTarget.IsValid())
		return;
	if (m_TrackedRecorder->IsActive(RadarTarget)) {
		m_TrackedRecorder->UpdateFlight(RadarTarget);
	}
	else if (m_AutoRetrack) {
		bool r = m_TrackedRecorder->SetTrackedData(RadarTarget);
		if (r && m_AutoRetrack == 2) {
			string msg = string(RadarTarget.GetCallsign()) + " reconnected and is re-tracked.";
			DisplayUserMessage("MTEP-Recorder", "MTEPlugin", msg.c_str(), 1, 1, 0, 0, 0);
		}
	}
}

CRadarScreen* CMTEPlugIn::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	CMTEPScreen* screen = new CMTEPScreen();
	m_ScreenStack.push(screen);
	return screen;
}

bool CMTEPlugIn::OnCompileCommand(const char* sCommandLine)
{
	string cmd = sCommandLine;
	smatch match; // all regular expressions will ignore cases

	// custom cursor
	regex rxcc("^.MTEP CURSOR (ON|OFF)$", regex_constants::icase);
	if (regex_match(cmd, match, rxcc)) {
		CancelCustomCursor();
		bool cc = MakeUpper(match[1].str()) == "ON";
		if (cc)
			SetCustomCursor();
		SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", cc ? "1" : "0");
		return true;
	}

	// flightradar24 and variflight
	regex rxfr("^.MTEP (FR24|VARI) ([A-Z]{4})$", regex_constants::icase);
	if (regex_match(cmd, match, rxfr)) {
		string airport = MakeUpper(match[2].str());
		CSectorElement se = SectorFileElementSelectFirst(SECTOR_ELEMENT_AIRPORT);
		for (; se.IsValid() && airport != se.GetName();
			se = SectorFileElementSelectNext(se, SECTOR_ELEMENT_AIRPORT));
		CPosition pos;
		if (se.GetPosition(&pos, 0)) {
			string url_full = MakeUpper(match[1].str()) == "FR24" ? \
				"https://www.flightradar24.com/" + to_string(pos.m_Latitude) + "," + to_string(pos.m_Longitude) + "/9":\
				"https://flightadsb.variflight.com/tracker/" + to_string(pos.m_Longitude) + "," + to_string(pos.m_Latitude) + "/9";
			ShellExecute(NULL, "open", url_full.c_str(), NULL, NULL, SW_SHOW);
			return true;
		}
	}

	// load route checker
	regex rxrc("^.MTEP RC (.+\\.CSV)$", regex_constants::icase);
	if (regex_match(cmd, match, rxrc)) {
		LoadRouteChecker(match[1].str());
		SaveDataToSettings(SETTING_ROUTE_CHECKER_CSV, "route checker csv file", match[1].str().c_str());
		return m_RouteChecker != nullptr;
	}

	// route checker get route info
	regex rxrcod("^.MTEP RC ([A-Z]{4}) ([A-Z]{4})$", regex_constants::icase);
	if (regex_match(cmd, match, rxrcod)) {
		string msg = DisplayRouteMessage(MakeUpper(match[1].str()), MakeUpper(match[2].str()));
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
	regex rxds("^.MTEP DS RESET$", regex_constants::icase);
	if (regex_match(cmd, match, rxds)) {
		DeleteDepartureSequence();
		return true;
	}

	// reset tracked recorder
	regex rxtr("^.MTEP TR RESET$", regex_constants::icase);
	if (regex_match(cmd, match, rxtr)) {
		ResetTrackedRecorder();
		return true;
	}

	// set auto retrack
	regex rxtrrt("^.MTEP TR ([0-2])$", regex_constants::icase);
	if (regex_match(cmd, match, rxtrrt)) {
		string res = match[1].str();
		const char* descr = "auto retrack mode";
		if (res == "1") {
			m_AutoRetrack = 1;
			SaveDataToSettings(SETTING_AUTO_RETRACK, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack mode 1 (silent) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "2") {
			m_AutoRetrack = 2;
			SaveDataToSettings(SETTING_AUTO_RETRACK, descr, "2");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack mode 2 (notify) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "0") {
			m_AutoRetrack = 0;
			SaveDataToSettings(SETTING_AUTO_RETRACK, descr, "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack is off", 1, 0, 0, 0, 0);
		}
		return true;
	}

	// load transition level
	regex rxtl("^.MTEP TL (.+\\.CSV)$", regex_constants::icase);
	if (regex_match(cmd, match, rxtl)) {
		SaveDataToSettings(SETTING_TRANS_LVL_CSV, "transition levels csv file", match[1].str().c_str());
		return LoadTransitionLevel(match[1].str());
	}

	// set transition level for single airport
	regex rxtlt("^.MTEP ([A-Z]{4}) TL (S|F)(\\d+)$", regex_constants::icase);
	regex rxtlb("^.MTEP ([A-Z]{4}) (QNH|QFE)$", regex_constants::icase);
	regex rxtlr("^.MTEP ([A-Z]{4}) R (\\d+)$", regex_constants::icase);
	if (regex_match(cmd, match, rxtlt)) {
		string airport = MakeUpper(match[1].str());
		int tl = stoi(MakeUpper(match[3].str())) * 100;
		if (MakeUpper(match[2].str()) == "S") {
			tl = MetricAlt::LvlMtoFeet(tl);
		}
		if (m_TransitionLevel->SetAirportParam(airport, tl, -1, -1)) {
			string msg = airport + ", transition level is set to " + to_string(tl) + " ft.";
			DisplayUserMessage("MESSAGE", "MTEPlugin", msg.c_str(), 1, 0, 0, 0, 0);
			return true;
		}
	}
	if (regex_match(cmd, match, rxtlb)) {
		string airport = MakeUpper(match[1].str());
		bool qfe = MakeUpper(match[2].str()) == "QFE";
		if (m_TransitionLevel->SetAirportParam(airport, -1, qfe, -1)) {
			string msg = airport + ", altitude reference is set to " + string(qfe ? "QFE." : "QNH.");
			DisplayUserMessage("MESSAGE", "MTEPlugin", msg.c_str(), 1, 0, 0, 0, 0);
			return true;
		}
	}
	if (regex_match(cmd, match, rxtlr)) {
		string airport = MakeUpper(match[1].str());
		int r = stoi(match[2].str());
		if (m_TransitionLevel->SetAirportParam(airport, -1, -1, r)) {
			string msg = airport + ", QNH/QFE range is set to " + match[2].str() + " miles.";
			DisplayUserMessage("MESSAGE", "MTEPlugin", msg.c_str(), 1, 0, 0, 0, 0);
			return true;
		}
	}

	// load MetricAlt settings
	regex rxma("^.MTEP MA (.+\\.TXT)$", regex_constants::icase);
	if (regex_match(cmd, match, rxma)) {
		SaveDataToSettings(SETTING_TRANS_MALT_TXT, "altitude menu definition txt file", match[1].str().c_str());
		return LoadMetricAltitude(match[1].str());
	}

	// set custom number mapping
	regex rxnm("^.MTEP NUM ([\\S]{10})$", regex_constants::icase);
	if (regex_match(cmd, match, rxnm)) {
		m_CustomNumMap = match[1].str();
		SaveDataToSettings(SETTING_CUSTOM_NUMBER_MAP, "custom number mapping (0-9)", m_CustomNumMap.c_str());
		DisplayUserMessage("MESSAGE", "MTEPlugin", ("Numbers are mapped to (0-9): " + m_CustomNumMap).c_str(), 1, 0, 0, 0, 0);
		return true;
	}

	// set amend QFE in CFL
	regex rxac("^.MTEP QFE ([0-2])$", regex_constants::icase);
	if (regex_match(cmd, match, rxac)) {
		string res = match[1].str();
		const char* descr = "amend QFE in CFL";
		if (res == "1") {
			m_AmendCFL = 1;
			SaveDataToSettings(SETTING_AMEND_CFL, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL mode 1 (MTEP) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "2") {
			m_AmendCFL = 2;
			SaveDataToSettings(SETTING_AMEND_CFL, descr, "2");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL mode 2 (all) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "0") {
			m_AmendCFL = 0;
			SaveDataToSettings(SETTING_AMEND_CFL, descr, "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL is off", 1, 0, 0, 0, 0);
		}
		return true;
	}

	return false;
}

void CMTEPlugIn::CallNativeItemFunction(const char* sCallsign, int FunctionId, POINT Pt, RECT Area)
{
	while (!m_ScreenStack.empty()) {
		auto s = m_ScreenStack.top();
		if (s->m_Opened)
			return s->StartTagFunction(sCallsign, nullptr, 0, nullptr, nullptr, FunctionId, Pt, Area);
		else {
			delete s;
			m_ScreenStack.pop();
		}
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
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Cursor is reset!", 1, 0, 0, 0, 0);
	m_CustomCursor = false;
}

void CMTEPlugIn::LoadRouteChecker(string filename)
{
	UnloadRouteChecker();
	if (filename[0] == '@') {
		filename = GetAbsolutePath(filename.substr(1));
	}
	try {
		m_RouteChecker = new RouteChecker(this, filename);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route checker is loaded successfully. CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route checker failed to load (" + e + "). CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		UnloadRouteChecker();
	}
	catch (exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route checker failed to load (" + string(e.what()) + "). CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		UnloadRouteChecker();
	}
}

void CMTEPlugIn::UnloadRouteChecker(void)
{
	if (m_RouteChecker != nullptr) {
		delete m_RouteChecker;
		m_RouteChecker = nullptr;
		DisplayUserMessage("MESSAGE", "MTEPlugin", "Route checker is unloaded!", 1, 0, 0, 0, 0);
	}
}

void CMTEPlugIn::DeleteDepartureSequence(void)
{
	if (m_DepartureSequence != nullptr) {
		delete m_DepartureSequence;
		m_DepartureSequence = nullptr;
		DisplayUserMessage("MESSAGE", "MTEPlugin", "Departure sequence is deleted!", 1, 0, 0, 0, 0);
	}
}

void CMTEPlugIn::ResetTrackedRecorder(void)
{
	if (m_TrackedRecorder != nullptr)
		delete m_TrackedRecorder;
	m_TrackedRecorder = new TrackedRecorder(this);
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Tracked recorder is reset!", 1, 0, 0, 0, 0);
}

bool CMTEPlugIn::LoadTransitionLevel(string filename)
{
	if (filename[0] == '@') {
		filename = GetAbsolutePath(filename.substr(1));
	}
	try {
		m_TransitionLevel->LoadCSV(filename);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Transition levels are loaded successfully. CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Transition levels failed to load (" + e + "). CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		delete m_TransitionLevel;
		m_TransitionLevel = new TransitionLevel(this);
		return false;
	}
	catch (exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Transition levels failed to load (" + string(e.what()) + "). CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		delete m_TransitionLevel;
		m_TransitionLevel = new TransitionLevel(this);
		return false;
	}
	return true;
}

bool CMTEPlugIn::LoadMetricAltitude(string filename)
{
	if (filename[0] == '@') {
		filename = GetAbsolutePath(filename.substr(1));
	}
	try {
		MetricAlt::LoadAltitudeDefinition(filename);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Altitude menu definitions are loaded successfully. TXT file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Altitude menu definitions failed to load (" + e + "). TXT file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		return false;
	}
	catch (exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Altitude menu definitions failed to load (" + string(e.what()) + "). TXT file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		return false;
	}
	return true;
}

string CMTEPlugIn::DisplayRouteMessage(string departure, string arrival)
{
	if (m_RouteChecker == nullptr) return "";
	auto rinfo = m_RouteChecker->GetRouteInfo(departure, arrival);
	if (!rinfo.size()) return "";
	string res = departure + "-" + arrival;
	DisplayUserMessage("MTEP-Route", res.c_str(), (to_string(rinfo.size()) + " route(s):").c_str(), 1, 1, 0, 0, 0);
	for (auto& ri : rinfo) {
		DisplayUserMessage("MTEP-Route", nullptr, ri.c_str(), 1, 0, 0, 0, 0);
		res += "\n" + ri;
	}
	return res;
}

string MakeUpper(string str)
{
	for (auto& c : str) c = toupper(c);
	return str;
}

string GetAbsolutePath(string relativePath)
{
	// add DLL directory before relative path
	if (pluginModule != nullptr) {
		TCHAR pBuffer[MAX_PATH] = { 0 };
		GetModuleFileName(pluginModule, pBuffer, sizeof(pBuffer) / sizeof(TCHAR) - 1);
		string currentPath = pBuffer;
		return currentPath.substr(0, currentPath.find_last_of("\\/") + 1) + relativePath;
	}
	return string();
}

int CalculateVerticalSpeed(CRadarTarget RadarTarget)
{
	if (!RadarTarget.IsValid()) return 0;
	CRadarTargetPositionData curpos = RadarTarget.GetPosition();
	CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
	double curAlt = curpos.GetPressureAltitude();
	double preAlt = prepos.GetPressureAltitude();
	double preT = prepos.GetReceivedTime();
	double curT = curpos.GetReceivedTime();
	double deltaT = preT - curT;
	deltaT = deltaT ? deltaT : INFINITE;
	return (int)round((curAlt - preAlt) / deltaT * 60.0);
}

bool IsCFLAssigned(CFlightPlan FlightPlan)
{
	// tell when cleared altitude is 0, is CFL not assigned or assigned to RFL
	// true means CFL is assigned to RFL, false means no CFL
	if (FlightPlan.IsValid() && strlen(FlightPlan.GetTrackingControllerCallsign())) { // tracked by someone
		CRadarTarget RadarTarget = FlightPlan.GetCorrelatedRadarTarget();
		if (RadarTarget.IsValid()) {
			string squawk = RadarTarget.GetPosition().GetSquawk();
			if (!(squawk == "7700" || squawk == "7600" || squawk == "7500" || !RadarTarget.GetPosition().GetTransponderC()))
				return true;
		}
	}
	return false;
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
