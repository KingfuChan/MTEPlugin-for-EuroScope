# MTEPforES
Miscellaneous Tag Enhancement Plugin for EuroScope


## Tag Items and Behavious

1. **GS(KPH) with indicator** - ground speed in kph, with accel(A) and decel(L) indicator.
2. **RMK/STS indicator** - shows a **\*** if RMK/ or STS/ is found in flight plan remarks.
3. **VS(fpm) in 4 digits** - vertical speed in xxxx, will not display if vs<=100 fpm.
4. **Level indicator** - combination of climb, descent, level flight indicator.
5. **Actual altitude (m)** - uses QNH altitude below transition level and STD altitude above.
6. **Cleared flight level (m)** - shows Chinese metric RVSM levels if matches, or FLxxx, otherwise calculated meters.
7. **Final flight level (m/FL)** - shows Chinese metric RVSM levels if matches, otherwise Flight Level in feet.
8. **Similar callsign indicator** - shows **SC** if similar callsigns are detected. Note that /t and CN/EN are considered.
9. **RFL unit indicator** - shows **#** if final altitude does not match Chinese metric RVSM levels.
10. **RVSM indicator** - shows **V** for VFR flights, **A SPACE** if aircraft has RVSM capability, **X** if not.
11. **COMM ESTB indicator** - shows a white **C** when assuming. Use **Set COMM ESTB** function to cancel this **C**.
12. **RECAT-CN** - Re-categorization (Chinese) for H(eavy) aircrafts. Only includes **-B -C**.


## Tag Functions

1. **Set COMM ESTB** - establish communication and extinguishes **COMM ESTB indicator**.
2. **Open CFL popup list** - Chinese metric RVSM altitudes along with ILS/VA/NONE options. Supports keyboard entry: ***xxx*** for metric, ***Fxxx*** for FLxxx.
3. **Open RFL popup list** - Chinese metric RVSM altitudes. Supports keyboard entry: ***xxx*** for metric, ***Fxxx*** for FLxxx.


## Custom Cursor Settings

You may turn the default mouse arrow into a cross to simulate real-world radar screens.

Enter **.MTEP CURSOR ON** (case-insensitive) in your command line at the bottom of the screen to activate the cursor feature.

Enter **".MTEP CURSOR OFF** to de-activate.

This setting will be saved in your EuroScope plugin settings.
