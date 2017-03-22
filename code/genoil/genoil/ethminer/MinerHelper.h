#pragma once
#include <tchar.h>
#include <vector>
#include <thread>
#include <iostream>
#include <shlobj.h>
#include <Shlwapi.h>
#include <iostream>
#include <Tlhelp32.h>
#pragma comment(lib,"shlwapi.lib")
using namespace std;
//�����˳��� exitcode
//-1:��������
//1:û���ҵ����ʵ��Կ��豸���Դ治��
//2:�Զ�����Կ��豸��û���ҵ����ʵ��Կ��豸���ų��� interl�Կ���

//
//
////��Ϣ
////����DAG���
//#define WM_USER_DAG		WM_USER+701
////ͨ��wParamָ��DAG��Ϣ����
//#define DAG_START_CREATE 0
//#define DAG_CREATE_PROGRESS 1
//#define DAG_CREATE_SUCCESS 2
//#define DAG_CREATE_FAIL 3
//
//
////WM_COPYDATA ͨ��wParamָ����Ϣ����
////�ϱ��ٶ�
//#define WP_SPEED		    100
//
////�ϱ��ҵ���
//#define WP_SOLUTION		    200
//
//struct SOLUTION_SS
//{
//	char sz_nonce[MAX_PATH];
//	char sz_headerHash[MAX_PATH];
//	char sz_mixHash[MAX_PATH];
//
//};
//
//inline void PostMessageToUserWnd(UINT Msg, WPARAM wParam, LPARAM lParam)
//{
//	static HWND hUserWnd = NULL;
//	if (NULL == hUserWnd)
//	{
//		hUserWnd = FindWindowA("UserWnd_{FEE8E80D-0A47-44DD-AD58-9E7F6F08C4E8}", NULL);
//	}
//	if (hUserWnd && ::IsWindow(hUserWnd)) {
//		PostMessageA(hUserWnd, Msg, wParam, lParam);
//	}
//}
//
//inline void SendMessageToUserWnd(UINT Msg, WPARAM wParam, LPARAM lParam)
//{
//	static HWND hUserWnd = NULL;
//	if (NULL == hUserWnd)
//	{
//		hUserWnd = FindWindowA("UserWnd_{FEE8E80D-0A47-44DD-AD58-9E7F6F08C4E8}", NULL);
//	}
//	if (hUserWnd && ::IsWindow(hUserWnd)) {
//		SendMessageA(hUserWnd, Msg, wParam, lParam);
//	}
//}

inline HANDLE GetParentProcessHandle()
{
	HANDLE hParent = NULL;
	DWORD dwParentPID = 0;
	HANDLE hProcessSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (INVALID_HANDLE_VALUE != hProcessSnapshot)
	{
		DWORD dwCurPID = GetCurrentProcessId();
		PROCESSENTRY32 pe = { sizeof(pe) };
		int nProcessNumbers = 0;
		for (BOOL bEnumResult = ::Process32First(hProcessSnapshot, &pe);
			bEnumResult; bEnumResult = ::Process32Next(hProcessSnapshot, &pe))
		{
			if (pe.th32ProcessID == dwCurPID)
			{
				dwParentPID = pe.th32ParentProcessID;
				break;
			}
		}
		CloseHandle(hProcessSnapshot);
	}
	if (dwParentPID > 0)
	{
		hParent = OpenProcess(PROCESS_VM_READ | SYNCHRONIZE, FALSE, dwParentPID);
	}
	return hParent;
}
inline std::string GetUpperPath(const std::string strPath)
{
	size_t nPos = strPath.rfind("\\");
	if (nPos == std::string::npos)
	{
		return strPath;
	}
	if (nPos == (strPath.length() - 1))
	{
		return GetUpperPath(std::string(strPath.c_str(), strPath.length() - 1));
	}
	return strPath.substr(0, nPos + 1);
}

inline void DeleteUselessDagFile(std::string &strCurDag, std::string &strNextDag)
{
	std::string strDir = GetUpperPath(strCurDag);
	std::string strCurtDagName = strCurDag.substr(strCurDag.find_last_of('\\') + 1);
	std::string strNextDagName = strNextDag.substr(strNextDag.find_last_of('\\') + 1);
	std::string strFlag = "*.*";
	CHAR szSearchPath[MAX_PATH] = { 0 };
	::PathCombineA(szSearchPath, strDir.c_str(), strFlag.c_str());
	WIN32_FIND_DATAA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFileA(szSearchPath, &fd);
	while (INVALID_HANDLE_VALUE != hFind)
	{
		if (stricmp(fd.cFileName, (".."))
			&& stricmp(fd.cFileName, ("."))
			&& stricmp(fd.cFileName, strCurtDagName.c_str())
			&& stricmp(fd.cFileName, strNextDagName.c_str())
			&& FILE_ATTRIBUTE_DIRECTORY != fd.dwFileAttributes)
		{
			CHAR szLnkFileTmp[MAX_PATH] = { 0 };
			PathCombineA(szLnkFileTmp, strDir.c_str(), fd.cFileName);
			cout << "delete dag file = " << szLnkFileTmp << endl;
			DeleteFileA(szLnkFileTmp);

		}

		if (FindNextFileA(hFind, &fd) == 0)
		{
			break;
		}
	}
	FindClose(hFind);

	return;
}

inline bool IsDebug()
{
#if defined _DEBUG
	return true;
#else
	if (::PathFileExistsA("C:\\GXZB_CONFIG\\GXZB.ini"))
	{
		return true;
	}
	return false;
#endif
}

inline BOOL AllowMessageToRecv(UINT uiMsg, BOOL bAllowed)
{
	BOOL bSuc = FALSE;
	TCHAR szUser32DllFilePath[MAX_PATH] = { 0 };
	if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, 0, szUser32DllFilePath))
		&& ::PathAppend(szUser32DllFilePath, _T("user32.dll"))) {
		if (::PathFileExists(szUser32DllFilePath) && !::PathIsDirectory(szUser32DllFilePath)) {
			HMODULE hUser32Dll = ::LoadLibrary(szUser32DllFilePath);
			if (hUser32Dll) {
				typedef BOOL(__stdcall *PFN_ChangeWindowMessageFilter)(UINT, DWORD);
				PFN_ChangeWindowMessageFilter pfnChangeWindowMessageFilter = (PFN_ChangeWindowMessageFilter)::GetProcAddress(hUser32Dll, "ChangeWindowMessageFilter");
				if (pfnChangeWindowMessageFilter) {
					bSuc = (*pfnChangeWindowMessageFilter)(uiMsg, (bAllowed ? 1 : 2)); // 1 - MSGFLT_ADD; 2 - MSGFLT_REMOVE
				}
				::FreeLibrary(hUser32Dll);
			}
		}
	}
	return bSuc;
};