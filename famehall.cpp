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

#include "db_session.h"
#include "att_res.h"
#include "clandata.h"
#include "famehall.h"
#include "role.h"

#include "../common/ServerDefine/famehall_server_define.h"

FameHall g_fameHall;

// ���̵߳���
VOID FameHall::Update()
{
	for (INT32 nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		m_ArrClanTrunk[nClanType].Update();
	}
}

// ��������������Ϣ
VOID FameHall::HandleUpdateRepRank(tag_net_message* pCmd)
{
	NET_DB2S_get_rep_rank_list* p_receive = (NET_DB2S_get_rep_rank_list*)pCmd;
	ASSERT(JDG_VALID(ECLT, p_receive->byClanType)); 
	m_ArrClanTrunk[p_receive->byClanType].HandleUpdateRepRank(p_receive->repRank, p_receive->nRecNum);
}


// ��ͼ�̵߳���
BOOL FameHall::TryEnterFameHall(Role* pRole, ECLanType eClanType)
{
	return m_ArrClanTrunk[eClanType].TryEnterFameHall(pRole);
}

// ��ȡ���������ǰ50��nameid��
VOID FameHall::GetMemberTop50(BYTE* pData, ECLanType eClanType)
{
	m_ArrClanTrunk[eClanType].GetFameHallTop50(pData);
}

// ��ȡ���������ǰ50������Ŀ
INT32 FameHall::GetMemberTop50Num(ECLanType byClanType)
{
	return m_ArrClanTrunk[byClanType].GetFameHallTop50Num();
}

// �������������tagRepRankData��
VOID FameHall::GetRepRankTop(PVOID pData, ECLanType eClanType)
{
	m_ArrClanTrunk[eClanType].GetRepRank(pData);
}

// �������������С
INT32 FameHall::GetRepRankNum(ECLanType byClanType)
{
	return m_ArrClanTrunk[byClanType].GetRepRankNum();
}
// ����Ѽ��������䱦�б�
VOID FameHall::GetActClanTreasure(PVOID pData, ECLanType eClanType)
{
	m_ArrClanTrunk[eClanType].GetActTreasureList(pData);
}

// ����Ѽ��������䱦��С
INT32 FameHall::GetActClanTreasureNum( ECLanType byClanType )
{
	return m_ArrClanTrunk[byClanType].GetActTreasureNum();
}

BOOL FameHall::Init()
{
	for (INT32 nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		if (FALSE == m_ArrClanTrunk[nClanType].Init((ECLanType)nClanType, ENTER_FAMEHALL_REP_LIM, ENTER_ACTCOUNT_REWARD))
			return FALSE;
	}
	m_bInitOK = FALSE;
	return TRUE;
}

// �������ݿ���Ϣ
VOID FameHall::SendLoadDBData()
{
	NET_DB2C_fame_hall_init_start	fameHallInitStart;

	g_dbSession.Send(&fameHallInitStart, fameHallInitStart.dw_size);
}

// �����ʼ�������䱦�б���Ϣ
VOID FameHall::HandleInitActTreasureList(tag_net_message* pCmd)
{
	NET_DB2S_get_act_treasure_list* p_receive = (NET_DB2S_get_act_treasure_list*)pCmd;
	INT nOffset = 0;
	for (INT nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		m_ArrClanTrunk[nClanType].HandleInitActTreasureList(&(p_receive->treasureData[nOffset]), p_receive->nRecNum[nClanType]);
		nOffset += p_receive->nRecNum[nClanType];
		ASSERT( nOffset <= MAX_ACT_TREASURE_NUM );
	}

}

// ��ʼ����������Ϣ
VOID FameHall::HandleInitFameHallTop50(tag_net_message* pCmd)
{
	NET_DB2S_get_fame_hall_enter_snap* p_receive = (NET_DB2S_get_fame_hall_enter_snap*)pCmd;
	INT nOffset = 0;
	for (INT nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		ASSERT ( p_receive->nRecNum[nClanType] <= MAX_ENTER_FAMEHALL_SNAP_NUM );
		m_ArrClanTrunk[nClanType].HandleInitFameHallTop50(&(p_receive->enterSnap[nOffset]), p_receive->nRecNum[nClanType]);
		nOffset += p_receive->nRecNum[nClanType];
	}
	
}

// ��ʼ����������ʱ��
VOID FameHall::HandleInitRepRstTimeStamp(tag_net_message* pCmd)
{
	NET_DB2S_get_rep_reset_times_tamp* p_receive = (NET_DB2S_get_rep_reset_times_tamp*)pCmd;

	for (INT16 nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		m_ArrClanTrunk[nClanType].HandleInitRepRstTimeStamp(tagDWORDTime(p_receive->dwResetTime[nClanType]));
	}
	Interlocked_Exchange((LPLONG)&m_bInitOK, TRUE);
}

// ���������䱦�����������̣߳�
DWORD FameHall::ActClanTreasure( Role* pRole, UINT16 u16TreasureID )
{
	const tagClanTreasureProto* pTreasureProto = AttRes::GetInstance()->GetClanTreasureProto(u16TreasureID);
	ASSERT( VALID_POINT(pTreasureProto) );
	return m_ArrClanTrunk[pTreasureProto->eClanType].ActiveClanTreasure(pRole, u16TreasureID);
}

VOID FameHall::RoleRepUpdate( Role* pRole, ECLanType eClanType )
{
	ASSERT(VALID_POINT(pRole));
	ClanData* pClanData = &pRole->GetClanData();

	BOOL bBeReset = FALSE;

	if (eClanType != ECLT_NULL)
	{
		bBeReset = pClanData->ResetReputation(m_ArrClanTrunk[eClanType].GetEnterSnapUpdateTime(), eClanType, ERL_Legend);
	}
	else
	{
		for (INT nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
		{
			if ( pClanData->ResetReputation(m_ArrClanTrunk[nClanType].GetEnterSnapUpdateTime(), (ECLanType)nClanType, ERL_Legend) )
			{
				bBeReset = TRUE;
			}
		}

	}
	if (bBeReset)
	{
		pClanData->SetRepRstTimeStamp(g_world.GetWorldTime());
	}
}