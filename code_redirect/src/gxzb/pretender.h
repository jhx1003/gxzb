#pragma once

#include <shlobj.h>
#include <list>
#include "ImagePreReader.h"
#define KERNEL_NAME  "zbkernel.dll"
class Pretender
{
public:
	Pretender() : m_bInitOK(FALSE), m_hKernel(NULL)
	{

		SetDependDirToEnvironmentVariable();
		//const TCHAR *rgszFileName[] = {
		//	_T("zlib1.dll"), 
		//	//_T("minizip.dll"),  
		//	_T("xlfsio.dll"), 
		//	_T("xlluaruntime.dll"), 
		//	_T("libexpat.dll"), 
		//	_T("libpng13.dll"), 
		//	_T("xlgraphic.dll"), 
		//	_T("xlgraphicplus.dll"),
		//	_T("xlue.dll")
		//};
		//HMODULE rghmodDeps[_countof(rgszFileName)] = {NULL};
		//PreLoadDll(rgszFileName, _countof(rgszFileName), rghmodDeps);
		//BOOL bAllOk = TRUE;
		//for (int i = 0; i < _countof(rghmodDeps); ++i)
		//{
		//	if (NULL == rghmodDeps[i])
		//	{
		//		bAllOk = FALSE;
		//	}
		//}
		//if (!bAllOk) return;
		//for (int i = 0; i < _countof(rghmodDeps); ++i)
		//{
		//	m_modList.push_back(rghmodDeps[i]);
		//}

		CHAR szExePath[MAX_PATH] = {0};
		GetModuleFileNameA(NULL, szExePath, MAX_PATH);
		PathRemoveFileSpecA(szExePath);

		const CHAR *dllFileName[] = {
			("zlib1.dll"),   
			("xlfsio.dll"), 
			("xlluaruntime.dll"), 
			("libexpat.dll"), 
			("libpng13.dll"), 
			("xlgraphic.dll"), 
			("xlgraphicplus.dll"),
			("xlue.dll"),
			(KERNEL_NAME)
		};
		PreReadDll(dllFileName, _countof(dllFileName),szExePath);
		m_hKernel = LoadLibraryA(KERNEL_NAME);
		if (NULL == m_hKernel)
		{
			return;
		}
		m_bInitOK = TRUE;
	}

	~Pretender()
	{
		if (m_hKernel)
		{
			FreeLibrary(m_hKernel);
			m_hKernel = NULL;
		}

		for (std::list< HMODULE >::reverse_iterator rit = m_modList.rbegin();
			rit != m_modList.rend();
			++rit)
		{
			if (*rit)
			{
				//TSDEBUG(_T("About to FreeLibrary(0x%p)"), *rit);
				::FreeLibrary(*rit);
			}
		}
	}

	static void PreLoadDll(const TCHAR **rgszFileName, ULONG nFileCount, HMODULE *pResult)
	{
		for (unsigned int i = 0; i < nFileCount; ++i)
		{
			pResult[i] = ::LoadLibrary(rgszFileName[i]);
			DWORD dwLastError = ::GetLastError();
			//TSDEBUG(_T("LoadLibrary(%s) return 0x%p, dwLastError = %lu"), rgszFileName[i], pResult[i], dwLastError);
		}
	}

	static void PreReadDll(const CHAR **rgszFileName, ULONG nFileCount, const CHAR * szDir)
	{
		for (unsigned int i = 0; i < nFileCount; ++i)
		{
			const size_t kStepSize = 1024 * 1024;
			CHAR szTmp[MAX_PATH] = {0}; 
			PathCombineA(szTmp,szDir,rgszFileName[i]);
			CImagePreReader::PreReadImage(szTmp, 0, kStepSize);
		}
	}

	BOOL Init(LPTSTR lpCmdLine = NULL)
	{
		BOOL bRet = FALSE;
		if (m_bInitOK)
		{
			//1:先处理单例
			typedef HRESULT (STDAPICALLTYPE * PHandleSingleton)();
			PHandleSingleton pHandleSingleton = (PHandleSingleton)GetProcAddress(m_hKernel,"HandleSingleton");
			if(NULL == pHandleSingleton)
			{
				MessageBox(NULL, L"无法找到 'HandleSingleton', 请重新安装", L"错误", MB_OK);
				TerminateProcess(GetCurrentProcess(), (UINT)-10);
				return -10;
			}
			HRESULT hr = pHandleSingleton();
			if(SUCCEEDED( hr))
			{
				//处理过了
				TSDEBUG(_T("HandleSingleton return true, so exit.lpCmdLine = %s"), lpCmdLine);
				return bRet;		
			}

			typedef BOOL (STDAPICALLTYPE * PInitXLUE)(wchar_t*);
			PInitXLUE pInitXLUE = (PInitXLUE)GetProcAddress(m_hKernel, "InitXLUE");
			if (pInitXLUE)
			{
				bRet = pInitXLUE(lpCmdLine);
			}
		}
		return bRet;
	}

	void SetDependDirToEnvironmentVariable()
	{
		//设置环境变量
		TCHAR szEVPath[32768] = {0};
		TCHAR szNewEVPath[32768] = {0};

		TCHAR szExeDir[MAX_PATH] = {0};
		GetModuleFileName(NULL, szExeDir, MAX_PATH);
		PathRemoveFileSpec(szExeDir);

		GetEnvironmentVariable(_T("Path"), szEVPath, 32768);
		
		TCHAR szDepDirx86[MAX_PATH] = {0};
		PathCombine(szDepDirx86,szExeDir,_T("depx86"));

		TCHAR szDepDirx64[MAX_PATH] = {0};
		PathCombine(szDepDirx64,szExeDir,_T("depx64"));

		_sntprintf(szNewEVPath, 32768, _T("%s;%s;%s;%s"), szDepDirx86,szDepDirx64,szExeDir, szEVPath);	

		SetEnvironmentVariable(_T("Path"), szNewEVPath);

		TSDEBUG4CXX("SetDependDirToEnvironmentVariable : "<<szNewEVPath);
	}
private:

private:
	HMODULE m_hKernel;
	BOOL	m_bInitOK;
	std::list< HMODULE > m_modList;
};