/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//排行榜相关
#include "StdAfx.h"

#include "player_session.h"
#include "../../common/WorldDefine/rank_protocol.h"
#include "role.h"
#include "role_mgr.h"
#include "RankMgr.h"

// 获取等级排行榜
DWORD PlayerSession::HandleGetLevelRank(tag_net_message* pCmd )
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_level_rank);
	NET_SIC_get_level_rank * p_receive = ( NET_SIC_get_level_rank * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = RankMgr::GetInstance()->SendLevelRank(pRole);

	NET_SIS_get_level_rand_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
//获取师徒榜
DWORD PlayerSession::HandleGetMasterRank(tag_net_message* pCmd)
{
	NET_SIC_get_master_rank *p_receive = (NET_SIC_get_master_rank*)pCmd;
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = RankMgr::GetInstance()->SendMasterRank(pRole);

	NET_SIS_get_master_rand_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
// 获取装备榜
DWORD PlayerSession::HandleGetEquipRank(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_equip_rank);
	NET_SIC_get_equip_rank * p_receive = ( NET_SIC_get_equip_rank * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = RankMgr::GetInstance()->SendEquipRank(pRole);

	NET_SIS_get_equip_rank_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 获取帮会榜
DWORD PlayerSession::HandleGetGuildRank(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_rank);
	NET_SIC_get_guild_rank * p_receive = ( NET_SIC_get_guild_rank * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = RankMgr::GetInstance()->SendGulilRank(pRole);

	NET_SIS_get_guild_rank_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 获取恶人榜
DWORD PlayerSession::HandleGetKillRank(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_skill_rank);
	NET_SIC_get_skill_rank * p_receive = ( NET_SIC_get_skill_rank * ) pCmd ;  	
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = RankMgr::GetInstance()->SendKillRank(pRole);

	NET_SIS_get_skill_rank_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;

}

// 获取正义榜
DWORD PlayerSession::HandleGetJusticeRank(tag_net_message* pCmd)
{
	NET_SIC_get_justice_rank* p_receive = (NET_SIC_get_justice_rank*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_error = RankMgr::GetInstance()->SendJueticeRank(pRole);

	NET_SIS_get_justice_rank_result send;
	send.dwError = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 获取噬魂榜
DWORD PlayerSession::HandleGetShihunRank(tag_net_message* pCmd)
{
	NET_SIC_get_meili_rank* p_receive = (NET_SIC_get_meili_rank*) pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_error = RankMgr::GetInstance()->SendShihunRank(pRole);

	NET_SIS_get_meili_rank_result send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 成就点数榜
DWORD PlayerSession::HandleGetAchPointRank(tag_net_message* pCmd)
{
	NET_SIC_get_ach_point_rank* p_receive = (NET_SIC_get_ach_point_rank*) pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_error = RankMgr::GetInstance()->SendAchPointRank(pRole);

	NET_SIS_get_ach_point_rank_result send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 成就数量榜
DWORD PlayerSession::HandleGetAchNumberRank(tag_net_message* pCmd)
{
	NET_SIC_get_ach_number_rank* p_receive = (NET_SIC_get_ach_number_rank*) pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_error = RankMgr::GetInstance()->SendAchNumberRank(pRole);

	NET_SIS_get_ach_number_rank_result send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 获取1v1积分榜
DWORD PlayerSession::HandGet1v1Rank(tag_net_message* pCmd)
{
	NET_SIC_get_1v1_rank* p_receive = (NET_SIC_get_1v1_rank*)pCmd;
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = RankMgr::GetInstance()->Send1v1ScoreRank(pRole);

	NET_SIS_get_1v1_rank_result send;
	send.dw_error = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
