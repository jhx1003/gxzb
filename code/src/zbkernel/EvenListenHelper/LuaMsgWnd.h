#pragma once
#include "atlwin.h"
#include "map"

#define LUA_MSG_WND_CALSS _T("{659630F6-734C-4ead-B105-1BE419F93F5C}_prowndclass")
#define LUA_MSG_MUTEX _T("{659630F6-734C-4ead-B105-1BE419F93F5C}_promutex")

#include <XLLuaRuntime.h>
typedef void (*funResultCallBack) (DWORD userdata1,DWORD userdata2, const char* pszKey,  DISPPARAMS* pParams);


struct CallbackNode
{
	funResultCallBack pCallBack;
	DWORD userData1;
	DWORD userData2;
	const void* luaFunction;
};

class LuaMsgWindow : public  CWindowImpl<LuaMsgWindow>
{
public:
	static LuaMsgWindow* Instance()
	{
        static LuaMsgWindow s;
		return &s;
	}

	int AttachListener(DWORD userData1,DWORD userData2,funResultCallBack pfn, const void* pfun);
	int DetachListener(DWORD userData1, const void* pfun);
	void CloseSingletonMutex();
	bool HandleSingleton();
	DECLARE_WND_CLASS(LUA_MSG_WND_CALSS)
	BEGIN_MSG_MAP(LuaMsgWindow)
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)

	END_MSG_MAP()
private:
	LuaMsgWindow(void);
	~LuaMsgWindow(void);
	std::vector<CallbackNode> m_allCallBack;

	void Fire_LuaEvent(const char* pszKey, DISPPARAMS* pParams)
	{
		TSAUTO();
		for(size_t i = 0;i<m_allCallBack.size();i++)
		{
 			m_allCallBack[i].pCallBack(m_allCallBack[i].userData1,m_allCallBack[i].userData2, pszKey,pParams);
		}
	}
private:
	HANDLE m_hMutex;

public:
	LRESULT OnCopyData(UINT , WPARAM , LPARAM , BOOL&);
public:
	void SetKeyboardHook(void);
	void DelKeyboardHook(void);
private:
	static LRESULT CALLBACK  KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
public:
	HHOOK m_hKeyboardHook;
};
