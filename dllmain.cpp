// dllmain.cpp
#include "pch.h"
#include <EuroScopePlugIn.h>
#include "MTEPlugIn.h"

// Interface for EuroScope plugin loading
CMTEPlugIn* pMyPlugIn = nullptr;

void __declspec (dllexport)
EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pMyPlugIn = new CMTEPlugIn;
}

void __declspec (dllexport)
EuroScopePlugInExit(void)
{
	delete pMyPlugIn;
}