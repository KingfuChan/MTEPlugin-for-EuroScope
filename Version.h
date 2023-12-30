// Version.h
// Revision number is used in next Release build

#pragma once

#ifndef VERSION_NUM_H
#define VERSION_NUM_H

#define STR_(s) #s
#define STR(s) STR_(s)
#define VERSION_MAJOR 3
#define VERSION_MINOR 3
#define VERSION_REVISION 1
#define VERSION_BUILD 353

#define VERSION_FILE VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION
#define VERSION_FILE_STR \
    STR(VERSION_MAJOR) \
    "." STR(VERSION_MINOR) \
    "." STR(VERSION_REVISION)

#define VERSION_PRODUCT VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILD
#define VERSION_PRODUCT_STR \
    STR(VERSION_MAJOR) \
    "." STR(VERSION_MINOR) \
    "." STR(VERSION_REVISION) \
    "." STR(VERSION_BUILD)

#endif // VERSION_NUM_H
