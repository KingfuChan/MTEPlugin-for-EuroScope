// MetricAlt.cpp

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "MetricAlt.h"

int MetricAlt::MtoFeet(const int meter)
{
	return round(meter * 3.28084);
}

int MetricAlt::FeettoM(const int feet)
{
	return round(feet / 3.28084);
}

int MetricAlt::LvlMtoFeet(const int meter)
{
	return m_mtof.count(meter) ? m_mtof.at(meter) : MtoFeet(meter);
}

int MetricAlt::LvlFeettoM(const int feet)
{
	return m_ftom.count(feet) ? m_ftom.at(feet) : FeettoM(feet);
}

bool MetricAlt::RflFeettoM(const int feet, int& meter) {
	// matches to int& meter
	if (m_ftom.count(feet)) {
		meter = m_ftom.at(feet);
		return true;
	}
	else {
		return false;
	}
}