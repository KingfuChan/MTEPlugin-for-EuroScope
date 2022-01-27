# MTEPforES

Miscellaneous Tag Enhancement Plugin for EuroScope (MTEPlugin)

## Tag Item Types and Behaviours

1. **GS(KPH) with trend indicator** - ground speed in kph, with accel(A) and decel(L) indicator.
2. **RMK/STS indicator** - shows a **\*** if RMK/ or STS/ is found in flight plan remarks.
3. **Vertical speed (4-digit FPM)** - vertical speed in xxxx, will not display if vs<=100 fpm.
4. **Climb/Descend/Level indicator** - combination of climb, descent, level flight indicator.
5. **Actual altitude (m)** - uses QNH altitude below transition level and STD altitude above.
6. **Cleared flight level** - shows Chinese metric RVSM levels if matches, or FLxxx, otherwise calculated meters.
7. **Final flight level** - shows Chinese metric RVSM levels if matches, otherwise Flight Level in feet.
8. **Similar callsign indicator** - shows **SC** if similar callsigns are detected. Flight plan status, /t and CN/EN are considered.
9. **RFL unit indicator** - shows **#** if final altitude does not match Chinese metric RVSM levels.
10. **RVSM indicator** - shows **V** for VFR flights, **A SPACE** if aircraft has RVSM capability, **X** if not.
11. **COMM ESTB indicator** - shows a white **C** when assuming. Use **Set COMM ESTB** function to cancel this **C**.
12. **RECAT-CN** - Re-categorization (Chinese) for H(eavy) aircrafts. Only includes **-B -C**.
13. **Route validity** - route checker item, see detail below.
14. **Tracked DUPE warning** - squawk DUPE warning only for in-air aircrafts tracked by myself.
15. **Departure sequence** - departure sequence item, see detail below.

## Tag Item Functions

1. **Set COMM ESTB** - establish communication and extinguishes **COMM ESTB indicator**.
2. **Open CFL popup menu** - Chinese metric RVSM altitudes, along with ILS/VA/NONE options.
3. **Open RFL popup menu** - Chinese metric RVSM altitudes.
   + Supports keyboard entry: ***xxx*** for metric, ***Fxxx*** for FLxxx, ***550.*** for 550m, ***F4500.*** for 4500ft, etc.
4. **Open similar callsign list** - shows a list of all callsigns that are similar to the current one. Selecting one will set the ASEL aircraft, which works the same as a mouse click so it can be used along with command lines and function keys (open list first, then enter commands or keys, finally select in the list).
5. **Show route checker info** - route checker function, see detail below.
6. **Set departure sequence** - departure sequence function, see detail below.

## Custom Cursor Settings

You may turn the default mouse arrow into a cross to simulate real-world radar screens. This only works with the original DLL file name (MTEPlugin.dll).

Enter **.MTEP CURSOR ON** (case-insensitive) in your command line at the bottom of the screen to activate the cursor feature.

Enter **.MTEP CURSOR OFF** to de-activate.

This setting will be saved in your EuroScope plugin settings.

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

CSV files with incorrect columns will not be loaded. Empty cells are accepted. However it's still possible to cause unpredicted issues if any cells doesn't follow the rules above.

You need to use a command line to load the CSV file: **.MTEP RC PATH** (case-insensitive). **PATH** should be replace by the CSV file path and file name.

+ If you use relative path, please note it's based on working directory.
+ Or you can insert **@** to the front of the path to make it relative to the DLL file. This only works with the original DLL file name (MTEPlugin.dll).
+ The file setting will be saved in your EuroScope plugin settings.

Tag item type **Route validity** shows:

+ **Y** - both route and final altitude is valid.
+ **L** - route is valid but final altitude is not.
+ **X** - route is not valid.
+ **?** - no route is found for this DEP-ARR.
+ shows nothing when clearance received flag is set or route checker is not configured.

Tag item function **Show route checker info**: Displays a *MTEP-Route* message in chat list and shows route information for current DEP-ARR if seleted flight plan is invalid.

Command line function: **.MTEP RC DDDD AAAA** will show the valid routes for DDDD-AAAA if found.

## Departure Sequence

This module is a simplified version from my GitHub repository [Departure-List-Sequencing-PlugIn](https://github.com/KingfuChan/Departure-List-Sequencing-PlugIn-for-EuroScope).

## Other Command Line Features

1. **.MTEP FR24 ICAO / .MTEP VARI ICAO** - opens ***www.flightradar24.com / flightadsb.variflight.com*** in web browser and centers the map on the given **ICAO** airport. Only works with airports within sector file.
