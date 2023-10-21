// MTEPScreen.h

#pragma once
#include "pch.h"

class CMTEPScreen :
	public EuroScopePlugIn::CRadarScreen
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
