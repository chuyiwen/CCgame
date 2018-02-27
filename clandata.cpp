/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#include "StdAfx.h"
#include "clandata.h"
#include "role.h"
#include "famehall.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/currency_protocol.h"

const INT32 ClanData::ArrNRepLvlMin[ERL_NUM + 1]	= { -46800, -10800, -3600, 0, 3600, 10800, 22800, 40800, 76800, 148800, 328800};
const INT32 ClanData::ArrNConUpLim[ERL_NUM]			= { 0, 0, 5000, 5000, 10000, 20000, 40000, 70000, 120000, 200000 };
const INT8	ClanData::MAX_ACT_COUNT					= 1;				// Ĭ�ϼ����䱦����
const INT32	ClanData::REP_FULL_VALUE				= 328800;			// ������ֵ

ClanData::ClanData()
{
	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		m_nContribution[clantype]	= 0;
		m_n8ActCount[clantype]		= 0;
		m_nReputation[clantype]		= 0;
		OnRepChange(ECLanType(clantype));
		OnClanConChange(ECLanType(clantype));
	}
	m_dwtLastResetTime = g_world.GetWorldTime();
}

void ClanData::Init(const BYTE*& pData, Role *pRole)
{
	m_bChanged = FALSE;
	ASSERT( VALID_POINT(pRole) );
	m_pRole = pRole;
	M_trans_pointer(pRepData, pData, s_db_repute_data);
	ASSERT( VALID_POINT(pRepData) );
	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		m_nContribution[clantype]	= pRepData->n_contribution_[clantype];
		m_n8ActCount[clantype]		= pRepData->n8_act_count_[clantype];
		m_nReputation[clantype]		= pRepData->n_reputation_[clantype];
		ASSERT( m_nReputation[clantype] <= ClanData::REP_FULL_VALUE);
		OnActCountChange(ECLanType(clantype), FALSE);
		OnRepChange(ECLanType(clantype));
		OnClanConChange(ECLanType(clantype));
	}
	m_dwtLastResetTime = pRepData->last_reset_time_;
	m_u16FameMask = pRepData->u16_fame_mask;

	pData = pData + sizeof(s_db_repute_data);

	// ����������ø���
	g_fameHall.RoleRepUpdate(m_pRole, ECLT_NULL);
}
void ClanData::Save2DB( IN LPVOID pData, OUT LPVOID &pOutPointer, BOOL& bChg )
{
	bChg = TRUE;
	if(FALSE == m_bChanged)
	{	
		bChg = FALSE;
		return;
	}
		
	m_bChanged = FALSE;

	M_trans_pointer(pRepData, pData, s_db_repute_data);
	
	ASSERT( VALID_POINT(pRepData) );
	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		pRepData->n_contribution_[clantype]		= m_nContribution[clantype];
		pRepData->n8_act_count_[clantype]		= m_n8ActCount[clantype];
		pRepData->n_reputation_[clantype]		= m_nReputation[clantype];
	}
	pRepData->u16_fame_mask = m_u16FameMask;
	pRepData->last_reset_time_ = m_dwtLastResetTime;
	pOutPointer = reinterpret_cast<BYTE*>(pData) + sizeof(s_db_repute_data);
}

BOOL ClanData::ResetReputation( tagDWORDTime dwtResetTime, ECLanType eClanType, EReputationLevel eRepLvl )
{
	ASSERT( JDG_VALID(ECLT, eClanType) );

	// �ȼ��ж�
	if (RepGetLvl(eClanType) < eRepLvl)
	{
		return FALSE;
	}

	// ʱ���ж�
	if (dwtResetTime <= GetRepRstTimeStamp())
	{
		return FALSE;
	}

	// ��Ҫ����
	RepSetVal(eClanType, ArrNRepLvlMin[eRepLvl]);

	return TRUE;
}
VOID ClanData::ActCountSetVal( INT8 n8ActCount, ECLanType clantype )
{
	ASSERT( JDG_VALID(ECLT, clantype));
	if (n8ActCount > MAX_ACT_COUNT)
	{
		n8ActCount = MAX_ACT_COUNT;
	}
	m_n8ActCount[clantype] = n8ActCount;
	OnActCountChange(clantype);
}
INT8 ClanData::ActCountGetVal( ECLanType clantype )
{
	ASSERT( JDG_VALID(ECLT, clantype));
		
	return m_n8ActCount[clantype];
}

BOOL ClanData::ActCountDec(ECLanType clantype)
{
	ASSERT( JDG_VALID(ECLT, clantype));
	if (m_n8ActCount[clantype] <= 0)
	{
		return FALSE;
	}
	else
	{
		--m_n8ActCount[clantype];
		OnActCountChange(clantype);
		return TRUE;
	}
}

INT32 ClanData::ClanConGetVal(ECLanType eClanType) const 
{	
	ASSERT( JDG_VALID(ECLT, eClanType));
	return m_nContribution[eClanType];
}
INT32 ClanData::ClanConGetMaxVal(ECLanType eClanType) const 
{
	ASSERT( JDG_VALID(ECLT, eClanType)); 
	EReputationLevel repLvl = m_eRepLevel[eClanType];
	ASSERT( JDG_VALID(ERL, repLvl));

	return ArrNConUpLim[repLvl];
}
INT32 ClanData::ClanConDec( INT32 nNeed, ECLanType eClanType, BOOL bSend /*= FALSE*/ )
{
	ASSERT( JDG_VALID(ECLT, eClanType) ); 
	ASSERT( nNeed >= 0 );

	INT32 nSrc = m_nContribution[eClanType];
	m_nContribution[eClanType] -= nNeed;
	OnClanConChange(eClanType);
	INT32 nChg = nSrc - m_nContribution[eClanType];
	
	if (bSend)
	{
		NET_SIS_gens_contribute send;
		send.nCurClanCon = m_nContribution[eClanType];
		send.nChangeClanCon = -nChg;
		send.byClanType = eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}

	return nChg;
}
INT32 ClanData::ClanConInc( INT32 nIncr, ECLanType eClanType, BOOL bSend /*= FALSE*/ )
{
	ASSERT( JDG_VALID(ECLT, eClanType)); 
	ASSERT( nIncr >= 0 );

	INT32 nSrc = m_nContribution[eClanType];
	m_nContribution[eClanType] += nIncr;
	OnClanConChange(eClanType);
	INT32 nChg = m_nContribution[eClanType] - nSrc;

	if (bSend)
	{
		NET_SIS_gens_contribute send;
		send.nCurClanCon = m_nContribution[eClanType];
		send.nChangeClanCon = nChg;
		send.byClanType = eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}

	return nChg;
}

VOID ClanData::RepSetVal( ECLanType eClanType, INT32 nNewRep, BOOL bSend /*= TRUE*/ )
{
	ASSERT( JDG_VALID( ECLT, eClanType) );
	INT32 nOldRepVal = m_nReputation[eClanType];
	m_nReputation[eClanType] = nNewRep;
	OnRepChange(eClanType);
	INT32 nNewRepVal = m_nReputation[eClanType];
	if (bSend)
	{
		NET_SIS_change_role_repute send;
		send.nRepute	= nNewRepVal;
		send.nChange	= nNewRepVal - nOldRepVal;
		send.byClanType	= eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}
}
VOID ClanData::RepModVal( ECLanType eClanType, INT32 nMod, BOOL bSend /*= TRUE*/ )
{
	ASSERT( JDG_VALID( ECLT, eClanType) );
	INT32 nOldRepVal = m_nReputation[eClanType];
	m_nReputation[eClanType] += nMod;
	OnRepChange(eClanType);
	INT32 nNewRepVal = m_nReputation[eClanType];

	if (bSend)
	{
		NET_SIS_change_role_repute send;
		send.nRepute	= nNewRepVal;
		send.nChange	= nNewRepVal - nOldRepVal;
		send.byClanType	= eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}
}
INT32 ClanData::RepGetVal(ECLanType eClanType)
{
	ASSERT( JDG_VALID( ECLT, eClanType) );
	INT32 nRtv = m_nReputation[eClanType];
	ASSERT( EReputationValid(nRtv) );
	return nRtv;
}
INT16 ClanData::RepGetLvl(ECLanType eClanType)
{
	INT16 nRtv = m_eRepLevel[eClanType];
	ASSERT( JDG_VALID( ERL, nRtv) );
	return nRtv;
}

VOID ClanData::OnRepChange(ECLanType eClanType)
{
	m_bChanged = TRUE;
	// ��������ֵ
	if (m_nReputation[eClanType] < ArrNRepLvlMin[ERL_BEGIN])
	{
		m_nReputation[eClanType] = ArrNRepLvlMin[ERL_BEGIN];
	}
	else if (m_nReputation[eClanType] >= ArrNRepLvlMin[ERL_END])
	{
		m_nReputation[eClanType] = ArrNRepLvlMin[ERL_END];
	}

	// ��������
	EReputationLevel erl = GetRepLvl(m_nReputation[eClanType]);
	if ( ERL_NULL == erl )
	{
		ASSERT(0);
	}
	else if (erl != m_eRepLevel[eClanType])
	{
		m_eRepLevel[eClanType] = erl;
	}
	//  ��������
	
	// ���Խ���������
	if (RepGetVal(eClanType) >= ENTER_FAMEHALL_REP_LIM)
	{
		g_fameHall.TryEnterFameHall(m_pRole, eClanType);
	}	
}
VOID ClanData::OnClanConChange(ECLanType eClanType)
{
	m_bChanged = TRUE;

	BOOL bUpperLimit = FALSE;

	// ��������ֵ
	if (m_nContribution[eClanType] >= ArrNConUpLim[ m_eRepLevel[eClanType] ])
	{
		m_nContribution[eClanType] = ArrNConUpLim[ m_eRepLevel[eClanType] ];
		bUpperLimit = TRUE;
	}
	else if (m_nContribution[eClanType] < ArrNConUpLim[ ERL_BEGIN ])
	{
		m_nContribution[eClanType] = ArrNConUpLim[ ERL_BEGIN ];
	}

	if(bUpperLimit)
	{
		NET_SIS_gens_contribute_upper_limit send;
		send.eClanType = eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}
}
VOID ClanData::OnActCountChange( ECLanType eClanType, BOOL bSend /*= TRUE*/ )
{
	m_bChanged = TRUE;
	if (m_n8ActCount[eClanType] >= MAX_ACT_COUNT)
	{
		m_n8ActCount[eClanType] = MAX_ACT_COUNT;
	}
	else if (m_n8ActCount[eClanType] < 0)
	{
		m_n8ActCount[eClanType] = 0;
	}
	if( bSend )
	{
		NET_SIS_change_treasure_act_count send;
		send.nActCount	= static_cast<BYTE>(m_n8ActCount[eClanType]);
		send.eClanType	= eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}
}
BOOL ClanData::EReputationValid(INT32 nReputation)
{
	if (nReputation < ArrNRepLvlMin[ERL_BEGIN] || nReputation > ArrNRepLvlMin[ERL_END])
	{
		return FALSE;
	}
	return TRUE;
}

EReputationLevel ClanData::GetRepLvl(INT32 nReputation)
{
	for (INT nLvlIndex = ERL_BEGIN; nLvlIndex < ERL_END; ++nLvlIndex)
	{
		if (ArrNRepLvlMin[nLvlIndex] <= nReputation && 
			ArrNRepLvlMin[nLvlIndex + 1] > nReputation)
		{
			return EReputationLevel(nLvlIndex);
		}
	}
	if (REP_FULL_VALUE == nReputation)
	{
		return ERL_Legend;
	}
	return ERL_NULL;
}
VOID ClanData::SetFame(ECLanType eClanType, BOOL bFame/* = TRUE*/)
{ 
	ASSERT(JDG_VALID(ECLT, eClanType)); 
	UINT16 u16Mask	= (UINT16)(1<<eClanType);
	if (bFame)
	{
		m_u16FameMask |= u16Mask;
		NET_SIS_becomeframe send;
		send.eClanType	= eClanType;
		m_pRole->SendMessage(&send, send.dw_size);
	}
	else
		m_u16FameMask &= ~u16Mask;
}
BOOL ClanData::StaticTest()
{
	// ׼������
	s_db_repute_data dbData;
	Prepare(&dbData);

	// ���Գ�ʼ��
	ClanData clandata;
	const BYTE * pData = (const BYTE *)(&dbData);
	clandata.Init(pData, NULL);

	// ���Ա�������
	s_db_repute_data dbDataSave;
	LPVOID ppp;
	BOOL bchg;
	clandata.Save2DB((LPVOID)(&dbDataSave), ppp, bchg);
	
	// ���������ӿ�
	INT32 nVal = 1;
	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		ECLanType eclantype = ECLanType(clantype);
		clandata.RepSetVal(eclantype, -46800);
		nVal = clandata.RepGetLvl(eclantype);
		clandata.RepModVal(eclantype, 100000);
		nVal = clandata.RepGetLvl(eclantype);
	}

	// ���Թ��׽ӿ�
	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		ECLanType eclantype = ECLanType(clantype);
		nVal = clandata.ClanConGetMaxVal(eclantype);
		nVal = clandata.ClanConGetVal(eclantype);
		clandata.ClanConInc(1000, eclantype);
		clandata.ClanConDec(1000, eclantype);
	}

	// ���������ýӿ�
	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		ECLanType eclantype = ECLanType(clantype);
		clandata.ActCountSetVal(1,eclantype);
		nVal = clandata.ActCountGetVal(eclantype);
		nVal = clandata.ActCountDec(eclantype);
		nVal = clandata.ActCountGetVal(eclantype);
	}

	return TRUE;

}

BOOL ClanData::DynamicTest( INT nTestNO , ECLanType eClanType, INT nVal, BOOL bSend /*= TRUE*/ )
{
	if (!JDG_VALID(ECLT,eClanType))
	{
		return FALSE;
	}
	switch(nTestNO)
	{
	case 0:
		{
			INT nMaxConVal = ClanConGetMaxVal(eClanType);
			INT nSrcCon = ClanConGetVal(eClanType);
			ClanConInc(nVal, eClanType, bSend);
			INT nDstCon = ClanConGetVal(eClanType);
		}
		break;
	case 1:
		{
			INT nMaxConVal = ClanConGetMaxVal(eClanType);
			INT nSrcCon = ClanConGetVal(eClanType);
			ClanConDec(nVal, eClanType, bSend);
			INT nDstCon = ClanConGetVal(eClanType);
		}
		break;
	case 2:
		{
			INT nSrcLvl = RepGetLvl(eClanType);
			INT nSrcVal = RepGetVal(eClanType);
			RepModVal(eClanType, nVal, bSend);
			INT nDstLvl = RepGetLvl(eClanType);
			INT nDstVal = RepGetVal(eClanType);
		}
		break;
	case 3:
		{
			INT nSrcLvl = RepGetLvl(eClanType);
			INT nSrcVal = RepGetVal(eClanType);
			RepSetVal(eClanType, nVal, bSend);
			INT nDstLvl = RepGetLvl(eClanType);
			INT nDstVal = RepGetVal(eClanType);
		}
		break;
	case 4:
		{
			INT nActCount = ActCountGetVal(eClanType);
		}
		break;
	case 5:
		{
			BOOL bFameRole = IsFame(eClanType);
			SetFame(eClanType, nVal);
			if (nVal)
			{
				ASSERT(nVal != 0 && IsFame(eClanType));
			}
			else
			{
				ASSERT(nVal == 0 && !IsFame(eClanType));
			}
		}
		break;
	case 6:
		break;
	}
	return TRUE;
}

VOID ClanData::Prepare(s_db_repute_data* pdbData)
{
	for (INT32 clandata = ECLT_BEGIN; clandata < ECLT_END; ++clandata)
	{
		pdbData->n8_act_count_[clandata]		= 0;
		pdbData->n_reputation_[clandata]		= 0;
		pdbData->n_contribution_[clandata]		= 0;
	}
	pdbData->last_reset_time_	= g_world.GetWorldTime();
}