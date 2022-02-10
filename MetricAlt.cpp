// MetricAlt.cpp

#include "pch.h"
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

string MetricAlt::LvlFeetEvenOdd(const int feet)
{
	int i = 1; // first one is odd
	for (auto it = m_ftom.begin(); it != m_ftom.end(); it++, i++) {
		if (it->first == feet)
			return i % 2 ? "SO" : "SE";
	}
	if (!(feet % 1000)) { // imperial
		int ft = feet / 1000;
		if (ft <= 41)
			return ft % 2 ? "FO" : "FE";
		else if (ft <= 60 && !((ft - 41) % 2))
			return (ft - 41) / 2 % 2 ? "FE" : "FO";
	}
	return "NONE";
}
