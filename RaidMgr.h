/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
// ����ϵͳ

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
	// ��ȡ����ģ��
	DWORD	getRaidMode();
	INT		getLevel();
	// ������������
	VOID	AcitveAtt(BOOL bActive);
	// ���
	DWORD	BeginRaid();
	// ȡ�����
	DWORD	CancelRaid();
	// ����
	DWORD	Tog(BYTE byType, INT& nCritNum, INT& nGetExp);
private:
	
	Role*		m_pOwner;
	INT			m_nExpCur;			//��ǰ����	
	INT			m_nStep;			//��
	INT			m_nGrade;			//��
};