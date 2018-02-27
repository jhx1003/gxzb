#ifndef DOWNLOADKERNRL_H
#define DOWNLOADKERNRL_H
#include "..\Utility\Lock.h"
#include "DealSocket.h"
#include "FileHelper.h"
using namespace std;
class  CHttpDownLoad;
typedef struct __TD_PARAM
{
	CHttpDownLoad *pHttpDownLoad;
	int iSectIdx;
}TD_PARAM;

typedef struct __CHTTPDOWNLOADSECT
{
	string   strHost;      // Host��ַ.
	int      nPort;       // Host�˿ں�.
	string   strHttpAddr;      // Http�ļ���ַ(�������ļ���)
	string   strHttpFileName;  // Http�ļ���.
	string   strTmpFilePath;   // ���غ����ʱ�ļ�.
	DWORD    nStart;          // �ָ����ʼλ��.
	DWORD    nEnd;            // �ָ����ʼλ��. 
}CHTTPDOWNLOADSECT;

class  CHttpDownLoad  
{
public:
	CHttpDownLoad();
	virtual ~CHttpDownLoad();
public:
	DWORD GetTotalFileSize();
	DWORD GetFinishedSize();//�ܹ��Ѿ���ɵ�,�����ϴ����ص�
	string GetRemoteUrl();
public:
	string Start(const string& strUrl, const string& strFolder,UINT nThreadCnt = 3);
	BOOL ThreadWorking(int iSectIdx);
private:
	string m_strUrl;
	DWORD m_nFileSize;


	//�ϵ������ֿ�
	CHTTPDOWNLOADSECT *m_sect;
	//�߳�ͬ������
	mutable XMLib::CriticalSection m_cs;

	int m_nCount;//�߳���ɼ�����
	DWORD m_nFinishedSize;//�ܹ��Ѿ���ɵ�

	static CDealSocket sm_DealSocket;
	static CFileHelper sm_FileHelper;
private:
	BOOL HttpDownLoad(UINT nThreadCnt, const string& strHost, int nPort, const string& strHttpAddr, const string& strHttpFileName, const string& strDownLoadPath);
private:
	
	SOCKET ConnectServer(const string& strHost,int nPort);
	string SendHttpHeader(SOCKET hSocket, const string& strHost, const string& strHttpAddr, const string& strHttpFileName,DWORD nPos);
	BOOL ParseHeader(string strHead, DWORD &dwSize);
	DWORD GetHttpHeader(SOCKET hSocket,char *szHeadBuffer);
	BOOL SocketSend(SOCKET hSocket,const string& strSend);
	
	int GetResponseCode(const string& strResponse);
	string GetHeadValueByKey(const string& strHead, char *szKey);
	BOOL FileCombine(int iSectIdx, FILE *fpwrite);
};









#endif