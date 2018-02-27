
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//服务器客户端主框架

#include "StdAfx.h"
#include "world_session.h"
#include "login_session.h"
#include "player_session.h"
#include "world.h"
#include "game_guarder.h"
#include "../../common/WorldDefine/select_role_protocol.h"
#include "../../common/WorldDefine/TreasureChest_define.h"

const INT MAX_RECV_WAIT_UNIT_NUM = 64;		// 客户端等待处理的包的最大数量，超过数量将被踢掉
const INT MAX_SEND_CAST_UNIT_NUM = 1000;	// 发送给客户端但还没有收到响应的包数量，超过数量将被踢掉
const INT MAX_SEND_CAST_SIZE = 2*1024*1024;	// 发送给客户端但还没有收到相应的字节数量，超过即被踢掉
const INT MAX_SEND_WAIT_SEND_NUM = 3000;	// 将要发送的包的数量超过5000，超过即被踢掉

//-------------------------------------------------------------------------------
// constructor
//-------------------------------------------------------------------------------
WorldSession::WorldSession()
{
}

//-------------------------------------------------------------------------------
// destructor
//-------------------------------------------------------------------------------
WorldSession::~WorldSession()
{

}

//-------------------------------------------------------------------------------
// 得到一个session（要注意指针得到后的安全性）
//-------------------------------------------------------------------------------
PlayerSession* WorldSession::FindSession(DWORD dwID)
{
	m_AllSessionMutex.Acquire();
	PlayerSession* pSession = m_mapAllSessions.find(dwID);
	m_AllSessionMutex.Release();

	return pSession;
}

//--------------------------------------------------------------------------------
// 得到一个选人界面的session（只能在主线程中调用）
//--------------------------------------------------------------------------------
PlayerSession* WorldSession::FindGlobalSession(DWORD dwID)
{
	m_GlobalSessionMutex.Acquire();
	PlayerSession* pSession = m_mapGlobalSessions.find(dwID);
	m_GlobalSessionMutex.Release();

	return pSession;
}

//--------------------------------------------------------------------------------
// 检测一个sessionID是否存在
//--------------------------------------------------------------------------------
BOOL WorldSession::IsSessionExist(DWORD dwID)
{
	m_AllSessionMutex.Acquire();
	BOOL bExist = m_mapAllSessions.is_exist(dwID);
	m_AllSessionMutex.Release();

	return bExist;
}

//--------------------------------------------------------------------------------
// 加入一个新的session
//-------------------------------------------------------------------------------
VOID WorldSession::AddSession(PlayerSession* pSession)
{ 
	if( VALID_POINT(pSession) )
	{
		m_AllSessionMutex.Acquire();
		m_mapAllSessions.add(pSession->GetSessionID(), pSession);
		m_AllSessionMutex.Release();
	}
}
//-------------------------------------------------------------------------------
// 删除一个session
//-------------------------------------------------------------------------------
VOID WorldSession::RemoveSession(UINT32 dwID)
{
	m_AllSessionMutex.Acquire();
	m_mapAllSessions.erase(dwID);
	m_AllSessionMutex.Release();
}

//-------------------------------------------------------------------------------
// 踢掉一个连接
//-------------------------------------------------------------------------------
VOID WorldSession::Kick(DWORD dwInternalIndex)
{
	m_pNetSession->kick_client(dwInternalIndex);
}
//-------------------------------------------------------------------------------
// 强行踢掉一个连接
//-------------------------------------------------------------------------------
VOID WorldSession::Close(DWORD dwInternalIndex)
{
	m_pNetSession->clost_client(dwInternalIndex);
}
//-------------------------------------------------------------------------------
// 向选人界面session中加入一个session
//-------------------------------------------------------------------------------
VOID WorldSession::AddGlobalSession(PlayerSession* pSession)
{
	if( VALID_POINT(pSession) )
	{
		if( m_GlobalSessionMutex.TryAcquire() )
		{
			m_mapGlobalSessions.add(pSession->GetSessionID(), pSession);
			m_GlobalSessionMutex.Release();
		}
		else
		{
			m_InsertPoolMutex.Acquire();
			m_listInsertPool.push_back(pSession);
			m_InsertPoolMutex.Release();
		}
	}
}

//-------------------------------------------------------------------------------
// 从选人界面session列表中删除一个session
//-------------------------------------------------------------------------------
VOID WorldSession::RemoveGlobalSession(DWORD dwSessionID)
{
	m_GlobalSessionMutex.Acquire();
	m_mapGlobalSessions.erase(dwSessionID);
	m_GlobalSessionMutex.Release();
}

//-------------------------------------------------------------------------------
// 初始化
//-------------------------------------------------------------------------------
BOOL WorldSession::Init()
{
	
	m_pNetSession = new IOCP;
	tstring log_name = create_network_log();
	m_pNetSession->get_log()->create_log(log_name.c_str());

	// 加载配置文件
	m_nPlayerNumLimit = (INT)World::p_var->get_dword(_T("player_num_limit"), _T("world"));
	m_nPort = (INT)World::p_var->get_dword(_T("port"), _T("server"));

	// 设定网络底层
	tag_server_config InitParam;
	InitParam.fn_login		=	fastdelegate::MakeDelegate(this, &WorldSession::LoginCallBack);
	InitParam.fn_logout		=	fastdelegate::MakeDelegate(this, &WorldSession::LogoutCallBack);
	InitParam.b_repeat_port	=	true;
	InitParam.n_port			=	m_nPort;

	m_pNetSession->init(InitParam);
	m_nPort = m_pNetSession->get_config()->n_port;
	m_nTreasureSum = 0;

	// 注册所有客户端命令
	PlayerSession::RegisterAllPlayerCmd();
	// 注册所有服务器端发送的命令
	PlayerSession::RegisterALLSendCmd();

	return TRUE;
}

//-------------------------------------------------------------------------------
// 更新
//-------------------------------------------------------------------------------
VOID WorldSession::Update()
{
	Interlocked_Exchange((LPLONG)&m_nMsgSendThisTick, 0);
	Interlocked_Exchange((LPLONG)&m_nMsgProceedThisTick, 0);
	Interlocked_Exchange((LPLONG)&m_nMsgRecvWait, 0);

	// 从InsertPool中拿出要加入的session
	m_InsertPoolMutex.Acquire();
	PlayerSession* pSession = m_listInsertPool.pop_front();
	while( VALID_POINT(pSession) )
	{
		AddGlobalSession(pSession);
		pSession = m_listInsertPool.pop_front();
	}
	m_InsertPoolMutex.Release();

	// 更新选人界面的session
	UpdateSession();
}

//-------------------------------------------------------------------------------
// 更新各个客户端session
//-------------------------------------------------------------------------------
VOID WorldSession::UpdateSession()
{
	// 更新所有选人界面的session
	m_GlobalSessionMutex.Acquire();

	PlayerSession* pSession = NULL;
	m_mapGlobalSessions.reset_iterator();

	while( m_mapGlobalSessions.find_next(pSession) )
	{
		if( CON_LOST == pSession->Update())
		{
			// print_message(_T("player logout in worldsession, sessionid=%u, internalid=%u\r\n"), pSession->GetSessionID(), pSession->GetInternalIndex());

			RemoveSession(pSession->GetSessionID());
			m_mapGlobalSessions.erase(pSession->GetSessionID());
			g_loginSession.SendPlayerLogout(pSession->GetSessionID());

			PlayerLogout();

			g_gameGuarder.Logout(pSession->GetSessionID(), pSession->GetAccount());
			SAFE_DELETE(pSession);
		}
	}

	m_GlobalSessionMutex.Release();
}

//-------------------------------------------------------------------------------
// 做一些网络层的清理工作
//-------------------------------------------------------------------------------
VOID WorldSession::DoHouseKeeping()
{
	// 启动底层网络层开始发送
	m_pNetSession->active_send();

	// 轮询所有session
	m_AllSessionMutex.Acquire();

	PlayerSession* pSession = NULL;
	package_map<DWORD, PlayerSession*>::map_iter it = m_mapAllSessions.begin();

	INT nAllSendCast = 0;
	INT nMaxChokeSize = 0;
	DWORD dwMaxChokeSessionID = INVALID_VALUE;

	while( m_mapAllSessions.find_next(it, pSession) )
	{
		// 检查客户端接受队列的消息数量
		if( pSession->GetMsgWaitNum() >= MAX_RECV_WAIT_UNIT_NUM && !pSession->IsKicked() )
		{
			print_message(_T("Kick Too Fast Player[%u, %d]\r\n"), pSession->GetSessionID(), pSession->GetMsgWaitNum());
			Kick(pSession->GetInternalIndex());
			pSession->SetKicked();

			pSession->Log_Free_Message();
			continue;
		}

		/*INT n_send_num = GetSendNum(pSession->GetInternalIndex());
		if(n_send_num >= MAX_SEND_WAIT_SEND_NUM && !pSession->IsKicked())
		{
			print_message(_T("Kick Send NumChoke Player[%u, %u, %d]\r\n"), pSession->GetSessionID(), pSession->GetInternalIndex(), n_send_num);
			Kick(pSession->GetInternalIndex());
			pSession->SetKicked();
			continue;
		}*/

		// 检查该客户端未收到确认的包的数量
		INT nCastSize = GetSendCastSize(pSession->GetInternalIndex());
		nAllSendCast += nCastSize;

		if( nCastSize >= MAX_SEND_CAST_SIZE && !pSession->IsKicked() )
		{
			print_message(_T("Kick Choke Player[%u, %u, %d]\r\n"), pSession->GetSessionID(), pSession->GetInternalIndex(), nCastSize);
			Kick(pSession->GetInternalIndex());
			pSession->SetKicked();
		}
		else if( nCastSize > nMaxChokeSize )
		{
			nMaxChokeSize = nCastSize;
			dwMaxChokeSessionID = pSession->GetSessionID();
		}
	}

	// todo: 如果出现内存不足等情况，则踢掉最卡的玩家

	// 设置本Tick的SendCast量
	SetMsgSendCast(nAllSendCast);

	m_AllSessionMutex.Release();
}

//-------------------------------------------------------------------------------
// 销毁
//-------------------------------------------------------------------------------
VOID WorldSession::Destroy()
{
	// 删除所有的session（因为所有线程已经停止，所以这里不需要锁定）
	PlayerSession* pSession = m_listInsertPool.pop_front();
	while( VALID_POINT(pSession) )
	{
		g_gameGuarder.Logout(pSession->GetSessionID(), pSession->GetAccount());
		SAFE_DELETE(pSession);
		pSession = m_listInsertPool.pop_front();
	}

	m_mapGlobalSessions.clear();
	
	m_mapAllSessions.reset_iterator();
	while( m_mapAllSessions.find_next(pSession) )
	{
		g_gameGuarder.Logout(pSession->GetSessionID(), pSession->GetAccount());
		SAFE_DELETE(pSession);
	}
	m_mapAllSessions.clear();

	m_pNetSession->destroy();
	SAFE_DELETE(m_pNetSession);

	// 撤销所有注册的网络命令和GM命令
	PlayerSession::UnRegisterALL();

	
}

//-------------------------------------------------------------------------------
// 登陆回调函数
//-------------------------------------------------------------------------------
UINT WorldSession::LoginCallBack(tag_unit* pUnit, tag_login_param* pParam)
{
	static DWORD dwJoinGameCmdCrc = get_tool()->crc32("NET_SIC_join_game");

	if( g_world.IsShutingdown() )
		return INVALID_VALUE;

	// 查看第一条消息

	
	NET_SIC_join_game* pCmd = (NET_SIC_join_game*)pUnit->p_buffer;

	if( pCmd->dw_message_id != dwJoinGameCmdCrc )
	{
		return INVALID_VALUE;
	}

	// 锁住网络
	g_world.LockNetWork();

	// 登陆服务器判断帐号是否合法
	BYTE byPrivilege = 0;
	BOOL bGuard = true;
	DWORD dwAccOLSec = 0;
	char sz_account[X_SHORT_NAME] = {0};

	tagDWORDTime dwPreLoginTime;
	DWORD	dwPreLoginIP=0;
	if( FALSE == g_loginSession.IsAccountValid(pCmd->dw_account_id, pCmd->dwVerifyCode, byPrivilege, bGuard, 
												dwAccOLSec, sz_account,dwPreLoginTime,dwPreLoginIP) )
	{
		g_world.UnlockNetWork();
		return INVALID_VALUE;
	}

	if( IsSessionExist(pCmd->dw_account_id) )
	{
		g_world.UnlockNetWork();
		return INVALID_VALUE;
	}

	// 加入一个新的session
	PlayerSession* pSession = new PlayerSession(pCmd->dw_account_id, pParam->dw_handle, pParam->dw_address, byPrivilege,
												bGuard, dwAccOLSec, sz_account,dwPreLoginTime,dwPreLoginIP);

	// 分别加入到总session和globlesession中
	AddSession(pSession);
	AddGlobalSession(pSession);

	// 设置登陆
	PlayerLogin();

	// 通知登陆服务器帐号登陆成功
	g_loginSession.SendPlayerLogin(pCmd->dw_account_id, pParam->dw_address, E_Success);

	// 反外挂记录
	g_gameGuarder.Login(pCmd->dw_account_id, sz_account, pParam->dw_address);

	// 解锁
	g_world.UnlockNetWork();

	return pSession->GetSessionID();
}

//--------------------------------------------------------------------------------------
// 登出回调函数
//--------------------------------------------------------------------------------------
UINT WorldSession::LogoutCallBack(DWORD dwSessionID)
{
	// print_message(_T("Log out callback, sessionid=%u\r\n"), dwSessionID);

	if( g_world.IsShutingdown() )
		return INVALID_VALUE;

	// 锁住网络
	g_world.LockNetWork();

	PlayerSession* pSession = FindSession(dwSessionID);

	if( VALID_POINT(pSession) )
	{
		// print_message(_T("Log out callback, sessionid=%u, internalid=%u\r\n"), pSession->GetSessionID(), pSession->GetInternalIndex());
		pSession->SessionLogout();
	}

	// 解锁网络
	g_world.UnlockNetWork();

	return 0;
}

//--------------------------------------------------------------------------------------
// 服务器宝箱开启计数加一
//--------------------------------------------------------------------------------------
VOID WorldSession::IncTreasureSum()
{
	Interlocked_Exchange_Add((LPLONG)&m_nTreasureSum, 1);
	if (m_nTreasureSum > SERVER_CHEST_RATE)
		Interlocked_Exchange((LPLONG)&m_nTreasureSum, 1);
}

//--------------------------------------------------------------------------------------
// 设置服务器宝箱开启计数
//--------------------------------------------------------------------------------------
VOID WorldSession::SetTreasureSum(INT nSum)
{
	if (nSum > SERVER_CHEST_RATE)
		nSum = 1;
	Interlocked_Exchange((LPLONG)&m_nTreasureSum, nSum);
}

//--------------------------------------------------------------------------------------
// 获取所有在线玩家AccountId
//--------------------------------------------------------------------------------------
INT WorldSession::GetAllOLAccountID( DWORD* pAccountIDs )
{
	std::list<PlayerSession*> listPlayerSessions;
	m_AllSessionMutex.Acquire();

	m_mapAllSessions.copy_value_to_list(listPlayerSessions);

	INT i=0;
	std::list<PlayerSession*>::iterator itr = listPlayerSessions.begin();
	while (itr != listPlayerSessions.end())
	{
		if (VALID_POINT(*itr)/* && (*itr)->GetFatigueGuarder().IsGuard()*/)
		{
			pAccountIDs[i++] = (*itr)->GetSessionID();
		}
		++itr;
	}

	m_AllSessionMutex.Release();

	return i;
}

tstring	WorldSession::create_network_log()
{
	tstring str_file_name = _T("log\\");

	TCHAR sz_time[MAX_PATH], sz_handle_name[MAX_PATH];
	wsprintf(sz_handle_name, _T("GameServerNetWork"));

	TCHAR *p_result = _tcsrchr(sz_handle_name, _T('\\'));
	p_result = p_result ?	p_result+1 :	p_result = (TCHAR *)sz_handle_name;

	TCHAR* p_result1 = _tcsrchr(p_result, _T('.'));
	if( p_result1 )
		*p_result1 = _T('\0');

	str_file_name += p_result;
	str_file_name += _T("_");

	FILETIME st_current_time;
	GetSystemTimeAsFileTime(&st_current_time);
	sz_time[0] = _T('\0');
	WORD w_data, w_time;
	if (FileTimeToLocalFileTime(&st_current_time, &st_current_time) &&
		FileTimeToDosDateTime(&st_current_time, &w_data, &w_time))
	{
		wsprintf(sz_time, _T("[%d-%d-%d %02d-%02d-%02d %05d___%d].log"),
			(w_data / 32) & 15, w_data & 31, (w_data / 512) + 1980,
			(w_time >> 11), (w_time >> 5) & 0x3F, (w_time & 0x1F) * 2, 
			GetCurrentProcessId(),
			rand());
	}

	str_file_name += sz_time;

	return str_file_name;
}

WorldSession g_worldSession;
