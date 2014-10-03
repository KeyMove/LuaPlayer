#include "stdafx.h"
#include "IO.h"


CIO::CIO(void)
{
}


CIO::~CIO(void)
{
}

typedef bool (_stdcall *InitializeWinIo)();
typedef void (_stdcall *ShutdownWinIo)();
typedef bool (_stdcall *GetPortVal)(WORD, PDWORD, BYTE);
typedef bool (_stdcall *SetPortVal)(WORD, DWORD , BYTE);

InitializeWinIo Init;
ShutdownWinIo DeInit;
GetPortVal GetValue;
SetPortVal SetValue;

bool CIO::LoadWinIO(void)
{
	CFile c;
	if(c.Open(_T("winio32.dll"),CFile::modeRead))
	{
		c.Close();
		RegWinIO();
		return true;
	}
	return false;
}


bool CIO::RegWinIO(void)
{
	HMODULE hModule=::LoadLibrary(_T("winio32.dll"));
	Init=(InitializeWinIo)GetProcAddress(hModule,"InitializeWinIo");
	DeInit=(ShutdownWinIo)GetProcAddress(hModule,"ShutdownWinIo");
	GetValue=(GetPortVal)GetProcAddress(hModule,"GetPortVal");
	SetValue=(SetPortVal)GetProcAddress(hModule,"SetPortVal");
	return true;
}
