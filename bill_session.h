
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

#pragma once

//-----------------------------------------------------------------------------
// 服务器与划拨服务器的对话层
//-----------------------------------------------------------------------------
class BillSession
{
public:
	//-------------------------------------------------------------------------
	// Constructor
	//-------------------------------------------------------------------------
	BillSession();
	~BillSession();

	BOOL	Init();
	VOID	Destroy();
	VOID	Update();

	BOOL	IsConnected()	{ return m_pTran->is_connect(); }

private:
	VOID	RegisterAllBillCommand();
	VOID	UpdateSession();

	UINT	thread_connect();
	static UINT WINAPI static_thread_connect(LPVOID p_data);

private:
	DWORD	HandleServerLogin(tag_net_message* p_message, DWORD);
	DWORD	HandlePickBaiBao(tag_net_message* p_message, DWORD);
	DWORD	HandleChangeReceive(tag_net_message* p_message, DWORD);
	DWORD	HandleReloadAutoPaimai(tag_net_message* p_message, DWORD);

private:
	few_connect_client*			m_pTran;
	
	

	volatile BOOL				m_bTermConnect;
	volatile BOOL				m_bInitOK;

	tstring						m_strIP;
	DWORD						m_dwPort;
	DWORD						m_dwSectionID;
	DWORD						m_dwWorldID;
};


extern BillSession g_billSession;