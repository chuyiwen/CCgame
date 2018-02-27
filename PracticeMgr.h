/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
#ifndef _PRACTICEMGR_H_
#define _PRACTICEMGR_H_

/**
*	@file		role_ride
*	@author		lc
*	@date		2010/12/2	initial
*	@version	0.0.1.0
*	@brief		武器修炼
*/

class Role;
struct tagItem;
class PracticeMgr
{
public:
	PracticeMgr(Role* pRole);
	~PracticeMgr();
	
	//开始修炼
	VOID	BeginPractice();
	//结束修炼
	VOID	EndPractice(DWORD dw_error_code);
	//设置修炼相关数据
	//param1 : 武器id
	//param2 : 加速道具id
	//param3 : 加值道具id
	VOID	SetPracticeData(INT64 n64EquipId, package_list<tagItem*>& itemList);

	//修炼更新逻辑
	VOID	Update();
	
	//是否可以修炼
	BOOL	IsInCanPracticeState();

	// 设置离线修炼时间
	VOID	SetLeavePracticeTime(BYTE byTimeType);

	// 设置离线修炼倍数及消耗
	VOID	SetLeavePraciticeMul(BYTE byMulType);

	// 通过离线修炼倍数类型获取实际倍数
	INT		GetLeavePraciticeLoveFromType(BYTE byMulType);

	INT		GetLeavePraciticeTotalTime() { return m_nLeaveCountDown; } 
	DWORD	GetLeavePraciticeTotalLove() { return dw_LoveTotal; }

	VOID	SendLeavePraciticeLog();

	VOID	SetAddLoveTotal(DWORD	dw_love);

private:
	Role*	m_pOwner;
	INT64	m_n64EquipID;
	
	// 使用的加速道具列表
	//package_list<tagItem*> m_itemList;
	package_list<INT64>	m_item_serial;

	//DWORD	m_dw_item_id;			// 加速道具id
	//DWORD	m_dw_item_num;			// 加速道具数量

	BOOL	m_bPracticing;

	INT		m_nTickCountDown;	//修炼倒计时	

	INT		m_nLeavePracticeTime;			// 离线修炼时间
	INT		m_nLeavePracitceMul;			// 离线修炼倍数
	INT		m_nLeavePraciteceLove;			// 离线修炼消耗爱心值
	INT		m_nLeaveCountDown;
	INT		m_nLeaveLoveCountDown;
	DWORD	dw_LoveTotal;					// 扣除的离线修炼爱心值总数
};

#endif