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
*	@file		guild_manager.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/

#include "StdAfx.h"

#include "../../common/WorldDefine/filter.h"
#include "../../common/WorldDefine/guild_protocol.h"
#include "../common/ServerDefine/guild_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"

#include "guild_manager.h"
#include "role_mgr.h"
#include "role.h"
#include "guild.h"
#include "script_mgr.h"
#include "mall.h"
#include "item_mgr.h"
#include "item_creator.h"
#include "map_creator.h"
#include "hearSay_helper.h"

guild_manager g_guild_manager;

guild_manager::guild_manager()
{
	map_guild_kill_boss.clear();
	m_dw_SBK_guild = INVALID_VALUE;
	m_bSBKStart = false;
	m_bOpen = false;
	m_SBK_tick = SBKTICK;
	memset(&m_SBK_data, 0, sizeof(m_SBK_data));
}

guild_manager::~guild_manager()
{
	destroy();
}


BOOL guild_manager::init()
{
	INT32 n_temp = 0;

	g_ScriptMgr.GetGlobal("guild_CreateRoleMinLevel", n_temp);
	st_guild_config.nCreateRoleMinLevel	= n_temp;

	g_ScriptMgr.GetGlobal("guild_GoldCreateNeeded", n_temp);
	st_guild_config.nGoldCreateNeeded	= n_temp;

	g_ScriptMgr.GetGlobal("guild_GoldGuildWarNeeded", n_temp);
	st_guild_config.nGoldGuildWarNeeded = n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildLevelBegin", n_temp);
	st_guild_config.byGuildLevelBegin	= (BYTE)n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildPeaceBegin", n_temp);
	st_guild_config.n16GuildPeaceBegin	= (INT16)n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildRepBegin", n_temp);
	st_guild_config.nGuildRepBegin		= n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildFundBegin", n_temp);
	st_guild_config.nGuildFundBegin		= n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildProsperityBegin", n_temp);
	st_guild_config.nGuildProsperityBegin = n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildMaterialBegin", n_temp);
	st_guild_config.nGuildMaterialBegin	= n_temp;

	g_ScriptMgr.GetGlobal("guild_GuildGroupPurchaseBegin", n_temp);
	st_guild_config.nGuildGroupPurchaseBegin = n_temp;

	g_ScriptMgr.GetGlobal("guild_JoinRoleMinLevel", n_temp);
	st_guild_config.nJoinRoleMinLevel	= n_temp;

	g_ScriptMgr.GetGlobal("guild_SignRoleMinLevel", n_temp);
	st_guild_config.nSignRoleMinLevel = n_temp;

	g_ScriptMgr.GetGlobal("guild_MinNpcNameNum", n_temp);
	st_guild_config.nGuildMinNpcNameNum = n_temp;

	g_ScriptMgr.GetGlobal("guild_MaxNpcNameNum", n_temp);
	st_guild_config.nGuildMaxNpcNameNum = n_temp;

	return TRUE;
}


VOID guild_manager::update()
{
	guild *p_guild;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		p_guild->update();

		if(p_guild->get_delete())
		{
			map_guild.erase(p_guild->get_guild_att().dwID);
			SAFE_DELETE(p_guild);
		}
	}

	if (!is_begin_SBK())
		return;
	
	if (m_SBK_tick-- <= 0)
	{
		// 判断占领沙巴克城公会
		DWORD dw_guild_map_id = get_tool()->crc32(szSBKHuangGong);
		Map* p_map = g_mapCreator.get_map(dw_guild_map_id, INVALID_VALUE);
		if (!VALID_POINT(p_map)) return;
		DWORD dwGuildID = p_map->getAllRoleSameGuildID();
		guild* pGuild = get_guild(dwGuildID);
		if (VALID_POINT(pGuild) && pGuild->get_guild_att().bSignUpAttact)
		{
			if (dwGuildID != m_dw_SBK_guild)
			{
				set_SBK_gulid(dwGuildID);
				//发送公告
				TCHAR sz_buff[X_LONG_NAME] = _T("");
				tstring stname = pGuild->get_guild_att().str_name;
				_tcsncpy(sz_buff, stname.c_str(), stname.size());

				HearSayHelper::SendMessage(EHST_GUILDFIRSTKILL,
					INVALID_VALUE, m_dw_SBK_guild,  0, INVALID_VALUE, INVALID_VALUE, NULL, FALSE, sz_buff, (stname.length() + 1) * sizeof(TCHAR));


			}
		}	
		

		m_SBK_tick = SBKTICK; 
	}
	
}

VOID guild_manager::destroy()
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		SAFE_DELETE(p_guild);
	}

	map_guild.clear();

	s_guild_pvp_data* p_guild_pvp_data = NULL;
	MAP_PVP::map_iter pvpiter = map_pvp_data.begin();
	while(map_pvp_data.find_next(pvpiter, p_guild_pvp_data))
	{
		SAFE_DELETE(p_guild_pvp_data);
	}
	map_pvp_data.clear();
}

//-------------------------------------------------------------------------------
//! 初始化数据库传来的帮会数据
//-------------------------------------------------------------------------------
VOID guild_manager::init_db_guild(const s_guild_load* p_guild_load_)
{
	if(map_guild.is_exist(p_guild_load_->dwID))
	{
		return;
	}
	
	guild *p_new_guild = new guild(p_guild_load_);
	if(!VALID_POINT(p_new_guild))
	{
		ASSERT(0);
		print_message(_T("\n\n\nguild init faild<id=%u>!\n\n"), p_guild_load_->dwID);
		return;
	}

	map_guild.add(p_guild_load_->dwID, p_new_guild);
}

//-------------------------------------------------------------------------------
// 初始化数据库传来的帮会成员
//-------------------------------------------------------------------------------
VOID guild_manager::init_db_guild_member(const s_guild_member_load *p_guild_member_load_, const INT32 n_num_)
{
	ASSERT(n_num_ > 0);

	guild *p_check_guild = get_guild(p_guild_member_load_[0].dw_guild_id_);
	if(VALID_POINT(p_check_guild))
	{
		//！检查是否是初次连接数据库
		/*if(p_check_guild->get_guild_member_num() > 0)
		{
			ASSERT(0);
			print_message(_T("\n\nnot first connect db! \n"));
			print_message(_T("\tif get data please reset dbserver!\n\n"));
			return;
		}*/
	}
	
	for(INT32 i=0; i<n_num_; ++i)
	{
		guild *p_guild = get_guild(p_guild_member_load_[i].dw_guild_id_);
		if(!VALID_POINT(p_guild))
		{
			ASSERT(0);
			continue;
		}

		p_guild->init_db_guild_member(p_guild_member_load_[i].s_guild_member_);
	}
}

//-------------------------------------------------------------------------------
//! 检查所有帮会是否初始成功
//-------------------------------------------------------------------------------
BOOL guild_manager::is_guild_init_ok()
{
	BOOL b_result	= TRUE;

	guild* p_guild	= NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while (map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
		{
			continue;
		}

		if (!p_guild->is_guild_init_ok())
		{
			ASSERT(0);
			SI_LOG->write_log(_T("帮会初始化失败<id=%u>! \n"), p_guild->get_guild_att().dwID);
			b_result = FALSE;
		}
	}
	
	// 帮会信息初始化完成脚本事件
	const WorldScript* pScript = g_ScriptMgr.GetWorldScript();
	if (VALID_POINT(pScript)  )
	{
		pScript->OnGuildInitOk();
	}

	return b_result;
}

//-------------------------------------------------------------------------------
//! 关闭服务器是保存所有帮会信息
//-------------------------------------------------------------------------------
VOID guild_manager::send_all_guild_to_db()
{
	INT32 n_guild_num = get_guild_num();
	if(n_guild_num <= 0)
	{
		return;
	}

	INT32 n_message_size = sizeof(NET_DB2C_save_all_guild) + (n_guild_num - 1) * sizeof(tagGuildBase);
	CREATE_MSG(p_send,n_message_size,NET_DB2C_save_all_guild);
	p_send->n_guild_num	= n_guild_num;
	
	INT32 n_message_size2 = sizeof(NET_DB2C_save_all_guild_plant) + (n_guild_num - 1) * sizeof(s_guild_plant);
	CREATE_MSG(p_send2,n_message_size2,NET_DB2C_save_all_guild_plant);
	p_send2->n_guild_num	= n_guild_num;

	INT32 n_index	= 0;
	guild *pGuild	= NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, pGuild))
	{
		p_send->guild_base[n_index] = (tagGuildBase)pGuild->get_guild_att();
		p_send2->guild_plant[n_index].dw_guild_id = pGuild->get_guild_att().dwID;
		pGuild->get_plant_data(p_send2->guild_plant[n_index].s_data);
		n_index++;
		 
	}

	ASSERT(n_index == n_guild_num);
	g_dbSession.Send(p_send, p_send->dw_size);
	g_dbSession.Send(p_send2, p_send2->dw_size);
	MDEL_MSG(p_send);
	MDEL_MSG(p_send2);
}

//-------------------------------------------------------------------------------
//! 发送连线客户端的帮会信息
//-------------------------------------------------------------------------------
VOID guild_manager::send_role_guild_info(Role *p_role_, 
									OUT tagGuildBase &st_guild_base_, 
									OUT tagGuildMember &st_guild_member_)
{
	ASSERT(VALID_POINT(p_role_));

	st_guild_base_.dwID			= INVALID_VALUE;
	st_guild_member_.dw_role_id	= INVALID_VALUE;

	guild *p_guild = get_guild(p_role_->GetGuildID());
	if(!VALID_POINT(p_guild))
	{
		if(map_guild.size() > 0)
		{
			p_role_->SetGuildID(INVALID_VALUE);
			return;
		}
		
		ASSERT(0);
		print_message(_T("\n\n\tguild db init have problem!\n\n"));
		return;
	}

	st_guild_base_ = (const tagGuildBase)p_guild->get_guild_att();
	st_guild_base_.n16MemberNum = p_guild->get_guild_member_num();

	tagGuildMember *p_member = p_guild->get_member(p_role_->GetID());
	if(!VALID_POINT(p_member))
	{
		if(p_guild->get_guild_member_num() > 0)
		{
			p_role_->SetGuildID(INVALID_VALUE);
		}
		else
		{
			ASSERT(0);
			print_message(_T("\n\nguild member load faild!\n\n"));
		}
		
		return;
	}

	st_guild_member_ = *p_member;
}

VOID guild_manager::send_all_guild_info(Role* p_role, BOOL bSBKList)
{
	//guild* pGuild = g_guild_manager.get_guild(p_role->GetGuildID());
	//if(!VALID_POINT(pGuild))
	//	return;

	//if (pGuild->get_guild_att().dwLeaderRoleID != p_role->GetID())
	//	return;
	
	
	INT nIndex = 0;
	INT32 n_message_size = sizeof(NET_SIS_get_all_guild_info) + (g_guild_manager.get_guild_num() - 1) * sizeof(tagGuildWarInfo);

	CREATE_MSG(p_send, n_message_size, NET_SIS_get_all_guild_info);
	p_send->nNum = g_guild_manager.get_guild_num();
	
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
			continue;
		
		if (bSBKList && !p_guild->get_guild_att().bSignUpAttact)
			continue;

		p_send->s_guildInfo[nIndex].dwGuildID = p_guild->get_guild_att().dwID;
		p_send->s_guildInfo[nIndex].byLevel = p_guild->get_guild_att().byLevel;
		p_send->s_guildInfo[nIndex].nMemberNumber = p_guild->get_guild_member().get_member_num();
		p_send->s_guildInfo[nIndex].nApplyLevel = p_guild->get_guild_att().nApplyLevel;
		p_send->s_guildInfo[nIndex].bAttack = p_guild->get_guild_att().bSignUpAttact;
		p_send->s_guildInfo[nIndex].nRank = p_guild->getRank();
		//p_send->s_guildInfo[nIndex].dwDaogaoNum = p_guild->GetDaogaoNumber();
		//p_send->s_guildInfo[nIndex].nFund = p_guild->get_guild_att().nFund;
		//p_send->s_guildInfo[nIndex].nProsperity = p_guild->get_guild_att().nProsperity;
		_tcscpy_s(p_send->s_guildInfo[nIndex].szGuildName, p_guild->get_guild_att().str_name.size() + 1, p_guild->get_guild_att().str_name.c_str());
		s_role_info* pRoleInfo = g_roleMgr.get_role_info(p_guild->get_guild_att().dwLeaderRoleID);
		if (VALID_POINT(pRoleInfo))
		{
			_tcscpy_s(p_send->s_guildInfo[nIndex].szLeaderName, X_SHORT_NAME, pRoleInfo->sz_role_name);
		}
			
		
		nIndex++;
	}

	p_role->SendMessage(p_send, n_message_size);

	MDEL_MSG(p_send);
}
//-------------------------------------------------------------------------------
//! 删除角色时清理帮会数据
//-------------------------------------------------------------------------------
DWORD guild_manager::clear_role_remove(DWORD dw_role_id_)
{
	guild *p_guild = NULL;
	tagGuildMember *p_member = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		p_member = p_guild->get_member(dw_role_id_);
		if(VALID_POINT(p_member))
		{
			return p_guild->clear_role_remove(dw_role_id_);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 客户端请求通用错误处理
//-------------------------------------------------------------------------------
VOID guild_manager::send_guild_failed_to_client(PlayerSession *p_session_, DWORD dw_error_code_)
{
	ASSERT(VALID_POINT(p_session_));
	ASSERT(dw_error_code_ != E_Success);

	if(!VALID_POINT(p_session_)) return;
	NET_SIS_guild_failed_disposal send;
	send.dw_error_code = dw_error_code_;
	p_session_->SendMessage(&send, send.dw_size);
}


VOID guild_manager::send_create_guild_to_db(const s_guild& st_guild_)
{
	INT32 n_name_length = st_guild_.str_name.size();
	INT32 n_message_size = sizeof(NET_DB2C_create_guild) + n_name_length * sizeof(TCHAR);
	
	CREATE_MSG(p_send, n_message_size, NET_DB2C_create_guild);
	
	p_send->create_guild_info_.dw_guild_id			= st_guild_.dwID;
	p_send->create_guild_info_.dw_create_role_name_id	= st_guild_.dwFounderNameID;

	p_send->create_guild_info_.n_guild_rep			= st_guild_.nReputation;
	p_send->create_guild_info_.n_guild_fund			= st_guild_.nFund;
	p_send->create_guild_info_.n_guild_material		= st_guild_.nMaterial;
	p_send->create_guild_info_.n_group_purchase		= st_guild_.nGroupPurchase;

	p_send->create_guild_info_.n16_guild_peace		= st_guild_.n16Peace;
	p_send->create_guild_info_.by_guild_level		= st_guild_.byLevel;
	p_send->create_guild_info_.by_affair_remain_times	= st_guild_.byAffairRemainTimes;

	p_send->create_guild_info_.dw_create_time = g_world.GetWorldTime();

	p_send->create_guild_info_.dw_up_level_time = st_guild_.dwMianzhanTime;
	p_send->create_guild_info_.dw_jujue_time = st_guild_.dwJujueTime;

	p_send->create_guild_info_.dw_spec_state = st_guild_.dwSpecState;

	memcpy(p_send->create_guild_info_.n_family_name, st_guild_.n_family_name, sizeof(p_send->create_guild_info_.n_family_name));
	memcpy(p_send->create_guild_info_.n_name, st_guild_.n_name, sizeof(p_send->create_guild_info_.n_name));

	memcpy(p_send->create_guild_info_.dw_sign_role_id, st_guild_.dwSignRoleID, sizeof(p_send->create_guild_info_.dw_sign_role_id));
	memcpy(p_send->create_guild_info_.dw_enemy_id, st_guild_.dwEnemyID, sizeof(p_send->create_guild_info_.dw_enemy_id));

	_tcscpy_s(p_send->create_guild_info_.sz_name, n_name_length + 1, st_guild_.str_name.c_str());
	memcpy(p_send->create_guild_info_.sz_pos_name,st_guild_.szPosName,sizeof(st_guild_.szPosName));
	memcpy(p_send->create_guild_info_.dw_pos_power,st_guild_.dwPosPower,sizeof(p_send->create_guild_info_.dw_pos_power));
	ASSERT(_T('\0') == p_send->create_guild_info_.sz_name[n_name_length]);

	g_dbSession.Send(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
}


VOID guild_manager::send_dismiss_guild_to_db(DWORD dw_guild_id_)
{
	NET_DB2C_dismiss_guild send;
	send.dw_guild_id = dw_guild_id_;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_manager::send_reset_signUpAttact_to_db()
{
	ET_DB2C_reset_signUpAttact send;
	g_dbSession.Send(&send, send.dw_size);

}

//-------------------------------------------------------------------------------
//! 创建帮会
//-------------------------------------------------------------------------------
DWORD guild_manager::create_guild(Role* p_creator_, LPCTSTR sz_guild_name_, INT32 n_string_length_, INT64 n64ItemID)
{
	ASSERT(VALID_POINT(p_creator_));

	//！判断是否已经有帮会
	if(p_creator_->IsInGuild())
	{
		ASSERT(VALID_POINT(get_guild(p_creator_->GetGuildID())));

		return E_Guild_Create_AlreadyIn;
	}

	//！判断等级
	if(p_creator_->get_level() < g_guild_manager.get_guild_config().nCreateRoleMinLevel)
	{
		return E_Guild_Create_LevelLimit;
	}

	tstring sz_temp_guild_name(sz_guild_name_, n_string_length_);

	//！帮会名称是否重复
	DWORD dw_new_guild_id = g_world.LowerCrc32(sz_temp_guild_name.c_str());
	if(map_guild.is_exist(dw_new_guild_id))
	{
		return E_Guild_Create_NameExist;
	}

	//！判断帮会名称是否合法
	DWORD dw_error_code = Filter::CheckName(sz_temp_guild_name.c_str(), AttRes::GetInstance()->GetVariableLen().nGuildNameMax, 
							AttRes::GetInstance()->GetVariableLen().nGuildNameMin, AttRes::GetInstance()->GetNameFilterWords());
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	tagItem* pItem = p_creator_->GetItemMgr().GetBagItem(n64ItemID);
	if (!VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EISF_CreateGuild)
	{
		return INVALID_VALUE;
	}
	//！判断金钱是否足够
	//INT64 n64_cost = MSilver2Copper(get_guild_config().nGoldCreateNeeded);
	//if(p_creator_->GetCurMgr().GetBagAllSilver() < n64_cost)
	//{
	//	return E_BagSilver_NotEnough;
	//}

	//if(p_creator_->GetItemMgr().GetBagFreeSize() < 1)
	//	return E_Guild_Bag_Full;

	//tagItem* p_item = ItemCreator::Create(EICM_Guild, p_creator_->GetID(), 7300009, 1, TRUE);
	//if(!VALID_POINT(p_item))
	//	return INVALID_VALUE;

	//！ 创建帮会
	guild *p_new_guild = create_guild(p_creator_, sz_temp_guild_name, dw_new_guild_id);
	if(!VALID_POINT(p_new_guild))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}


	//！ 扣除金钱
	//p_creator_->GetCurMgr().DecBagSilverEx(n64_cost, elcid_guild_create);

	map_guild.add(p_new_guild->get_guild_att().dwID, p_new_guild);

	send_create_guild_to_db(p_new_guild->get_guild_att());

	//p_creator_->GetItemMgr().Add2Bag(p_item, 1, elcid_guild_create);	
	p_creator_->GetItemMgr().DelFromBag(n64ItemID, elcid_guild_create, 1);


	p_creator_->SendInitStateGuild();

	// 清除帮会榜数据
	if(g_guild_manager.is_have_recruit(p_creator_->GetID()))
	{
		g_guild_manager.delete_role_from_guild_recruit(p_creator_->GetID());
		NET_C2DB_delete_guild_recruit send;
		send.dw_role_id = p_creator_->GetID();
		g_dbSession.Send(&send, send.dw_size);
	}

	//! 同步到周围玩家
	tagGuildMember* p_member = p_new_guild->get_member(p_creator_->GetID());
	Map*			p_map	= p_creator_->get_map();
	if (VALID_POINT(p_member) && VALID_POINT(p_map))
	{
		NET_SIS_remote_role_guild_info_change send;
		send.dwGuildID		= p_new_guild->get_guild_att().dwID;
		send.dw_role_id		= p_creator_->GetID();
		send.n8GuildPos		= p_member->eGuildPos;

		p_map->send_big_visible_tile_message(p_creator_, &send, send.dw_size);
	}

	p_new_guild->change_guild_to_formal();

	NET_SIS_synchronize_guild_info send1;
	send1.sGuildInfo = (tagGuildBase)p_new_guild->get_guild_att();
	p_creator_->SendMessage(&send1, send1.dw_size);

	if( VALID_POINT( p_creator_->GetScript( ) ) )
	{
		p_creator_->GetScript( )->OnCreateGuild( p_creator_, p_new_guild->get_guild_att().dwID );
	}
	//gx add 2013.7.20 成功创建行会后，全服公告
	TCHAR sz_buff[X_LONG_NAME] = _T("");
	tstring stname = p_new_guild->get_guild_att().str_name;
	_tcsncpy(sz_buff, stname.c_str(), stname.size());

	HearSayHelper::SendMessage(EHST_CREATEGUILD,p_creator_->GetID(),
		INVALID_VALUE,INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, NULL, FALSE, sz_buff, (stname.length() + 1) * sizeof(TCHAR));
	return E_Guild_Create_Success;
}

DWORD	guild_manager::create_guild_gm(Role* p_creator_, LPCTSTR sz_guild_name_, INT32 n_string_length_)
{
	ASSERT(VALID_POINT(p_creator_));

	//！判断是否已经有帮会
	if(p_creator_->IsInGuild())
	{
		ASSERT(VALID_POINT(get_guild(p_creator_->GetGuildID())));

		return E_Guild_Create_AlreadyIn;
	}


	tstring sz_temp_guild_name(sz_guild_name_, n_string_length_);

	//！帮会名称是否重复
	DWORD dw_new_guild_id = g_world.LowerCrc32(sz_temp_guild_name.c_str());
	if(map_guild.is_exist(dw_new_guild_id))
	{
		return E_Guild_Create_NameExist;
	}

	//！判断帮会名称是否合法
	DWORD dw_error_code = Filter::CheckName(sz_temp_guild_name.c_str(), AttRes::GetInstance()->GetVariableLen().nGuildNameMax, 
		AttRes::GetInstance()->GetVariableLen().nGuildNameMin, AttRes::GetInstance()->GetNameFilterWords());
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}


	//！ 创建帮会
	guild *p_new_guild = create_guild(p_creator_, sz_temp_guild_name, dw_new_guild_id);
	if(!VALID_POINT(p_new_guild))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	map_guild.add(p_new_guild->get_guild_att().dwID, p_new_guild);

	send_create_guild_to_db(p_new_guild->get_guild_att());

	NET_SIS_guild_info send;
	send.sGuildInfo = (tagGuildBase)p_new_guild->get_guild_att();

	p_creator_->SendMessage(&send, send.dw_size);

	// 清除帮会榜数据
	if(g_guild_manager.is_have_recruit(p_creator_->GetID()))
	{
		g_guild_manager.delete_role_from_guild_recruit(p_creator_->GetID());
		NET_C2DB_delete_guild_recruit send;
		send.dw_role_id = p_creator_->GetID();
		g_dbSession.Send(&send, send.dw_size);
	}
	

	//! 同步到周围玩家
	tagGuildMember* p_member = p_new_guild->get_member(p_creator_->GetID());
	Map*			p_map	= p_creator_->get_map();
	if (VALID_POINT(p_member) && VALID_POINT(p_map))
	{
		NET_SIS_remote_role_guild_info_change send;
		send.dwGuildID		= p_new_guild->get_guild_att().dwID;
		send.dw_role_id		= p_creator_->GetID();
		send.n8GuildPos		= p_member->eGuildPos;

		p_map->send_big_visible_tile_message(p_creator_, &send, send.dw_size);
	}

	p_new_guild->change_guild_to_formal();

	NET_SIS_synchronize_guild_info send1;
	send.sGuildInfo = (tagGuildBase)p_new_guild->get_guild_att();
	p_creator_->SendMessage(&send1, send1.dw_size);

	if( VALID_POINT( p_creator_->GetScript( ) ) )
	{
		p_creator_->GetScript( )->OnCreateGuild( p_creator_, p_new_guild->get_guild_att().dwID );
	}

	return E_Guild_Create_Success;
}


//-------------------------------------------------------------------------------
//! 创建帮会
//-------------------------------------------------------------------------------
guild* guild_manager::create_guild(Role* p_creator_, const tstring& str_guild_name_, DWORD dw_new_guild_id_)
{
	guild* p_new_guild = new guild();
	if(!VALID_POINT(p_new_guild))
	{
		ASSERT(0);
		return NULL;
	}

	p_new_guild->init_create(p_creator_, str_guild_name_, dw_new_guild_id_, g_guild_manager.get_guild_config());

	
	return p_new_guild;
}

//-------------------------------------------------------------------------------
//! 解散帮会
//-------------------------------------------------------------------------------
DWORD guild_manager::dismiss_guild(Role* p_leader_, DWORD dw_guild_id_)
{
	ASSERT(VALID_POINT(p_leader_));
	ASSERT(p_leader_->GetGuildID() == dw_guild_id_);
	
	guild *p_guild = get_guild(dw_guild_id_);
	if(!VALID_POINT(p_guild))
	{
		return E_Guild_NotExist;
	}

	//! 判断是否可以删除帮会
	DWORD dw_error_code = p_guild->dismiss_guild(p_leader_);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	//！清除帮会团购数据
	g_mall.RemoveGuildPurchaseInfo(dw_guild_id_);

	//！广播
	INT32 n_guild_name_length = p_guild->get_guild_att().str_name.size();
	INT32 n_message_size = sizeof(NET_SIS_guild_dismiss_broad) + n_guild_name_length * sizeof(TCHAR);

	CREATE_MSG(p_send, n_message_size, NET_SIS_guild_dismiss_broad);
	p_send->dw_role_id	= p_leader_->GetID();
	_tcscpy_s(p_send->szGuildName, n_guild_name_length + 1, p_guild->get_guild_att().str_name.c_str());
	g_roleMgr.send_world_msg(p_send, p_send->dw_size);
	MDEL_MSG(p_send);

	Map* p_map	= p_leader_->get_map();
	if (VALID_POINT(p_map))
	{
		NET_SIS_remote_role_guild_info_change send;
		send.dwGuildID		= INVALID_VALUE;
		send.dw_role_id		= p_leader_->GetID();
		send.n8GuildPos		= EGMP_Null;

		p_map->send_big_visible_tile_message(p_leader_, &send, send.dw_size);
	}

	SAFE_DELETE(p_guild);
	map_guild.erase(dw_guild_id_);

	send_dismiss_guild_to_db(dw_guild_id_);
	
	return E_Success;
}


//-------------------------------------------------------------------------------
//! 帮会仓库添加事件
//-------------------------------------------------------------------------------
VOID guild_manager::add_guild_ware_event(DWORD dw_sender_, EEventType e_event_type_, DWORD dw_size_, VOID* p_message_)
{
	Role* p_role = g_roleMgr.get_role(dw_sender_);
	if (!VALID_POINT(p_role))
	{
		return;
	}

	DWORD dw_guild_id = p_role->GetGuildID();
	if (!VALID_VALUE(dw_guild_id))
	{
		return;
	}

	guild* p_guild = get_guild(dw_guild_id);
	if (!VALID_POINT(p_guild))
	{
		return;
	}
	
	p_guild->get_guild_warehouse().AddEvent(dw_sender_, e_event_type_, dw_size_, p_message_);
}

//-------------------------------------------------------------------------------
// 每日重置帮派事务状态并扣除消耗
//-------------------------------------------------------------------------------
VOID guild_manager::daily_guild_reset()
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
		{
			continue;
		}
		p_guild->daily_guild_reset();
		p_guild->get_upgrade().DayDecProsperity();
	}
}

//! 没周清除宣战过的帮会
VOID guild_manager::week_clear_enemy()
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
		{
			continue;
		}
		p_guild->clearEnemyGuild();
	}
}

// 清空帮会圣兽使用次数
VOID guild_manager::clear_guild_holiness_num()
{
	guild* p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if(!VALID_POINT(p_guild))
		{
			continue;
		}
		p_guild->get_upgrade().ClearHolinessNum();
	}
}

//! 刷新帮会免战状态
VOID guild_manager::daily_guild_war_state_reset()
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
		{
			continue;
		}
		p_guild->get_guild_war().reset_avlidance_state();
	}
}

// 刷新帮会鼎使用次数
VOID guild_manager::daily_guild_lobby_use()
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
		{
			continue;
		}
		p_guild->get_upgrade().ResetGuildPracitceUseNum();
	}
}

VOID guild_manager::guild_quest_reset( )
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (VALID_POINT(p_guild)) p_guild->guild_quest_reset( );
	}
}

//-------------------------------------------------------------------------------
// ! 帮会技能升级
//-------------------------------------------------------------------------------
VOID guild_manager::upgrade_guild_skill()
{
	guild *p_guild = NULL;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		if (!VALID_POINT(p_guild))
		{
			continue;
		}
		p_guild->get_guild_skill().skill_upgrade_on_clock();
	}
}

//-------------------------------------------------------------------------------
//! 设置帮会PVP副本ID
//-------------------------------------------------------------------------------
VOID guild_manager::set_pvp_instance_id(DWORD dw_instance_id_, DWORD dw_act_id_)
{
	s_guild_pvp_data* p_guild_pvp_data = map_pvp_data.find(dw_act_id_);
	if(!VALID_POINT(p_guild_pvp_data))
	{
		p_guild_pvp_data = new s_guild_pvp_data;
		p_guild_pvp_data->Init();
		p_guild_pvp_data->n_act_id = dw_act_id_;
		p_guild_pvp_data->dw_instance_id = dw_instance_id_;
		map_pvp_data.add(dw_act_id_, p_guild_pvp_data);
		
		NET_DB2C_insert_pvp_data send;
		send.n_act_id = dw_act_id_;
		memcpy(send.dw_guild_id, p_guild_pvp_data->dw_guild_id, sizeof(send.dw_guild_id));
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		p_guild_pvp_data->dw_instance_id = dw_instance_id_;
	}
}

//-------------------------------------------------------------------------------
//! 初始化帮会pvp数据
//-------------------------------------------------------------------------------
VOID guild_manager::init_pvp_data(s_load_guild_pvp_data *p_load_guild_pvp_data_, INT n_num_)
{
	if(n_num_ <= 0)
		return;

	for(INT i = 0; i < n_num_; i++)
	{
		s_guild_pvp_data* p_guild_pvp_data = map_pvp_data.find(p_load_guild_pvp_data_[i].n_act_id);
		if(VALID_POINT(p_guild_pvp_data))
			continue;

		p_guild_pvp_data = new s_guild_pvp_data;
		p_guild_pvp_data->Init();
		p_guild_pvp_data->n_act_id = p_load_guild_pvp_data_[i].n_act_id;
		memcpy(p_guild_pvp_data->dw_guild_id, p_load_guild_pvp_data_[i].dw_guild_id, sizeof(p_guild_pvp_data->dw_guild_id));
		map_pvp_data.add(p_guild_pvp_data->n_act_id, p_guild_pvp_data);
	}
}

//-------------------------------------------------------------------------------
//! 更新帮会pvp数据
//-------------------------------------------------------------------------------
VOID guild_manager::update_pvp_data(INT n_act_id_)
{
	s_guild_pvp_data* p_guild_pvp_data = map_pvp_data.find(n_act_id_);
	if(!VALID_POINT(p_guild_pvp_data))
		return;

	NET_DB2C_up_pvp_data send;
	send.n_act_id = n_act_id_;
	memcpy(send.dw_guild_id, p_guild_pvp_data->dw_guild_id, sizeof(send.dw_guild_id));
	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------
//初始化帮会招募榜
//-------------------------------------------------------------------------------
VOID guild_manager::load_guild_recruit(DWORD* p_data, INT n_num)
{
	//for(INT i = 0; i < n_num; i++)
	//{
	//	list_guild_recruit.push_front(*p_data);
	//	p_data++;
	//}
}

//-------------------------------------------------------------------------------
//是否在帮会招募榜中
//-------------------------------------------------------------------------------
BOOL guild_manager::is_have_recruit(DWORD dw_role_id, DWORD dwGuildID)
{
	DWORD dw_role_id_ = INVALID_VALUE;
	DWORD dw_guild_id = INVALID_VALUE;
	list_guild_recruit.reset_iterator();
	while(list_guild_recruit.find_next(dw_role_id_, dw_guild_id))
	{
		if(dw_role_id == dw_role_id_)
		{
			if (dwGuildID != INVALID_VALUE && dw_guild_id != dwGuildID)
				return FALSE;
			else
				return TRUE;
		}

	}
	return FALSE;
}

DWORD	guild_manager::get_recruit_guild(DWORD dw_role_id)
{
	return list_guild_recruit.find(dw_role_id);
}
//-------------------------------------------------------------------------------
//加入帮会招募榜
//-------------------------------------------------------------------------------
DWORD guild_manager::join_guild_recruit(Role* pRole, DWORD dwGuildID)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(is_have_recruit(pRole->GetID()))
		return E_Guild_recruit_have_exist;

	if(pRole->GetGuildID() != INVALID_VALUE)
		return E_Guild_Join_AlreadyIn;
	
	guild* pGuild = get_guild(dwGuildID);
	if (!VALID_POINT(pGuild) || pGuild->get_guild_att().nApplyLevel > pRole->get_level())
		return INVALID_VALUE;


	list_guild_recruit.add(pRole->GetID(), dwGuildID);

	//NET_C2DB_insert_guild_recruit send;
	//send.dw_role_id = pRole->GetID();
	//g_dbSession.Send(&send, send.dw_size);

	//send_query_result(pRole, 0, pRole->get_list_guild_recruit());
	if(VALID_POINT(pRole->GetScript()))
		pRole->GetScript()->OnJoinRecruit(pRole);

	return E_Success;
}

//-------------------------------------------------------------------------------
//离开帮会招募榜
//-------------------------------------------------------------------------------
DWORD guild_manager::leave_guild_recruit(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(!is_have_recruit(pRole->GetID()))
		return E_Guild_recruit_nohave_exist;

	DWORD dw_role_id = pRole->GetID();

	list_guild_recruit.erase(dw_role_id);

	//NET_C2DB_delete_guild_recruit send;
	//send.dw_role_id = pRole->GetID();
	//g_dbSession.Send(&send, send.dw_size);

	//send_query_result(pRole, 0, pRole->get_list_guild_recruit());

	return E_Success;
}

//-------------------------------------------------------------------------------
//计算帮会招募榜页数
//-------------------------------------------------------------------------------
VOID guild_manager::cal_page_num(INT n_size, INT& n_page)
{
	if(n_size <= 0)
	{
		n_page = 0;
		return;
	}

	n_page = n_size / 10;
	if(n_page <= 0)
	{
		if((n_size % 10) > 0)
		{
			n_page += 1;
			return;
		}
	}
	else
	{
		if((n_size % (n_page*10)) > 0)
		{
			n_page += 1;
			return;
		}
	}
}

//-------------------------------------------------------------------------------
//发送查询结果
//-------------------------------------------------------------------------------
VOID guild_manager::send_query_result(Role* p_role, INT n_page_num,	package_list<DWORD>& list_query_guild_recruid)
{
	INT n_page = 0;
	cal_page_num(list_query_guild_recruid.size(), n_page);

	if(n_page_num < 0 || n_page_num > n_page)
		return;

	INT n_message_size = sizeof(NET_SIS_query_guild_recruit) + 9 * sizeof(tag_guild_recrguit_info);
	CREATE_MSG(p_send, n_message_size, NET_SIS_query_guild_recruit);

	//p_send->n_page = n_page;
	p_send->n_num = 0;
	//p_send->b_register = is_have_recruit(p_role->GetID());

	M_trans_pointer(p, p_send->p_data, tag_guild_recrguit_info);

	DWORD dw_role_id = INVALID_VALUE;
	INT		n_temp = 0;
	package_list<DWORD>::list_iter iter = list_query_guild_recruid.begin();
	while(list_query_guild_recruid.find_next(iter, dw_role_id))
	{
		//if(n_page > 1)
		//{
		//	if(n_temp < n_page_num*10)
		//	{
		//		n_temp++;
		//		continue;
		//	}
		//}

		s_role_info* p_role_info = g_roleMgr.get_role_info(dw_role_id);
		if(!VALID_POINT(p_role_info))
			continue;

		p->dw_role_id = dw_role_id;
		p->n_level = p_role_info->by_level;
		p->e_class = p_role_info->e_class_type_;
		p->b_line = p_role_info->b_online_;

		p_send->n_num++;
		p++;

		if(p_send->n_num >= 10)
			break;
	}

	if(p_send->n_num > 0)
	{
		p_send->dw_size = sizeof(NET_SIS_query_guild_recruit) + (p_send->n_num-1)*sizeof(tag_guild_recrguit_info);
	}
	else
	{
		p_send->dw_size = sizeof(NET_SIS_query_guild_recruit);
	}

	p_role->SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(p_send);
}

//-------------------------------------------------------------------------------
//查询帮会招募榜
//-------------------------------------------------------------------------------
VOID guild_manager::query_guild_recruit(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;
	
	guild* pGuild = get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return ;


	pRole->get_list_guild_recruit().clear();
	DWORD dw_guild_id = INVALID_VALUE;
	DWORD dw_role_id = INVALID_VALUE;
	list_guild_recruit.reset_iterator();
	while(list_guild_recruit.find_next(dw_role_id, dw_guild_id))
	{
		if(dw_guild_id != pRole->GetGuildID())
			continue;

		pRole->get_list_guild_recruit().push_back(dw_role_id);
	}

	send_query_result(pRole, 0, pRole->get_list_guild_recruit());
}

VOID guild_manager::delete_role_from_guild_recruit(DWORD dw_role_id)
{
	list_guild_recruit.erase(dw_role_id);
}

VOID guild_manager::clean_guild_recruit_to_db()
{
	//NET_C2DB_clean_guild_recruit send;
	//g_dbSession.Send(&send, send.dw_size);

	list_guild_recruit.clear();
}

//获取帮会脚本数据索引为6的帮会id列表
VOID guild_manager::get_script_data_six_list(std::map<int, DWORD>& map)
{
	guild* pGuild = NULL;
	map_guild.reset_iterator();
	while(map_guild.find_next(pGuild))
	{
		INT32 nScriptData = pGuild->get_guild_att().n32ScriptData[6];
		map.insert(make_pair(nScriptData, pGuild->get_guild_att().dwID));
	}
}

VOID guild_manager::load_guild_kill_boss(s_guild_kill_boss* p_kill_boss, INT n_num)
{
	for(INT i = 0; i < n_num; i++)
	{
		map_guild_kill_boss.add(p_kill_boss->dw_monster_id, p_kill_boss->dw_guild_id);
		p_kill_boss++;
	}
}

VOID guild_manager::load_guild_war_history(tagGuildWarHistory* p_war_history, INT n_num)
{
	for (INT i = 0; i < n_num; i++)
	{
		guild* pGuild = get_guild(p_war_history->dw_guild_id);
		if (VALID_POINT(pGuild))
		{
			if (CalcTimeDiff(GetCurrentDWORDTime(), p_war_history->dw_time) > 14 * 24 * 60 * 60)
			{
				NET_C2DB_delete_guild_war_history send;
				send.st_guild_war_history = *p_war_history;

				g_dbSession.Send(&send, send.dw_size);
			}
			else
			{
				pGuild->add_war_history(*p_war_history, false);
			}
		}
		p_war_history++;
	}
}

VOID guild_manager::load_guild_plant(DWORD dwID, tagPlantData* p_plant_data, INT n_num)
{
	guild* pGuild = get_guild(dwID);
	if (VALID_POINT(pGuild))
	{
		pGuild->initPlanData(p_plant_data, n_num);
	}
}

BOOL guild_manager::is_monster_kill(DWORD	dw_monster_id)
{
	DWORD	dw_guild_id = map_guild_kill_boss.find(dw_monster_id);

	if(!VALID_VALUE(dw_guild_id))
	{
		return FALSE;
	}

	return TRUE;
}

VOID guild_manager::add_monster_kill(DWORD dw_monster_id, DWORD dw_guild_id)
{
	map_guild_kill_boss.add(dw_monster_id, dw_guild_id);

	NET_C2DB_insert_guild_skill_boss send;
	send.st_guild_kill_boss.dw_monster_id = dw_monster_id;
	send.st_guild_kill_boss.dw_guild_id = dw_guild_id;
	g_dbSession.Send(&send, send.dw_size);
}

VOID guild_manager::getSBKData( tagGuildSBKData& data )
{
	memcpy(&data, &m_SBK_data, sizeof(data));
}

VOID guild_manager::set_SBK_gulid( DWORD dwGuildID, BOOL bSave )
{
	memset(&m_SBK_data, 0, sizeof(m_SBK_data));

	m_dw_SBK_guild = dwGuildID;

	guild* pSBKGuild = get_guild(m_dw_SBK_guild);
	if (VALID_VALUE(pSBKGuild))
	{
		m_SBK_data.dwData[0] = pSBKGuild->get_guild_att().dwID;
		MAP_GUILD_MEMBER::map_iter iter = pSBKGuild->get_guild_member_map().begin();
		tagGuildMember* p_guild_member = NULL;
		while(pSBKGuild->get_guild_member_map().find_next(iter, p_guild_member))
		{
			if(!VALID_POINT(p_guild_member))
				continue;

			if (p_guild_member->eGuildPos > EGMP_BangZhong && 
				p_guild_member->eGuildPos < EGMP_OFFICER_4)
			{
				m_SBK_data.dwData[p_guild_member->eGuildPos] = p_guild_member->dw_role_id;
			}

		}

	}

	if (bSave)
	{
		NET_C2DB_save_sbk send;
		send.dw_guild_id = dwGuildID;
		g_dbSession.Send(&send, send.dw_size);

	}

}

VOID guild_manager::reset_member_ballot()
{
	guild* p_guild = g_guild_manager.get_guild(m_dw_SBK_guild);
	if (VALID_POINT(p_guild))
	{
		MAP_GUILD_MEMBER::map_iter iter = p_guild->get_guild_member_map().begin();
		tagGuildMember* p_guild_member = NULL;
		while(p_guild->get_guild_member_map().find_next(iter, p_guild_member))
		{
			if(!VALID_POINT(p_guild_member))
				continue;

			if(!p_guild_member->bBallot)
				continue;

			p_guild_member->bBallot = FALSE;

			p_guild->change_member_ballot(p_guild_member->dw_role_id, p_guild_member->bBallot = FALSE);
		}
	}
	
}

VOID guild_manager::resetSBKData()
{
	set_SBK_gulid(0);

	memset(&m_SBK_data, 0, sizeof(m_SBK_data));
}

void guild_manager::onEndSBKWar()
{
	set_begin_SBK(FALSE);
	reset_member_ballot();

	guild *p_guild;
	MAP_GUILD::map_iter iter = map_guild.begin();
	while(map_guild.find_next(iter, p_guild))
	{
		p_guild->setSignUpAttack(false);

		
	}

	send_reset_signUpAttact_to_db();

	guild* pGuild = get_guild(get_SBK_guild());
	if (VALID_POINT(pGuild))
	{
		//发送公告
		TCHAR sz_buff[X_LONG_NAME] = _T("");
		tstring stname = pGuild->get_guild_att().str_name;
		_tcsncpy(sz_buff, stname.c_str(), stname.size());

		HearSayHelper::SendMessage(EHST_GUILDFIRSTKILL,
			INVALID_VALUE, get_SBK_guild(),  1, INVALID_VALUE, INVALID_VALUE, NULL, FALSE, sz_buff, (stname.length() + 1) * sizeof(TCHAR));

		pGuild->setSignUpAttack(true ,true);

		//pGuild->save_guild_att_to_db();
	}
}

//BOOL guild_manager::is_begin_SBK()
//{
//	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(16);
//	if (VALID_VALUE(pActivity))
//	{
//		return pActivity->is_start();
//	}
//
//	return false;
//}
