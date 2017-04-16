#pragma once

#define IPC_UTIL_CLASS	"IPC.Util.Class"
#define IPC_UTIL_OBJ		"IPC.Util"

#include "..\MinerType\MinerClient.h"
/*�����ڿ�ͻ�������*/
typedef enum
{
	MINER_GENOIL = 1,
}MinerType;


class LuaIPCUtil
{
public:
	LuaIPCUtil(void);
	~LuaIPCUtil(void);

private:

public:
	static LuaIPCUtil * __stdcall Instance(void *);
	static void RegisterObj(XL_LRT_ENV_HANDLE hEnv);

public:
	static int Init(lua_State* pLuaState);
	static int Start(lua_State* pLuaState);
	static int Quit(lua_State* pLuaState);
	static int Pause(lua_State* luaState);
	static int Resume(lua_State* luaState);
	static int IsWorkProcessRunning(lua_State* pLuaState);
private:
	static XLLRTGlobalAPI sm_LuaMemberFunctions[];
	static HANDLE m_hWorkProcess;
	static HANDLE m_hPipeThread;

	//������: ����stdin�ܵ������˾��    
	static HANDLE  m_hStdInRead;         //�ӽ����õ�stdin�Ķ����    
	static HANDLE  m_hStdInWrite;        //�������õ�stdin�Ķ����    

	//������: ����stdout�ܵ������˾��    
	static HANDLE  m_hStdOutRead;     ///�������õ�stdout�Ķ����    
	static HANDLE  m_hStdOutWrite;    ///�ӽ����õ�stdout��д���
public:
	static void CycleHandleInfoFromPipe();
	static void Clear();
	static void TerminateMiningProcess();
	static BOOL EnableDebugPrivilege();
};