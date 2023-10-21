// SimilarCallsign.h

#pragma once

#include "pch.h"

const std::unordered_set<std::string> m_CHNCallsign = {
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

bool CompareCallsign(std::string callsign1, std::string callsign2);
