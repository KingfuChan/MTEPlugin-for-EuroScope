// SimilarCallsign.cpp

#include "pch.h"
#include "SimilarCallsign.h"

std::string ExtractNumfromCallsign(const std::string& callsign)
{
	// extract num from callsign
	std::string res;
	bool num1st = false;
	copy_if(callsign.begin(), callsign.end(), back_inserter(res), [&num1st](char c) {
		num1st = num1st || (c >= '1' && c <= '9'); // leading '0's are excluded
		return num1st;
		});
	return res;
}

bool CompareFlightNum(const std::string& num1, const std::string& num2)
{
	// compares two callsigns in same size
	if (num1.size() != num2.size()) return false;
	// compare by character
	std::vector<bool> diff_pos(num1.size(), false);
	transform(num1.begin(), num1.end(), num2.begin(), diff_pos.begin(),
		[](char a, char b) {return a != b; });
	size_t diff_cnt = count(diff_pos.begin(), diff_pos.end(), true);
	if (diff_cnt <= 1) { // less than 1 difference
		return true;
	}
	else if (diff_cnt == 2) {
		std::set<char> dif1, dif2; // unique and sorted
		for (size_t i = 0; i < diff_pos.size(); i++) {
			if (diff_pos[i]) {
				dif1.insert(num1[i]);
				dif2.insert(num2[i]);
			}
		}
		return dif1 == dif2;
	}
	return false;
}

bool CompareCallsign(const std::string& callsign1, const std::string& callsign2)
{
	// compares two complete callsigns
	std::string cs1 = ExtractNumfromCallsign(callsign1);
	std::string cs2 = ExtractNumfromCallsign(callsign2);
	if (!cs1.size() || !cs2.size()) { // one of it doesn't have a number
		return false;
	}
	else if (cs1.size() <= 1 && cs2.size() <= 1) { // prevents (1,1) bug in CompareCallsignNum()
		return cs1 == cs2;
	}
	else if (cs1.size() == cs2.size()) {
		return CompareFlightNum(cs1, cs2);
	}
	else {
		// exchange, make cs1 the longer callsign for justification
		size_t max_size = max(cs1.size(), cs2.size());
		std::string cs1_left = std::string(max_size - cs1.size(), ' ') + cs1;
		std::string cs2_left = std::string(max_size - cs2.size(), ' ') + cs2;
		std::string cs1_right = cs1 + std::string(max_size - cs1.size(), ' ');
		std::string cs2_right = cs2 + std::string(max_size - cs2.size(), ' ');
		return CompareFlightNum(cs1_left, cs2_left) || CompareFlightNum(cs1_right, cs2_right);
	}
}
