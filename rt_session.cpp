
/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/


#include "stdafx.h"

#include "../ServerDefine/msg_rt_s.h"
#include "../ServerDefine/base_define.h"
#include "rt_session.h"
#include "world.h"
#include "world_session.h"
#include "gm_policy.h"
#include "role_mgr.h"
#include "../WorldDefine/mall_define.h"
#include "mall.h"
#include "../WorldDefine/chat.h"
#include "channel_mgr.h"
#include "../WorldDefine/chat_define.h"
//-----------------------------------------------------------------------------
//! construction
//-----------------------------------------------------------------------------
montioring_session::montioring_session() : m_Trunk(this)
{
	m_bInitOK					=	FALSE;
	m_bTermConnect				=	FALSE;
	m_nSendInfoTickCountDown	=	SEND_INFO_INTERVAL;
}

//-----------------------------------------------------------------------------
//! destruction
//-----------------------------------------------------------------------------
montioring_session::~montioring_session()
{
}

//-----------------------------------------------------------------------------
// init
//-----------------------------------------------------------------------------
BOOL montioring_session::Init()
{
	m_pThread		=	"Thread";
	m_pUtil = new Util;

	// 初始化成员属性
	m_strIP			=	World::p_var->GetString(_T("ip gm_server"));
	m_dwPort		=	World::p_var->GetDword(_T("port gm_server"));
	m_dwSectionID	=	World::p_var->GetDword(_T("section_id world"));
	m_dwWorldID		=	World::p_var->GetDword(_T("id world"));

	// 创建消息管理
	m_pTran = new few_connect_client;
	if( !P_VALID(m_pTran) )
	{
		ERR(_T("Create ToRT(few_connect_client) obj failed!\r\n"));
		return FALSE;
	}
	m_pTran->init();

	// 注册所有网络命令
	RegisterAllRTCommand();

	// 启动连接线程
	if(!m_pThread->CreateThread(_T("ConnectRT"), 
		(serverbase::THREADPROC)m_Trunk.sfp0(&montioring_session::ThreadConnectRT), NULL))
	{
		return FALSE;
	}

	while( !m_pThread->IsThreadActive(_T("ConnectRT")) )
	{
		continue;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// destroy
//-----------------------------------------------------------------------------
VOID montioring_session::Destroy()
{
	// 等待所有线程结束
	InterlockedExchange((LONG*)&m_bTermConnect, TRUE);
	m_pThread->WaitForThreadDestroy(_T("ConnectRT"), INFINITE);

	m_pTran->destory();
	SAFE_DEL(m_pTran);

	// 注销消息管理
	UnRegisterAllRTCommand();

	SAFE_DEL(m_pUtil);
}



//-----------------------------------------------------------------------------
// update
//-----------------------------------------------------------------------------
VOID montioring_session::Update()
{
	UpdateSession();
	SendServerInfo();
}

//-----------------------------------------------------------------------------
// 接收消息
//-----------------------------------------------------------------------------
VOID montioring_session::UpdateSession()
{
	if( NULL == m_pTran )
		return;

	if( !m_pTran->is_connect() && !m_pThread->IsThreadActive(_T("ConnectRT")) )
	{
		InterlockedExchange((LONG*)&m_bTermConnect, TRUE);
		m_pTran->disconnect();

		m_pThread->WaitForThreadDestroy(_T("ConnectRT"), INFINITE);

		// 重新启动登陆服务器连接线程
		InterlockedExchange((LONG*)&m_bTermConnect, FALSE);
		m_pThread->CreateThread(_T("ConnectRT"), (serverbase::THREADPROC)m_Trunk.sfp0(&montioring_session::ThreadConnectRT), NULL);

		while(FALSE == m_pThread->IsThreadActive(_T("ConnectRT")))
		{
			continue;
		}

		return;
	}

	while(m_pTran->is_connect())
	{
		DWORD dw_size = 0;
		LPBYTE p_receive = m_pTran->recv_msg(dw_size);
		if( !P_VALID(p_receive) )
			break;

		// 处理消息
		serverbase::NetCmdMgr::GetSingleton().HandleCmd((tagNetCmd*)p_receive, dw_size, GT_INVALID);

		// 回收资源
		m_pTran->free_recv_msg(p_receive);

		
	}	
}

//-----------------------------------------------------------------------------
// 发送服务器情况
//-----------------------------------------------------------------------------
VOID montioring_session::SendServerInfo()
{
	if( !m_pTran->is_connect() || !m_bInitOK ) return;

	if( --m_nSendInfoTickCountDown > 0 )
		return;

	m_nSendInfoTickCountDown = SEND_INFO_INTERVAL;

	tagNSC_WorldInfo send;
	
	if( g_world.IsWell() )
	{
		send.eStatus = EWS_Well;
	}
	else
	{
		send.eStatus = EWS_SystemError;
	}
	send.nMaxOnlineNum = g_worldSession.GetPlayerNumLimit();
	send.nOnlineNum = g_worldSession.GetPlayerNumCurrent();
	m_pTran->send_msg(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// 连接线程(连接监控服务器)
//-----------------------------------------------------------------------------
DWORD montioring_session::ThreadConnectRT()
{
//#ifdef _DEBUG
	THROW_EXCEPTION_START;
//#endif

	while( FALSE == m_bTermConnect )
	{
		if( !m_pTran->is_connect() )
		{
			if( !m_pTran->is_trying_create_connect() )
			{
				m_pTran->try_create_connect(m_pUtil->UnicodeToUnicode8(m_strIP.c_str()), m_dwPort);
			}

			Sleep(100);
			continue;	// 重新检测连接
		}

		IMSG(_T("Contected to RT Server at %s: %d\r\n"), m_strIP.c_str(), m_dwPort);

		
		tagNSC_LoginService send;
		send.dwSectionID	=	m_dwSectionID;
		send.dw_world_id		=	m_dwWorldID;
		send.eType			=	EST_World;
		m_pTran->send_msg(&send, send.dw_size);

		break;
	}

//#ifdef _DEBUG
	THROW_EXCEPTION_END;
//#endif
	return 0;
}


//----------------------------------------------------------------------------------------------
// 注册所有的网络命令
//----------------------------------------------------------------------------------------------
VOID montioring_session::RegisterAllRTCommand()
{
	REGISTER_NET_MSG("NSS_LoginService",	montioring_session,	HandleServerLogin,	_T("RT Server Certification Reply"));
	REGISTER_NET_MSG("NSS_Close",		montioring_session,		HandleCloseServer,	_T("Close Server"));
	REGISTER_NET_MSG("NSS_Double",		montioring_session,	HandleDouble,			_T("Double Rate"));
	REGISTER_NET_MSG("NSS_AutoNotice",	montioring_session,	HandleAutoNotice,		_T("AutoNotice"));
	REGISTER_NET_MSG("NSS_RightNotice",	montioring_session,	HandleRightNotice,	_T("RightNotice"));
	REGISTER_NET_MSG("NSS_MaxNum",		montioring_session,	HandleSetMaxNum,		_T("Set Max User Online"));
	REGISTER_NET_MSG("NSS_UpdateMall",	montioring_session,	HandleUpdateMall,		_T("Update Mall"));
	REGISTER_NET_MSG("NSS_AutoChatNotice",	montioring_session,HandleAutoChatNotice,	_T("Chat Notice"));
	REGISTER_NET_MSG("NSS_CancelDouble",	montioring_session,	HandleCancelDouble,	_T("Cancel Double"));
}

VOID montioring_session::UnRegisterAllRTCommand()
{
	UNREGISTER_NET_MSG("NSS_LoginService",	montioring_session,	HandleServerLogin);
	UNREGISTER_NET_MSG("NSS_Close",		montioring_session,		HandleCloseServer);
	UNREGISTER_NET_MSG("NSS_Double",		montioring_session,	HandleDouble);
	UNREGISTER_NET_MSG("NSS_AutoNotice",	montioring_session,	HandleAutoNotice);
	UNREGISTER_NET_MSG("NSS_RightNotice",	montioring_session,	HandleRightNotice);
	UNREGISTER_NET_MSG("NSS_MaxNum",		montioring_session,	HandleSetMaxNum);
	UNREGISTER_NET_MSG("NSS_UpdateMall",	montioring_session,	HandleUpdateMall);
	UNREGISTER_NET_MSG("NSS_AutoChatNotice",	montioring_session,HandleAutoChatNotice);
	UNREGISTER_NET_MSG("NSS_CancelDouble",	montioring_session,	HandleCancelDouble);
}

//----------------------------------------------------------------------------------------------
// 服务器认证网络消息
//----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleServerLogin(tagNetCmd* p_message, DWORD)
{
	tagNSS_LoginService* p_receive = (tagNSS_LoginService*)p_message;

	InterlockedExchange((LPLONG)&m_bInitOK, TRUE);
	return 0;
}

//-----------------------------------------------------------------------------------------------
// 服务器关闭网络消息
//-----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleCloseServer(tagNetCmd* p_message, DWORD)
{
	g_world.ShutDown();
	return 0;
}

//-----------------------------------------------------------------------------------------------
// 设置双倍
//-----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleDouble( tagNetCmd* p_message, DWORD )
{
	MGET_MSG(p_receive, p_message, NSS_Double);

	g_GMPolicy.SetRate(p_receive->eDType, p_receive->dwRatio, p_receive->dwOpenTime, p_receive->dwLastTime);

	tagNSC_Double send;
	send.dw_error_code	= 0;
	send.dw_client_id		= p_receive->dw_client_id;
	m_pTran->send_msg(&send, send.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------------------------
// 设置自动循环公告
//-----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleAutoNotice( tagNetCmd* p_message, DWORD )
{
	MGET_MSG(pSend, p_message, NSS_AutoNotice);

	DWORD dwLen = sizeof(tagNS_AutoNotice) - sizeof(TCHAR) + 2*(wcslen(pSend->szContent) + 1);

	MCREATE_MSG(pSendMsg, dwLen, NS_AutoNotice);

	pSendMsg->nCirInterval = pSend->nCirInterval;
	pSendMsg->nType = pSend->eType;
	_tcscpy(pSendMsg->szContent, pSend->szContent);
	
	g_roleMgr.SendWorldMsg(pSendMsg, dwLen);

	MDEL_MSG(pSendMsg);

	tagNSC_AutoNotice msg;
	msg.dw_error_code	= 0;
	msg.dw_client_id		= pSend->dw_client_id;
	m_pTran->send_msg(&msg, msg.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------------------------
// 设置右下角公告
//-----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleRightNotice(tagNetCmd* p_message, DWORD)
{
	MGET_MSG(pSend, p_message, NSS_RightNotice);

	DWORD dwLen = sizeof(tagNS_RightNotice) - sizeof(TCHAR) + 2*( wcslen(pSend->szTitle) + wcslen(pSend->szContent) + wcslen(pSend->szLink) +1);
	
	MCREATE_MSG(pSendMsg, dwLen, NS_RightNotice);

	pSendMsg->nTitleLen = wcslen(pSend->szTitle);
	pSendMsg->nContentLen = wcslen(pSend->szContent);
	pSendMsg->nLinkLen = wcslen(pSend->szLink);

	_tcscpy(pSendMsg->szContent, pSend->szTitle);
	_tcscat(pSendMsg->szContent, pSend->szLink);
	_tcscat(pSendMsg->szContent, pSend->szContent);

	g_roleMgr.SendWorldMsg(pSendMsg, dwLen);

	MDEL_MSG(pSendMsg);

	tagNSC_RightNotice msg;
	msg.dw_error_code	= 0;
	msg.dw_client_id		= pSend->dw_client_id;
	m_pTran->send_msg(&msg, msg.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------------------------
// 设置游戏最大在线人数
//-----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleSetMaxNum(tagNetCmd* p_message, DWORD)
{
	MGET_MSG(pSend, p_message, NSS_MaxNum);

	g_worldSession.SetPlayerNumLimit(pSend->nMaxNum);

	tagNSC_MaxNum send;
	send.dw_client_id = pSend->dw_client_id;
	send.dw_error_code = 0;
	m_pTran->send_msg(&send, send.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------------------------
// 更新商城
//-----------------------------------------------------------------------------------------------
DWORD montioring_session::HandleUpdateMall(tagNetCmd* p_message, DWORD)
{
	MGET_MSG(pSend, p_message, NSS_UpdateMall);
	
	BOOL bSuccess = FALSE;
	bSuccess = g_mall.ReInit();

	tagNSC_UpdateMall send;
	send.dw_client_id = pSend->dw_client_id;
	send.dw_error_code = bSuccess;
	m_pTran->send_msg(&send, send.dw_size);

	return 0;
}

DWORD montioring_session::HandleAutoChatNotice(tagNetCmd* p_message, DWORD)
{
	MGET_MSG(p_receive, p_message, NSS_AutoChatNotice);

	DWORD dw_size = sizeof(tagNS_RoleChat) + X_HUGE_STRING;
	MCREATE_MSG(pSend, dw_size, NS_RoleChat);
	pSend->byChannel		= (BYTE)ESCC_Affiche;
	pSend->dwID				= TObjRef<Util>()->Crc32("NS_RoleChat");
	pSend->dwDestRoleID		= GT_INVALID;
	pSend->dwSrcRoleID		= GT_INVALID;
	pSend->dw_error_code		= 0;
	pSend->dw_size			= dw_size;
	IFASTCODE->MemCpy(pSend->szMsg, p_receive->szNotice, X_HUGE_STRING);

	g_roleMgr.ForEachRoleInWorld(SendAction(ESCC_Affiche, pSend));

	MDEL_MSG(pSend);
	return 0;
}

DWORD montioring_session::HandleCancelDouble(tagNetCmd* p_message, DWORD)
{
	MGET_MSG(p_receive, p_message, NSS_CancelDouble);

	g_GMPolicy.DoubleSwitch(EDoubleType_Exp, FALSE);
	g_GMPolicy.DoubleSwitch(EDoubleType_Item, FALSE);

	tagNSC_CancelDouble send;
	send.dw_error_code	= 0;
	send.dw_client_id		= p_receive->dw_client_id;
	m_pTran->send_msg(&send, send.dw_size);

	return 0;
}

montioring_session g_rtSession;
