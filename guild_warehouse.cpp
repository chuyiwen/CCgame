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
*	@file		guild_warehouse.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/

#pragma once

#include "StdAfx.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../../common/WorldDefine/function_npc_protocol.h"
#include "../../common/WorldDefine/guild_define.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/guild_server_define.h"
#include "container_restrict.h"
#include "role.h"
#include "role_mgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "guild_warehouse.h"
#include "att_res.h"
#include "map.h"


GuildWarehouse::GuildWarehouse()
{
	p_guild = NULL;
	p_container = NULL;
	b_init_ok = FALSE;
}

GuildWarehouse::~GuildWarehouse()
{
	SAFE_DELETE(p_container);
}

VOID GuildWarehouse::init(guild* p_guild_, BOOL b_request_ /*= FALSE*/)
{
	if (!VALID_POINT(p_guild_))
	{
		return;
	}

	b_init_ok		= FALSE;
	p_guild		= p_guild_;
	INT16 n_current_size	= MGuildWareCurSpace(p_guild_->get_guild_att().byLevel);
	p_container	= new ItemContainer(EICT_GuildWare, n_current_size, MAX_GUILD_WARE_SPACE, INVALID_VALUE, p_guild_->get_guild_att().dwID, new GuildWareRestrict);

	dw_guild_ware_time	= g_world.GetWorldTime();
	dw_last_save_time	= g_world.GetWorldTime();
	vector_update_time.resize(n_current_size);
	vector_update_time.assign(n_current_size, dw_guild_ware_time);

	register_guild_ware_event();

	if (b_request_)
	{
		NET_DB2C_load_guild_ware_items send;
		send.dw_guild_id = p_guild->get_guild_att().dwID;
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		b_init_ok = TRUE;
	}
}

VOID GuildWarehouse::reset_init()
{
	if (!VALID_POINT(p_guild))
	{
		ASSERT(p_guild);
		return;
	}
	else
	{
		//! 仓库当前容量
		INT16 n_current_size = p_container->GetCurSpaceSize();

		//! 计算仓库扩充大小
		INT16 n16_extend_size = MGuildWareCurSpace(p_guild->get_guild_att().byLevel) - n_current_size;
		if (n16_extend_size <= 0)
		{
			ASSERT(n16_extend_size > 0);
			return;
		}

		//! 扩充仓库
		p_container->IncreaseSize(n16_extend_size);

		n_current_size = p_container->GetCurSpaceSize();
		dw_guild_ware_time = g_world.GetWorldTime();
		vector_update_time.resize(n_current_size);
		vector_update_time.assign(n_current_size, dw_guild_ware_time);
	}
}

VOID GuildWarehouse::update()
{
	if (!b_init_ok)
	{
		return;
	}

	EventMgr<GuildWarehouse>::Update();

	save_update_item_to_db();
}

VOID GuildWarehouse::on_move_to( DWORD dw_sender_id_, VOID* p_event_message_ )
{
	if (!b_init_ok)
	{
		return;
	}

	Role* p_role = g_roleMgr.get_role(dw_sender_id_);
	if (!VALID_POINT(p_role))
	{
		return;
	}

	DWORD dw_guild_id = p_role->GetGuildID();
	if (!VALID_VALUE(dw_guild_id) || p_guild->get_guild_att().dwID != dw_guild_id)
	{
		return;
	}

	tagGuildMember* p_member = p_guild->get_member(dw_sender_id_);
	if (!VALID_POINT(p_member))
	{
		return;
	}

	if (!p_member->bUseGuildWare && !p_guild->get_guild_power(p_member->eGuildPos).bSetWareRights)
	{
		return;
	}

	//GET_MESSAGE(p_receive, p_event_message_, NET_SIC_item_position_change);
	NET_SIC_item_position_change * p_receive = ( NET_SIC_item_position_change * ) p_event_message_ ;  
	DWORD dw_error_code = move(p_role, p_receive->n64_serial, p_receive->n16Num, p_receive->n16PosDst, elcid_item_move);
	
}

VOID GuildWarehouse::on_move_to_other( DWORD dw_sender_id_, VOID* p_event_message_ )
{
	if (!b_init_ok)
	{
		return;
	}

	Role* p_role = g_roleMgr.get_role(dw_sender_id_);
	if (!VALID_POINT(p_role))
	{
		return;
	}

	DWORD dw_guild_id = p_role->GetGuildID();
	if (!VALID_VALUE(dw_guild_id) || p_guild->get_guild_att().dwID != dw_guild_id)
	{
		return;
	}

	tagGuildMember* p_member = p_guild->get_member(dw_sender_id_);
	if (!VALID_POINT(p_member))
	{
		return;
	}

	if (!p_member->bUseGuildWare && !p_guild->get_guild_power(p_member->eGuildPos).bSetWareRights)
	{
		return;
	}

	//GET_MESSAGE(p_receive, p_event_message_, NET_SIC_item_position_change_extend);
	NET_SIC_item_position_change_extend * p_receive = ( NET_SIC_item_position_change_extend * ) p_event_message_ ;  
	DWORD dw_error_code = move_to_other(p_role, p_receive->eConTypeSrc, p_receive->n64Serial1, 
		p_receive->eConTypeDst, p_receive->n16PosDst, elcid_item_move_to_other);
}

//-------------------------------------------------------------------------------------------------------
// 注册延后处理消息
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::register_guild_ware_event()
{
	RegisterEventFunc(EVT_GuildWareMoveTo,			&GuildWarehouse::on_move_to);
	RegisterEventFunc(EVT_GuildWareMove2Other,		&GuildWarehouse::on_move_to_other);
}

//-------------------------------------------------------------------------------------------------------
//! 获取帮会仓库内容
//-------------------------------------------------------------------------------------------------------
DWORD GuildWarehouse::get_guild_ware_info( BYTE* p_buffer_, INT& n_item_num_, DWORD& dw_last_update_time_, INT16& n_ware_size_, INT32& n_size_ )
{
	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	if (!VALID_POINT(p_buffer_))
	{
		return INVALID_VALUE;
	}
	BYTE* p_temp_buffer = p_buffer_;
	n_item_num_ = 0;
	n_size_ = 0;

	if (dw_guild_ware_time <= dw_last_update_time_)
	{
		return E_GuildWare_NoChange;
	}

	n_ware_size_ = p_container->GetCurSpaceSize();

	INT n_current_size = vector_update_time.size();
	for (INT16 n=0; n<n_current_size; n++)
	{
		if (vector_update_time[n] > dw_last_update_time_)
		{
			tagGuildWareUpdate* p_temp = (tagGuildWareUpdate*)p_temp_buffer;
			p_temp->nIndex = n;

			tagItem* p_item = p_container->GetItem(n);
			if (!VALID_POINT(p_item))
			{
				if (dw_last_update_time_ == 0)
				{
					continue;
				}
				p_temp->eType = EGWU_Delete;
				n_item_num_++;
				p_temp_buffer += p_temp->Size();
				continue;
			}

			if (!MIsEquipment(p_item->dw_data_id))
			{
				get_fast_code()->memory_copy(p_temp->byData, p_item, sizeof(tagItem));
			}
			else
			{
				get_fast_code()->memory_copy(p_temp->byData, p_item, sizeof(tagEquip));
			}
			n_item_num_++;

			p_temp_buffer += p_temp->Size();
		}
	}

	n_size_ = (p_temp_buffer - p_buffer_) * sizeof(BYTE);
	dw_last_update_time_ = dw_guild_ware_time;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//! 帮会仓库物品移动（n16_num_==0代表全部移动)
//-------------------------------------------------------------------------------------------------------
DWORD GuildWarehouse::move( Role* p_role_, INT64 n64_serial_, INT16 n16_num_, INT16 n16_dest_position_, DWORD dw_command_id_ )
{
	ASSERT(n16_num_ >= 0);
	ASSERT(VALID_POINT(p_role_));

	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = move_allowable(p_role_);
	if (dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(p_role_->GetSession(), dw_error_code);
		return dw_error_code;
	}

	ItemContainer *p_item_container = p_container;
	if(!VALID_POINT(p_item_container))
	{
		return INVALID_VALUE;
	}

	tagItem *p_item1 = p_item_container->GetItem(n64_serial_);
	if(!VALID_POINT(p_item1))
	{
		return INVALID_VALUE;
	}

	dw_error_code	= INVALID_VALUE;
	INT16 n16_src_position		= p_item1->n16Index;	

	s_item_move	st_item_move;

	if(0 == n16_num_)
	{
		n16_num_ = p_item1->n16Num;
		dw_error_code = p_item_container->MoveTo(n64_serial_, n16_dest_position_, st_item_move);
	}
	else if(n16_num_ > 0)
	{
		dw_error_code = p_item_container->MoveTo(n64_serial_, n16_num_, n16_dest_position_, st_item_move);
	}

	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);

		return dw_error_code;
	}

	NET_SIS_item_position_change send;
	send.eConType	= EICT_GuildWare;
	send.n64Serial1	= n64_serial_;
	send.n64Serial2	= st_item_move.p_item2 != NULL ? st_item_move.p_item2->n64_serial : INVALID_VALUE;

	send.n16Num1	= st_item_move.n_num_res1;
	send.n16Num2	= st_item_move.n_num_res2;
	send.bCreateItem = st_item_move.b_create_item;

	if(st_item_move.b_change_pos)
	{
		send.n16PosDst1	= n16_dest_position_;
		send.n16PosDst2	= n16_src_position;
	}
	else
	{
		send.n16PosDst1	= n16_src_position;
		send.n16PosDst2	= n16_dest_position_;
	}

	if(st_item_move.b_overlap)
	{
		send.n16PosDst1	= n16_src_position;
		send.n16PosDst2	= n16_dest_position_;
	}

	send_message(p_role_, &send, send.dw_size);

	if(st_item_move.b_create_item || st_item_move.b_overlap)
	{
		log_item(p_role_->GetID(), *p_item1, st_item_move.p_item2, -n16_num_, dw_command_id_);
	}

	if(0 == st_item_move.n_num_res1)
	{
		delete_item_from_db(n64_serial_, p_item1->dw_data_id);

		Destroy(p_item1);
	}

	if(st_item_move.b_create_item)
	{
		NET_DB2C_new_item send;
		get_fast_code()->memory_copy(&send.item, st_item_move.p_item2, SIZE_ITEM);

		g_dbSession.Send(&send, send.dw_size);
	}

	set_update_time(n16_src_position);
	set_update_time(n16_dest_position_);

	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
//! 帮会仓库与玩家背包之间移动
//-------------------------------------------------------------------------------------------------------
DWORD GuildWarehouse::move_to_other( Role* p_role_, EItemConType e_container_type_src_, INT64 n64_serial_, 
									EItemConType e_container_type_dest_, INT16 n16_position_dest_, DWORD dw_command_id_ )
{
	ASSERT(VALID_POINT(p_role_));

	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = move_allowable(p_role_);
	if (dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(p_role_->GetSession(), dw_error_code);
		return dw_error_code;
	}

	ItemContainer *p_container_src = NULL;
	ItemContainer *p_container_dest = NULL;

	if (e_container_type_src_ == EICT_Bag && e_container_type_dest_ == EICT_GuildWare)
	{
		p_container_src = &(p_role_->GetItemMgr().GetBag());
		p_container_dest = p_container;
	}
	else if (e_container_type_src_ == EICT_GuildWare && e_container_type_dest_ == EICT_Bag)
	{
		p_container_src = p_container;
		p_container_dest = &(p_role_->GetItemMgr().GetBag());
	}
	else
	{
		return INVALID_VALUE;
	}

	if(!VALID_POINT(p_container_src) || !VALID_POINT(p_container_dest))
	{
		return INVALID_VALUE;
	}

	tagItem *p_item1 = p_container_src->GetItem(n64_serial_);
	if(!VALID_POINT(p_item1))
	{
		return INVALID_VALUE;
	}

	if (p_item1->IsBind())
	{
		return INVALID_VALUE;
	}

	dw_error_code	= INVALID_VALUE;
	INT16 n16_position_src1	= p_item1->n16Index;	
	INT16 n16_num		= p_item1->n16Num;
	DWORD dw_data_id		= p_item1->dw_data_id;
	INT16 n16destNum = 0;
	DWORD dw_dest_data_id = 0;
	if(!p_container_dest->IsOnePlaceFree(n16_position_dest_)){
		tagItem *pDstItem = p_container_dest->GetItem(n16_position_dest_);
		if(VALID_POINT(pDstItem)){
			n16destNum = pDstItem->n16Num;
			dw_dest_data_id = pDstItem->dw_data_id;
		}
	}


	s_item_move	st_item_move;

	if(INVALID_VALUE == n16_position_dest_)	//! 右键点击，没有指定具体位置
	{
		dw_error_code = p_container_src->MoveTo(n64_serial_, *p_container_dest, st_item_move, n16_position_dest_);
	}
	else
	{
		dw_error_code = p_container_src->MoveTo(n64_serial_, *p_container_dest, n16_position_dest_, st_item_move);
	}

	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);

		return dw_error_code;
	}


	NET_SIS_item_position_change_extend send;

	send.eConTypeSrc1	= e_container_type_src_;
	send.eConTypeSrc2	= e_container_type_dest_;
	send.n64Serial1		= n64_serial_;
	send.n64Serial2		= st_item_move.p_item2 != NULL ? st_item_move.p_item2->n64_serial : INVALID_VALUE;
	send.n16Num1		= st_item_move.n_num_res1;
	send.n16Num2		= st_item_move.n_num_res2;

	if(st_item_move.b_overlap)
	{
		send.eConTypeDst1	= e_container_type_src_;
		send.eConTypeDst2	= e_container_type_dest_;
		send.n16PosDst1		= n16_position_src1;
		send.n16PosDst2		= n16_position_dest_;
	}
	else
	{
		send.eConTypeDst1	= e_container_type_dest_;
		send.eConTypeDst2	= e_container_type_src_;
		send.n16PosDst1		= n16_position_dest_;
		send.n16PosDst2		= n16_position_src1;
	}

	if( VALID_POINT(p_role_) && e_container_type_src_ == EICT_GuildWare && e_container_type_dest_ == EICT_Bag)
	{
		if(st_item_move.b_overlap) n16_num -= send.n16Num1;
		if(st_item_move.b_overlap) n16destNum -= send.n16Num2;
		if(n16_num) p_role_->on_quest_event(EQE_Item, dw_data_id, n16_num, TRUE);
		if(dw_dest_data_id && n16destNum){
			p_role_->on_quest_event(EQE_Item, dw_dest_data_id, n16destNum, FALSE);
		}
	}
	else if(  VALID_POINT(p_role_) && e_container_type_src_ == EICT_Bag && e_container_type_dest_ == EICT_GuildWare)
	{
		if(st_item_move.b_overlap) n16_num -= send.n16Num1;
		if(st_item_move.b_overlap) n16destNum -= send.n16Num2;
		if(n16_num)p_role_->on_quest_event(EQE_Item, dw_data_id, n16_num, FALSE);
		if(dw_dest_data_id && n16destNum){
			p_role_->on_quest_event(EQE_Item, dw_dest_data_id, n16destNum, TRUE);
		}
	}

	send_message(p_role_, &send, send.dw_size);

	log_item(p_role_->GetID(), *p_item1, st_item_move.p_item2, p_item1->n16Num - n16_num, dw_command_id_);

	if(0 == st_item_move.n_num_res1)
	{
		delete_item_from_db(n64_serial_, p_item1->dw_data_id);

		Destroy(p_item1);
	}

	if (e_container_type_src_ == EICT_GuildWare)
	{
		set_update_time(n16_position_src1);
	}
	else if (e_container_type_dest_ == EICT_GuildWare)
	{
		set_update_time(n16_position_dest_);
	}
	else
	{
		ASSERT(0);
	}

	return dw_error_code;
}

VOID GuildWarehouse::send_message( Role* p_role_, LPVOID p_message_, DWORD dw_size_ )
{
	ASSERT(VALID_POINT(p_role_));

	if (!VALID_POINT(p_role_))
	{
		return;
	}

	PlayerSession *p_session = p_role_->GetSession();
	if(VALID_POINT(p_session))
	{
		p_session->SendMessage(p_message_, dw_size_);
	}

	ASSERT(VALID_POINT(p_session));
}


VOID GuildWarehouse::log_item(DWORD dw_role_id_, const tagItem &st_item1_, const tagItem *st_item2_, INT16 n16_num_, DWORD dw_command_id_)
{
	if(!(is_item_need_log(st_item1_) || (VALID_POINT(st_item2_) && is_item_need_log(*st_item2_))))
	{
		return;
	}

	NET_DB2C_log_item send;
	send.s_log_item_.dw_role_id		= dw_role_id_;
	send.s_log_item_.dw_data_id		= st_item1_.pProtoType->dw_data_id;

	send.s_log_item_.n64_serial1	= st_item1_.n64_serial;
	send.s_log_item_.n8_con_type1	= st_item1_.eConType;
	send.s_log_item_.n16_res_num1	= st_item1_.n16Num;

	send.s_log_item_.n16_opt_num		= n16_num_;
	send.s_log_item_.dw_cmd_id		= dw_command_id_;

	if(VALID_POINT(st_item2_))
	{
		send.s_log_item_.n64_serial2	= st_item2_->n64_serial;
		send.s_log_item_.n8_con_type2	= st_item2_->eConType;
		send.s_log_item_.n16_res_num2	= st_item2_->n16Num;
	}
	else
	{
		send.s_log_item_.n64_serial2	= 0;
		send.s_log_item_.n8_con_type2	= EICT_Null;
		send.s_log_item_.n16_res_num2	= 0;
	}

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
//! 设置指定栏位的更新时间
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::set_update_time( INT16 n_position_ )
{
	vector_update_time[n_position_] = dw_guild_ware_time = g_world.GetWorldTime();
}

//-------------------------------------------------------------------------------------------------------
//! 帮会仓库插入新物品
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::insert_item_to_db(tagItem &st_item_)
{
	if(MIsEquipment(st_item_.dw_data_id))
	{
		NET_DB2C_new_equip send;
		get_fast_code()->memory_copy(&send.equip, &st_item_, SIZE_EQUIP);
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_new_item send;
		get_fast_code()->memory_copy(&send.item, &st_item_, SIZE_ITEM);
		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
//! 删除帮会物品
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::delete_item_from_db(INT64 n64_serial_, INT32 dw_data_id_)
{
	if(!MIsEquipment(dw_data_id_))
	{
		NET_DB2C_delete_item send;
		send.n64_serial = n64_serial_;

		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_delete_equip send;
		send.n64_serial = n64_serial_;

		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
//! 更新帮会仓库物品
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::save_update_item_to_db()
{
	//! 检查帮会仓库物品是否需要更新
	if (dw_guild_ware_time <= dw_last_save_time)
	{
		return;
	}

	INT n_message_size = sizeof(NET_DB2C_save_guild_ware) + p_container->GetCurSpaceSize() * sizeof(s_item_update);
	CREATE_MSG(p_send, n_message_size, NET_DB2C_save_guild_ware);
	M_trans_pointer(p_item_update, p_send->by_data, s_item_update);
	p_send->n_item_num = 0;

	tagItem* p_temp	= NULL;
	for(INT16 i=0; i<p_container->GetCurSpaceSize(); ++i)
	{
		p_temp = p_container->GetItem(i);
		if(VALID_POINT(p_temp) && p_temp->eStatus != EUDBS_Null)
		{
			p_item_update[p_send->n_item_num].by_conType	= p_temp->eConType;
			p_item_update[p_send->n_item_num].dw_owner_id	= p_temp->dwOwnerID;
			p_item_update[p_send->n_item_num].dw_account_id= p_temp->dw_account_id;
			p_item_update[p_send->n_item_num].n16_index	= p_temp->n16Index;
			p_item_update[p_send->n_item_num].n16_num		= p_temp->n16Num;
			p_item_update[p_send->n_item_num].n64_serial	= p_temp->n64_serial;
			p_item_update[p_send->n_item_num].n_use_times	= p_temp->nUseTimes;
			p_item_update[p_send->n_item_num].by_bind		= p_temp->byBind;
			p_item_update[p_send->n_item_num].dw_bind_time = p_temp->dwBindTime;
			memcpy(p_item_update[p_send->n_item_num].dw_script_data, p_temp->dw_script_data, sizeof(DWORD)*2);
			p_temp->SetUpdate(EUDBS_Null);

			p_send->n_item_num++;
		}
	}

	if (p_send->n_item_num > 0)
	{
		p_send->dw_size = sizeof(NET_DB2C_save_guild_ware) + p_send->n_item_num * sizeof(s_item_update);

		g_dbSession.Send(p_send, p_send->dw_size);

		dw_last_save_time = dw_guild_ware_time;
	}
}

//-------------------------------------------------------------------------------------------------------
//! 初始化时载入帮会仓库物品
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::load_warehouse_items( const BYTE* p_data_, INT n_item_num_ )
{
	INT32 n_item_size		= sizeof(tagItem);
	INT32 n_equip_size	= sizeof(tagEquip);

	DWORD dw_error_code = INVALID_VALUE;
	package_list<tagItem *> list_item;
	const tagItem	*p_temp_item	= NULL;
	tagItem			*p_new_item	= NULL;

	p_temp_item = (const tagItem *)p_data_;
	for(INT32 i=0; i<n_item_num_; ++i)
	{
		if(!MIsEquipment(p_temp_item->dw_data_id))
		{
			p_new_item = new tagItem;
			get_fast_code()->memory_copy(p_new_item, p_temp_item, n_item_size);
			p_new_item->pProtoType = AttRes::GetInstance()->GetItemProto(p_temp_item->dw_data_id);

			p_temp_item = (const tagItem*)((BYTE*)p_temp_item + n_item_size);
		}
		else
		{
			p_new_item = new tagEquip;
			get_fast_code()->memory_copy(p_new_item, p_temp_item, n_equip_size);
			p_new_item->pProtoType = AttRes::GetInstance()->GetEquipProto(p_temp_item->dw_data_id);

			p_temp_item = (tagEquip*)((BYTE*)p_temp_item + n_equip_size);
		}

		if(!VALID_POINT(p_new_item->pProtoType))
		{
			ASSERT(VALID_POINT(p_new_item->pProtoType));
			
			Destroy(p_new_item);
			continue;
		}

		p_new_item->eStatus = EUDBS_Null;
		p_new_item->pScript = g_ScriptMgr.GetItemScript( p_new_item->dw_data_id);

		dw_error_code = p_container->Add(p_new_item, p_new_item->n16Index, FALSE, FALSE);
		if(dw_error_code != E_Success)
		{
			if(dw_error_code != E_Item_Place_NotFree)
			{
				ASSERT(0);
				Destroy(p_new_item);
				continue;
			}

			list_item.push_back(p_new_item);
		}
	}

	while(list_item.size() != 0)
	{
		INT16 n_index;
		s_item_move st_item_move;
		p_new_item = list_item.pop_front();

		switch(p_new_item->eConType)
		{
		case EICT_GuildWare:
			dw_error_code = p_container->Add(p_new_item, n_index, st_item_move, FALSE, FALSE);
			if(E_Con_NotEnoughSpace == dw_error_code)
			{
				
			}
			break;

		default:
			ASSERT(0);
			Destroy(p_new_item);
			continue;
		}
	}

	b_init_ok = TRUE;
}

//-------------------------------------------------------------------------------------------------------
// ! 检查玩家是否可以操作帮会仓库
//-------------------------------------------------------------------------------------------------------
DWORD GuildWarehouse::move_allowable( Role* p_role_ )
{
	ASSERT(VALID_POINT(p_role_));

	DWORD dw_error_code = E_Success;

	DWORD dw_guild_id = p_role_->GetGuildID();
	if (!VALID_VALUE(dw_guild_id) || p_guild->get_guild_att().dwID != dw_guild_id)
	{
		dw_error_code = E_Guild_MemberNotIn;
	}

	tagGuildMember* p_member = p_guild->get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		p_role_->SetGuildID(INVALID_VALUE);
		dw_error_code = E_Guild_MemberNotIn;
	}
	if (!p_member->bUseGuildWare && !p_guild->get_guild_power(p_member->eGuildPos).bSetWareRights)
	{
		dw_error_code = E_Guild_Power_NotEnough;
	}

	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
//! 帮会解散时删除所有物品
//-------------------------------------------------------------------------------------------------------
VOID GuildWarehouse::remove_all_items()
{
	for(INT16 i=0; i<p_container->GetCurSpaceSize(); ++i)
	{
		tagItem *p_item = p_container->GetItem(i);

		if (VALID_POINT(p_item))
		{
			p_container->Remove(p_item->n64_serial, TRUE, FALSE);

			delete_item_from_db(p_item->n64_serial, p_item->dw_data_id);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
//! 发送帮会仓库权限列表
//-------------------------------------------------------------------------------------------------------
DWORD GuildWarehouse::send_guild_ware_power_list_to_client( Role* p_role_ )
{
	if (!b_init_ok)
	{
		return INVALID_VALUE;
	}

	ASSERT(VALID_POINT(p_role_));
	if (!VALID_POINT(p_role_))
	{
		return INVALID_VALUE;
	}

	INT16 n_num = p_guild->get_guild_member_num();
	ASSERT(n_num > 0);
	if (n_num <= 0)
	{
		return INVALID_VALUE;
	}

	tagGuildMember* p_member = p_guild->get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}

	if (!p_guild->get_guild_power(p_member->eGuildPos).bSetWareRights)
	{
		return E_Guild_Power_NotEnough;
	}

	INT32 n_message_size = sizeof(NET_SIS_get_guild_ware_power_list) + (n_num - 1) * sizeof(tagGuildWarePri);
	CREATE_MSG(p_send, n_message_size, NET_SIS_get_guild_ware_power_list);

	p_send->n_num = 0;
	M_trans_pointer(pPriInfo, p_send->sGuildWarePri, tagGuildWarePri);

	MAP_GUILD_MEMBER& mapMember = p_guild->get_guild_member_map();

	p_member = NULL;
	MAP_GUILD_MEMBER::map_iter iter = mapMember.begin();
	while (mapMember.find_next(iter, p_member))
	{
		if (!VALID_POINT(p_member))
		{
			continue;
		}

		if (p_guild->get_guild_power(p_member->eGuildPos).bSetWareRights)
		{
			continue;
		}

		pPriInfo[p_send->n_num].dw_role_id	= p_member->dw_role_id;
		pPriInfo[p_send->n_num].bCanUse	= p_member->bUseGuildWare;
		p_send->n_num++;
	}

	p_send->dw_size = sizeof(NET_SIS_get_guild_ware_power_list) + (p_send->n_num - 1) * sizeof(tagGuildWarePri);

	p_role_->SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
//! 向帮会仓库中放入物品
//-------------------------------------------------------------------------------------------------------
DWORD GuildWarehouse::add_item( tagItem *&p_item_, DWORD dw_command_id_, BOOL b_insert_to_db_ /*= TRUE*/ )
{
	ASSERT(VALID_POINT(p_item_));
	ASSERT(VALID_POINT(p_item_->n64_serial));

	INT16		n16_index;
	DWORD		dw_error_code;
	s_item_move	st_item_move;
	INT16		n16_add_num = p_item_->n16Num;

	if(0 == p_item_->dw1stGainTime)
	{
		p_item_->dw1stGainTime = g_world.GetWorldTime();
	}

	dw_error_code = p_container->Add(p_item_, n16_index, st_item_move);

	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
		

		return dw_error_code;
	}

	INT16 n16Num = st_item_move.n_num_res1 != 0 ? st_item_move.n_num_res1: st_item_move.n_num_res2;

	if(b_insert_to_db_ && !st_item_move.b_overlap)
	{
		insert_item_to_db(*p_item_);
		p_item_->SetUpdate(EUDBS_Null);
	}
	
	log_item(INVALID_VALUE, *p_item_, st_item_move.p_item2, n16_add_num, dw_command_id_);

	if(st_item_move.b_overlap)
	{
		Destroy(p_item_);
		p_item_ = st_item_move.p_item2;
	}

	set_update_time(n16_index);

	return dw_error_code;
}
