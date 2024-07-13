// MTEPlugin.cpp

#pragma once

#include "pch.h"
#include "Version.h"
#include "MTEPlugin.h"

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
		VERSION_FILE_STR,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	DisplayUserMessage("MESSAGE", "MTEPlugin",
		std::format("Version {} loaded.", VERSION_DISPLAY).c_str(),
		1, 0, 0, 0, 0);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	DWORD dwCurPID = GetCurrentProcessId();
	EnumWindows(EnumWindowsProc, (LPARAM)&dwCurPID); // to set pluginWindow
	pluginModule = AfxGetInstanceHandle();

	UINT dpiSys = GetDpiForWindow(GetDesktopWindow());
	UINT dpiWnd = GetDpiForWindow(pluginWindow);
	int curSize = (int)((float)(dpiSys < 96 ? 96 : dpiSys) / (float)(dpiWnd < 96 ? 96 : dpiWnd) * 32.0);
	pluginCursor = CopyCursor(LoadImage(pluginModule, MAKEINTRESOURCE(IDC_CURSORCROSS), IMAGE_CURSOR, curSize, curSize, LR_SHARED));

	if (GetPluginSetting<bool>(SETTING_CUSTOM_CURSOR))
		SetCustomCursor();

	ResetTrackRecorder();
	LoadRouteChecker();
	LoadTransitionLevel();
	LoadMetricAltitude();

	AddAlias(".mteplugin", GITHUB_LINK); // for testing and for fun

	RegisterTagItemType("GS (value & trend)", TAG_ITEM_TYPE_GS_W_TRND);
	RegisterTagItemType("RMK/STS indicator", TAG_ITEM_TYPE_RMK_IND);
	RegisterTagItemType("VS (auto)", TAG_ITEM_TYPE_VS_AHIDE);
	RegisterTagItemType("VS indicator", TAG_ITEM_TYPE_LVL_IND);
	RegisterTagItemType("MCL/AFL", TAG_ITEM_TYPE_AFL_MTR);
	RegisterTagItemType("CFL", TAG_ITEM_TYPE_CFL_FLX);
	RegisterTagItemType("RFL", TAG_ITEM_TYPE_RFL_ICAO);
	RegisterTagItemType("Similar callsign indicator", TAG_ITEM_TYPE_SC_IND);
	RegisterTagItemType("Unit indicator 1 (RFL)", TAG_ITEM_TYPE_UNIT_IND_1);
	RegisterTagItemType("RVSM indicator", TAG_ITEM_TYPE_RVSM_IND);
	RegisterTagItemType("Coordination flag", TAG_ITEM_TYPE_COORD_FLAG);
	RegisterTagItemType("RECAT-CN (H-B/C)", TAG_ITEM_TYPE_RECAT_BC);
	RegisterTagItemType("Route validity", TAG_ITEM_TYPE_RTE_CHECK);
	RegisterTagItemType("Squawk DUPE flag", TAG_ITEM_TYPE_SQ_DUPE);
	RegisterTagItemType("Departure sequence", TAG_ITEM_TYPE_DEP_SEQ);
	RegisterTagItemType("Radar vector indicator", TAG_ITEM_TYPE_RVEC_IND);
	RegisterTagItemType("CFL (m)", TAG_ITEM_TYPE_CFL_MTR);
	RegisterTagItemType("Reconnected indicator", TAG_ITEM_TYPE_RCNT_IND);
	RegisterTagItemType("Departure status", TAG_ITEM_TYPE_DEP_STS);
	RegisterTagItemType("RECAT-CN (LMCBJ)", TAG_TIEM_TYPE_RECAT_WTC);
	RegisterTagItemType("ASP bound (Topsky, +/-)", TAG_ITEM_TYPE_ASPD_BND);
	RegisterTagItemType("GS (value)", TAG_ITEM_TYPE_GS_VALUE);
	RegisterTagItemType("Unit indicator 2 (PUS)", TAG_ITEM_TYPE_UNIT_IND_2);
	RegisterTagItemType("VS (toggle)", TAG_ITEM_TYPE_VS_TOGGL);
	RegisterTagItemType("VS (always)", TAG_ITEM_TYPE_VS_ALWYS);
	RegisterTagItemType("GS (trend)", TAG_ITEM_TYPE_GS_TRND);
	RegisterTagItemType("Emergency flag", TAG_ITEM_TYPE_SQ_EMRG);
	RegisterTagItemType("CLAM flag", TAG_ITEM_TYPE_CLAM_FLAG);
	RegisterTagItemType("RAM flag", TAG_ITEM_TYPE_RAM_FLAG);

	RegisterTagItemFunction("Set coordination flag", TAG_ITEM_FUNCTION_SET_CFLAG);
	RegisterTagItemFunction("Restore assigned data", TAG_ITEM_FUNCTION_RCNT_RST);
	RegisterTagItemFunction("Toggle VS display", TAG_ITEM_FUNCTION_VS_DISP);
	RegisterTagItemFunction("Open CFL popup menu", TAG_ITEM_FUNCTION_CFL_MENU);
	RegisterTagItemFunction("Open CFL popup edit", TAG_ITEM_FUNCTION_CFL_EDIT);
	RegisterTagItemFunction("Acknowledge CFL, open Topsky CFL menu", TAG_ITEM_FUNCTION_CFL_TOPSKY);
	RegisterTagItemFunction("Open RFL popup menu", TAG_ITEM_FUNCTION_RFL_MENU);
	RegisterTagItemFunction("Open RFL popup edit", TAG_ITEM_FUNCTION_RFL_EDIT);
	RegisterTagItemFunction("Open similar callsign list", TAG_ITEM_FUNCTION_SC_LIST);
	RegisterTagItemFunction("Show route checker info", TAG_ITEM_FUNCTION_RC_INFO);
	RegisterTagItemFunction("Amend route checker to strip", TAG_ITEM_FUNCTION_RC_STRIP);
	RegisterTagItemFunction("Set departure sequence", TAG_ITEM_FUNCTION_DSQ_MENU);
	RegisterTagItemFunction("Set departure status", TAG_ITEM_FUNCTION_DSQ_STS);
	RegisterTagItemFunction("Open ASP popup menu", TAG_ITEM_FUNCTION_SPD_LIST);
	RegisterTagItemFunction("Open unit settings popup menu", TAG_ITEM_FUNCTION_UNIT_MENU);

	DisplayUserMessage("MESSAGE", "MTEPlugin",
		std::format("For help please refer to {}.", GITHUB_LINK).c_str(),
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
	auto PrintStr = [sItemString](const std::string& s) {
		strcpy_s(sItemString, 16, s.substr(0, 15).c_str());
		};

	switch (ItemCode)
	{
	case TAG_ITEM_TYPE_GS_TRND:
	case TAG_ITEM_TYPE_GS_VALUE:
	case TAG_ITEM_TYPE_GS_W_TRND: {
		if (!RadarTarget.IsValid()) break;
		// determine if using calculated or reported
		int threshold = GetPluginSetting<int>(SETTING_GS_MODE); // knots, when the gap is smaller than this value, use reported, otherwise use calculated.
		CRadarTargetPositionData curpos = RadarTarget.GetPosition();
		double gsrpt = curpos.GetReportedGS(); // can be 0 but should not affect consequent selection
		CRadarTargetPositionData prepos = RadarTarget.GetPreviousPosition(curpos);
		double distance = prepos.GetPosition().DistanceTo(curpos.GetPosition()); // n miles
		double elapsed = GetLastRadarInterval(curpos, prepos);
		double gscal = abs(distance / elapsed * 3600.0); // knots
		double gskts = abs(gsrpt - gscal) < (double)threshold ? gsrpt : gscal;
		std::string strgs = "";
		if (ItemCode != TAG_ITEM_TYPE_GS_TRND) {
			if (m_TrackRecorder->IsForceKnot(RadarTarget)) { // force knot
				strgs = gskts < 995 ? std::format("{:02d}", (int)round(gskts / 10.0)) : "++"; // due to rounding
			}
			else { //convert to kph
				double gskph = KN_KPH(gskts);
				strgs = gskph < 1995 ? std::format("{:03d}", (int)round(gskph / 10.0)) : "+++"; // due to rounding
			}
		}
		if (ItemCode == TAG_ITEM_TYPE_GS_TRND ||
			(ItemCode == TAG_ITEM_TYPE_GS_W_TRND && strgs.find('+') == std::string::npos)) {
			char uTrend = GetPluginSetting<char>(SETTING_GS_INC);
			char sTrend = GetPluginSetting<char>(SETTING_GS_STA);
			char dTrend = GetPluginSetting<char>(SETTING_GS_DEC);
			if (uTrend || sTrend || dTrend) { // at least one trend mark
				double gsrpt1 = prepos.GetReportedGS();
				CRadarTargetPositionData prepos1 = RadarTarget.GetPreviousPosition(prepos);
				double distance1 = prepos1.GetPosition().DistanceTo(prepos.GetPosition()); // n miles
				double elapsed1 = GetLastRadarInterval(prepos, prepos1);
				double gscal1 = abs(distance1 / elapsed1 * 3600.0); // knots
				double gskts1 = abs(gsrpt1 - gscal1) < (double)threshold ? gsrpt1 : gscal1;
				double diff = KN_KPH(gskts - gskts1);
				int trendThld = abs(GetPluginSetting<int>(SETTING_GS_TREND));
				if (trendThld > 0) {
					strgs += diff >= trendThld ? uTrend : (diff <= -trendThld ? dTrend : sTrend);
				}
			}
		}
		PrintStr(strgs);
		break;
	}
	case TAG_ITEM_TYPE_RMK_IND: {
		if (!FlightPlan.IsValid()) break;
		std::string remarks;
		remarks = FlightPlan.GetFlightPlanData().GetRemarks();
		if (remarks.find("RMK/") != std::string::npos || remarks.find("STS/") != std::string::npos)
			PrintStr("*");
		else
			PrintStr("");
		break;
	}
	case TAG_ITEM_TYPE_VS_AHIDE:
	case TAG_ITEM_TYPE_VS_TOGGL:
	case TAG_ITEM_TYPE_VS_ALWYS: {
		if (!RadarTarget.IsValid()) break;
		int vs = abs(CalculateVerticalSpeed(RadarTarget, true));
		int thld = abs(GetPluginSetting<int>(SETTING_VS_THLD));
		// determines whether to show
		if (ItemCode == TAG_ITEM_TYPE_VS_AHIDE) {
			if (vs < thld) {
				break;
			}
		}
		else if (ItemCode == TAG_ITEM_TYPE_VS_TOGGL &&
			!m_TrackRecorder->IsDisplayVerticalSpeed(RadarTarget.GetSystemID())) {
			break;
		}
		vs = vs >= thld ? vs : 0;
		PrintStr(std::format("{:04d}", OVRFLW4(vs)));
		break;
	}
	case TAG_ITEM_TYPE_LVL_IND: {
		if (!RadarTarget.IsValid()) break;
		int vs = CalculateVerticalSpeed(RadarTarget);
		int thld = abs(GetPluginSetting<int>(SETTING_VS_THLD));
		PrintStr(vs >= thld ? "^" : (vs <= -thld ? "|" : ">"));
		break;
	}
	case TAG_ITEM_TYPE_AFL_MTR: {
		if (!RadarTarget.IsValid()) break;
		int altref;
		int rdrAlt = m_TransitionLevel->GetRadarDisplayAltitude(RadarTarget, altref);
		int dspAlt;
		std::string tmpStr;
		if (!m_TrackRecorder->IsForceFeet(RadarTarget)) {
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
			auto setNumMap = GetDataFromSettings(SETTING_CUSTOM_NUMBER_MAP);
			if (setNumMap != nullptr && strlen(setNumMap) == 10) {
				std::transform(dspStr.begin(), dspStr.end(), dspStr.begin(), [setNumMap](auto& c) {
					return setNumMap[int(c - '0')];
					});
			}
		}
		else if (altref == AltitudeReference::ALT_REF_QFE) {
			dspStr = "(" + dspStr + ")";
		}
		PrintStr(dspStr);
		break;
	}
	case TAG_ITEM_TYPE_CFL_FLX: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackRecorder->IsCflAcknowledged(FlightPlan.GetCallsign())) {
			GetColorDefinition(SETTING_COLOR_CFL_CONFRM, pColorCode, pRGB);
		}
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		if (cflAlt == 2) { // cleared for visual approach
			PrintStr("VA");
		}
		else if (cflAlt == 1) { // cleared for ILS approach
			PrintStr("ILS");
		}
		else if (!IsCFLAssigned(FlightPlan)) { // no cleared level or CFL==RFL
			PrintStr("    ");
		}
		else { // have a cleared level
			cflAlt = FlightPlan.GetClearedAltitude(); // difference: no ILS/VA, no CFL will show RFL
			int dspAlt;
			int trslvl, elev;
			bool isQFE = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev).size() && elev && cflAlt < trslvl;
			cflAlt -= isQFE ? elev : 0;
			bool isfeet = m_TrackRecorder->IsForceFeet(FlightPlan);
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
			std::string dspAltStr;
			if (isfeet) {
				dspAlt = cflAlt / 100; // FL xx0, show FL in feet
				dspAltStr = std::format("{:03d}", OVRFLW3(dspAlt));
			}
			else {
				dspAltStr = std::format("{:04d}", OVRFLW4(dspAlt));
			}
			if (isQFE) {
				dspAltStr = std::format("({})", dspAltStr);
			}
			PrintStr(dspAltStr);
		}
		break;
	}
	case TAG_ITEM_TYPE_CFL_MTR: {
		if (!FlightPlan.IsValid()) break;
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		int dspAlt = MetricAlt::LvlFeettoM(cflAlt);
		PrintStr(std::format("{:04d}", OVRFLW4(dspAlt / 10)));
		break;
	}
	case TAG_ITEM_TYPE_RFL_ICAO: {
		if (!FlightPlan.IsValid()) break;
		int rflAlt = FlightPlan.GetFinalAltitude();
		int dspMtr;
		if (MetricAlt::RflFeettoM(rflAlt, dspMtr) && !m_TrackRecorder->IsForceFeet(FlightPlan)) {
			// is metric RVSM and not forced feet
			char trsMrk = rflAlt >= GetTransitionAltitude() ? 'S' : 'M';
			PrintStr(std::format("{}{:04d}", trsMrk, OVRFLW4(dspMtr / 10)));
		}
		else {
			rflAlt = (int)round(rflAlt / 100.0);
			PrintStr(std::format("F{:03d}", OVRFLW3(rflAlt)));
		}
		break;
	}
	case TAG_ITEM_TYPE_SC_IND: {
		if (!FlightPlan.IsValid()) break;
		if (m_TrackRecorder->IsSimilarCallsign(FlightPlan.GetCallsign())) {
			PrintStr("SC");
			GetColorDefinition(SETTING_COLOR_CS_SIMILR, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_UNIT_IND_1: {
		if (!FlightPlan.IsValid()) break;
		char c = m_TrackRecorder->IsDifferentUnitRFL(FlightPlan) ? \
			GetPluginCharSetting(SETTING_UNIT_IND_1O, DEFAULT_UNIT_IND_1O) : \
			GetPluginCharSetting(SETTING_UNIT_IND_1X, DEFAULT_UNIT_IND_1X);
		PrintStr(std::string(1, c));
		break;
	}
	case TAG_ITEM_TYPE_UNIT_IND_2: {
		if (!RadarTarget.IsValid()) break;
		char c = m_TrackRecorder->IsDifferentUnitPUS(RadarTarget) ? \
			GetPluginCharSetting(SETTING_UNIT_IND_2O, DEFAULT_UNIT_IND_2O) : \
			GetPluginCharSetting(SETTING_UNIT_IND_2X, DEFAULT_UNIT_IND_2X);
		PrintStr(std::string(1, c));
		break;
	}
	case TAG_ITEM_TYPE_RVSM_IND: {
		if (!FlightPlan.IsValid()) break;
		CFlightPlanData fpdata = FlightPlan.GetFlightPlanData();
		std::string acinf = fpdata.GetAircraftInfo();
		char ind = GetPluginCharSetting(SETTING_FLAG_RVSM_O, DEFAULT_FLAG_RVSM_O);
		if (!strcmp(fpdata.GetPlanType(), "V"))
			ind = GetPluginCharSetting(SETTING_FLAG_RVSM_V, DEFAULT_FLAG_RVSM_V);
		else if (acinf.size() <= 8) { // assume FAA format
			char capa = fpdata.GetCapibilities();
			if (std::string("HWJKLZ?").find(capa) == std::string::npos)
				ind = GetPluginCharSetting(SETTING_FLAG_RVSM_X, DEFAULT_FLAG_RVSM_X);
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
				ind = GetPluginCharSetting(SETTING_FLAG_RVSM_X, DEFAULT_FLAG_RVSM_X);
		}
		PrintStr(std::string(1, ind));
		GetColorDefinition(SETTING_COLOR_RVSM_IND, pColorCode, pRGB);
		break;
	}
	case TAG_ITEM_TYPE_COORD_FLAG: {
		if (!FlightPlan.IsValid()) break;
		char c = m_TrackRecorder->GetCoordinationFlag(FlightPlan.GetCallsign()) ? \
			GetPluginCharSetting(SETTING_FLAG_COORD_O, DEFAULT_FLAG_COORD_O) : \
			GetPluginCharSetting(SETTING_FLAG_COORD_X, DEFAULT_FLAG_COORD_X);
		PrintStr(std::string(1, c));
		GetColorDefinition(SETTING_COLOR_COORD_FLAG, pColorCode, pRGB);
		break;
	}
	case TAG_ITEM_TYPE_RECAT_BC: {
		if (!FlightPlan.IsValid()) break;
		if (FlightPlan.GetFlightPlanData().GetAircraftWtc() == 'H') {
			auto rc = m_ReCatMap.find(FlightPlan.GetFlightPlanData().GetAircraftFPType());
			if (rc != m_ReCatMap.end() && rc->second != 'J')
				PrintStr("-" + std::string(1, rc->second));
		}
		break;
	}
	case TAG_TIEM_TYPE_RECAT_WTC: {
		if (!FlightPlan.IsValid()) break;
		auto rc = m_ReCatMap.find(FlightPlan.GetFlightPlanData().GetAircraftFPType());
		PrintStr(std::string(1,
			rc != m_ReCatMap.end() ? rc->second : FlightPlan.GetFlightPlanData().GetAircraftWtc()));
		break;
	}
	case TAG_ITEM_TYPE_RTE_CHECK: {
		if (!m_RouteChecker ||
			!FlightPlan.IsValid() ||
			FlightPlan.GetClearenceFlag())
			break;
		std::string ind = "?";
		std::string mrk = ""; // for * (other controller set strip annotation)
		int res = RouteCheckerConstants::NOT_FOUND;
		std::regex rxAnno(R"(/RC/(\d+)/(\S+)/)");
		std::smatch match;
		std::string anno = FlightPlan.GetControllerAssignedData().GetFlightStripAnnotation(3);
		if (anno.size() && std::regex_match(anno, match, rxAnno)) {
			res = std::stoi(match[1].str());
			if (match[2].str() != ControllerMyself().GetCallsign()) { // someone else has set annotation
				mrk = "*";
			}
		}
		res = res != RouteCheckerConstants::NOT_FOUND ? res : m_RouteChecker->CheckFlightPlan(FlightPlan, false, false);
		switch (res)
		{
		case RouteCheckerConstants::INVALID:
			ind = "X";
			GetColorDefinition(SETTING_COLOR_RC_ALT, pColorCode, pRGB);
			break;
		case RouteCheckerConstants::PARTIAL_NO_LEVEL:
			GetColorDefinition(SETTING_COLOR_RC_ALT, pColorCode, pRGB);
			[[fallthrough]];
		case RouteCheckerConstants::PARTIAL_OK_LEVEL:
			ind = "P";
			break;
		case RouteCheckerConstants::STRUCT_NO_LEVEL:
			GetColorDefinition(SETTING_COLOR_RC_ALT, pColorCode, pRGB);
			[[fallthrough]];
		case RouteCheckerConstants::STRUCT_OK_LEVEL:
			ind = "S";
			break;
		case RouteCheckerConstants::TEXT_NO_LEVEL:
			GetColorDefinition(SETTING_COLOR_RC_ALT, pColorCode, pRGB);
			[[fallthrough]];
		case RouteCheckerConstants::TEXT_OK_LEVEL:
			ind = "Y";
			break;
		default:
			break;
		}
		PrintStr(ind + mrk);
		break;
	}
	case TAG_ITEM_TYPE_SQ_DUPE: {
		if (!RadarTarget.IsValid()) break;
		if (m_TrackRecorder->IsSquawkDUPE(RadarTarget)) {
			std::string flag = GetPluginSetting<std::string>(SETTING_FLAG_DUPE);
			PrintStr(flag);
			GetColorDefinition(SETTING_COLOR_SQ_DUPE, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_DEP_SEQ: {
		if (!FlightPlan.IsValid()) break;
		if (!m_DepartureSequence) m_DepartureSequence = std::make_unique<DepartureSequence>();
		int seq = m_DepartureSequence->GetSequence(FlightPlan);
		if (seq > 0)
			PrintStr(std::format("{:02d}", OVRFLW2(seq)));
		else if (seq < 0) { // reconnected
			PrintStr("--");
			GetColorDefinition(SETTING_COLOR_DS_NUMBR, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_DEP_STS: {
		if (!FlightPlan.IsValid()) break;
		std::string gsts = FlightPlan.GetGroundState();
		if (gsts.size()) {
			PrintStr(gsts);
			if (!FlightPlan.GetClearenceFlag()) {
				GetColorDefinition(SETTING_COLOR_DS_STATE, pColorCode, pRGB);
			}
		}
		else if (FlightPlan.GetClearenceFlag()) {
			PrintStr("CLRD");
		}
		break;
	}
	case TAG_ITEM_TYPE_RVEC_IND: {
		if (FlightPlan.IsValid() && FlightPlan.GetControllerAssignedData().GetAssignedHeading() &&
			GetTrackingStatus(FlightPlan) == TRACK_STATUS_MYSF) {
			PrintStr("RV");
			GetColorDefinition(SETTING_COLOR_RDRV_IND, pColorCode, pRGB);
		}
		break;
	}
	case TAG_ITEM_TYPE_RCNT_IND: {
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackRecorder->IsActive(FlightPlan)) {
			PrintStr("r");
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
			PrintStr("+");
		}
		else if (strip.find("/s-/") != std::string::npos) {
			PrintStr("-");
		}
		break;
	}
	case TAG_ITEM_TYPE_SQ_EMRG: {
		if (!RadarTarget.IsValid()) break;
		std::string squawk = RadarTarget.GetPosition().GetSquawk();
		if (squawk != "7700" && squawk != "7600" && squawk != "7500") break;
		std::stringstream ssFlag(GetPluginSetting<std::string>(SETTING_FLAG_EMG));
		std::string flag, flag77, flag76, flag75;
		std::getline(ssFlag, flag77, ':');
		std::getline(ssFlag, flag76, ':');
		std::getline(ssFlag, flag75, ':');
		if (squawk == "7700") {
			flag = flag77;
			GetColorDefinition(SETTING_COLOR_SQ_7700, pColorCode, pRGB);
		}
		else if (squawk == "7600") {
			flag = flag76;
			GetColorDefinition(SETTING_COLOR_SQ_7600, pColorCode, pRGB);
		}
		else if (squawk == "7500") {
			flag = flag75;
			GetColorDefinition(SETTING_COLOR_SQ_7500, pColorCode, pRGB);
		}
		flag = flag.substr(0, 15);// prevent overflow pool
		PrintStr(flag);
		break;
	}
	case TAG_ITEM_TYPE_CLAM_FLAG: {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid()) break;
		int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
		if (cflAlt == 1 || cflAlt == 2) break; // final approach
		else if (cflAlt <= 200 && !IsCFLAssigned(FlightPlan)) break; // inhibits when no CFL
		cflAlt = FlightPlan.GetClearedAltitude();
		// get MCL
		int trsl, _elev;
		m_TransitionLevel->GetTargetAirport(FlightPlan, trsl, _elev);
		int stdAlt = RadarTarget.GetPosition().GetFlightLevel();
		int qnhAlt = RadarTarget.GetPosition().GetPressureAltitude();
		int mclAlt = stdAlt >= trsl ? stdAlt : qnhAlt;
		if (abs(mclAlt - cflAlt) <= 200) break; // within normal zone
		// get VS trend
		int vs = CalculateVerticalSpeed(RadarTarget);
		int thld = abs(GetPluginSetting<int>(SETTING_VS_THLD));
		// basic logic
		if (abs(vs) < thld) { // maintaining altitude
			if (m_TrackRecorder->GetCflElapsedTime(FlightPlan.GetCallsign()) < 60) {
				// inhibit within last CFL assign time + 60s
				break;
			}
		}
		else { // changing altitude
			if (vs * (cflAlt - mclAlt) > 0) { // going towards cfl
				break;
			}
		}
		PrintStr(GetPluginSetting<std::string>(SETTING_FLAG_CLAM));
		GetColorDefinition(SETTING_COLOR_CLAM_FLAG, pColorCode, pRGB);
		break;
	}
	case TAG_ITEM_TYPE_RAM_FLAG: {
		if (!FlightPlan.IsValid() || !RadarTarget.IsValid()) break;
		if (!FlightPlan.GetRAMFlag()) break;
		auto route = FlightPlan.GetExtractedRoute();
		int n = route.GetPointsNumber();
		auto posOrig = route.GetPointPosition(0);
		auto posDest = route.GetPointPosition(n - 1);
		auto posCurr = FlightPlan.GetFPTrackPosition().GetPosition();
		double d1 = posCurr.DistanceTo(posDest);
		double d2 = posCurr.DistanceTo(posOrig);
		if (d1 <= 30 || d2 <= 30)
			break; // inhibit RAM when near origin/destination
		PrintStr(GetPluginSetting<std::string>(SETTING_FLAG_RAM));
		GetColorDefinition(SETTING_COLOR_RAM_FLAG, pColorCode, pRGB);
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
	case TAG_ITEM_FUNCTION_SET_CFLAG: {
		if (!FlightPlan.IsValid()) break;
		m_TrackRecorder->SetCoordinationFlag(std::string(FlightPlan.GetCallsign()));
		break;
	}
	case TAG_ITEM_FUNCTION_RCNT_RST: {
		if (!FlightPlan.IsValid()) break;
		m_TrackRecorder->SetTrackedData(FlightPlan);
		break;
	}
	case TAG_ITEM_FUNCTION_VS_DISP: {
		if (!RadarTarget.IsValid()) break;
		m_TrackRecorder->ToggleVerticalSpeed(std::string(RadarTarget.GetSystemID()));
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_SET_MENU: {
		if (!FlightPlan.IsValid()) break;
		int tgtAlt = MetricAlt::GetAltitudeFromMenuItem(sItemString, !m_TrackRecorder->IsForceFeet(FlightPlan));
		if (tgtAlt > MetricAlt::ALT_MAP_NOT_FOUND) {
			if (tgtAlt == 1 || tgtAlt == 2) { // ILS or VA
				FlightPlan.GetControllerAssignedData().SetAssignedHeading(0);
			}
			else if (tgtAlt > 2) {
				if (GetPluginSetting<int>(SETTING_AMEND_CFL) == 1) {
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
			m_TrackRecorder->SetAltitudeUnit(FlightPlan, true);
			break;
		}
		else if (input == "M") {
			m_TrackRecorder->SetAltitudeUnit(FlightPlan, false);
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
		if (tgtAlt > 2 && GetPluginSetting<int>(SETTING_AMEND_CFL) == 1) {
			int trslvl, elev;
			std::string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
			tgtAlt += aptgt.size() && tgtAlt < trslvl ? elev : 0; // convert QNH to QFE
		}
		FlightPlan.GetControllerAssignedData().SetClearedAltitude(tgtAlt); // no need to check overflow
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_MENU: {
		if (m_TrackRecorder->ToggleAltitudeUnit(RadarTarget, GetPluginSetting<int>(SETTING_ALT_TOGG))) break;
		if (!FlightPlan.IsValid()) break;
		if (GetTrackingStatus(FlightPlan) == TRACK_STATUS_OTHR) {
			// don't show list if other controller is tracking
			break;
		}
		else if (!m_TrackRecorder->IsCflAcknowledged(FlightPlan.GetCallsign())) {
			// acknowledge previous CFL first
			m_TrackRecorder->AcknowledgeCfl(FlightPlan.GetCallsign());
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
		auto m_alt = MetricAlt::GetMenuItems(!m_TrackRecorder->IsForceFeet(FlightPlan), trslvl);
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
		if (m_TrackRecorder->ToggleAltitudeUnit(RadarTarget, GetPluginSetting<int>(SETTING_ALT_TOGG))) break;
		if (!FlightPlan.IsValid()) break;
		if (GetTrackingStatus(FlightPlan) == TRACK_STATUS_OTHR) {
			// don't show list if other controller is tracking
			break;
		}
		else if (!m_TrackRecorder->IsCflAcknowledged(FlightPlan.GetCallsign())) {
			// acknowledge previous CFL first
			m_TrackRecorder->AcknowledgeCfl(FlightPlan.GetCallsign());
			break;
		}
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_CFL_SET_EDIT, "");
		break;
	}
	case TAG_ITEM_FUNCTION_CFL_TOPSKY: {
		if (m_TrackRecorder->ToggleAltitudeUnit(RadarTarget, GetPluginSetting<int>(SETTING_ALT_TOGG))) break;
		if (!FlightPlan.IsValid()) break;
		if (!m_TrackRecorder->IsCflAcknowledged(FlightPlan.GetCallsign())) {
			// acknowledge previous CFL first
			m_TrackRecorder->AcknowledgeCfl(FlightPlan.GetCallsign());
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
		int tgtAlt = MetricAlt::GetAltitudeFromMenuItem(sItemString, !m_TrackRecorder->IsForceFeet(FlightPlan));
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
			m_TrackRecorder->SetAltitudeUnit(FlightPlan, true);
			break;
		}
		else if (input == "M") {
			m_TrackRecorder->SetAltitudeUnit(FlightPlan, false);
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
		if (m_TrackRecorder->ToggleAltitudeUnit(RadarTarget, GetPluginSetting<int>(SETTING_ALT_TOGG))) break;
		if (!FlightPlan.IsValid()) break;
		if (GetTrackingStatus(FlightPlan) == TRACK_STATUS_OTHR) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupList(Area, "RFL Menu", 1);
		// pre-select altitude
		int rflAlt = FlightPlan.GetFinalAltitude();
		auto m_alt = MetricAlt::GetMenuItems(!m_TrackRecorder->IsForceFeet(FlightPlan), 0);
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
		if (m_TrackRecorder->ToggleAltitudeUnit(RadarTarget, GetPluginSetting<int>(SETTING_ALT_TOGG))) break;
		if (!FlightPlan.IsValid()) break;
		if (GetTrackingStatus(FlightPlan) == TRACK_STATUS_OTHR) {
			// don't show list if other controller is tracking
			break;
		}
		OpenPopupEdit(Area, TAG_ITEM_FUNCTION_RFL_SET_EDIT, "");
		break;
	}
	case TAG_ITEM_FUNCTION_SC_LIST: {
		if (!FlightPlan.IsValid()) break;
		std::string cs = FlightPlan.GetCallsign();
		if (!m_TrackRecorder->IsSimilarCallsign(cs)) // not a SC
			break;
		OpenPopupList(Area, "SC List", 1);
		AddPopupListElement(cs.c_str(), nullptr, TAG_ITEM_FUNCTION_SC_SELECT, true);
		auto cset = m_TrackRecorder->GetSimilarCallsigns(cs);
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
	case TAG_ITEM_FUNCTION_RC_INFO:
	case TAG_ITEM_FUNCTION_RC_STRIP: {
		if (!m_RouteChecker ||
			!FlightPlan.IsValid() ||
			FlightPlan.GetClearenceFlag())
			break;
		int rc = m_RouteChecker->CheckFlightPlan(FlightPlan, true, false); // force a refresh here to avoid error
		if (rc == RouteCheckerConstants::NOT_FOUND) break;
		if (FunctionId == TAG_ITEM_FUNCTION_RC_INFO) {
			DisplayRouteMessage(FlightPlan.GetFlightPlanData().GetOrigin(), FlightPlan.GetFlightPlanData().GetDestination());
		}
		else if (ControllerMyself().IsController()) {
			// assign to flight strip, format: /RC/{rc}/{my controller callsign}/
			std::string strpn = std::format("/RC/{}/{}/", rc, ControllerMyself().GetCallsign());
			FlightPlan.GetControllerAssignedData().SetFlightStripAnnotation(3, strpn.c_str());
		}
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
		if (GetTrackingStatus(FlightPlan) == TRACK_STATUS_OTHR) {
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
		bool isfeet = m_TrackRecorder->IsForceFeet(FlightPlan) || m_TrackRecorder->IsForceFeet(RadarTarget);
		bool isknot = m_TrackRecorder->IsForceKnot(RadarTarget);
		bool showvs = m_TrackRecorder->IsDisplayVerticalSpeed(RadarTarget.IsValid() ? RadarTarget.GetSystemID() : "");
		OpenPopupList(Area, "Units", 2);
		// fix length = 5
		AddPopupListElement(isfeet ? "ALT:F" : "ALT:M", "", TAG_ITEM_FUNCTION_UNIT_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, false);
		AddPopupListElement(isknot ? "SPD:S" : "SPD:K", "", TAG_ITEM_FUNCTION_UNIT_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, false);
		AddPopupListElement(showvs ? "VS :O" : "VS :X", "", TAG_ITEM_FUNCTION_UNIT_SET, false, POPUP_ELEMENT_NO_CHECKBOX, false, false);
		break;
	}
	case TAG_ITEM_FUNCTION_UNIT_SET: {
		std::string type = sItemString;
		char c = type.at(4);
		if (type.substr(0, 3) == "ALT") {
			if (RadarTarget.IsValid()) {
				m_TrackRecorder->SetAltitudeUnit(RadarTarget, c != 'F');
			}
			else if (FlightPlan.IsValid()) {
				m_TrackRecorder->SetAltitudeUnit(FlightPlan, c != 'F');
			}
		}
		else if (type.substr(0, 3) == "SPD") {
			if (RadarTarget.IsValid()) {
				m_TrackRecorder->SetSpeedUnit(RadarTarget, c != 'S');
			}
		}
		else if (type.substr(0, 3) == "VS ") {
			if (RadarTarget.IsValid()) {
				m_TrackRecorder->ToggleVerticalSpeed(std::string(RadarTarget.GetSystemID()));
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
		if (GetPluginSetting<int>(SETTING_AMEND_CFL) == 2) { // amend only when mode 2 (all) is set
			int cflAlt = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
			if (cflAlt > 2) { // exclude ILS, VA, NONE
				int trslvl, elev;
				std::string aptgt = m_TransitionLevel->GetTargetAirport(FlightPlan, trslvl, elev);
				if (aptgt.size() && cflAlt < trslvl && elev > 0) {
					static bool suppress = false;
					if (GetConnectionType() != CONNECTION_TYPE_VIA_PROXY && !suppress) {
						suppress = true;
						cflAlt += elev; // convert QNH to QFE
						FlightPlan.GetControllerAssignedData().SetClearedAltitude(cflAlt);
						suppress = false;
						return;
					}
				}
			}
		}
		m_TrackRecorder->HandleNewCfl(FlightPlan, !GetTrackingStatus(FlightPlan)); // initiate CFL not ack
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
		GetConnectionType() != CONNECTION_TYPE_VIA_PROXY &&
		FlightPlan.GetClearenceFlag() &&
		!strlen(FlightPlan.GetGroundState())) {
		CallItemFunction(FlightPlan.GetCallsign(), TAG_ITEM_FUNCTION_SET_CLEARED_FLAG, POINT(), RECT());
	}
	m_TrackRecorder->UpdateFlight(FlightPlan);
}

void CMTEPlugIn::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	if (!FlightPlan.IsValid())
		return;
	if (m_RouteChecker)
		m_RouteChecker->RemoveCache(FlightPlan);
	if (m_DepartureSequence)
		m_DepartureSequence->EditSequence(FlightPlan, -1);
	if (GetTrackingStatus(FlightPlan) == TRACK_STATUS_MYSF)
		m_TrackRecorder->UpdateFlight(FlightPlan, false);
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
	if (m_TrackRecorder->IsActive(RadarTarget)) {
		m_TrackRecorder->UpdateFlight(RadarTarget);
	}
	else if (GetConnectionType() != CONNECTION_TYPE_VIA_PROXY) {
		int retrack = GetPluginSetting<int>(SETTING_AUTO_RETRACK);
		if (retrack == 1 || retrack == 2) {
			if (m_TrackRecorder->SetTrackedData(RadarTarget) && retrack == 2) {
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
		SaveDataToSettings(SETTING_CUSTOM_CURSOR.name, "set custom mouse cursor", cc ? "1" : "0");
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

	// reset track recorder
	std::regex rxtr("^.MTEP TR RESET$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxtr)) {
		ResetTrackRecorder();
		return true;
	}

	// set auto retrack
	std::regex rxtrrt("^.MTEP TR ([0-2])$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxtrrt)) {
		std::string res = match[1].str();
		const char* descr = "auto retrack mode";
		if (res == "1") {
			SaveDataToSettings(SETTING_AUTO_RETRACK.name, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack mode 1 (silent) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "2") {
			SaveDataToSettings(SETTING_AUTO_RETRACK.name, descr, "2");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Auto retrack mode 2 (notify) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "0") {
			SaveDataToSettings(SETTING_AUTO_RETRACK.name, descr, "0");
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
			m_TrackRecorder->ResetAltitudeUnit(true);
			SaveDataToSettings(SETTING_ALT_FEET.name, "force feet", "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Altitude unit is set to feet", 1, 0, 0, 0, 0);
		}
		else if (res == "M") {
			m_TrackRecorder->ResetAltitudeUnit(false);
			SaveDataToSettings(SETTING_ALT_FEET.name, "force feet", "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Altitude unit is set to meter", 1, 0, 0, 0, 0);
		}
		else if (res == "S") {
			m_TrackRecorder->SetSpeedUnit(true);
			SaveDataToSettings(SETTING_GS_KNOT.name, "force knot", "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Speed unit is set to KTS", 1, 0, 0, 0, 0);
		}
		else if (res == "K") {
			m_TrackRecorder->SetSpeedUnit(false);
			SaveDataToSettings(SETTING_GS_KNOT.name, "force knot", "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Speed unit is set to KPH", 1, 0, 0, 0, 0);
		}
		return true;
	}

	// set vertical speed display
	std::regex rxvs("^.MTEP VS (ON|OFF)$", std::regex_constants::icase);
	if (regex_match(cmd, match, rxvs)) {
		std::string res = MakeUpper(match[1].str());
		const char* descr = "display vertical speed";
		if (res == "ON") {
			m_TrackRecorder->ToggleVerticalSpeed(true);
			SaveDataToSettings(SETTING_VS_MODE.name, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Global vertical speed display is ON", 1, 0, 0, 0, 0);
		}
		else if (res == "OFF") {
			m_TrackRecorder->ToggleVerticalSpeed(false);
			SaveDataToSettings(SETTING_VS_MODE.name, descr, "0");
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
			SaveDataToSettings(SETTING_AMEND_CFL.name, descr, "1");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL mode 1 (MTEP) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "2") {
			SaveDataToSettings(SETTING_AMEND_CFL.name, descr, "2");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL mode 2 (all) is set", 1, 0, 0, 0, 0);
		}
		else if (res == "0") {
			SaveDataToSettings(SETTING_AMEND_CFL.name, descr, "0");
			DisplayUserMessage("MESSAGE", "MTEPlugin", "Amend QFE in CFL is off", 1, 0, 0, 0, 0);
		}
		return true;
	}

	return false;
}

template<typename T>
inline T CMTEPlugIn::GetPluginSetting(const CommonSetting& setting)
{
	// only accept 0/1 for bool
	T bufval{};
	auto data = GetDataFromSettings(setting.name);
	if (data != nullptr) {
		std::stringstream ss(data);
		ss >> bufval;
		if (!ss.fail()) {
			return bufval;
		}
	}
	std::stringstream ssf(setting.fallback);
	ssf >> bufval;
	return bufval;
}

inline char CMTEPlugIn::GetPluginCharSetting(const char* setting, const char& fallback)
{
	char c = '\0';
	auto cs = GetDataFromSettings(setting);
	if (cs == nullptr) {
		c = fallback;
	}
	else if (strcmp(cs, R"(\0)")) { // "\0" is recognized as '\0'
		c = cs[0];
	}
	return c;
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
		int rnd = abs(GetPluginSetting<int>(SETTING_VS_RNDG));
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

bool CMTEPlugIn::GetColorDefinition(const ColorSetting setting, int* pColorCode, COLORREF* pRGB)
{
	// If setting is not present or invalid, it will not touch anything
	unsigned int r(256), g(256), b(256);
	auto settingValue = GetDataFromSettings(setting.name);
	if (settingValue != nullptr &&
		sscanf_s(settingValue, "%u:%u:%u", &r, &g, &b) == 3 &&
		r <= 255 && g <= 255 && b <= 255) {
		*pColorCode = TAG_COLOR_RGB_DEFINED;
		*pRGB = RGB(r, g, b);
		return true;
	}
	*pColorCode = setting.code;
	return false;
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
			("RC loaded. File: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("RC load fail (" + e + "). File: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_RouteChecker.reset();
	}
	catch (std::exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("RC load fail (" + std::string(e.what()) + "). File: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_RouteChecker.reset();
	}
}

void CMTEPlugIn::ResetDepartureSequence(void)
{
	m_DepartureSequence.reset();
}

void CMTEPlugIn::ResetTrackRecorder(void)
{
	m_TrackRecorder.reset(new TrackRecorder(this));
	m_TrackRecorder->ResetAltitudeUnit(GetPluginSetting<bool>(SETTING_ALT_FEET));
	m_TrackRecorder->SetSpeedUnit(GetPluginSetting<bool>(SETTING_GS_KNOT));
	m_TrackRecorder->ToggleVerticalSpeed(GetPluginSetting<bool>(SETTING_VS_MODE));
	m_TrackRecorder->SetCoordinationFlag(GetPluginSetting<bool>(SETTING_FLAG_COORD_MODE));
	DisplayUserMessage("MESSAGE", "MTEPlugin", "TR is ready!", 1, 0, 0, 0, 0);
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
			("TL loaded. File: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("TL load fail (" + e + "). File: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_TransitionLevel.reset(new TransitionLevel(this));
	}
	catch (std::exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("TL load fail (" + std::string(e.what()) + "). File: " + fn).c_str(),
			1, 0, 0, 0, 0);
		m_TransitionLevel.reset(new TransitionLevel(this));
	}
}

void CMTEPlugIn::LoadMetricAltitude(void)
{
	auto setfn = GetDataFromSettings(SETTING_TRANS_MALT_TXT);
	if (setfn == nullptr) {
		return;
	}
	std::string fn = GetRealFileName(setfn);
	try {
		MetricAlt::LoadAltitudeDefinition(fn);
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("MA loaded. File: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::string e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("MA load fail (" + e + "). File: " + fn).c_str(),
			1, 0, 0, 0, 0);
	}
	catch (std::exception e) {
		DisplayUserMessage("MESSAGE", "MTEPlugin",
			("MA load fail (" + std::string(e.what()) + "). File: " + fn).c_str(),
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
	// issue: sometimes FlightPlan.GetControllerAssignedData().GetClearedAltitude() returns negative value
	if (!FlightPlan.IsValid()) return false;
	int cfl = FlightPlan.GetControllerAssignedData().GetClearedAltitude();
	if (cfl > 0) return true;
	else if (strlen(FlightPlan.GetTrackingControllerCallsign())) { // tracked by someone
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

inline int GetTrackingStatus(CFlightPlan FlightPlan)
{
	// 0: none, 1: myself, -1: others
	return FlightPlan.GetTrackingControllerIsMe() ? TRACK_STATUS_MYSF : \
		(strlen(FlightPlan.GetTrackingControllerId()) ? TRACK_STATUS_OTHR : TRACK_STATUS_NONE);
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
