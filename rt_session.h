
/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/


#pragma once

static const INT	SEND_INFO_INTERVAL	=	TICK_PER_SECOND;		// 每隔多长时间发送一次服务器信息

//-----------------------------------------------------------------------------
// 服务器与监控服务的对话层
//-----------------------------------------------------------------------------
class montioring_session
{
public:
	//-------------------------------------------------------------------------
	// Constructor
	//-------------------------------------------------------------------------
	montioring_session();
	~montioring_session();

	BOOL	Init();
	VOID	Destroy();
	VOID	Update();

	BOOL	IsConnected()	{ return m_pTran->is_connect(); }

private:
	VOID	SendServerInfo();
	VOID	RegisterAllRTCommand();
	VOID	UnRegisterAllRTCommand();
	VOID	UpdateSession();

	DWORD	ThreadConnectRT();

private:
	DWORD	HandleServerLogin(tagNetCmd* p_message, DWORD);
	DWORD	HandleCloseServer(tagNetCmd* p_message, DWORD);
	
	DWORD	HandleDouble(tagNetCmd* p_message, DWORD);
	DWORD	HandleAutoNotice(tagNetCmd* p_message, DWORD);
	DWORD	HandleRightNotice(tagNetCmd* p_message, DWORD);
	DWORD	HandleSetMaxNum(tagNetCmd* p_message, DWORD);
	DWORD   HandleUpdateMall(tagNetCmd* p_message, DWORD);
	DWORD   HandleAutoChatNotice(tagNetCmd* p_message, DWORD);
	DWORD	HandleCancelDouble(tagNetCmd* p_message, DWORD);

private:
	TSFPTrunk<montioring_session>		m_Trunk;
	few_connect_client*			m_pTran;
	TObjRef<Thread>				m_pThread;
	

	volatile BOOL				m_bTermConnect;
	volatile BOOL				m_bInitOK;
	INT							m_nSendInfoTickCountDown;

	tstring						m_strIP;
	DWORD						m_dwPort;
	DWORD						m_dwSectionID;
	DWORD						m_dwWorldID;
};


extern montioring_session g_rtSession;