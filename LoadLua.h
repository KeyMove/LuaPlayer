#pragma once
#include "afxwin.h"
class LoadLua
{
public:
	LoadLua(void);
	~LoadLua(void);
	bool InitLua(void);
	bool doFile(CString file);
	void windowapi(void);
	void TimerCall(int index);
	void SetHWND(HWND hwnd);
	int CloseAllTimer(void);
	CString GetErrorLog(void);
	CString ErrorLog;
};

