/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#pragma once

#include "famehall_part.h"
//----------------------------------------------------------------------------
//���ݿⱣ��취
//	���������ã�	���ݿ������Ϣ
//	���������䱦��	���ݿ������Ϣ
//	��������ʱ�䣺	���ݿ������Ϣ
//	��ɫ������		STWorld���ڱ���������Ϣ
//���ݿ��ȡ������	���ݿ���Ϣ

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// ������
//----------------------------------------------------------------------------
class FameHall
{
public:// ���̵߳���
	FameHall()	{}
	~FameHall()	{	Destroy();	}
	BOOL Init();

	VOID Destroy(){}

	// ���ڸ��¸�����������
	VOID Update();

	// �������ݿ���Ϣ
	VOID SendLoadDBData();

	// ��������������Ϣ
	VOID HandleUpdateRepRank(tag_net_message* pCmd);

	// �����ʼ�������䱦�б���Ϣ
	VOID HandleInitActTreasureList(tag_net_message* pCmd);

	// ��ʼ�������ý������
	VOID HandleInitFameHallTop50(tag_net_message* pCmd);

	// ��ʼ����������ʱ��
	VOID HandleInitRepRstTimeStamp(tag_net_message* pCmd);

	// ���������䱦�����������̣߳�
	DWORD ActClanTreasure(Role* pRole, UINT16 u16TreasureID);

	// ��ɫ��ȡ���������ֵ
	VOID RoleRepUpdate(Role* pRole, ECLanType eClanType);

public:// ��ͼ�̵߳���
	// ���Խ���������
	BOOL TryEnterFameHall(Role* pRole, ECLanType eClanType);

	// ��ȡ���������ǰ50��nameid��
	VOID GetMemberTop50(BYTE* pData, ECLanType eClanType);

	// ��ȡ���������ǰ50������Ŀ
	INT32 GetMemberTop50Num(ECLanType byClanType);

	// �������������tagRepRankData��
	VOID GetRepRankTop(PVOID pData, ECLanType eClanType);

	// �������������Ŀ
	INT32 GetRepRankNum(ECLanType byClanType);

	// ����Ѽ��������䱦�б�
	VOID GetActClanTreasure(PVOID pData, ECLanType eClanType);

	// ����Ѽ��������䱦��Ŀ
	INT32 GetActClanTreasureNum(ECLanType byClanType);

	// ���������������ʱ��
	tagDWORDTime GetRepRankTimeStamp(ECLanType byClanType) const	{	return m_ArrClanTrunk[byClanType].GetRepRankUpdateTime();}

	// ����������б����ʱ��
	tagDWORDTime GetFameSnapTimeStamp(ECLanType byClanType) const	{	return m_ArrClanTrunk[byClanType].GetEnterSnapUpdateTime();}

private:
	ClanTrunk			m_ArrClanTrunk[ECLT_NUM];			// ������������
	BOOL				m_bInitOK;							// ��ʼ���ɹ�

private:// ���ÿ���
	FameHall(const FameHall& rhs);
	FameHall& operator = (const FameHall& rhs);
};


extern FameHall g_fameHall;
