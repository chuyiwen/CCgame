
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//服务器与付费服务器的对话层

#include "stdafx.h"

#include "../common/ServerDefine/bill_message_server.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "bill_session.h"
#include "world_session.h"
#include "player_session.h"
#include "role.h"
#include "auto_paimai.h"

//-----------------------------------------------------------------------------
//! construction
//-----------------------------------------------------------------------------
BillSession::BillSession()
{
	m_bInitOK					=	FALSE;
	m_bTermConnect				=	FALSE;
}

//-----------------------------------------------------------------------------
//! destruction
//-----------------------------------------------------------------------------
BillSession::~BillSession()
{
}

//-----------------------------------------------------------------------------
// init
//-----------------------------------------------------------------------------
BOOL BillSession::Init()
{
	
	// 初始化成员属性
	m_strIP			=	World::p_var->get_string(_T("ip bill_server"));
	m_dwPort		=	World::p_var->get_dword(_T("port bill_server"));
	m_dwSectionID	=	World::p_var->get_dword(_T("section_id world"));
	m_dwWorldID		=	World::p_var->get_dword(_T("id world"));

	// 创建消息管理
	/*CreateObj("ToBillNetCmdMgr", "net_command_manager");
	m_pMsgCmdMgr = "ToBillNetCmdMgr";*/

	// 创建连接对象，并初始化
	/*CreateObj("ToBill", "few_connect_client");
	m_pTran = "ToBill";*/
	m_pTran = new few_connect_client;
	if( !VALID_POINT(m_pTran) )
	{
		ERROR_CLUE_ON(_T("Create ToBill(few_connect_client) obj failed!\r\n"));
		return FALSE;
	}
	m_pTran->init();

	// 注册所有网络命令
	RegisterAllBillCommand();

	// 启动连接线程
	if(!World::p_thread->create_thread(_T("ConnectBill"), 
		&BillSession::static_thread_connect, this))
	{
		return FALSE;
	}

	while( !World::p_thread->is_thread_active(_T("ConnectBill")) )
	{
		continue;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// destroy
//-----------------------------------------------------------------------------
VOID BillSession::Destroy()
{
	// 等待所有线程结束
	Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
	World::p_thread->waitfor_thread_destroy(_T("ConnectBill"), INFINITE);

	UNREGISTER_NET_MSG("NBW_Login",			BillSession, HandleServerLogin);
	UNREGISTER_NET_MSG("NBW_PickBaiBao",	BillSession, HandlePickBaiBao);
	UNREGISTER_NET_MSG("NBW_ChangeReceive", BillSession, HandleChangeReceive);

	m_pTran->destory();
	SAFE_DELETE(m_pTran);
	
	
}



//-----------------------------------------------------------------------------
// update
//-----------------------------------------------------------------------------
VOID BillSession::Update()
{
	UpdateSession();
}

//-----------------------------------------------------------------------------
// 接收消息
//-----------------------------------------------------------------------------
VOID BillSession::UpdateSession()
{
	if( NULL == m_pTran )
		return;

	if( !m_pTran->is_connect() && !World::p_thread->is_thread_active(_T("ConnectBill")) )
	{
		Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
		m_pTran->disconnect();

		World::p_thread->waitfor_thread_destroy(_T("ConnectBill"), INFINITE);

		// 重新启动登陆服务器连接线程
		Interlocked_Exchange((LONG*)&m_bTermConnect, FALSE);
		World::p_thread->create_thread(_T("ConnectBill"), &BillSession::static_thread_connect, this);

		while(FALSE == World::p_thread->is_thread_active(_T("ConnectBill")))
		{
			continue;
		}

		return;
	}

	while(m_pTran->is_connect())
	{
		DWORD dw_size = 0;
		LPBYTE p_receive = m_pTran->recv_msg(dw_size);
		if( !VALID_POINT(p_receive) )
			break;

		// 处理消息
		if (!serverframe::net_command_manager::get_singleton().handle_message((tag_net_message*)p_receive, dw_size, INVALID_VALUE))
			print_message(_T("bill_session unknown msg"));
		// 回收资源
		m_pTran->free_recv_msg(p_receive);
	}	
}

//-----------------------------------------------------------------------------
// 连接线程(连接监控服务器)
//-----------------------------------------------------------------------------
UINT BillSession::thread_connect()
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
				m_pTran->try_create_connect(get_tool()->unicode_to_unicode8(m_strIP.c_str()), m_dwPort);
			}

			Sleep(100);
			continue;	// 重新检测连接
		}

		print_message(_T("Contected to Bill Server at %s: %d\r\n"), m_strIP.c_str(), m_dwPort);


		tagNWB_Login send;
		_tcsncpy(send.sz_world_name, g_world.GetWorldName(), cal_tchar_array_num(send.sz_world_name) - 1);
		m_pTran->send_msg(&send, send.dw_size);

		break;
	}

//#ifdef _DEBUG
	THROW_EXCEPTION_END;
//#endif
	return 0;
}

UINT BillSession::static_thread_connect(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	BillSession* p_this = (BillSession*)p_data;
	return p_this->thread_connect();
}

//----------------------------------------------------------------------------------------------
// 注册所有的网络命令
//----------------------------------------------------------------------------------------------
VOID BillSession::RegisterAllBillCommand()
{
	REGISTER_NET_MSG("NBW_Login",			BillSession, HandleServerLogin,	_T("Bill Server Certification Reply"));
	REGISTER_NET_MSG("NBW_PickBaiBao",	BillSession, HandlePickBaiBao,	_T("Notice DB & Client to Reflesh BaiBao"));
	REGISTER_NET_MSG("NBW_ChangeReceive", BillSession, HandleChangeReceive, _T(""));
	REGISTER_NET_MSG("NBW_ReloadAutoPaimai", BillSession, HandleReloadAutoPaimai, _T(""));
}

//----------------------------------------------------------------------------------------------
// 服务器认证网络消息
//----------------------------------------------------------------------------------------------
DWORD BillSession::HandleServerLogin(tag_net_message* p_message, DWORD)
{
	tagNBW_Login* p_receive = (tagNBW_Login*)p_message;

	if (p_receive->dw_error_code == E_Success)
	{
		Interlocked_Exchange((LPLONG)&m_bInitOK, TRUE);
	}
	else
	{
		m_pTran->disconnect();
		Interlocked_Exchange((LPLONG)&m_bInitOK, FALSE);
	}
	return 0;
}

//----------------------------------------------------------------------------------------------
// 刷新百宝袋内容
//----------------------------------------------------------------------------------------------
DWORD BillSession::HandlePickBaiBao( tag_net_message* p_message, DWORD )
{
	tagNBW_PickBaiBao* p_receive = (tagNBW_PickBaiBao*)p_message;

	// 找到指定账号的在线玩家
	PlayerSession* pPlayerSession = g_worldSession.FindSession(p_receive->dw_account_id);
	
	// 账号不在线
	if (!VALID_POINT(pPlayerSession))
		return INVALID_VALUE;


	// 更新百宝袋中元宝的数量
	NET_DB2C_load_baibao_yuanbao SendYuanBao;
	SendYuanBao.dw_account_id = p_receive->dw_account_id;
	SendYuanBao.dw_recharge_num = p_receive->dw_recharge_num;
	g_dbSession.Send(&SendYuanBao, SendYuanBao.dw_size);

	// 在游戏中
	//Role* pRole = pPlayerSession->GetRole();
	//if (VALID_POINT(pRole))
	//{
	//	// 如果百宝袋中有空位，则申请检查是否还有未放入的物品
	//	if(pRole->GetItemMgr().GetBaiBaoFreeSize() > 0)
	//	{
	//		NET_DB2C_load_baibao send;
	//		send.n64_serial = p_receive->n64_serial;
	//		send.dw_account_id = p_receive->dw_account_id;
	//		send.n_free_space_size_ = pRole->GetItemMgr().GetBaiBaoFreeSize();

	//		g_dbSession.Send(&send, send.dw_size);
	//	}
	//}

	return 0;
}

DWORD BillSession::HandleChangeReceive(tag_net_message* p_message, DWORD)
{
	tagNBW_ChangeReceive* p_recv = (tagNBW_ChangeReceive*)p_message;

	// 找到指定账号的在线玩家
	PlayerSession* pPlayerSession = g_worldSession.FindSession(p_recv->dw_account_id);

	// 账号不在线
	if (!VALID_POINT(pPlayerSession))
		return INVALID_VALUE;

	NET_DB2C_load_web_receive send;
	send.dw_account_id = p_recv->dw_account_id;
	g_dbSession.Send(&send, send.dw_size);

	return 0;
}

DWORD BillSession::HandleReloadAutoPaimai(tag_net_message* p_message, DWORD)
{
	g_auto_paimai.reload_auto_paimai();
	return 0;
}

BillSession g_billSession;
