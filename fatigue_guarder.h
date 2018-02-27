
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

#pragma once

#include "../../common/WorldDefine/time.h"
#include "../../common/WorldDefine/base_define.h"
#include "../../common/WorldDefine/enthrallment_protocol.h"
#include "../common/ServerDefine/anti_addiction_server_define.h"

//-----------------------------------------------------------------------------
// 沉迷防卫器
//-----------------------------------------------------------------------------
class FatigueGuarder
{
	enum
	{
		NUM_POINTS_NOTIFY = 8,		// 通知客户端时间点的个数
	};

	//-----------------------------------------------------------------------------
	// 状态结构
	//-----------------------------------------------------------------------------
	struct tagStateRangeRate
	{
		DWORD	dwBegin;
		DWORD	dwEnd;
		DWORD	dwEarnRate;
	};

public:
	FatigueGuarder(PlayerSession* pPlayerSession, BOOL bGuard = TRUE, DWORD dwAccOLAcc = 0)
		:m_bGuard(bGuard), m_dwAccOLMin(dwAccOLAcc), m_dwState(0), m_pPlayerSession(pPlayerSession)
	{
		ASSERT( VALID_POINT(m_pPlayerSession) );
	}

	BOOL IsGuard()	const {return m_bGuard;}
	//-----------------------------------------------------------------------------
	// 更新
	//-----------------------------------------------------------------------------
	VOID SetAccOLTimeMin(DWORD dwState, DWORD dwAccOLTimeMin);

	//-----------------------------------------------------------------------------
	// 收益率
	//-----------------------------------------------------------------------------
	DWORD GetEarnRate()	const 
	{	
		if ( !m_bGuard ) return 10000;
		ASSERT( EFS_VALID(m_dwState) );
		return m_arrStateRangeRate[m_dwState].dwEarnRate; 
	}

	//-----------------------------------------------------------------------------
	// 通知客户端
	//-----------------------------------------------------------------------------
	void NotifyClient() const ;
private:
	//-----------------------------------------------------------------------------
	// 获得状态
	//-----------------------------------------------------------------------------
	static e_anti_addiction_state GetState(const DWORD& dwAccOLMin)
	{
		for (INT i = eaas_begin; i < eaas_end; ++i)
		{
			if ( dwAccOLMin >= m_arrStateRangeRate[i].dwBegin&&
				 dwAccOLMin < m_arrStateRangeRate[i].dwEnd )
			{
				return static_cast<e_anti_addiction_state>(i);
			}
		}
		ASSERT( 0 );
		return static_cast<e_anti_addiction_state>(INVALID_VALUE);		
	}
	
	
private:

	BOOL						m_bGuard;									// 是否是防沉迷用户
	DWORD						m_dwState;									// 游戏状态
	DWORD						m_dwAccOLMin;								//  当前累计在线时间

	PlayerSession*				m_pPlayerSession;							// 当前 PlayerSession

	static tagStateRangeRate	m_arrStateRangeRate[eaas_num];				// 收益率列表
};
