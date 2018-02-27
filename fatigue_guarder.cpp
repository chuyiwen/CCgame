
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//防沉迷系统

#include "StdAfx.h"
#include "fatigue_guarder.h"
#include "player_session.h"
//-----------------------------------------------------------------------------
// 收益率列表
//-----------------------------------------------------------------------------
FatigueGuarder::tagStateRangeRate 
FatigueGuarder::m_arrStateRangeRate[eaas_num]	= {	{0,		180,	10000},			// 0h-3h
													{180,	300,	5000},			// 3h-5h
													{300, INVALID_VALUE, 0},			// 5h-UNLIMIT
													};

//-----------------------------------------------------------------------------
// 更新
//-----------------------------------------------------------------------------
VOID FatigueGuarder::SetAccOLTimeMin( DWORD dwState, DWORD dwAccOLTimeMin )
{
	if (!m_bGuard)	return;

	m_dwAccOLMin = dwAccOLTimeMin;

	m_dwState = dwState;
	ASSERT( EFS_VALID(m_dwState) );
	NotifyClient();
}

//-----------------------------------------------------------------------------
// 通知客户端
//-----------------------------------------------------------------------------
void FatigueGuarder::NotifyClient() const
{
	// 通知客户端
	NET_SIS_enthrallment_info send;
	send.byState = (BYTE)m_dwState;
	send.dwCurOLSec = m_dwAccOLMin * 60;
	m_pPlayerSession->SendMessage(&send, send.dw_size);
}