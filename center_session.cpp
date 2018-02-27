#include "StdAfx.h"
#include "center_session.h"
#include "player_session.h"
#include "world_session.h"
#include "World.h"
#include "../common/ServerDefine/paimai_define.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../../common/WorldDefine/protocol_common_errorcode.h"
#include "../../common/WorldDefine/verification_protocol.h"

center_session::center_session(void)
{
	m_bInitOK					=	FALSE;
	m_bTermConnect				=	FALSE;
	p_memory_pool = new memorysystem::MemPool(2 * 1024);
}

center_session::~center_session(void)
{
	SAFE_DELETE(p_memory_pool);
}

BOOL center_session::Init()
{
	m_strIP			=	World::p_var->get_string(_T("ip center_server"));
	m_dwPort		=	World::p_var->get_dword(_T("port center_server"));
	m_dwSectionID	=	World::p_var->get_dword(_T("section_id world"));
	m_dwWorldID		=	World::p_var->get_dword(_T("id world"));

	m_pTran = new few_connect_client;
	if( !VALID_POINT(m_pTran) )
	{
		ERROR_CLUE_ON(_T("初始化交易服务器连线失败!\r\n"));
		return FALSE;
	}
	m_pTran->init();

	// 注册所有网络命令
	RegisterAllCommand();

	// 启动连接线程
	if(!World::p_thread->create_thread(_T("ConnectExchange"), 
		&center_session::static_thread_connect, this))
	{
		return FALSE;
	}

	while( !World::p_thread->is_thread_active(_T("ConnectExchange")) )
	{
		continue;
	}
	return TRUE;
}

VOID center_session::Destroy()
{
	// 等待所有线程结束
	Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
	World::p_thread->waitfor_thread_destroy(_T("ConnectExchange"), INFINITE);

	UnRegisterAllCommand();

	m_pTran->destory();
	SAFE_DELETE(m_pTran);

}

VOID center_session::Update()
{
	UpdateSession();
}

VOID center_session::RegisterAllCommand()
{
	REGISTER_NET_MSG("NET_P2G_Login",		center_session, HandleServerLogin,	_T("Exchange Login Result"));
	REGISTER_NET_MSG("NET_C2W_verify_code",	center_session, HandleVerifyCode,	_T("receive verify code"));
}

VOID center_session::UnRegisterAllCommand()
{
	UNREGISTER_NET_MSG("NET_P2G_Login",			center_session, HandleServerLogin);
	UNREGISTER_NET_MSG("NET_C2W_verify_code",	center_session, HandleVerifyCode);
}

VOID center_session::UpdateSession()
{
	if( NULL == m_pTran )
		return;

	if( !m_pTran->is_connect() && !World::p_thread->is_thread_active(_T("ConnectExchange")) )
	{
		Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
		m_pTran->disconnect();

		World::p_thread->waitfor_thread_destroy(_T("ConnectExchange"), INFINITE);

		// 重新启动登陆服务器连接线程
		Interlocked_Exchange((LONG*)&m_bTermConnect, FALSE);
		World::p_thread->create_thread(_T("ConnectExchange"), &center_session::static_thread_connect, this);

		while(FALSE == World::p_thread->is_thread_active(_T("ConnectExchange")))
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
			print_message(_T("center unknown msg"));

		// 回收资源
		m_pTran->free_recv_msg(p_receive);
	}	
}

UINT center_session::thread_connect()
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

		print_message(_T("交易服务器连接成功 %s: %d\r\n"), m_strIP.c_str(), m_dwPort);

		NET_G2P_Login send;
		_tcsncpy(send.sz_ServerName, g_world.GetWorldName(), cal_tchar_array_num(send.sz_ServerName) - 1);
		m_pTran->send_msg(&send, send.dw_size);

		break;
	}

	//#ifdef _DEBUG
	THROW_EXCEPTION_END;
	//#endif
	return 0;
}

UINT center_session::static_thread_connect(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	center_session* p_this = (center_session*)p_data;
	return p_this->thread_connect();
}

DWORD center_session::HandleServerLogin(tag_net_message* p_message, DWORD)
{
	NET_P2G_Login* pRecv = (NET_P2G_Login*)p_message;

	if(pRecv->dw_Error == E_Success)
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

DWORD center_session::HandleVerifyCode(tag_net_message* p_message, DWORD)
{
	NET_C2W_verify_code* pRecv = (NET_C2W_verify_code*)p_message;
	
	PlayerSession* pSession = g_worldSession.FindGlobalSession(pRecv->dw_account_id);

	if( !VALID_POINT(pSession) ) 
	{
		pSession = g_worldSession.FindSession(pRecv->dw_account_id);
	}

	if( !VALID_POINT(pSession) ) 
		return INVALID_VALUE;
	
	if (pRecv->dw_error_code == INVALID_VALUE)
	{
		pSession->GetVerification().reset();
		return 0;
	}
	if (pSession->m_bRoleVerifying)
	{
		//pSession->GetVerification().setIndex(pRecv->dw_param);
		pSession->GetVerification().setAnswer(pRecv->dw_answer_crc);

		// 发给客户端
		int nMsgSize = sizeof(NET_SIS_reset_verification_code) - sizeof(BYTE) + sizeof(BYTE) * pRecv->n_verify_code_size;
		BYTE* pBuff = (BYTE*)p_memory_pool->alloc_memory(nMsgSize);
		ZeroMemory(pBuff, nMsgSize);
			
		NET_SIS_reset_verification_code* pSend = (NET_SIS_reset_verification_code*)pBuff;
		pSend->dw_message_id = pSend->message_id_crc("NET_SIS_reset_verification_code");
		pSend->n_verify_code_size = pRecv->n_verify_code_size;
		memcpy(pSend->by_code, pRecv->by_code, nMsgSize);
		pSend->byVerificationCodeIndex = pRecv->dw_param;
		pSend->dwType = pRecv->dw_type;
		pSend->dw_size = nMsgSize;
		pSession->SendMessage(pSend, pSend->dw_size);

		pSession->m_bRoleVerifying = false;

		p_memory_pool->free_memory(pBuff);
	}
	return 0;
}
center_session g_center_session;
