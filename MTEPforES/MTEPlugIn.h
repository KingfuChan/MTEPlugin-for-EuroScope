// MTEPlugin.h

#pragma once

#include "pch.h"
#include "MTEPScreen.h"
#include "MetricAlt.h"
#include "ReCat.h"
#include "SimilarCallsign.h"
#include "RouteChecker.h"
#include "DepartureSequence.h"
#include "TrackRecorder.h"
#include "TransitionLevel.h"

using namespace EuroScopePlugIn;

#ifndef TAG_TYPE // TAG ITEM TYPE
#define TAG_TYPE
const int TAG_ITEM_TYPE_GS_W_TRND = 1; // GS (value & trend)
const int TAG_ITEM_TYPE_RMK_IND = 2; // RMK/STS indicator
const int TAG_ITEM_TYPE_VS_AHIDE = 3; // VS (auto)
const int TAG_ITEM_TYPE_LVL_IND = 4; // VS indicator
const int TAG_ITEM_TYPE_AFL_MTR = 5; // MCL/AFL
const int TAG_ITEM_TYPE_CFL_FLX = 6; // CFL
const int TAG_ITEM_TYPE_RFL_ICAO = 7; // RFL
const int TAG_ITEM_TYPE_SC_IND = 8; // Similar callsign indicator
const int TAG_ITEM_TYPE_UNIT_IND_1 = 9; // Unit indicator 1 (RFL)
const int TAG_ITEM_TYPE_RVSM_IND = 10; // RVSM indicator
const int TAG_ITEM_TYPE_COORD_FLAG = 11; // Coordination flag
const int TAG_ITEM_TYPE_RECAT_BC = 12; // RECAT-CN (H-B/C)
const int TAG_ITEM_TYPE_RTE_CHECK = 13; // Route validity
const int TAG_ITEM_TYPE_SQ_DUPE = 14; // Squawk DUPE flag
const int TAG_ITEM_TYPE_DEP_SEQ = 15; // Departure sequence
const int TAG_ITEM_TYPE_RVEC_IND = 16; // Radar vector indicator
const int TAG_ITEM_TYPE_CFL_MTR = 17; // CFL (m)
const int TAG_ITEM_TYPE_RCNT_IND = 18; // Reconnected indicator
const int TAG_ITEM_TYPE_DEP_STS = 19; // Departure status
const int TAG_TIEM_TYPE_RECAT_WTC = 20; // RECAT-CN (LMCBJ)
const int TAG_ITEM_TYPE_ASPD_BND = 21; // ASP bound (Topsky, +/-)
const int TAG_ITEM_TYPE_GS_VALUE = 22; // GS (value)
const int TAG_ITEM_TYPE_UNIT_IND_2 = 23; // Unit indicator 2 (PUS)
const int TAG_ITEM_TYPE_VS_TOGGL = 24; // VS (toggle)
const int TAG_ITEM_TYPE_VS_ALWYS = 25; // VS (always)
const int TAG_ITEM_TYPE_GS_TRND = 26; // GS (trend)
const int TAG_ITEM_TYPE_SQ_EMRG = 27; // Emergency flag
const int TAG_ITEM_TYPE_CLAM_FLAG = 28; // CLAM flag
const int TAG_ITEM_TYPE_RAM_FLAG = 29; // RAM flag
#endif // !TAG_TYPE

#ifndef TAG_FUNCTION // TAG ITEM FUNCTION
#define TAG_FUNCTION
const int TAG_ITEM_FUNCTION_SET_CFLAG = 1; // Set coordination flag
const int TAG_ITEM_FUNCTION_RCNT_RST = 2; // Restore assigned data
const int TAG_ITEM_FUNCTION_VS_DISP = 3; // Toggle VS display
const int TAG_ITEM_FUNCTION_CFL_SET_EDIT = 10; // Set CFL from edit (not registered)
const int TAG_ITEM_FUNCTION_CFL_MENU = 11; // Open CFL popup menu
const int TAG_ITEM_FUNCTION_CFL_EDIT = 12; // Open CFL popup edit
const int TAG_ITEM_FUNCTION_CFL_SET_MENU = 13; // Set CFL from menu (not registered)
const int TAG_ITEM_FUNCTION_CFL_TOPSKY = 14; // Acknowledge CFL, open Topsky CFL menu
const int TAG_ITEM_FUNCTION_RFL_SET_EDIT = 20; // Set RFL from edit (not registered)
const int TAG_ITEM_FUNCTION_RFL_MENU = 21; // Open RFL popup menu
const int TAG_ITEM_FUNCTION_RFL_EDIT = 22; // Open RFL popup edit
const int TAG_ITEM_FUNCTION_RFL_SET_MENU = 23; // Set RFL from menu (not registered)
const int TAG_ITEM_FUNCTION_SC_LIST = 30; // Open similar callsign list
const int TAG_ITEM_FUNCTION_SC_SELECT = 31; // Select in similar callsign list (not registered)
const int TAG_ITEM_FUNCTION_RC_INFO = 40; // Show route checker info
const int TAG_ITEM_FUNCTION_RC_STRIP = 41; // Amend route checker to strip
const int TAG_ITEM_FUNCTION_DSQ_MENU = 50; // Set departure sequence
const int TAG_ITEM_FUNCTION_DSQ_EDIT = 51; // Open departure sequence popup edit (not registered)
const int TAG_ITEM_FUNCTION_DSQ_STS = 52; // Set departure status
const int TAG_ITEM_FUNCTION_SPD_SET = 60; // Set ASP (not registered)
const int TAG_ITEM_FUNCTION_SPD_LIST = 61; // Open ASP popup menu
const int TAG_ITEM_FUNCTION_UNIT_MENU = 70; // Open unit settings popup menu
const int TAG_ITEM_FUNCTION_UNIT_SET = 71; // Set unit from menu (not registered)
#endif // !TAG_FUNCTION

#ifndef GENERAL_FUNCTION
#define GENERAL_FUNCTION
static constexpr double KN_KPH(const double& k) { return 1.85184 * k; } // 1 knot = 1.85184 kph
static constexpr double KPH_KN(const double& k) { return k / 1.85184; } // 1.85184 kph = 1 knot
static constexpr int OVRFLW2(const int& t) { return t > 99 || t < 0 ? 99 : t; } // overflow pre-process 2 digits
static constexpr int OVRFLW3(const int& t) { return t > 999 || t < 0 ? 999 : t; } // overflow pre-process 3 digits
static constexpr int OVRFLW4(const int& t) { return t > 9999 || t < 0 ? 9999 : t; }  // overflow pre-process 4 digits
inline std::string MakeUpper(const std::string& str);
inline bool IsCFLAssigned(CFlightPlan FlightPlan);
inline int GetLastRadarInterval(CRadarTargetPositionData pos1, CRadarTargetPositionData pos2);
inline int GetTrackingStatus(CFlightPlan FlightPlan);
constexpr auto TRACK_STATUS_NONE = 0;
constexpr auto TRACK_STATUS_MYSF = 1;
constexpr auto TRACK_STATUS_OTHR = -1;
#endif // !GENERAL_FUNCTION

#ifndef ES_SETTINGS
#define ES_SETTINGS
typedef struct _common_setting {
	const char* name;
	const char* fallback;
} CommonSetting;
typedef struct _color_setting {
	const char* name;
	const int code;
} ColorSetting;
inline std::string GetRealFileName(const std::string& path);
// NON-REALTIME READ SETTINGS, CHANGE BY COMMAND ONLY
constexpr auto SETTING_TRANS_LVL_CSV = "TransLevelCSV"; // file name
constexpr auto SETTING_ROUTE_CHECKER_CSV = "RteCheckerCSV"; // file name
constexpr auto SETTING_TRANS_MALT_TXT = "MetricAltitudeTXT"; // file name
const CommonSetting SETTING_CUSTOM_CURSOR = { "CustomCursor", "0" }; // bool, *0*
const CommonSetting SETTING_AUTO_RETRACK = { "AutoRetrack", "0" }; // *0*: off, 1: silent, 2: notify
const CommonSetting SETTING_AMEND_CFL = { "AmendQFEinCFL", "1" }; // 0: off, *1*: MTEP, 2: all
// REALTIME READ SETTINGS, CHANGE BY LOAD SETTINGS
constexpr auto SETTING_CUSTOM_NUMBER_MAP = "CustomNumber0-9"; // char[10]
// ALTITUDE
const CommonSetting SETTING_ALT_FEET = { "ALT/Feet", "0" }; // bool, *0*, include command
const CommonSetting SETTING_ALT_TOGG = { "ALT/ToggleDura", "5" }; // int, *5* seconds
// VERTICAL SPEED
const CommonSetting SETTING_VS_MODE = { "VS/Mode", "0" }; // bool, *0*: hide, 1: show, only valid for toggle, included in command.
const CommonSetting SETTING_VS_THLD = { "VS/Threshold", "100" }; // positive int, *100* FPM
const CommonSetting SETTING_VS_RNDG = { "VS/Rounding", "1" }; // positive int, *1* FPM
// GOUND SPEED
const CommonSetting SETTING_GS_KNOT = { "GS/Knot", "0" }; // bool, *0*, include command
const CommonSetting SETTING_GS_MODE = { "GS/ModeThreshold", "0" }; // int, *0* KTS
const CommonSetting SETTING_GS_TREND = { "GS/TrendThreshold", "5" }; // positive int, *5* KPH
const CommonSetting SETTING_GS_INC = { "GS/IncreaseMark", "" }; // char, *NULL*
const CommonSetting SETTING_GS_STA = { "GS/StableMark", "" }; // char, *NULL*
const CommonSetting SETTING_GS_DEC = { "GS/DecreaseMark", "" }; // char, *NULL*
// INDICATOR & FLAG
// X means extinguished, O means illuminated
// use GetPluginCharSetting for char because space is allowed
// use "\0" in settings file to explicit string terminator, to prevent being discarded by ES when saving settings
constexpr auto SETTING_UNIT_IND_1X = "Unit/Indicator1X"; // char, *NULL*
constexpr auto DEFAULT_UNIT_IND_1X = '\0';
constexpr auto SETTING_UNIT_IND_1O = "Unit/Indicator1O"; // char, *#*
constexpr auto DEFAULT_UNIT_IND_1O = '#';
constexpr auto SETTING_UNIT_IND_2X = "Unit/Indicator2X"; // char, * *(space)
constexpr auto DEFAULT_UNIT_IND_2X = ' ';
constexpr auto SETTING_UNIT_IND_2O = "Unit/Indicator2O"; // char, *NULL*
constexpr auto DEFAULT_UNIT_IND_2O = '\0';
// coordination indicator
const CommonSetting SETTING_FLAG_COORD_MODE = { "Flag/CoordinationAuto", "1" }; // bool, 0: manual *1*: auto on assume
constexpr auto SETTING_FLAG_COORD_X = "Flag/CoordinationX"; // char, *NULL*
constexpr auto DEFAULT_FLAG_COORD_X = '\0';
constexpr auto SETTING_FLAG_COORD_O = "Flag/CoordinationO"; // char, *C*
constexpr auto DEFAULT_FLAG_COORD_O = 'C';
// RVSM indicator - 3 modes
constexpr auto SETTING_FLAG_RVSM_O = "Flag/RvsmO"; // char, * *(space)
constexpr auto DEFAULT_FLAG_RVSM_O = ' ';
constexpr auto SETTING_FLAG_RVSM_X = "Flag/RvsmX"; // char, *X*
constexpr auto DEFAULT_FLAG_RVSM_X = 'X';
constexpr auto SETTING_FLAG_RVSM_V = "Flag/RvsmV"; // char, *V*
constexpr auto DEFAULT_FLAG_RVSM_V = 'V';
const CommonSetting SETTING_FLAG_EMG = { "Flag/Emergency", "EM:RF:HJ" }; // string, 2: EM:RF:HJ, 3: EMG:RDO:HIJ
const CommonSetting SETTING_FLAG_CLAM = { "Flag/CLAM", "CL" }; // string
const CommonSetting SETTING_FLAG_RAM = { "Flag/RAM", "RA" }; // string
const CommonSetting SETTING_FLAG_DUPE = { "Flag/DUPE", "DU" }; // string
// COLOR DEFINITIONS (R:G:B)
const ColorSetting SETTING_COLOR_CFL_CONFRM = { "Color/CFLNotAckd", TAG_COLOR_REDUNDANT };
const ColorSetting SETTING_COLOR_CS_SIMILR = { "Color/SimilarCallsign", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_COORD_FLAG = { "Color/CoordFlag", TAG_COLOR_REDUNDANT };
const ColorSetting SETTING_COLOR_RC_ALT = { "Color/RCLevelInvalid", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_SQ_DUPE = { "Color/SquawkDupe", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_DS_NUMBR = { "Color/DSRestore", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_DS_STATE = { "Color/DSNotCleared", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_RDRV_IND = { "Color/RadarVector", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_RECONT_IND = { "Color/Reconnected", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_RVSM_IND = { "Color/RVSMIndicator", TAG_COLOR_DEFAULT };
const ColorSetting SETTING_COLOR_SQ_7700 = { "Color/Squawk7700", TAG_COLOR_EMERGENCY };
const ColorSetting SETTING_COLOR_SQ_7600 = { "Color/Squawk7600", TAG_COLOR_EMERGENCY };
const ColorSetting SETTING_COLOR_SQ_7500 = { "Color/Squawk7500", TAG_COLOR_EMERGENCY };
const ColorSetting SETTING_COLOR_CLAM_FLAG = { "Color/CLAM", TAG_COLOR_INFORMATION };
const ColorSetting SETTING_COLOR_RAM_FLAG = { "Color/RAM", TAG_COLOR_INFORMATION };
#endif // !ES_SETTINGS

class CMTEPlugIn :
	public CPlugIn
{
public:
	CMTEPlugIn(void);
	~CMTEPlugIn(void);

	virtual void OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagMemData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	virtual void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);
	virtual void OnFlightPlanDisconnect(CFlightPlan FlightPlan);
	virtual void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
	virtual void OnRadarTargetPositionUpdate(CRadarTarget RadarTarget);
	virtual CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);
	virtual bool OnCompileCommand(const char* sCommandLine);

private:
	std::stack<std::shared_ptr<CMTEPScreen>> m_ScreenStack; // for StartTagFunction
	std::unique_ptr<RouteChecker> m_RouteChecker;
	std::unique_ptr<DepartureSequence> m_DepartureSequence;
	std::unique_ptr<TrackRecorder> m_TrackRecorder;
	std::unique_ptr<TransitionLevel> m_TransitionLevel;
	bool m_CustomCursor = false; // status, doesn't reflect setting

	template<typename T>
	inline T GetPluginSetting(const CommonSetting& setting);
	inline char GetPluginCharSetting(const char* setting, const char& fallback);

	inline int CalculateVerticalSpeed(CRadarTarget RadarTarget, bool rounded = false);
	void CallItemFunction(const char* sCallsign, const int& FunctionId, const POINT& Pt, const RECT& Area); // overload for ES internal function
	void CallItemFunction(const char* sCallsign, const char* sItemPlugInName, int ItemCode, const char* sItemString, const char* sFunctionPlugInName, int FunctionId, POINT Pt, RECT Area);
	bool GetColorDefinition(const ColorSetting setting, int* pColorCode, COLORREF* pRGB);
	void SetCustomCursor(void);
	void CancelCustomCursor(void);
	void LoadRouteChecker(void);
	void ResetDepartureSequence(void);
	void ResetTrackRecorder(void);
	void LoadTransitionLevel(void);
	void LoadMetricAltitude(void);
	std::string DisplayRouteMessage(const std::string& departure, const std::string& arrival);
};
