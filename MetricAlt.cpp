// MetricAlt.cpp

#include "pch.h"
#include "MetricAlt.h"

using namespace MetricAlt;

vector<AltitudeEntry> m_AltStrMap = v_atos1; // initialize with default fallback

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

bool MetricAlt::LoadAltitudeDefinition(const string filename)
{
	m_AltStrMap.clear(); // clear previous record
	ifstream inFile;
	inFile.open(filename, ios::in);
	if (!inFile.is_open()) // unable to open file
	{
		m_AltStrMap = v_atos1;
		throw string("unable to open file");
	}
	string line;
	try {
		while (getline(inFile, line)) {
			istringstream ssin(line);
			string stra, strm, strf, strma, strfa;
			getline(ssin, stra, '\t');
			getline(ssin, strm, '\t');
			getline(ssin, strf, '\t');
			getline(ssin, strma, '\t');
			getline(ssin, strfa, '\t');
			int alt = stoi(stra);
			if (strm.size() || strf.size())
				m_AltStrMap.push_back({ alt, strm, strf, strma, strfa });
		}
		inFile.close();
		return true;
	}
	catch (...) {
		inFile.close();
		m_AltStrMap = v_atos1;
		throw;
	}
	return false;
}

vector<AltitudeMenuEntry> MetricAlt::GetMenuItems(const bool metric, const int trans_level)
{
	vector<AltitudeMenuEntry> res;
	for (auto& as : m_AltStrMap) {
		string s = "";
		if (metric) {
			if (as.m.size()) {
				if (as.ma.size() && as.alt > 3 && as.alt < trans_level) {
					res.push_back({ as.alt, as.ma });
					continue;
				}
				else {
					res.push_back({ as.alt, as.m });
					continue;
				}
			}
			if (as.ma.size()) {
				res.push_back({ as.alt, as.ma });
			}
		}
		else {
			if (as.f.size()) {
				if (as.fa.size() && as.alt > 3 && as.alt < trans_level) {
					res.push_back({ as.alt, as.fa });
					continue;
				}
				else {
					res.push_back({ as.alt, as.f });
					continue;
				}
			}
			if (as.fa.size()) {
				res.push_back({ as.alt, as.fa });
			}
		}
	}
	return res;
}

int MetricAlt::GetAltitudeFromMenuItem(const string menuItem, const bool metric)
{
	for (auto& as : m_AltStrMap) {
		string s = metric ? as.m : as.f;
		string sa = metric ? as.ma : as.fa;
		if (menuItem == s || menuItem == sa)
			return as.alt;
	}
	return ALT_MAP_NOT_FOUND;
}
