# MTEPforES

Miscellaneous Tag Enhancement Plugin for EuroScope (MTEPlugin)

## Tag Item Types

1. **GS(KPH) with trend indicator** - ground speed in kph, with accel(A) and decel(L) indicator. Trend threshold is 5 KPH compared to last radar position.
2. **RMK/STS indicator** - shows a **\*** if RMK/ or STS/ is found in flight plan remarks.
3. **Vertical speed (4-digit FPM)** - vertical speed in xxxx, will not display if vs<=100 fpm.
4. **Climb/Descend/Level indicator** - combination of climb, descent, level flight indicator. Threshold is 100 fpm.
5. **Actual altitude (m)** - uses QNH/QFE altitude below transition level and STD altitude above.
    + Allows custom number mapping if below transition level. See command line features below.
    + Transition level and QFE settings can be customized. See **Transition Level** below.
6. **Cleared flight level (m/FL)** - shows Chinese metric RVSM levels if matches (4 digits), or FLxxx (3 digits), otherwise calculated meters (4 digits).
7. **Cleared flight level (m)** - shows Chinese metric RVSM levels if matches, otherwise calculated meters.
    + Similar to item 6, but won't show ILS/VA. More useful in a Sweatbox simulator session.
8. **Final flight level (ICAO)** - shows Chinese metric RVSM levels if matches, otherwise flight level.
    + Sxxx (above transition level), Mxxx (below transition level), Fxxx (regardless of transition level).
9. **RFL unit indicator** - shows **#** if final altitude of tracked aircraft does not match Chinese metric RVSM levels.
10. **RVSM indicator** - shows **V** for VFR flights, **A SPACE** if aircraft has RVSM capability, **X** if not.
11. **RECAT-CN (H-B/C)** - wake turbulence re-categorization (RECAT-CN) for H(eavy) aircrafts. Only includes **-B -C**.
12. **RECAT-CN (LMCBJ)** - wake turbulence re-categorization (RECAT-CN) for all aircrafts. Includes **L M C B J**.
13. **Route validity** - route checker item, see detail below.
14. **Departure sequence** - departure sequence item, see detail below.
15. **Departure status** - departure sequence item, see detail below.
16. **Radar vector indicator** - shows **RV** for tracked aircraft with heading assigned.
    + The color is set by *Symbology Settings->Datablock->Information*.
17. **COMM ESTB indicator** - tracked recorder item, see detail below.
18. **Tracked DUPE warning** - tracked recorder item, see detail below.
19. **Similar callsign indicator** - tracked recorder item, see detail below.
20. **Reconnected indicator** - tracked recorder item, see detail below.

## Tag Item Functions

1. **Open CFL popup menu** - Chinese metric RVSM altitudes. Includes ILS/VA/NONE options.
2. **Open CFL popup edit** - Chinese metric RVSM altitudes.
3. **Open RFL popup menu** - Chinese metric RVSM altitudes.
   + CFL popup edit supports (standalone or in-menu): ***xxx*** for metric RVSM levels, ***Fxxx*** for FLxxx, ***550.*** for 550m, ***F4500.*** for 4500ft, etc. Enter ***0*** to clear CFL.
   + RFL popup edit (in-menu) supports: ***xxx*** for metric, ***Fxxx*** for FLxxx. Enter ***0*** to reset RFL to the final altitude in flight plan.
   + Enter a ***F*** or ***M*** (case-insensitive) will force all altitude displays for this aircraft in imperial or metric unit, only for tracekd aircrafts.
4. **Open assigned speed popup list** - open IAS or MACH assign list based on current altitude. IAS for 7500m/FL246 and below, MACH for above.
5. **Show route checker info** - route checker function, see detail below.
6. **Set departure sequence** - departure sequence function, see detail below.
7. **Set departure status** - departure sequence function, see detail below.
8. **Set COMM ESTB** - tracked recorder function, see detail below.
9. **Open similar callsign list** - tracked recorder function, see detail below.
10. **Restore assigned data** - tracked recorder function, see detail below.

## Route Checker

This module will automatically check route validity. Requires a CSV file in the format below:

Dep|Arr|Name|EvenOdd|AltList|MinAlt|Route|Remarks
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
|ZBAA/ZBAD|ZSSS/ZSPD|test|SE/SO/FE/FO|S81/S89/S107/F350/F450|9800|ELKUR .... SASAN|test only

+ ***Dep, Arr*** can be a list seperated by "/".
+ ***Name*** no need to explain.
+ ***EvenOdd*** should be a combination of SE(metric, even) SO(metric, odd) FE(imperial, even) FO(imperial, odd). No seperation mark is required.
+ ***AltList*** means level restriction, which can also be a list seperated by "/".
  + Only exact altitudes will be valid. When ***AltList*** is not empty, ***EvenOdd*** will be deprecated.
  + For metric altitudes, use Sxxx, e.g. S81 for 8100m, S107 for 10700m.
  + For imperial flight levels, use Fxxx, e.g. F350 for FL350.
+ ***MinAlt*** should be in feet. If the flight plan final altitude is lower than given value, it should be an invalid flight plan.
+ ***Route*** is not necessarily the full route but can be partial. One DEP and ARR pair can have multiple routes with different restrictions.
+ ***Remarks*** no need to explain.

CSV files with incorrect column names or not in the given order, will not be loaded. Empty cells are accepted. However it's still possible to cause unpredicted issues if any cells doesn't follow the rules above.

You need to use a command line to load the CSV file: **.MTEP RC PATH** (case-insensitive). **PATH** should be replace by the CSV file path and file name.

+ If you use relative path, please note it's based on working directory where EuroScope.exe is running.
+ Or you can insert **@** to the front of the path to make it relative to the DLL file.
+ The file setting will be saved in your EuroScope plugin settings.

Tag item type **Route validity** shows:

||Route|Final Altitude|
|:--|:--:|:--:|
|**Y**|Yes|Yes|
|**P**|Partial|Yes|
|**YL**|Yes|No|
|**PL**|Partial|No|
|**X**|No|/|
|**?**|Not Found|/|

+ Blanks out when route checker is not configured or clearance received flag is set.
+ Different colors are used to distinguish two methods. Default color implies text-comparison method (**Y/YL/?**). Color set by *Symbology Settings->Datablock->Redundant* implies structurized-comparison method, in which case it could be inaccurate. Color set by *Symbology Settings->Datablock->Information* implies invalid route (**X**) with both methods.

Tag item function **Show route checker info**: Displays a *MTEP-Route* message in chat list and shows route information for current DEP-ARR if seleted flight plan is invalid.

Command line function: **.MTEP RC DDDD AAAA** will display the valid routes from DDDD to AAAA and copy to clipboard.

## Departure Sequence

This module is a simplified version of my [Departure-List-Sequencing-PlugIn](https://github.com/KingfuChan/Departure-List-Sequencing-PlugIn-for-EuroScope) with different display method, no synchronization, but consistent with default EuroScope groud state.

Tag item type **Departure sequence** shows:

+ **two digits number 01~99** - the sequence of the flight.
+ **--** - indicating a reconnected flight. The color is set by *Symbology Settings->Datablock->Information*.

Tag item function **Set departure sequence** is used to:

+ initiate a sequence - if no previous sequence at all.
+ edit a sequence - if the flight has a active sequence.
+ re-activate a sequence - if the flight has just reconnected.
+ delete a sequence - entering 0 in the popup edit.

There are 3 ways to remove a flight from the queue:

+ entering 0 in popup edit.
+ set any of the ground states (NSTS, STUP, PUSH, TAXI, DEPA).
+ setting clearance flag for flights without a ground state (NSTS).

There are 2 ways to re-activate reconnected flights and restoring previous sequence:

+ resetting previous ground state.
+ use **Set departure sequence** for NSTS flights.

Command line function: **.MTEP DS RESET** resets all memories.

Tag item type **Departure status** provides a compound display for ground status by adding clearance received flag (**CLRD**) to native EuroScope **Ground status** item type (**STUP, PUSH, TAXI, DEPA** depending on Euroscope version).

+ **CLRD** is shown when clearance received flag is set and no ground status.
+ Use different color when there is a ground status but clearanced received flag hasn't been set. The color is set by *Symbology Settings->Datablock->Information*.

Tag item function **Set departure status** is used to set clearance received flag and ground status.

+ Sets clearance received flag when it isn't.
+ Opens ground status popup list when clearance received flag is set.
+ Setting **NSTS** will reset clearance received flag if no ground status is present.
+ This function requires permission to draw on display types. Go to *Plug-ins settings->Allow to draw on types* and add ALL display types.

## Tracked Recorder

Some of the tag item types and functions are viable through this module. This module stores data of aircrafts tracked by myself.

Tag item types:

+ **Cleared flight level (m/FL)** - confirmed status is stored in tracked recorder.
  + The color will be set by *Symbology Settings->Datablock->Redundant* if a new flight level is cleared but not confirmed.
+ **Actual altitude (m), Cleared flight level (m/FL), Final flight level (ICAO)** - force altitude display unit is only available for aircrafts tracked by myself.
+ **Similar callsign indicator** - shows **SC** if similar callsigns in tracked flights are detected.
  + Considers Chinese/English crews and ignores those text-only. Enter *\*EN* in scratch pad to distinguish Chinese airlines speaking English.
  + The color is set by *Symbology Settings->Datablock->Information*.
+ **COMM ESTB indicator** - shows a **C** at tracking. Works with **Set COMM ESTB** function.
  + The color is set by *Symbology Settings->Datablock->Redundant*.
+ **Tracked DUPE warning** - squawk DUPE warning only for aircrafts tracked by myself.
  + The color is set by *Symbology Settings->Datablock->Information*.
+ **Reconnected indicator** - shows **r** if reconnected. With auto retrack mode 1/2, should show nothing (see command line features below).
  + The color is set by *Symbology Settings->Datablock->Information*.

Tag item functions:

+ **Set COMM ESTB** - establish communication and extinguishes **COMM ESTB indicator**.
+ **Open similar callsign list** - shows a list of all callsigns that are similar to the current one.
  + Selecting one will toggle native ***.find*** command.
+ **Restore assigned data** - restore previously assigned data for reconnected flights and start tracking.
  + Assigned data includes: *communication type, squawk, heading/DCT point, cleared altitude, final altitude, speed/Mach, rate, scratch pad*.

Related command line functions:

+ **.MTEP TR 0/1/2** - set auto retrack mode. This setting will be saved in your EuroScope plugin settings.
  + **(0)** no auto retrack;
  + **(1)** auto retrack, no notifications;
  + **(2)** auto retrack, notified through *MTEP-Recorder* message.
+ **.MTEP TR RESET** - resets tracked recorder. Use this command if this module is not working properly. Note that it also deletes all saved data for reconnected flights.

## Transition Level

EuroScope native transition altitude cannot be seperately applied to airports with different transition altitudes. This module allows such practice, and improves display logic. AFL originally use QNH altitudes when below transition altitude and use QNE altitudes when above transition altitude. This plugin considers transition level instead of transition altitude for better results. In addition, this module provides QFE altitudes on demand.

A CSV file is required for the customization, in the format below.

|Ident|TransLevel|Elevation|IsQFE|Range|Boundary|
|:---:|:---:|:---:|:---:|:---:|:---:|
|ZGSZ|S33|13|0||112.7/21.0 112.5/21.9 112.5/22.2 ...|
|ZPLJ|S66|7359|0|50||
|ZGNN|S36|420|1||109.1/23.1 109.1/22.5 108.6/22.0 ...|
|ZSWH||148|1|50||
|VHHH|F110|28|0||113.6/22.2 113.8/22.4 114.2/22.6 ...|

+ ***Ident*** is the ICAO identification for a single airport.
+ ***TransLevel*** only accepts Sxxx or Fxxx. E.g. S33 for 3300m, S60 for 6000m, F110 for FL110/11000ft. If this field is empty, the module will instead use EuroScope internal setting, *General Settings (in setting file) -> m_TransitionAltitude*.
+ ***Elevation*** is the airport elevation in feet. This will be redundant if QFE is not in use.
+ ***IsQFE*** determines whether QFE is in use. Use 0 (by default) for QNH.
+ ***Range*** (in nautical miles) is used to set a range for QNH/QFE lateral boundary. If the value is greater than 0, the following ***Boundary*** entry will be ignored.
+ ***Boundary*** should be a list of coordinates delineating QNH/QFE lateral boundary. Longitude/latitude values are in decimals (supports more digits for better precision); use space as seperator.

CSV files with incorrect column names or not in the given order, will not be loaded. It's possible to cause unpredicted issues if any cells doesn't follow the rules above.

It's recommended to fill in either ***Range*** or ***Boundary***. If an aircraft is outside of all lateral boundaries/radius, **Actual altitude (m)** will always show in QNE, even below EuroScope internal transition altitude. This also suggests entering ALL available airports in csv. When using QFE, **Actual altitude (m)** and **Cleared flight level (m/FL)** will show in parentheses "**()**", and custom number mapping (see below) won't apply.

When QFE in use and under certain circumstances, tag item function **Open CFL popup menu** and **Open CFL popup edit** will automatically convert elevation (QFE) to altitude (QNH), by adding the elevation of airport to EuroScope internal cleared altitude. This helps eliminate CLAM warnings for airports at higher altitudes, but will cause discrepancy between **Cleared flight level (m/FL)** and all other native item types (e.g. Matias).

Related command line functions:

+ **.MTEP TL PATH** - **PATH** should be replace by the CSV file path and file name. Rules for **Route Checker** also applies.
+ **.MTEP ICAO TL S/Fxxx** - sets the transition level for airport **ICAO**. Use the same altitude format as the csv.
+ **.MTEP ICAO QFE/QNH** - sets QFE or QNH for the airport.
+ **.MTEP ICAO R xx** - sets the range (in nautical miles) for the airport.

All customizations for single airport through command line functions won't be saved to the csv file.

## Other Command Line Features

All command line functions are case-insensitive, including those mentioned above.

1. **.MTEP FR24 ICAO / .MTEP VARI ICAO** - opens [Flightradar24](https://www.flightradar24.com/) / [飞常准ADS-B](https://flightadsb.variflight.com/) in web browser and centers the map on the given **ICAO** airport. Only works with airports within sector file.
2. **.MTEP CURSOR ON/OFF** - turns mouse cursor into Topsky or Eurocat style; may conflict with other plugins. On Windows 10 1607 or later systems, the size of cursor is set according to current EuroScope Hi-DPI setting. This setting will be saved in your EuroScope plugin settings.
3. **.MTEP NUM 0123456789** - sets custom number mapping to replace corresponding 0-9 characters, which will be used in **Actual altitude (m)** if below transition level (Tips: use with custom font, e.g. number underscores). Use at own risk of crashing EuroScope (offline setting recommended). This setting will be saved in your EuroScope plugin settings. Note that not all characters are available through command line, in which case a direct modification in settings should work.
