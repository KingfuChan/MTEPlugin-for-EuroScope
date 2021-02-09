// MetricAlt.h

#pragma once

#include "pch.h"
#include "framework.h"
#include "resource.h"		
#include <map>

namespace MetricAlt {
	//std::map<int, int> m_mft;
	//std::map<int, int> m_ftm;

	int MtoFeet(int meter);
	int FeettoM(int feet);
	int LvlMtoFeet(int meter);
	int LvlFeettoM(int feet);
}