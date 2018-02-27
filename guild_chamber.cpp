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
*	@file		guild_chamber_of_commerce.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		商会
*/

#include "StdAfx.h"

#include "../common/ServerDefine/guild_server_define.h"
#include "../../common/WorldDefine/guild_protocol.h"

#include "guild_chamber.h"
#include "att_res.h"
#include "world.h"
#include "role.h"
#include "role_mgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "guild_commodity.h"


guild_chamber::guild_chamber()
{
	b_init_ok		= FALSE;
	dw_npc_id		= INVALID_VALUE;
	dw_shop_id		= INVALID_VALUE;
	by_occupy_city	= 0;
	dw_refresh_time	= 0;
	map_sell_goods.clear();
	list_observer.clear();
}

guild_chamber::~guild_chamber()
{
	destroy();
}


BOOL guild_chamber::init( DWORD dw_npc_id_, DWORD dw_shop_id_ )
{
	if (!VALID_VALUE(dw_npc_id_) || !VALID_VALUE(dw_shop_id_))
	{
		return FALSE;
	}

	//! 读取商货数据
	if (!AttRes::GetInstance()->LoadCofCGoodInfo(dw_shop_id_, map_sell_goods, map_buy_goods))
	{
		return FALSE;
	}

	// ！是不是特产商会
	const s_cof_csp_proto* p_special_chamber_proto = AttRes::GetInstance()->GetCofCSPProto(dw_shop_id_);
	if (VALID_POINT(p_special_chamber_proto))
	{
		by_occupy_city = p_special_chamber_proto->by_holder_id;
	}
	
	list_observer.clear();
	dw_npc_id		= dw_npc_id_;
	dw_shop_id		= dw_shop_id_;
	dw_refresh_time	= 0;
	b_init_ok		= TRUE;

	return TRUE;
}

VOID guild_chamber::update()
{
	tagDWORDTime dw_time = g_world.GetWorldTime();
	BOOL b_change = FALSE;

	//! 刷新商货价格
	if (dw_time.sec == 0 && (DWORD)dw_time != (DWORD)dw_refresh_time)
	{
		dw_refresh_time = dw_time;

		if (dw_time.min % 5 == 0)
		{
			refresh_buy_goods();
			b_change = TRUE;
		}

		//! 刷新商货数量
		if (dw_time.min % 10 == 0)
		{
			refresh_sell_goods();
			b_change = TRUE;
		}

		if (b_change)
		{
			send_commerce_goods_to_role(NULL);
		}
	}
}

VOID guild_chamber::destroy()
{
	tag_chamber_sell_good* p_sell_goods = NULL;
	MAP_SELL_GOODS::map_iter iter = map_sell_goods.begin();
	while (map_sell_goods.find_next(iter, p_sell_goods))
	{
		SAFE_DELETE(p_sell_goods);
	}
	map_sell_goods.clear();

	dw_npc_id	= INVALID_VALUE;
	b_init_ok	= FALSE;
}

//-----------------------------------------------------------------------------
//! 刷新商货
//-----------------------------------------------------------------------------
VOID guild_chamber::refresh_sell_goods()
{
	tag_chamber_sell_good* p_sell_goods = NULL;
	MAP_SELL_GOODS::map_iter iter = map_sell_goods.begin();
	while (map_sell_goods.find_next(iter, p_sell_goods))
	{
		if (!VALID_POINT(p_sell_goods))
		{
			continue;
		}
		const s_commodity_proto* p_proto = AttRes::GetInstance()->GetCommodityProto(p_sell_goods->dw_goods_id);
		if (!VALID_POINT(p_proto))
		{
			continue;
		}
		p_sell_goods->by_free_num = p_proto->by_refresh_num;
		refresh_goods_price(p_sell_goods->n_cost, p_proto->n_low_price, p_proto->n_high_price);
	}
}

VOID guild_chamber::refresh_buy_goods()
{
	tag_chamber_buy_good* p_buy_goods = NULL;
	MAP_BUY_GOODS::map_iter iter_buy = map_buy_goods.begin();
	while (map_buy_goods.find_next(iter_buy, p_buy_goods))
	{
		if (!VALID_POINT(p_buy_goods))
		{
			continue;
		}
		const s_commodity_proto* p_proto = AttRes::GetInstance()->GetCommodityProto(p_buy_goods->dw_goods_id);
		if (!VALID_POINT(p_proto))
		{
			continue;
		}

		refresh_goods_price(p_buy_goods->n_cost, p_proto->n_low_price, p_proto->n_high_price);
	}
}

//-----------------------------------------------------------------------------
//! 获得商店物品数据
//-----------------------------------------------------------------------------
VOID guild_chamber::get_goods_info( DWORD dw_role_id_, OUT tagCommerceGoodInfo* p_goods_info_, INT& by_num_ )
{
	by_num_ = 0;

	tag_chamber_sell_good* p_sell_goods = NULL;
	MAP_SELL_GOODS::map_iter iter = map_sell_goods.begin();
	while (map_sell_goods.find_next(iter, p_sell_goods))
	{
		p_goods_info_[by_num_++] = *(tagCommerceGoodInfo*)p_sell_goods;
	}

	if (VALID_VALUE(dw_role_id_) && !list_observer.is_exist(dw_role_id_))
	{
		list_observer.push_back(dw_role_id_);
	}
}

//-----------------------------------------------------------------------------
//! 卖出商货
//-----------------------------------------------------------------------------
DWORD guild_chamber::sell_goods( INT32 n_price_, DWORD dw_goods_id_, BYTE by_buy_num_, OUT INT32 &n_cost_price_ )
{
	if (!VALID_VALUE(dw_goods_id_) || by_buy_num_<=0)
	{
		return INVALID_VALUE;
	}

	//! 找到商货
	tag_chamber_sell_good* pGoodInfo = map_sell_goods.find(dw_goods_id_);
	if (!VALID_POINT(pGoodInfo))
	{
		return E_CofC_ItemNotFind;
	}

	//！ 检查是否有足够商货
	if (by_buy_num_ > pGoodInfo->by_free_num)
	{
		return E_CofC_ItemNotEnough;
	}

	n_cost_price_ = by_buy_num_ * pGoodInfo->n_cost;
	if (n_price_ < by_buy_num_ * pGoodInfo->n_cost)
	{
		return E_GuildCommodity_NotEnough_Tael;
	}

	pGoodInfo->by_free_num	-= by_buy_num_;

	send_commerce_goods_to_role(NULL, pGoodInfo->dw_goods_id);

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 获取商货收购价
//-----------------------------------------------------------------------------
INT32 guild_chamber::get_goods_price( DWORD dw_goods_id_ )
{
	const s_commodity_proto* p_proto = AttRes::GetInstance()->GetCommodityProto(dw_goods_id_);
	if (!VALID_POINT(p_proto))
	{
		return 0;
	}

	const FLOAT f_profit = AttRes::GetInstance()->GetCofCProfit(dw_shop_id, p_proto->by_holder_id);
	if (!VALID_VALUE((INT32)f_profit))
	{
		return 0;
	}

	tag_chamber_buy_good* p_buy_goods = map_buy_goods.find(dw_goods_id_);
	if (!VALID_POINT(p_buy_goods))
	{
		return 0;
	}

	return (INT32)(p_buy_goods->n_cost * f_profit);
}

//-----------------------------------------------------------------------------
//! 创建商会
//-----------------------------------------------------------------------------
guild_chamber* guild_chamber::create_chamber( DWORD dw_npc_id_, DWORD dw_shop_id_ )
{
	guild_chamber* p_chamber = new guild_chamber;
	if (!VALID_POINT(p_chamber))
	{
		return NULL;
	}

	if (!p_chamber->init(dw_npc_id_, dw_shop_id_))
	{
		ASSERT(0);
		SAFE_DELETE(p_chamber);
		return NULL;
	}
	
	return p_chamber;
}

//-----------------------------------------------------------------------------
//! 刷新商货价格
//-----------------------------------------------------------------------------
VOID guild_chamber::refresh_goods_price( INT32& n_current_price_, INT32 n_low_price_, INT32 n_height_price_ )
{
	ASSERT(n_height_price_ >= n_low_price_);
	ASSERT(n_current_price_ >= n_low_price_);
	ASSERT(n_current_price_ <= n_height_price_);

	INT32 n_old	= (INT32)((FLOAT)(n_height_price_ - n_current_price_) / (FLOAT)(n_height_price_ - n_low_price_) * 10000.0f);

	INT32 n_rand_pct = get_tool()->tool_rand() % 10000;
	if (n_rand_pct < n_old)
	{
		n_current_price_ = get_tool()->rand_in_range((n_height_price_+n_low_price_)/2, n_height_price_);
	}
	else
	{
		n_current_price_ = get_tool()->rand_in_range(n_low_price_, (n_height_price_+n_low_price_)/2);
	}
}

//-----------------------------------------------------------------------------
//! 发送商会信息
//-----------------------------------------------------------------------------
DWORD guild_chamber::send_commerce_goods_to_role( Role* p_role_, DWORD dw_goods_id_ /*= INVALID_VALUE*/ )
{
	if (!VALID_POINT(p_role_) && list_observer.empty())
	{
		return E_Success;
	}

	INT n_message_size = sizeof(NET_SIS_get_chamber_goods_info) + sizeof(tagCommerceGoodInfo) * (MAX_COFC_GOODS_NUM - 1);
	CREATE_MSG(p_chamber_info, n_message_size, NET_SIS_get_chamber_goods_info);

	p_chamber_info->dwCofCID		= dw_shop_id;
	p_chamber_info->byHoldCity	= by_occupy_city;
	if (!VALID_VALUE(dw_goods_id_))
	{
		if (VALID_POINT(p_role_))
		{
			get_goods_info(p_role_->GetID(), p_chamber_info->sGoodInfo, p_chamber_info->nGoodNum);
		}
		else
		{
			get_goods_info(INVALID_VALUE, p_chamber_info->sGoodInfo, p_chamber_info->nGoodNum);
		}
	}
	else
	{
		tagCommerceGoodInfo* p_sell_goods = (tagCommerceGoodInfo*)(map_sell_goods.find(dw_goods_id_));
		if (VALID_POINT(p_sell_goods))
		{
			p_chamber_info->nGoodNum		= 1;
			p_chamber_info->sGoodInfo[0]	= *p_sell_goods;
		}
	}
	
	p_chamber_info->dw_size = sizeof(NET_SIS_get_chamber_goods_info) + sizeof(tagCommerceGoodInfo) * (p_chamber_info->nGoodNum - 1);

	if (VALID_POINT(p_role_))
	{
		p_role_->SendMessage(p_chamber_info, p_chamber_info->dw_size);
	}
	else
	{
		send_message_to_observer(p_chamber_info, p_chamber_info->dw_size);
	}

	//! 更新观察者商货信息
	if (!VALID_VALUE(dw_goods_id_))
	{
		n_message_size = sizeof(NET_SIS_get_commodity_info) + sizeof(tagCommerceGoodInfo) * (MAX_COMMODITY_CAPACITY - 1);
		CREATE_MSG(pCommodityInfo, n_message_size, NET_SIS_get_commodity_info);

		if (VALID_POINT(p_role_))
		{
			guild* p_guild = g_guild_manager.get_guild(p_role_->GetGuildID());
			if (!VALID_POINT(p_guild))
			{
				return INVALID_VALUE;
			}

			guild_commodity* p_commodity = p_guild->get_guild_commerce().get_commodity(p_role_->GetID());
			if (!VALID_POINT(p_commodity))
			{
				return INVALID_VALUE;
			}

			if (p_commodity->get_commodity_info(pCommodityInfo->sGoodInfo, pCommodityInfo->nGoodNum, pCommodityInfo->nCurTael, pCommodityInfo->nLevel) == E_Success)
			{
				//！ 更新商货收购价格
				for (int n=0; n<pCommodityInfo->nGoodNum; n++)
				{
					pCommodityInfo->sGoodInfo[n].nCost = get_goods_price(pCommodityInfo->sGoodInfo[n].dwGoodID);
				}

				pCommodityInfo->dw_size = sizeof(NET_SIS_get_commodity_info) + sizeof(tagCommerceGoodInfo) * (pCommodityInfo->nGoodNum - 1);

				p_role_->SendMessage(pCommodityInfo, pCommodityInfo->dw_size);
			}
		} 
		else
		{
			DWORD dw_observer_id = INVALID_VALUE;
			LIST_OBSERVER::list_iter iter = list_observer.begin();
			while (list_observer.find_next(iter, dw_observer_id))
			{
				Role* pObserver = g_roleMgr.get_role(dw_observer_id);
				if (!VALID_POINT(pObserver))
				{
					list_observer.erase(dw_observer_id);
					continue;
				}

				guild* p_guild = g_guild_manager.get_guild(pObserver->GetGuildID());
				if (!VALID_POINT(p_guild))
				{
					list_observer.erase(dw_observer_id);
					continue;
				}

				guild_commodity* p_commodity = p_guild->get_guild_commerce().get_commodity(pObserver->GetID());
				if (!VALID_POINT(p_commodity))
				{
					list_observer.erase(dw_observer_id);
					continue;
				}

				if (p_commodity->get_commodity_info(pCommodityInfo->sGoodInfo, pCommodityInfo->nGoodNum, pCommodityInfo->nCurTael, pCommodityInfo->nLevel) == E_Success)
				{
					for (int n=0; n<pCommodityInfo->nGoodNum; n++)
					{
						pCommodityInfo->sGoodInfo[n].nCost = get_goods_price(pCommodityInfo->sGoodInfo[n].dwGoodID);
					}

					pCommodityInfo->dw_size = sizeof(NET_SIS_get_commodity_info) + sizeof(tagCommerceGoodInfo) * (pCommodityInfo->nGoodNum - 1);

					pObserver->SendMessage(pCommodityInfo, pCommodityInfo->dw_size);
				}
			}
		}
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
//! 向观察者群发消息
//-----------------------------------------------------------------------------
VOID guild_chamber::send_message_to_observer( LPVOID p_message_, DWORD dw_size_ )
{
	DWORD dw_observer_id = INVALID_VALUE;
	LIST_OBSERVER::list_iter iter = list_observer.begin();
	while (list_observer.find_next(iter, dw_observer_id))
	{
		Role* p_observer = g_roleMgr.get_role(dw_observer_id);
		if (!VALID_POINT(p_observer))
		{
			list_observer.erase(dw_observer_id);
			continue;
		}

		p_observer->SendMessage(p_message_, dw_size_);
	}
}