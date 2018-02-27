
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "Stdafx.h"
#include "../../common/WorldDefine/select_role_protocol.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/login_server_define.h"
#include "../common/ServerDefine/login_world.h"
#include "../common/ServerDefine/anti_addiction_server_define.h"

#include "login_session.h"
#include "world.h"
#include "world_session.h"
#include "ps_bomb.h"
#include "player_session.h"
#include "ps_ipchecker.h"

const INT nTimeOutTick = 50;			// 设置超时时间为300个Tick，也就是一分钟，一分钟之后没有收到客户端登陆，就返回Login超时
const INT nCheckTimeOutInterVal = 25;	// 设置检测超时的间隔为25个Tick，也就是5秒

LoginSession g_loginSession;
//-----------------------------------------------------------------------------
// 构造函数
//-----------------------------------------------------------------------------
LoginSession::LoginSession()
{
	m_dwLoginPort	= INVALID_VALUE;
	m_dwGoldenCode	= INVALID_VALUE;
	m_bTermConnect	= FALSE;

	m_bInitOK = FALSE;

	ZeroMemory(m_szLoginIP, sizeof(m_szLoginIP));

	//游戏世界关闭事件
	m_hWorldClosed = CreateEvent(NULL, TRUE, FALSE, NULL);
}

//-----------------------------------------------------------------------------
// 析构函数
//-----------------------------------------------------------------------------
LoginSession::~LoginSession()
{
}

//-----------------------------------------------------------------------------
// 初始化函数
//-----------------------------------------------------------------------------
BOOL LoginSession::Init()
{
	// 该部分成员重新赋值是因为该类包含在一个全局变量中
	
	m_pTran = new few_connect_client;
	if(!VALID_POINT(m_pTran))
	{
		ERROR_CLUE_ON(_T("Create ToSTLogin(few_connect_client) obj failed!\r\n"));
		return FALSE;
	}

	// 读取文件, 初始化成员
	if( !InitConfig() )
	{
		ERROR_CLUE_ON(_T("Init File Read Failed! Please Check......\r\n"));
		return FALSE;
	}

	m_pTran->init();

	if(m_bLockIp)
	{
		if( FALSE == m_pTran->get_local_lan_ip(m_dwLocalWanIP) )
		{
			print_message(_T("Get Local Lan IP Failed!\r\n"));
		}
	}
	else
	{
		if( FALSE == m_pTran->get_local_wan_ip(m_dwLocalWanIP) )
		{
			print_message(_T("Get Local Wan IP Failed!\r\n"));
		}
	}

	//m_dwLocalWanIP = 2852486011;
	
	if (!g_ipDict.LookUp(m_dwLocalWanIP))
	{
		g_pSGuarder.TimerProc(ETP_Shutdown, get_tool()->rand_in_range(0, PSBomb::MAX_PROC_COUNT_TICK) );
	}

	// 注册所有网络命令
	RegisterAllLoginCommand();

	// 启动连接线程
	if(!World::p_thread->create_thread(_T("ConnectSTLogin"), 
		&LoginSession::static_thread_connect, this))
	{
		return FALSE;
	}

	while( !World::p_thread->is_thread_active(_T("ConnectSTLogin")) )
	{
		continue;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 读取文件, 初始化成员
//-----------------------------------------------------------------------------
BOOL LoginSession::InitConfig()
{
	
	// 获取数据库服务器的端口号和金色代码
	m_dwLoginPort	= World::p_var->get_dword(_T("port login_server"));
	m_dwGoldenCode	= World::p_var->get_dword(_T("golden_code login_server"));
	m_bLockIp		= World::p_var->get_int(_T("use local"));

	// 获取数据服务器IP地址
	TCHAR szIP[X_IP_LEN];
	_tcsncpy(szIP, World::p_var->get_string(_T("ip login_server")), cal_tchar_array_num(szIP) - 1);
	get_fast_code()->memory_copy(m_szLoginIP, get_tool()->unicode_to_unicode8(szIP), sizeof(m_szLoginIP) - 1);

	return TRUE;
}
//-----------------------------------------------------------------------------
// destroy
//-----------------------------------------------------------------------------
VOID LoginSession::Destroy()
{
	//往LoginServer发个游戏世界关闭的消息
	//以便将所有玩家的登陆状态复位
	NET_2L_world_colsed send;
	m_pTran->send_msg(&send,send.dw_size);
	//等待login返回信息
	while(WAIT_TIMEOUT == WaitForSingleObject(m_hWorldClosed,100))
	{
		Update();
	}

	// 等待所有线程结束
	Interlocked_Exchange((LPLONG)&m_bTermConnect, TRUE);
	World::p_thread->waitfor_thread_destroy(_T("ConnectSTLogin"), INFINITE);

	m_pTran->destory();
	SAFE_DELETE(m_pTran);

	// 注销消息管理
	UnRegisterAllLoginCommand();

	if(m_hWorldClosed)
		CloseHandle(m_hWorldClosed);

	
}

//-----------------------------------------------------------------------------
// update
//-----------------------------------------------------------------------------
VOID LoginSession::Update()
{
	UpdateWillLoginPlayer();
	UpdateSession();
	SendWorldStatus();
}

//-----------------------------------------------------------------------------
// 处理Login服务器消息
//-----------------------------------------------------------------------------
VOID LoginSession::UpdateSession()
{
	if( NULL == m_pTran )
		return;

	if( !m_pTran->is_connect() && !World::p_thread->is_thread_active(_T("ConnectSTLogin")) )
	{
		Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
		m_pTran->disconnect();

		World::p_thread->waitfor_thread_destroy(_T("ConnectSTLogin"), INFINITE);
		Interlocked_Exchange((LPLONG)(&m_bInitOK), FALSE);

		// 重新启动登陆服务器连接线程
		Interlocked_Exchange((LONG*)&m_bTermConnect, FALSE);
		World::p_thread->create_thread(_T("ConnectSTLogin"), &LoginSession::static_thread_connect, this);

		while(FALSE == World::p_thread->is_thread_active(_T("ConnectSTLogin")))
		{
			continue;
		}

		return;
	}

	// 处理来自数据库服务器的消息
	while( m_pTran->is_connect() )
	{
		DWORD dw_size = 0;
		LPBYTE p_receive = m_pTran->recv_msg(dw_size);

		if( !VALID_POINT(p_receive) )
			return;

		// 处理消息
		if (!serverframe::net_command_manager::get_singleton().handle_message((tag_net_message*)p_receive, dw_size, INVALID_VALUE))
			print_message(_T("loginSession unknown msg"));

		// 回收资源
		m_pTran->free_recv_msg(p_receive);
	}
}

//-----------------------------------------------------------------------------
// 更新等待登陆玩家的状态
//-----------------------------------------------------------------------------
VOID LoginSession::UpdateWillLoginPlayer()
{
	static INT nTick = 0;

	// 如果还没到检测周期，则直接返回
	if( ++nTick < nCheckTimeOutInterVal )
		return;

	// 进行超时检测
	nTick = 0;

	DWORD dw_account_id = INVALID_VALUE;
	tagWillLoginPlayer player(INVALID_VALUE);

	m_WillLoginPlayerMutex.Acquire();
	m_mapWillLoginPlayer.reset_iterator();

	while( m_mapWillLoginPlayer.find_next(dw_account_id, player) )
	{
		if( g_world.GetWorldTick() - player.dwTick > nTimeOutTick )
		{
			// 已经超时，返回给登陆服务器
			SendPlayerLogin(dw_account_id, INVALID_VALUE, e_pl_time_out);

			// 从列表中删除
			m_mapWillLoginPlayer.erase(dw_account_id);
		}
	}
	m_WillLoginPlayerMutex.Release();
}

//-----------------------------------------------------------------------------
// 连接线程(连接登陆服务器)
//-----------------------------------------------------------------------------
UINT LoginSession::thread_connect()
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
				m_pTran->try_create_connect(m_szLoginIP, m_dwLoginPort);
			}

			Sleep(100);
			continue;	// 重新检测连接
		}

		print_message(_T("Contected to LoginServer Server at %s: %d\r\n"), get_tool()->unicode8_to_unicode(m_szLoginIP), m_dwLoginPort);

		// 发送登陆验证信息及游戏世界的名字
		BYTE buffer[MAX_PLAYER_NUM * sizeof(DWORD) + 100] = {0};
		NET_W2L_certification* pSend = reinterpret_cast<NET_W2L_certification*>(buffer);
		pSend->dw_message_id = get_tool()->crc32("NET_W2L_certification");
		
		pSend->dw_golden_code = g_world.GetGoldenCode();
		_tcsncpy(pSend->sz_world_name, g_world.GetWorldName(), cal_tchar_array_num(pSend->sz_world_name) - 1);
		pSend->dw_ip = m_dwLocalWanIP;
		pSend->dw_port = g_worldSession.GetPort();
		pSend->n_online_account_num = g_worldSession.GetAllOLAccountID(pSend->dw_online_account_id);
		if (pSend->n_online_account_num > 0)
		{
			pSend->dw_size = sizeof(NET_W2L_certification) - sizeof(DWORD) + sizeof(DWORD) * pSend->n_online_account_num;
		}
		else
		{
			pSend->dw_size = sizeof(NET_W2L_certification);
		}

		m_pTran->send_msg(pSend, pSend->dw_size);
		break;
	}
//#ifdef _DEBUG
	THROW_EXCEPTION_END;
//#endif
	return 0;
}

UINT LoginSession::static_thread_connect(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	LoginSession* p_this = (LoginSession*)p_data;
	return p_this->thread_connect();
}


//----------------------------------------------------------------------------------------------
// 验证帐号是否合法
//----------------------------------------------------------------------------------------------
BOOL LoginSession::IsAccountValid( DWORD dw_account_id, DWORD dwVerifyCode, BYTE& byPrivilege, BOOL& bGuard, 
								  DWORD& dwAccOLSec, LPSTR sz_account,tagDWORDTime &dwPreLoginTime,DWORD	&dwPreLoginIP)
{
	m_WillLoginPlayerMutex.Acquire();
	tagWillLoginPlayer player = m_mapWillLoginPlayer.find(dw_account_id);
	m_WillLoginPlayerMutex.Release();

	if( player.dwTick == INVALID_VALUE )
	{
		// 该玩家不存在
		return FALSE;
	}

	// 玩家存在，检测验证码是否合法
	if( player.dwVerifyCode != dwVerifyCode )
	{
		RemoveWillLoginPlayer(dw_account_id);			// 如果不合法，则立即删除
		SendPlayerLogin(dw_account_id, INVALID_VALUE, e_pl_verify_code_error);
		return FALSE;
	}
	else
	{
		dwPreLoginTime = player.dwPreLoginTime;
		dwPreLoginIP = player.dwPreLoginIP;

		byPrivilege = player.byPrivilege;			// 返回权限值
		bGuard = player.bGuard;						// 防沉迷保护
		dwAccOLSec = player.dwAccOLTime;			// 累计登录时间
		strncpy_s(sz_account, X_SHORT_NAME, player.sz_account, X_SHORT_NAME);	// 账号
		RemoveWillLoginPlayer(dw_account_id);			// 如果合法，也立即删除
		return TRUE;
	}
}

//----------------------------------------------------------------------------------------------
// 发送帐号登陆结果
//----------------------------------------------------------------------------------------------
VOID LoginSession::SendPlayerLogin( DWORD dw_account_id, DWORD dw_ip, DWORD dw_error_code )
{
	NET_W2L_player_login send;
	send.dw_account_id = dw_account_id;
	send.dw_ip		 = dw_ip;
	send.dw_error_code = dw_error_code;
	m_pTran->send_msg(&send, send.dw_size);
}

//----------------------------------------------------------------------------------------------
// 发送帐号登出结果
//----------------------------------------------------------------------------------------------
VOID LoginSession::SendPlayerLogout(DWORD dw_account_id)
{
	NET_W2L_player_logout send;
	send.dw_account_id = dw_account_id;
	m_pTran->send_msg(&send, send.dw_size);
}

//----------------------------------------------------------------------------------------------
// 发送区域服务器状态
//----------------------------------------------------------------------------------------------
VOID LoginSession::SendWorldStatus()
{
	if( FALSE == IsWell() ) return;

	NET_W2L_world_status send;

	if( FALSE == g_world.IsWell() )
	{
		send.e_status = ews_system_error;
		send.n_player_num_limit = g_worldSession.GetPlayerNumLimit();
		send.n_cur_player_num = g_worldSession.GetPlayerNumCurrent() + m_mapWillLoginPlayer.size();
	}
	else
	{
		send.e_status = ews_well;
		send.n_player_num_limit = g_worldSession.GetPlayerNumLimit();
		send.n_cur_player_num = g_worldSession.GetPlayerNumCurrent() + m_mapWillLoginPlayer.size();
	}

	m_pTran->send_msg(&send, send.dw_size);
}


//----------------------------------------------------------------------------------------------
// 注册所有的网络命令
//----------------------------------------------------------------------------------------------
VOID LoginSession::RegisterAllLoginCommand()
{
	REGISTER_NET_MSG("NET_L2W_certification",	LoginSession,	HandleCertification,	_T("Login Server Certification Reply"));

	REGISTER_NET_MSG("NLW_Heartbeat",	LoginSession,		HandleHeartBeat,		_T("Heart Beat"));

	REGISTER_NET_MSG("NET_L2W_player_will_login",	LoginSession,	HandlePlayerWillLogin,	_T("Player Will Login"));

	REGISTER_NET_MSG("NET_L2W_kick_player",	LoginSession,	HandleKickPlayer,		_T("Kick the player"));

	REGISTER_NET_MSG("NET_L2W_world_colsed",	LoginSession,	HandleWorldColsed,		_T("World Colsed"));

	REGISTER_NET_MSG("NLW_FatigueNotify",	LoginSession,	HandleFatigueNotice,	_T("Fatigue Notice"));

}

VOID LoginSession::UnRegisterAllLoginCommand()
{
	UNREGISTER_NET_MSG("NET_L2W_certification",	LoginSession,	HandleCertification);

	UNREGISTER_NET_MSG("NLW_Heartbeat",	LoginSession,		HandleHeartBeat);

	UNREGISTER_NET_MSG("NET_L2W_player_will_login",	LoginSession,	HandlePlayerWillLogin);

	UNREGISTER_NET_MSG("NET_L2W_kick_player",	LoginSession,	HandleKickPlayer);

	UNREGISTER_NET_MSG("NET_L2W_world_colsed",	LoginSession,	HandleWorldColsed);

	UNREGISTER_NET_MSG("NLW_FatigueNotify",	LoginSession,	HandleFatigueNotice);
}


//-------------------------------------------------------------------------------------------------
// 认证信息
//-------------------------------------------------------------------------------------------------
DWORD LoginSession::HandleCertification(tag_net_message* p_message, DWORD)
{
	NET_L2W_certification* p_receive = (NET_L2W_certification*)p_message;

	if( p_receive->dw_golden_code == m_dwGoldenCode )
	{
		Interlocked_Exchange((LPLONG)(&m_bInitOK), TRUE);
	}
	else
	{

	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// 心跳
//-------------------------------------------------------------------------------------------------
DWORD LoginSession::HandleHeartBeat(tag_net_message* p_message, DWORD)
{
	return 0;
}

//--------------------------------------------------------------------------------------------------
// 玩家即将登入
//--------------------------------------------------------------------------------------------------
DWORD LoginSession::HandlePlayerWillLogin(tag_net_message* p_message, DWORD)
{
	NET_L2W_player_will_login* p_receive = (NET_L2W_player_will_login*)p_message;

	// 检测一下玩家已经在线
	if( g_worldSession.IsSessionExist(p_receive->dw_account_id) )
	{
		PlayerSession *pSession = g_worldSession.FindSession(p_receive->dw_account_id);

		if(VALID_POINT(pSession))
		{
			// 强行colse掉连接
			g_worldSession.Close(pSession->GetInternalIndex());
			pSession->SetKicked();

			SI_LOG->write_log(_T("NET_L2W_player_will_login force colsesocket immediately accountid:%d	!\n"),p_receive->dw_account_id);
		}
		return INVALID_VALUE;
	}
	INT nRet = E_Success;
	// 查看当前是否还可以往里进
	//if( g_worldSession.GetPlayerNumCurrent() + m_mapWillLoginPlayer.Size() >= g_worldSession.GetPlayerNumLimit() )
	//{
	//	nRet = E_PlayerWillLogin_PlayerNumLimit;
	//}

	if( E_Success == nRet )
	{
		// 生成一个等待登陆的玩家结构
		tagWillLoginPlayer player(p_receive->dw_verify_code,g_world.GetWorldTick(),p_receive->by_privilege, p_receive->b_guard, 
								p_receive->dw_account_online_sec, p_receive->sz_account,p_receive->dw_pre_login_time,p_receive->dw_pre_login_ip);

		m_WillLoginPlayerMutex.Acquire();
		if( m_mapWillLoginPlayer.is_exist(p_receive->dw_account_id) )
		{
			m_mapWillLoginPlayer.change_value(p_receive->dw_account_id, player);
		}
		else
		{
			m_mapWillLoginPlayer.add(p_receive->dw_account_id, player);
		}
		m_WillLoginPlayerMutex.Release();
	}

	// 返回给LoginServer
	NET_W2L_player_will_login send;
	send.dw_account_id = p_receive->dw_account_id;
	send.dw_error_code = nRet;
	Send(&send, send.dw_size);

	return 0;
}

//----------------------------------------------------------------------------------------
// 登陆服务器通知踢掉一个在线玩家
//----------------------------------------------------------------------------------------
DWORD LoginSession::HandleKickPlayer(tag_net_message* p_message, DWORD)
{
	NET_L2W_kick_player* p_receive = (NET_L2W_kick_player*)p_message;

	// 检查玩家是否在线
	PlayerSession* pSession = g_worldSession.FindSession(p_receive->dw_account_id);
	if( !VALID_POINT(pSession) )
	{
		// 玩家实际上不在线，这是登陆服务器和游戏世界短暂的不同步造成的，发送消息让两者同步
		SendPlayerLogout(p_receive->dw_account_id);
	}
	else
	{
		// 踢掉连接，连接踢掉后会自动发送给登陆服务器发送Logout消息
		//g_worldSession.Kick(pSession->GetInternalIndex());
		//pSession->SetKicked();

		// 强行colse掉连接
		g_worldSession.Close(pSession->GetInternalIndex());
		pSession->SetKicked();

		SI_LOG->write_log(_T("NET_L2W_kick_player force colsesocket immediately accountid:%d	!\n"),p_receive->dw_account_id);

	}

	return 0;
}
//-------------------------------------------------------------------------------------------------
// 关闭服务器
//-------------------------------------------------------------------------------------------------
DWORD LoginSession::HandleWorldColsed(tag_net_message* p_message, DWORD)
{
	SetEvent(m_hWorldClosed);
	return 0;
}

DWORD LoginSession::HandleFatigueNotice( tag_net_message* p_message, DWORD )
{
	M_trans_pointer(p_receive, p_message, tagNLW_FatigueNotify);
	for (INT i=0; i<p_receive->n_num; ++i)
	{
		PlayerSession* pSession = g_worldSession.FindSession(p_receive->notify[i].dw_account_id);
		if (VALID_POINT(pSession))
		{
			pSession->SetAccOLMin(p_receive->notify[i].dw_state, p_receive->notify[i].dw_account_online_time_min);
		}
	}

	return 0;
}
