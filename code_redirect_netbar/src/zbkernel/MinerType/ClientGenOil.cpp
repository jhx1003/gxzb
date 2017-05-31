#include "StdAfx.h"
#include "ClientGenOil.h"
#include "ClientMsgDefine.h"
#include <Tlhelp32.h>
#include "..\Utility\StringOperation.h"
#include "boost/regex.hpp"
#include <boost/algorithm/string.hpp>
#include "../EvenListenHelper/LuaMsgWnd.h"

#define LOGCFG_PATH _T("C:\\GXZB_CONFIG\\GenOil.ini")
#define CLIENT_NAME _T("ShareGenoil.exe")

CClientGenOil::CClientGenOil(void)
{
	m_hMsgWnd = FindWindow(LUA_MSG_WND_CALSS, NULL);
	m_strLastLeft = "";
	m_DagProgress = -1;
}

CClientGenOil::~CClientGenOil(void)
{

}

bool CClientGenOil::IsDebug()
{
#if defined _DEBUG
	return true;
#else
	if (::PathFileExists(LOGCFG_PATH))
	{
		return true;
	}
	return false;
#endif
}

void CClientGenOil::TerminateAllClientInstance()
{
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		BOOL bResult = ::Process32First(hSnap, &pe);
		while (bResult)
		{
			if(_tcsicmp(pe.szExeFile, CLIENT_NAME) == 0 && pe.th32ProcessID != 0)
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
				::TerminateProcess(hProcess, 4);
			}
			bResult = ::Process32Next(hSnap, &pe);
		}
		::CloseHandle(hSnap);
	}
}

void CClientGenOil::LogString(const char *szBuffer)
{
	if (IsDebug())
	{
		std::wstring wstrInfo = ultra::_A2T(szBuffer);
		TSDEBUG4CXX(L"<Client GenOil> Output: " <<wstrInfo.c_str());
	}
}

void CClientGenOil::PostWndMsg(WPARAM wParam, LPARAM lParam)
{
	PostMessageA(m_hMsgWnd, WM_GENOIL_MSG, wParam, lParam);
}

void CClientGenOil::OnAutoExit(DWORD dwExitCode)
{
	PostWndMsg(WP_GENOIL_AUTOEXIT, dwExitCode);
}

void CClientGenOil::RetSet()
{
	TSDEBUG4CXX(L"RetSet GenOil client Param");
	m_DagProgress = -1; 
	m_strLastLeft = "";
}


void CClientGenOil::ProcessString(const char *szBuffer)
{
	
	//LogString(szBuffer);
	RegexString(szBuffer);
}

void CClientGenOil::RegexString(const char *szBuffer)
{
	std::string strBuffer = m_strLastLeft;
	strBuffer.append(szBuffer);
	LogString(strBuffer.c_str());
	std::string::const_iterator start = strBuffer.begin();
	std::string::const_iterator end = strBuffer.end();
	boost::regex exp("",boost::regex::icase);
	boost::smatch what;
	do 
	{
		m_strLastLeft = "";
		if (boost::iends_with(strBuffer.c_str(),"\r\n"))
		{
			break;
		}
		exp.assign(".*\\r\\n(.*)", boost::regex::icase);
		if(!boost::regex_search(start,end,what,exp))
		{
			break;
		} 
		if (what.size() == 2 && what[1].matched )
		{
			m_strLastLeft = what[1];
			std::wstring wstrInfo = ultra::_A2T(m_strLastLeft);
			TSDEBUG4CXX(L"[RegexString]: m_strLastLeft = " << wstrInfo.c_str());

		}
	} while (FALSE);

	//�ύshare�ɹ�
	//�ɹ� ʾ����"B-) Submitted and accepted."
	//ʧ�� ʾ����":-( Not accepted."
	if (boost::icontains(strBuffer,"B-) Submitted and accepted."))
	{
		PostWndMsg(WP_GENOIL_SHARE, 0);
		TSDEBUG4CXX(L"[RegexString]: " << L"B-) Submitted and accepted");
	}
	else if (boost::icontains(strBuffer,":-( Not accepted."))
	{
		PostWndMsg(WP_GENOIL_SHARE, 1);
		TSDEBUG4CXX(L"[RegexString]: " << L":-( Not accepted");
	}
	//��ǰ����
	//ʾ����"Mining on PoWhash #84f8453a : 0.00MH/s [A0+0:R0+0:F0]"
	exp.assign("Mining on PoWhash #[^:]* : ([0-9\\.]+)MH/s ", boost::regex::icase);
	UINT uCurrentSpeed = 0;
	while(boost::regex_search(start,end,what,exp))
	{
		if (what.size() == 2)
		{
			if (what[1].matched)
			{
				std::string strSpeed = what[1];
				float fNewSpeed = 0.00f;
				{
					std::stringstream ssNewSpeed;
					ssNewSpeed << strSpeed;
					ssNewSpeed >> fNewSpeed;
				}
				UINT uNewSpeed = ( int )( fNewSpeed * 100 + 0.5 );
				if (uNewSpeed > uCurrentSpeed)
				{
					uCurrentSpeed = uNewSpeed;
				}

			}
		}
		start = what[0].second; 
	} 
	if (uCurrentSpeed >= 1)
	{
		m_DagProgress = 101; //�Ѿ����ٶ��˾Ͳ��ٽ���DAG��
		PostWndMsg(WP_GENOIL_SPEED, uCurrentSpeed);
		TSDEBUG4CXX(L"[RegexString]: " << L"uCurrentSpeed = "<< uCurrentSpeed);
		return;
	}
	
	//����DAG
	//ʾ����"OPENCL#0: 0%"
	//TSDEBUG4CXX(L"[RegexString]: " << L"m_DagProgress = "<< m_DagProgress);
	if (m_DagProgress < 101)
	{
		exp.assign("OPENCL#[0-9]+: ([0-9]+)%", boost::regex::icase);
		while(boost::regex_search(start,end,what,exp))
		{
			if (what.size() == 2)
			{
				if (what[1].matched)
				{
					std::string strProgress = what[1];
					int iNewProgress = 0;
					{
						std::stringstream ssNewProgress;
						ssNewProgress << strProgress;
						ssNewProgress >> iNewProgress;
					}
					if (iNewProgress > m_DagProgress)
					{
						m_DagProgress = iNewProgress;
					}
				}
			}
			start = what[0].second; 
		} 

		if (m_DagProgress >= 0)
		{
			PostWndMsg(WP_GENOIL_DAG, m_DagProgress);
			TSDEBUG4CXX(L"[RegexString]: " << L"m_DagProgress = "<< m_DagProgress);
			return;
		}
	}
	//��Ҫ��Ϣ���ȴ���

	//���ӿ�γɹ�
	//1:"Connected!" 2:"Connected to stratum server "
	if (boost::icontains(strBuffer,"Connected!") || boost::icontains(strBuffer,"Connected to stratum server "))
	{
		PostWndMsg(WP_GENOIL_CONNECT_POOL, 0);
		TSDEBUG4CXX(L"[RegexString]: " << L"Connecte server success");
	}
	//�������
	//���������ҽ����Զ��˳�
	//�����в�������"Invalid argument:"
	//�Ҳ������ʵ��Կ���"No GPU device with sufficient memory was found. Can't GPU mine. Remove the -G argument"

	if (boost::icontains(strBuffer,"Invalid argument:"))
	{
		PostWndMsg(WP_GENOIL_ERROR_INFO, 1);
		TSDEBUG4CXX(L"[RegexString]: " << L"Invalid argument");
	}
	else if(boost::icontains(strBuffer,"No GPU device with sufficient memory was found. Can't GPU mine. Remove the -G argument"))
	{
		PostWndMsg(WP_GENOIL_ERROR_INFO, 2);
		TSDEBUG4CXX(L"[RegexString]: " << L"No GPU device with sufficient memory was found");
	}
	//���������ǽ��̼�������
	// �������ʧ�ܣ�"Reconnecting in 3 seconds..."
	//opencl �������"error: front end compiler failed build"
	//opencl �ڲ�ִ�д���"error: invalid storage-class specifiers in OpenCL"


	if(boost::icontains(strBuffer,"Reconnecting in 3 seconds..."))
	{
		PostWndMsg(WP_GENOIL_CONNECT_POOL, 1);
		TSDEBUG4CXX(L"[RegexString]: " << L"Connecte server fail, try to reconnect");
	}
	else if (boost::icontains(strBuffer,"error: front end compiler failed build"))
	{
		PostWndMsg(WP_GENOIL_ERROR_INFO, 3);
		TSDEBUG4CXX(L"[RegexString]: " << L"Error: front end compiler failed build");
	}
	else if (boost::icontains(strBuffer,"error: invalid storage-class specifiers in OpenCL"))
	{
		PostWndMsg(WP_GENOIL_ERROR_INFO, 4);
		TSDEBUG4CXX(L"[RegexString]: " << L"Error: invalid storage-class specifiers in OpenCL");
	}
	else if (boost::icontains(strBuffer,"error: "))
	{
		PostWndMsg(WP_GENOIL_ERROR_INFO, 5);
		TSDEBUG4CXX(L"[RegexString]: " << L"Error: new resone");
	}
	//opencl ������ clEnqueueWriteBuffer(-38)

	//����boost::regex ��assign ��2������Ĭ����regex_constants::normal �����Ǵ�Сд���е�(����Ҫ���Сд����)
	exp.assign("(cl[A-Z][\\w]+)\\((\\-[0-9]+\\))");

	int iErrorCode = 0;
	std::string strErrorFun;
	if(boost::regex_search(start,end,what,exp))
	{
		if (what.size() == 3 && what[1].matched && what[2].matched)
		{
			strErrorFun = what[1];
			std::string strErrorCode = what[2];
			{
				std::stringstream ssErrorCode;
				ssErrorCode << strErrorCode;
				ssErrorCode >> iErrorCode;
			}
		}
	} 
	if (iErrorCode < 0)
	{	
		PostWndMsg(WP_GENOIL_ERROR_OPENCL, iErrorCode);
		std::wstring wstrErrorFun = ultra::_A2T(strErrorFun);
		TSDEBUG4CXX(L"[RegexString]: " << L"OpenCL Error function(" <<wstrErrorFun.c_str()<<"), ErrorCode = "<<iErrorCode);
	}

}