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
*	@file		guild_commerce.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		帮会跑商
*/


#include "StdAfx.h"

#include "../common/ServerDefine/guild_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "../../common/WorldDefine/guild_protocol.h"

#include "guild.h"
#include "guild_commodity.h"
#include "guild_chamber.h"
#include "role.h"
#include "db_session.h"
#include "role_mgr.h"

//-----------------------------------------------------------------------------
// 构造和析构
//-----------------------------------------------------------------------------
guild_commerce::guild_commerce()
{
	p_guild	= NULL;
	b_commend	= FALSE;
	map_commodity.clear();
	vector_rank.clear();
	b_init_ok	= FALSE;
}

guild_commerce::~guild_commerce()
{
	destroy();
}


BOOL guild_commerce::init( guild* p_guild_, BOOL b_request_ /*= FALSE*/ )
{
	if (!VALID_POINT(p_guild_))
	{
		return FALSE;
	}
	p_guild	= p_guild_;
	b_commend	= p_guild_->get_guild_att().bCommendation;
	map_commodity.clear();
	vector_rank.clear();

	if (b_request_)
	{
		NET_DB2C_load_commodity send;
		send.dw_guild_id	= p_guild_->get_guild_att().dwID;
		g_dbSession.Send(&send, send.dw_size);

		NET_DB2C_load_commerce_rank send_rank;
		send_rank.dw_guild_id	= p_guild_->get_guild_att().dwID;
		g_dbSession.Send(&send_rank, send_rank.dw_size);

		b_init_ok		= FALSE;
	}
	else
	{
		b_init_ok	= TRUE;
	}

	return TRUE;
}

VOID guild_commerce::destroy()
{
	b_init_ok	= FALSE;

	guild_commodity* p_commodity = NULL;
	MAP_COMMODITY::map_iter iter_commodity = map_commodity.begin();
	while (map_commodity.find_next(iter_commodity, p_commodity))
	{
		SAFE_DELETE(p_commodity);
	}
	map_commodity.clear();

	VECTOR_RANK::iterator iter_rank = vector_rank.begin();
	while (iter_rank != vector_rank.end())
	{
		tagCommerceRank* p_rank_info	= *iter_rank;
		SAFE_DELETE(p_rank_info);
		iter_rank++;
	}
	vector_rank.clear();
}

//-----------------------------------------------------------------------------
//! 接受跑商任务
//-----------------------------------------------------------------------------
DWORD guild_commerce::accept_commerce( Role* p_role_ )
{
	if (!VALID_POINT(p_role_))
	{
		return INVALID_VALUE;
	}

	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	if (p_role_->get_level() < 20)
	{
		return E_GuildMember_Level_Limit;
	}

	if (p_guild->is_in_guild_state_any(EGDSS_Warfare))
	{
		return E_Guild_State_Limit;
	}

	tagGuildMember* p_member = p_guild->get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}
	if (!p_guild->get_guild_power(p_member->eGuildPos).bCommerce)
	{
		return E_Guild_Power_NotEnough;
	}

	//! 是否有足够的保证金
	const s_commerce_info* pCommerceInfo = AttRes::GetInstance()->GetGuildCommerceInfo(p_role_->get_level());
	if (p_role_->GetCurMgr().GetBagSilver() < pCommerceInfo->tagTaelInfo.nDeposit)
	{
		return E_BagSilver_NotEnough;
	}

	if (p_role_->IsInRoleState(ERS_Commerce))
	{
		return E_GuildCommerce_Status_Error;
	}

	p_role_->GetCurMgr().DecBagSilver(pCommerceInfo->tagTaelInfo.nDeposit, elcid_guild_commerce);

	guild_commodity* p_commodity = map_commodity.find(p_role_->GetID());
	if (VALID_POINT(p_commodity))
	{
		SAFE_DELETE(p_commodity);
		map_commodity.erase(p_role_->GetID());
	}
	p_commodity = new guild_commodity;
	p_commodity->init(p_role_->GetID(), p_role_->get_level(), &pCommerceInfo->tagTaelInfo, &pCommerceInfo->s_redound_info_);
	map_commodity.add(p_role_->GetID(), p_commodity);

	NET_DB2C_create_commodity send;
	send.dw_guild_id	= p_guild->get_guild_att().dwID;
	send.dw_role_id	= p_role_->GetID();
	send.n_level		= p_role_->get_level();
	send.n_tael		= pCommerceInfo->tagTaelInfo.nBeginningTael;
	g_dbSession.Send(&send, send.dw_size);

	p_role_->SetRoleState(ERS_Commerce);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ! 完成跑商任务
//-----------------------------------------------------------------------------
DWORD guild_commerce::complete_commerce( Role* p_role_, INT32& n_fund_ )
{
	if (!VALID_POINT(p_role_))
	{
		return INVALID_VALUE;
	}
	
	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	if (p_guild->is_in_guild_state_any(EGDSS_Warfare))
	{
		return E_Guild_State_Limit;
	}

	if (!p_role_->IsInRoleState(ERS_Commerce))
	{
		return E_GuildCommerce_Status_Error;
	}

	//! 判断商银指标
	guild_commodity* pCommodity = map_commodity.find(p_role_->GetID());
	if (!VALID_POINT(pCommodity))
	{
		return E_GuildCommerce_Status_Error;
	}
	if (pCommodity->get_tael_progress() < 100)
	{
		return E_GuildCommerce_Tael_NotEnough;
	}

	n_fund_	= (INT32)(pCommodity->get_gain() * 0.9f + 0.5f);
	p_guild->increase_guild_fund(p_role_->GetID(), n_fund_, elcid_guild_commerce);


	const s_redound_info* p_redound_info	= pCommodity->get_redound_info();
	const tagTaelInfo* p_tael_info		= pCommodity->get_tael_info();
	if (VALID_POINT(p_redound_info) && VALID_POINT(p_tael_info))
	{
		//p_role_->ExpChange(p_redound_info->n_exp);													
		p_guild->change_member_contribution(p_role_->GetID(), p_redound_info->n_contribution, TRUE);	

		INT32 n_redound_siliver = 0;
		if (p_guild->is_in_guild_state_any(EGDSS_Distress | EGDSS_Shortage))
		{
			n_redound_siliver = (INT32)(pCommodity->get_gain() * 0.1f + pCommodity->get_tael_info()->nDeposit * 1.2f + 0.5f);
		}
		else
		{
			n_redound_siliver = (INT32)(pCommodity->get_gain() * 0.1f + pCommodity->get_tael_info()->nDeposit * 0.9f + 0.5f);
		}
		p_role_->GetCurMgr().IncBagSilver(n_redound_siliver, elcid_guild_commerce);
	}

	p_role_->UnsetRoleState(ERS_Commerce);

	add_to_commerce_rank(p_role_->GetID(), n_fund_);

	SAFE_DELETE(pCommodity);
	map_commodity.erase(p_role_->GetID());

	NET_DB2C_remove_commodity send;
	send.dw_role_id	= p_role_->GetID();
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 放弃跑商任务
//-----------------------------------------------------------------------------
DWORD guild_commerce::abandon_commerce( DWORD dw_role_id_, BOOL b_clear_rank_ /*= FALSE*/ )
{
	if (!VALID_VALUE(dw_role_id_))
	{
		return INVALID_VALUE;
	}

	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	if (b_clear_rank_)
	{
		remove_commerce_rank(dw_role_id_);
	}

	guild_commodity* p_commodity	= map_commodity.find(dw_role_id_);
	if (!VALID_POINT(p_commodity))
	{
		return E_GuildCommerce_Status_Error;
	}

	//! 增加帮会资金
	INT32 n_fund	= (INT32)(p_commodity->get_gain() * 0.9f + 0.5f);
	p_guild->increase_guild_fund(dw_role_id_, n_fund, elcid_guild_commerce);

	SAFE_DELETE(p_commodity);
	map_commodity.erase(dw_role_id_);

	Role* pRole = g_roleMgr.get_role(dw_role_id_);
	if (VALID_POINT(pRole))
	{
		pRole->UnsetRoleState(ERS_Commerce);
		NET_SIS_abandon_commerce send;
		pRole->SendMessage(&send, send.dw_size);
	}

	NET_DB2C_remove_commodity send;
	send.dw_role_id	= dw_role_id_;
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 获取跑商排行榜
//-----------------------------------------------------------------------------
DWORD guild_commerce::get_commerce_rank_info( tagCommerceRank* p_rank_info_, INT& n_rank_num_, BOOL& b_commend_ )
{
	if (!VALID_POINT(p_rank_info_))
	{
		return INVALID_VALUE;
	}

	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	n_rank_num_	= 0;
	b_commend_	= b_commend;

	VECTOR_RANK	vector_rank_info(MAX_COMMERCE_RANK_INFO);
	partial_sort_copy(vector_rank.begin(), vector_rank.end(), vector_rank_info.begin(), vector_rank_info.end(), rank_compare());

	VECTOR_RANK::iterator iter_rank = vector_rank_info.begin();
	for (; iter_rank != vector_rank_info.end(); iter_rank++)
	{
		if (!VALID_POINT(*iter_rank))
		{
			break;
		}
		p_rank_info_[n_rank_num_++] = *(*iter_rank);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// ! 开/关 跑商奖励
//-----------------------------------------------------------------------------
DWORD guild_commerce::switch_commendation( DWORD dw_role_id_, BOOL b_on_ )
{
	if (!VALID_VALUE(dw_role_id_))
	{
		return INVALID_VALUE;
	}

	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	tagGuildMember* p_member	= p_guild->get_member(dw_role_id_);
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}
	if (!p_guild->get_guild_power(p_member->eGuildPos).bSetCommend)
	{
		return E_Guild_Power_NotEnough;
	}

	if (p_guild->get_guild_att().byLevel < 3)
	{
		return E_Guild_Level_Limit;
	}

	if (b_commend != b_on_)
	{
		b_commend	= b_on_;

		NET_DB2C_set_commendation send;
		send.dw_guild_id	= p_guild->get_guild_att().dwID;
		send.b_commend	= b_commend;
		g_dbSession.Send(&send, send.dw_size);

	}
	else
	{
		return E_GuildCommerce_Commend_Error;
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// ! 跑商奖励
//-----------------------------------------------------------------------------
VOID guild_commerce::extend_commendation()
{
	if (!b_init_ok)			return;

	if (p_guild->get_guild_att().byLevel < 3)	return;
	
	if (vector_rank.empty())	return;

	if (b_commend)
	{
		VECTOR_RANK vector_result(MAX_COMMEND_PLAYER);
		partial_sort_copy(vector_rank.begin(), vector_rank.end(), vector_result.begin(), vector_result.end(), rank_compare());

		for (INT n=1; n <= MAX_COMMEND_PLAYER; n++)
		{
			if (!VALID_POINT(vector_result[n-1]))
			{
				break;
			}

			Role* p_role_	= g_roleMgr.get_role(vector_result[n-1]->dw_role_id);
			switch (n)
			{
			case 1:
				p_guild->change_member_contribution(vector_result[0]->dw_role_id, 20, TRUE);
				if (VALID_POINT(p_role_))
				{
					p_role_->GetCurMgr().IncWareSilver(vector_result[0]->nTael/100, elcid_guild_commerce);
				}
				else
				{
					CurrencyMgr::ModifyWareSilver(vector_result[0]->dw_role_id, vector_result[0]->nTael/100, elcid_guild_commerce);
				}
				break;

			case 2:
				p_guild->change_member_contribution(vector_result[1]->dw_role_id, 10, TRUE);
				if (VALID_POINT(p_role_))
				{
					p_role_->GetCurMgr().IncWareSilver(vector_result[1]->nTael*5/1000, elcid_guild_commerce);
				}
				else
				{
					CurrencyMgr::ModifyWareSilver(vector_result[1]->dw_role_id, vector_result[1]->nTael*5/1000, elcid_guild_commerce);
				}
				break;

			case 3:
				p_guild->change_member_contribution(vector_result[2]->dw_role_id, 5, TRUE);
				if (VALID_POINT(p_role_))
				{
					p_role_->GetCurMgr().IncWareSilver(vector_result[2]->nTael*2/1000, elcid_guild_commerce);
				}
				else
				{
					CurrencyMgr::ModifyWareSilver(vector_result[2]->dw_role_id, vector_result[2]->nTael*2/1000, elcid_guild_commerce);
				}
				break;

			default:
				break;
			}
		}
	}

	remove_commerce_rank(INVALID_VALUE);
}

//-----------------------------------------------------------------------------
//! 载入帮会跑商信息
//-----------------------------------------------------------------------------
DWORD guild_commerce::load_commerce_info( s_guild_commerce_info* p_info_, INT n_info_num_ )
{
	if (n_info_num_ < 0)
	{
		return INVALID_VALUE;
	}

	for (int n=0; n<n_info_num_; n++)
	{
		guild_commodity* p_commodity	= new guild_commodity;
		if (p_commodity->load_commodity_info(&p_info_[n]) == E_Success)
		{
			map_commodity.add(p_info_[n].dw_role_id, p_commodity);
		}
	}

	b_init_ok	= TRUE;
	
	return E_Success;
}

//-----------------------------------------------------------------------------
//! 载入帮会跑商排名信息
//-----------------------------------------------------------------------------
DWORD guild_commerce::load_commerce_rank_info(tagCommerceRank* p_info_, INT n_info_num_)
{
	if (n_info_num_ < 0)
	{
		return INVALID_VALUE;
	}

	for (int n=0; n<n_info_num_; n++)
	{
		add_to_commerce_rank(p_info_[n].dw_role_id, p_info_[n].nTael, p_info_[n].nTimes, FALSE);		
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// ! 获得商货信息
//-----------------------------------------------------------------------------
DWORD guild_commerce::get_commodity_goods_info( Role* p_role_, tagCommerceGoodInfo* p_goods_info_, INT& n_goods_num_, INT32& n_tael_, INT& n_level_ )
{
	if (!VALID_POINT(p_role_) || !VALID_POINT(p_goods_info_))
	{
		return INVALID_VALUE;
	}

	if (!p_role_->IsInRoleState(ERS_Commerce))
	{
		return E_GuildCommerce_Status_Error;
	}

	guild_commodity* p_commodity = get_commodity(p_role_->GetID());
	if (!VALID_POINT(p_commodity))
	{
		return E_GuildCommerce_Status_Error;
	}

	if (!p_commodity->get_commodity_info(p_goods_info_, n_goods_num_, n_tael_, n_level_))
	{
		return INVALID_VALUE;
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 购买商货
//-----------------------------------------------------------------------------
DWORD guild_commerce::buy_goods( Role* p_role_, DWORD dw_npc_id_, DWORD dw_goods_id_, BYTE by_buy_num_ )
{
	if (!VALID_POINT(p_role_))
	{
		return INVALID_VALUE;
	}

	if (!p_role_->IsInRoleState(ERS_Commerce))
	{
		return E_GuildCommerce_Status_Error;
	}

	Map* p_map = p_role_->get_map();
	if (!VALID_POINT(p_map))
	{
		return INVALID_VALUE;
	}

	guild_chamber* p_chamber = p_map->get_chamber(dw_npc_id_);
	if (!VALID_POINT(p_chamber))
	{
		return E_CofC_NotExist;
	}

	if (!p_guild->is_occupy_city(p_chamber->get_occupy_city()))
	{
		return E_CofC_HoldCity_Limit;
	}

	guild_commodity* p_commodity = get_commodity(p_role_->GetID());
	if (!VALID_POINT(p_commodity))
	{
		return E_GuildCommerce_Status_Error;
	}

	DWORD dw_error_code = p_commodity->is_full(dw_goods_id_, by_buy_num_);
	if (E_Success != dw_error_code)
	{
		return dw_error_code;
	}

	INT32 n_cost_tael = 0;
	dw_error_code = p_chamber->sell_goods(p_commodity->get_tael(), dw_goods_id_, by_buy_num_, n_cost_tael);
	if (E_Success != dw_error_code)
	{
		return dw_error_code;
	}

	p_commodity->add_goods(dw_goods_id_, by_buy_num_, p_chamber->get_goods_price(dw_goods_id_));
	p_commodity->decrease_tael(n_cost_tael);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ! 出售商货
//-----------------------------------------------------------------------------
DWORD guild_commerce::sell_goods( Role* p_role_, DWORD dw_npc_id_, DWORD dw_goods_id_, BYTE by_sell_num_ )
{
	if (!VALID_POINT(p_role_) || by_sell_num_ <= 0)
	{
		return INVALID_VALUE;
	}

	if (!p_role_->IsInRoleState(ERS_Commerce))
	{
		return E_GuildCommerce_Status_Error;
	}

	Map* p_map = p_role_->get_map();
	if (!VALID_POINT(p_map))
	{
		return INVALID_VALUE;
	}

	guild_chamber* p_chamber = p_map->get_chamber(dw_npc_id_);
	if (!VALID_POINT(p_chamber))
	{
		return E_CofC_NotExist;
	}

	if (p_chamber->get_occupy_city() != 0)
	{
		return E_CofC_ItemCannotSell;
	}

	guild_commodity* p_commodity = get_commodity(p_role_->GetID());
	if (!VALID_POINT(p_commodity))
	{
		return E_GuildCommerce_Status_Error;
	}

	DWORD dw_error_code = p_commodity->is_exist(dw_goods_id_, by_sell_num_);
	if (E_Success != dw_error_code)
	{
		return dw_error_code;
	}

	INT32 n_cost = p_chamber->get_goods_price(dw_goods_id_);
	if (!VALID_VALUE(n_cost))
	{
		return E_CofC_ItemCannotSell;
	}

	p_commodity->remove_goods(dw_goods_id_, by_sell_num_, p_chamber->get_goods_price(dw_goods_id_));
	p_commodity->increase_tael(n_cost*by_sell_num_);

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 获得跑商初始信息
//-----------------------------------------------------------------------------
DWORD guild_commerce::get_commerce_init_info( DWORD dw_role_id_, INT& n_level_, tagTaelInfo& st_tael_info_ )
{
	guild_commodity* p_commodity = get_commodity(dw_role_id_);
	if (!VALID_POINT(p_commodity))
	{
		return E_GuildCommerce_Status_Error;
	}

	n_level_		= p_commodity->get_level();
	st_tael_info_	= *(p_commodity->get_tael_info());

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 保存跑商排行榜
//-----------------------------------------------------------------------------
VOID guild_commerce::add_to_commerce_rank( DWORD dw_role_id_, INT32 n_tael_, INT n_times_ /*= INVALID_VALUE*/, BOOL b_save_db_/* = TRUE*/)
{
	if (!VALID_VALUE(dw_role_id_) || n_tael_ < 0)
	{
		return;
	}

	tagCommerceRank* p_rank_info = NULL;
	VECTOR_RANK::iterator iter_rank	= vector_rank.begin();
	for (; iter_rank != vector_rank.end(); iter_rank++)
	{
		if (!VALID_POINT(*iter_rank))
		{
			continue;
		}

		if ((*iter_rank)->dw_role_id == dw_role_id_)
		{
			p_rank_info	= *iter_rank;
			break;
		}
	}
	if (iter_rank == vector_rank.end())
	{
		p_rank_info				= new tagCommerceRank;
		p_rank_info->dw_role_id		= dw_role_id_;
		p_rank_info->nTael		= n_tael_;
		if (!VALID_VALUE(n_times_))
		{
			n_times_ = 1;
		}
		p_rank_info->nTimes		= n_times_;
		vector_rank.push_back(p_rank_info);
	}
	else
	{
		(*iter_rank)->nTael += n_tael_;
		(*iter_rank)->nTimes++;
	}

	if (b_save_db_ && VALID_POINT(p_rank_info))
	{
		NET_DB2C_update_commerce_rank send_rank;
		send_rank.dw_guild_id	= p_guild->get_guild_att().dwID;
		send_rank.s_rank_info	= *p_rank_info;
		g_dbSession.Send(&send_rank, send_rank.dw_size);
	}
}

//-----------------------------------------------------------------------------
//！清除跑商排行榜
//-----------------------------------------------------------------------------
VOID guild_commerce::remove_commerce_rank( DWORD dw_role_id_ )
{
	if (VALID_VALUE(dw_role_id_))
	{
		VECTOR_RANK::iterator iter_rank	= vector_rank.begin();
		while (iter_rank != vector_rank.end())
		{
			if (!VALID_POINT(*iter_rank))
			{
				iter_rank++;
				continue;
			}
			if ((*iter_rank)->dw_role_id == dw_role_id_)
			{
				SAFE_DELETE(*iter_rank);
				vector_rank.erase(iter_rank);
				break;
			}
			iter_rank++;
		}
	}
	else
	{
		VECTOR_RANK::iterator iter_rank	= vector_rank.begin();
		while (iter_rank != vector_rank.end())
		{
			if (!VALID_POINT(*iter_rank))
			{
				iter_rank++;
				continue;
			}
			SAFE_DELETE(*iter_rank);
			iter_rank++;
		}
		vector_rank.clear();
	}

	NET_DB2C_remove_commerce_rank send_rank;
	send_rank.dw_guild_id	= p_guild->get_guild_att().dwID;
	send_rank.dw_role_id	= dw_role_id_;
	g_dbSession.Send(&send_rank, send_rank.dw_size);
}