// MetricAlt.h

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"		
#include <map>

namespace MetricAlt {
	int MtoFeet(const int meter);
	int FeettoM(const int feet);
	int LvlMtoFeet(const int meter);
	int LvlFeettoM(const int feet);
	bool RflFeettoM(const int feet, int& meter);
}