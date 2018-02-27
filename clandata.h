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

#include "../common/ServerDefine/clandata_server_define.h"



class ClanData
{
public:
	static const INT8	MAX_ACT_COUNT;						// Ĭ�ϼ����䱦����
	static const INT32	REP_FULL_VALUE;						// ������ֵ
	static const INT32	ArrNRepLvlMin[ERL_NUM + 1];			// �����׶�����
	static const INT32	ArrNConUpLim[ERL_NUM];				// ���׽׶�����

public:// ���ڴ�ȡ���ݿ�
	ClanData();
	void	Init(const BYTE*& pData, Role *pRole);
	void	Save2DB(IN LPVOID pData, OUT LPVOID &pOutPointer, BOOL& bChg);

public:// ����������
	BOOL	ResetReputation(tagDWORDTime dwtResetTime, ECLanType eClanType, EReputationLevel eRepLvl);
	VOID	SetRepRstTimeStamp(tagDWORDTime dwtRepRstTime){	m_dwtLastResetTime = dwtRepRstTime; m_bChanged = TRUE;}
	tagDWORDTime	GetRepRstTimeStamp(){return m_dwtLastResetTime;}
	VOID	ActCountSetVal(INT8 n8ActCount, ECLanType clantype);
	INT8	ActCountGetVal(ECLanType clantype);
	BOOL	ActCountDec(ECLanType clantype);
	VOID	SetFame(ECLanType eClanType, BOOL bFame = TRUE);
	BOOL	IsFame(ECLanType eClanType) const	{ ASSERT(JDG_VALID(ECLT, eClanType)); return m_u16FameMask & (1<<eClanType);	}

public:// �������ѹ���
	INT32	ClanConGetVal(ECLanType eClanType) const ;
	INT32	ClanConGetMaxVal(ECLanType eClanType) const ;
	INT32	ClanConDec(INT32 nNeed, ECLanType eClanType, BOOL bSend = FALSE);
	INT32	ClanConInc(INT32 nIncr, ECLanType eClanType, BOOL bSend = FALSE);

public:// ���ڸı�����
	VOID	RepSetVal(ECLanType eClanType, INT32 nNewRep, BOOL bSend = TRUE);
	VOID	RepModVal(ECLanType eClanType, INT32 nMod, BOOL bSend = TRUE);
	INT32	RepGetVal(ECLanType eClanType);
	INT16	RepGetLvl(ECLanType eClanType);

private:
	VOID	OnRepChange(ECLanType eClanType);
	VOID	OnClanConChange(ECLanType eClanType);
	VOID	OnActCountChange(ECLanType eClanType, BOOL bSend = TRUE);
	BOOL	EReputationValid(INT32 nReputation);
	EReputationLevel GetRepLvl(INT32 nReputation);

private:
	Role*				m_pRole;							// ��ɫ
	EReputationLevel	m_eRepLevel[ECLT_NUM];				// ��������

	INT8				m_n8ActCount[ECLT_NUM];				// ʣ�༤�����
	INT32				m_nReputation[ECLT_NUM];			// ����ֵ
	INT32				m_nContribution[ECLT_NUM];			// ����ֵ
	tagDWORDTime		m_dwtLastResetTime;					// �ϴ���������ʱ��
	UINT16				m_u16FameMask;						// �����ó�Ա

	BOOL				m_bChanged;							// �Ƿ��иı�
private:
	static VOID Prepare(s_db_repute_data* pdbData);

public:// ���ڲ���
	static BOOL StaticTest();
	BOOL DynamicTest(INT nTestNO , ECLanType eClanType, INT nVal, BOOL bSend = TRUE);

};