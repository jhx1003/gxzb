#pragma once
#include "MinerClient.h"

class CClientZcashA : public CMinerClient
{
public:
	CClientZcashA(void);
	~CClientZcashA(void);
public:
	void ProcessString(const char *szBuffer);
	void TerminateAllClientInstance();
	void OnAutoExit(DWORD dwExitCode);
private:
	bool IsDebug();
	void LogString(const char *szBuffer);
	void RegexString(const char *szBuffer);
	void PostWndMsg(WPARAM wParam, LPARAM lParam);
	void RetSet();
private:
	HWND m_hMsgWnd;
	//Ϊ������ƥ�����������һ�����ضϵ�\r\n�Ժ����������һ��
	std::string m_strLastLeft;
};

