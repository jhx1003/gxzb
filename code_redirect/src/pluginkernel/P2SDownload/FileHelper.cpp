#include "stdafx.h"
#include "FileHelper.h"
#include <sys/stat.h>   // ״̬��ʾͷ�ļ�.
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
using namespace std;

CFileHelper::CFileHelper()
{
}

CFileHelper::~CFileHelper()
{
}

BOOL CFileHelper::FileExists(LPCSTR lpszFileName)
{
	DWORD dwAttributes = GetFileAttributesA(lpszFileName);
	if (dwAttributes == 0xFFFFFFFF)
		return FALSE;

	if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		==	FILE_ATTRIBUTE_DIRECTORY)
	{
		return FALSE;
	}
	else{
		return TRUE;
	}
}

FILE* CFileHelper::GetFilePointer(LPCSTR lpszFileName)
{
	FILE *fp;
	if(FileExists(lpszFileName)){
		// �������ļ�����д����.
		fp=fopen(lpszFileName,"r+b");
	}
	else{
		// �������ļ�����д����.
		fp=fopen(lpszFileName,"w+b");
	}

	return fp;
}

DWORD CFileHelper::GetFileSizeByName(LPCSTR lpszFileName)
{
	if(!FileExists(lpszFileName)) return 0;
	struct _stat ST; 
	// ��ȡ�ļ�����.
	_stat(lpszFileName, &ST);
	UINT nFilesize=ST.st_size;
	return nFilesize; 
}

string CFileHelper::GetDownLoadPath(const string& strFolder, const string& strFileName)
{
	char szPath[1024] = {0};
	::PathCombineA(szPath, strFolder.c_str(), strFileName.c_str());
	return string(szPath); 
}