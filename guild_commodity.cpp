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
*	@file		guild_commodity.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		跑商任务
*/


#include "StdAfx.h"

#include "../common/ServerDefine/guild_server_define.h"
#include "../../common/WorldDefine/guild_protocol.h"

#include "att_res.h"
#include "db_session.h"
#include "guild_commodity.h"
#include "role_mgr.h"
#include "role.h"

guild_commodity::guild_commodity()
{
	n_current_tael	= 0;
	map_goods.clear();
}

guild_commodity::~guild_commodity()
{
	destroy();
}

BOOL guild_commodity::init( DWORD dw_role_id_, INT n_level_, const tagTaelInfo* p_tael_info_ /*= NULL*/, const s_redound_info* p_redound_info_ /*= NULL*/ )
{
	if (!VALID_VALUE(dw_role_id_))
	{
		return FALSE;
	}

	dw_owner_id	= dw_role_id_;
	n_level	= n_level_;
	map_goods.clear();

	if (VALID_POINT(p_tael_info_) && VALID_POINT(p_redound_info_))
	{
		st_redound_info	= *p_redound_info_;
		st_tael_info		= *p_tael_info_;
		n_current_tael		= p_tael_info_->nBeginningTael;
	}

	return TRUE;
}

VOID guild_commodity::destroy()
{
	tagCommerceGoodInfo* p_info	= NULL;
	MAP_GOODS::map_iter iter	= map_goods.begin();
	while (map_goods.find_next(iter, p_info))
	{
		SAFE_DELETE(p_info);
	}
	map_goods.clear();
}

//------------------------------------------------------------------------------
//! 载入跑商信息
//------------------------------------------------------------------------------
DWORD guild_commodity::load_commodity_info( s_guild_commerce_info* p_load_info_ )
{
	if (!VALID_POINT(p_load_info_))
	{
		return INVALID_VALUE;
	}

	dw_owner_id		= p_load_info_->dw_role_id;

	const s_commerce_info* p_info = AttRes::GetInstance()->GetGuildCommerceInfo(p_load_info_->n_level);
	if (!VALID_POINT(p_info))
	{
		return INVALID_VALUE;
	}
	n_level		= p_load_info_->n_level;
	st_tael_info		= p_info->tagTaelInfo;
	st_redound_info	= p_info->s_redound_info_;

	n_current_tael		= p_load_info_->n_tael;

	map_goods.clear();
	for (int n=0; n<MAX_COMMODITY_CAPACITY; n++)
	{
		if (!p_load_info_->s_good_info[n].IsValid())
		{
			break;
		}
		tagCommerceGoodInfo* p_goods_info = new tagCommerceGoodInfo(p_load_info_->s_good_info[n]);
		map_goods.add(p_load_info_->s_good_info[n].dwGoodID, p_goods_info);
	}

	return E_Success;
}

BOOL guild_commodity::increase_tael( INT32 n_tael_ )
{
	if (n_tael_ <= 0 || n_current_tael > st_tael_info.nMaxTael)
	{
		return FALSE;
	}

	n_current_tael += n_tael_;
	if (n_current_tael > st_tael_info.nMaxTael || n_current_tael <= 0)	
	{
		n_current_tael = st_tael_info.nMaxTael;
	}

	
	save_tael_to_db();

	Role* p_role = g_roleMgr.get_role(dw_owner_id);
	if (VALID_POINT(p_role))
	{
		NET_SIS_get_commodity_info	send;
		send.nCurTael		= n_current_tael;
		send.nLevel			= n_level;
		send.nGoodNum		= 0;
		p_role->SendMessage(&send, send.dw_size);
	}

	return TRUE;
}

BOOL guild_commodity::decrease_tael( INT32 n_tael_ )
{
	if (n_tael_ <= 0 || n_current_tael <= 0)
	{
		return FALSE;
	}

	n_current_tael -= n_tael_;
	if (n_current_tael < 0)
	{
		n_current_tael = 0;
	}

	save_tael_to_db();

	Role* p_role = g_roleMgr.get_role(dw_owner_id);
	if (VALID_POINT(p_role))
	{
		NET_SIS_get_commodity_info	send;
		send.nCurTael		= n_current_tael;
		send.nLevel			= n_level;
		send.nGoodNum		= 0;
		p_role->SendMessage(&send, send.dw_size);
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//! 添加商货
//------------------------------------------------------------------------------
BOOL guild_commodity::add_goods( DWORD dw_goods_id_, BYTE by_num_, INT32 n_price_ /*= 0*/ )
{
	if (!VALID_VALUE(dw_goods_id_))
	{
		return FALSE;
	}

	tagCommerceGoodInfo* p_goods_info = NULL;
	if (map_goods.is_exist(dw_goods_id_))
	{
		p_goods_info = map_goods.find(dw_goods_id_);
		if (!VALID_POINT(p_goods_info))
		{
			return FALSE;
		}
		INT16 n16Num = p_goods_info->byGoodNum + by_num_;
		if (n16Num > MAX_COMMODITY_GOOD_NUM)
		{
			n16Num = MAX_COMMODITY_GOOD_NUM;
		}
		p_goods_info->byGoodNum = (BYTE)n16Num;
	}
	else if (map_goods.size() >= MAX_COMMODITY_CAPACITY)
	{
		return FALSE;
	}
	else
	{
		// by_num_不可能超过255
		p_goods_info = new tagCommerceGoodInfo;
		p_goods_info->dwGoodID		= dw_goods_id_;
		p_goods_info->byGoodNum	= by_num_;
		p_goods_info->nCost		= n_price_;
		map_goods.add(dw_goods_id_, p_goods_info);
	}

	save_commodity_to_db();

	Role* p_role = g_roleMgr.get_role(dw_owner_id);
	if (VALID_POINT(p_role))
	{
		NET_SIS_get_commodity_info	send;
		send.nCurTael		= n_current_tael;
		send.nLevel			= n_level;
		send.nGoodNum		= 1;
		send.sGoodInfo[0]	= *p_goods_info;
		p_role->SendMessage(&send, send.dw_size);
	}

	return TRUE;
}

BOOL guild_commodity::remove_goods( DWORD dw_goods_id_, BYTE by_num_, INT32 n_price_ /*= 0*/ )
{
	if (!VALID_VALUE(dw_goods_id_) || !map_goods.is_exist(dw_goods_id_))
	{
		return FALSE;
	}

	tagCommerceGoodInfo* p_goods_info = map_goods.find(dw_goods_id_);
	if (!VALID_POINT(p_goods_info))
	{
		return FALSE;
	}

	BYTE by_remain_num = p_goods_info->byGoodNum;
	if (by_remain_num < by_num_)
	{
		return FALSE;
	}
	else
	{
		p_goods_info->byGoodNum	= by_remain_num - by_num_;
		p_goods_info->nCost		= n_price_;
	}

	Role* p_role = g_roleMgr.get_role(dw_owner_id);
	if (VALID_POINT(p_role))
	{
		NET_SIS_get_commodity_info	send;
		send.nCurTael		= n_current_tael;
		send.nLevel			= n_level;
		send.nGoodNum		= 1;
		send.sGoodInfo[0]	= *p_goods_info;
		p_role->SendMessage(&send, send.dw_size);
	}

	if (p_goods_info->byGoodNum == 0)
	{
		map_goods.erase(dw_goods_id_);
		SAFE_DELETE(p_goods_info);
	}

	save_commodity_to_db();

	return TRUE;
}

//------------------------------------------------------------------------------
//! 获取商货信息
//------------------------------------------------------------------------------
DWORD guild_commodity::get_commodity_info( tagCommerceGoodInfo* p_goods_, INT& n_goods_num_, INT32& n_tael_, INT32& n_level_ )
{
	if (!VALID_POINT(p_goods_))
	{
		return INVALID_VALUE;
	}

	n_tael_	= n_current_tael;
	n_level_	= n_level;

	n_goods_num_		= 0;

	DWORD dw_goods_id	= INVALID_VALUE;
	tagCommerceGoodInfo* p_goods_info = NULL;

	MAP_GOODS::map_iter iter = map_goods.begin();
	while (map_goods.find_next(iter, dw_goods_id, p_goods_info))
	{
		if (!VALID_POINT(p_goods_info))
		{
			map_goods.erase(dw_goods_id);
			continue;
		}

		if (!p_goods_info->IsValid() || n_goods_num_ >= MAX_COMMODITY_CAPACITY)
		{
			map_goods.erase(dw_goods_id);
			continue;
		}

		p_goods_[n_goods_num_++]	= *p_goods_info;
	}

	return E_Success;
}

VOID guild_commodity::save_tael_to_db()
{
	NET_DB2C_change_tael send;
	send.dw_role_id	= dw_owner_id;
	send.n_tael		= n_current_tael;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_commodity::save_commodity_to_db()
{
	NET_DB2C_save_commodity send;
	send.dw_role_id	= dw_owner_id;

	DWORD dw_goods_id					= INVALID_VALUE;
	tagCommerceGoodInfo* p_goods_info	= NULL;
	INT n_goods_type					= 0;

	MAP_GOODS::map_iter iter = map_goods.begin();
	while (map_goods.find_next(iter, dw_goods_id, p_goods_info))
	{
		if (!VALID_POINT(p_goods_info))
		{
			map_goods.erase(dw_goods_id);
			continue;
		}
		if (!p_goods_info->IsValid() || n_goods_type >= MAX_COMMODITY_CAPACITY)
		{
			map_goods.erase(dw_goods_id);
			continue;
		}
		send.s_good_info[n_goods_type++] = *p_goods_info;
	}
	
	for (int n=n_goods_type; n<MAX_COMMODITY_CAPACITY; n++)
	{
		send.s_good_info[n].dwGoodID	= INVALID_VALUE;
		send.s_good_info[n].byGoodNum	= 0;
		send.s_good_info[n].nCost		= 0;
	}

	g_dbSession.Send(&send, send.dw_size);
}

//------------------------------------------------------------------------------
//! 商货是否已满
//------------------------------------------------------------------------------
DWORD guild_commodity::is_full( DWORD dw_goods_id_, BYTE by_num_ )
{
	tagCommerceGoodInfo* p_goods_info = map_goods.find(dw_goods_id_);

	INT n_result_num = by_num_;
	if (VALID_POINT(p_goods_info))
	{
		n_result_num += p_goods_info->byGoodNum;
		if (n_result_num > MAX_COMMODITY_GOOD_NUM)
		{
			return E_GuildCommodity_ItemMaxHold;
		}
	}
	else if (get_goods_num() >= MAX_COMMODITY_CAPACITY)
	{
		return E_GuildCommodity_NotEnough_Space;
	}
	else if (n_result_num > MAX_COMMODITY_GOOD_NUM)
	{
		return E_GuildCommodity_NotEnough_Space;
	}
	
	return E_Success;
}

//------------------------------------------------------------------------------
//! 商货是否足够
//------------------------------------------------------------------------------
DWORD guild_commodity::is_exist( DWORD dw_goods_id_, BYTE by_num_ )
{
	tagCommerceGoodInfo* p_goods_info = map_goods.find(dw_goods_id_);
	if (!VALID_POINT(p_goods_info))
	{
		return E_CofC_ItemNotFind;
	}
	else if (p_goods_info->byGoodNum < by_num_)
	{
		return E_CofC_ItemNotEnough;
	}
	
	return E_Success;
}

//------------------------------------------------------------------------------
//! 利润
//------------------------------------------------------------------------------
INT32 guild_commodity::get_gain()
{
	if (n_current_tael > st_tael_info.nBeginningTael)
	{
		return n_current_tael - st_tael_info.nBeginningTael;
	}

	return 0;
}

//------------------------------------------------------------------------------
//! 死亡惩罚
//------------------------------------------------------------------------------
VOID guild_commodity::dead_penalty()
{
	decrease_tael((INT32)(n_current_tael * 0.2f + 0.5f));

	MAP_GOODS::map_iter iter = map_goods.begin();
	INT32 n_goods_num = 0;
	tagCommerceGoodInfo* p_goods_info = NULL;
	while (map_goods.find_next(iter, p_goods_info))
	{
		if (!VALID_POINT(p_goods_info))
			continue;

		n_goods_num += p_goods_info->byGoodNum;
	}

	n_goods_num = (INT32)(n_goods_num * 0.2f);

	//! 随机扣除商货
	DWORD dw_goods_id = INVALID_VALUE;
	while (n_goods_num > 0)
	{
		map_goods.rand_find(dw_goods_id, p_goods_info);
		if (!VALID_VALUE(dw_goods_id) || !VALID_POINT(p_goods_info))
		{
			continue;
		}

		//! 随机扣除商货数量
		BYTE by_num = get_tool()->rand_in_range(1, min(p_goods_info->byGoodNum, n_goods_num));

		// 扣除商货
		remove_goods(p_goods_info->dwGoodID, by_num);

		n_goods_num -= by_num;
	}
}