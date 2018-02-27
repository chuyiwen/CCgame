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
*	@file		guild_buy.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		帮会团购
*/



#include "StdAfx.h"

#include "../../common/WorldDefine/mall_define.h"
#include "../../common/WorldDefine/mall_protocol.h"
#include "../../common/WorldDefine/ItemDefine.h"

#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/mall_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "guild_buy.h"
#include "guild.h"
#include "att_res.h"
#include "item_creator.h"
#include "mall.h"
#include "role_mgr.h"
#include "currency.h"
#include "role.h"
#include "guild_manager.h"

guild_buy::guild_buy()
{
	p_guild = NULL;
	map_guild_buy_info.clear();
}

guild_buy::~guild_buy()
{
	destroy();
}

BOOL guild_buy::init( DWORD dw_guild_id_ )
{
	map_guild_buy_info.clear();

	ASSERT(VALID_VALUE(dw_guild_id_));
	p_guild = g_guild_manager.get_guild(dw_guild_id_);
	if (!VALID_POINT(p_guild))
	{
		return FALSE;
	}

	return TRUE;
}

VOID guild_buy::update(DWORD dw_time_)
{
	INT64 n_map_key = INVALID_VALUE;
	tagGroupPurchase* p_guild_buy_info = NULL;

	MAP_GUILD_BUY_INFO::map_iter iter = map_guild_buy_info.begin();
	while (map_guild_buy_info.find_next(iter, n_map_key, p_guild_buy_info))
	{
		//! 帮会团购超时
		p_guild_buy_info->dwRemainTime -= dw_time_;
		if ((INT32)(p_guild_buy_info->dwRemainTime) <= 0)
		{
			remove_guild_buy_info(p_guild_buy_info, FALSE);
		}
	}

}

VOID guild_buy::destroy()
{
	tagGroupPurchase* p_guild_buy_info = NULL;
	MAP_GUILD_BUY_INFO::map_iter iter = map_guild_buy_info.begin();
	while (map_guild_buy_info.find_next(iter, p_guild_buy_info))
	{
		SAFE_DELETE(p_guild_buy_info);
	}
	map_guild_buy_info.clear();
}

BOOL guild_buy::add( tagGroupPurchase* p_guild_buy_info_, BOOL b_notify_db_ /*= TRUE*/ )
{
	ASSERT(p_guild_buy_info_);
	if (!VALID_POINT(p_guild_buy_info_))
	{
		return FALSE;
	}

	if (p_guild_buy_info_->dwGuildID != p_guild->get_guild_att().dwID)
	{
		return FALSE;
	}

	INT64 n_key = get_key(p_guild_buy_info_->dw_role_id, p_guild_buy_info_->dwMallID);
	if (map_guild_buy_info.is_exist(n_key))
	{
		return FALSE;
	}

	map_guild_buy_info.add(n_key, p_guild_buy_info_);

	if (b_notify_db_)
	{
		add_guild_buy_info_to_db(p_guild_buy_info_);
	}

	return TRUE;
}

BOOL guild_buy::remove( tagGroupPurchase* p_guild_buy_info_, BOOL b_notify_db_ /*= TRUE*/ )
{
	ASSERT(p_guild_buy_info_);
	if (!VALID_POINT(p_guild_buy_info_))
	{
		return FALSE;
	}

	if (p_guild_buy_info_->dwGuildID != p_guild->get_guild_att().dwID)
	{
		return FALSE;
	}

	BOOL b_result = map_guild_buy_info.erase(get_key(p_guild_buy_info_->dw_role_id, p_guild_buy_info_->dwMallID));

	if (b_result && b_notify_db_)
	{
		remove_guild_buy_info_to_db(p_guild_buy_info_);
	}

	return b_result;
}

DWORD guild_buy::get_all_guild_buy_info( INT &n_guild_buy_info_num_, tagSimGPInfo* p_data_ )
{
	if (map_guild_buy_info.empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	tagGroupPurchase* p_guild_buy_info = NULL;
	n_guild_buy_info_num_ = 0;

	MAP_GUILD_BUY_INFO::map_iter iter = map_guild_buy_info.begin();
	while (map_guild_buy_info.find_next(iter, p_guild_buy_info))
	{
		if (!VALID_POINT(p_guild_buy_info))
		{
			ASSERT(p_guild_buy_info);
			continue;
		}
		p_data_[n_guild_buy_info_num_].dwGuildID			= p_guild_buy_info->dwGuildID;
		p_data_[n_guild_buy_info_num_].dw_role_id			= p_guild_buy_info->dw_role_id;
		p_data_[n_guild_buy_info_num_].dwMallID			= p_guild_buy_info->dwMallID;
		p_data_[n_guild_buy_info_num_].nPrice			= p_guild_buy_info->nPrice;
		p_data_[n_guild_buy_info_num_].nParticipatorNum	= p_guild_buy_info->nParticipatorNum;
		p_data_[n_guild_buy_info_num_].nRequireNum		= p_guild_buy_info->nRequireNum;
		p_data_[n_guild_buy_info_num_].dwRemainTime		= p_guild_buy_info->dwRemainTime;

		n_guild_buy_info_num_++;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//! 获取指定帮会团购响应者列表
//-------------------------------------------------------------------------------------------------------
DWORD guild_buy::get_response( DWORD dw_id_, DWORD dw_role_id_, DWORD *p_data_ )
{
	if (map_guild_buy_info.empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	tagGroupPurchase* p_guild_buy_info = NULL;
	INT64 n_map_key = get_key(dw_role_id_, dw_id_);

	p_guild_buy_info = map_guild_buy_info.find(n_map_key);

	if (!VALID_POINT(p_guild_buy_info) || !VALID_POINT(p_guild_buy_info->listParticipators))
	{
		return E_GroupPurchase_NoInfo;
	}

	if (p_guild_buy_info->listParticipators->empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	INT i = 0;
	p_guild_buy_info->listParticipators->reset_iterator();
	while (p_guild_buy_info->listParticipators->find_next(p_data_[i++]));

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//！ 获取帮会团购信息数量
//-------------------------------------------------------------------------------------------------------
INT guild_buy::get_guild_buy_info_num()
{
	return map_guild_buy_info.size();
}

//-------------------------------------------------------------------------------------------------------
//! 获取帮会团购响应者数量
//-------------------------------------------------------------------------------------------------------
INT guild_buy::get_response_num( DWORD dw_id_, DWORD dw_role_id_ )
{
	if (map_guild_buy_info.empty())
	{
		return 0;
	}

	tagGroupPurchase* p_guild_buy_info = NULL;
	INT64 n_map_key = get_key(dw_role_id_, dw_id_);

	p_guild_buy_info = map_guild_buy_info.find(n_map_key);

	if (!VALID_POINT(p_guild_buy_info) || !VALID_POINT(p_guild_buy_info->listParticipators))
	{
		return 0;
	}

	return p_guild_buy_info->listParticipators->size();
}

//-------------------------------------------------------------------------------------------------------
//! 发起帮会团购
//-------------------------------------------------------------------------------------------------------
DWORD guild_buy::lauch_guild_buy(Role *p_role_, DWORD dw_id_, BYTE by_scope_,
		BYTE by_index_, INT n_unit_price_, OUT tagGroupPurchase* &p_guild_buy_info_, OUT DWORD& dw_item_type_id_)
{
	ASSERT(VALID_POINT(p_role_));
	ASSERT(g_mall.is_init_ok());
	ASSERT(n_unit_price_ > 0);

	DWORD dw_error_code = E_Success;

	const tagMallItemProto *p_proto = g_mall.GetMallItem(by_index_, EMIT_Item)->pMallItem;
	if (!VALID_POINT(p_proto))
	{
		return INVALID_VALUE;
	}

	//! 检查商城物品是否可以团购
	switch(by_scope_)
	{
	case EGPS_SMALL:
		/*if (p_proto->bySmallGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}*/
		break;

	case EGPS_MEDIUM:
		/*if (p_proto->byMediumGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}*/
		break;

	case EGPS_BIG:
		/*if (p_proto->byBigGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}*/
		break;

	default:
		return INVALID_VALUE;
		break;
	}

	INT32 n_guild_buy_exponent = p_guild->get_guild_att().nGroupPurchase;

	if(p_proto->dwID != dw_id_)
	{
		return E_Mall_ID_Error;
	}

	//！检查物品是否已经发起了团购
	INT64 n_map_key = get_key(p_role_->GetID(), dw_id_);
	if (map_guild_buy_info.is_exist(n_map_key))
		return E_GroupPurchase_AlreadyInitiate;

	INT n_price = 0;

	if(p_proto->dwTimeSaleEnd != INVALID_VALUE && g_world.GetWorldTime() < p_proto->dwTimeSaleEnd)
	{
		n_price = p_proto->nSalePrice;
	}
	else
	{
		n_price = p_proto->nPrice;
	}

	if(n_price != n_unit_price_)
	{
		return E_Mall_YuanBao_Error;
	}

	//! 物品团购价格
	n_price *= p_proto->byGroupPurchaseAmount;

	switch(by_scope_)
	{
	case EGPS_SMALL:
		//n_price = (INT)(n_price * (p_proto->bySmallGroupDiscount/100.0f - 0.2f*n_guild_buy_exponent/100000));
		break;

	case EGPS_MEDIUM:
		//n_price = (INT)(n_price * (p_proto->byMediumGroupDiscount/100.0f - 0.2f*n_guild_buy_exponent/100000));
		break;

	case EGPS_BIG:
		//n_price = (INT)(n_price * (p_proto->byBigGroupDiscount/100.0f - 0.2f*n_guild_buy_exponent/100000));
		break;
	}

	//！检查是否有足够的元宝
	if(n_price > p_role_->GetCurMgr().GetBaiBaoYuanBao() || n_price <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	p_guild_buy_info_ = new tagGroupPurchase;
	if (!VALID_POINT(p_guild_buy_info_))
	{
		ASSERT(p_guild_buy_info_);
		return INVALID_VALUE;
	}

	p_guild_buy_info_->dwGuildID		= p_role_->GetGuildID();
	p_guild_buy_info_->dw_role_id		= p_role_->GetID();
	p_guild_buy_info_->dwMallID		= dw_id_;
	p_guild_buy_info_->nPrice			= n_price;
	p_guild_buy_info_->dwRemainTime	= p_proto->dwPersistTime * 60 * 60; 

	switch (by_scope_)
	{
	case EGPS_SMALL:
		//p_guild_buy_info_->nRequireNum = p_proto->bySmallGroupHeadcount;
		break;

	case EGPS_MEDIUM:
		//p_guild_buy_info_->nRequireNum = p_proto->byMediumGroupHeadcount;
		break;

	case EGPS_BIG:
		//p_guild_buy_info_->nRequireNum = p_proto->byBigGroupHeadcount;
		break;
	}
	p_guild_buy_info_->nParticipatorNum = 1;
	p_guild_buy_info_->listParticipators = new package_list<DWORD>;
	if (!VALID_POINT(p_guild_buy_info_->listParticipators))
	{
		ASSERT(p_guild_buy_info_->listParticipators);
		SAFE_DELETE(p_guild_buy_info_);
		return INVALID_VALUE;
	}
	p_guild_buy_info_->listParticipators->push_back(p_guild_buy_info_->dw_role_id);

	//！ 帮会团购信息加入管理器
	add(p_guild_buy_info_);

	dw_item_type_id_ = p_proto->dw_data_id;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//! 响应帮会团购
//-------------------------------------------------------------------------------------------------------
DWORD guild_buy::respond_guild_buy( Role *p_role_, DWORD dw_id_, DWORD dw_role_id_,
									   INT n_price_, OUT tagGroupPurchase* &p_guild_buy_info_ )
{
	if (!VALID_VALUE(p_role_->GetGuildID()))
		return E_GroupPurchase_NotInGuild;

	if (!VALID_POINT(p_guild->get_member(p_role_->GetID())))
		return E_GroupPurchase_NotMember;

	//! 是不是团购发起人
	if (dw_role_id_ == p_role_->GetID())
	{
		return E_GroupPurchase_IsInitiate;
	}

	//！检查帮会团购是否已经结束
	INT64 n_map_key = get_key(dw_role_id_, dw_id_);
	p_guild_buy_info_ = map_guild_buy_info.find(n_map_key);

	if (!VALID_POINT(p_guild_buy_info_) || (INT32)(p_guild_buy_info_->dwRemainTime) <= 0)
		return E_GroupPurchase_AlreadyEnd;

	if (!VALID_POINT(p_guild_buy_info_->listParticipators))
		return E_GroupPurchase_AlreadyEnd;

	//！检查是否响应过此帮会团购
	DWORD dw_temp_role_id = p_role_->GetID();
	if (p_guild_buy_info_->listParticipators->is_exist(dw_temp_role_id))
	{
		return E_GroupPurchase_AlreadyInitiate;
	}

	if(n_price_ != p_guild_buy_info_->nPrice)
	{
		return E_Mall_YuanBao_Error;
	}

	if(n_price_ > p_role_->GetCurMgr().GetBaiBaoYuanBao() || n_price_ <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//！ 创建帮会团购物品
//-------------------------------------------------------------------------------------------------------
DWORD guild_buy::create_guild_buy_items( DWORD dw_id_, OUT tagMallItemSell &st_item_sell_ )
{
	ASSERT(VALID_VALUE(dw_id_));

	const tagMallItemProto *p_proto = AttRes::GetInstance()->GetMallItemProto(dw_id_);

	if (!VALID_POINT(p_proto))
	{
		return E_Mall_ID_Error;
	}

	tagItem *p_new_item = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, p_proto->dw_data_id, p_proto->byGroupPurchaseAmount, EIQ_Quality0);
	if(!VALID_POINT(p_new_item))
	{
		ASSERT(VALID_POINT(p_new_item));
		return E_Mall_CreateItem_Failed;
	}

	//! 查看是否有赠品
	tagItem *p_present_item = NULL;
	if(p_proto->dwPresentID != INVALID_VALUE)
	{
		p_present_item = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
			p_proto->dwPresentID, (INT16)p_proto->byPresentNum * p_proto->byGroupPurchaseAmount, EIQ_Quality0);
		if(!VALID_POINT(p_present_item))
		{
			::Destroy(p_new_item);
			ASSERT(VALID_POINT(p_present_item));
			return E_Mall_CreatePres_Failed;
		}
	}

	if (p_proto->byExAssign >= 0)
	{
		st_item_sell_.nExVolumeAssign = p_proto->byExAssign;
	}

	st_item_sell_.pItem			= p_new_item;
	st_item_sell_.pPresent		= p_present_item;
	st_item_sell_.nYuanBaoNeed	= 0;				
	st_item_sell_.byRemainNum	= INVALID_VALUE;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//！ 删除帮会团购数据
//-------------------------------------------------------------------------------------------------------
VOID guild_buy::remove_guild_buy_info( tagGroupPurchase* &p_guild_buy_info_, BOOL b_success_ /*= TRUE*/ )
{
	ASSERT(p_guild_buy_info_);

	if (!VALID_POINT(p_guild_buy_info_))
		return;

	// 删除管理器中的信息
	INT64 n_map_key = get_key(p_guild_buy_info_->dw_role_id, p_guild_buy_info_->dwMallID);
	map_guild_buy_info.erase(n_map_key);

	//! 帮会团购失败
	if (!b_success_)
	{
		return_cost_to_role(p_guild_buy_info_);

		p_guild->modify_group_exponent(FALSE);

		//! 广播帮会团购失败
		const tagMallItemProto *p_proto = AttRes::GetInstance()->GetMallItemProto(p_guild_buy_info_->dwMallID);

		if (!VALID_POINT(p_proto))
		{
			ASSERT(p_proto);
			return;
		}

		NET_SIS_respond_broad send;
		send.eType = ERespondBroadCast_Lose;
		send.dw_role_id = p_guild_buy_info_->dw_role_id;
		send.dw_data_id = p_proto->dw_data_id;
		p_guild->send_guild_message(&send, send.dw_size);
	}

	remove_guild_buy_info_to_db(p_guild_buy_info_);

	SAFE_DELETE(p_guild_buy_info_);

}

//-------------------------------------------------------------------------------------------------------
//！ 删除帮会团购数据
//-------------------------------------------------------------------------------------------------------
VOID guild_buy::remove_guild_buy_info()
{
	tagGroupPurchase *p_guild_buy_info_ = NULL;
	if (!map_guild_buy_info.empty())
	{
		MAP_GUILD_BUY_INFO::map_iter iter = map_guild_buy_info.begin();
		while (map_guild_buy_info.find_next(iter, p_guild_buy_info_))
		{
			return_cost_to_role(p_guild_buy_info_);
			SAFE_DELETE(p_guild_buy_info_);
		}
	}
	map_guild_buy_info.clear();

	remove_guild_buy_info_to_db();

}

//-------------------------------------------------------------------------------------------------------
// ！ 退还玩家费用
//-------------------------------------------------------------------------------------------------------
VOID guild_buy::return_cost_to_role( tagGroupPurchase* p_guild_buy_info_ )
{
	if (!VALID_POINT(p_guild_buy_info_) || !VALID_POINT(p_guild_buy_info_->listParticipators))
		return;

	DWORD dw_role_id = INVALID_VALUE;

	p_guild_buy_info_->listParticipators->reset_iterator();
	while(p_guild_buy_info_->listParticipators->find_next(dw_role_id))
	{
		Role* p_role = g_roleMgr.get_role(dw_role_id);
		if (!VALID_POINT(p_role))
		{
			//! 修改离线玩家元宝数量
			CurrencyMgr::ModifyBaiBaoYuanBao(dw_role_id, p_guild_buy_info_->nPrice, elcid_group_purchase_faild);
		}
		else
		{
			//！ 修改在线玩家元宝
			p_role->GetCurMgr().IncBaiBaoYuanBao(p_guild_buy_info_->nPrice, elcid_group_purchase_faild);
		}
	}
}

VOID guild_buy::add_guild_buy_info_to_db( tagGroupPurchase* p_guild_buy_info_ )
{
	ASSERT(VALID_POINT(p_guild_buy_info_));
	if (!VALID_POINT(p_guild_buy_info_))
		return;

	NET_DB2C_add_gp_info send;

	send.gp_info.dw_guild_id			= p_guild_buy_info_->dwGuildID;
	send.gp_info.dw_role_id			= p_guild_buy_info_->dw_role_id;
	send.gp_info.dw_mall_id			= p_guild_buy_info_->dwMallID;
	send.gp_info.n_price				= p_guild_buy_info_->nPrice;
	send.gp_info.n_participator_num	= p_guild_buy_info_->nParticipatorNum;
	send.gp_info.n_require_num			= p_guild_buy_info_->nRequireNum;
	send.gp_info.n_remain_time			= p_guild_buy_info_->dwRemainTime;
	send.gp_info.dw_participators[0]	= p_guild_buy_info_->dw_role_id;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_buy::update_respond_info_to_db( tagGroupPurchase* p_guild_buy_info_ )
{
	ASSERT(VALID_POINT(p_guild_buy_info_));
	if (!VALID_POINT(p_guild_buy_info_))
		return;

	NET_DB2C_update_gp_info send;

	send.gp_info_key_.dw_guild_id	= p_guild_buy_info_->dwGuildID;
	send.gp_info_key_.dw_role_id		= p_guild_buy_info_->dw_role_id;
	send.gp_info_key_.dw_mall_id		= p_guild_buy_info_->dwMallID;
	send.dw_new_participator_		= p_guild_buy_info_->listParticipators->get_list().back();

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_buy::remove_guild_buy_info_to_db( tagGroupPurchase* p_guild_buy_info_ )
{
	ASSERT(VALID_POINT(p_guild_buy_info_));
	if (!VALID_POINT(p_guild_buy_info_))
		return;

	NET_DB2C_remove_gp_info send;

	send.gp_info_key_.dw_guild_id	= p_guild_buy_info_->dwGuildID;
	send.gp_info_key_.dw_role_id		= p_guild_buy_info_->dw_role_id;
	send.gp_info_key_.dw_mall_id		= p_guild_buy_info_->dwMallID;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_buy::remove_guild_buy_info_to_db()
{
	ASSERT(VALID_POINT(p_guild));
	if (!VALID_POINT(p_guild))
	{
		return;
	}

	NET_DB2C_remove_guild_gp_info send;

	send.dw_guild_id = p_guild->get_guild_att().dwID;

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
//! 创建map键值
//-------------------------------------------------------------------------------------------------------
INT64 guild_buy::get_key( DWORD dw_role_id_, DWORD dw_id_ )
{
	ASSERT(VALID_VALUE(dw_role_id_) && VALID_VALUE(dw_id_));
	if (!VALID_VALUE(dw_role_id_) || !VALID_VALUE(dw_id_))
		return INVALID_VALUE;

	INT64 n64_key = dw_role_id_;

	n64_key = (n64_key << 32) | dw_id_;

	return n64_key;
}

//-------------------------------------------------------------------------------------------------------
//! 帮会团购成功
//-------------------------------------------------------------------------------------------------------
DWORD guild_buy::guild_buy_success( tagGroupPurchase* p_guild_buy_info_ )
{
	if (p_guild->get_guild_att().dwID != p_guild_buy_info_->dwGuildID)
	{
		return INVALID_VALUE;
	}

	DWORD dw_item_type_id		= INVALID_VALUE;
	DWORD dw_launch_role_id		= p_guild_buy_info_->dw_role_id;

	//! 向此团购的所有玩家发送物品
	DWORD dw_temp_role_id = INVALID_VALUE;
	package_list<DWORD>::list_iter iter = p_guild_buy_info_->listParticipators->begin();	
	
	while(p_guild_buy_info_->listParticipators->find_next(iter, dw_temp_role_id))
	{
		// 生成物品
		tagMallItemSell	st_item_sell;
		DWORD dw_error_code = create_guild_buy_items(p_guild_buy_info_->dwMallID, st_item_sell);
		if (dw_error_code != E_Success)
		{
			return dw_error_code;
		}
		else if (VALID_POINT(st_item_sell.pItem))
		{
			dw_item_type_id = st_item_sell.pItem->dw_data_id;
		}
		else
		{
			return INVALID_VALUE;
		}

		if(VALID_POINT(st_item_sell.pItem))
		{		
			INT64 n64_serial = st_item_sell.pItem->n64_serial;
			DWORD dw_gain_time = g_world.GetWorldTime();
			INT16 n16_buy_num = st_item_sell.pItem->n16Num;

			ItemMgr::ProcBaiBaoRecord(st_item_sell.pItem->dw_data_id, dw_temp_role_id, INVALID_VALUE, EBBRT_GroupPurchase, dw_gain_time);

			g_mall.LogMallSell(dw_temp_role_id, INVALID_VALUE, *st_item_sell.pItem, n64_serial, n16_buy_num, 
				dw_gain_time, st_item_sell.nYuanBaoNeed, 0, elcid_group_purchase_buy_item);

			Role *p_temp_role = g_roleMgr.get_role(dw_temp_role_id);
			if(VALID_POINT(p_temp_role))
			{
				p_temp_role->GetItemMgr().Add2BaiBao(st_item_sell.pItem, elcid_group_purchase_buy_item);
			}
			else
			{
				ItemMgr::InsertBaiBao2DB(st_item_sell.pItem, dw_temp_role_id, elcid_group_purchase_buy_item);

				::Destroy(st_item_sell.pItem);
			}

			if(VALID_POINT(st_item_sell.pPresent))
			{
				ItemMgr::ProcBaiBaoRecord(st_item_sell.pPresent->dw_data_id, dw_temp_role_id, INVALID_VALUE, EBBRT_Mall, dw_gain_time);

				if(VALID_POINT(p_temp_role))
				{
					p_temp_role->GetItemMgr().Add2BaiBao(st_item_sell.pPresent, elcid_group_purchase_buy_item_add);
				}
				else
				{
					ItemMgr::InsertBaiBao2DB(st_item_sell.pPresent, dw_temp_role_id, elcid_group_purchase_buy_item_add);

					::Destroy(st_item_sell.pItem);
				}
			}

			if (VALID_POINT(p_temp_role) && st_item_sell.nExVolumeAssign > 0)
			{
				p_temp_role->GetCurMgr().IncExchangeVolume(st_item_sell.nExVolumeAssign, elcid_group_purchase_buy_item);
			}
		}
	}

	//! 增加帮会团购指数
	p_guild->modify_group_exponent(TRUE);

	//！删除此团购记录
	remove_guild_buy_info(p_guild_buy_info_);

	NET_SIS_respond_broad send;
	send.eType = ERespondBroadCast_Success;
	send.dw_role_id = dw_launch_role_id;
	send.dw_data_id = dw_item_type_id;
	p_guild->send_guild_message(&send, send.dw_size);

	return E_Success;
}