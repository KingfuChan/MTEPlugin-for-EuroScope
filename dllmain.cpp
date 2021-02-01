// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <EuroScopePlugIn.h>
#include "MTEPlugIn.h"

//用于EuroScope加载插件
CMTEPlugIn* pMyPlugIn = nullptr;

void __declspec (dllexport)
EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	// allocate
	*ppPlugInInstance = pMyPlugIn = new CMTEPlugIn;
}

void __declspec (dllexport)
EuroScopePlugInExit(void)
{
	delete pMyPlugIn;
}
//以上为ES特有
