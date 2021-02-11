// MetricAlt.h

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"		
#include <map>

namespace MetricAlt {
	//std::map<int, int> m_mft;
	//std::map<int, int> m_ftm;

	int MtoFeet(const int meter);
	int FeettoM(const int feet);
	int LvlMtoFeet(const int meter);
	int LvlFeettoM(const int feet);
}