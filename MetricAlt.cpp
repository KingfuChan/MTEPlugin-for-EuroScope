// MetricAlt.cpp

#include "pch.h"
#include "MetricAlt.h"

int MetricAlt::MtoFeet(const int meter)
{
	return (int)round(meter * 3.28084);
}

int MetricAlt::FeettoM(const int feet)
{
	return (int)round(feet / 3.28084);
}

int MetricAlt::LvlMtoFeet(const int meter)
{
	auto m = m_mtof.find(meter);
	return m != m_mtof.end() ? m->second : MtoFeet(meter);
}

int MetricAlt::LvlFeettoM(const int feet)
{
	auto m = m_ftom.find(feet);
	return m != m_ftom.end() ? m->second : FeettoM(feet);
}

bool MetricAlt::RflFeettoM(const int feet, int& meter) {
	// matches to int& meter
	auto m = m_ftom.find(feet);
	if (m != m_ftom.end()) {
		meter = m->second;
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
