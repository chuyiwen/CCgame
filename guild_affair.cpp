/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
*	@file		guild_affair.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		
*/


#include "StdAfx.h"

#include "../common/ServerDefine/log_server_define.h"

#include "att_res.h"
#include "guild.h"
#include "guild_affair.h"
#include "role_mgr.h"
#include "role.h"

//-----------------------------------------------------------------------------
// 构造和析构
//-----------------------------------------------------------------------------
guild_affair::guild_affair()
{
	b_init			= FALSE;
	by_count		= 0;
	p_guild		= NULL;
}

guild_affair::~guild_affair()
{
}

//-----------------------------------------------------------------------------
// ! 初始化帮会事务
//-----------------------------------------------------------------------------
VOID guild_affair::init( guild* p_guild_ )
{
	p_guild	= p_guild_;
	by_count	= 0;
	p_guild->re_calculate_affair_remain_num(0);
	b_init		= TRUE;
}

//-----------------------------------------------------------------------------
// ! 发布帮会事务
//-----------------------------------------------------------------------------
DWORD guild_affair::issuance_guild_affair( Role* p_role_, DWORD dw_buffer_id_ )
{
	if (!b_init)
	{
		return INVALID_VALUE;
	}

	tagGuildMember* p_member = p_guild->get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}

	if (!p_guild->get_guild_power(p_member->eGuildPos).bAffair)
	{
		return E_Guild_Power_NotEnough;
	}

	if (p_guild->is_in_guild_state_any(EGDSS_Shortage | EGDSS_Distress | EGDSS_Chaos | EGDSS_Warfare))
	{
		return E_Guild_State_Limit;
	}

	//! 发布帮务次数限制
	if (by_count >= MGuildAffairTimes(p_guild->get_guild_att().byLevel))
	{
		return E_GuildAffair_Times_Limit;
	}

	const s_guild_affair_info* p_affair_proto = AttRes::GetInstance()->GetGuildAffairInfo(dw_buffer_id_);
	if (!VALID_POINT(p_affair_proto))	return INVALID_VALUE;
	
	//！帮会等级限制
	if (p_guild->get_guild_att().byLevel < p_affair_proto->by_guild_level)
	{
		return E_Guild_Level_Limit;
	}

	//！ 帮会占领城市限制
	if (p_affair_proto->by_hold_city != 0)
	{
		BOOL b_hold_city = FALSE;
		for (int n=0; n<MAX_GUILD_HOLDCITY; n++)
		{
			if (p_affair_proto->by_hold_city == p_guild->get_guild_att().byHoldCity[n])
			{
				b_hold_city = TRUE;
				break;
			}
		}
		if (!b_hold_city)
		{
			return E_Guild_HoldCity_Limit;
		}
	}

	//！ 帮会资源限制
	if (p_guild->get_guild_att().nFund < p_affair_proto->n_fund)
	{
		return E_Guild_Fund_NotEnough;
	}

	if (p_guild->get_guild_att().nMaterial < p_affair_proto->n_material)
	{
		return E_Guild_Material_NotEnough;
	}

	//！扣除消耗
	p_guild->decrease_guild_fund(p_role_->GetID(), p_affair_proto->n_fund, elcid_guild_spread_affair);
	p_guild->decrease_guild_material(p_role_->GetID(), p_affair_proto->n_material, elcid_guild_spread_affair);

	by_count++;
	p_guild->re_calculate_affair_remain_num(by_count);

	const tagBuffProto* p_buffer_proto = AttRes::GetInstance()->GetBuffProto(dw_buffer_id_);
	if( !VALID_POINT(p_buffer_proto) ) return INVALID_VALUE;

	NET_SIS_issuance_guild_affair send;
	send.dwBuffID		= dw_buffer_id_;
	send.dwRole			= p_role_->GetID();
	send.byRemainTimes	= p_guild->get_guild_att().byAffairRemainTimes;

	MAP_GUILD_MEMBER& map_member = p_guild->get_guild_member_map();

	tagGuildMember* p_guild_member = NULL;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while (map_member.find_next(iter, p_guild_member))
	{
		Role* p_guild_role = g_roleMgr.get_role(p_guild_member->dw_role_id);
		if (VALID_POINT(p_guild_role))
		{
			p_guild_role->TryAddBuff(p_guild_role, p_buffer_proto, NULL, NULL, NULL);
			p_guild_role->SendMessage(&send, send.dw_size);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
//! 重置帮会事务发布次数
//-----------------------------------------------------------------------------
VOID guild_affair::reset_affair_count()
{
	if (!b_init)
	{
		return;
	}

	by_count = 0;
	p_guild->re_calculate_affair_remain_num(0);
}