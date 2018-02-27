// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
#pragma once

//-----------------------------------------------------------------------------
// �������ݿ�����
//-----------------------------------------------------------------------------
#pragma warning(disable:4355)	// Level 3: "this": ���ڻ���Ա��ʼֵ�趨���б�
#pragma warning(disable:4251)	// Level 3: need to have dll-interface
#pragma warning(disable:4996)
#pragma warning(disable:4311)
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4245)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4127)
#pragma warning(disable:4312)

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT 0x0403


#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;



#include "Math.h"

//---------------------------------------------------------------------
// serverbase
//---------------------------------------------------------------------
#include "..\common\serverbase\serverbase.h"
using namespace serverbase;

// #ifdef _DEBUG
// #pragma comment(lib,"..\\vsout\\serverbase\\debug\\serverbase.lib")
// #else
// #define X_STRING_RUN_TIME "Release"
// #pragma comment(lib,"..\\vsout\\serverbase\\release\\serverbase.lib")
// #endif

#include "..\common\network\network_define.h"
using namespace networkbase;

// #ifdef _DEBUG
// #pragma comment(lib,"..\\vsout\\network\\debug\\network.lib")
// #else
// #define X_STRING_RUN_TIME "Release"
// #pragma comment(lib,"..\\vsout\\network\\release\\network.lib")
// #endif

#include "..\common\dump\dump_define.h"
using namespace serverdump;

// #ifdef _DEBUG
// #define X_STRING_RUN_TIME "Debug"
// #pragma comment(lib,"..\\vsout\\dump\\debug\\dump.lib")
// #else
// #define X_STRING_RUN_TIME "Release"
// #pragma comment(lib,"..\\vsout\\dump\\release\\dump.lib")
// #endif

#include "..\common\filesystem\file_define.h"
using namespace filesystem;

// #ifdef _DEBUG
// #define X_STRING_RUN_TIME "Debug"
// #pragma comment(lib,"..\\vsout\\filesystem\\debug\\filesystem.lib")
// #else
// #define X_STRING_RUN_TIME "Release"
// #pragma comment(lib,"..\\vsout\\filesystem\\release\\filesystem.lib")
// #endif

#include "..\common\serverframe\frame_define.h"
using namespace serverframe;

// #ifdef _DEBUG
// #define X_STRING_RUN_TIME "Debug"
// #pragma comment(lib,"..\\vsout\\serverframe\\debug\\serverframe.lib")
// #else
// #define X_STRING_RUN_TIME "Release"
// #pragma comment(lib,"..\\vsout\\serverframe\\release\\serverframe.lib")
// #endif

//#ifdef ASSERT
//#undef ASSERT
//#define ASSERT(f)		((void)0)
//#endif



//-----------------------------------------------------------------------
// ServerDefine
//-----------------------------------------------------------------------
#ifdef _DEBUG
//#pragma comment(lib,"..\\vsout\\ServerDefine\\debug\\ServerDefine.lib")
#else
//#pragma comment(lib,"..\\vsout\\ServerDefine\\release\\ServerDefine.lib")
#endif

#include "..\..\common\WorldDefine\creature_define.h"
#include "..\..\common\WorldDefine\base_define.h"
#include "..\..\common\WorldDefine\MapAttDefine.h"
//-----------------------------------------------------------------------
// Other
//-----------------------------------------------------------------------
#include "../../common/WorldDefine/time.h"

//------------------------------------------------------------------------------
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"winmm.lib")

//-------------------------------------------------------------------------------
class World;
class WorldSession;
class DBSession;
class PlayerSession;
class LoginSession;
class Map;
class MapMgr;
class map_creator;
class Role;
class PlayerNetCmdMgr;
class CSocialMgr;
class CGroupMgr;
class CTradeYB;
class activity_mgr;

