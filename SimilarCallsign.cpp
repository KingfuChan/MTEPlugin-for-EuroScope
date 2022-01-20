// SimilarCallsign.cpp

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "SimilarCallsign.h"

using namespace std;

const unordered_set<string> m_CHNCallsign = {
	"ALP","AYE","BDJ","BFM","BGC","BJN","BNH","BSH","CAF","CAO",
	"CBG","CBJ","CCA","CCD","CCO","CCS","CDC","CDG","CES","CFA",
	"CFB","CFI","CFZ","CGH","CGN","CGZ","CHB","CHC","CHF","CHH",
	"CJX","CKK","CNM","CNW","CQH","CQN","CSC","CSG","CSH","CSN",
	"CSS","CSY","CSZ","CTH","CTJ","CUA","CUH","CWR","CXA","CXN",
	"CYH","CYN","CYZ","DER","DGA","DKH","DLC","DXH","EPA","EPB",
	"FJT","FSJ","FTU","FZA","GCR","GDC","GSC","GWL","HAH","HBH",
	"HFJ","HHG","HLF","HNJ","HSJ","HTK","HXA","HYN","HYT","ICU",
	"JAE","JBE","JDL","JGJ","JHK","JOY","JSU","JYH","KJT","KNA",
	"KPA","KXA","LHA","LKE","LLJ","LNM","MSF","MZT","NEJ","NMG",
	"NSJ","NSY","OKA","OLD","OMA","OTC","OTT","PHF","QDA","QJT",
	"QSR","RBW","RFH","RLH","SHQ","SNG","SXS","SZA","TBA","TXJ",
	"UEA","UNA","UTP","VGA","VRE","WFH","WLF","WUA","XAI","XTH",
	"YZR",
};

bool IsChineseCallsign(EuroScopePlugIn::CFlightPlan FlightPlan) {
	string scrpd = FlightPlan.GetControllerAssignedData().GetScratchPadString();
	string csicao = string(FlightPlan.GetCallsign()).substr(0, 3);
	return m_CHNCallsign.count(csicao) && scrpd.find("*EN") == -1;
}

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
		int i = 0;
		for (csl = cs2, csr = cs2; i < cs1.size() - cs2.size(); i++) {
			// use space to fill digits
			csl.push_back(' ');
			csr.push_front(' ');
		}
		isSimilar = CompareFlightNum(cs1, csl) || CompareFlightNum(cs1, csr);
	}
	return isSimilar;
}

unordered_set<string> ParseSimilarCallsignSet(unordered_set<string> callsigns)
{
	// input: callsigns, output: callsigns that are similar
	unordered_set<string> res, tmp;
	for (auto itn : callsigns) {
		for (auto itt : tmp) {
			if (CompareCallsign(itn, itt)) {
				res.insert(itt);
				res.insert(itn);
				break;
			}
		}
		tmp.insert(itn);
	}
	return res;
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
