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
	return m_mf.count(meter) ? m_mf.at(meter) : MtoFeet(meter);
}

int MetricAlt::LvlFeettoM(const int feet)
{
	return m_fm.count(feet) ? m_fm.at(feet) : FeettoM(feet);
}

bool MetricAlt::RflFeettoM(const int feet, int& meter) {
	// matches to int& meter
	if (m_fm.count(feet)) {
		meter = m_fm.at(feet);
		return true;
	}
	else {
		return false;
	}
}