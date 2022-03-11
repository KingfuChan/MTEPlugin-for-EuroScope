// SimilarCallsign.cpp

#include "pch.h"
#include "SimilarCallsign.h"

typedef list<char> char_list;

char_list ExtractNumfromCallsign(const string callsign);
bool CompareFlightNum(char_list num1, char_list num2);

bool CompareCallsign(string callsign1, string callsign2)
{
	// compares two complete callsigns
	bool isSimilar = false;
	char_list cs1 = ExtractNumfromCallsign(callsign1);
	char_list cs2 = ExtractNumfromCallsign(callsign2);
	if (!cs1.size() || !cs2.size()) // one of it doesn't have a number
		isSimilar = false;
	else if (cs1.size() <= 1 && cs2.size() <= 1) { // prevents (1,1) bug in CompareCallsignNum()
		isSimilar = cs1 == cs2;
	}
	else if (cs1.size() == cs2.size()) {
		isSimilar = CompareFlightNum(cs1, cs2);
	}
	else {
		// exchange, make cs1 the longer callsign for justification
		char_list cst = cs1.size() < cs2.size() ? cs1 : cs2;
		cs1 = cs1.size() > cs2.size() ? cs1 : cs2;
		cs2 = cst;
		char_list csl, csr;
		size_t i = 0;
		for (csl = cs2, csr = cs2; i < cs1.size() - cs2.size(); i++) {
			// use space to fill digits
			csl.push_back(' ');
			csr.push_front(' ');
		}
		isSimilar = CompareFlightNum(cs1, csl) || CompareFlightNum(cs1, csr);
	}
	return isSimilar;
}

char_list ExtractNumfromCallsign(const string callsign)
{
	// extract num from callsign
	char_list csnum;
	bool numbegin = false;
	for (size_t i = 0; i < callsign.size(); i++) {
		numbegin = numbegin || (callsign[i] >= '1' && callsign[i] <= '9'); // excluding leading '0's
		if (numbegin)
			csnum.push_back(callsign[i]);
	}
	return csnum;
}

bool CompareFlightNum(char_list num1, char_list num2)
{
	// compares two callsign, char_list in same size
	int size;
	if ((size = num1.size()) != num2.size()) return false;
	char_list::iterator p1, p2;
	int same = 0; // same number on same position count
	char_list dn1, dn2; // different number on same position list
	for (p1 = num1.begin(), p2 = num2.begin(); p1 != num1.end() && p2 != num2.end(); p1++, p2++) {
		if (*p1 == *p2)
			same++;
		else {
			dn1.push_back(*p1);
			dn2.push_back(*p2);
		}
	}
	bool isSimilar = false;
	if (size - same <= 1)
		isSimilar = true;
	else if (size - same <= size - 2) {
		dn1.sort();
		dn2.sort();
		isSimilar = dn1 == dn2;
	}
	return isSimilar;
}
