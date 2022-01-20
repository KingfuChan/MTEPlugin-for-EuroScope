// MTEPlugin.cpp

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MTEPlugin.h"

#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_FILE "MTEPlugin.dll"
#define PLUGIN_VERSION "2.4.1"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2022 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugin-for-EuroScope"
#endif // !COPYRIGHTS

using namespace std;
using namespace EuroScopePlugIn;

// TAG ITEM TYPE
const int TAG_ITEM_TYPE_GS_W_IND = 1; // GS(KPH) with trend indicator
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_FPM = 3; // Vertical speed (4-digit FPM)
const int TAG_ITEM_TYPE_LVL_IND = 4; // Climb/Descend/Level indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // Actual altitude (m)
const int TAG_ITEM_TYPE_CFL_MTR = 6; // Cleared flight level
const int TAG_ITEM_TYPE_RFL_ICAO = 7; // Final flight level
const int TAG_ITEM_TYPE_SC_IND = 8; // Similar callsign indicator
const int TAG_ITEM_TYPE_RFL_IND = 9; // RFL unit indicator
const int TAG_ITEM_TYPE_RVSM_IND = 10; // RVSM indicator
const int TAG_ITEM_TYPE_COMM_IND = 11; // COMM ESTB indicator
const int TAG_ITEM_TYPE_RECAT = 12; // RECAT-CN
const int TAG_ITEM_TYPE_RTE_CHECK = 13; // Route validity
const int TAG_ITEM_TYPE_SQ_DUPE = 14; // Tracked DUPE warning

// TAG ITEM FUNCTION
const int TAG_ITEM_FUNCTION_COMM_ESTAB = 1; // Set COMM ESTB
const int TAG_ITEM_FUNCTION_CFL_SET = 10; // Set CFL (not registered)
const int TAG_ITEM_FUNCTION_CFL_MENU = 11; // Open CFL popup menu
const int TAG_ITEM_FUNCTION_CFL_EDIT = 12; // Open CFL popup edit (not registered)
const int TAG_ITEM_FUNCTION_RFL_SET = 20; // Set RFL (not registered)
const int TAG_ITEM_FUNCTION_RFL_MENU = 21; // Open RFL popup menu
const int TAG_ITEM_FUNCTION_RFL_EDIT = 22; // Open RFL popup edit (not registered)
const int TAG_ITEM_FUNCTION_SC_LIST = 30; // Open similar callsign list
const int TAG_ITEM_FUNCTION_SC_SELECT = 31; // Select in similar callsign list (not registered)
const int TAG_ITEM_FUNCTION_RTE_INFO = 40; // Show route checker info

// GROUND SPEED TREND CHAR
const char CHR_GS_NON = ' ';
const char CHR_GS_ACC = 'A';
const char CHR_GS_DEC = 'L';

// COMPUTERISING RELATED
const float THRESHOLD_ACC_DEC = 2.5; // threshold (kph) to determine accel/decel
const int TIMER_REFRESH_INTERVAL = 3; // OnTimer refresh interval (seconds)
constexpr double KN_KPH(double k) { return 1.85184 * k; } // 1 knot = 1.85184 kph
constexpr double KPH_KN(double k) { return k / 1.85184; } // 1.85184 kph = 1 knot
constexpr int OVRFLW3(int t) { return abs(t) > 999 ? 999 : abs(t); } // overflow pre-process 3 digits
constexpr int OVRFLW4(int t) { return abs(t) > 9999 ? 9999 : abs(t); }  // overflow pre-process 4 digits
string MakeUpper(string str);
int CalculateVerticalSpeed(CRadarTarget RadarTarget);
bool IsCFLAssigned(CFlightPlan FlightPlan);

// SETTING NAMES
const char* SETTING_CUSTOM_CURSOR = "CustomCursor";
const char* SETTING_ROUTE_CHECKER_CSV = "RteCheckerCSV";

// CURSOR RELATED
bool initCursor = true; // if cursor has not been set to custom / if using default cursor
bool customCursor = false; // if cursor need to be set to custom
WNDPROC gSourceProc;
HWND pluginWindow;
HCURSOR myCursor = nullptr;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Note that cursor setting will only be effective with it's original file name (MTEPlugin.dll)

CMTEPlugIn::CMTEPlugIn(void)
	: CPlugIn(COMPATIBILITY_CODE,
		PLUGIN_NAME,
		PLUGIN_VERSION,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	AddAlias(".mteplugin", GITHUB_LINK); // for testing and for fun
	RegisterTagItemType("GS(KPH) with trend indicator", TAG_ITEM_TYPE_GS_W_IND);
	RegisterTagItemType("RMK/STS indicator", TAG_ITEM_TYPE_RMK_IND);
	RegisterTagItemType("Vertical speed (4-digit FPM)", TAG_ITEM_TYPE_VS_FPM);
	RegisterTagItemType("Climb/Descend/Level indicator", TAG_ITEM_TYPE_LVL_IND);
	RegisterTagItemType("Actual altitude (m)", TAG_ITEM_TYPE_AFL_MTR);
	RegisterTagItemType("Cleared flight level", TAG_ITEM_TYPE_CFL_MTR);
	RegisterTagItemType("Final flight level", TAG_ITEM_TYPE_RFL_ICAO);
	RegisterTagItemType("Similar callsign indicator", TAG_ITEM_TYPE_SC_IND);
	RegisterTagItemType("RFL unit indicator", TAG_ITEM_TYPE_RFL_IND);
	RegisterTagItemType("RVSM indicator", TAG_ITEM_TYPE_RVSM_IND);
	RegisterTagItemType("COMM ESTB indicator", TAG_ITEM_TYPE_COMM_IND);
	RegisterTagItemType("RECAT-CN", TAG_ITEM_TYPE_RECAT);
	RegisterTagItemType("Route validity", TAG_ITEM_TYPE_RTE_CHECK);
	RegisterTagItemType("Tracked DUPE warning", TAG_ITEM_TYPE_SQ_DUPE);

	RegisterTagItemFunction("Set COMM ESTB", TAG_ITEM_FUNCTION_COMM_ESTAB);
	RegisterTagItemFunction("Open CFL popup menu", TAG_ITEM_FUNCTION_CFL_MENU);
	RegisterTagItemFunction("Open RFL popup menu", TAG_ITEM_FUNCTION_RFL_MENU);
	RegisterTagItemFunction("Open similar callsign list", TAG_ITEM_FUNCTION_SC_LIST);
	RegisterTagItemFunction("Show route checker info", TAG_ITEM_FUNCTION_RTE_INFO);

	DisplayUserMessage("MESSAGE", "MTEPlugin",
		(string("MTEPlugin is loaded! For help please refer to ") + GITHUB_LINK).c_str(),
		1, 0, 0, 0, 0);

	const char* setcc = GetDataFromSettings(SETTING_CUSTOM_CURSOR);
	customCursor = setcc == nullptr ? false : !strcmp(setcc, "1"); // 1 means true
	myCursor = CopyCursor((HCURSOR)::LoadImage(GetModuleHandle(PLUGIN_FILE), MAKEINTRESOURCE(IDC_CURSOR1), IMAGE_CURSOR, 0, 0, LR_SHARED));

	m_RouteChecker = nullptr;
	const char* setrc = GetDataFromSettings(SETTING_ROUTE_CHECKER_CSV);
	if (setrc != nullptr) {
		LoadRouteChecker(setrc);
	}
}

CMTEPlugIn::~CMTEPlugIn(void)
{
	UnloadRouteChecker();
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
		int dspgs = round(KN_KPH(curgs) / 10.0); // display GS
		dspgs = dspgs ? dspgs : RadarTarget.GetGS(); // If reported GS is 0 then uses the calculated one.
		int pregs = prepos.GetReportedGS(); // previous GS
		double diff = KN_KPH(double(curgs) - double(pregs));
		char gsTrend;
		if (diff >= THRESHOLD_ACC_DEC)
			gsTrend = CHR_GS_ACC;
		else if (diff <= -THRESHOLD_ACC_DEC)
			gsTrend = CHR_GS_DEC;
		else
			gsTrend = CHR_GS_NON;

		sprintf_s(sItemString, 5, "%03d%c", OVRFLW3(dspgs), gsTrend);

		break; }
	case TAG_ITEM_TYPE_RMK_IND: {
		string remarks;
		remarks = FlightPlan.GetFlightPlanData().GetRemarks();
		// remarks.MakeUpper(); // could crash by some special characters in certain system environment
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
		int dspAlt = round(MetricAlt::FeettoM(GetRadarDisplayAltitude(RadarTarget)) / 10.0);
		sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt));

		break; }
	case TAG_ITEM_TYPE_CFL_MTR: {
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		switch (cflAlt)
		{
		case 2: // cleared for visual approach
			sprintf_s(sItemString, 5, " VA ");
			break;
		case 1: // cleared for ILS approach
			sprintf_s(sItemString, 5, " ILS");
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
			if (MetricAlt::RflFeettoM(cflAlt, dspAlt)) { // is metric RVSM
				sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt / 10));
			}
			else if (cflAlt % 1000) { // not metric RVSM nor FL xx0, convert to metric
				dspAlt = MetricAlt::FeettoM(cflAlt) / 10;
				sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt));
			}
			else { // FL xx0, show FL in feet
				dspAlt = cflAlt / 100;
				sprintf_s(sItemString, 5, "F%03d", OVRFLW3(dspAlt));
			}
			break; }
		}
		if (FlightPlan.GetTrackingControllerIsMe() && m_CFLConfirmMap[FlightPlan.GetCallsign()]) {
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 255); // white
		}
		break; }
	case TAG_ITEM_TYPE_RFL_ICAO: {
		int rflAlt = FlightPlan.GetFinalAltitude();
		int dspMtr;
		if (MetricAlt::RflFeettoM(rflAlt, dspMtr)) { // is metric RVSM
			char trsMrk = rflAlt >= GetTransitionAltitude() ? 'S' : 'M';
			sprintf_s(sItemString, 6, "%c%04d", trsMrk, OVRFLW4(dspMtr / 10));
		}
		else {
			rflAlt = round(rflAlt / 100.0);
			sprintf_s(sItemString, 5, "F%03d", OVRFLW3(rflAlt));
		}

		break; }
	case TAG_ITEM_TYPE_SC_IND: {
		if (m_SimilarCallsignSet.find(RadarTarget.GetCallsign()) != m_SimilarCallsignSet.end()) { // there is similar
			sprintf_s(sItemString, 3, "SC");
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 0); // yellow
		}

		break; }
	case TAG_ITEM_TYPE_RFL_IND: {
		int rflAlt = FlightPlan.GetFinalAltitude();
		int _meter;
		if (!MetricAlt::RflFeettoM(rflAlt, _meter))  // not metric RVSM
			sprintf_s(sItemString, 2, "#");

		break; }
	case TAG_ITEM_TYPE_RVSM_IND: {
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
		if (FlightPlan.GetTrackingControllerIsMe() && RadarTarget.GetPosition().GetTransponderC()
			&& !m_ComEstbMap[RadarTarget.GetCallsign()]) {
			sprintf_s(sItemString, 2, "C");
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 255); // white
		}
		break; }
	case TAG_ITEM_TYPE_RECAT: {
		char categ = FlightPlan.GetFlightPlanData().GetAircraftWtc();
		string acType = FlightPlan.GetFlightPlanData().GetAircraftFPType();
		if (categ == 'H' && m_ReCatMap.count(acType))
			sprintf_s(sItemString, 3, "-%c", m_ReCatMap.at(acType));

		break; }
	case TAG_ITEM_TYPE_RTE_CHECK: {
		if (m_RouteChecker == nullptr) break;
		char rc = m_RouteChecker->CheckFlightPlan(FlightPlan);
		sprintf_s(sItemString, 2, "%c", rc);
		if (rc != 'Y' && rc != '?' && rc != ' ') {
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 0); // yellow
		}

		break; }
	case TAG_ITEM_TYPE_SQ_DUPE: {
		if (m_SquawkDupeSet.find(RadarTarget.GetCallsign()) != m_SquawkDupeSet.end()) {
			sprintf_s(sItemString, 5, "DUPE");
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = RGB(255, 255, 0); // yellow
		}
		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	CRadarTarget RadarTarget = RadarTargetSelectASEL();
	CFlightPlan FlightPlan = FlightPlanSelectASEL();
	if (!RadarTarget.IsValid() && !FlightPlan.IsValid()) return;

	switch (FunctionId)
	{
	case TAG_ITEM_FUNCTION_COMM_ESTAB: {
		if (FlightPlan.GetTrackingControllerIsMe() && RadarTarget.GetPosition().GetTransponderC())
			m_ComEstbMap[RadarTarget.GetCallsign()] = true;

		break; }
	case TAG_ITEM_FUNCTION_CFL_SET: {
		string input = sItemString;
		int tgtAlt = -1;
		if (input == "NONE")
			tgtAlt = 0;
		else if (input == "ILS")
			tgtAlt = 1;
		else if (input == "VA")
			tgtAlt = 2;
		else {
			bool f = input[0] == 'F' || input[0] == 'f'; // whether 'F' is present and at first place
			bool d = false; // whether '.'(dot) is present
			int alt = 0;
			for (size_t i = f; i < input.size(); i++) { // parse input
				if (input[i] >= '0' && input[i] <= '9') {
					alt = alt * 10 + input[i] - '0';
				}
				else {
					d = input[i] == '.';
					break;
				}
			}
			if (alt <= 0); // invalid entry, only NONE from menu will clear CFL
			else if (f && d) // FL in feet, precise
				tgtAlt = alt;
			else if (f && !d) // FL in feet
				tgtAlt = alt * 100;
			else if (!f && d) // metric, precise
				tgtAlt = MetricAlt::LvlMtoFeet(alt);
			else if (!f && !d)  // otherwise metric
				tgtAlt = MetricAlt::LvlMtoFeet(alt * 100);
		}
		FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow

		break; }
	case TAG_ITEM_FUNCTION_CFL_MENU: {
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		else if (FlightPlan.GetTrackingControllerIsMe() && m_CFLConfirmMap[FlightPlan.GetCallsign()]) {
			// confirm previous CFL first
			m_CFLConfirmMap[FlightPlan.GetCallsign()] = false;
			break;
		}
		OpenPopupList(Area, "CFL Menu", 2);
		// pre-select altitude
		int minDif(1e6), minAlt(0); // a big enough number
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int rdrAlt = GetRadarDisplayAltitude(RadarTarget);
		int cmpAlt = cflAlt <= 2 ? rdrAlt : cflAlt;
		for (auto it = MetricAlt::m_ftom.begin(); it != MetricAlt::m_ftom.end(); it++) {
			int dif = abs(it->first - cmpAlt);
			if (dif < minDif) {
				minDif = dif;
				minAlt = it->first;
			}
		}
		printf_s("cfl:%d,rdr:%d,cmp:%d,min:%d", cflAlt, rdrAlt, cmpAlt, minAlt);
		for (auto it = MetricAlt::m_mtof.rbegin(); it != MetricAlt::m_mtof.rend(); it++) {
			int m = it->first / 100;
			int f = it->second / 100;
			char ms[4], fs[4];
			sprintf_s(ms, 4, "%d", m);
			sprintf_s(fs, 4, "%d", f);
			AddPopupListElement(ms, fs, TAG_ITEM_FUNCTION_CFL_SET, it->second == minAlt, POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		AddPopupListElement("[   ", "  ]", TAG_ITEM_FUNCTION_CFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		AddPopupListElement("ILS", "", TAG_ITEM_FUNCTION_CFL_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		AddPopupListElement("VA", "", TAG_ITEM_FUNCTION_CFL_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		AddPopupListElement("NONE", "", TAG_ITEM_FUNCTION_CFL_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break; }
	case TAG_ITEM_FUNCTION_CFL_EDIT: {
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_CFL_SET, "");

		break; }
	case TAG_ITEM_FUNCTION_RFL_SET: {
		CFlightPlanData fplData = FlightPlan.GetFlightPlanData();
		CFlightPlanControllerAssignedData ctrData = FlightPlan.GetControllerAssignedData();
		if (strlen(sItemString)) {
			int alt;
			if (sscanf_s(sItemString, "F%3d", &alt) || sscanf_s(sItemString, "f%3d", &alt)) { // FL in feet
				alt = alt * 100;
				fplData.SetFinalAltitude(alt);
				ctrData.SetFinalAltitude(alt);
			}
			else if (sscanf_s(sItemString, "%3d", &alt)) { // otherwise Metric
				alt = MetricAlt::LvlMtoFeet(alt * 100);
				fplData.SetFinalAltitude(alt);
				ctrData.SetFinalAltitude(alt);
			}
		}
		break; }
	case TAG_ITEM_FUNCTION_RFL_MENU: {
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupList(Area, "RFL Menu", 2);
		// pre-select altitude
		int minDif(1e6), minAlt(0); // a big enough number
		int rflAlt = FlightPlan.GetFinalAltitude();
		for (auto it = MetricAlt::m_ftom.begin(); it != MetricAlt::m_ftom.end(); it++) {
			int dif = abs(it->first - rflAlt);
			if (dif < minDif) {
				minDif = dif;
				minAlt = it->first;
			}
		}
		for (auto it = MetricAlt::m_mtof.rbegin(); it != MetricAlt::m_mtof.rend(); it++) {
			int m = it->first / 100;
			int f = it->second / 100;
			char ms[4], fs[4];
			sprintf_s(ms, 4, "%d", m);
			sprintf_s(fs, 4, "%d", f);
			AddPopupListElement(ms, fs, TAG_ITEM_FUNCTION_RFL_SET, it->second == minAlt, POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		AddPopupListElement("[   ", "  ]", TAG_ITEM_FUNCTION_RFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break; }
	case TAG_ITEM_FUNCTION_RFL_EDIT: {
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_RFL_SET, "");

		break; }
	case TAG_ITEM_FUNCTION_SC_LIST: {
		string cs0 = RadarTarget.GetCallsign();
		if (m_SimilarCallsignSet.find(cs0) == m_SimilarCallsignSet.end()) // not a SC
			break;
		OpenPopupList(Area, "SC List", 1);
		AddPopupListElement(cs0.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT, true);
		bool cs0isCHN = IsChineseCallsign(FlightPlan);
		for (CRadarTarget rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt)) {
			string cs1 = rt.GetCallsign();
			if (cs0 != cs1 &&
				m_SimilarCallsignSet.find(cs1) != m_SimilarCallsignSet.end() &&
				cs0isCHN == IsChineseCallsign(rt.GetCorrelatedFlightPlan()) &&
				CompareCallsign(cs0, cs1))
				// also a SC
				AddPopupListElement(cs1.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT);
		}
		break; }
	case TAG_ITEM_FUNCTION_SC_SELECT: {
		SetASELAircraft(FlightPlanSelect(sItemString));

		break; }
	case TAG_ITEM_FUNCTION_RTE_INFO: {
		if (m_RouteChecker == nullptr) break;
		char rc = m_RouteChecker->CheckFlightPlan(FlightPlan);
		if (rc == 'Y' || rc == ' ') break; // no need to show ok routes and cleared routes
		DisplayRouteMessage(FlightPlan.GetFlightPlanData().GetOrigin(), FlightPlan.GetFlightPlanData().GetDestination());

		break; }
	default:
		break;
	}
}

void CMTEPlugIn::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
	if (!FlightPlan.IsValid())
		return;
	if (DataType == CTR_DATA_TYPE_TEMPORARY_ALTITUDE &&
		FlightPlan.GetTrackingControllerIsMe() &&
		FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetTransponderC() &&
		(IsCFLAssigned(FlightPlan) || FlightPlan.GetControllerAssignedData().GetClearedAltitude())
		) {
		// initiate CFL to be confirmed
		m_CFLConfirmMap[FlightPlan.GetCallsign()] = true;
	}
}

void CMTEPlugIn::OnTimer(int Counter)
{
	if (Counter % TIMER_REFRESH_INTERVAL) return;

	if (initCursor && customCursor) SetCustomCursor(); // cursor

	unordered_set<string> setCN, setEN;
	unordered_map<string, string> mapSQ; // first string is squawk
	m_SquawkDupeSet.clear();
	for (CRadarTarget rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt)) {
		CFlightPlan fp = rt.GetCorrelatedFlightPlan();
		if (fp.GetTrackingControllerIsMe() && rt.GetPosition().GetTransponderC()) { // in the air and tracked by me
			// for similar callsign
			if (!fp.IsTextCommunication()) {
				if (IsChineseCallsign(fp))
					setCN.insert(rt.GetCallsign());
				else
					setEN.insert(rt.GetCallsign());
			}
			// for duplicated callsign
			string sq = rt.GetPosition().GetSquawk();
			if (mapSQ.find(sq) != mapSQ.end()) {
				m_SquawkDupeSet.insert(mapSQ[sq]);
				m_SquawkDupeSet.insert(rt.GetCallsign());
			}
			else {
				mapSQ[sq] = rt.GetCallsign();
			}
		}
		else {
			// for communication map and CFL confirm map
			m_ComEstbMap[rt.GetCallsign()] = false;
			m_CFLConfirmMap[fp.GetCallsign()] = false;
		}
	}
	m_SimilarCallsignSet.clear();
	m_SimilarCallsignSet.merge(ParseSimilarCallsignSet(setCN));
	m_SimilarCallsignSet.merge(ParseSimilarCallsignSet(setEN));
}

bool CMTEPlugIn::OnCompileCommand(const char* sCommandLine) {
	string cmd = sCommandLine;
	smatch match; // all regular expressions will ignore cases

	// custom cursor
	regex rxcc0("^.MTEP CURSOR OFF$", regex_constants::icase);
	regex rxcc1("^.MTEP CURSOR ON$", regex_constants::icase);
	if (regex_match(cmd, match, rxcc0)) {
		if (!customCursor) return true;
		customCursor = false;
		CancelCustomCursor();
		SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", "0");
		return true;
	}
	else if (regex_match(cmd, match, rxcc1)) {
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

	// flightradar24 and variflight
	regex rxfr("^.MTEP FR24 ([A-Z]{4})$", regex_constants::icase);
	regex rxvr("^.MTEP VARI ([A-Z]{4})$", regex_constants::icase);
	bool mf, mv;
	mf = regex_match(cmd, match, rxfr);
	if (!mf)
		mv = regex_match(cmd, match, rxvr);
	if (mf || mv) {
		string airport, url_base, url_full;
		airport = MakeUpper(match[1].str());
		if (mf) {
			url_base = "https://www.flightradar24.com/";
		}
		else if (mv) {
			url_base = "https://flightadsb.variflight.com/tracker/";
		}
		for (CSectorElement se = SectorFileElementSelectFirst(SECTOR_ELEMENT_AIRPORT);
			se.IsValid(); se = SectorFileElementSelectNext(se, SECTOR_ELEMENT_AIRPORT)) {
			if (airport == se.GetName()) {
				CPosition pos;
				se.GetPosition(&pos, 0);
				if (mf)
					url_full = url_base + to_string(pos.m_Latitude) + "," + to_string(pos.m_Longitude) + "/9";
				else if (mv)
					url_full = url_base + to_string(pos.m_Longitude) + "," + to_string(pos.m_Latitude) + "/9";
				ShellExecute(NULL, "open", url_full.c_str(), NULL, NULL, SW_SHOW);
				return true;
			}
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
		return msg.size();
	}

	return false;
}

int CMTEPlugIn::GetRadarDisplayAltitude(CRadarTarget RadarTarget) {
	// Radar Display Altitude, will consider transition level/altitude
	int trsAlt = GetTransitionAltitude(); // should be transition level
	int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
	int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
	return stdAlt >= trsAlt ? stdAlt : qnhAlt;
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

void CMTEPlugIn::LoadRouteChecker(string filename)
{
	UnloadRouteChecker();
	if (filename[0] == '@') {
		// for relative directory to dll file, only works for original dll file name (MTEPlugin.dll)
		HMODULE hMod = GetModuleHandle(PLUGIN_FILE);
		if (hMod != NULL) {
			TCHAR szBuffer[MAX_PATH] = { 0 };
			GetModuleFileName(hMod, szBuffer, sizeof(szBuffer) / sizeof(TCHAR) - 1);
			string nfname = szBuffer;
			filename = nfname.substr(0, nfname.find(PLUGIN_FILE)) + filename.substr(1);
		}
	}
	try {
		m_RouteChecker = new RouteChecker(filename);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route Checker is loaded successfully. CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("Route Checker failed to load (" + e + "). CSV file name: " + filename).c_str(),
			1, 0, 0, 0, 0);
		UnloadRouteChecker();
	}
}

void CMTEPlugIn::UnloadRouteChecker(void)
{
	if (m_RouteChecker != nullptr) {
		delete m_RouteChecker;
		m_RouteChecker = nullptr;
		DisplayUserMessage("MESSAGE", "MTEPlugin", "Route Checker is unloaded!", 1, 0, 0, 0, 0);
	}
}

string CMTEPlugIn::DisplayRouteMessage(string departure, string arrival)
{
	if (m_RouteChecker == nullptr) return "";
	auto rinfo = m_RouteChecker->GetRouteInfo(departure, arrival);
	if (!rinfo.size()) return "";
	string res = departure + "-" + arrival;
	DisplayUserMessage("MTEP-Route", res.c_str(), (to_string(rinfo.size()) + " route(s):").c_str(), 1, 1, 0, 0, 0);
	for (auto ri : rinfo) {
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

int CalculateVerticalSpeed(CRadarTarget RadarTarget)
{
	CRadarTargetPositionData curpos = RadarTarget.GetPosition();
	CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
	double curAlt = curpos.GetPressureAltitude();
	double preAlt = prepos.GetPressureAltitude();
	double preT = prepos.GetReceivedTime();
	double curT = curpos.GetReceivedTime();
	double deltaT = preT - curT;
	deltaT = deltaT ? deltaT : INFINITE;
	return round((curAlt - preAlt) / deltaT * 60.0);
}

bool IsCFLAssigned(CFlightPlan FlightPlan)
{
	// tell when cleared altitude is 0, is CFL not assigned or assigned to RFL
	// true means CFL is assigned to RFL, false means no CFL
	bool noCfl;
	if (strlen(FlightPlan.GetTrackingControllerCallsign())) { // tracked by someone
		CRadarTargetPositionData rdpos = FlightPlan.GetCorrelatedRadarTarget().GetPosition();
		string squawk = rdpos.GetSquawk();
		if (squawk == "7700" || squawk == "7600" || squawk == "7500" || !rdpos.GetTransponderC())
			// emergency or on the ground
			noCfl = false;
		else // in air no emergency
			noCfl = true;
	}
	else { // not tracked
		noCfl = false;
	}
	return noCfl;
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
