
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ͻ����������
#include "StdAfx.h"
#include "player_net_cmd_mgr.h"
#include "player_session.h"
#include "world.h"

//-----------------------------------------------------------------------------
// construct
//-----------------------------------------------------------------------------
PlayerNetCmdMgr::PlayerNetCmdMgr()
{
	m_mapRecvProc.clear();
	m_mapSendProc.clear();
	
}

//-----------------------------------------------------------------------------
// destruct
//-----------------------------------------------------------------------------
PlayerNetCmdMgr::~PlayerNetCmdMgr()
{
	Destroy();
	
}

//-----------------------------------------------------------------------------
// destroy
//-----------------------------------------------------------------------------
VOID PlayerNetCmdMgr::Destroy()
{
}

//-----------------------------------------------------------------------------
// ��ӡ���е�������Ϣͳ����Ϣ��log
//-----------------------------------------------------------------------------
VOID PlayerNetCmdMgr::LogAllMsg()
{
	tagPlayerCmd* pCmd = NULL;

	g_world.get_log()->write_log(_T("\r\n\r\n"));

	// ���ȴ�ӡ���еĿͻ�����Ϣ
	g_world.get_log()->write_log(_T("Client Msg Statistics:\r\n"));

	m_mapRecvProc.reset_iterator();
	while( m_mapRecvProc.find_next(pCmd) )
	{
		g_world.get_log()->write_log(_T("%s\t\t%u\r\n"), get_tool()->unicode8_to_unicode(pCmd->strCmd.c_str()), pCmd->nTimes);
	}
	g_world.get_log()->write_log(_T("\r\n\r\n"));

	// �ٴ�ӡ�������˵���Ϣ
	g_world.get_log()->write_log(_T("Server Msg Statistics:\r\n"));

	m_mapSendProc.reset_iterator();
	while( m_mapSendProc.find_next(pCmd) )
	{
		g_world.get_log()->write_log(_T("%s\t\t%u\r\n"), get_tool()->unicode8_to_unicode(pCmd->strCmd.c_str()), pCmd->nTimes);
	}
	g_world.get_log()->write_log(_T("\r\n\r\n"));
}

//-----------------------------------------------------------------------------
// ע�������Ϣ
//-----------------------------------------------------------------------------
BOOL PlayerNetCmdMgr::RegisterRecvProc(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size)
{
	DWORD dwID = get_tool()->crc32(szCmd);

	tagPlayerCmd* pCmd = m_mapRecvProc.find(dwID);

	if( VALID_POINT(pCmd) )
	{
		if( pCmd->strCmd != szCmd )
		{
			ASSERT(0);	// ��������ӵ����ͬ��CRC
			return FALSE;
		}
	}
	else
	{
		pCmd = new tagPlayerCmd;
		pCmd->nTimes = 0;
		pCmd->dw_size = dw_size;
		pCmd->handler = fp;
		pCmd->strCmd = szCmd;
		pCmd->strDesc = szDesc;
		m_mapRecvProc.add(dwID, pCmd);
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// ע�ᷢ����Ϣ
//------------------------------------------------------------------------------
BOOL PlayerNetCmdMgr::RegisterSendProc(LPCSTR szCmd)
{
	DWORD dwID = get_tool()->crc32(szCmd);

	tagPlayerCmd* pCmd = m_mapSendProc.find(dwID);

	if( VALID_POINT(pCmd) )
	{
		if( pCmd->strCmd != szCmd )
		{
			ASSERT(0);
			return FALSE;
		}
	}
	else
	{
		pCmd = new tagPlayerCmd;
		pCmd->nTimes = 0;
		pCmd->dw_size = 0;
		pCmd->handler = NULL;
		pCmd->strCmd = szCmd;
		m_mapSendProc.add(dwID, pCmd);
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// ȡ��ע��
//------------------------------------------------------------------------------
VOID PlayerNetCmdMgr::UnRegisterAll()
{
	tagPlayerCmd* pCmd = NULL;

	m_mapRecvProc.reset_iterator();
	while( m_mapRecvProc.find_next(pCmd) )
	{
		SAFE_DELETE(pCmd);
	}
	m_mapRecvProc.clear();

	m_mapSendProc.reset_iterator();
	while( m_mapSendProc.find_next(pCmd) )
	{
		SAFE_DELETE(pCmd);
	}
	m_mapSendProc.clear();
}

//------------------------------------------------------------------------------
// �õ�ĳ����ϢID��Ӧ�Ĵ�����
//------------------------------------------------------------------------------
NETMSGHANDLER PlayerNetCmdMgr::GetHandler(tag_net_message* p_message, UINT32 nMsgSize)
{
	tagPlayerCmd* pCmd = m_mapRecvProc.find(p_message->dw_message_id);
	if( !VALID_POINT(pCmd) )
	{
		/*TCHAR sz_message_name[100];
		memset(sz_message_name, 0, sizeof(sz_message_name));
		MultiByteToWideChar(CP_UTF8,NULL,p_message->sz_message_name,-1,sz_message_name,100);*/
		print_message(_T("Unknow player command recved[<cmdid>%u <size>%d]\r\n"), p_message->dw_message_id, nMsgSize);
		//print_message(_T("Unknow player command recved[<name>%s]\r\n"), sz_message_name);
		return NULL;
	}

	if( p_message->dw_size != nMsgSize || nMsgSize > GET_MAX_PACKAGE_LENGTH || nMsgSize < pCmd->dw_size )
	{
		/*TCHAR sz_message_name[100];
		memset(sz_message_name, 0, sizeof(sz_message_name));
		MultiByteToWideChar(CP_UTF8,NULL,p_message->sz_message_name,-1,sz_message_name,100);*/
		print_message(_T("Invalid net command size[<cmd>%u <recvSize>%d <mySize>%d]\r\n"), p_message->dw_message_id, p_message->dw_size, nMsgSize);
		//print_message(_T("Invalid net command[<name>%s]\r\n"), sz_message_name);
		return NULL;
	}

	Interlocked_Exchange_Add((LPLONG)&pCmd->nTimes, 1);

	return pCmd->handler;
}

//------------------------------------------------------------------------------------------
// ִ����Ϣ������
//------------------------------------------------------------------------------------------
BOOL PlayerNetCmdMgr::HandleCmd(tag_net_message* p_message, DWORD nMsgSize, PlayerSession* pSession)
{
	if( !VALID_POINT(pSession) ) return FALSE;

	NETMSGHANDLER fp = GetHandler(p_message, nMsgSize);
	if( NULL == fp ) return FALSE;

	(pSession->*fp)(p_message);

	return TRUE;
}

//-------------------------------------------------------------------------------------------
// �������˵ķ�������
//-------------------------------------------------------------------------------------------
VOID PlayerNetCmdMgr::CountServerMsg(DWORD dwMsgID)
{
	tagPlayerCmd* pCmd = m_mapSendProc.find(dwMsgID);

	if( VALID_POINT(pCmd) )
	{
		Interlocked_Exchange_Add((LPLONG)&pCmd->nTimes, 1);
	}
}

//-------------------------------------------------------------------------------------------
// ȡ�ý�������ִ�д���
//-------------------------------------------------------------------------------------------
UINT32 PlayerNetCmdMgr::GetRecvCmdRunTimes( DWORD dwMsgID )
{
	tagPlayerCmd* pCmd = m_mapRecvProc.find(dwMsgID);

	if (VALID_POINT(pCmd))
	{
		return pCmd->nTimes;
	}

	return INVALID_VALUE;
}
