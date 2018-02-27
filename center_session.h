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
*	@file		center_session.h
*	@author		lc
*	@date		2011/03/16	initial
*	@version	0.0.1.0
*	@brief		ÅÄÂô¿Í»§¶Ë
*/


#ifndef CENTER_SESSION
#define CENTER_SESSION

class center_session
{
public:
	center_session(void);
	~center_session(void);

	BOOL	Init();
	VOID	Destroy();
	VOID	Update();
	
	BOOL	IsWell() { return (m_bInitOK && m_pTran->is_connect()); }

	VOID	Send(LPVOID p_message, DWORD dwMsgSize)	
	{ 
		if(FALSE == IsWell() ) 
			return;		
		m_pTran->send_msg(p_message, dwMsgSize); 
	}

	BOOL	IsConnected()	{ return m_pTran->is_connect(); }

private:
	VOID	RegisterAllCommand();
	VOID	UnRegisterAllCommand();
	VOID	UpdateSession();

	UINT	thread_connect();
	static UINT WINAPI static_thread_connect(LPVOID p_data);

	DWORD HandleServerLogin(tag_net_message* p_message, DWORD);
	DWORD HandleVerifyCode(tag_net_message* p_message, DWORD);

private:
	few_connect_client*			m_pTran;



	volatile BOOL				m_bTermConnect;
	volatile BOOL				m_bInitOK;

	tstring						m_strIP;
	DWORD						m_dwPort;
	DWORD						m_dwSectionID;
	DWORD						m_dwWorldID;

	memorysystem::MemPool*					p_memory_pool;
};

extern center_session g_center_session;

#endif
