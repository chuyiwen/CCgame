/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
 *	@file		GMSession
 *	@author		mwh
 *	@date		2011/04/07	initial
 *	@version	0.0.1.0
 *	@brief		连接GM服务器
*/

#ifndef __GMSESSION_H__
#define __GMSESSION_H__

enum{ STATUSREPORTINTERVAL = TICK_PER_SECOND };

//=================================================
//	连接GM服务器
//=================================================
class GMSession
{
	tstring mIP;
	DWORD mPort;
	DWORD mWordID;
	DWORD mSectionID;
	INT mSendStatusTick;
	volatile BOOL mTerminateConnect;
	volatile BOOL mInit;
	few_connect_client* mNetSession;
public:
	GMSession();
	~GMSession();
public:
	BOOL Init();
	VOID Destroy();
	VOID Update();
public:
	inline VOID SendMessage(LPVOID lp, DWORD size);
	inline BOOL IsConnected() const;
private:
	VOID SendServerInfo();
	VOID RegisterCmd();
	VOID UnregisterCmd();
	VOID UpdateSession();
private:
	UINT ThreadConnect();
	static UINT WINAPI ThreadCall(LPVOID lpVoid);
private:
	BOOL ReConnect();
	BOOL CreateConnectThread();
	VOID HandleMessage();
//=================================================
//	以下是逻辑处理
//=================================================
private:
	DWORD HandleServerLogin(tag_net_message* lpMsg, DWORD);
	DWORD HandleForbidTalk(tag_net_message* lpMsg, DWORD);
	DWORD HandleAfficheMsg(tag_net_message* lpMsg, DWORD);
	DWORD HandleGMKick(tag_net_message* lpMsg,	DWORD);
	DWORD HandleShutDown(tag_net_message* lpMsg,	DWORD);
	DWORD HandleAddItem(tag_net_message* lpMsg, DWORD);
	DWORD HandleSetDoublePolicy(tag_net_message* lpMsg, DWORD);
	DWORD HandleSetMaxPlayerNumber(tag_net_message* lpMsg, DWORD);
	DWORD HandleGMToolCreateEquip(tag_net_message* lpMsg, DWORD);
	DWORD HandleGMToolCreateItem(tag_net_message* lpMsg, DWORD);
	DWORD HandleGMToolGiveMoney(tag_net_message* lpMsg, DWORD);
	DWORD HandleGMValidate(tag_net_message* lpMsg, DWORD);
//=================================================
//	以上是逻辑处理
//=================================================
};

inline BOOL GMSession::IsConnected() const { return mNetSession->is_connected(); }
inline VOID GMSession::SendMessage(LPVOID lp, DWORD size)
{
	if(!VALID_POINT(mNetSession))
		return;

	if(!mNetSession->is_connect() || !mInit)
		return;

	mNetSession->send_msg(lp, size);
}

extern GMSession g_rtSession;

#endif // __GMSESSION_H__
