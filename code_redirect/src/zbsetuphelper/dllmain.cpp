// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <string>
#include <comdef.h>
#include <atlbase.h>
#include <WTL/atlapp.h>
#include <Urlmon.h>
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "Version.lib")
#include "shlobj.h"
#include <shellapi.h>
#include <tlhelp32.h>
#include <atlstr.h>
#include "..\zbkernel\Utility\PeeIdHelper.h"
#include "base64.h"
#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#include <sstream>

#include "commonshare/md5.h"
#include "shortcut/Shortcut.h"
#include "StringOperation.h"
#include "OpenCL.h"
#include "base64standard.h"

using namespace std;

extern "C" typedef HRESULT (__stdcall *PSHGetKnownFolderPath)(  const  GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR* pszPath);

void EncryptString(const char* pszData,std::string &strOut);
void DecryptString(const char* pszBase64Data,std::string &strOut);

void SendStatProxy(const char* fu1,const char* version_channel, const char* fu5,  const char* fu6);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

int CheckRegDisplayCardInfo()
{
	TSAUTO();
	const char* szRegPath = "Software\\DisplayCardInfo"; 
	int iRet = 0;
	do 
	{
		HKEY hKey;
		if (ERROR_SUCCESS != ::RegOpenKeyExA(HKEY_CURRENT_USER,szRegPath,0,KEY_READ,&hKey))
		{
			iRet = 1;
			break;
		}
		DWORD dwValue = 0;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		if (::RegQueryValueExA(hKey,"SupportOpenCL", 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS || dwValue != 1)
		{
			iRet =  2;
			break;
		}
		if (::RegQueryValueExA(hKey,"DiscreteCard", 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS || dwValue != 1)
		{
			iRet = 3;
			break;
		}
		if (::RegQueryValueExA(hKey,"Memory", 0, &dwType, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS || dwValue < 3072)
		{
			iRet = 4;
			break;
		}
	} while (FALSE);
	TSDEBUG4CXX("CheckRegDisplayCardInfo  iRet = "<<iRet);
	return iRet;
	
}



extern "C" __declspec(dllexport) BOOL CheckCLEnvir(const char* szExePath, const char* szfu1,const char* version_channel)
{
	TSAUTO();
	
	TSDEBUG4CXX(L"CheckCLEnvir szExePath = "<<ultra::_A2UTF(szExePath));
	std::string strfu1 = szfu1;
	std::string strversion_channel = version_channel;
	std::string strfu5 = "genoilenvinfo";

	int dwRegCond = CheckRegDisplayCardInfo();
	std::string strRegCond;
	{
		std::stringstream ss;
		ss << dwRegCond;
		ss >> strRegCond;
	}

	STARTUPINFOA si = { 0 };
	si.cb = sizeof(STARTUPINFOA);
	GetStartupInfoA(&si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	PROCESS_INFORMATION   pi = {0};
	if (!CreateProcessA(NULL, (LPSTR)szExePath, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)){
		DWORD dwLastError = ::GetLastError();
		TSDEBUG4CXX(L"CheckCLEnvir create process error, dwLastError = "<<dwLastError);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		
	
		std::string strfu6 = "1_";
		strfu6.append(strRegCond);
		SendStatProxy(strfu1.c_str(),strversion_channel.c_str(),strfu5.c_str(),strfu6.c_str());
		if (dwRegCond == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	DWORD dwResult = WaitForSingleObject(pi.hProcess, 30*1000);
	DWORD dwExitCode = 0xFFFFFFFF;
	if (!GetExitCodeProcess(pi.hProcess, &dwExitCode)){
		DWORD dwLastError = ::GetLastError();
		std::string strLastError;
		{
			std::stringstream ss;
			ss << dwLastError;
			ss >> strLastError;
		}

		std::string strfu6 = "2_";
		strfu6.append(strRegCond);
		strfu6.append("_");
		strfu6.append(strLastError.c_str());

		SendStatProxy(strfu1.c_str(),strversion_channel.c_str(),strfu5.c_str(),strfu6.c_str());

		if (dwRegCond == 0)
		{

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	TSDEBUG4CXX(L"CheckCLEnvir dwExitCode = "<<dwExitCode);
	std::string strExitCode;
	{
		std::stringstream ss;
		ss << dwExitCode;
		ss >> strExitCode;
	}
	
	if (dwExitCode == 0 || dwExitCode == STILL_ACTIVE)
	{
		std::string strfu6 = "0_";
		strfu6.append(strRegCond);
		strfu6.append("_");
		strfu6.append(strExitCode.c_str());

		SendStatProxy(strfu1.c_str(),strversion_channel.c_str(),strfu5.c_str(),strfu6.c_str());

		return TRUE;
	}
	else
	{
		std::string strfu6 = "3_";
		strfu6.append(strRegCond);
		strfu6.append("_");
		strfu6.append(strExitCode.c_str());

		SendStatProxy(strfu1.c_str(),strversion_channel.c_str(),strfu5.c_str(),strfu6.c_str());
		if (dwRegCond == 0)
		{

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

//�����˳���֤�������߳̽���
static HANDLE s_ListenHandle = CreateEvent(NULL,TRUE,TRUE,NULL);
//���ü���,Ĭ����0
static int s_ListenCount = 0;
static CRITICAL_SECTION s_csListen;
static bool b_Init = false;

void ResetUserHandle()
{
	if(!b_Init){
		InitializeCriticalSection(&s_csListen);
		b_Init = true;
	}
	EnterCriticalSection(&s_csListen);
	if(s_ListenCount == 0)
	{
		ResetEvent(s_ListenHandle);
	}
	++s_ListenCount; //���ü�����1
	TSDEBUG4CXX("ResetUserHandle  s_ListenCount = "<<s_ListenCount);
	LeaveCriticalSection(&s_csListen);
}

void SetUserHandle()
{
	EnterCriticalSection(&s_csListen);
	--s_ListenCount;//���ü�����1
	TSDEBUG4CXX("SetUserHandle  s_ListenCount = "<<s_ListenCount);
	if (s_ListenCount == 0)
	{
		SetEvent(s_ListenHandle);
	}
	LeaveCriticalSection(&s_csListen);
}

DWORD WINAPI SendHttpStatThread(LPVOID pParameter)
{
	//TSAUTO();
	CHAR szUrl[MAX_PATH] = {0};
	strcpy(szUrl,(LPCSTR)pParameter);
	delete [] pParameter;

	CHAR szBuffer[MAX_PATH] = {0};
	::CoInitialize(NULL);
	HRESULT hr = E_FAIL;
	__try
	{
		hr = ::URLDownloadToCacheFileA(NULL, szUrl, szBuffer, MAX_PATH, 0, NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		TSDEBUG4CXX("URLDownloadToCacheFile Exception !!!");
	}
	::CoUninitialize();
	SetUserHandle();
	return SUCCEEDED(hr)?ERROR_SUCCESS:0xFF;
}

 BOOL WStringToString(const std::wstring &wstr,std::string &str)
 {    
     int nLen = (int)wstr.length();    
     str.resize(nLen,' ');
 
     int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)wstr.c_str(),nLen,(LPSTR)str.c_str(),nLen,NULL,NULL);
 
     if (nResult == 0)
     {
         return FALSE;
     }
 
     return TRUE;
 }

extern "C" __declspec(dllexport) void WaitForStat()
{
	DWORD ret = WaitForSingleObject(s_ListenHandle, 20000);
	if (ret == WAIT_TIMEOUT){
	}
	else if (ret == WAIT_OBJECT_0){	
	}
	if(b_Init){
		DeleteCriticalSection(&s_csListen);
	}
}

extern "C" __declspec(dllexport) void SetUpExit()
{
	TerminateProcess(GetCurrentProcess(), 0);
}



extern "C" __declspec(dllexport) void SendAnyHttpStat(CHAR *fu1,CHAR *version_channel, CHAR *fu5,CHAR *fu6)
{
	TSAUTO();
	char szPid[256] = {0};
	extern void GetPeerID(CHAR * pszPeerID);
	GetPeerID(szPid);
	std::string strfu2= szPid;
	std::string strfu1= "";
	if (NULL != fu1)
	{
		strfu1 = fu1;
	}
	std::string strfu3= "";
	std::string strfu4= "";
	if (NULL != version_channel)
	{
		std::string strTmp = version_channel;
		size_t iPos = strTmp.find("_", 0);
		if (iPos != std::string::npos)
		{
			strfu3 = strTmp.substr(0,iPos);
			strfu4 = strTmp.substr(iPos+1);
		}
		else
		{
			strfu3 = strTmp;
		}
	}
	std::string strfu5= "";
	if (NULL != fu5)
	{
		strfu5 = fu5;
	}

	std::string strfu6= "";
	if (NULL != fu6)
	{
		strfu6 = fu6;
	}
	CHAR szParam[MAX_PATH] = {0};
	sprintf(szParam, "/install?fu1=%s&fu2=%s&fu3=%s&fu4=%s&fu5=%s&fu6=%s&fu7=&fu8=&fu9=&fu10=",
		strfu1.c_str(),strfu2.c_str(), strfu3.c_str(),strfu4.c_str(),strfu5.c_str(),strfu6.c_str());
	
	TSDEBUG4CXX(L"SendAnyHttpStat szParam = "<<ultra::_A2UTF(szParam));
	
	wchar_t pszMD5[MAX_PATH] = {0};
	GetStringMd5(szParam,pszMD5);
	std::string strmd5 = ultra::_T2A(pszMD5);

	TSDEBUG4CXX(L"SendAnyHttpStat pszMD5 = "<<pszMD5);

	CHAR* szURL = new CHAR[MAX_PATH];
	memset(szURL, 0, MAX_PATH);
	sprintf(szURL, "http://pgv.eastredm.com/pc%s&md5=%s",szParam,strmd5.c_str());

	TSDEBUG4CXX(L"SendAnyHttpStat szURL = "<<ultra::_A2UTF(szURL));
	ResetUserHandle();
	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, SendHttpStatThread, (LPVOID)szURL,0, &dwThreadId);
	CloseHandle(hThread);
	//SendHttpStatThread((LPVOID)szURL);
}

extern "C" __declspec(dllexport) void GetFileVersionString(CHAR* pszFileName, CHAR * pszVersionString)
{
	if(pszFileName == NULL || pszVersionString == NULL)
		return ;

	BOOL bResult = FALSE;
	DWORD dwHandle = 0;
	DWORD dwSize = ::GetFileVersionInfoSizeA(pszFileName, &dwHandle);
	if(dwSize > 0)
	{
		CHAR * pVersionInfo = new CHAR[dwSize+1];
		if(::GetFileVersionInfoA(pszFileName, dwHandle, dwSize, pVersionInfo))
		{
			VS_FIXEDFILEINFO * pvi;
			UINT uLength = 0;
			if(::VerQueryValueA(pVersionInfo, "\\", (void **)&pvi, &uLength))
			{
				sprintf(pszVersionString, "%d.%d.%d.%d",
					HIWORD(pvi->dwFileVersionMS), LOWORD(pvi->dwFileVersionMS),
					HIWORD(pvi->dwFileVersionLS), LOWORD(pvi->dwFileVersionLS));
				bResult = TRUE;
			}
		}
		delete pVersionInfo;
	}
}

void SetRegPeerID(const char* szPeerID)
{
	std::string strEncrypt ="";
	EncryptString(szPeerID,strEncrypt);
	HKEY hKey, hTempKey;
	if (ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software",0,KEY_SET_VALUE, &hKey))
	{
		if (ERROR_SUCCESS == ::RegCreateKeyA(hKey, "Share4Money", &hTempKey))
		{
			::RegSetValueExA(hTempKey, "PeerId", 0, REG_SZ, (LPBYTE)strEncrypt.c_str(), strEncrypt.size()+1);
			RegCloseKey(hTempKey);
		}
		RegCloseKey(hKey);
	}
}
extern "C" __declspec(dllexport) void GetPeerID(CHAR * pszPeerID)
{
	TSAUTO();
	HKEY hKEY;
	LPCSTR data_Set= "Software\\Share4Money";
	if (ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_LOCAL_MACHINE,data_Set,0,KEY_READ,&hKEY))
	{
		char szValue[260] = {0};
		DWORD dwSize = sizeof(szValue);
		DWORD dwType = REG_SZ;
		if (::RegQueryValueExA(hKEY,"PeerId", 0, &dwType, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
		{
			DWORD dwLen = strlen(szValue);
			TSDEBUG4CXX(L"GetPeerID dwLen = "<<dwLen);
			
			if (dwLen == 16)
			{
				strcpy(pszPeerID, szValue);
				SetRegPeerID(szValue);
				return;
			}
			else
			{
				std::string strDecrypt ="";
				DecryptString(szValue,strDecrypt);
				if (!strDecrypt.empty() && strDecrypt.size() == 16)
				{
					strcpy(pszPeerID, strDecrypt.c_str());
					::RegCloseKey(hKEY);
					return;
				}
			}
		}
		::RegCloseKey(hKEY);
	}
	std::wstring wstrPeerID;
	GetPeerId_(wstrPeerID);
	std::string strPeerID;
	WStringToString(wstrPeerID, strPeerID);
	strcpy(pszPeerID,strPeerID.c_str());
	SetRegPeerID(strPeerID.c_str());

}

extern "C" __declspec(dllexport) void NsisTSLOG(char* pszInfo)
{
	if(pszInfo == NULL)
		return;
	TSDEBUG4CXX("<NSIS> " << pszInfo);
}

extern "C" __declspec(dllexport) void GetTime(LPDWORD pnTime)
{
	TSAUTO();
	if(pnTime == NULL)
		return;
	time_t t;
	time( &t );
	*pnTime = (DWORD)t;
}

#ifndef DEFINE_KNOWN_FOLDER
#define DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif

#ifndef FOLDERID_Public
DEFINE_KNOWN_FOLDER(FOLDERID_Public, 0xDFDF76A2, 0xC82A, 0x4D63, 0x90, 0x6A, 0x56, 0x44, 0xAC, 0x45, 0x73, 0x85);
#endif

EXTERN_C const GUID DECLSPEC_SELECTANY FOLDERID_UserPin \
	= { 0x9E3995AB, 0x1F9C, 0x4F13, { 0xB8, 0x27,  0x48,  0xB2,  0x4B,  0x6C,  0x71,  0x74 } };


extern "C" __declspec(dllexport) bool GetProfileFolder(char* szMainDir)	// ʧ�ܷ���'\0'
{
	char szAllUserDir[MAX_PATH] = {0};
	if(('\0') == szAllUserDir[0])
	{
		HMODULE hModule = ::LoadLibraryA("shell32.dll");
		PSHGetKnownFolderPath SHGetKnownFolderPath = (PSHGetKnownFolderPath)GetProcAddress( hModule, "SHGetKnownFolderPath" );
		if ( SHGetKnownFolderPath)
		{
			PWSTR szPath = NULL;
			HRESULT hr = SHGetKnownFolderPath( FOLDERID_Public, 0, NULL, &szPath );
			if ( FAILED( hr ) ||  szPath == NULL)
			{
				TSERROR4CXX("Failed to get public folder");
				FreeLibrary(hModule);
				return false;
			}
			if(0 == WideCharToMultiByte(CP_ACP, 0, szPath, -1, szAllUserDir, MAX_PATH, NULL, NULL))
			{
				TSERROR4CXX("WideCharToMultiByte failed");
				return false;
			}
			::CoTaskMemFree( szPath );
			FreeLibrary(hModule);
		}
		else
		{
			HRESULT hr = SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, szAllUserDir);
			if ( FAILED( hr ) )
			{
				TSERROR4CXX("Failed to get main pusher dir");
				return false;
			}
		}
	}
	strcpy(szMainDir, szAllUserDir);
	TSERROR4CXX("GetProfileFolder, szMainDir = "<<szMainDir);
	return true;
}

DWORD WINAPI ThreadDownLoadUrl(LPVOID pParameter)
{
	//TSAUTO();
	const char *pUrl = (const char*)pParameter, *pName = pUrl + strlen(pUrl)+1, *pCmd = pName + strlen(pName)+1;
	//MessageBoxA(0, pUrl, "```pUrl", 0);MessageBoxA(0, pName, "````pName", 0);MessageBoxA(0, pCmd, "```pCmd", 0);
	::CoInitialize(NULL);
	HRESULT hr = E_FAIL;
	__try
	{
		hr = ::URLDownloadToFileA(NULL, pUrl, pName, 0, NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		TSDEBUG4CXX("URLDownloadToCacheFile Exception !!!");
	}
	::CoUninitialize();
	if (SUCCEEDED(hr) && ::PathFileExistsA(pName))
	{
		::ShellExecuteA(NULL,"open", pName, pCmd, NULL, SW_HIDE);
	}
	return SUCCEEDED(hr)?ERROR_SUCCESS:0xFF;
}

extern "C" __declspec(dllexport) void DownLoadUrlAndInstall(const char* szUrl, const char* szName, const char* szCmd, int Millisec = INFINITE)
{
	TSAUTO();
	char szBuff[2048] = {0};
	char *pUrl = szBuff, *pName = szBuff + strlen(szUrl)+1, *pCmd = pName + strlen(szName)+1;
	strcpy(pUrl, szUrl);
	strcpy(pName, szName);
	strcpy(pCmd, szCmd);
	//MessageBoxA(0, pUrl, "pUrl", 0);MessageBoxA(0, pName, "pName", 0);MessageBoxA(0, pCmd, "pCmd", 0);

	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, ThreadDownLoadUrl, (LPVOID)szBuff,0, &dwThreadId);
	if (NULL != hThread)
	{
		DWORD dwRet = WaitForSingleObject(hThread, Millisec);
		if (dwRet == WAIT_FAILED)
		{
			TSDEBUG4CXX("wait for DownLoadd Software failed, error = " << ::GetLastError());
		}
		CloseHandle(hThread);
	}
	return;
}

#include <vector>
#include <COMUTIL.H>
typedef std::vector<std::wstring> VectorVerbName;
VectorVerbName*  GetVerbNames(bool bPin)
{
	TSAUTO();
	static bool bInit = false;
	static std::vector<std::wstring> vecPinStartMenuNames;
	static std::vector<std::wstring> vecUnPinStartMenuNames;
	if (!bInit )
	{	
		bInit = true;
		vecPinStartMenuNames.push_back(_T("��������ʼ�˵�"));vecPinStartMenuNames.push_back(_T("��������ʼ���˵�"));
		vecUnPinStartMenuNames.push_back(_T("�ӡ���ʼ���˵�����"));vecUnPinStartMenuNames.push_back(_T("(�ӡ���ʼ���˵�����"));
	}

	return bPin? &vecPinStartMenuNames : &vecUnPinStartMenuNames;
}

bool VerbNameMatch(TCHAR* tszName, bool bPin)
{
	TSAUTO();
	VectorVerbName *pVec = GetVerbNames(bPin);
	
	VectorVerbName::iterator iter = pVec->begin();
	VectorVerbName::iterator iter_end = pVec->end();
	while(iter!=iter_end)
	{
		std::wstring strName= *iter;
		if ( 0 == _wcsnicmp(tszName,strName.c_str(),strName.length()))
			return true;
		iter ++;
	}
	return false;
}

wchar_t* AnsiToUnicode( const char* szStr )
{
	int nLen = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0 );
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen );
	return pResult;
}


#define IF_FAILED_OR_NULL_BREAK(rv,ptr) \
{if (FAILED(rv) || ptr == NULL) break;}

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
extern "C" __declspec(dllexport) bool PinToStartMenu4XP(bool bPin, char* szPath)
{
	TSAUTO();

	TCHAR file_dir[MAX_PATH + 1] = {0};
	TCHAR *file_name;
	wchar_t* pwstr_Path = AnsiToUnicode(szPath);
	if(pwstr_Path == NULL){
		return false;
	}

	wcscpy_s(file_dir,MAX_PATH,pwstr_Path);
	PathRemoveFileSpecW(file_dir);
	file_name = PathFindFileName(pwstr_Path);
	::CoInitialize(NULL);
	CComPtr<IShellDispatch> pShellDisp;
	CComPtr<Folder> folder_ptr;
	CComPtr<FolderItem> folder_item_ptr;
	CComPtr<FolderItemVerbs> folder_item_verbs_ptr;


	HRESULT rv = CoCreateInstance( CLSID_Shell, NULL, CLSCTX_SERVER,IID_IDispatch, (LPVOID *) &pShellDisp );
	do 
	{
		IF_FAILED_OR_NULL_BREAK(rv,pShellDisp);
		rv = pShellDisp->NameSpace(_variant_t(file_dir),&folder_ptr);
		IF_FAILED_OR_NULL_BREAK(rv,folder_ptr);
		rv = folder_ptr->ParseName(CComBSTR(file_name),&folder_item_ptr);
		IF_FAILED_OR_NULL_BREAK(rv,folder_item_ptr);
		rv = folder_item_ptr->Verbs(&folder_item_verbs_ptr);
		IF_FAILED_OR_NULL_BREAK(rv,folder_item_verbs_ptr);
		long count = 0;
		folder_item_verbs_ptr->get_Count(&count);
		for (long i = 0; i < count ; ++i)
		{
			FolderItemVerb* item_verb = NULL;
			rv = folder_item_verbs_ptr->Item(_variant_t(i),&item_verb);
			if (SUCCEEDED(rv) && item_verb)
			{
				CComBSTR bstrName;
				item_verb->get_Name(&bstrName);

				if ( VerbNameMatch(bstrName,bPin) )
				{
					TSDEBUG4CXX("Find Verb to Pin:"<< bstrName);
					int i = 0;
					do
					{
						rv = item_verb->DoIt();
						TSDEBUG4CXX("Try Do Verb. NO." << i+1 << ", return="<<rv);
						if (SUCCEEDED(rv))
						{
							::SHChangeNotify(SHCNE_UPDATEDIR|SHCNE_INTERRUPT|SHCNE_ASSOCCHANGED, SHCNF_IDLIST |SHCNF_FLUSH | SHCNF_PATH|SHCNE_ASSOCCHANGED,
								pwstr_Path,0);
							Sleep(500);
							delete [] pwstr_Path;
							::CoUninitialize();
							return true;
						}else
						{
							Sleep(500);
							rv = item_verb->DoIt();
						}
					}while ( i++ < 3);
						
					break;
				}
			}
		}
	} while (0);
	delete [] pwstr_Path;
	::CoUninitialize();
	return false;
}


extern "C" __declspec(dllexport) void RefleshIcon(char* szPath)
{
	wchar_t* pwstr_Path = AnsiToUnicode(szPath);
	::SHChangeNotify(SHCNE_UPDATEDIR|SHCNE_INTERRUPT|SHCNE_ASSOCCHANGED, SHCNF_IDLIST |SHCNF_FLUSH | SHCNF_PATH|SHCNE_ASSOCCHANGED,
								pwstr_Path,0);
	delete [] pwstr_Path;

}

extern "C" __declspec(dllexport) void GetUserPinPath(char* szPath)
{
	static std::wstring strUserPinPath(_T(""));

	if (strUserPinPath.length() <= 0)
	{
		HMODULE hModule = LoadLibrary( _T("shell32.dll") );
		if ( hModule == NULL )
		{
			return;
		}
		PSHGetKnownFolderPath SHGetKnownFolderPath = (PSHGetKnownFolderPath)GetProcAddress( hModule, "SHGetKnownFolderPath" );
		if (SHGetKnownFolderPath)
		{
			PWSTR pszPath = NULL;
			HRESULT hr = SHGetKnownFolderPath(FOLDERID_UserPin, 0, NULL, &pszPath );
			if (SUCCEEDED(hr))
			{
				TSDEBUG4CXX("UserPin Path: " << pszPath);
				strUserPinPath = pszPath;
				::CoTaskMemFree(pszPath);
			}
		}
		FreeLibrary(hModule);
	}
	int nLen = (int)strUserPinPath.length();    
    int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)strUserPinPath.c_str(),nLen,szPath,nLen,NULL,NULL);
}

bool IsVistaOrLatter()
{
	OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX) };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx( (LPOSVERSIONINFO)&osvi ))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx( (LPOSVERSIONINFO)&osvi ))
		{
		}
	}
	return (osvi.dwMajorVersion >= 6);
}


BOOL GetNtVersionNumbers(OSVERSIONINFOEX& osVersionInfoEx)
{
	BOOL bRet= FALSE;
	HMODULE hModNtdll= ::LoadLibraryW(L"ntdll.dll");
	if (hModNtdll)
	{
		typedef void (WINAPI *pfRTLGETNTVERSIONNUMBERS)(DWORD*,DWORD*, DWORD*);
		pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;
		pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
		if (pfRtlGetNtVersionNumbers)
		{
			pfRtlGetNtVersionNumbers(&osVersionInfoEx.dwMajorVersion, &osVersionInfoEx.dwMinorVersion,&osVersionInfoEx.dwBuildNumber);
			osVersionInfoEx.dwBuildNumber&= 0x0ffff;
			bRet = TRUE;
		}

		::FreeLibrary(hModNtdll);
		hModNtdll = NULL;
	}

	return bRet;
}

extern "C" __declspec(dllexport) void NewGetOSVersionInfo(char* szOutput)
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi,sizeof(OSVERSIONINFOEX));
	if (!GetNtVersionNumbers(osvi))
	{
		return;
	}
	sprintf(szOutput, "%u.%u", osvi.dwMajorVersion, osvi.dwMinorVersion);
}

extern "C" __declspec(dllexport) void CreateGuid(char* szOutPut)
{
	if (szOutPut == NULL){
		return;
	}
	GUID guid;
	if (CoCreateGuid(&guid) == S_OK){
		_snprintf(szOutPut, MAX_PATH, "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], 
		guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	}
}

void SetMachineID(const char* szMachineID)
{
	TSAUTO();
	std::string strEncrypt ="";
	EncryptString(szMachineID,strEncrypt);
	HKEY hKey, hTempKey;
	if (ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_CURRENT_USER, "Software",0,KEY_SET_VALUE, &hKey))
	{
		if (ERROR_SUCCESS == ::RegCreateKeyA(hKey, "Share4Money", &hTempKey))
		{
			::RegSetValueExA(hTempKey, "machineid", 0, REG_SZ, (LPBYTE)strEncrypt.c_str(), strEncrypt.size()+1);
			RegCloseKey(hTempKey);
		}
		RegCloseKey(hKey);
	}
}

extern "C" __declspec(dllexport) void WriteMachineID()
{
	TSAUTO();
	HKEY hKEY;
	char * szPath = "Software\\Share4Money";
	if (ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_CURRENT_USER,szPath,0,KEY_READ,&hKEY))
	{
		char szValue[260] = {0};
		DWORD dwSize = sizeof(szValue);
		DWORD dwType = REG_SZ;
		if (::RegQueryValueExA(hKEY,"machineid", 0, &dwType, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
		{
			DWORD dwLen = strlen(szValue);
			TSDEBUG4CXX(L"WriteMachineID dwLen = "<<dwLen);

			if (dwLen == 32)
			{
				SetMachineID(szValue);
				return;
			}
			else
			{
				std::string strDecrypt ="";
				DecryptString(szValue,strDecrypt);
				if (!strDecrypt.empty() && strDecrypt.size() == 32)
				{
					::RegCloseKey(hKEY);
					return;
				}
			}
		}
	}
	char szGuid[MAX_PATH] = {0};
	TSDEBUG4CXX(L"Create guid init ");
	ZeroMemory(szGuid,MAX_PATH);
	strcpy(szGuid,"00000000000000000000000000000000");
	GUID guid;
	TSDEBUG4CXX(L"Create guid start ");
	if (CoCreateGuid(&guid) == S_OK)
	{
		_snprintf(szGuid, MAX_PATH, "%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], 
			guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	}
	TSDEBUG4CXX(L"Create guid end ");
	SetMachineID(szGuid);
}

extern "C" __declspec(dllexport) BOOL FindProcessByName(const char* szProName)
{
	BOOL bRet = FALSE;
	wchar_t* wszProName = AnsiToUnicode(szProName);
	TSDEBUG4CXX("Find process by name:"<< wszProName);
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		BOOL bResult = ::Process32First(hSnap, &pe);
		while (bResult)
		{
			if(_tcsicmp(pe.szExeFile, wszProName) == 0)
			{
				bRet = TRUE;
				break;
			}
			bResult = ::Process32Next(hSnap, &pe);
		}
		::CloseHandle(hSnap);
	}
	return bRet;
}

extern "C" __declspec(dllexport) void TerminateProcessByName(const char* szProName)
{
	wchar_t* wszProName = AnsiToUnicode(szProName);
	TSDEBUG4CXX("Terminate process by name:"<< wszProName);
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		BOOL bResult = ::Process32First(hSnap, &pe);
		while (bResult)
		{
			if(_tcsicmp(pe.szExeFile, wszProName) == 0 && pe.th32ProcessID != 0)
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
				TSDEBUG4CXX("Terminate process hProcess = "<< hProcess);
				BOOL bRet = ::TerminateProcess(hProcess, -4);
				TSDEBUG4CXX("Terminate process bRet = "<< bRet);
				break;
			}
			bResult = ::Process32Next(hSnap, &pe);
		}
		::CloseHandle(hSnap);
	}
}

void EncryptAESToFileHelper(const unsigned char* pszKey, const char* pszMsg, unsigned char* out_str, int& nlen)
{
	EVP_CIPHER_CTX ctx;
	// init
	EVP_CIPHER_CTX_init(&ctx);
	EVP_CIPHER_CTX_set_padding(&ctx, 1);

	EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)pszKey, NULL);

	//���EVP_EncryptUpdate��ʵ��ʵ�ʾ��ǽ�in����inl�ĳ���ȥ���ܣ�ʵ�ֻ�ȡ�ø�cipher�Ŀ��С����aes_128��˵��16�ֽڣ�����block-size��������ȥ���ܡ�
	//�������Ϊ50�ֽڣ���˴�������48�ֽڣ�outlҲΪ48�ֽڡ�����in�е�������ֽڿ�����ctx->buf����������  
	//����inlΪblock_size�����������Σ���ctx->buf��û����ǰ����������ʱ��ֱ�Ӽӽ��ܲ�����ʡȥ�ܶ����������  
	int msglen = strlen(pszMsg);
	EVP_EncryptUpdate(&ctx, out_str, &nlen, (const unsigned char*)pszMsg, msglen);
	//�������n�ֽڡ��˴����д���
	//�����֧��pading���һ������ݵĻ��ͳ������򣬽�block_size-�������ֽ����������ֽ�����Ϊ�˸�����ֵ����block_size=16,���ݳ���Ϊ4���򽫺����12�ֽ�����Ϊ16-4=12������Ϊһ���������� 
	//����ǰ��Ϊ������ʱ������������Ϊ16�ֽڣ�����ٵ��ô�Finalʱ�������Ƕ�16��0���м��ܣ������Ĳ��ü��ɣ�Ҳ�����ò��ŵ�һ����Final��
	int outl = 0;
	EVP_EncryptFinal_ex(&ctx, out_str + nlen, &outl);  
	nlen += outl;
	EVP_CIPHER_CTX_cleanup(&ctx);
}

void EncryptString(const char* pszData,std::string &strOut)
{
	char *pszKey = "RpXVQTFlU7NaeMcV";
	int ubuff = strlen(pszKey)>16?strlen(pszKey):16;
	char* pszNewKey = new(std::nothrow) char[ubuff+1];
	memset(pszNewKey,0,ubuff+1);

	strcpy_s(pszNewKey,ubuff+1,pszKey);

	int msglen = strlen(pszData);
	int flen = ((msglen >> 4) + 1) << 4;
	unsigned char* out_str = (unsigned char*)malloc(flen + 1);
	memset(out_str, 0, flen + 1);

	int nlen = 0;
	EncryptAESToFileHelper((const unsigned char*)pszNewKey, pszData, out_str, nlen);
	delete[] pszNewKey;
	if (nlen > 0)
	{
		std::string strBase64 = base64_encode(out_str,nlen);
		strOut = strBase64;
		free(out_str);
	}
	return;
}

void DecryptFileAESHelper(const unsigned char* pszKey, const unsigned char* pszMsg, int nlen, unsigned char* out_str)
{
	EVP_CIPHER_CTX ctx;
	// init
	EVP_CIPHER_CTX_init(&ctx);

	EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, pszKey, NULL); 

	int outl = 0;
	EVP_DecryptUpdate(&ctx, out_str, &outl, pszMsg, nlen);
	int len = outl;

	outl = 0;
	EVP_DecryptFinal_ex(&ctx, out_str + len, &outl);  
	len += outl;
	out_str[len]=0;

	EVP_CIPHER_CTX_cleanup(&ctx);
}


void DecryptString(const char* pszBase64Data,std::string &strOut)
{
	char *pszKey = "RpXVQTFlU7NaeMcV";
	std::string strData = base64_decode(pszBase64Data);
	if (strData.size() <= 0)
	{
		return;
	}

	int ubuff = strlen(pszKey)>16?strlen(pszKey):16;
	char* pszNewKey = new(std::nothrow) char[ubuff+1];
	memset(pszNewKey,0,ubuff+1);
	strcpy_s(pszNewKey,ubuff+1,pszKey);

	int flen = ((strData.length() >> 4) + 1) << 4;
	unsigned char* out_str = (unsigned char*)malloc(flen + 1);
	memset(out_str, 0, flen + 1);
	DecryptFileAESHelper((const unsigned char*)pszNewKey, (const unsigned char*)strData.c_str(), strData.length(), out_str);
	delete[] pszNewKey;
	strOut = (char*)out_str;
	free(out_str);
	return ;
}


void SendStatProxy(const char* fu1,const char* version_channel, const char* fu5,  const char* fu6)
{	
	TSAUTO();
	char szfu1[MAX_PATH] = {0};
	ZeroMemory(szfu1,0);
	strcpy_s(szfu1,MAX_PATH-1,fu1);

	char szversion_channel[MAX_PATH] = {0};
	ZeroMemory(szversion_channel,0);
	strcpy_s(szversion_channel,MAX_PATH-1,version_channel);

	char szfu5[MAX_PATH] = {0};
	ZeroMemory(szfu5,0);
	strcpy_s(szfu5,MAX_PATH-1,fu5);

	char szfu6[MAX_PATH] = {0};
	ZeroMemory(szfu6,0);
	strcpy_s(szfu6,MAX_PATH-1,fu6);

	SendAnyHttpStat(szfu1,szversion_channel,szfu5,szfu6);
}

BOOL GetUserDisplayCardInfo(vector<DISPLAY_CARD_INFO> &vDISPLAY_CARD_INFO)
{
	if (LoadLibraryA("opencl32.dll") == NULL)
	{
		TSDEBUG4CXX("[GetUserDisplayCardInfo] load opencl fail, nErrorCode = "<< GetLastError());
		return FALSE;  
	}

	GetDisplayCardInfo(vDISPLAY_CARD_INFO);
	return TRUE;  
};
extern "C" __declspec(dllexport) BOOL CheckZcashNCond()
{
	vector<DISPLAY_CARD_INFO> vDISPLAY_CARD_INFO;
	if (!GetUserDisplayCardInfo(vDISPLAY_CARD_INFO))
	{
		return FALSE;
	}
	for (std::vector<DISPLAY_CARD_INFO>::const_iterator iter = vDISPLAY_CARD_INFO.begin(); iter != vDISPLAY_CARD_INFO.end(); iter++) {
		TSDEBUG4CXX(L"[CheckZcashNCond] Dispaly Card Info: name = "<< iter->name.c_str()<<L", vendor = "<<iter->vendor<<L", memory_size = "<<iter->memory_size);
		//std::cout << "Information for GPU #" <<index<< ": " << std::endl;
		//std::cout << "GPU Name = " << iter->name.c_str()<< std::endl;
		//std::string strType = "UnKnow";
		//if (iter->vendor == vendor_t::intel)
		//{
		//	strType = "Intel";
		//}
		//else if(iter->vendor == vendor_t::amd)
		//{
		//	strType = "AMD";
		//}
		//else if(iter->vendor == vendor_t::nvidia)
		//{
		//	strType = "NVIDIA";
		//}
		//std::cout << "GPU Type = " <<strType.c_str()<< std::endl;

		//std::cout << "GPU Driver Version = " << iter->version << std::endl;
		//std::cout << "GPU Adapter RAM = " << iter->memory_size/(1024*1024) << std::endl;
		//std::cout << "GPU Cache Size = " << iter->cache_size/(1024*1024) << std::endl;
		//std::cout << "GPU OpenCL PlatformID =  " << iter->platformid<< std::endl;
		//std::cout << std::endl;
		

		if (iter->vendor == vendor_t::nvidia &&  iter->memory_size >= 2000000000)
		{
			TSDEBUG4CXX("[CheckZcashNCond] check success");
			return TRUE;
		}
	}
	return FALSE;
}

extern "C" __declspec(dllexport) BOOL CheckZcashACond()
{
	vector<DISPLAY_CARD_INFO> vDISPLAY_CARD_INFO;
	if (!GetUserDisplayCardInfo(vDISPLAY_CARD_INFO))
	{
		return FALSE;
	}
	for (std::vector<DISPLAY_CARD_INFO>::const_iterator iter = vDISPLAY_CARD_INFO.begin(); iter != vDISPLAY_CARD_INFO.end(); iter++) {
		TSDEBUG4CXX(L"[CheckZcashNCond] Dispaly Card Info: name = "<< iter->name.c_str()<<L", vendor = "<<iter->vendor<<L", memory_size = "<<iter->memory_size);
		if (iter->vendor == vendor_t::amd &&  iter->memory_size >= 2000000000)
		{
			TSDEBUG4CXX("[CheckZcashACond] check success");
			return TRUE;
		}
	}
	return FALSE;
}

extern "C" __declspec(dllexport) BOOL CheckZcashCond()
{
	//return CheckZcashACond() || CheckZcashNCond();
	return CheckZcashNCond();
}

void EncryptAESToFileHelperByPeerID(const unsigned char* pszKey, const char* pszMsg, unsigned char* out_str, int& nlen)
{
	EVP_CIPHER_CTX ctx;
	// init
	EVP_CIPHER_CTX_init(&ctx);
	EVP_CIPHER_CTX_set_padding(&ctx, 1);

	EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)pszKey, NULL);

	//���EVP_EncryptUpdate��ʵ��ʵ�ʾ��ǽ�in����inl�ĳ���ȥ���ܣ�ʵ�ֻ�ȡ�ø�cipher�Ŀ��С����aes_128��˵��16�ֽڣ�����block-size��������ȥ���ܡ�
	//�������Ϊ50�ֽڣ���˴�������48�ֽڣ�outlҲΪ48�ֽڡ�����in�е�������ֽڿ�����ctx->buf����������  
	//����inlΪblock_size�����������Σ���ctx->buf��û����ǰ����������ʱ��ֱ�Ӽӽ��ܲ�����ʡȥ�ܶ����������  
	int msglen = strlen(pszMsg);
	EVP_EncryptUpdate(&ctx, out_str, &nlen, (const unsigned char*)pszMsg, msglen);
	//�������n�ֽڡ��˴����д���
	//�����֧��pading���һ������ݵĻ��ͳ������򣬽�block_size-�������ֽ����������ֽ�����Ϊ�˸�����ֵ����block_size=16,���ݳ���Ϊ4���򽫺����12�ֽ�����Ϊ16-4=12������Ϊһ���������� 
	//����ǰ��Ϊ������ʱ������������Ϊ16�ֽڣ�����ٵ��ô�Finalʱ�������Ƕ�16��0���м��ܣ������Ĳ��ü��ɣ�Ҳ�����ò��ŵ�һ����Final��
	int outl = 0;
	EVP_EncryptFinal_ex(&ctx, out_str + nlen, &outl);  
	nlen += outl;
	EVP_CIPHER_CTX_cleanup(&ctx);
}

void EncryptStringByPeerID(const char* szPid , const char* pszData,std::string &strOut)
{
	char pszKey[MAX_PATH] = {0};
	memset(pszKey,0,MAX_PATH);
	sprintf(pszKey,"RpXVQTFlU7%s",szPid);

	int ubuff = strlen(pszKey)>16?strlen(pszKey):16;
	char* pszNewKey = new(std::nothrow) char[ubuff+1];
	memset(pszNewKey,0,ubuff+1);

	strcpy_s(pszNewKey,ubuff+1,pszKey);

	int msglen = strlen(pszData);
	int flen = ((msglen >> 4) + 1) << 4;
	unsigned char* out_str = (unsigned char*)malloc(flen + 1);
	memset(out_str, 0, flen + 1);

	int nlen = 0;
	EncryptAESToFileHelperByPeerID((const unsigned char*)pszNewKey, pszData, out_str, nlen);
	delete[] pszNewKey;
	if (nlen > 0)
	{
		std::string strBase64 = base64standard::base64_encode(out_str,nlen);
		strOut = strBase64;
		free(out_str);
	}
	return;
}

extern "C" __declspec(dllexport) void SendHttpStatEx(CHAR *fu2, CHAR *fu6,CHAR *version_channel)
{
	TSAUTO();
	char szPid[256] = {0};
	extern void GetPeerID(CHAR *);
	GetPeerID(szPid);
	std::string strfu1= szPid;
	std::string strKey = "";
	{
		std::string strTmp = szPid;
		strKey = strTmp.substr(6,6);

	}
	std::string strfu2= "";
	if (NULL != fu2)
	{
		strfu2 = fu2;
	}

	std::string strfu6= "";
	if (NULL != fu6)
	{
		strfu6 = fu6;
	}

	std::string strfu7= "";
	std::string strfu8= "";
	if (NULL != version_channel)
	{
		std::string strTmp = version_channel;
		size_t iPos = strTmp.find("_", 0);
		if (iPos != std::string::npos)
		{
			strfu7 = strTmp.substr(0,iPos);
			strfu8 = strTmp.substr(iPos+1);
		}
		else
		{
			strfu7 = strTmp;
		}
	}
	std::string strfu9= "";
	__time64_t nCurrentTime = 0;
	_time64(&nCurrentTime);

	{
		std::stringstream sstmp;
		sstmp << nCurrentTime;
		sstmp >> strfu9;
	}

	CHAR szJson[MAX_PATH] = {0};
	memset(szJson,0,MAX_PATH);
	sprintf(szJson, "{\"fu6\":\"%s\",\"fu7\":\"%s\",\"fu8\":\"%s\",\"fu9\":\"%s\"}",
		strfu6.c_str(),strfu7.c_str(), strfu8.c_str(),strfu9.c_str());

	//TSDEBUG4CXX(L"SendEncyptHttpStat szJson = "<<ultra::_A2UTF(szJson));

	std::string strfu5 = "";
	EncryptStringByPeerID(strKey.c_str(),szJson,strfu5);

	TSDEBUG4CXX(L"SendEncyptHttpStat strfu5 = "<<strfu5);
	strfu5 = ultra::URLEncode(strfu5);

	CHAR* szURL = new CHAR[MAX_PATH];
	memset(szURL, 0, MAX_PATH);
	sprintf(szURL, "http://pgv.eastredm.com/pc/cc?fu1=%s&fu2=%s&fu3=&fu4=&fu5=%s",strfu1.c_str(),strfu2.c_str(),strfu5.c_str());

	TSDEBUG4CXX(L"SendAnyHttpStat szURL = "<<ultra::_A2UTF(szURL));
	ResetUserHandle();
	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, SendHttpStatThread, (LPVOID)szURL,0, &dwThreadId);
	CloseHandle(hThread);
	//SendHttpStatThread((LPVOID)szURL);
}