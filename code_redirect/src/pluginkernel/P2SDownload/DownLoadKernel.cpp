#include "stdafx.h"
#include "..\Utility\StringOperation.h"
#include <iostream>
#include "CParseUrl.h"
#include "DownLoadKernel.h"
#include <process.h>
#define MAX_HEAD_LEN 1024
#define MAX_THREAD_CNT 10

#ifdef _MANAGED
#pragma managed(pop)
#endif

CDealSocket CHttpDownLoad::sm_DealSocket;
CFileHelper CHttpDownLoad::sm_FileHelper;


UINT WINAPI ThreadDownLoad(void* pParam)
{
	TD_PARAM *p = (TD_PARAM*)pParam;
	p->pHttpDownLoad->ThreadWorking(p->iSectIdx);
	delete p;
	p = NULL;
	return 0;
}

CHttpDownLoad::CHttpDownLoad()
{
	m_nFileSize = 0;
	m_nCount = 0;
	m_nFinishedSize = 0;

}

CHttpDownLoad::~CHttpDownLoad()
{
	if (NULL != m_sect)
	{
		delete[] m_sect;
		m_sect = NULL;
	}
}

DWORD CHttpDownLoad::GetTotalFileSize()
{
	XMLib::CriticalSectionLockGuard lck(m_cs);
	return m_nFileSize;
}

DWORD CHttpDownLoad::GetFinishedSize()
{
	XMLib::CriticalSectionLockGuard lck(m_cs);
	return m_nFinishedSize;
}

string CHttpDownLoad::GetRemoteUrl()
{
	return m_strUrl;
}

string CHttpDownLoad::Start(const string& strUrl, const string& strFolder, UINT nThreadCnt)
{
	m_strUrl = strUrl;
	Url url(strUrl.c_str());
	string strHost = url.GetHost();
	int nPort = 80;
	if (!url.GetPort().empty())
	{
		nPort	  = (int)atoi(url.GetPort().c_str());
	}
	string strHttpPath = url.GetPath();
	string::size_type nPos = strHttpPath.find_last_of("/");
	if (string::npos == nPos)
	{
		return "";
	}
	string strAnsiFileName = strHttpPath.substr(nPos+1);
	//httpĬ����UTF8���룬���԰����ı���ΪUTF8
	string utf8HttpFileName = ultra::_A2UTF(strAnsiFileName);
	char szHttpAddr[MAX_PATH] = {0};
	sprintf(szHttpAddr,"%s://%s%s",url.GetScheme().c_str(), url.GetHost().c_str(), strHttpPath.substr(0,nPos+1).c_str());
	string strHttpAddr = szHttpAddr;

	SOCKET hSocket = ConnectServer(strHost,nPort);
	if(hSocket == INVALID_SOCKET) 
		return "";
	string strHead = SendHttpHeader(hSocket,strHost, strHttpAddr, utf8HttpFileName,0);
	closesocket(hSocket);
	if (strHead.empty())
	{
		return "";
	}
	if (!ParseHeader(strHead, m_nFileSize))
	{
		return "";
	}
	string strDownLoadPath = sm_FileHelper.GetDownLoadPath(strFolder, strAnsiFileName);
	if(!HttpDownLoad(nThreadCnt,strHost, nPort, strHttpAddr, utf8HttpFileName, strDownLoadPath))
		return "";
	return strDownLoadPath;

}

BOOL CHttpDownLoad::HttpDownLoad(UINT nThreadCnt, const string& strHost, int nPort, const string& strHttpAddr, const string& strHttpFileName, const string& strDownLoadPath)
{
	_ASSERTE(nThreadCnt>0 && nThreadCnt<=MAX_THREAD_CNT);

	m_nCount=0;                                    // �Ѽ���������.
	m_sect = new CHTTPDOWNLOADSECT[nThreadCnt];              // ����Ϣ�ṹ�����ڴ�.
	DWORD nSize= m_nFileSize/nThreadCnt;           // ����ָ�εĴ�С.

	// �����߳�.���ܳ���50����
	HANDLE hMutlThread[MAX_THREAD_CNT];
	for(UINT i=0;i<nThreadCnt;i++)
	{		 
		m_sect[i].strHost = strHost;     
		m_sect[i].nPort  = nPort;	 
		m_sect[i].strHttpAddr = strHttpAddr;     
		m_sect[i].strHttpFileName = strHttpFileName; 		     


		// ������ʱ�ļ���.
		char szTempFilePath[1024] = {0};
		sprintf(szTempFilePath, "%s_%d",strDownLoadPath.c_str(), i);
		m_sect[i].strTmpFilePath = szTempFilePath; // ���غ����ʱ�ļ�.

		if(i<nThreadCnt-1){
			m_sect[i].nStart=i*nSize;            // �ָ����ʼλ��.
			m_sect[i].nEnd=(i+1)*nSize;          // �ָ����ֹλ��.
		}
		else{
			m_sect[i].nStart=i*nSize;            // �ָ����ʼλ��.
			m_sect[i].nEnd=m_nFileSize;        // �ָ����ֹλ��.
		}
		TD_PARAM *pParam = new TD_PARAM;
		pParam->pHttpDownLoad = this;
		pParam->iSectIdx = i;
		hMutlThread[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadDownLoad, pParam, 0, NULL);

	}

	// �ȴ������߳̽���.
	WaitForMultipleObjects(nThreadCnt, hMutlThread, TRUE,INFINITE);

	//��ĳ���ļ���δ�����꣬���ش���Ӧ�����ϲ�������������ء�
	int nFinishCnt = 0;
	{
		XMLib::CriticalSectionLockGuard lck(m_cs);
		nFinishCnt = m_nCount;// ����.
	}
	if(nFinishCnt != nThreadCnt)
		return FALSE;

	FILE *fpwrite;

	// ��д�ļ�.
	if((fpwrite=fopen(strDownLoadPath.c_str(),"wb"))==NULL){
		return FALSE;
	}

	for(UINT uSect=0; uSect<nThreadCnt; uSect++){
		FileCombine(uSect,fpwrite);
	}

	fclose(fpwrite);

	delete[] m_sect;
	m_sect = NULL;
	return TRUE;
}

SOCKET CHttpDownLoad::ConnectServer(const string& strHost,int nPort)
{
	printf("try to connect server\r\n");
	SOCKET hSocket=sm_DealSocket.GetConnect(strHost,nPort);
	if(hSocket == INVALID_SOCKET)
		return INVALID_SOCKET;
	return hSocket;
}

//---------------------------------------------------------------------------
// ����: strHostAddr="www.aitenshi.com",
// strHttpAddr="http://www.aitenshi.com/bbs/images/",
// strHttpFilename="pics.jpg".

string CHttpDownLoad::SendHttpHeader(SOCKET hSocket, const string& strHost, const string& strHttpAddr, const string& strHttpFileName,DWORD nPos)
{
	char szTmp[MAX_HEAD_LEN];
	string strSend;
	
	// Line1: �����·��,�汾.
	sprintf(szTmp,"GET %s%s HTTP/1.1\r\n", strHttpAddr.c_str() , strHttpFileName.c_str());
	strSend.append(szTmp);
	ZeroMemory(szTmp,MAX_HEAD_LEN);

	// Line2:����.
	sprintf(szTmp,"Host: %s\r\n",strHost.c_str());
	strSend.append(szTmp);
	ZeroMemory(szTmp,MAX_HEAD_LEN);

	// Line3:���յ���������.
	sprintf(szTmp, "Accept: */*\r\n");
	strSend.append(szTmp);
	ZeroMemory(szTmp,MAX_HEAD_LEN);
	
	// Line4:�ο���ַ.
	sprintf(szTmp, "Referer: %s\r\n",strHttpAddr.c_str());
	strSend.append(szTmp);
	ZeroMemory(szTmp,MAX_HEAD_LEN);

	// Line5:���������.
	sprintf(szTmp,"User-Agent: Mozilla/4.0 \
				 (compatible; MSIE 5.0; Windows NT; DigExt; DTS Agent;)\r\n");
	strSend.append(szTmp);
	ZeroMemory(szTmp,MAX_HEAD_LEN);

	// ����. Range ��Ҫ���ص����ݷ�Χ������������Ҫ.
	sprintf(szTmp, "Range: bytes=%d-\r\n",nPos);
	strSend.append(szTmp);
	ZeroMemory(szTmp,MAX_HEAD_LEN);

	// LastLine: ����.
	strSend.append("\r\n");

	if(!SocketSend(hSocket,strSend)) 
		return "";

	// ȡ��httpͷ.
	char szHeadBuffer[MAX_HEAD_LEN] = {0};
	UINT uHeadSize = GetHttpHeader(hSocket,szHeadBuffer);
	if(!uHeadSize)
	{
		printf("get http head error.\n");
		return "";
	}
	
	return string(szHeadBuffer);
}

BOOL CHttpDownLoad::ParseHeader(string strHead, DWORD &dwSize)
{
	int nResponseCode = GetResponseCode(strHead);

	if(nResponseCode != 206) 
		return FALSE;

	string strFileSize = GetHeadValueByKey(strHead, "Content-Length");
	{
		std::stringstream ss;
		ss << strFileSize;
		ss >> dwSize;
	}
	if (dwSize <= 0)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CHttpDownLoad::SocketSend(SOCKET hSocket,const string& strSend)
{
	int iLen=strSend.length();
	if(send (hSocket,strSend.c_str(),iLen,0)==SOCKET_ERROR)
	{
		closesocket(hSocket);
		printf("send request fail!\n");
		return FALSE;
	}

	return TRUE;
}

DWORD CHttpDownLoad::GetHttpHeader(SOCKET sckDest,char *str)
{
	DWORD nResponseHeaderSize;
	char c = 0;
	int nIndex = 0;
	BOOL bFinish = FALSE;
	while(!bFinish && nIndex < 1024)
	{
		recv(sckDest,&c,1,0);
		str[nIndex++] = c;
		if(nIndex >= 4)
		{
			if( str[nIndex - 4] == '\r' && 
				str[nIndex - 3] == '\n' && 
				str[nIndex - 2] == '\r' && 
				str[nIndex - 1] == '\n')
				bFinish = TRUE;
		}
	}

	str[nIndex]=0;
	nResponseHeaderSize = nIndex;

	return nResponseHeaderSize;
}

int CHttpDownLoad::GetResponseCode(const string& strResponse)
{
	string strLower = ultra::ToLower(strResponse);
	strLower = ultra::Trim(strLower);
	string::size_type nBegainPos = 0;
	string::size_type nEndPos = 0;
	if ((nBegainPos = strLower.find("http/")) != 0)
	{
		return -1;
	}
	nBegainPos = strLower.find (" ");
	if (string::npos == nBegainPos)
	{
		return -1;
	}
	nEndPos = strLower.find(" ",nBegainPos+1);
	if (string::npos == nEndPos)
	{
		return -1;
	}
	string strCode = strLower.substr(nBegainPos+1, nEndPos-nBegainPos);
	strCode = ultra::Trim(strCode);

	return (DWORD)atoi(strCode.c_str());
}

string CHttpDownLoad::GetHeadValueByKey(const string& strHead, char *szKey)
{
	string strKey = szKey;
	strKey.append(":");
	string::size_type nBegainPos = strHead.find(strKey);
	if (nBegainPos == string::npos)
	{
		return "";
	}
	string::size_type nEndPos = strHead.find("\r\n",nBegainPos+strKey.length());
	if (nEndPos == string::npos)
	{
		return "";
	}
	UINT uLen = nEndPos - (nBegainPos+strKey.length());
	string strValue = strHead.substr(nBegainPos+strKey.length(), uLen);
	return ultra::Trim(strValue);
}

BOOL CHttpDownLoad::ThreadWorking(int iSectIdx)
{
	CHTTPDOWNLOADSECT *pSect = &m_sect[iSectIdx];

	SOCKET hSocket = ConnectServer(pSect->strHost,pSect->nPort);
	if(hSocket == INVALID_SOCKET) 
	{
		printf("thread %d connect server error", iSectIdx);
		return FALSE;
	}


	// ������ʱ�ļ���С��Ϊ�˶ϵ�����
	DWORD nFileSize=sm_FileHelper.GetFileSizeByName(pSect->strTmpFilePath.c_str());
	DWORD nSectSize=(pSect->nEnd)-(pSect->nStart);

	// �˶����������.
	if(nFileSize==nSectSize)
	{
		{
			XMLib::CriticalSectionLockGuard lck(m_cs);
			m_nCount++;// ����.
			m_nFinishedSize += nSectSize;
		}
		printf("sect %d download finish", iSectIdx);
		return TRUE;
	}
	else if(nFileSize > 0)
	{
		{
			XMLib::CriticalSectionLockGuard lck(m_cs);
			m_nFinishedSize += nFileSize;
		}
	}
	FILE *fpwrite=sm_FileHelper.GetFilePointer(pSect->strTmpFilePath.c_str());
	if(!fpwrite) 
	{
		printf("thread %d get file pointer error", iSectIdx);
		return FALSE;
	}

	// �������ط�Χ.
	SendHttpHeader(hSocket,pSect->strHost,pSect->strHttpAddr,pSect->strHttpFileName,pSect->nStart+nFileSize);

	// �����ļ�дָ����ʼλ�ã��ϵ�����
	fseek(fpwrite,nFileSize,SEEK_SET);

	DWORD nLen; 
	DWORD nSumLen=0; 
	char szBuffer[1024];

	while(1)
	{
		if(nSumLen>=nSectSize-nFileSize) 
			break;
		nLen=recv(hSocket,szBuffer,sizeof(szBuffer),0);
		{
			XMLib::CriticalSectionLockGuard lck(m_cs);
			m_nFinishedSize += nLen;
		}
		if (nLen == SOCKET_ERROR){
			printf("thread %d recv error!\n", iSectIdx);
			fclose(fpwrite);
			return 1;
		}

		if(nLen==0) 
			break;
		nSumLen +=nLen;
		//printf("thread %d recv data len %d\n",iSectIdx, nLen);
		fwrite(szBuffer,nLen,1,fpwrite);
	}

	fclose(fpwrite);      // �ر�д�ļ�.
	closesocket(hSocket); // �ر��׽���.
	{
		XMLib::CriticalSectionLockGuard lck(m_cs);
		m_nCount++;
	}
	return 0;
}

BOOL CHttpDownLoad::FileCombine(int iSectIdx, FILE *fpwrite)
{	
	CHTTPDOWNLOADSECT *pSect = &m_sect[iSectIdx];
	FILE *fpread;

	// ���ļ�.
	if((fpread=fopen(pSect->strTmpFilePath.c_str(),"rb"))==NULL)
		return FALSE;

	DWORD nPos=pSect->nStart;

	// �����ļ�дָ����ʼλ��.
	fseek(fpwrite,nPos,SEEK_SET);

	int c;
	// ���ļ�����д�뵽���ļ�.		
	while((c=fgetc(fpread))!=EOF)
	{
		fputc(c,fpwrite);
		nPos++;
		if(nPos==pSect->nEnd) break;
	}

	fclose(fpread);
	DeleteFileA(pSect->strTmpFilePath.c_str());

	return TRUE;
}