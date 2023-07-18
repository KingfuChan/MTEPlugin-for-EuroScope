// SimilarCallsign.h

#pragma once

#include "pch.h"
#include <string>
#include <unordered_set>
#include <list>

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

bool CompareCallsign(string callsign1, string callsign2);
