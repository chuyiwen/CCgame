
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

#pragma once

#include "StdAfx.h"
#include "Mutex.h"

//-----------------------------------------------------------------------------
// 服务器所能承受的最大在线人数（强制限制）
//-----------------------------------------------------------------------------
const INT MAX_PLAYER_NUM = 10000;

class PlayerSession;

class WorldSession
{
public:
	WorldSession();
	~WorldSession();

	//-------------------------------------------------------------------------
	// 初始化，更新及销毁
	//-------------------------------------------------------------------------
	BOOL			Init();
	VOID			Update();
	VOID			Destroy();

	BOOL			LoadGameConfig();

	//-------------------------------------------------------------------------
	// 各种Get
	//-------------------------------------------------------------------------
	DWORD			GetPort()				{ return m_nPort; }

	//-------------------------------------------------------------------------
	// 客户端连接相关
	//-------------------------------------------------------------------------
	PlayerSession*	FindSession(DWORD dwID);
	PlayerSession*	FindGlobalSession(DWORD dwID);
	BOOL			IsSessionExist(DWORD dwID);

	VOID			AddSession(PlayerSession* pSession);
	VOID			RemoveSession(UINT32 dwID);
	VOID			Kick(DWORD dwInternalIndex);
	VOID			Close(DWORD dwInternalIndex);

	VOID			AddGlobalSession(PlayerSession* pSession);
	VOID			RemoveGlobalSession(DWORD dwSessionID);

	VOID			UpdateSession();
	VOID			DoHouseKeeping();

	INT				GetAllOLAccountID(DWORD* pAccountIDs);

	//---------------------------------------------------------------------------
	// 消息相关
	//---------------------------------------------------------------------------
	__forceinline LPBYTE RecvMsg(DWORD& dw_size, INT& nMsgNum, DWORD dwInternalIndex)
	{
		return m_pNetSession->recv(dwInternalIndex, dw_size, nMsgNum);
	}

	__forceinline VOID Log_Free_Message(DWORD dw_InternalIndex, DWORD dw_Session_id)
	{
		m_pNetSession->log_free_msg(dw_InternalIndex, dw_Session_id);
	}

	__forceinline VOID ReturnMsg(LPBYTE p_message)
	{
		m_pNetSession->free_recv(p_message);
	}

	__forceinline VOID SendMsg(DWORD dwInternalIndex, LPBYTE p_message, DWORD dw_size)
	{
		Interlocked_Exchange_Add((LPLONG)&m_nMsgSendThisTick, 1);
		m_pNetSession->send(dwInternalIndex, p_message, dw_size);
	}

	__forceinline INT GetSendCast(DWORD dwInternalIndex)
	{
		return m_pNetSession->get_send_cast(dwInternalIndex);
	}

	__forceinline INT GetSendCastSize(DWORD dwInternalIndex)
	{
		return m_pNetSession->get_send_cast_size(dwInternalIndex);
	}

	__forceinline INT GetSendNum(DWORD dwInternalIndex)
	{
		return m_pNetSession->get_send_queue_num(dwInternalIndex);
	};

	//---------------------------------------------------------------------------
	// 统计信息
	//---------------------------------------------------------------------------
	INT	GetPlayerNumLimit()				{ return m_nPlayerNumLimit; }
	INT GetPlayerNumCurrent()			{ return m_nPlayerNumCurrent; }
	INT GetPlayerNumPeek()				{ return m_nPlayerNumPeek; }
	INT GetPlayerLoginTimes()			{ return m_nPlayerIn; }
	INT GetPlayerLogoutTimes()			{ return m_nPlayerOut; }
	INT GetMsgSendCast()				{ return m_nSendCast; }
	INT GetMsgSendThisTick()			{ return m_nMsgSendThisTick; }
	INT GetMsgProceedThisTick()			{ return m_nMsgProceedThisTick; }
	INT GetMsgRecvWait()				{ return m_nMsgRecvWait; }

	INT64 GetTotalSendSize()			{ return m_pNetSession->get_send_size(); }
	INT	  GetFreeClientNum()			{ return m_pNetSession->get_free_client_num(); }
	INT	  GetAcceptNum()				{ return m_pNetSession->get_accept_num(); } 
	INT64 GetTotalRealSendSize()		{ return m_pNetSession->get_compress_send_size(); }
	INT32 GetAddNum()					{ return m_pNetSession->get_add_num(); }
	INT32 GetDropNum()					{ return m_pNetSession->get_drop_num(); }
	INT32 GetRecvAddNum()				{ return m_pNetSession->get_recv_num(); }
	INT32 get_recv_drop_num()				{ return m_pNetSession->get_recv_drop_num(); }
	INT64 get_sec_send_size()				{ return m_pNetSession->get_sec_send_size(); }
	INT64 get_io_pending()				{ return m_pNetSession->get_io_pending(); }
	DWORD get_io_pending_id()			{ return m_pNetSession->get_io_pending_id(); }
	INT GetSendComRatio();

	VOID SetMsgSendCast(INT n_num)			{ Interlocked_Exchange((LPLONG)&m_nSendCast, n_num); }
	VOID SetUnitSendThisTick(INT n_num)		{ Interlocked_Exchange((LPLONG)&m_nMsgSendThisTick, n_num); }
	VOID SetUnitProceedThisTick(INT n_num)	{ Interlocked_Exchange((LPLONG)&m_nMsgProceedThisTick, n_num); }
	VOID SetUnitRecvWait(INT n_num)			{ Interlocked_Exchange((LPLONG)&m_nMsgRecvWait, n_num); }

	VOID SetPlayerNumLimit(INT nPlayerLimit)
	{
		if( nPlayerLimit < 0 ) return;
		if( nPlayerLimit > MAX_PLAYER_NUM ) nPlayerLimit = MAX_PLAYER_NUM;

		Interlocked_Exchange((LPLONG)&m_nPlayerNumLimit, nPlayerLimit);
	}
	VOID PlayerLogin()
	{
		Interlocked_Exchange_Add((LPLONG)&m_nPlayerNumCurrent, 1);
		Interlocked_Exchange_Add((LPLONG)&m_nPlayerIn, 1);

		// 超过了峰值
		if( m_nPlayerNumCurrent > m_nPlayerNumPeek )
		{
			Interlocked_Exchange((LPLONG)&m_nPlayerNumPeek, m_nPlayerNumCurrent);
		}
	}
	VOID PlayerLogout()
	{
		Interlocked_Exchange_Add((LPLONG)&m_nPlayerNumCurrent, -1);
		Interlocked_Exchange_Add((LPLONG)&m_nPlayerOut, 1);
	}

	tstring	create_network_log();

private:
	

	package_map<DWORD, PlayerSession*>			m_mapAllSessions;		// 所有连接到服务器的客户端连接（帐号为主键）
	Mutex								m_AllSessionMutex;		// m_mapAllSessions的锁

	package_map<DWORD, PlayerSession*>			m_mapGlobalSessions;	// 所有还没有进入到游戏中的客户端连接（帐号为主键）
	Mutex								m_GlobalSessionMutex;	// m_mapGlobalSessions的锁

	package_list<PlayerSession*>				m_listInsertPool;		// 要加入到Global Session中的Session
	Mutex								m_InsertPoolMutex;		// m_listInsertPool的锁

	IOCP*							m_pNetSession;			// 完成端口
	INT									m_nPort;				// 监听端口

	volatile INT						m_nPlayerIn;			// 总玩家登入数量
	volatile INT						m_nPlayerOut;			// 总玩家登出数量

	volatile INT						m_nPlayerNumLimit;		// 服务器人数限制
	volatile INT						m_nPlayerNumCurrent;	// 当前在线人数

	volatile INT						m_nPlayerNumPeek;		// 峰值人数

	volatile INT						m_nSendCast;			// 网络底层还没有收到返回确认的发送消息数量
	volatile INT						m_nMsgSendThisTick;		// 网络底层本tick发送的消息数量
	volatile INT						m_nMsgProceedThisTick;	// 本Tick处理了多少消息
	volatile INT						m_nMsgRecvWait;			// 还有多少接收消息在等待处理

private:
	UINT LoginCallBack(tag_unit*, tag_login_param*);						// 客户端登陆本服务器回调函数
	UINT LogoutCallBack(DWORD);								// 客户端登出本服务器回调函数

private:
	volatile INT						m_nTreasureSum;			// 服务器总开启宝箱数
public:
	INT GetTreasureSum()				{ return m_nTreasureSum; }
	VOID IncTreasureSum();
	VOID SetTreasureSum(INT nSum);

};

//--------------------------------------------------------------------------------------------
// 得到压缩率
//--------------------------------------------------------------------------------------------
inline INT WorldSession::GetSendComRatio()
{
	INT64 nSendSize = m_pNetSession->get_send_size();
	INT64 nRealSendSize = m_pNetSession->get_compress_send_size();

	if( 0 >= nSendSize ) return 0;

	FLOAT fRatio = FLOAT(nSendSize - nRealSendSize) / FLOAT(nSendSize);

	return INT(fRatio * 100.0f);
}

extern WorldSession g_worldSession;
