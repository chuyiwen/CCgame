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
*	@brief		��������
*/

class Role;
struct tagItem;
class PracticeMgr
{
public:
	PracticeMgr(Role* pRole);
	~PracticeMgr();
	
	//��ʼ����
	VOID	BeginPractice();
	//��������
	VOID	EndPractice(DWORD dw_error_code);
	//���������������
	//param1 : ����id
	//param2 : ���ٵ���id
	//param3 : ��ֵ����id
	VOID	SetPracticeData(INT64 n64EquipId, package_list<tagItem*>& itemList);

	//���������߼�
	VOID	Update();
	
	//�Ƿ��������
	BOOL	IsInCanPracticeState();

	// ������������ʱ��
	VOID	SetLeavePracticeTime(BYTE byTimeType);

	// ����������������������
	VOID	SetLeavePraciticeMul(BYTE byMulType);

	// ͨ�����������������ͻ�ȡʵ�ʱ���
	INT		GetLeavePraciticeLoveFromType(BYTE byMulType);

	INT		GetLeavePraciticeTotalTime() { return m_nLeaveCountDown; } 
	DWORD	GetLeavePraciticeTotalLove() { return dw_LoveTotal; }

	VOID	SendLeavePraciticeLog();

	VOID	SetAddLoveTotal(DWORD	dw_love);

private:
	Role*	m_pOwner;
	INT64	m_n64EquipID;
	
	// ʹ�õļ��ٵ����б�
	//package_list<tagItem*> m_itemList;
	package_list<INT64>	m_item_serial;

	//DWORD	m_dw_item_id;			// ���ٵ���id
	//DWORD	m_dw_item_num;			// ���ٵ�������

	BOOL	m_bPracticing;

	INT		m_nTickCountDown;	//��������ʱ	

	INT		m_nLeavePracticeTime;			// ��������ʱ��
	INT		m_nLeavePracitceMul;			// ������������
	INT		m_nLeavePraciteceLove;			// �����������İ���ֵ
	INT		m_nLeaveCountDown;
	INT		m_nLeaveLoveCountDown;
	DWORD	dw_LoveTotal;					// �۳���������������ֵ����
};

#endif