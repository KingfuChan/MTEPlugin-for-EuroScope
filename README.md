# MTEPforES

Miscellaneous Tag Enhancement Plugin for EuroScope

## Tag Item Types and Behaviours

1. **GS(KPH) with indicator** - ground speed in kph, with accel(A) and decel(L) indicator.
2. **RMK/STS indicator** - shows a **\*** if RMK/ or STS/ is found in flight plan remarks.
3. **VS(fpm) in 4 digits** - vertical speed in xxxx, will not display if vs<=100 fpm.
4. **Level indicator** - combination of climb, descent, level flight indicator.
5. **Actual altitude (m)** - uses QNH altitude below transition level and STD altitude above.
6. **Cleared flight level (m)** - shows Chinese metric RVSM levels if matches, or FLxxx, otherwise calculated meters.
7. **Final flight level (m/FL)** - shows Chinese metric RVSM levels if matches, otherwise Flight Level in feet.
8. **Similar callsign indicator** - shows **SC** if similar callsigns are detected. Flight plan status, /t and CN/EN are considered.
9. **RFL unit indicator** - shows **#** if final altitude does not match Chinese metric RVSM levels.
10. **RVSM indicator** - shows **V** for VFR flights, **A SPACE** if aircraft has RVSM capability, **X** if not.
11. **COMM ESTB indicator** - shows a white **C** when assuming. Use **Set COMM ESTB** function to cancel this **C**.
12. **RECAT-CN** - Re-categorization (Chinese) for H(eavy) aircrafts. Only includes **-B -C**.
13. **Route validity** - route checker item, see detail below.

## Tag Item Functions

1. **Set COMM ESTB** - establish communication and extinguishes **COMM ESTB indicator**.
2. **Open CFL popup menu** - Chinese metric RVSM altitudes, along with ILS/VA/NONE options.
3. **Open RFL popup menu** - Chinese metric RVSM altitudes.
   + Supports keyboard entry: ***xxx*** for metric, ***Fxxx*** for FLxxx, ***550.*** for 550m, ***F4500.*** for 4500ft, etc.
4. **Open similar callsign list** - shows a list of all callsigns that are similar to the current one. Selecting one will set the ASEL aircraft, which works the same as a mouse click so it can be used along with command lines and function keys (open list first, then enter commands or keys, finally select in the list).
5. **Show route checker info** - route checker function, see detail below.

## Custom Cursor Settings

You may turn the default mouse arrow into a cross to simulate real-world radar screens.

Enter **.MTEP CURSOR ON** (case-insensitive) in your command line at the bottom of the screen to activate the cursor feature.

Enter **.MTEP CURSOR OFF** to de-activate.

This setting will be saved in your EuroScope plugin settings.

## Route Checker

This module will automatically check route validity. Requires a CSV file in the format below:

|Dep|Arr|EvenOdd|LevelRestr|Route|
|:---:|:---:|:---:|:---:|:---:|
|ZBAA|ZGGG|EVEN||OMDEK .... ATAGA|
|ZBAA|ZSSS/ZSPD|ODD||ELKUR .... SASAN|
|ZBAA/ZBAD|ZYHB|ODD||DOTRA .... PIGAM|
|ZSSS|ZSOF|EVEN|157|POMOK .... MADUK|
|ZSSS|VHHH|EVEN|276/256|NXD .... MAGOG|

+ ***Dep, Arr*** can be a list seperated by "/".
+ ***EvenOdd*** can only be "EVEN" or "ODD" (uppercase), other values will be ignored.
+ ***LevelRestr*** means level restriction, which can also be a list.
  + Only exact altitudes will be valid. When ***LevelRestr*** is not empty, ***EvenOdd*** will be deprecated.
  + Currently it only supports Chinese Metric RVSM but inside CSV file please use feet as the samples above.
+ ***Route*** is not necessarily the full route but can be partial. One DEP and ARR pair can have multiple routes with different restrictions.

CSV files with incorrect format will not be loaded. However it's still possible to cause unpredicted issues if file is corrupted.

You need to use a command line to load the CSV file: **.MTEP RC PATH** (case-insensitive). **PATH** should be replace by the CSV file path and file name. If you use relatvie path, please note it's based on working directory instead of the DLL path. The file setting will be saved in your EuroScope plugin settings.

Tag item type **Route validity** shows:

+ **Y** - both route and final altitude is valid.
+ **L** - route is valid but final altitude is not.
+ **N** - route is not valid.
+ **A SPACE** - no route is found for this DEP-ARR.

Tag item function **Show route checker info**: Displays a *MTEP-Route* message in chat list and shows route information regarding current DEP-ARR if current route is invalid.

## Other Command Line Features

1. **.MTEP FR24 ICAO / .MTEP VARI ICAO** - opens ***www.flightradar24.com / flightadsb.variflight.com*** in web browser and centers the map on the given **ICAO** airport. Only works with airports within sector file.
