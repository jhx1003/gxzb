// taskms.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "CheckEnvironment.h"
#include "Util.h"
#define  BUILDNUM  L"2"
#define  CHANNEL  L"0003"

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
		}
	}
	return bIsWow64;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	if (!IsWow64())
	{
		TSDEBUG4CXX(L"is not 64 bit system"); 
		return 1;
	}
	std::wstring wstrCmdLine = lpstrCmdLine;
	if (wstrCmdLine.find(L"/killall") != std::wstring::npos)
	{
		RunEnvironment::TerminateAllClientInstance();
		TSDEBUG4CXX(L"just kill client"); 
		return 0;
	}
	RunEnvironment run;
	if (!run.CheckEnvironment())
	{
		TSDEBUG4CXX(L"check environment return false"); 
		return 2;
	}

	std::wstring strCmdline = L"";
	run.GetRunCmd(strCmdline);
	if (strCmdline.empty())
	{
		TSDEBUG4CXX(L"get client cmdline error"); 
		return 3;
	}
	TSDEBUG4CXX(L"strCmdLine = "<< strCmdline.c_str()); 


	RunEnvironment::TerminateAllClientInstance();


	//SECURITY_ATTRIBUTES sa = {0};  
	//sa.nLength = sizeof(sa);  
	//sa.bInheritHandle = TRUE;  
	//HANDLE hStdInRead,hStdInWrite;
	////����һ������stdin�Ĺܵ����õ�����HANDLE:  hStdInRead�����ӽ��̶����ݣ�hStdInWrite����������д������     
	//if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0))  
	//	return 5;  
	//
	//HANDLE hStdOutRead,hStdOutWrite;
	////����һ������stdout�Ĺܵ����õ�����HANDLE:  m_hStdOutRead����������������ݣ�m_hStdOutWrite�����ӳ���д������    
	//if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0))  
	//	return 5;  
	//

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));  
	si.cb = sizeof(STARTUPINFO);  
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;  //��������showwindow�������½��̵���������������  
	si.hStdOutput = NULL;     //��˼�ǣ��ӽ��̵�stdout�����m_hStdOutWrite    
	si.hStdError = NULL;      //��˼�ǣ��ӽ��̵�stderr�����m_hStdOutWrite    
	si.hStdInput = NULL;  
	si.wShowWindow = SW_HIDE;
	::SetErrorMode(::SetErrorMode(0)|SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX|SEM_NOGPFAULTERRORBOX|SEM_NOALIGNMENTFAULTEXCEPT);
	if(!CreateProcess( NULL,(LPTSTR)strCmdline.c_str(), NULL, NULL, TRUE, NULL, NULL, NULL,&si,	&pi ))
	{
		TSDEBUG4CXX(L"launch client  failed, last error = "<< GetLastError()); 
		return 4;
	}
	SetPriorityClass(pi.hProcess,BELOW_NORMAL_PRIORITY_CLASS);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	TSDEBUG4CXX(L"launch client success");
	HANDLE hThread = SendAnyHttpStat(L"startup", CHANNEL, BUILDNUM, 1);
	if (hThread)
	{
		DWORD dwRet = WaitForSingleObject(hThread, 15*1000);
		if (dwRet == WAIT_TIMEOUT){
			TSDEBUG4CXX(L"wait stat report failed, last error = "<< GetLastError()); 
		}
		
	}
	return 0;
}

