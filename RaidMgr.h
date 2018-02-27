/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
// 坐骑系统

#pragma once

class Role;

class RaidMgr
{
public:
	RaidMgr();
	~RaidMgr();

	BOOL	init(Role* pRole, INT nStep, INT nGrade, INT nexp);

	VOID	SaveToDB();
	VOID	ExpChange(INT nExpMod, BOOL bSend, BOOL bOverFlow = TRUE);	
	INT		GetExpLvlUp();

	INT		GetStep() { return m_nStep; }

	VOID	sendRaidData();
	// 获取坐骑模型
	DWORD	getRaidMode();
	INT		getLevel();
	// 激活坐骑属性
	VOID	AcitveAtt(BOOL bActive);
	// 骑乘
	DWORD	BeginRaid();
	// 取消骑乘
	DWORD	CancelRaid();
	// 培养
	DWORD	Tog(BYTE byType, INT& nCritNum, INT& nGetExp);
private:
	
	Role*		m_pOwner;
	INT			m_nExpCur;			//当前经验	
	INT			m_nStep;			//阶
	INT			m_nGrade;			//等
};