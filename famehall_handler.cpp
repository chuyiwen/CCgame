
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//称号消息处理过程

#include "stdafx.h"
#include "player_session.h"
#include "famehall.h"
#include "world.h"
#include "../../common/WorldDefine/celeb_protocol.h"


//player session
DWORD PlayerSession::HandleGetFameHallRoles(tag_net_message* pCmd)
{
	
	NET_SIC_celeb_role* p_receive = (NET_SIC_celeb_role*)pCmd;

	if (!JDG_VALID(ECLT, p_receive->byClanType))
		return INVALID_VALUE;

	tagDWORDTime dwtUpdate = g_fameHall.GetFameSnapTimeStamp((ECLanType)p_receive->byClanType);

	if (dwtUpdate <= p_receive->dwUpdateTime)
	{
		NET_SIS_celeb_role send;
		send.eClanType = p_receive->byClanType;
		send.byErrCode = E_FrameHall_MemberOrderUnchanged;
		send.dwUpdateTime = dwtUpdate;
		send.byRoleNum = 0;
		
		SendMessage(&send, send.dw_size);
		return 0;
	}
	
	
	DWORD dwNum = g_fameHall.GetMemberTop50Num((ECLanType)p_receive->byClanType);

	DWORD dw_size =  dwNum * sizeof(DWORD) - 1 + sizeof(NET_SIS_celeb_role);

	CREATE_MSG(pSend, dw_size, NET_SIS_celeb_role);

	pSend->byRoleNum = (BYTE)dwNum;
	pSend->eClanType = p_receive->byClanType;
	pSend->dwUpdateTime = g_world.GetWorldTime();
	g_fameHall.GetMemberTop50(pSend->byData, (ECLanType)p_receive->byClanType);

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return 0;
}

//player session
DWORD PlayerSession::HandleGetReputeTop(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_reputation_top);
	NET_SIC_reputation_top * p_receive = ( NET_SIC_reputation_top * ) pCmd ;  
	if (!JDG_VALID(ECLT, p_receive->byClanType))
		return INVALID_VALUE;

	tagDWORDTime dwtUpdate = g_fameHall.GetRepRankTimeStamp((ECLanType)p_receive->byClanType);

	// 已经是最新的不用更新
	if (dwtUpdate <= p_receive->dwUpdateTime)
	{
		NET_SIS_reputation_top send;
		send.byRoleNum = p_receive->byClanType;
		send.byErrCode = E_FrameHall_RepOrderUnchanged;
		send.dwUpdateTime = dwtUpdate;
		send.byRoleNum = 0;

		SendMessage(&send, send.dw_size);

		return 0;
	}

	DWORD dwNum = g_fameHall.GetRepRankNum((ECLanType)p_receive->byClanType);

	if (dwNum > 0)
	{
		DWORD dw_size = dwNum * sizeof(tagRepRankData) - 1 + sizeof(NET_SIS_reputation_top);

		CREATE_MSG(pSend, dw_size, NET_SIS_reputation_top);

		pSend->byRoleNum = (BYTE)dwNum;
		pSend->byClanType = p_receive->byClanType;
		pSend->dwUpdateTime = g_world.GetWorldTime();
		g_fameHall.GetRepRankTop(pSend->byData, (ECLanType)p_receive->byClanType);

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}
	else
	{
		NET_SIS_reputation_top send;
		send.byClanType = p_receive->byClanType;
		send.byRoleNum = 0;
		send.dwUpdateTime = dwtUpdate;
		SendMessage(&send, send.dw_size);
	}
	
	return 0;
}

//player session
DWORD PlayerSession::HandleGetActClanTreasure(tag_net_message* pCmd)
{

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_activation_gens_treasure);
	NET_SIC_get_activation_gens_treasure * p_receive = ( NET_SIC_get_activation_gens_treasure * ) pCmd ;  
	if (!JDG_VALID(ECLT, p_receive->byClanType))
		return INVALID_VALUE;

	DWORD dwNum = g_fameHall.GetActClanTreasureNum((ECLanType)p_receive->byClanType);
	if (dwNum > 0)
	{
		DWORD dw_size = dwNum * sizeof(tagTreasureData) - 1 + sizeof(NET_SIS_get_activation_gens_treasure);

		CREATE_MSG(pSend, dw_size, NET_SIS_get_activation_gens_treasure);

		pSend->n16ActTreasureNum = (BYTE)dwNum;
		pSend->byClanType = p_receive->byClanType;

		g_fameHall.GetActClanTreasure(pSend->byData, (ECLanType)p_receive->byClanType);

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}
	else
	{
		NET_SIS_get_activation_gens_treasure send;
		send.byClanType = p_receive->byClanType;
		send.n16ActTreasureNum = 0;

		SendMessage(&send, send.dw_size);
	}
	return 0;
}

//world session
DWORD PlayerSession::HandleActiveTreasure(tag_net_message* pCmd)
{

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_activation_treasure);
	NET_SIC_activation_treasure * p_receive = ( NET_SIC_activation_treasure * ) pCmd ;  
	DWORD dwRtv = g_fameHall.ActClanTreasure(m_pRole, p_receive->u16TreasureID);

	NET_SIS_activation_treasure send;
	send.u16TreasureID = p_receive->u16TreasureID;
	send.dwErrCode = dwRtv;
	SendMessage(&send, send.dw_size);

	return 0;
}