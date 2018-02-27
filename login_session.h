/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once

#include "StdAfx.h"
#include "../../common/WorldDefine/base_define.h"
#include "../../common/WorldDefine/base_define.h"
#include "../../common/WorldDefine/time.h"
#include "mutex.h"

//------------------------------------------------------------------------------
// 即将登陆区域服务器的帐号结构
//------------------------------------------------------------------------------
struct tagWillLoginPlayer
{
	DWORD			dwVerifyCode;	// 验证码
	DWORD			dwTick;			// 通知时的Tick
	BYTE			byPrivilege;	// 帐号权限
	BOOL			bGuard;			// 防沉迷
	DWORD			dwAccOLTime;	// 累计登录时间
	char			sz_account[X_SHORT_NAME];//账号

	tagDWORDTime	dwPreLoginTime;				//上次登录时间
	DWORD			dwPreLoginIP;				//上次登录ip

	tagWillLoginPlayer(DWORD)
	{
		dwVerifyCode = INVALID_VALUE;
		dwTick = INVALID_VALUE;
		byPrivilege = 0;
		bGuard = TRUE;
		dwAccOLTime = 0;

		dwPreLoginTime = 0;
		dwPreLoginIP = 0;
	}

	tagWillLoginPlayer(DWORD verifyCode, DWORD tick, BYTE privilege, BOOL guard, 
						DWORD AccOLTime, LPCSTR Account,tagDWORDTime PreLoginTime,
						DWORD PreLoginIP)
	{
		dwVerifyCode = verifyCode;
		dwTick = tick;
		byPrivilege = privilege;
		bGuard = guard;
		dwAccOLTime = AccOLTime;
		strncpy_s(sz_account, Account, X_SHORT_NAME);

		dwPreLoginTime = PreLoginTime;
		dwPreLoginIP = PreLoginIP;
	}
};

//--------------------------------------------------------------------------------
// Login Session
//--------------------------------------------------------------------------------
class LoginSession
{
public:
	LoginSession();
	~LoginSession();

	BOOL Init();
	VOID Destroy();
	VOID Update();

	BOOL IsWell() { return (m_bInitOK && m_pTran->is_connect()); }

	VOID Send(LPVOID p_message, DWORD dwMsgSize)	{ if(FALSE == IsWell() ) return;		m_pTran->send_msg(p_message, dwMsgSize); }

	INT	GetUnsendPackageNum() { return m_pTran->get_nosend_package_num(); }
	INT	GetReceivedPackageNum() { return m_pTran->get_recv_package_num(); }

public:
	VOID RemoveWillLoginPlayer(DWORD dw_account_id)
	{
		m_WillLoginPlayerMutex.Acquire();
		m_mapWillLoginPlayer.erase(dw_account_id);
		m_WillLoginPlayerMutex.Release();
	}

public:
	BOOL IsAccountValid(DWORD dw_account_id, DWORD dwVerifyCode, BYTE& byPrivilege, BOOL& bGuard, 
						DWORD& dwAccOLSec, LPSTR sz_account,tagDWORDTime &dwPreLoginTime,DWORD &dwPreLoginIP);
	VOID SendPlayerLogin(DWORD dw_account_id, DWORD dw_ip, DWORD dw_error_code);
	VOID SendPlayerLogout(DWORD dw_account_id);
	VOID SendWorldStatus();
private:
	VOID UpdateSession();
	VOID UpdateWillLoginPlayer();

private:
	// 读取文件, 初始化成员
	BOOL InitConfig();
	// 注册所有的网络命令
	VOID RegisterAllLoginCommand();

	VOID UnRegisterAllLoginCommand();

private:
	// 连接数据库服务器
	UINT thread_connect();
	static UINT WINAPI static_thread_connect(LPVOID p_data);

private:
	DWORD HandleCertification(tag_net_message* p_message, DWORD);
	DWORD HandleHeartBeat(tag_net_message* p_message, DWORD);
	DWORD HandlePlayerWillLogin(tag_net_message* p_message, DWORD);
	DWORD HandleKickPlayer(tag_net_message* p_message, DWORD);
	DWORD HandleWorldColsed(tag_net_message* p_message, DWORD);
	DWORD HandleFatigueNotice(tag_net_message* p_message, DWORD);
	
private:
	
	few_connect_client*					m_pTran;
	
	// 连接参数
	CHAR								m_szLoginIP[X_IP_LEN];		// 登陆服务器ip
	DWORD								m_dwLoginPort;				// 登陆服务器port
	
	DWORD								m_dwLocalWanIP;				// 本机广域网IP

	DWORD								m_dwGoldenCode;				// 登陆服务器金色代码
	BOOL								m_bLockIp;					// 是否无网络环境可用

	volatile BOOL						m_bTermConnect;				// 记录当前连接状态
	volatile BOOL						m_bInitOK;					// 是否初始化完成

	package_map<DWORD, tagWillLoginPlayer>		m_mapWillLoginPlayer;		// 将要登陆到区域服务器的玩家列表
	Mutex								m_WillLoginPlayerMutex;		// 即将登陆到区域服务器玩家列表锁

	HANDLE								m_hWorldClosed;
};

extern LoginSession g_loginSession;		// Login Session全局对象