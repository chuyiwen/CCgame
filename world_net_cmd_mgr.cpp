
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//消息管理类 -- 需要地图线程上层处理的消息管理
#include "StdAfx.h"

#include "world_net_cmd_mgr.h"
#include "role.h"
#include "role_mgr.h"

//WorldNetCmdMgr g_worldNetCmdMgr;
WorldNetCmdMgr* WorldNetCmdMgr::m_pInstance = NULL;
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// 构造&析构
//-------------------------------------------------------------------------------
WorldNetCmdMgr::WorldNetCmdMgr()
{
	m_lstMsgEx.clear();
	m_mapMsgCount.clear();
}

WorldNetCmdMgr::~WorldNetCmdMgr()
{
	tagMsgEx* p_message = m_lstMsgEx.pop_front();
	while( VALID_POINT(p_message) )
	{
		SAFE_DELETE(p_message);
		p_message = m_lstMsgEx.pop_front();
	}
	m_mapMsgCount.clear();

	m_WorldNetMgr.UnRegisterAll();
}

WorldNetCmdMgr* WorldNetCmdMgr::GetInstance()
{
	if(!VALID_POINT(m_pInstance))
	{
		m_pInstance = new WorldNetCmdMgr;
	}

	return m_pInstance;
}

VOID	 WorldNetCmdMgr::Destroy()
{
	SAFE_DELETE(m_pInstance);
}

//-------------------------------------------------------------------------------
// 将消息添加到消息队列
//-------------------------------------------------------------------------------
VOID WorldNetCmdMgr::Add(DWORD dwSessionID, LPBYTE p_message, DWORD dw_size)
{
	tagMsgEx *pMsgEx = new tagMsgEx(dwSessionID, p_message, dw_size);
	
	m_mutex.Acquire();
	m_lstMsgEx.push_back(pMsgEx);
	ResetMsgCounter(((tag_net_message*)p_message)->dw_message_id);
	m_mutex.Release();
}

//-------------------------------------------------------------------------------
// 注册消息处理函数
//-------------------------------------------------------------------------------
VOID WorldNetCmdMgr::RegisterRecvProc(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size)
{
	m_WorldNetMgr.RegisterRecvProc(szCmd, fp, szDesc, dw_size);
	ResetMsgCounter(szCmd);
}

//-------------------------------------------------------------------------------
// 重置消息调用计数
//-------------------------------------------------------------------------------
VOID WorldNetCmdMgr::ResetMsgCounter( LPCSTR szCmd )
{
	tool util;
	DWORD dwID = util.crc32(szCmd);

	ResetMsgCounter(dwID);
}

VOID WorldNetCmdMgr::ResetMsgCounter( DWORD dwMsgID )
{
	UINT32 nCount = m_WorldNetMgr.GetRecvCmdRunTimes(dwMsgID);
	if (!VALID_VALUE(nCount))
		return;

	if( !m_mapMsgCount.change_value(dwMsgID, nCount) )
	{
		m_mapMsgCount.add(dwMsgID, 0);
		return;
	}
}

UINT32 WorldNetCmdMgr::GetRunTimesPerTick( DWORD dwMsgID )
{
	UINT32 nCurCount = m_WorldNetMgr.GetRecvCmdRunTimes(dwMsgID);
	if (!VALID_VALUE(nCurCount))
		return INVALID_VALUE;

	UINT32 nTickCount = m_mapMsgCount.find(dwMsgID);
	if (!VALID_VALUE(nTickCount))
	{
		m_mapMsgCount.add(dwMsgID, 0);
		nTickCount = 0;
	}

	ASSERT(nCurCount >= nTickCount);

	return nCurCount - nTickCount;
}

//-------------------------------------------------------------------------------
// 注册消息处理函数
//-------------------------------------------------------------------------------
VOID WorldNetCmdMgr::Update()
{
	PlayerSession *pSession = NULL;

	tagMsgEx* p_message = m_lstMsgEx.pop_front();
	while( VALID_POINT(p_message) )
	{
		pSession = g_worldSession.FindSession(p_message->dwID);
		if( VALID_POINT(pSession) )
		{
			M_trans_pointer(pCmd, p_message->p_message, tag_net_message);
			m_WorldNetMgr.HandleCmd(pCmd, p_message->dw_size, pSession);
		}

		SAFE_DELETE(p_message);
		p_message = m_lstMsgEx.pop_front();
	}
}