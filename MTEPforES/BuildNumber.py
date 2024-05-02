import time
import re

RX_BUILD = r"#define VERSION_BUILD (.+)"
RX_TIME = r"#define BUILD_TIME (.+)"

originlines = open("Version.h", 'r').readlines()
newlines = []
for l in originlines:
    if (mb := re.match(RX_BUILD, l)) is not None:
        bn = int(mb[1])+1
        newlines.append(f"#define VERSION_BUILD {bn}\n")
    elif (mt := re.match(RX_TIME, l)) is not None:
        tm = time.strftime(r"%y%m%d%H%M", time.gmtime())
        newlines.append(f"#define BUILD_TIME {tm}\n")
    else:
        newlines.append(l)
open("Version.h", 'w', newline='\r\n').writelines(newlines)
