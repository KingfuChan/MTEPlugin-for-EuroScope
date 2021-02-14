# MTEPforES
Miscellaneous Tag Enhancement Plugin for EuroScope


## Tag Items and Behavious

1. **GS(KPH) with indicator** - Ground speed in kph, with accel(A) and decel(L) indicator.
2. **RMK/STS indicator** - shows a **\*** if RMK/ or STS/ is found in flight plan remarks.
3. **VS(fpm) in 4 digits** - vertical speed in xxxx, will not display if vs<=100 fpm.
4. **Level indicator** - combination of climb, descent, level flight indicator.
5. **Actual altitude (m)** - uses QNH altitude below transition level and STD altitude above.
6. **Cleared flight level (m)** - shows Chinese metric RVSM levels if matches, else calculats.
7. **Final flight level (m)** - shows Chinese metric RVSM levels if matches, else calculats.
8. **Similar callsign indicator** - algorithm is too complex to explain here and may contain bugs.


## Custom Cursor Settings

You may turn the default mouse arrow into a cross to simulate real-world radar screens.

Enter **.MTEP CURSOR ON** (case-insensitive) in your command line at the bottom of the screen to activate the cursor feature.

Enter **".MTEP CURSOR OFF** to de-activate.

This setting will be saved in your EuroScope plugin settings.
