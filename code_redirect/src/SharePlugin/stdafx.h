// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#pragma warning(disable:4702)
#include <string>
#include <atlbase.h>
#include <WTL/atlapp.h>
#define TSLOG
#define TSLOG_GROUP "GXZB"	//��ѡ,Ĭ��Ϊ "TSLOG"
#include <tslog/tslog.h>

extern CAppModule _Module;

#include <atlwin.h>

#ifndef _countof
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
