
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//Lua脚本管理器
#include "stdafx.h"

#include "script_mgr.h"
#include "lua_functions.h"
#include "world.h"
#include "unit.h"
#include "role.h"
#include "creature.h"
#include "creature_ai.h"
#include "map.h"
#include "activity_mgr.h"

ScriptMgr g_ScriptMgr;

//------------------------------------------------------------------------------------
// 初始化
//------------------------------------------------------------------------------------
BOOL ScriptMgr::Init()
{
	

	RegisterScriptEventFunc();

	m_pMasterState = luaL_newstate();
	luaL_openlibs(m_pMasterState);

	m_mapThreadState.clear();

	m_dwScriptMutexIDGen = 0;
	m_mapScriptMutex.clear();

	m_mapQusetScript.clear();
	m_mapCreatureScript.clear();
	m_mapMapScript.clear();
	m_mapItemScript.clear();
	m_mapSkillScript.clear();
	m_mapActScript.clear();
	m_pRoleScript = NULL;
	m_pWorldScript = NULL;
	m_dwMaxPcallTime = 0;

	RegisterCoreFunctions();		// 注册C接口函数
	LoadScripts();					// 加载脚本

	return TRUE;
}

//-------------------------------------------------------------------------------------
// 注册异步调用函数
//-------------------------------------------------------------------------------------
VOID ScriptMgr::RegisterScriptEventFunc()
{
	RegisterEventFunc(EVT_Script_Reload,		&ScriptMgr::OnReload);
}

//-------------------------------------------------------------------------------------
// 异步事件——脚本重新加载
//-------------------------------------------------------------------------------------
VOID ScriptMgr::OnReload(DWORD dwSenderID, LPVOID pEventMessage)
{
	Reload();
}

//-------------------------------------------------------------------------------------
// 脚本管理器销毁
//-------------------------------------------------------------------------------------
VOID ScriptMgr::Destroy()
{
	DestroyScripts();	// 删除所有脚本
	DestroyAllStates();	// 删除所有State相关内容
}

//--------------------------------------------------------------------------------------
// 更新
//--------------------------------------------------------------------------------------
VOID ScriptMgr::Update()
{
	EventMgr<ScriptMgr>::Update();
}

//--------------------------------------------------------------------------------------------------
// 重新加载脚本
//--------------------------------------------------------------------------------------------------
VOID ScriptMgr::Reload()
{
	UnloadScripts();	// 卸载所有脚本
	DestroyAllStates();	// 删除所有State相关内容

	// 重新生成主State
	m_pMasterState = luaL_newstate();
	luaL_openlibs(m_pMasterState);

	RegisterCoreFunctions();		// 注册C接口函数
	LoadScripts();					// 加载脚本
}

//--------------------------------------------------------------------------------------------------
// 注册供Lua使用的函数
//--------------------------------------------------------------------------------------------------
VOID ScriptMgr::RegisterCoreFunctions()
{
	LuaOpenCommLibs(m_pMasterState);
}


//----------------------------------------------------------------------------------------------------
// Lua引擎加载脚本
//-----------------------------------------------------------------------------------------------------
VOID ScriptMgr::LoadScripts()
{
	vector<tstring> luaFiles;

	// 得到脚本的路径
	tstring strPath = World::p_var->get_string(_T("path"), _T("script"));

	// 将所有Lua文件存储进来
	LoadScriptsFromDir(strPath.c_str(), luaFiles, TRUE);

	print_message(_T("Loading %d Scripts...\r\n"), luaFiles.size());

	TCHAR szFileName[MAX_PATH];

	// 依次加载各个Lua文件，并编译执行
	for(vector<tstring>::iterator itr = luaFiles.begin(); itr != luaFiles.end(); ++itr)
	{
		_sntprintf(szFileName, MAX_PATH, _T("%s"), itr->c_str());
		print_message(_T("Loading Script %s...\r\n"), itr->c_str());

		// 载入并编译文件
		if( luaL_loadfile(m_pMasterState, get_tool()->unicode_to_ansi(szFileName)) != 0 )
		{
			print_message(_T("Script %s, load failed!!!\r\n"), itr->c_str());
			const CHAR* szMsg = lua_tostring(m_pMasterState, -1);
			if( szMsg != NULL )
			{
				print_message(_T("LuaEngine, reason: %s\r\n"), get_tool()->ansi_to_unicode(szMsg));
			}

			lua_pop(m_pMasterState, 1);

			continue;
		}

		// 执行Lua文件
		if( lua_pcall(m_pMasterState, 0, LUA_MULTRET, 0) != 0 )
		{
			print_message(_T("LuaEngine, could not run %s!!!!"), itr->c_str());
			const CHAR* szMsg = lua_tostring(m_pMasterState, -1);
			if( szMsg != NULL )
			{
				print_message(_T("LuaEngine, reason: %s"), get_tool()->ansi_to_unicode(szMsg));
			}

			lua_pop(m_pMasterState, 1);

			continue;
		}
	}

	// 清空堆栈
	lua_settop(m_pMasterState, 0);

}

//-----------------------------------------------------------------------------------
// 从目录搜索
//-----------------------------------------------------------------------------------
VOID ScriptMgr::LoadScriptsFromDir(LPCTSTR szDir, std::vector<tstring>& luaFiles, BOOL bFirst)
{
	if( bFirst )
	{
		// 如果是第一次进入该函数，则寻找全局脚本目录
		TCHAR szGlobalDir[MAX_PATH];
		_tcscpy(szGlobalDir, szDir);
		_tcscat(szGlobalDir, _T("\\"));
		_tcscat(szGlobalDir, _T("gloable"));
		LoadScriptsFromDir(szGlobalDir, luaFiles);
	}

	HANDLE hFile;
	WIN32_FIND_DATA fd;
	memset(&fd, 0, sizeof(fd));

	TCHAR szSearchName[MAX_PATH];

	_tcscpy(szSearchName, szDir);
	_tcscat(szSearchName, _T("\\*.*"));

	hFile = FindFirstFile(szSearchName, &fd);
	FindNextFile(hFile, &fd);

	while( FindNextFile(hFile, &fd) )
	{
		// 如果是目录，则递归调用
		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if( !bFirst || 0 != _tcscmp(fd.cFileName, _T("gloable")) )
			{
				_tcscpy(szSearchName, szDir);
				_tcscat(szSearchName, _T("\\"));
				_tcscat(szSearchName, fd.cFileName);
				LoadScriptsFromDir(szSearchName, luaFiles);
			}
		}
		else
		{
			TCHAR* ext = _tcsrchr(fd.cFileName, _T('.'));

			if(VALID_POINT(ext) && !_tcsicmp(ext, _T(".lua")) )
			{
				tstring fname = szDir;
				fname += _T("\\");
				fname += fd.cFileName;

				luaFiles.push_back(fname);
			}
		}
	}
	FindClose(hFile);
}

//-----------------------------------------------------------------------------------
// 删除所有脚本
//-----------------------------------------------------------------------------------
VOID ScriptMgr::DestroyScripts()
{
	// 清空任务脚本
	quest_script* pQuestScript = NULL;
	m_mapQusetScript.reset_iterator();
	while( m_mapQusetScript.find_next(pQuestScript) )
	{
		SAFE_DELETE(pQuestScript);
	}
	m_mapQusetScript.clear();

	// 清空怪物脚本
	CreatureScript* pCreatureScript = NULL;
	m_mapCreatureScript.reset_iterator();
	while( m_mapCreatureScript.find_next(pCreatureScript) )
	{
		SAFE_DELETE(pCreatureScript);
	}
	m_mapCreatureScript.clear();

	// 清空地图脚本
	MapScript* pMapScript = NULL;
	m_mapMapScript.reset_iterator();
	while( m_mapMapScript.find_next(pMapScript) )
	{
		SAFE_DELETE(pMapScript);
	}
	m_mapMapScript.clear();

	// 清空活动脚本
	ActScript* pActScript = NULL;
	m_mapActScript.reset_iterator();
	while( m_mapActScript.find_next(pActScript) )
	{
		SAFE_DELETE(pActScript);
	}
	m_mapActScript.clear();

	// 清空物品脚本
	ItemScript* pItemScript = NULL;
	m_mapItemScript.reset_iterator();
	while( m_mapItemScript.find_next(pItemScript) )
	{
		SAFE_DELETE(pItemScript);
	}
	m_mapItemScript.clear();

	// 清空技能脚本
	SkillScript* pSkillScript = NULL;
	m_mapSkillScript.reset_iterator();
	while( m_mapSkillScript.find_next(pSkillScript) )
	{
		SAFE_DELETE(pSkillScript);
	}
	m_mapSkillScript.clear();


	// 清空buff脚本
	BuffScript* pBuffScript = NULL;
	m_mapBuffScript.reset_iterator();
	while( m_mapBuffScript.find_next(pBuffScript) )
	{
		SAFE_DELETE(pBuffScript);
	}
	m_mapBuffScript.clear();

	// 清空玩家脚本
	if( VALID_POINT(m_pRoleScript) )
	{
		SAFE_DELETE(m_pRoleScript);
	}

	// 清空世界脚本
	if( VALID_POINT(m_pWorldScript) )
	{
		SAFE_DELETE(m_pWorldScript);
	}

	if ( VALID_POINT(m_pRankScript))
	{
		SAFE_DELETE(m_pRankScript);
	}

}

//-----------------------------------------------------------------------------------
// 卸载所有脚本
//-----------------------------------------------------------------------------------
VOID ScriptMgr::UnloadScripts()
{
	// 重置任务脚本
	quest_script* pQuestScript = NULL;
	m_mapQusetScript.reset_iterator();
	while( m_mapQusetScript.find_next(pQuestScript) )
	{
		pQuestScript->Destroy();
	}

	// 重置怪物脚本
	CreatureScript* pCreatureScript = NULL;
	m_mapCreatureScript.reset_iterator();
	while( m_mapCreatureScript.find_next(pCreatureScript) )
	{
		pCreatureScript->Destroy();
	}

	// 重置地图脚本
	MapScript* pMapScript = NULL;
	m_mapMapScript.reset_iterator();
	while( m_mapMapScript.find_next(pMapScript) )
	{
		pMapScript->Destroy();
	}

	// 重置活动脚本
	ActScript* pActScript = NULL;
	m_mapActScript.reset_iterator();
	while( m_mapActScript.find_next(pActScript) )
	{
		pActScript->Destroy();
	}

	// 重置物品脚本
	ItemScript* pItemScript = NULL;
	m_mapItemScript.reset_iterator();
	while( m_mapItemScript.find_next(pItemScript) )
	{
		pItemScript->Destroy();
	}

	// 重置技能脚本
	SkillScript* pSkillScript = NULL;
	m_mapSkillScript.reset_iterator();
	while( m_mapSkillScript.find_next(pSkillScript) )
	{
		pSkillScript->Destroy();
	}

	// 清空buff脚本
	BuffScript* pBuffScript = NULL;
	m_mapBuffScript.reset_iterator();
	while( m_mapBuffScript.find_next(pBuffScript) )
	{
		pBuffScript->Destroy();
	}

	// 重置玩家脚本
	if( VALID_POINT(m_pRoleScript) )
	{
		m_pRoleScript->Destroy();
	}

	// 清空世界脚本
	if( VALID_POINT(m_pWorldScript) )
	{
		m_pWorldScript->Destroy();
	}

	if (VALID_POINT(m_pRankScript))
	{
		m_pRankScript->Destroy();
	}

}

//-----------------------------------------------------------------------------------
// 卸载所有的State相关内容
//-----------------------------------------------------------------------------------
VOID ScriptMgr::DestroyAllStates()
{
	// 删除所有的线程状态
	std::list<DWORD> listThreadID;
	m_mapThreadState.copy_key_to_list(listThreadID);

	for( std::list<DWORD>::iterator it = listThreadID.begin(); it != listThreadID.end(); ++it )
	{
		DWORD dwThreadID = *it;

		char szBuf[64] = {0};
		_ultoa_s(dwThreadID, szBuf, 10);

		lua_pushnil(m_pMasterState);
		lua_setfield(m_pMasterState, LUA_REGISTRYINDEX, szBuf);
	}
	m_mapThreadState.clear();

	// 删除主state
	lua_close(m_pMasterState);

	// 删除所有脚本锁
	package_map<DWORD, Mutex*>::map_iter it = m_mapScriptMutex.begin();
	Mutex* pMutex = NULL;

	while( m_mapScriptMutex.find_next(it, pMutex) )
	{
		SAFE_DELETE(pMutex);
	}
	m_mapScriptMutex.clear();
}

//-----------------------------------------------------------------------------------
// 得到一个可用的thread
//-----------------------------------------------------------------------------------
lua_State* ScriptMgr::GetThreadState()
{
	//DWORD dwThreadID = GetCurrentThreadId();

	//lua_State* pThreadState = m_mapThreadState.find(dwThreadID);

	//if( !VALID_POINT(pThreadState) )
	//{
	//	pThreadState = CreateThreadState(dwThreadID);
	//	ASSERT( VALID_POINT(pThreadState) );
	//}

	//return pThreadState;

	return m_pMasterState;
}

//-----------------------------------------------------------------------------------
// 生成一个新的thread
//-----------------------------------------------------------------------------------
lua_State* ScriptMgr::CreateThreadState(DWORD dwThreadID)
{
	print_message(_T("create thread state, thread is %u\r\n"), dwThreadID);

	m_Lock.Acquire();

	lua_State* pThreadState = lua_newthread(m_pMasterState);

	// 将新的state写入注册表，防止被垃圾收集
	char szBuf[64] = {0};
	_ultoa_s(dwThreadID, szBuf, 10);
	lua_setfield(m_pMasterState, LUA_REGISTRYINDEX, szBuf);

	// 将新的state加入到map中
	m_mapThreadState.add(dwThreadID, pThreadState);

	m_Lock.Release();

	return pThreadState;
}

//-----------------------------------------------------------------------------------
// 错误屏显
//-----------------------------------------------------------------------------------
VOID ScriptMgr::ErrMsg(lua_State* pState)
{
	print_message(_T("Script Error: \r\n"));
	print_message(_T("%s\r\n"), get_tool()->ansi_to_unicode(lua_tostring(pState, -1)));
	lua_pop(pState, 1);
}

//-----------------------------------------------------------------------------------
// 通用脚本调用函数（目前还不能返回字符串）
//-----------------------------------------------------------------------------------
VOID ScriptMgr::CallScriptFunction(CHAR* szFunction, CHAR* szFormat, ...)
{
	if( !VALID_POINT(szFunction) ) return;

	lua_State* pThreadState = GetThreadState();
	if( !VALID_POINT(pThreadState) ) return;

	m_Lock.Acquire();

	DWORD dwStartTime = timeGetTime();

	lua_getglobal(pThreadState, szFunction);
	if( lua_isnil(pThreadState, -1) )
	{
		print_message(_T("Script Error: unknown function \r\n"));
		print_message(_T("%s\r\n"), get_tool()->ansi_to_unicode(szFunction));
		lua_pop(pThreadState, 1);

		m_Lock.Release();
		return;
	}

	va_list vl;
	INT nArg = 0, nRes = 0;		// 参数和结果的数量

	va_start(vl, szFormat);

	// 压入参数
	BOOL bArgEnd = FALSE;
	for(nArg = 0; *szFormat && FALSE == bArgEnd; ++nArg)
	{
		luaL_checkstack(pThreadState, 1, "too many arguments");

		switch(*szFormat++)
		{
		case 'd':	// double参数
			{
				double d = va_arg(vl, double);
				lua_pushnumber(pThreadState, d);
			}
			break;

		case 'f':	// float参数
			{
				double f = va_arg(vl, double);
				lua_pushnumber(pThreadState, f);
			}
			break;

		case 'u':	// dword参数
			{
				unsigned int u = va_arg(vl, unsigned int);
				lua_pushnumber(pThreadState, u);
			}
			break;

		case 'i':	// int参数
			{
				int i = va_arg(vl, int);
				lua_pushinteger(pThreadState, i);
			}
			break;

		case 's':	// 字符串参数
			{
				char* c = (char*)va_arg(vl, unsigned int);
				lua_pushstring(pThreadState, c);
			}
			break;

		case '>':	// 参数结束
			bArgEnd = TRUE;
			goto _arg_end; 

		default:
			break;
		}
	}

_arg_end:

	// 调用函数
	nRes = strlen(szFormat);
	if( lua_pcall(pThreadState, nArg, nRes, 0) != 0 )
	{
		ErrMsg(pThreadState);

		m_Lock.Release();
		return;
	}

	INT n_num = lua_gettop(pThreadState);

	// 检索结果
	INT nIndex = -nRes;

	while(*szFormat)
	{
		switch(*szFormat++)
		{
		case 'd':		// double结果
			*va_arg(vl, double*) = lua_tonumber(pThreadState, nIndex);
			break;

		case 'f':		// float结果
			*va_arg(vl, float*)  = lua_tonumber(pThreadState, nIndex);
			break;

		case 'u':		// dword结果
			*va_arg(vl, unsigned int*) = lua_tonumber(pThreadState, nIndex);
			break;

		case 'i':		// int结果
			*va_arg(vl, int*)  = lua_tointeger(pThreadState, nIndex);
			break;

		case 'b':		// BOOL结果
			*va_arg(vl, BOOL*)  = lua_toboolean(pThreadState, nIndex);
			break;

		default:
			break;
		}

		++nIndex;
	}

	lua_pop(pThreadState, nRes);

	// 结束
	va_end(vl);

	DWORD dwEndTime = timeGetTime();

	//if(m_dwMaxPcallTime < (dwEndTime - dwStartTime) )
	{
		m_dwMaxPcallTime = dwEndTime - dwStartTime;
		if(m_dwMaxPcallTime > 0)
		{
			g_world.get_log()->write_log(_T("Max calling time script function is <%s>, useing %d millisecond.\r\n"), get_tool()->unicode8_to_unicode(szFunction), m_dwMaxPcallTime);
			//PRINT_MESSAGE(_T("Max calling time script function is <%s>, useing %d millisecond.\r\n"), get_tool()->unicode8_to_unicode(szFunction), m_dwMaxPcallTime);
		}
	}

	m_Lock.Release();
}

//-----------------------------------------------------------------------------------
// 注册怪物AI脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterCreatureEvent(DWORD dwID, EScriptCreatureEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_VALUE(dwID) && ( eEvent >= 0 && eEvent < ESCAE_End ) && VALID_POINT(szFunction) );

	// 首先查找该怪物是否已经有相应的脚本了，如果没有就生成
	CreatureScript* pScript = m_mapCreatureScript.find(dwID);

	if( !VALID_POINT(pScript) )
	{
		pScript = new CreatureScript;
		m_mapCreatureScript.add(dwID, pScript);
	}

	// 注册
	pScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册玩家脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterRoleEvent(EScriptRoleEvent eEvent, const CHAR* szFunction)
{
	ASSERT( (eEvent >= 0 && eEvent < ESRE_End) && VALID_POINT(szFunction) );

	// 首先查找该玩家是否已经有相应的脚本了，如果没有就生成
	if( !VALID_POINT(m_pRoleScript) )
	{
		m_pRoleScript = new RoleScript;
	}

	// 注册
	m_pRoleScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册地图脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterMapEvent(const CHAR* szMapName, EScriptMapEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_POINT(szMapName) && (eEvent >= 0 && eEvent < ESME_End) && VALID_POINT(szFunction) );

	DWORD dwMapID = get_tool()->crc32(get_tool()->unicode8_to_unicode(szMapName));

	// 如果没有地图脚本就生成
	MapScript* pScript = m_mapMapScript.find(dwMapID);
	if( !VALID_POINT(pScript) )
	{
		pScript = new MapScript;
		m_mapMapScript.add(dwMapID, pScript);
	}

	// 注册
	pScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册任务脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterQuestEvent(UINT16 u16ID, EScriptQuestEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_VALUE(u16ID) && ( eEvent >= 0 && eEvent < ESQE_End ) && VALID_POINT(szFunction) );

	// 首先查找该任务是否已经有相应的脚本了，如果没有就生成
	quest_script* pScript = m_mapQusetScript.find(u16ID);
	if( !VALID_POINT(pScript) )
	{
		pScript = new quest_script;
		m_mapQusetScript.add(u16ID, pScript);
	}

	// 注册
	pScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册固定活动脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterActEvent(DWORD dwActID, EScriptActEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_VALUE(dwActID) && ( eEvent >= 0 && eEvent < ESAE_End ) && VALID_POINT(szFunction) );

	// 首先查找该活动是否已经有相应的脚本了，如果没有就生成
	ActScript* pScript = m_mapActScript.find(dwActID);
	if( !VALID_POINT(pScript) )
	{
		pScript = new ActScript;
		m_mapActScript.add(dwActID, pScript);
	}

	// 注册
	pScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册游戏世界脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterWorldEvent(EScriptWorldEvent eEvent, const CHAR* szFunction)
{
	ASSERT(( eEvent >= 0 && eEvent < ESWE_End ) && VALID_POINT(szFunction) );

	if( !VALID_POINT(m_pWorldScript) )
	{
		m_pWorldScript = new WorldScript;
	}

	m_pWorldScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册游戏世界脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterItemEvent(DWORD dw_data_id, EScriptItemEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_VALUE(dw_data_id) && ( eEvent >= 0 && eEvent < ESIE_End ) && VALID_POINT(szFunction) );

	ItemScript* pScript = m_mapItemScript.find(dw_data_id);
	if( !VALID_POINT(pScript) )
	{
		pScript = new ItemScript;
		m_mapItemScript.add(dw_data_id, pScript);
	}

	pScript->RegisterFunc(eEvent, szFunction);
}

//-----------------------------------------------------------------------------------
// 注册技能脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterSkillEvent(DWORD dw_data_id, EScriptSkillEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_VALUE(dw_data_id) && ( eEvent >= 0 && eEvent < ESSE_End ) && VALID_POINT(szFunction) );

	SkillScript* pScript = m_mapSkillScript.find(dw_data_id);
	if( !VALID_POINT(pScript) )
	{
		pScript = new SkillScript;
		m_mapSkillScript.add(dw_data_id, pScript);
	}

	pScript->RegisterFunc(eEvent, szFunction);
}
//-----------------------------------------------------------------------------------
// 注册buff脚本函数
//-----------------------------------------------------------------------------------
VOID ScriptMgr::RegisterBuffEvent(DWORD dw_data_id, EScriptBuffEvent eEvent, const CHAR* szFunction)
{
	ASSERT( VALID_VALUE(dw_data_id) && ( eEvent >= 0 && eEvent < ESBE_End ) && VALID_POINT(szFunction) );

	BuffScript* pScript = m_mapBuffScript.find(dw_data_id);
	if( !VALID_POINT(pScript) )
	{
		pScript = new BuffScript;
		m_mapBuffScript.add(dw_data_id, pScript);
	}

	pScript->RegisterFunc(eEvent, szFunction);
}

// 注册排行榜脚本函数
VOID ScriptMgr::RegisterRankEvent(EScriptRankEvent eEvent, const CHAR* szFunction)
{
	ASSERT( ( eEvent >= 0 && eEvent < ESRKE_End ) && VALID_POINT(szFunction) );

	if (!VALID_POINT(m_pRankScript))
	{
		m_pRankScript = new RankScript;
	}

	m_pRankScript->RegisterFunc(eEvent, szFunction);
}

DWORD ScriptMgr::GetMemcory()
{
	if (VALID_POINT(m_pMasterState))
	{
		return lua_gc(m_pMasterState, LUA_GCCOUNT,0)*1024 + lua_gc(m_pMasterState, LUA_GCCOUNTB, 0);
	}

	return 0;
}

//---------------------------------------------------------------------------------------
// 任务脚本对象的相关事件函数
//---------------------------------------------------------------------------------------
INT  quest_script::check_accept(UINT16 quest_id, Role* p_role, Creature* p_npc, INT& tErrorCode) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_CheckAccept]) ) return TRUE;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return FALSE;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();
	DWORD dw_npc = VALID_POINT(p_npc) ? p_npc->GetID() : INVALID_VALUE;

	INT n_ret = 0;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_CheckAccept], "uuiuu>ib", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_npc, &tErrorCode, &n_ret);

	return n_ret;
}

INT  quest_script::check_complete(UINT16 quest_id, Role* p_role, Creature* p_npc, INT& tErrorCode) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_CheckComplete]) ) return TRUE;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return FALSE;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();
	DWORD dw_npc = VALID_POINT(p_npc) ? p_npc->GetID() : INVALID_VALUE;

	INT n_ret = 0;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_CheckComplete], "uuiuu>ib", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_npc, &tErrorCode, &n_ret);

	return n_ret;
}

VOID quest_script::on_init(UINT16 quest_id, Role* p_role) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_Init]) ) return;

	Map* p_map = p_role->get_map();
	if(!VALID_POINT(p_map))	return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Init], "uuiu", dw_map_id,dw_instance_id, quest_id, p_role->GetID());
}

VOID quest_script::on_accept(UINT16 quest_id, Role* p_role, Creature* p_npc) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_Accept]) ) return;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();
	DWORD dw_npc = VALID_POINT(p_npc) ? p_npc->GetID() : INVALID_VALUE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Accept], "uuiuu", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_npc);
}

VOID quest_script::on_complete(UINT16 quest_id, Role* p_role, Creature* p_npc) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_Complete]) ) return;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();
	DWORD dw_npc = VALID_POINT(p_npc) ? p_npc->GetID() : INVALID_VALUE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Complete], "uuiuu", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_npc);
}

VOID quest_script::on_cancel(UINT16 quest_id, Role* p_role) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_Cancel]) ) return;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Cancel], "uuiu", dw_map_id, dw_instance_id, quest_id, p_role->GetID());
}

VOID quest_script::on_creature_kill(UINT16 quest_id, Role* p_role, DWORD dw_creature_type, BOOL bAddExp) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_Creature_Kill]) ) return;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Creature_Kill], "uuiuuu", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_creature_type, (DWORD)bAddExp);
}

VOID quest_script::on_npc_talk(UINT16 quest_id, Role* p_role, DWORD dw_npc, DWORD dw_npc_type) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_NPC_Talk]) ) return;

	Map* p_map = p_role->get_map();
	if( !VALID_POINT(p_map) ) return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_NPC_Talk], "uuiuuu", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_npc, dw_npc_type);
}

VOID quest_script::on_default_dialog(UINT16 quest_id, Role* p_role, DWORD option) const
{
	if( !VALID_POINT(m_szFunc[ESQE_On_Dlg_Default]) ) return;

	Map* p_map = p_role->get_map();
	if(!VALID_POINT(p_map)) return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Dlg_Default], "uuiuu", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), option);
}

VOID quest_script::on_invest(UINT16 quest_id, Role* p_role, DWORD dw_creature_type) const
{
	if(!VALID_POINT(m_szFunc[ESQE_On_Invest])) return;

	Map* p_map = p_role->get_map();
	if(!VALID_POINT(p_map))	return;

	DWORD dw_map_id = p_map->get_map_id();
	DWORD dw_instance_id = p_map->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESQE_On_Invest], "uuiuu", dw_map_id, dw_instance_id, quest_id, p_role->GetID(), dw_creature_type);
}
//-----------------------------------------------------------------------------------------
// 怪物脚本对象的相关事件函数
//-----------------------------------------------------------------------------------------
VOID CreatureScript::OnLoad(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Load]) ) return;

	Map* pMap = pCreature->get_map();
	if( !VALID_POINT(pMap) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Load], "uuu", dwMapID, dwInstanceID, pCreature->GetID());
}

VOID CreatureScript::OnTalk(Creature* pCreature, Role* pRole, INT nIndex) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Talk]) ) return;

	Map* pMap = pCreature->get_map();
	if( !VALID_POINT(pMap) ) return;
	
	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Talk], "uuuuui", dwMapID, dwInstanceID, pCreature->GetID(), pCreature->GetTypeID(), pRole->GetID(), nIndex);
}

VOID CreatureScript::OnRespawn(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Respawn]) ) return;

	Map* pMap = pCreature->get_map();
	if( !VALID_POINT(pMap) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Respawn], "uuu", dwMapID, dwInstanceID, pCreature->GetID());
}

VOID CreatureScript::OnEnterCombat(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Enter_Combat]) ) return;

	Map* pMap = pCreature->get_map();
	if( !VALID_POINT(pMap) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Enter_Combat], "uuu", dwMapID, dwInstanceID, pCreature->GetID());
}

VOID CreatureScript::OnLeaveCombat(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Leave_Combat]) ) return;

	Map* pMap = pCreature->get_map();
	if( !VALID_POINT(pMap) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Leave_Combat], "uuu", dwMapID, dwInstanceID, pCreature->GetID());
}


VOID CreatureScript::OnDie(Creature* pCreature, Unit* pKiller, DWORD dwTaggedOwner) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Die]) ) return;

	Map* pMap = pCreature->get_map();
	if( !VALID_POINT(pMap) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();
	DWORD dwKillerID = VALID_POINT(pKiller) ? pKiller->GetID() : INVALID_VALUE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Die], "uuuuuu", dwMapID, dwInstanceID, pCreature->GetID(), pCreature->GetTypeID(), dwKillerID, dwTaggedOwner);
}

VOID CreatureScript::OnInvest(Creature* pCreature, Role* pScr) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_Invest])) return;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_Invest], "uuuuu", dwMapID, dwInstanceID, pCreature->GetID(), pCreature->GetTypeID(), pScr->GetID());
}

VOID CreatureScript::OnRangeEvent(Creature* pCreature, DWORD dwEventType) const
{
	if (!VALID_POINT(m_szFunc[ESCAE_On_RangeEvent]))	return;

	Map* pMap = pCreature->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_RangeEvent], "uuuu", dwMapID, dwInstanceID, pCreature->GetID(), dwEventType);

	
}

VOID CreatureScript::OnBeHelp(Creature* pSrc, Creature* pHelp) const
{
	if(!VALID_POINT(m_szFunc[ESCAE_On_BeHelp]))		return;

	Map* pMap = pSrc->get_map();
	if(!VALID_POINT(pMap)) return;

	pMap = pHelp->get_map();
	if(!VALID_POINT(pMap)) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_BeHelp], "uuuu", dwMapID, dwInstanceID, pSrc->GetID(), pHelp->GetID());
}

VOID CreatureScript::OnUpdateAI(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_UpdateAI]) ) return;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();
	
	// 好大头的log
	//if (pCreature->GetProto()->dw_data_id == 3020023 && pCreature->IsMonster())
	//{
	//	static DWORD dw_time = timeGetTime();
	//	DWORD dw_new_time = timeGetTime();
	//	if(dw_new_time - dw_time > 5 * 60 * 1000)
	//	{
	//		g_world.get_log()->write_log(_T("Creature 3020023 script run!!!!!!\r\n"));
	//		dw_time = timeGetTime();
	//	}
	//}

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_UpdateAI], "uuuu", dwMapID, dwInstanceID, pCreature->GetID(), pAI->GetCurrentStateType());
}

VOID CreatureScript::OnUpdateCurAI(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_UpdateCurAI]) ) return;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_UpdateCurAI], "uuuu", dwMapID, dwInstanceID, pCreature->GetID(), pAI->GetCurrentStateType());
}

VOID CreatureScript::OnEnterCurAI(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_EnterCurAI]) ) return;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_EnterCurAI], "uuuu", dwMapID, dwInstanceID, pCreature->GetID(), pAI->GetCurrentStateType());
}

VOID CreatureScript::OnLeaveCurAI(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_LeaveCurAI]) ) return;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_LeaveCurAI], "uuuu", dwMapID, dwInstanceID, pCreature->GetID(), pAI->GetCurrentStateType());
}

BOOL CreatureScript::OnEventCurAI(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_EventCurAI]) ) return FALSE;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return FALSE;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return FALSE;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	BOOL bRet = FALSE;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_EventCurAI], "uuuu>i", dwMapID, dwInstanceID, pCreature->GetID(), pAI->GetCurrentStateType(), bRet);

	return bRet;
}


//  增加怪物导航点事件
BOOL  CreatureScript::OnArrivalPoint(Creature* pCreature, int nNode ) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_ReachPoint]) ) return FALSE;


	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return FALSE;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return FALSE;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	BOOL bRet = FALSE;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_ReachPoint], "uuuu", dwMapID, dwInstanceID, nNode, pCreature->GetID());

	return bRet;
}

VOID CreatureScript::OnReachEndPath(Creature* pCreature) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_ReachEndPath]) ) return;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_ReachEndPath], "uuu", dwMapID, dwInstanceID, pCreature->GetID());
}

INT CreatureScript::OnBeAttack(Creature* pCreature, DWORD dwSkillID, DWORD dwSkillLevel, BOOL bBlock, BOOL bCrited, INT nDmg) const
{
	if( !VALID_POINT(m_szFunc[ESCAE_On_BeAttack]) ) return nDmg;

	Map* pMap = pCreature->get_map();
	if(!VALID_POINT(pMap))	return nDmg;

	AIController* pAI = pCreature->GetAI();
	if( !VALID_POINT(pAI) ) return nDmg;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	INT dmg = 0;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESCAE_On_BeAttack], "uuuuuuui>i", dwMapID, dwInstanceID, pCreature->GetID(), dwSkillID, dwSkillLevel, bBlock, bCrited, nDmg, &dmg);
	return dmg;
}
//-------------------------------------------------------------------------------------------
// 玩家脚本
//-------------------------------------------------------------------------------------------
VOID RoleScript::OnRoleOnline(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_On_Online]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Online], "u", pRole->GetID());
}

VOID RoleScript::OnRoleFirstOnline(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_On_FirstOnline]) ) return;
	
	if (!VALID_POINT(pRole->GetSession())) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_FirstOnline], "u", pRole->GetID());
}

VOID RoleScript::OnRoleIntoWorld(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_On_IntoWorld])) return;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap) )	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_IntoWorld], "uuui", dwMapID, dwInstanceID, pRole->GetID(), pRole->GetSession()->IsRobort());
}

VOID RoleScript::OnRoleFirstIntoWorld(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_On_FirstIntoWorld])) return;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap) )	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_FirstIntoWorld], "uuui", dwMapID, dwInstanceID, pRole->GetID(), pRole->GetSession()->IsRobort());
}

VOID RoleScript::OnRoleEnterMap(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_On_EnterMap])) return;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap) )	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_EnterMap], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

VOID RoleScript::OnRoleLevelChange(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_On_LevelChange])) return;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap) )	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_LevelChange], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

INT RoleScript::IsDeadPenalty(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESRE_IsDeadPenalty]) ) return TRUE;
	INT  nRet = 1;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap))		return TRUE;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_IsDeadPenalty], "uuu>i", dwMapID, dwInstanceID, pRole->GetID(), &nRet);

	return nRet;
}

VOID RoleScript::OnOpenChest(Role* pRole, DWORD dwChestID, DWORD dwKeyID) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_OpenChest]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_OpenChest], "uuuuu", dwMapID, dwInstanceID, pRole->GetID(), dwChestID, dwKeyID);
}

VOID RoleScript::OnStopChest(Role* pRole, DWORD dwChestID, DWORD dwKeyID) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_StopChest]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_StopChest], "uuuuu", dwMapID, dwInstanceID, pRole->GetID(), dwChestID, dwKeyID);
}

VOID RoleScript::OnAgainChest(Role *pRole) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_AgainChest]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_AgainChest], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

VOID RoleScript::OnGetItem(Role* pRole, DWORD dwItemID, INT n_num) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_AgainChest]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_GetItem], "uuuui", dwMapID, dwInstanceID, pRole->GetID(), dwItemID, n_num);
}

VOID RoleScript::OnEquip(Role* pRole, INT nPos, DWORD dw_data_id) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_Equip]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Equip], "uuuiu", dwMapID, dwInstanceID, pRole->GetID(), nPos, dw_data_id);
}	

VOID RoleScript::OnUnEquip(Role* pRole, INT nPos, DWORD dw_data_id) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_UnEquip]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_UnEquip], "uuuiu", dwMapID, dwInstanceID, pRole->GetID(), nPos, dw_data_id);
}

VOID RoleScript::OnRangeEvent(Role* pRole, DWORD dwEventType) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_RangeEvent]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_RangeEvent], "uuuu", dwMapID, dwInstanceID, pRole->GetID(), dwEventType);

}

VOID RoleScript::OnAddFriend( Role* pRole, DWORD dwFriendID ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_AddFriend]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_AddFriend], "uuuu", dwMapID, dwInstanceID, pRole->GetID(), dwFriendID);
}

FLOAT RoleScript::OnCalFriendExp(Role* pRole, DWORD dwFriendVal) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_CalFriendExp])) return 0.0f;

	FLOAT fRes = 0.0f;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return fRes;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_CalFriendExp], "uuuu>f", dwMapID, dwInstanceID, pRole->GetID(), dwFriendVal, &fRes);

	return fRes;

}

VOID RoleScript::OnWeaponLevelUp( Role* pRole, DWORD dwLevel ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_WeaponLevelUp]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_WeaponLevelUp], "uuuu", dwMapID, dwInstanceID, pRole->GetID(), dwLevel );
}

VOID RoleScript::OnWeaponFusion( Role* pRole ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_WeapFusion]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_WeapFusion], "uuu", dwMapID, dwInstanceID, pRole->GetID() );
}

VOID RoleScript::OnEquipConsolidate( Role* pRole, DWORD dwLevel ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_EquipConsolidate]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_EquipConsolidate], "uuuu", dwMapID, dwInstanceID, pRole->GetID(), dwLevel );
}

VOID RoleScript::OnEquipDestroy( Role* pRole ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_EquipDestroy]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_EquipDestroy], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

VOID RoleScript::OnMakeMaster( Role* pRole, DWORD dwMaster, DWORD dwPrentice ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_MakeMaster]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_MakeMaster], "uuuu", dwMapID, dwInstanceID, dwPrentice, dwMaster );
}

VOID RoleScript::OnOpenRank( Role* pRole ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_OpenRank]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_OpenRank], "uuu", dwMapID, dwInstanceID, pRole->GetID( ) );
}

VOID RoleScript::OnPutOutXSQuest( Role* pRole, DWORD dwQuestID ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_PutOutXSQuest]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_PutOutXSQuest], "uuuu", dwMapID, dwInstanceID, pRole->GetID( ), dwQuestID );
}

VOID RoleScript::OnJoinGuild( Role* pRole, DWORD dwGuildID ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_JoinGuild]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_JoinGuild], "uuuu", dwMapID, dwInstanceID, pRole->GetID( ), dwGuildID );
}

VOID RoleScript::OnCreateGuild( Role* pRole, DWORD dwGuildID ) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_CreateGuild]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_CreateGuild], "uuuu", dwMapID, dwInstanceID, pRole->GetID( ), dwGuildID );
}
VOID RoleScript::OnLearnSkill( Role* pRole, DWORD dwSkillID, INT nLevel ) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_LearnSkill]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_LearnSkill], "uuuui", dwMapID, dwInstanceID, pRole->GetID( ), dwSkillID, nLevel);
}

VOID RoleScript::OnLearnGodSkill( Role* pRole, DWORD dwSkillID ) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_LearnGodSkill]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_LearnGodSkill], "uuuu", dwMapID, dwInstanceID, pRole->GetID( ), dwSkillID);

}

VOID RoleScript::OnLianhuen(Role* pRole) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_lianhuen]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_lianhuen], "uuu", dwMapID, dwInstanceID, pRole->GetID( ));

}

VOID RoleScript::OnSonghua(Role* pRole,INT16 nSendFlowers) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_songhua]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_songhua], "uuui", dwMapID, dwInstanceID, pRole->GetID( ),nSendFlowers);

}

VOID RoleScript::OnShuangxiu(Role* pRole) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_shuangxiu]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_shuangxiu], "uuu", dwMapID, dwInstanceID, pRole->GetID( ));

}

VOID RoleScript::OnShangxiang(Role* pRole) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_shangxiang]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_shangxiang], "uuu", dwMapID, dwInstanceID, pRole->GetID( ));

}

VOID RoleScript::OnLottery(Role* pRole) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_Lottery]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Lottery], "uuu", dwMapID, dwInstanceID, pRole->GetID( ));

}

VOID RoleScript::OnSBKLottery(Role* pRole, int nPos) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_SBK_Reward]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_SBK_Reward], "uuui", dwMapID, dwInstanceID, pRole->GetID( ), nPos);
}

VOID RoleScript::OnRecharge(Role* pRole, int Recharge, int nTotleRecharge) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_Recharge]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Recharge], "uuuii", dwMapID, dwInstanceID, pRole->GetID( ), Recharge, nTotleRecharge);
	
}

VOID RoleScript::OnGetOpenReceive(Role* pRole, int nType, int bFirst) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_Get_Open_Receive]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Get_Open_Receive], "uuuii", dwMapID, dwInstanceID, pRole->GetID( ), nType, bFirst);
}

VOID RoleScript::OnGetBattleGift(Role* pRole, BOOL bWin, int nRank) const 
{
	if (!VALID_POINT(m_szFunc[ESRE_On_Get_Battle_Gift]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Get_Battle_Gift], "uuuii", dwMapID, dwInstanceID, pRole->GetID( ), bWin, nRank);

}

VOID RoleScript::OnInstanceSaodang(Role* pRole, int nIndex) const
{
	if (!VALID_POINT(m_szFunc[ESME_On_Instance_SaoDang]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_Instance_SaoDang], "uuui", dwMapID, dwInstanceID, pRole->GetID( ), nIndex);

}

VOID RoleScript::OnRideConsolidate( Role* pRole, INT nLevel ) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_RideConsolidate]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_RideConsolidate], "uuui", dwMapID, dwInstanceID, pRole->GetID( ), nLevel);
}
VOID RoleScript::OnProduceEquip(Role* pRole, DWORD dwTypeID, DWORD dwQuality, BOOL bBox) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_ProduceEquip]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_ProduceEquip], "uuuuuu", dwMapID, dwInstanceID, pRole->GetID(), dwTypeID, dwQuality, bBox );
}

VOID RoleScript::OnProduceItem(Role* pRole, DWORD dwTypeID, BOOL bSecc, BOOL bBox) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_ProdueceItem]))	return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_ProdueceItem], "uuuuuu", dwMapID, dwInstanceID, pRole->GetID(), dwTypeID, bSecc, bBox );

}
VOID RoleScript::OnDeComposeItem(Role* pRole, DWORD dwTypeID) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_DeCompose])) return;
	
	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap)) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_DeCompose], "uuuu", dwMapID, dwInstanceID, pRole->GetID(), dwTypeID);
}

BOOL RoleScript::OnVigourReward(Role* pRole, DWORD dwVigourCost, DWORD dwFirstLogin) const
{
	if (!VALID_POINT(m_szFunc[ESRE_On_VigourReward]))	return FALSE;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return FALSE;

	BOOL bSendMailOk = FALSE;
	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_VigourReward], "uuuuu>i", dwMapID, dwInstanceID, pRole->GetID(), dwVigourCost, dwFirstLogin, &bSendMailOk );

	return bSendMailOk;
}

VOID RoleScript::OnJoinRecruit(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_JoinRecruit])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_JoinRecruit], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

VOID RoleScript::OnJoinMasterRecruit(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_JoinMasterRecruit])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_JoinMasterRecruit], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

VOID RoleScript::OnGuildTrunOver(Role* pRole, Role* pNewRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_Guild_TrunOver])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	Map* pNewMap = pNewRole->get_map();
	if(!VALID_POINT(pNewMap)) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	DWORD dwNewMapID = pNewMap->get_map_id();
	DWORD dwNewInstanceID = pNewMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Guild_TrunOver], "uuuuuu", dwMapID, dwInstanceID, pRole->GetID(), dwNewMapID, dwNewInstanceID, pNewRole->GetID());
}

VOID RoleScript::OnInlayEquip(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_InlayEquip])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_InlayEquip], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}
VOID RoleScript::OnBindEquip(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_BindEquip])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_BindEquip], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}
VOID RoleScript::OnRonghePet(Role* pRole, DWORD dwQuality, BOOL bAdd) const
{

	if(!VALID_POINT(m_szFunc[ESRE_On_RonghePet])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_RonghePet], "uuuiu", dwMapID, dwInstanceID, pRole->GetID(), bAdd, dwQuality);
}

VOID RoleScript::OnSetRebornMap(Role* pRole, DWORD dwMapID, DWORD dwInstanceID) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_SetRebornMap])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_SetRebornMap], "uuu", dwMapID, dwInstanceID, pRole->GetID());
}

VOID RoleScript::OnReceiveAccountReward(Role* pRole, DWORD dwMapID, DWORD dwInstanceID, INT16 n16ReceiveType) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_ReceiveAccountReward])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_ReceiveAccountReward], "uuui", dwMapID, dwInstanceID, pRole->GetID(), n16ReceiveType);
}

VOID RoleScript::OnReceiveAccountRewardEx(Role* pRole, DWORD dwMapID, DWORD dwInstanceID, DWORD dwReceiveType) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_ReceiveAccountRewardEx])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_ReceiveAccountRewardEx], "uuuu", dwMapID, dwInstanceID, pRole->GetID(), dwReceiveType);
}

VOID RoleScript::OnSayGoodbyeToMaster(Role* pRole, DWORD dwMasterID, BOOL bGo) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_SayGoodbyeToMaster])) return;
	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_SayGoodbyeToMaster], "uuuuu", dwMapID, dwInstanceID, pRole->GetID(), dwMasterID, bGo);
}
VOID RoleScript::OnMasterPrenticeBreak(Role* pRole, DWORD dwMasterID, DWORD dwPrenticeID) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_MasterPrenticeBreak])) return;
	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_MasterPrenticeBreak], "uuuu", dwMapID, dwInstanceID, dwMasterID, dwPrenticeID);
}

VOID RoleScript::OnGet1v1Award(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_Get1v1Award])) return;
	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap)) return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Get1v1Award], "uuui", dwMapID, dwInstanceID, pRole->GetID(), pRole->get_1v1_score().n16_score_award);
}

VOID RoleScript::OnLeavePractice(Role* pRole, DWORD dwTime, DWORD dwLove, DWORD dwType) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_LeavePractice])) return;
	
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_LeavePractice], "uuuu", pRole->GetID(), dwTime, dwLove, dwType);
}

VOID RoleScript::OnChangeMap(Role* pRole, DWORD	dw_src_map_id,	DWORD dw_des_map_id, DWORD dw_instance_id) const
{
	if(!VALID_POINT(m_szFunc[ESME_ChangeMap])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_ChangeMap], "uuuu", pRole->GetID(), dw_src_map_id, dw_des_map_id, dw_instance_id);
}

VOID RoleScript::OnLeaveTimeReward(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_LeaveTimeReward])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_LeaveTimeReward], "u", pRole->GetID());
}

VOID RoleScript::OnLineCompensate(Role* pRole, DWORD nYear,	DWORD nMonth, DWORD nDay) const
{
	if(!VALID_POINT(m_szFunc[ESME_OnLineCompensate])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnLineCompensate], "uuuu", pRole->GetID(), nYear, nMonth, nDay);
}

VOID RoleScript::OnReceiveSerialReward(Role* pRole, INT n_type, DWORD& dw_result) const
{
	if(!VALID_POINT(m_szFunc[ESME_On_ReceiveSerialReward])) return;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_ReceiveSerialReward], "uuui>u", pRole->GetID(), dwMapID, dwInstanceID, n_type, &dw_result);
}

VOID RoleScript::OnFastCheck(Role* pRole, INT nType) const
{
	if(!VALID_POINT(m_szFunc[ESME_On_FastCheck])) return ;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_FastCheck], "uuui", dwMapID, dwInstanceID, pRole->GetID(), nType);
}

VOID RoleScript::OnTouPlant(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_On_TouPlant])) return ;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_TouPlant], "uuu", dwMapID, dwInstanceID, pRole->GetID());

}
VOID RoleScript::OnJuanMate(Role* pRole, INT nNum) const
{
	if(!VALID_POINT(m_szFunc[ESME_On_JuanMate])) return ;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_JuanMate], "uuui", dwMapID, dwInstanceID, pRole->GetID(), nNum);

}
VOID RoleScript::OnJuanMonery(Role* pRole, INT nNum) const
{
	if(!VALID_POINT(m_szFunc[ESME_On_JuanMonery])) return ;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_JuanMonery], "uuui", dwMapID, dwInstanceID, pRole->GetID(), nNum);

}

VOID RoleScript::OnPetSkillChange(Role* pRole, DWORD dwSkillID, BOOL bActive) const
{
	if(!VALID_POINT(m_szFunc[ESME_Pet_Skill_Change])) return ;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_Pet_Skill_Change], "uuuui", dwMapID, dwInstanceID, pRole->GetID(), dwSkillID, bActive);


}

VOID RoleScript::OnSign(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_sign])) return ;

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))	return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id( );

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_sign], "uuu", dwMapID, dwInstanceID, pRole->GetID());

}
DWORD RoleScript::OnActiveReceive(Role* pRole, INT nIndex) const
{
	if(!VALID_POINT(m_szFunc[ESME_ActiveReceive])) return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_ActiveReceive], "uuui>u", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), nIndex, &dw_error);

	return dw_error;
}

DWORD RoleScript::OnActiveDone(Role* pRole, INT nIndex,INT nbeishu) const
{
	if(!VALID_POINT(m_szFunc[ESME_ActiveDone])) return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_ActiveDone], "uuuii>u", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), nIndex,nbeishu, &dw_error);

	return dw_error;
}
//gx add 2013.12.18 每日活动一键传送
VOID RoleScript::OnDailyActTransmit(Role* pRole,INT nIndex) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_DailyAct_Transmit])) 
		return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return ;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_DailyAct_Transmit], "uuui", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), nIndex);

}
DWORD RoleScript::OnGuildActiveReceive(Role* pRole, INT nIndex) const
{
	if(!VALID_POINT(m_szFunc[ESME_GuildActiveReceive])) return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_GuildActiveReceive], "uuui>u", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), nIndex, &dw_error);

	return dw_error;
}

VOID RoleScript::OnConsumeYuanBao(Role* pRole, INT sum, DWORD dwCmdid) const
{
	if(!VALID_POINT(m_szFunc[ESME_RoleConsumeYuanbao])) return ;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap)) return ;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_RoleConsumeYuanbao], "uuuiu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), sum, dwCmdid);

}

VOID RoleScript::OnConsumeReward(Role *pRole, INT index) const
{
	if(!VALID_POINT(m_szFunc[ESME_RoleConsumeReward])) return ;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap)) return ;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_RoleConsumeReward], "uuui", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), index);

}

VOID RoleScript::ShowConsumeUI(Role *pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_RoleShowConsumeUI])) return ;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap)) return ;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_RoleShowConsumeUI], "uuui", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());
}

VOID RoleScript::OnEquipXiLi(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_EquipXiLi])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_EquipXiLi], "uuu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());

	return;
}

VOID RoleScript::OnJoinTeam(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_JoinTeam])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_JoinTeam], "uuu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());

	return;
}

VOID RoleScript::OnPetFeed(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_PetFeed])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_PetFeed], "uuu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());

	return;
}

VOID RoleScript::OnChatWorld(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_ChatWorld])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_ChatWorld], "uuu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());

	return;
}

VOID RoleScript::OnFishing(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESME_Fishing])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_Fishing], "uuu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());

	return;
}

VOID RoleScript::OnGetWuhuen(Role* pRole) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_getwuhuen])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_getwuhuen], "uuu", pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id());

	return;
}

VOID RoleScript::OnHang(Role* pRole, int type) const
{
	if(!VALID_POINT(m_szFunc[ESRE_On_Hang])) return;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRE_On_Hang], "uuui", 
		pRole->GetID(), pMap->get_map_id(), pMap->get_instance_id(), type);

	return;
}

//---------------------------------------------------------------------------------------------
// 地图脚本
//---------------------------------------------------------------------------------------------
VOID MapScript::OnInit(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnInit]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnInit], "uu", pMap->get_map_id(), pMap->get_instance_id());
}

VOID MapScript::OnTimer(Map* pMap, INT nMilliSec) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnTimer]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnTimer], "uui", pMap->get_map_id(), pMap->get_instance_id(), nMilliSec);
}

VOID MapScript::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanEnterWhenOnline]) ) return;

	// 统一脚本坐标为格子坐标
	vOut.x = vOut.x / TILE_SCALE;
	vOut.z = vOut.z / TILE_SCALE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanEnterWhenOnline], "uu>ufff", dwOutMapID,  pRole->GetID(), 
		&dwOutMapID, &vOut.x, &vOut.y, &vOut.z);
	// 统一脚本坐标为格子坐标
	vOut.x = vOut.x * TILE_SCALE;
	vOut.z = vOut.z * TILE_SCALE;
}

VOID MapScript::GetOnePerfectMap(Role* pRole, DWORD &dwInstanceID) const
{
	if( !VALID_POINT(m_szFunc[ESME_GetOnePerfectMap]) ) return;
	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap))   return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_GetOnePerfectMap], "uuu>u", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID(), &dwInstanceID);
}

INT MapScript::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut) const
{
	if( !VALID_POINT(m_szFunc[ESME_GetExportMapAndCoord]) ) return 0;
	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap))   return 0;

	INT nRet = 0;

	// 统一脚本坐标为格子坐标
	vOut.x = vOut.x / TILE_SCALE;
	vOut.z = vOut.z / TILE_SCALE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_GetExportMapAndCoord], "uuu>iufff", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID(), 
		&nRet, &dwOutMapID, &vOut.x, &vOut.y, &vOut.z);

	// 统一脚本坐标为格子坐标
	vOut.x = vOut.x * TILE_SCALE;
	vOut.z = vOut.z * TILE_SCALE;

	return nRet;
}

INT MapScript::CanEnter(Role* pRole) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanEnter]) ) return 0;
	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap))  return 0;

	INT nRet = 0;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanEnter], "uuu>i", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID(), &nRet);

	return nRet;
}

VOID MapScript::GuildWarRelay(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_GuildWarRelay]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_GuildWarRelay], "uu", pMap->get_map_id(), pMap->get_instance_id());
}

VOID MapScript::GuildWarStart(Map* pMap, DWORD dwdefentNumber, DWORD dwAttackNum, DWORD dwDefentID, DWORD dwAttackID) const
{
	if( !VALID_POINT(m_szFunc[ESME_GuildWarStart]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_GuildWarStart], "uuuuuu", pMap->get_map_id(), pMap->get_instance_id(), dwdefentNumber, dwAttackNum, dwDefentID, dwAttackID);
}

VOID MapScript::GuildWarEnd(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_GuildWarEnd]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_GuildWarEnd], "uu", pMap->get_map_id(), pMap->get_instance_id());
}

VOID MapScript::OnPlayerEnter(Role* pRole, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnPlayerEnter]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnPlayerEnter], "uuu", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID());
}

VOID MapScript::OnPlayerLeave(Role* pRole, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnPlayerLeave]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnPlayerLeave], "uuu", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID());
}

VOID MapScript::OnCreatureDie(Creature* pCreature, Unit* pKiller, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnCreatureDie]) ) return;
	if( !VALID_POINT(pCreature) ) return;

	DWORD dwKillerID = INVALID_VALUE;
	if( VALID_POINT(pKiller) ) dwKillerID = pKiller->GetID();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnCreatureDie], "uuuuu", pMap->get_map_id(), pMap->get_instance_id(), pCreature->GetID(), pCreature->GetTypeID(), dwKillerID);
}

VOID MapScript::OnCreatureDisappear(Creature* pCreature, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnCreatureDisappear]) ) return;
	if( !VALID_POINT(pCreature) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnCreatureDisappear], "uuuu", pMap->get_map_id(), pMap->get_instance_id(), pCreature->GetID(), pCreature->GetTypeID());
}

VOID MapScript::OnRoleDie(Role* pRole, Unit* pKiller, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnRoleDie]) ) return;
	if( !VALID_POINT(pRole) ) return;

	DWORD dwKillerID = INVALID_VALUE;
	if( VALID_POINT(pKiller) ) dwKillerID = pKiller->GetID();

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnRoleDie], "uuuu", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID(), dwKillerID);
}

BOOL MapScript::can_set_safe_guard(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_Safeguard]) ) return  TRUE;  
	BOOL bRet = TRUE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_Safeguard],"uu>b",pMap->get_map_id(), pMap->get_instance_id(), &bRet);

	return bRet;
}

BOOL MapScript::can_use_item(Map* pMap, DWORD dw_data_id, INT64 n64_Item_Serial) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanUseItem]) ) return  TRUE;  
	BOOL bRet = TRUE;

	const INT32 nMask	= 0xFFFFFFFF;

	INT32 n32High	= (INT32)((n64_Item_Serial >> 32) & nMask);
	INT32 n32Low	= (INT32)(n64_Item_Serial & nMask);


	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanUseItem],"uuuuu>b",pMap->get_map_id(), pMap->get_instance_id(), dw_data_id, n32High, n32Low, &bRet);

	return bRet;
}

VOID MapScript::onclock(Map* pMap, INT nClock) const
{
	if(!VALID_POINT(m_szFunc[ESME_Clock])) return;

	if(!VALID_POINT(pMap))
		return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_Clock], "uui", pMap->get_map_id(), pMap->get_instance_id(), nClock);

	return;
}

BOOL MapScript::can_use_skill(Map* pMap, DWORD dw_data_id) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanUseSkill]) ) return  TRUE;
	BOOL bRet = TRUE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanUseSkill], "uuu>b", pMap->get_map_id(), pMap->get_instance_id(), dw_data_id, &bRet);

	return bRet;
}

VOID MapScript::Revive(Role* pRole, ERoleReviveType eType, INT &nReviveHP, INT &nReviveMP, FLOAT &fx, FLOAT &fy, FLOAT &fz, DWORD &dwRebornMapID) const 
{
	if( !VALID_POINT(m_szFunc[ESME_On_Revive]) )  return;
	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap))   return;

	DWORD dwMapID = pMap->get_map_id();
	DWORD dwInstanceID = pMap->get_instance_id();

	// 统一脚本坐标为格子坐标
	fx = fx / TILE_SCALE;
	fz = fz / TILE_SCALE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_On_Revive], "uuuiiifffu>iifffu", dwMapID, dwInstanceID, pRole->GetID(), eType, 
		nReviveHP, nReviveMP, fx, fy, fz, dwRebornMapID, &nReviveHP, &nReviveMP, &fx, &fy, &fz, &dwRebornMapID);


	fx = fx * TILE_SCALE;
	fz = fz * TILE_SCALE;
}

VOID MapScript::OnEnterTrigger(Role* pRole, tag_map_trigger_info* pTrigger, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnEnterTrigger]) ) return;
	if( !VALID_POINT(pRole) || !VALID_POINT(pTrigger) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnEnterTrigger], "uuuu", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID(), pTrigger->dw_att_id);
}

VOID MapScript::OnEnterArea(Role* pRole, tag_map_area_info* pArea, Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_OnEnterArea]) ) return;
	if( !VALID_POINT(pRole) || !VALID_POINT(pArea) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_OnEnterArea], "uuuu", pMap->get_map_id(), pMap->get_instance_id(), pRole->GetID(), pArea->dw_att_id);
}

INT  MapScript::can_invite_join_team(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanInviteJoinTeam]) ) return 0;
	INT	 nRet = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanInviteJoinTeam], "uu>i", pMap->get_map_id(), pMap->get_instance_id(), &nRet);

	return nRet;
}

INT MapScript::can_leave_team(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanLeaveTeam]) ) return 0;
	INT  nRet = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanLeaveTeam], "uu>i", pMap->get_map_id(), pMap->get_instance_id(), &nRet);

	return nRet;
}

INT MapScript::can_change_leader(Map* pMap) const
{
	if( !VALID_POINT(m_szFunc[ESME_CanChangeLeader]) ) return 0;
	INT  nRet = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanChangeLeader], "uu>i", pMap->get_map_id(), pMap->get_instance_id(), &nRet);

	return nRet;
}

INT MapScript::can_kick_member(Map* pMap) const
{
	if ( !VALID_POINT(m_szFunc[ESME_CanKickMember]) ) return 0;
	INT nRet = E_Success;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_CanKickMember], "uu>i", pMap->get_map_id(), pMap->get_instance_id(), &nRet);

	return nRet;
}

DWORD MapScript::FriendEnemy(Map* pMap, Unit* pSrc, Unit* pTarget, BOOL& bIgnore) const
{
	if( !VALID_POINT(m_szFunc[ESME_FriendEnemy]) ) return 0;

	BOOL bFriend		=	FALSE;
	BOOL bHostile		=	FALSE;
	BOOL bIndependent	=	FALSE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESME_FriendEnemy], "uuuu>bbbb", pMap->get_map_id(), pMap->get_instance_id(), pSrc->GetID(), pTarget->GetID(),
		&bFriend, &bHostile, &bIndependent, &bIgnore);

	DWORD dwFlag = 0;
	if( bFriend )		dwFlag	|=	ETFE_Friendly;
	if( bHostile )		dwFlag	|=	ETFE_Hostile;
	if( bIndependent )	dwFlag	|=	ETFE_Independent;

	return dwFlag;
}

//---------------------------------------------------------------------------------------------
// 活动脚本
//---------------------------------------------------------------------------------------------
VOID ActScript::OnInit(DWORD dwActID) const
{
	if(!VALID_POINT(m_szFunc[ESAE_OnInit]))	return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_OnInit], "u", dwActID);
}

VOID ActScript::OnTimer(DWORD dwActID, INT nSec) const
{
	if( !VALID_POINT(m_szFunc[ESAE_OnTimer]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_OnTimer], "ui", dwActID, nSec);
}

VOID ActScript::OnTimerMin(DWORD dwActID) const
{
	if( !VALID_POINT(m_szFunc[ESAE_OnTimerMin]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_OnTimerMin], "u", dwActID);
}

VOID ActScript::OnActStart(DWORD dwActID) const
{
	if( !VALID_POINT(m_szFunc[ESAE_OnStart]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_OnStart], "u", dwActID);
}

VOID ActScript::on_act_end(DWORD dwActID) const
{
	if( !VALID_POINT(m_szFunc[ESAE_OnEnd]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_OnEnd], "u", dwActID);
}

VOID ActScript::OnDefaultRequest(DWORD dwActID, Role* pRole, DWORD	dwEventType) const
{
	if( !VALID_POINT(m_szFunc[ESAE_OnDefaultRequest]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_OnDefaultRequest], "uuu", dwActID, pRole->GetID(), dwEventType);
}

VOID ActScript::BroadActivityState(DWORD dwActID, INT nState) const
{
	if( !VALID_POINT(m_szFunc[ESAE_Broad]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESAE_Broad], "ui", dwActID, nState);
}

//---------------------------------------------------------------------------------------------
// 游戏世界事件脚本
//---------------------------------------------------------------------------------------------
VOID WorldScript::OnAdventure(Role *pRole) const
{
	if( !VALID_POINT(m_szFunc[ESWE_Adventure]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESWE_Adventure], "u", pRole->GetID());
}

VOID WorldScript::OnGuildInitOk() const
{
	if( !VALID_POINT(m_szFunc[ESWE_Guild_Init_Ok]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESWE_Guild_Init_Ok], "");
}
//---------------------------------------------------------------------------------------------
// 物品事件脚本
//---------------------------------------------------------------------------------------------
INT ItemScript::can_use_item(Map* pMap, DWORD dw_data_id, DWORD dwSrcID, DWORD dwTargetID, BOOL &bIgnore, INT64 n64_Item_Serial) const
{
	if( !VALID_POINT(m_szFunc[ESIE_CanUse]) ) return 0;
	BOOL bUseable = FALSE;
	INT	 nRet = E_Success;

	const INT32 nMask	= 0xFFFFFFFF;

	INT32 n32High	= (INT32)((n64_Item_Serial >> 32) & nMask);
	INT32 n32Low	= (INT32)(n64_Item_Serial & nMask);

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESIE_CanUse], "uuuuuuu>ii", pMap->get_map_id(), pMap->get_instance_id(), dw_data_id, dwSrcID, dwTargetID, n32High, n32Low, &nRet, &bIgnore);

	return nRet;
}

BOOL ItemScript::UseItem(Map* pMap, DWORD dw_data_id, DWORD dwSrcID, DWORD dwTargetID, INT64 n64_Item_Serial) const
{
	if( !VALID_POINT(m_szFunc[ESIE_Use]) ) return TRUE;
	
	const INT32 nMask	= 0xFFFFFFFF;

	INT32 n32High	= (INT32)((n64_Item_Serial >> 32) & nMask);
	INT32 n32Low	= (INT32)(n64_Item_Serial & nMask);

	BOOL bDelete = TRUE;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESIE_Use], "uuuuuuu>b", pMap->get_map_id(), pMap->get_instance_id(), dw_data_id, dwSrcID, dwTargetID, n32High, n32Low, &bDelete);
	return bDelete;
}

VOID ItemScript::DelItem(Map* pMap, DWORD dw_role_id, DWORD dw_data_id) const
{
	if( !VALID_POINT(m_szFunc[ESIE_Del]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESIE_Del], "uuuu", pMap->get_map_id(), pMap->get_instance_id(), dw_role_id, dw_data_id);
}

//-------------------------------------------------------------------------------------
// 能否使用技能
//-------------------------------------------------------------------------------------
DWORD SkillScript::CanCastSkill( Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDstUnitID ) const
{
	if( !VALID_POINT(m_szFunc[ESSE_CanCast]) ) return 0;
	BOOL bRet = FALSE;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESIE_CanUse], "uuuuuu>i", pMap->get_map_id(), pMap->get_instance_id(), dwSkillID, dwSkillLevel, dwOwnerID, dwDstUnitID, &bRet);

	return bRet;
}

//-------------------------------------------------------------------------------------
// 技能开始吟唱
//-------------------------------------------------------------------------------------
VOID SkillScript::PreparingSkill(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDstUnitID) const
{
	if( !VALID_POINT(m_szFunc[ESSE_Preparing]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESSE_Preparing], "uuuuuu", pMap->get_map_id(), pMap->get_instance_id(), dwSkillID, dwSkillLevel, dwOwnerID, dwDstUnitID);
}

//-------------------------------------------------------------------------------------
// 使用技能
//-------------------------------------------------------------------------------------
VOID SkillScript::CastSkill(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, BOOL &bIgnore) const
{
	if( !VALID_POINT(m_szFunc[ESSE_Cast]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESSE_Cast], "uuuuu>i", pMap->get_map_id(), pMap->get_instance_id(), dwSkillID, dwSkillLevel, dwOwnerID, &bIgnore);
}
//-------------------------------------------------------------------------------------
// 技能伤害计算
//-------------------------------------------------------------------------------------
FLOAT SkillScript::CalculateDmg(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDestUnitID, DWORD bblock, DWORD bcrit, FLOAT& fDmg) const
{
	if( !VALID_POINT(m_szFunc[ESSE_CalDmg]) ) return fDmg;	

	FLOAT dmg = 0;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESSE_CalDmg], "uuuuuufuu>f", pMap->get_map_id(), pMap->get_instance_id(), dwSkillID, dwSkillLevel, dwOwnerID, dwDestUnitID, fDmg, bblock, bcrit, &dmg);

	return dmg;
}
//-------------------------------------------------------------------------------------
// 技能击杀目标
//-------------------------------------------------------------------------------------
VOID SkillScript::KillUnit(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDestUnitID) const
{
	if( !VALID_POINT(m_szFunc[ESSE_KillUnit]) ) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESSE_KillUnit], "uuuuu", pMap->get_map_id(), pMap->get_instance_id(), dwSkillID, dwSkillLevel, dwOwnerID, dwDestUnitID);
}
//-------------------------------------------------------------------------------------
// buff伤害计算
//-------------------------------------------------------------------------------------
INT	BuffScript::CalculateDmg(Map* pMap, DWORD dwBuffID, DWORD dwBuffLevel, DWORD dwOwnerID, DWORD dwDestUnitID, INT& fDmg) const
{
	if( !VALID_POINT(m_szFunc[ESBE_CalDmg]) ) return fDmg;	

	INT dmg = 0;
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESBE_CalDmg], "uuuuuui>i", pMap->get_map_id(), pMap->get_instance_id(), dwBuffID, dwBuffLevel, dwOwnerID, dwDestUnitID, fDmg, &dmg);

	return dmg;
}
//-------------------------------------------------------------------------------------
// buff初始化
//-------------------------------------------------------------------------------------
INT	BuffScript::OnInit(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwSrcID) const
{
	if ( !VALID_POINT(m_szFunc[ESBE_Init])) return 0;
	
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESBE_Init], "uuuuuu", pMap->get_map_id(), pMap->get_instance_id(), dwBuffID, dwLevel, dwOwnerID, dwSrcID);
}
//-------------------------------------------------------------------------------------
// buff效果触发时
//-------------------------------------------------------------------------------------
INT	BuffScript::OnTrigger(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwSrcID) const
{
	if ( !VALID_POINT(m_szFunc[ESBE_Trigger])) return 0;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESBE_Trigger], "uuuuuu", pMap->get_map_id(), pMap->get_instance_id(), dwBuffID, dwLevel, dwOwnerID, dwSrcID);
}
//-------------------------------------------------------------------------------------
// buff结束时
//-------------------------------------------------------------------------------------
INT	BuffScript::OnDestory(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwSrcID) const
{
	if ( !VALID_POINT(m_szFunc[ESBE_Destory])) return 0;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESBE_Destory], "uuuuuu", pMap->get_map_id(), pMap->get_instance_id(), dwBuffID, dwLevel, dwOwnerID, dwSrcID);
}

// 角色排行榜第一次初始化
VOID RankScript::OnInitRoleLevel() const
{
	if (!VALID_POINT(m_szFunc[ESRKE_InitRoleLevel])) return ;
	
	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRKE_InitRoleLevel], "");
}

// 噬魂发奖
VOID RankScript::OnShihunGiveReward() const
{
	if(!VALID_POINT(m_szFunc[ESRKE_Shihun])) return;

	g_ScriptMgr.CallScriptFunction(m_szFunc[ESRKE_Shihun], "");
}

