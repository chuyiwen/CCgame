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
// ������½������������ʺŽṹ
//------------------------------------------------------------------------------
struct tagWillLoginPlayer
{
	DWORD			dwVerifyCode;	// ��֤��
	DWORD			dwTick;			// ֪ͨʱ��Tick
	BYTE			byPrivilege;	// �ʺ�Ȩ��
	BOOL			bGuard;			// ������
	DWORD			dwAccOLTime;	// �ۼƵ�¼ʱ��
	char			sz_account[X_SHORT_NAME];//�˺�

	tagDWORDTime	dwPreLoginTime;				//�ϴε�¼ʱ��
	DWORD			dwPreLoginIP;				//�ϴε�¼ip

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
	// ��ȡ�ļ�, ��ʼ����Ա
	BOOL InitConfig();
	// ע�����е���������
	VOID RegisterAllLoginCommand();

	VOID UnRegisterAllLoginCommand();

private:
	// �������ݿ������
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
	
	// ���Ӳ���
	CHAR								m_szLoginIP[X_IP_LEN];		// ��½������ip
	DWORD								m_dwLoginPort;				// ��½������port
	
	DWORD								m_dwLocalWanIP;				// ����������IP

	DWORD								m_dwGoldenCode;				// ��½��������ɫ����
	BOOL								m_bLockIp;					// �Ƿ������绷������

	volatile BOOL						m_bTermConnect;				// ��¼��ǰ����״̬
	volatile BOOL						m_bInitOK;					// �Ƿ��ʼ�����

	package_map<DWORD, tagWillLoginPlayer>		m_mapWillLoginPlayer;		// ��Ҫ��½�����������������б�
	Mutex								m_WillLoginPlayerMutex;		// ������½���������������б���

	HANDLE								m_hWorldClosed;
};

extern LoginSession g_loginSession;		// Login Sessionȫ�ֶ���