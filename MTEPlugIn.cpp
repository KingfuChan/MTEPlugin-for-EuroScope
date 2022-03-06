// MTEPlugin.cpp

#include "pch.h"
#include "MTEPlugin.h"

#ifndef COPYRIGHTS
#define PLUGIN_NAME "MTEPlugin"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2022 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/MTEPlugin-for-EuroScope"
#endif // !COPYRIGHTS

// VERSION INFO
const char VERSION_MAIN[] = {
	VERSION_MAJOR_INIT,
	'.',
	VERSION_MINOR_INIT,
	'.',
	VERSION_REVISION_INIT,
};
#ifdef _DEBUG
const char VERSION_BUILD[] = ".DEBUG";
#else
const char VERSION_BUILD[] =
{
	'.',
	BUILD_YEAR_CH2, BUILD_YEAR_CH3,
	BUILD_MONTH_CH0, BUILD_MONTH_CH1,
	BUILD_DAY_CH0, BUILD_DAY_CH1,
	BUILD_HOUR_CH0, BUILD_HOUR_CH1,
	BUILD_MIN_CH0, BUILD_MIN_CH1,
	'\0'
};
#endif


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
const int TAG_ITEM_TYPE_RECAT = 12; // RECAT-CN
const int TAG_ITEM_TYPE_RTE_CHECK = 13; // Route validity
const int TAG_ITEM_TYPE_SQ_DUPE = 14; // Tracked DUPE warning
const int TAG_ITEM_TYPE_DEP_SEQ = 15; // Departure sequence
const int TAG_ITEM_TYPE_RVEC_IND = 16; // Radar vector indicator
const int TAG_ITEM_TYPE_CFL_MTR = 17; // Cleared flight level (m)
const int TAG_ITEM_TYPE_RCNT_IND = 18; // Reconnected indicator

// TAG ITEM FUNCTION
const int TAG_ITEM_FUNCTION_COMM_ESTAB = 1; // Set COMM ESTB
const int TAG_ITEM_FUNCTION_RCNT_RST = 2; // Restore assigned data
const int TAG_ITEM_FUNCTION_CFL_SET = 10; // Set CFL (not registered)
const int TAG_ITEM_FUNCTION_CFL_MENU = 11; // Open CFL popup menu
const int TAG_ITEM_FUNCTION_CFL_EDIT = 12; // Open CFL popup edit
const int TAG_ITEM_FUNCTION_RFL_SET = 20; // Set RFL (not registered)
const int TAG_ITEM_FUNCTION_RFL_MENU = 21; // Open RFL popup menu
const int TAG_ITEM_FUNCTION_RFL_EDIT = 22; // Open RFL popup edit (not registered)
const int TAG_ITEM_FUNCTION_SC_LIST = 30; // Open similar callsign list
const int TAG_ITEM_FUNCTION_SC_SELECT = 31; // Select in similar callsign list (not registered)
const int TAG_ITEM_FUNCTION_RTE_INFO = 40; // Show route checker info
const int TAG_ITEM_FUNCTION_DSQ_MENU = 50; // Set departure sequence
const int TAG_ITEM_FUNCTION_DSQ_EDIT = 51; // Open departure sequence popup edit (not registered)
const int TAG_ITEM_FUNCTION_SPD_SET = 60; // Set assigned speed (not registerd)
const int TAG_ITEM_FUNCTION_SPD_LIST = 61; // Open assigned speed popup list

// COMPUTERISING RELATED
constexpr double KN_KPH(double k) { return 1.85184 * k; } // 1 knot = 1.85184 kph
constexpr double KPH_KN(double k) { return k / 1.85184; } // 1.85184 kph = 1 knot
constexpr int OVRFLW2(int t) { return abs(t) > 99 ? 99 : abs(t); } // overflow pre-process 2 digits
constexpr int OVRFLW3(int t) { return abs(t) > 999 ? 999 : abs(t); } // overflow pre-process 3 digits
constexpr int OVRFLW4(int t) { return abs(t) > 9999 ? 9999 : abs(t); }  // overflow pre-process 4 digits
string MakeUpper(string str);
int CalculateVerticalSpeed(CRadarTarget RadarTarget);
bool IsCFLAssigned(CFlightPlan FlightPlan);

// SETTING NAMES
const char* SETTING_CUSTOM_CURSOR = "CustomCursor";
const char* SETTING_ROUTE_CHECKER_CSV = "RteCheckerCSV";
const char* SETTING_AUTO_RETRACK = "AutoRetrack";

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
		VERSION_MAIN,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	DWORD dwCurPID = GetCurrentProcessId();
	EnumWindows(EnumWindowsProc, (LPARAM)&dwCurPID); // to set pluginWindow
	pluginModule = AfxGetInstanceHandle();

	const char* setcc = GetDataFromSettings(SETTING_CUSTOM_CURSOR);
	pluginCursor = CopyCursor(LoadImage(pluginModule, MAKEINTRESOURCE(IDC_CURSORCROSS), IMAGE_CURSOR, 0, 0, LR_SHARED));
	m_CustomCursor = false;
	if (setcc == nullptr ? false : stoi(setcc))
		SetCustomCursor();

	m_RouteChecker = nullptr;
	const char* setrc = GetDataFromSettings(SETTING_ROUTE_CHECKER_CSV);
	if (setrc != nullptr)
		LoadRouteChecker(setrc);

	m_DepartureSequence = nullptr;
	m_TrackedRecorder = new TrackedRecorder();
	const char* setar = GetDataFromSettings(SETTING_AUTO_RETRACK);
	m_AutoRetrack = setar == nullptr ? false : stoi(setar);

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
	RegisterTagItemType("RECAT-CN", TAG_ITEM_TYPE_RECAT);
	RegisterTagItemType("Route validity", TAG_ITEM_TYPE_RTE_CHECK);
	RegisterTagItemType("Tracked DUPE warning", TAG_ITEM_TYPE_SQ_DUPE);
	RegisterTagItemType("Departure sequence", TAG_ITEM_TYPE_DEP_SEQ);
	RegisterTagItemType("Radar vector indicator", TAG_ITEM_TYPE_RVEC_IND);
	RegisterTagItemType("Cleared flight level (m)", TAG_ITEM_TYPE_CFL_MTR);
	RegisterTagItemType("Reconnected indicator", TAG_ITEM_TYPE_RCNT_IND);

	RegisterTagItemFunction("Set COMM ESTB", TAG_ITEM_FUNCTION_COMM_ESTAB);
	RegisterTagItemFunction("Restore assigned data", TAG_ITEM_FUNCTION_RCNT_RST);
	RegisterTagItemFunction("Open CFL popup menu", TAG_ITEM_FUNCTION_CFL_MENU);
	RegisterTagItemFunction("Open CFL popup edit", TAG_ITEM_FUNCTION_CFL_EDIT);
	RegisterTagItemFunction("Open RFL popup menu", TAG_ITEM_FUNCTION_RFL_MENU);
	RegisterTagItemFunction("Open similar callsign list", TAG_ITEM_FUNCTION_SC_LIST);
	RegisterTagItemFunction("Show route checker info", TAG_ITEM_FUNCTION_RTE_INFO);
	RegisterTagItemFunction("Set departure sequence", TAG_ITEM_FUNCTION_DSQ_MENU);
	RegisterTagItemFunction("Open assigned speed popup list", TAG_ITEM_FUNCTION_SPD_LIST);

	DisplayUserMessage("MESSAGE", "MTEPlugin",
		(string("MTEPlugin finished loading (v") + VERSION_MAIN + VERSION_BUILD + string(")! For help please refer to ") + GITHUB_LINK).c_str(),
		1, 0, 0, 0, 0);
}

CMTEPlugIn::~CMTEPlugIn(void)
{
	UnloadRouteChecker();
	CancelCustomCursor();
	if (m_TrackedRecorder != nullptr)
		delete m_TrackedRecorder;
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
		int rdrAlt = GetRadarDisplayAltitude(RadarTarget);
		int dspAlt;
		if (!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign())) {
			dspAlt = (int)round(MetricAlt::FeettoM(rdrAlt) / 10.0);
			sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt));
		}
		else {
			dspAlt = (int)round(rdrAlt / 100.0);
			sprintf_s(sItemString, 5, "F%3d", OVRFLW3(dspAlt));
		}
		break; }
	case TAG_ITEM_TYPE_CFL_FLX: {
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
			if (!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign())) {
				if (MetricAlt::RflFeettoM(cflAlt, dspAlt)) { // is metric RVSM
					sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt / 10));
					break;
				}
				else if (cflAlt % 1000) { // not metric RVSM nor FL xx0, convert to metric
					dspAlt = MetricAlt::FeettoM(cflAlt) / 10;
					sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt));
					break;
				}
			}
			// FL xx0, show FL in feet
			dspAlt = cflAlt / 100;
			sprintf_s(sItemString, 5, "F%03d", OVRFLW3(dspAlt));
			break; }
		}
		if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			*pColorCode = TAG_COLOR_REDUNDANT;
		}
		break; }
	case TAG_ITEM_TYPE_CFL_MTR: {
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int dspAlt = MetricAlt::LvlFeettoM(cflAlt);
		sprintf_s(sItemString, 5, "%04d", OVRFLW4(dspAlt / 10));

		break; }
	case TAG_ITEM_TYPE_RFL_ICAO: {
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
		if (m_TrackedRecorder->IsSimilarCallsign(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 3, "SC");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_RFL_IND: {
		if (!FlightPlan.GetTrackingControllerIsMe()) break;
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
		if (!m_TrackedRecorder->IsCommEstablished(RadarTarget.GetCallsign())) {
			sprintf_s(sItemString, 2, "C");
			*pColorCode = TAG_COLOR_REDUNDANT;
		}
		break; }
	case TAG_ITEM_TYPE_RECAT: {
		char categ = FlightPlan.GetFlightPlanData().GetAircraftWtc();
		string acType = FlightPlan.GetFlightPlanData().GetAircraftFPType();
		auto rc = m_ReCatMap.find(acType);
		if (categ == 'H' && rc != m_ReCatMap.end())
			sprintf_s(sItemString, 3, "-%c", rc->second);

		break; }
	case TAG_ITEM_TYPE_RTE_CHECK: {
		if (m_RouteChecker == nullptr) break;
		char rc = m_RouteChecker->CheckFlightPlan(FlightPlan);
		sprintf_s(sItemString, 2, "%c", rc);
		if (rc != 'Y' && rc != '?' && rc != ' ') {
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_SQ_DUPE: {
		if (m_TrackedRecorder->IsSquawkDUPE(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 5, "DUPE");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_DEP_SEQ: {
		if (m_DepartureSequence == nullptr) m_DepartureSequence = new DepartureSequence();
		int seq = m_DepartureSequence->GetSequence(FlightPlan);
		if (seq > 0)
			sprintf_s(sItemString, 3, "%02d", OVRFLW2(seq));
		else if (seq < 0) { // reconnected
			sprintf_s(sItemString, 3, "--");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_RVEC_IND: {
		if (FlightPlan.GetTrackingControllerIsMe() && FlightPlan.GetControllerAssignedData().GetAssignedHeading()) {
			sprintf_s(sItemString, 3, "RV");
			*pColorCode = TAG_COLOR_INFORMATION;
		}
		break; }
	case TAG_ITEM_TYPE_RCNT_IND: {
		if (m_TrackedRecorder->IsReconnected(FlightPlan.GetCallsign())) {
			sprintf_s(sItemString, 2, "r");
			*pColorCode = TAG_COLOR_INFORMATION;
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
		m_TrackedRecorder->SetCommEstablished(FlightPlan.GetCallsign());

		break; }
	case TAG_ITEM_FUNCTION_RCNT_RST: {
		m_TrackedRecorder->SetTrackedData(FlightPlan);

		break; }
	case TAG_ITEM_FUNCTION_CFL_SET: {
		string input = sItemString;
		if (MakeUpper(input) == "F") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), true);
			break;
		}
		else if (MakeUpper(input) == "M") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), false);
			break;
		}
		int tgtAlt = -1;
		if (input == "NONE")
			tgtAlt = 0;
		else if (input == "ILS") {
			tgtAlt = 1;
			FlightPlan.GetControllerAssignedData().SetAssignedHeading(0);
		}
		else if (input == "VA") {
			tgtAlt = 2;
			FlightPlan.GetControllerAssignedData().SetAssignedHeading(0);
		}
		else {
			// use regular expressions to match input
			regex rxfd("^F([0-9]+)\\.$", regex_constants::icase);
			regex rxf("^F([0-9]+)$", regex_constants::icase);
			regex rxd("^([0-9]+)\\.$", regex_constants::icase);
			regex rxn("^([0-9]+)$", regex_constants::icase);
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
		}
		FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow

		break; }
	case TAG_ITEM_FUNCTION_CFL_MENU: {
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		else if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			// confirm previous CFL first
			m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign());
			break;
		}
		OpenPopupList(Area, "CFL Menu", 2);
		// pre-select altitude
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int rdrAlt = GetRadarDisplayAltitude(RadarTarget);
		int cmpAlt = cflAlt <= 2 ? rdrAlt : cflAlt;
		if (!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign())) {
			// metric
			int minDif(100000), minAlt(0); // a big enough number
			for (auto it = MetricAlt::m_ftom.begin(); it != MetricAlt::m_ftom.end(); it++) {
				int dif = abs(it->first - cmpAlt);
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
				AddPopupListElement(ms, fs, TAG_ITEM_FUNCTION_CFL_SET, it->second == minAlt, POPUP_ELEMENT_NO_CHECKBOX, false, false);
			}
		}
		else {
			// FLs
			for (int fl = 430; fl >= 100; fl -= 10) {
				char fs[5];
				sprintf_s(fs, 5, "F%d", fl);
				AddPopupListElement(fs, "", TAG_ITEM_FUNCTION_CFL_SET, fl / 10 == (int)round(cmpAlt / 1000.0), POPUP_ELEMENT_NO_CHECKBOX, false, false);
			}
		}
		AddPopupListElement("[   ", "  ]", TAG_ITEM_FUNCTION_CFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		AddPopupListElement("ILS", "", TAG_ITEM_FUNCTION_CFL_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		AddPopupListElement("VA", "", TAG_ITEM_FUNCTION_CFL_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);
		AddPopupListElement("NONE", "", TAG_ITEM_FUNCTION_CFL_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break; }
	case TAG_ITEM_FUNCTION_CFL_EDIT: {
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		else if (!m_TrackedRecorder->IsCFLConfirmed(FlightPlan.GetCallsign())) {
			// confirm previous CFL first
			m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign());
			break;
		}
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_CFL_SET, "");

		break; }
	case TAG_ITEM_FUNCTION_RFL_SET: {
		CFlightPlanControllerAssignedData ctrData = FlightPlan.GetControllerAssignedData();
		string input = sItemString;
		if (MakeUpper(input) == "F") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), true);
			break;
		}
		else if (MakeUpper(input) == "M") {
			m_TrackedRecorder->SetAltitudeUnit(FlightPlan.GetCallsign(), false);
			break;
		}
		regex rxm("^([0-9]{1,3})$");
		regex rxf("^F([0-9]{1,3})$", regex_constants::icase);
		smatch match;
		if (regex_match(input, match, rxf)) {
			ctrData.SetFinalAltitude(stoi(match[1]) * 100);
		}
		else if (regex_match(input, match, rxm)) {
			ctrData.SetFinalAltitude(MetricAlt::LvlMtoFeet(stoi(match[1]) * 100));
		}
		break; }
	case TAG_ITEM_FUNCTION_RFL_MENU: {
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupList(Area, "RFL Menu", 2);
		int rflAlt = FlightPlan.GetFinalAltitude();
		if (!m_TrackedRecorder->IsForceFeet(FlightPlan.GetCallsign())) {
			// metric
			// pre-select altitude
			int minDif(100000), minAlt(0); // a big enough number
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
		}
		else {
			// FLs
			for (int fl = 430; fl >= 100; fl -= 10) {
				char fs[5];
				sprintf_s(fs, 5, "F%d", fl);
				AddPopupListElement(fs, "", TAG_ITEM_FUNCTION_RFL_SET, fl / 10 == (int)round(rflAlt / 1000.0), POPUP_ELEMENT_NO_CHECKBOX, false, false);
			}
		}
		AddPopupListElement("[   ", "  ]", TAG_ITEM_FUNCTION_RFL_EDIT, false, POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break; }
	case TAG_ITEM_FUNCTION_RFL_EDIT: {
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_RFL_SET, "");

		break; }
	case TAG_ITEM_FUNCTION_SC_LIST: {
		string cs = RadarTarget.GetCallsign();
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
		if (m_RouteChecker == nullptr) break;
		char rc = m_RouteChecker->CheckFlightPlan(FlightPlan);
		if (rc == 'Y' || rc == ' ') break; // no need to show ok routes and cleared routes
		DisplayRouteMessage(FlightPlan.GetFlightPlanData().GetOrigin(), FlightPlan.GetFlightPlanData().GetDestination());

		break; }
	case TAG_ITEM_FUNCTION_DSQ_MENU: {
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
		int seq;
		if (sscanf_s(sItemString, "%d", &seq) != 1) break;
		if (seq >= 0) {
			m_DepartureSequence->EditSequence(FlightPlan, seq);
		}
		break;	}
	case TAG_ITEM_FUNCTION_SPD_SET: {
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
		if (!FlightPlan.GetTrackingControllerIsMe() && strlen(FlightPlan.GetTrackingControllerId())) {
			// don't show list if other controller is tracking
			break;
		}
		int alt = MetricAlt::FeettoM(GetRadarDisplayAltitude(RadarTarget));
		if (alt <= 7530) { // use IAS
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
	if (FlightPlan.GetTrackingControllerIsMe()) {
		m_TrackedRecorder->UpdateFlight(FlightPlan);
		if (DataType == CTR_DATA_TYPE_TEMPORARY_ALTITUDE &&
			FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetTransponderC() &&
			(IsCFLAssigned(FlightPlan) || FlightPlan.GetControllerAssignedData().GetClearedAltitude())) {
			// initiate CFL to be confirmed
			m_TrackedRecorder->SetCFLConfirmed(FlightPlan.GetCallsign(), false);
		}
	}
	if (DataType == CTR_DATA_TYPE_GROUND_STATE && m_DepartureSequence != nullptr) {
		m_DepartureSequence->EditSequence(FlightPlan, 0);
	}
}

void CMTEPlugIn::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid()) return;
	if (m_DepartureSequence != nullptr)
		m_DepartureSequence->EditSequence(FlightPlan, -1);
	if (FlightPlan.GetTrackingControllerIsMe())
		m_TrackedRecorder->UpdateFlight(FlightPlan, false);
}

void CMTEPlugIn::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	auto FlightPlan = RadarTarget.GetCorrelatedFlightPlan();
	if (!FlightPlan.IsValid())
		return;
	m_TrackedRecorder->UpdateFlight(FlightPlan);
	if (m_AutoRetrack && m_TrackedRecorder->IsReconnected(FlightPlan.GetCallsign())) {
		m_TrackedRecorder->SetTrackedData(FlightPlan);
		if (m_AutoRetrack == 2) {
			string msg = string(FlightPlan.GetCallsign()) + " reconnected and is re-tracked.";
			DisplayUserMessage("MTEP-Recorder", "MTEPlugin", msg.c_str(), 1, 1, 0, 0, 0);
		}
	}
}

bool CMTEPlugIn::OnCompileCommand(const char* sCommandLine) {
	string cmd = sCommandLine;
	smatch match; // all regular expressions will ignore cases

	// custom cursor
	regex rxcc0("^.MTEP CURSOR OFF$", regex_constants::icase);
	regex rxcc1("^.MTEP CURSOR ON$", regex_constants::icase);
	if (regex_match(cmd, match, rxcc0)) {
		CancelCustomCursor();
		SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", "0");
		return true;
	}
	else if (regex_match(cmd, match, rxcc1)) {
		CancelCustomCursor();
		SetCustomCursor();
		SaveDataToSettings(SETTING_CUSTOM_CURSOR, "set custom mouse cursor", "1");
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
		if (pluginModule != nullptr) {
			TCHAR pBuffer[MAX_PATH] = { 0 };
			GetModuleFileName(pluginModule, pBuffer, sizeof(pBuffer) / sizeof(TCHAR) - 1);
			string currentPath = pBuffer;
			filename = currentPath.substr(0, currentPath.find_last_of("\\/") + 1) + filename.substr(1);
		}
	}
	try {
		m_RouteChecker = new RouteChecker(filename);
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
	m_TrackedRecorder = new TrackedRecorder();
	DisplayUserMessage("MESSAGE", "MTEPlugin", "Tracked recorder is reset!", 1, 0, 0, 0, 0);
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
	return (int)round((curAlt - preAlt) / deltaT * 60.0);
}

bool IsCFLAssigned(CFlightPlan FlightPlan)
{
	// tell when cleared altitude is 0, is CFL not assigned or assigned to RFL
	// true means CFL is assigned to RFL, false means no CFL
	if (strlen(FlightPlan.GetTrackingControllerCallsign())) { // tracked by someone
		CRadarTargetPositionData rdpos = FlightPlan.GetCorrelatedRadarTarget().GetPosition();
		string squawk = rdpos.GetSquawk();
		if (!(squawk == "7700" || squawk == "7600" || squawk == "7500" || !rdpos.GetTransponderC()))
			return true;
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
