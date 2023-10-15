// MTEPScreen.h

#pragma once
#include "pch.h"

using namespace EuroScopePlugIn;

class CMTEPScreen :
	public CRadarScreen
{
public:
	bool m_Opened;

	CMTEPScreen(void) {
		m_Opened = true;
	};

	~CMTEPScreen(void) {

	};

	virtual void OnAsrContentToBeClosed(void) {
		m_Opened = false;
		// delete will be done when attempting to access
	};
};
