#pragma once
#include "MinerClient.h"

class CClientXmr : public CMinerClient
{
public:
	CClientXmr(UINT uMinerType);
	~CClientXmr(void);
public:
	void ProcessString(const char *szBuffer);
	void TerminateAllClientInstance();
	void OnAutoExit(DWORD dwExitCode);
	void SetMinerType(UINT uNewMinerType) 
	{
		m_uMinerType = uNewMinerType;
	}
private:
	bool IsDebug();
	void LogString(const char *szBuffer);
	void RegexString(const char *szBuffer);
	void PostWndMsg(WPARAM wParam, LPARAM lParam);
	void RetSet();
	void PostErrorMsg(const char *szBuffer, const char *szBeg);
private:
	HWND m_hMsgWnd;
	//Ϊ������ƥ�����������һ�����ضϵ�\r\n�Ժ����������һ��
	std::string m_strLastLeft;
	UINT m_uMinerType;
};