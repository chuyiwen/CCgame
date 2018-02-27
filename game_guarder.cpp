
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//反外挂系统

#include "StdAfx.h"

#include "../common/ServerDefine/login_world.h"
#include "../../common/WorldDefine/game_sentinel_protocol.h"

#include "ApexProxy.h"
#include "game_guarder.h"

#include "world_session.h"
#include "player_session.h"
#include "login_session.h"

#include "world.h"

#ifdef _DEBUG
//#define _DEBUG_GAMEGUARDER
#endif

_FUNC_S_REC		GameGuarder::m_spfRec	= NULL;
BOOL			GameGuarder::m_bEnable	= FALSE;
GameGuarder g_gameGuarder;

#define MGET_ACCSTR( buffer, n_size, nAccountID)	\
	char buffer[n_size] = {0};						\
	sprintf_s(buffer, n_size, "%d", nAccountID);

GameGuarder::GameGuarder()
{
	m_bEnable = TRUE;
	long lRtv = CHSStart(SendMsg, m_spfRec);
	/*ASSERT( !lRtv );*/
	CHSSetFunc(Kick, FLAG_KILLUSER);
}

GameGuarder::~GameGuarder()
{
	CHSEnd();
}

long GameGuarder::Login( DWORD dw_account_id, LPCSTR sz_account, DWORD dw_ip )
{
	TCHAR buf[512]={0};
	MultiByteToWideChar(
		CP_UTF8,
		0,
		sz_account,
		strlen(sz_account),
		buf,
		512);
	tstring str(buf);

	Recv('L', dw_account_id, (char*)str.c_str(), str.length()*sizeof(TCHAR));

	tagUserIP ip;
	ip.uIP = dw_ip;
	
	return Recv('S', dw_account_id, (const char *)(&ip), 5);
}

long GameGuarder::Logout( DWORD dw_account_id, LPCSTR sz_account )
{
	TCHAR buf[512]={0};
	MultiByteToWideChar(
		CP_UTF8,
		0,
		sz_account,
		strlen(sz_account),
		buf,
		512);

	tstring str(buf);

	return Recv('G', dw_account_id, (char*)str.c_str(), str.length()*sizeof(TCHAR));
}

long GameGuarder::Ret( DWORD dw_account_id, const char* pRet )
{

	return Recv('R', dw_account_id, pRet, 4);
}

long GameGuarder::Transport(DWORD dw_account_id, const char* pBuffer, const int nLen)
{
	return Recv('T', dw_account_id, pBuffer, nLen);
}

long GameGuarder::UserData(DWORD dw_account_id, const char* pBuffer, const int nLen)
{
	return Recv('D', dw_account_id, pBuffer, nLen);
}

VOID GameGuarder::SendKickOut(const NET_W2L_kick_log* pKickLog)
{
	g_loginSession.Send((LPVOID)pKickLog, pKickLog->dw_size);
}

long GameGuarder::Recv(char cMsgId,DWORD dw_account_id,const char * pBuffer,int nLen)
{
	if (!m_bEnable)
	{
		MGET_ACCSTR(szBuffer, 64, dw_account_id);
		m_spfRec('G', (signed int)dw_account_id, szBuffer, strlen(szBuffer));
		return 0;
	}
	/*ASSERT( VALID_POINT(m_spfRec) );*/
	const tagUserIP* pIp = (const tagUserIP*)pBuffer;
	if (VALID_POINT(m_spfRec))
		m_spfRec(cMsgId, (signed int)dw_account_id, pBuffer, nLen);

#ifdef _DEBUG_GAMEGUARDER
	print_message(_T("Apex Msg ____Recv.AccountID: %d, Len: %d, Flag: %c!\r\n"), dw_account_id, nLen, cMsgId);
#endif

	return 0;
}

long GameGuarder::SendMsg( signed int nAccountID,const char * pBuffer,int nLen )
{
	if (!m_bEnable)
		return 0;
	
	g_gameGuarder.Wait2Send(nAccountID, nLen, pBuffer);

#ifdef _DEBUG_GAMEGUARDER 
	print_message(_T("Apex Msg Sent____.AccountID: %d, Len: %d!\r\n"), nAccountID, nLen);
#endif	

	return 0;

}

long GameGuarder::Kick(signed int nAccountID, int nAction)
{
	if (!m_bEnable)
	{
		MGET_ACCSTR(szBuffer, 64, nAccountID);
		m_spfRec('G', nAccountID, szBuffer, strlen(szBuffer));
		return 0;
	}

	UINT16	u_16err_code	= static_cast<UINT16>((nAction >> 16) & 0x0000ffff);
	UINT16	u16SealMark = static_cast<UINT16>(nAction & 0x0000ffff);

#ifdef _DEBUG_GAMEGUARDER
	if (0 == u16SealMark)
	{
		// 封号
		print_message(_T("Account: %d Sealed!\r\n"), nAccountID);
	}
	else if (1 == u16SealMark)
	{
		// 踢人
		print_message(_T("Account: %d Kicked!\r\n"), nAccountID);
	}
#endif
	
	g_gameGuarder.Wait2Kick(nAccountID, g_world.GetWorldTime(), u_16err_code, u16SealMark);

	return 0;
}

VOID GameGuarder::Update()
{
	// 发送踢人log
	while(m_listKickAccount.size() > 0)
	{
		NET_W2L_kick_log* pKickLog = m_listKickAccount.pop_front();

		PlayerSession* pToKick = g_worldSession.FindSession(pKickLog->dw_account_id);

		if (VALID_POINT(pToKick))
		{
			//g_worldSession.Kick(pToKick->GetInternalIndex());
			//pToKick->SetKicked();
			SendKickOut(pKickLog);
		}

		SAFE_DELETE(pKickLog);
	}

	// 发送反外挂消息
	m_LockMsg.Acquire();

	while (!m_listMsg.empty())
	{
		tagAccountMsg bind = m_listMsg.front();
		m_listMsg.pop_front();
		PlayerSession* pSession = g_worldSession.FindSession(bind.dw_account_id);

		if (VALID_POINT(pSession))
		{
			pSession->SendMessage(bind.p_message, bind.p_message->dw_size);
		}

		DeleteGuardMsg(bind.p_message);
	}

	m_LockMsg.Release();
}

VOID GameGuarder::Wait2Kick(INT nAccountID, DWORD dwKickTime, UINT16 u_16err_code, UINT16 u16SealMark)
{
	NET_W2L_kick_log* pKickLog = new NET_W2L_kick_log;
	pKickLog->dw_account_id	= nAccountID;
	pKickLog->dw_kick_time	= dwKickTime;
	pKickLog->u_16err_code	= u_16err_code;
	pKickLog->by_seal		= !u16SealMark;			// 是否封号

	m_listKickAccount.push_back(pKickLog);
}

VOID GameGuarder::Wait2Send( INT nAccountID, INT nLen, LPCSTR pBuffer )
{

	NET_SIS_game_sentinel* pSend = CreateGuardMsg(nLen);

	get_fast_code()->memory_copy(pSend->chData, pBuffer, nLen);
	pSend->chCmd = 'T';
	pSend->nLen = nLen;

	tagAccountMsg bind;
	bind.dw_account_id	= nAccountID;
	bind.p_message			= pSend;

	m_LockMsg.Acquire();
	m_listMsg.push_back(bind);
	m_LockMsg.Release();
}

NET_SIS_game_sentinel* GameGuarder::CreateGuardMsg( INT nLen )
{
	DWORD dwMsgSize = sizeof(NET_SIS_game_sentinel) - 1 + nLen;

	BYTE* pTmp = new BYTE[dwMsgSize];
	ZeroMemory(pTmp, dwMsgSize);
	M_msg_init(pTmp, NET_SIS_game_sentinel);
	NET_SIS_game_sentinel* pSend = reinterpret_cast<NET_SIS_game_sentinel*>(pTmp);
	pSend->dw_size = dwMsgSize;

	return pSend;
}

VOID GameGuarder::DeleteGuardMsg( NET_SIS_game_sentinel* p_message )
{
	delete [](BYTE*)p_message;
}