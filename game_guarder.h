
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//反外挂

#pragma once
#include "mutex.h"

//-----------------------------------------------------------------------------
// ip转换
//-----------------------------------------------------------------------------
#pragma pack(push,1)
struct tagUserIP 
{
	char			chUserIPFlag;
	unsigned int	uIP;

#define ClientIpFlag  0x01
	tagUserIP(unsigned int ip = 0):chUserIPFlag(ClientIpFlag),uIP(ip){}
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
// 反外挂客户端消息
//-----------------------------------------------------------------------------
struct NET_SIS_game_sentinel;

//-----------------------------------------------------------------------------
// 账号和消息的封装
//-----------------------------------------------------------------------------
struct tagAccountMsg
{
	DWORD				dw_account_id;
	NET_SIS_game_sentinel*	p_message;
};

//-----------------------------------------------------------------------------
// 发送给Login的踢人消息
//-----------------------------------------------------------------------------
struct NET_W2L_kick_log;

//-----------------------------------------------------------------------------
// 反外挂包装
//-----------------------------------------------------------------------------
class GameGuarder
{
public:
	GameGuarder();
	~GameGuarder();

	//-----------------------------------------------------------------------------
	// 开关
	//-----------------------------------------------------------------------------
	VOID		Switch(BOOL bOn){	if (bOn != m_bEnable)Interlocked_Exchange((LPLONG)&m_bEnable, bOn);	}

	//-----------------------------------------------------------------------------
	// 主线程UPDATE
	//-----------------------------------------------------------------------------
	VOID		Update();

public:
	//-----------------------------------------------------------------------------
	// 反外挂流程
	//-----------------------------------------------------------------------------
	long		Login(DWORD dw_account_id, LPCSTR sz_account, DWORD dw_ip);
	long		Logout(DWORD dw_account_id, LPCSTR sz_account);
	long		Ret(DWORD dw_account_id, const char* pBuffer);
	long		Transport(DWORD dw_account_id, const char* pBuffer, const int nLen);
	long		UserData(DWORD dw_account_id, const char* pBuffer, const int nLen);

private:
	//-----------------------------------------------------------------------------
	// 内部方法
	//-----------------------------------------------------------------------------
	long		Recv(char cMsgId,DWORD dw_account_id,const char * pBuffer,int nLen);
	
	//-----------------------------------------------------------------------------
	// 保留到主线程执行
	//-----------------------------------------------------------------------------
	VOID		Wait2Send( INT nAccountID, INT nLen, LPCSTR pBuffer );
	VOID		Wait2Kick(INT nAccountID, DWORD dwKickTime, UINT16 u_16err_code, UINT16 u16SealMark);
	
	//-----------------------------------------------------------------------------
	// 主线程执行
	//-----------------------------------------------------------------------------
	static long	SendMsg(signed int nAccountID,const char * pBuffer,int nLen);
	static VOID SendKickOut(const NET_W2L_kick_log* pKickLog);

	//-----------------------------------------------------------------------------
	// 踢人回调
	//-----------------------------------------------------------------------------
	static long	Kick(signed int nAccountID,int nAction);
	
	//-----------------------------------------------------------------------------
	// 创建删除消息
	//-----------------------------------------------------------------------------
	static NET_SIS_game_sentinel*	CreateGuardMsg( INT nLen );
	static VOID	DeleteGuardMsg( NET_SIS_game_sentinel* p_message );

private:
	static long (*m_spfRec) (char cMsgId,signed int nId,const char * pBuffer,int nLen);
	static BOOL		m_bEnable;

private:
	package_safe_list<NET_W2L_kick_log*>	m_listKickAccount;

	std::list<tagAccountMsg>	m_listMsg;
	Mutex						m_LockMsg;
};

extern GameGuarder g_gameGuarder;
