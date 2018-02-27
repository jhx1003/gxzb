// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>


#include <shellapi.h>
#include <Wininet.h>
#include <Shlobj.h>
#include <wininet.h>
#include <mapidbg.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

// STL Header Files
#pragma warning(disable:4702)
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <fstream>
#include <algorithm>
#include <assert.h>
using namespace std;

// ATL Header Files
#pragma warning(default:4702)
#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlfile.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <atlsecurity.h>
#include <atltime.h>

// WTL Header Files
#include <WTL/atlapp.h>
#include <WTL/atldlgs.h>
#include <WTL/atlcrack.h>

// xlue Header Files
#include <XLI18N.h>
#include <XLUE.h>
#include <XLGraphic.h>
#include <XLGraphicPlus.h>
#include <XLLuaRuntime.h>
#include <XLFS.h>

#include <winsock2.h>
#include <Gdiplus.h>
#define TSLOG
#define TSLOG_GROUP "GXZB"	//��ѡ,Ĭ��Ϊ "TSLOG"
#include <tslog/tslog.h>

#include "Utility/LuaAPIHelper.h"
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
