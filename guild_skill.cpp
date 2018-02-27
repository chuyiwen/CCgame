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
*	@file		guild_skill.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/

#include "StdAfx.h"

#include "guild.h"
#include "guild_skill.h"
#include "att_res.h"
#include "role.h"

#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/guild_server_define.h"

//-----------------------------------------------------------------------------
// 构造与析构
//-----------------------------------------------------------------------------
guild_skill::guild_skill()
{
	b_init			= FALSE;
	p_guild		= NULL;
	dw_current_skill	= INVALID_VALUE;
	map_guild_skill.clear();
}

guild_skill::~guild_skill()
{
	destroy();
}

VOID guild_skill::init( guild* p_guild_, BOOL b_request_ /*= FALSE*/ )
{
	ASSERT(VALID_POINT(p_guild_));
	if (!VALID_POINT(p_guild_))
	{
		return;
	}

	b_init			= FALSE;
	p_guild		= p_guild_;
	dw_current_skill	= INVALID_VALUE;

	map_guild_skill.clear();

	if (b_request_)
	{
		NET_DB2C_load_guild_skill_info send;
		send.dw_guild_id	= p_guild->get_guild_att().dwID;
		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID guild_skill::destroy()
{
	b_init			= FALSE;
	p_guild		= NULL;
	dw_current_skill	= INVALID_VALUE;
	
	tagGuildSkill* p_skill_info = NULL;
	MAP_GUILD_SKILL::map_iter iter = map_guild_skill.begin();
	while (map_guild_skill.find_next(iter, p_skill_info))
	{
		SAFE_DELETE(p_skill_info);
	}
	map_guild_skill.clear();
}

//-----------------------------------------------------------------------------
//! 载入帮派技能数据
//-----------------------------------------------------------------------------
BOOL guild_skill::load_guild_skill_info( s_guild_skill_info* p_skill_info_, INT n_skill_num_ )
{
	if (!VALID_POINT(p_skill_info_) || n_skill_num_ <= 0)
	{
		return FALSE;
	}

	for (int n=0; n<n_skill_num_; n++)
	{
		tagGuildSkill* p_guild_skill	= new tagGuildSkill;
		if (!AttRes::GetInstance()->get_guild_skill_info(p_skill_info_[n].dw_skill_id, p_skill_info_[n].n_level, *p_guild_skill))
		{
			if (p_skill_info_[n].n_level > p_guild->get_guild_facilities().GetMaxSkillLevel())
			{
				map_guild_skill.add(p_skill_info_[n].dw_skill_id, p_guild_skill);
			}
			else
			{
				SAFE_DELETE(p_guild_skill);
			}
			continue;
		}
		p_guild_skill->n16Progress	= p_skill_info_[n].n16_progress;
		map_guild_skill.add(p_skill_info_[n].dw_skill_id, p_guild_skill);

		if (p_skill_info_[n].b_researching)
		{
			if (!VALID_VALUE(dw_current_skill))
			{
				dw_current_skill = p_skill_info_[n].dw_skill_id;
			}
			else if (p_skill_info_[n].dw_skill_id != dw_current_skill)
			{
				change_research_skill(p_skill_info_[n].dw_skill_id);
				dw_current_skill = p_skill_info_[n].dw_skill_id;
			}
		}
	}

	b_init = TRUE;

	return TRUE;
}

//-----------------------------------------------------------------------------
// ! 获取所有帮会技能数据
//-----------------------------------------------------------------------------
DWORD guild_skill::get_guild_skill_info( DWORD& dw_skill_id_, INT16& n16_progress_, INT& n_skill_num_, DWORD* p_level_info_ )
{
	if (!b_init)
	{
		return INVALID_VALUE;
	}

	if (!VALID_POINT(p_level_info_))
	{
		return INVALID_VALUE;
	}
	tagGuildSkill* p_guild_skill	= NULL;
	n_skill_num_					= 0;
	dw_skill_id_					= INVALID_VALUE;
	n16_progress_					= INVALID_VALUE;

	p_guild_skill = map_guild_skill.find(dw_current_skill);
	if (VALID_POINT(p_guild_skill))
	{
		dw_skill_id_		= dw_current_skill * 100 + p_guild_skill->nLevel;
		n16_progress_		= p_guild_skill->n16Progress;
	}
	
	MAP_GUILD_SKILL::map_iter iter = map_guild_skill.begin();
	while (map_guild_skill.find_next(iter, p_guild_skill))
	{
		if (!VALID_POINT(p_guild_skill))
		{
			continue;
		}

		p_level_info_[n_skill_num_++] = p_guild_skill->dwSkillID * 100 + p_guild_skill->nLevel;
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 帮会技能整点升级
//-----------------------------------------------------------------------------
DWORD guild_skill::skill_upgrade_on_clock()
{
	if (!b_init)
	{
		return INVALID_VALUE;
	}

	if (!VALID_VALUE(dw_current_skill))
	{
		return E_GuildSkill_NotResearch;
	}

	tagGuildSkill* p_guild_skill = map_guild_skill.find(dw_current_skill);
	if (!VALID_POINT(p_guild_skill))
	{
		return E_GuildSkill_NotExist;
	}

	if (p_guild_skill->nLevel > p_guild->get_guild_facilities().GetMaxSkillLevel())
	{
		return E_GuildSkill_Level_Limit;
	}

	if (p_guild->is_in_guild_state_any(EGDSS_Shortage | EGDSS_Distress | EGDSS_Chaos))
	{
		return E_Guild_State_Limit;
	}

	FLOAT f_modify = 1.0f;
	if (p_guild->is_in_guild_state(EGDSS_Refulgence))
	{
		f_modify = 1.5f;
	}
	else if (p_guild->is_in_guild_state(EGDSS_Primacy))
	{
		f_modify = 2.0f;
	}

	const s_guild& st_att = p_guild->get_guild_att();
	if (st_att.nFund < p_guild_skill->nResearchFund)
	{
		return E_Guild_Fund_NotEnough;
	}
	if (st_att.nMaterial < p_guild_skill->nResearchMaterial)
	{
		return E_Guild_Material_NotEnough;
	}

	p_guild->decrease_guild_fund(INVALID_VALUE, p_guild_skill->nResearchFund, elcid_guild_skill_research);
	p_guild->decrease_guild_material(INVALID_VALUE, p_guild_skill->nResearchMaterial, elcid_guild_skill_research);

	INT16 n16_upgrade = INT16(st_att.byLevel * 10.0f * (1.0f + (st_att.n16Peace - 5000.0f) / 10000.0f) * f_modify);

	p_guild_skill->n16Progress += n16_upgrade;
	if (p_guild_skill->n16Progress >= p_guild_skill->n16Fulfill)
	{
		INT n_max_level = p_guild->get_guild_facilities().GetMaxSkillLevel();

		p_guild_skill->nLevel++;
		if (p_guild_skill->nLevel > n_max_level)
		{
			dw_current_skill				= INVALID_VALUE;
			p_guild_skill->n16Progress	= 0;
			change_research_skill(INVALID_VALUE);

			NET_SIS_set_research_skill send;
			send.dwSkillID		= INVALID_VALUE;
			send.n16Progress	= 0;
			p_guild->send_guild_message(&send, send.dw_size);
		}
		else
		{
			AttRes::GetInstance()->get_guild_skill_info(p_guild_skill->dwSkillID, p_guild_skill->nLevel, *p_guild_skill);

			p_guild_skill->n16Progress -= p_guild_skill->n16Fulfill;
		}

		NET_SIS_guild_skill_level_up send_broad;
		send_broad.dwSkillID = p_guild_skill->dwSkillID * 100 + p_guild_skill->nLevel;
		p_guild->send_guild_message(&send_broad, send_broad.dw_size);
	}

	save_skill_info_to_db(p_guild_skill->dwSkillID);

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 上缴物品升级帮会技能
//-----------------------------------------------------------------------------
DWORD guild_skill::skill_upgrade_turn_in( Role* p_role_, INT64 n64_serial_, DWORD& dw_skill_id_, INT16& n16_progress_ )
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
	if (!p_guild->get_guild_power(p_member->eGuildPos).bAdvSkill)
	{
		return E_Guild_Power_NotEnough;
	}

	if (!VALID_VALUE(dw_current_skill))
	{
		return E_GuildSkill_NotResearch;
	}

	tagGuildSkill* p_guild_skill = map_guild_skill.find(dw_current_skill);
	if (!VALID_POINT(p_guild_skill))
	{
		return E_GuildSkill_NotExist;
	}

	if (p_guild_skill->nLevel > p_guild->get_guild_facilities().GetMaxSkillLevel())
	{
		return E_GuildSkill_Level_Limit;
	}

	tagItem* p_item = p_role_->GetItemMgr().GetBagItem(n64_serial_);
	if (!VALID_POINT(p_item) || p_item->pProtoType->eSpecFunc != EISF_CreateGuild)
	{
		return E_GuildSkill_Wrong_Item;
	}

	if (p_guild->is_in_guild_state_any(EGDSS_Shortage | EGDSS_Distress | EGDSS_Chaos))
	{
		return E_Guild_State_Limit;
	}

	FLOAT fModify = 1.0f;
	if (p_guild->is_in_guild_state(EGDSS_Refulgence))
	{
		fModify = 1.5f;
	}
	else if (p_guild->is_in_guild_state(EGDSS_Primacy))
	{
		fModify = 2.0f;
	}

	const s_guild& st_att = p_guild->get_guild_att();
	if (st_att.nFund < p_guild_skill->nResearchFund)
	{
		return E_Guild_Fund_NotEnough;
	}
	if (st_att.nMaterial < p_guild_skill->nResearchMaterial)
	{
		return E_Guild_Material_NotEnough;
	}

	p_guild->decrease_guild_fund(p_role_->GetID(), p_guild_skill->nResearchFund, elcid_guild_skill_research);
	p_guild->decrease_guild_material(p_role_->GetID(), p_guild_skill->nResearchMaterial, elcid_guild_skill_research);

	INT16 n16_upgrade = INT16(p_item->pProtoType->nSpecFuncVal1 * fModify);

	p_guild_skill->n16Progress += n16_upgrade;
	if (p_guild_skill->n16Progress >= p_guild_skill->n16Fulfill)
	{
		INT n_max_level = p_guild->get_guild_facilities().GetMaxSkillLevel();

		p_guild_skill->nLevel++;
		if (p_guild_skill->nLevel > n_max_level)
		{
			dw_current_skill				= INVALID_VALUE;
			p_guild_skill->n16Progress	= 0;
			change_research_skill(INVALID_VALUE);

			NET_SIS_set_research_skill send;
			send.dwSkillID		= INVALID_VALUE;
			send.n16Progress	= 0;
			p_guild->send_guild_message(&send, send.dw_size);
		}
		else
		{
			AttRes::GetInstance()->get_guild_skill_info(p_guild_skill->dwSkillID, p_guild_skill->nLevel, *p_guild_skill);

			p_guild_skill->n16Progress -= p_guild_skill->n16Fulfill;
		}

		NET_SIS_guild_skill_level_up send_broad;
		send_broad.dwSkillID = p_guild_skill->dwSkillID * 100 + p_guild_skill->nLevel;
		p_guild->send_guild_message(&send_broad, send_broad.dw_size);
	}

	save_skill_info_to_db(p_guild_skill->dwSkillID);

	p_guild->change_member_contribution(p_role_->GetID(), p_item->pProtoType->nSpecFuncVal1, TRUE);

	p_role_->GetItemMgr().RemoveFromRole(p_item->dw_data_id, 1, elcid_guild_skill_research);

	dw_skill_id_	= p_guild_skill->dwSkillID * 100 + p_guild_skill->nLevel;
	n16_progress_	= p_guild_skill->n16Progress;

	return E_Success;
}

//-----------------------------------------------------------------------------
// ！ 学习帮会技能
//-----------------------------------------------------------------------------
DWORD guild_skill::learn_guild_skill( Role* p_role_, DWORD dw_skill_id_, INT& n_level_, BOOL& b_learn_error_ )
{
	if (!b_init)
	{
		return INVALID_VALUE;
	}

	if (!VALID_POINT(p_role_) || !VALID_VALUE(dw_skill_id_))
	{
		return INVALID_VALUE;
	}

	b_learn_error_	= FALSE;

	tagGuildMember* p_member = p_guild->get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}

	tagGuildSkill* p_guild_skill = map_guild_skill.find(dw_skill_id_);
	if (!VALID_POINT(p_guild_skill))
	{
		return E_GuildSkill_NotExist;
	}

	BOOL b_learn = TRUE;
	Skill* p_skill = p_role_->GetSkill(dw_skill_id_);
	if (VALID_POINT(p_skill))
	{
		b_learn = FALSE;		
		if (p_guild_skill->nLevel <= p_skill->get_level()+1)
		{
			return E_GuildSkill_Level_Limit;
		}
	}

	if (p_guild->is_in_guild_state_any(EGDSS_Shortage | EGDSS_Distress | EGDSS_Warfare))
	{
		return E_Guild_State_Limit;
	}

	const tagGuildSkill* p_skill_proto = NULL;
	if (VALID_POINT(p_skill))
	{
		p_skill_proto = AttRes::GetInstance()->GetGuildSkillProto(p_skill->GetTypeID() + 1);
	}
	else
	{
		p_skill_proto = AttRes::GetInstance()->GetGuildSkillProto(Skill::CreateTypeID(dw_skill_id_, 1));
	}
	
	if (!VALID_POINT(p_skill_proto))
	{
		return E_GuildSkill_Level_Limit;
	}

	if (p_role_->GetCurMgr().GetBagSilver() < p_skill_proto->nLearnSilver)
	{
		return E_BagSilver_NotEnough;
	}
	if (p_member->nContribution < p_skill_proto->nLearnContribution)
	{
		return E_GuildMember_Contribution_Limit;
	}
	if(p_guild->get_guild_att().nFund < p_skill_proto->nLearnFund)
	{
		return E_Guild_Fund_NotEnough;
	}
	if (p_guild->get_guild_att().nMaterial < p_skill_proto->nLearnMaterial)
	{
		return E_Guild_Material_NotEnough;
	}

	b_learn_error_			= TRUE;
	DWORD dw_error_code	= INVALID_VALUE;
	if (b_learn)
	{
		dw_error_code = p_role_->LearnSkill(dw_skill_id_);
	}
	else
	{
		dw_error_code = p_role_->LevelUpSkill(dw_skill_id_);
	}

	if (E_Success == dw_error_code)
	{
		p_role_->GetCurMgr().DecBagSilver(p_skill_proto->nLearnSilver, elcid_guild_skill_learn);
		p_guild->change_member_contribution(p_role_->GetID(), p_skill_proto->nLearnContribution, FALSE);
		p_guild->decrease_guild_fund(p_role_->GetID(), p_skill_proto->nLearnFund, elcid_guild_skill_learn);
		p_guild->decrease_guild_material(p_role_->GetID(), p_skill_proto->nLearnMaterial, elcid_guild_skill_learn);
	}

	p_skill = p_role_->GetSkill(dw_skill_id_);
	if (VALID_POINT(p_skill))
	{
		n_level_	= p_skill->get_level();
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
//! 创建帮会技能
//-----------------------------------------------------------------------------
VOID guild_skill::create_guild_skill()
{
	AttRes::GetInstance()->LoadGuildSkillInfo(map_guild_skill);

	dw_current_skill = INVALID_VALUE;

	tagGuildSkill* p_skill_info = NULL;
	MAP_GUILD_SKILL::map_iter iter = map_guild_skill.begin();
	while (map_guild_skill.find_next(iter, p_skill_info))
	{
		if (!VALID_POINT(p_skill_info))
		{
			continue;
		}
		
		NET_DB2C_create_guild_skill send;
		send.dw_guild_id					= p_guild->get_guild_att().dwID;
		send.s_skill_info.n_level			= p_skill_info->nLevel;
		send.s_skill_info.dw_skill_id		= p_skill_info->dwSkillID;
		send.s_skill_info.n16_progress		= p_skill_info->n16Progress;
		send.s_skill_info.b_researching	= FALSE;

		g_dbSession.Send(&send, send.dw_size);
	}

	b_init = TRUE;
}

//-----------------------------------------------------------------------------
// ! 设置当前要升级的帮会技能
//-----------------------------------------------------------------------------
DWORD guild_skill::set_current_upgrade_guild_skill( DWORD dw_role_id_, DWORD dw_skill_id_, INT& nLevel_, INT16& n16_progress_ )
{
	if (!b_init)
	{
		return INVALID_VALUE;
	}

	if (!VALID_VALUE(dw_role_id_))
	{
		return INVALID_VALUE;
	}

	if (dw_skill_id_ == dw_current_skill)
	{
		return INVALID_VALUE;
	}

	tagGuildMember* p_member = p_guild->get_member(dw_role_id_);
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}
	if (!p_guild->get_guild_power(p_member->eGuildPos).bSetSkill)
	{
		return E_Guild_Power_NotEnough;
	}

	if (VALID_VALUE(dw_skill_id_))
	{
		tagGuildSkill* p_guild_skill = map_guild_skill.find(dw_skill_id_);
		if (!VALID_POINT(p_guild_skill))
		{
			return E_GuildSkill_NotExist;
		}
		if (p_guild_skill->nLevel > p_guild->get_guild_facilities().GetMaxSkillLevel())
		{
			return E_GuildSkill_Level_Limit;
		}
		n16_progress_	= p_guild_skill->n16Progress;
		nLevel_		= p_guild_skill->nLevel;

		AttRes::GetInstance()->get_guild_skill_info(p_guild_skill->dwSkillID, p_guild_skill->nLevel, *p_guild_skill);
	}

	change_research_skill(dw_skill_id_);
	dw_current_skill	= dw_skill_id_;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 与数据库通讯
//-----------------------------------------------------------------------------
VOID guild_skill::save_skill_info_to_db(DWORD dw_skill_id_)
{
	if (!VALID_VALUE(dw_skill_id_))
	{
		return;
	}

	tagGuildSkill* p_guild_skill = map_guild_skill.find(dw_skill_id_);
	if (!VALID_POINT(p_guild_skill))
	{
		return;
	}

	NET_DB2C_save_guild_skill send;
	send.dw_guild_id				= p_guild->get_guild_att().dwID;
	send.s_skill_info.n_level		= p_guild_skill->nLevel;
	send.s_skill_info.dw_skill_id	= p_guild_skill->dwSkillID;
	send.s_skill_info.n16_progress	= p_guild_skill->n16Progress;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_skill::change_research_skill( DWORD dw_new_skill_id_ )
{
	NET_DB2C_change_research_skill send;
	send.dw_guild_id		= p_guild->get_guild_att().dwID;
	send.dw_new_skill_id	= dw_new_skill_id_;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ! 清除所有帮会技能数据
//-----------------------------------------------------------------------------
VOID guild_skill::remove_guild_skill_info()
{
	if (!b_init)
	{
		return;
	}

	NET_DB2C_remove_guild_skill send;
	send.dw_guild_id = p_guild->get_guild_att().dwID;

	g_dbSession.Send(&send, send.dw_size);
}