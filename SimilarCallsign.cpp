// SimilarCallsign.cpp


#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "SimilarCallsign.h"


const std::set<CString> m_CHNCallsign = {
	"ALP","AYE","BDJ","BGC","BJN","BNH","BSH","BYC","CAG","CAO",
	"CBG","CBJ","CCA","CCD","CCO","CDC","CDG","CES","CFA","CFI",
	"CFJ","CFZ","CGH","CGN","CGZ","CHB","CHC","CHH","CJX","CKK",
	"CMJ","CNJ","CNM","CNW","CQH","CQN","CSC","CSH","CSN","CSS",
	"CSY","CSZ","CTE","CTH","CUA","CUH","CWR","CWU","CXA","CXH",
	"CXI","CXJ","CXN","CYH","CYN","CYZ","DBH","DER","DGA","DKH",
	"DLC","DXH","EPA","EPB","FSJ","FTU","FUH","FZA","GCR","GDC",
	"GSC","GWL","HAH","HBH","HFJ","HHG","HLF","HTK","HTU","HXA",
	"HYT","HZX","ICU","JAE","JBE","JGJ","JOY","JYH","KNA","KPA",
	"KXA","LHA","LKE","MSF","MZT","NEJ","NMG","NSJ","OKA","OTC",
	"OTT","QDA","QSR","RBW","RLH","SHQ","SNG","TBA","TXJ","UEA",
	"UNA","UTP","WFH","WUA","XAI","YZR","CSG"
};


bool IsCallsignChinese(EuroScopePlugIn::CFlightPlan FlightPlan) {
	CString scrpd = FlightPlan.GetControllerAssignedData().GetScratchPadString();
	CString csicao = CString(FlightPlan.GetCallsign()).Left(3);
	return m_CHNCallsign.count(csicao) && scrpd.Find("*EN") == -1;
}


CharList ExtractNumfromCallsign(const CString callsign)
{
	// extract num from callsign
	// no less than given digits, positive for right justify, negative for left
	CharList csnum;
	bool numbegin = false;
	for (int i = 0; i < callsign.GetLength(); i++) {
		numbegin = numbegin || (callsign[i] >= '1' && callsign[i] <= '9');
		if (numbegin)
			csnum.push_back(callsign[i]);
	}
	return csnum;
}

bool CompareCallsignNum(CharList cs1, CharList cs2)
{
	// compares tow callsign, CharList in same size
	int size;
	if ((size = cs1.size()) != cs2.size()) return false;
	CharList::iterator p1, p2;
	int same = 0; // same number on same position count
	CharList dn1, dn2; // different number on same position list
	for (p1 = cs1.begin(), p2 = cs2.begin(); p1 != cs1.end() && p2 != cs2.end(); p1++, p2++) {
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

CSMark ParseSimilarCallsign(CSMark MarkerMap)
{
	// parse m_similarMarker and set if a callsign is similar to others
	for (CSMark::iterator it1 = MarkerMap.begin(); it1 != MarkerMap.end(); it1++) {
		for (CSMark::iterator it2 = it1; it2 != MarkerMap.end(); it2++) {
			if (it1 == it2) continue;
			CharList cs1 = ExtractNumfromCallsign(it1->first);
			CharList cs2 = ExtractNumfromCallsign(it2->first);
			bool isSimilar = false;

			// compares
			if (!cs1.size() || !cs2.size()) // one of it doesn't have a number
				continue;
			else if (cs1.size() <= 1 && cs2.size() <= 1) { // prevents (1,1) bug in CompareCallsignNum()
				isSimilar = cs1 == cs2;
			}
			else if (cs1.size() == cs2.size()) {
				isSimilar = CompareCallsignNum(cs1, cs2);
			}
			else {
				// make cs1 the longer callsign for justification
				CharList cst = cs1.size() < cs2.size() ? cs1 : cs2;
				cs1 = cs1.size() > cs2.size() ? cs1 : cs2;
				cs2 = cst;
				CharList csl, csr;
				int i = 0;
				for (csl = cs2, csr = cs2; i < cs1.size() - cs2.size(); i++) {
					csl.push_back(' ');
					csr.push_front(' ');
				}
				isSimilar = CompareCallsignNum(cs1, csl) || CompareCallsignNum(cs1, csr);
			}

			if (isSimilar)
				it1->second = it2->second = true;
		}
	}

	return MarkerMap;
}
