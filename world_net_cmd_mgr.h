
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ϣ������ -- ��Ҫ��ͼ�߳��ϲ㴦�����Ϣ����
#pragma once

#include "mutex.h"
#include "player_net_cmd_mgr.h"
#include "world_session.h"
//-------------------------------------------------------------------------------
// ������Ϣ�ṹ��չ
//-------------------------------------------------------------------------------
struct tagMsgEx
{
	DWORD			dwID;		// session id
	LPBYTE			p_message;
	DWORD			dw_size;

	tagMsgEx(DWORD dwSessionID, LPBYTE _pMsg, DWORD _dwSize)
	{
		dwID	=	dwSessionID;
		p_message	=	_pMsg;
		dw_size	=	_dwSize;
	}

	~tagMsgEx()
	{
		if( VALID_POINT(p_message) )
		{
			g_worldSession.ReturnMsg(p_message);
		}
	}
};


//-------------------------------------------------------------------------------
// ��ͼ�ϲ���Ϣ������
//-------------------------------------------------------------------------------
class WorldNetCmdMgr
{
public:
	WorldNetCmdMgr();
	~WorldNetCmdMgr();

	BOOL Init() { return TRUE; }
	VOID Update();

	static WorldNetCmdMgr* GetInstance();
	static VOID	 Destroy();

public:
	VOID Add(DWORD dwSessionID, LPBYTE p_message, DWORD dw_size);

public:
	//VOID RegisterAll();
	VOID RegisterRecvProc(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size);
	VOID ResetMsgCounter(LPCSTR szCmd);
	VOID ResetMsgCounter(DWORD dwMsgID);

	UINT32 GetRunTimesPerTick(DWORD dwMsgID);

private:
	package_list<tagMsgEx*>	m_lstMsgEx;
	Mutex				m_mutex;

	PlayerNetCmdMgr		m_WorldNetMgr;

	// ��ϢÿTickͳ��
	package_map<DWORD, UINT32>	m_mapMsgCount;

	static WorldNetCmdMgr* m_pInstance;
};

//extern WorldNetCmdMgr g_worldNetCmdMgr;