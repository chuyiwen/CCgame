

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
*	@file		guild.cpp
*	@author		mmz
*	@date		2010/10/12	initial
*	@version	0.0.1.0
*	@brief		
*/


#include "StdAfx.h"

#include "guild.h"
#include "role.h"
#include "role_mgr.h"
#include "guild_manager.h"
#include "guild_warehouse.h"
#include "guild_upgrade.h"
#include "map_creator.h"
#include "map_instance_guild.h"
#include "mall.h"
#include "title_mgr.h"
#include "mail_mgr.h"

#include "../common/ServerDefine/guild_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"

#include "../../common/WorldDefine/guild_protocol.h"
#include "../../common/WorldDefine/mail_define.h"
#include "../../common/WorldDefine/vip_define.h"
#include "hearSay_helper.h"

const tagGuildPower		guild::null_guild_power;

//-------------------------------------------------------------------------------
//！ 获取帮会职位权限
//-------------------------------------------------------------------------------
const tagGuildPower& guild::get_guild_power(EGuildMemberPos e_guild_position_) const
{
	if(!::IsGuildPosValid(e_guild_position_))
	{
		ASSERT(0);
		print_message(_T("\n\n\nguild power invilad\n\n"));
		return guild::null_guild_power;
	}

	return *(tagGuildPower *) &guild_att.dwPosPower[e_guild_position_];
}

//-------------------------------------------------------------------------------
// 构造&析构
//-------------------------------------------------------------------------------
guild::guild()
: b_delete(FALSE),dw_guild_tripod_id(INVALID_VALUE),dw_tripod_map_id(INVALID_VALUE),m_nRank(INVALID_VALUE)
{
	memset(&m_plantData, 0, sizeof(m_plantData));
}

guild::guild(const s_guild_load *p_guild_load_)
{
	dw_guild_tripod_id = INVALID_VALUE;
	dw_tripod_map_id = INVALID_VALUE;
	dw_lost_tripod_time = INVALID_VALUE;
	m_nRank = INVALID_VALUE;
	_guild_position.init(p_guild_load_->byLevel);
	_member_manager.init(p_guild_load_->dwID);

	guild_att.dwID				= p_guild_load_->dwID;
	guild_att.dwFounderNameID	= p_guild_load_->dwFounderNameID;
	guild_att.dwLeaderRoleID	= p_guild_load_->dwLeaderRoleID;

	memcpy(guild_att.szPosName,p_guild_load_->szPosName,sizeof(guild_att.szPosName));
	memcpy(guild_att.dwPosPower,p_guild_load_->dwPosPower,sizeof(guild_att.dwPosPower));

	memcpy(guild_att.n16DKP, p_guild_load_->n16DKP, sizeof(guild_att.n16DKP));
	memcpy(guild_att.n32ScriptData, p_guild_load_->n32ScriptData, sizeof(guild_att.n32ScriptData));
	memcpy(guild_att.szText, p_guild_load_->szText, sizeof(guild_att.szText));

	memcpy(guild_att.n_family_name, p_guild_load_->n_family_name, sizeof(guild_att.n_family_name));
	memcpy(guild_att.n_name, p_guild_load_->n_name, sizeof(guild_att.n_name));
	//memcpy(guild_att.bDaogao, p_guild_load_->bDaogao, sizeof(guild_att.bDaogao));

	guild_att.bSignUpAttact = p_guild_load_->bSignUpAttact;

	guild_att.dwValue = p_guild_load_->dwValue;

	guild_att.dwSpecState		= p_guild_load_->dwSpecState;
	_guild_state.SetState((EGuildSpecState)guild_att.dwSpecState);

	guild_att.byLevel			= p_guild_load_->byLevel;

	guild_att.bFormal           = p_guild_load_->bFormal;	
	guild_att.bySignatoryNum	= p_guild_load_->bySignatoryNum;
	
	guild_att.nFund				= p_guild_load_->nFund;
	guild_att.nMaterial			= p_guild_load_->nMaterial;
	guild_att.nReputation		= p_guild_load_->nReputation;
	guild_att.n16Peace			= p_guild_load_->n16Peace;
	guild_att.n16MemberNum		= p_guild_load_->n16MemberNum;
	guild_att.n16Rank			= p_guild_load_->n16Rank;
	guild_att.byMinJoinLevel	= p_guild_load_->byMinJoinLevel;
	guild_att.nDailyCost		= p_guild_load_->nDailyCost;
	guild_att.nGroupPurchase	= p_guild_load_->nGroupPurchase;
	guild_att.byAffairRemainTimes = p_guild_load_->byAffairRemainTimes;
	guild_att.bCommendation		= p_guild_load_->bCommendation;
	guild_att.nProsperity = p_guild_load_->nProsperity;

	guild_att.str_name			= p_guild_load_->sz_name;
	guild_att.str_tenet			= p_guild_load_->sz_tenet;
	guild_att.str_symbol		= p_guild_load_->sz_symbol;
	//memcpy(guild_att.bySymbol, pGuildLoad->bySymbolURL, sizeof(guild_att.bySymbol));
	
	for(INT32 i=0; i<MAX_GUILD_HOLDCITY; ++i)
	{
		guild_att.byHoldCity[i]	= p_guild_load_->byHoldCity[i];
	}

	for(INT32 i = 0; i < MAX_GUILD_SIGN_NUM; i++)
	{
		guild_att.dwSignRoleID[i] = p_guild_load_->dwSignRoleID[i];
	}

	for(INT32 i = 0; i < MAX_ENEMY_NUM; i++)
	{
		guild_att.dwEnemyID[i] = p_guild_load_->dwEnemyID[i];
	}

	guild_att.dwCreateTime = p_guild_load_->dwCreateTime;

	guild_att.dwLeagueID = p_guild_load_->dwLeagueID;

	guild_att.dwUnLeagueBeginTime = p_guild_load_->dwUnLeagueBeginTime;

	guild_att.dwMianzhanTime = p_guild_load_->dwMianzhanTime;
	guild_att.dwJujueTime = p_guild_load_->dwJujueTime;

	//guild_att.dwChangeSymbolTime = pGuildLoad->dwChangeSymbolTime;

	set_guild_instance_id(INVALID_VALUE);

	//！ 帮会设施初始化
	_guild_upgrade.Init(this, TRUE);

	//！ 帮会帮务初始化
	_guild_affair.init(this);

	//！ 帮会技能初始化
	_guild_skill.init(this, TRUE);

	//！ 帮会跑商初始化
	_guild_commerce.init(this, TRUE);

	//！ 帮会仓库初始化
	_guild_warehouse.init(this, TRUE);

	//！ 帮会战争
	_guild_war.init(this);

	//！ 帮会弹劾
	_guild_delate.init(this, TRUE);

	set_delete(FALSE);
}

guild::~guild()
{
}

//-------------------------------------------------------------------------------
//! 初始化数据库传来的帮会成员
//-------------------------------------------------------------------------------
VOID guild::init_db_guild_member(const tagGuildMember& st_guild_member_)
{
	if(load_member(st_guild_member_) != E_Success)
	{
		ASSERT(0);
		return;
	}

	tagGuildMember *p_member = get_member(st_guild_member_.dw_role_id);
	ASSERT(VALID_POINT(p_member) && st_guild_member_.eGuildPos == p_member->eGuildPos);
	
	*p_member = st_guild_member_;
}

VOID guild::update()
{
	if(!map_invite.empty())
	{
		DWORD dwInviteeID = INVALID_VALUE;
		s_invite *pInviter = NULL;
		MAP_INVITE::map_iter iter = map_invite.begin();
		while(map_invite.find_next(iter, dwInviteeID, pInviter))
		{
			if(--pInviter->n_invite_count_down <= 0)
			{
				map_invite.erase(dwInviteeID);
				delete pInviter;
			}
		}
	}

	_guild_warehouse.Update();

	_guild_war.update();

	_guild_upgrade.UpDate();
}


BOOL guild::is_guild_init_ok()
{
	BOOL bRet = TRUE;

	if(get_guild_member_num() <= 0)
	{
		ASSERT(0);
		SI_LOG->write_log(_T("帮会成员数据库获取失败! id=<%u>\r\n"), guild_att.dwID);
		bRet = FALSE;
	}

	//if (!_guild_affair.is_init_ok())
	//{
	//	ASSERT(0);
	//	SI_LOG->write_log(_T("帮会事务初始化失败! id=<%u>\r\n"), guild_att.dwID);
	//	bRet = FALSE;
	//}

	if (!_guild_upgrade.is_init_ok())
	{
		ASSERT(0);
		SI_LOG->write_log(_T("帮会设施初始化失败! id=<%u>\r\n"), guild_att.dwID);
		bRet = FALSE;
	}

	/*if (!_guild_skill.is_init_ok())
	{
		ASSERT(0);
		SI_LOG->write_log(_T("帮会技能初始化失败! id=<%u>\r\n"), guild_att.dwID);
		bRet = FALSE;
	}*/

	/*if (!_guild_commerce.is_init_ok())
	{
		ASSERT(0);
		SI_LOG->write_log(_T("帮会跑商初始化失败! id=<%u>\r\n"), guild_att.dwID);
		bRet = FALSE;
	}*/

	/*if (!_guild_warehouse.is_init_ok())
	{
		ASSERT(0);
		SI_LOG->write_log(_T("帮会仓库初始化失败! id=<%u>\r\n"), guild_att.dwID);
		bRet = FALSE;
	}*/

	/*if(! _guild_delate.is_init_ok())
	{
		ASSERT(0);
		SI_LOG->write_log(_T("帮会弹劾初始化失败! id=<%u>\r\n"), guild_att.dwID);
		bRet = FALSE;*/
	/*}*/

	return bRet;
}

//-------------------------------------------------------------------------------
// ！初始化帮会
//-------------------------------------------------------------------------------
VOID guild::init_create(Role* p_creator_, const tstring& str_guild_name_, \
						DWORD dw_guild_id_, const tagGuildCfg &st_guild_config_)
{
	guild_att.Init(str_guild_name_, dw_guild_id_, p_creator_->GetID(), p_creator_->GetNameID(), 
				st_guild_config_.byGuildLevelBegin, st_guild_config_.n16GuildPeaceBegin, 
				st_guild_config_.nGuildFundBegin, st_guild_config_.nGuildMaterialBegin, 
				st_guild_config_.nGuildRepBegin, st_guild_config_.nGuildGroupPurchaseBegin,
				st_guild_config_.nGuildProsperityBegin);

	for(INT i = 0; i < 6; i++)
	{
		guild_att.n_family_name[i] = get_tool()->rand_in_range(g_guild_manager.get_guild_config().nGuildMinNpcNameNum, g_guild_manager.get_guild_config().nGuildMaxNpcNameNum);
		guild_att.n_name[i] = get_tool()->rand_in_range(g_guild_manager.get_guild_config().nGuildMinNpcNameNum, g_guild_manager.get_guild_config().nGuildMaxNpcNameNum);
	}

	_guild_position.init(guild_att.byLevel);
	_guild_position.init_position_name(&guild_att);
	_guild_position.init_position_power(&guild_att);
	_member_manager.init(dw_guild_id_);

	//！ 设置帮会创建者
	add_member(p_creator_->GetID(), EGMP_BangZhu);
	p_creator_->SetGuildID(dw_guild_id_);

	set_guild_instance_id(INVALID_VALUE);

	//！ 初始化帮会设施
	_guild_upgrade.Init(this);
	_guild_upgrade.CreateFacilities();

	//！初始化帮会帮务
	_guild_affair.init(this);

	//！初始化帮会技能
	_guild_skill.init(this);
	_guild_skill.create_guild_skill();

	//！初始化帮会跑商
	_guild_commerce.init(this);

	//！初始化帮会仓库
	_guild_warehouse.init(this);

	//！初始化帮会战争
	_guild_war.init(this);

	//！初始化帮会弹劾
	_guild_delate.init(this);
	_guild_delate.create_guild_delate();

}

VOID guild::initPlanData(tagPlantData* pPlantData, int nSize)
{
	if (nSize > MAX_PLANT_NUMBER)
		return;

	memcpy(&m_plantData, pPlantData, sizeof(tagPlantData) * nSize);

}
VOID guild::setPlantData(const tagPlantData& data, int nIndex)
{
	if (nIndex > MAX_PLANT_NUMBER)
		return;

	memcpy(&m_plantData[nIndex], &data, sizeof(tagPlantData));

}

VOID guild::setPlantData(DWORD dwPlantID, DWORD dwRoleID, int nNum, int nIndex)
{
	if (nIndex > MAX_PLANT_NUMBER)
		return;

	m_plantData[nIndex].dw_master_id = dwRoleID;
	m_plantData[nIndex].dw_npc_id = dwPlantID;
	m_plantData[nIndex].n_num = nNum;
	m_plantData[nIndex].dw_plant_time = g_world.GetWorldTime();
}

VOID guild::resetPlantData(int nIndex)
{
	if (nIndex > MAX_PLANT_NUMBER)
		return;

	memset(&m_plantData[nIndex], 0, sizeof(tagPlantData));
}

tagPlantData* guild::getPlantData(int nIndex)
{
	if (nIndex > MAX_PLANT_NUMBER)
		return NULL;

	return &m_plantData[nIndex];
}
VOID guild::dismiss_guild()
{
	MAP_GUILD_MEMBER::map_iter iter = _member_manager.get_member_map().begin();
	tagGuildMember* pGuildMember = NULL;
	while(_member_manager.get_member_map().find_next(iter, pGuildMember))
	{
		if(!VALID_POINT(pGuildMember))
			continue;

		Role* pRole = g_roleMgr.get_role(pGuildMember->dw_role_id);
		if(VALID_POINT(pRole))
		{
			pRole->SetGuildID(INVALID_VALUE);

			pRole->SetLeaveGuildTime();

			_guild_commerce.abandon_commerce(pRole->GetID(), TRUE);

			Map* pMap	= pRole->get_map();
			if (VALID_POINT(pMap))
			{
				NET_SIS_remote_role_guild_info_change send;
				send.dwGuildID		= INVALID_VALUE;
				send.dw_role_id		= pRole->GetID();
				send.n8GuildPos		= EGMP_Null;

				pMap->send_big_visible_tile_message(pRole, &send, send.dw_size);
			}
		}

		remove_member(pGuildMember->dw_role_id, pGuildMember->eGuildPos);
	}

	//！清空帮会仓库
	_guild_warehouse.remove_all_items();

	//！ 清除帮会设施信息
	_guild_upgrade.RemoveAllFacilitiesInfo();

	//！ 清除帮会技能信息
	_guild_skill.remove_guild_skill_info();

	//！ 清除帮会弹劾信息
	_guild_delate.remove_guild_delate();

	//！ 清除帮会团购信息
	g_mall.RemoveGuildPurchaseInfo(guild_att.dwID);

	//！ 清除帮会战信息
	_guild_war.remove_guild_war();

	NET_DB2C_dismiss_guild send;
	send.dw_guild_id = guild_att.dwID;

	g_dbSession.Send(&send, send.dw_size);

	DWORD dwGuildInstanceID = get_guild_instance_id();
	if( VALID_POINT(dwGuildInstanceID) )
	{
		map_instance_guild* pGuildInstance = get_guild_map();
		if( VALID_POINT(pGuildInstance) )
		{
			pGuildInstance->on_guild_dismiss(this);
		}
	}

	if(guild_att.bFormal)
	{
		INT32 nGuildNameCnt = get_guild_att().str_name.size();
		INT32 nMsgSz = sizeof(NET_SIS_guild_dismiss_broad) + nGuildNameCnt * sizeof(TCHAR);

		CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_dismiss_broad);
		pSend->dw_role_id	= get_guild_att().dwLeaderRoleID;
		_tcscpy_s(pSend->szGuildName, nGuildNameCnt + 1, get_guild_att().str_name.c_str());
		g_roleMgr.send_world_msg(pSend, pSend->dw_size);
		MDEL_MSG(pSend);

	}
	
	set_delete(TRUE);
}

//-------------------------------------------------------------------------------
//! 解散帮会
//-------------------------------------------------------------------------------
DWORD guild::dismiss_guild(Role* p_role_)
{
	ASSERT(VALID_POINT(p_role_));

	//! 获取帮派成员数据
	tagGuildMember *p_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}

	//! 是否有解散帮会权限
	if(!get_guild_power(p_member->eGuildPos).bDismissGuild)
	{
		return E_Guild_Power_NotEnough;
	}

	//! 是否还存在其他成员
	if(_member_manager.get_member_num() > 1)
	{
		return E_Guild_MemberHasMore;
	}

	//！清除成员
	remove_member(p_role_->GetID(), p_member->eGuildPos);

	//！设置角色帮会id
	p_role_->SetGuildID(INVALID_VALUE);

	//！ 设置角色离开帮会时间
	p_role_->SetLeaveGuildTime();

	//！ 清空帮会仓库
	_guild_warehouse.remove_all_items();

	//！ 清除帮会设施
	_guild_upgrade.RemoveAllFacilitiesInfo();

	//！ 清除帮会技能
	_guild_skill.remove_guild_skill_info();

	//！ 清除帮会跑商
	_guild_commerce.abandon_commerce(p_role_->GetID(), TRUE);

	//！ 清除帮会弹劾信息
	_guild_delate.remove_guild_delate();

	//！ 清空帮会副本
	DWORD dw_guild_instance_id = get_guild_instance_id();
	if( VALID_POINT(dw_guild_instance_id) )
	{	
		map_instance_guild* p_guild_instance = get_guild_map();
		if( VALID_POINT(p_guild_instance) )
		{
			p_guild_instance->on_guild_dismiss(this);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 能否邀请成员 -- b_insert_：== TRUE，放入待加入列表；==FALSE，从列表中删除
//-------------------------------------------------------------------------------
DWORD guild::can_invite_join(DWORD dw_inviter_id_, DWORD dw_invitee_id_, Role **pp_invitee_, BOOL b_insert_)
{
	//! 邀请人
	tagGuildMember *p_inviter = get_member(dw_inviter_id_);
	if(!VALID_POINT(p_inviter))
	{
		return INVALID_VALUE;
	}

	////! 职位判断
	//if(!get_guild_power(p_inviter->eGuildPos).bInviteJoin)
	//{
	//	return E_Guild_Power_NotEnough;
	//}
	if (p_inviter->eGuildPos == EGMP_BangZhong)
	{
		return E_Guild_Power_NotEnough;
	}

	//！是否到了帮会人数上限 
	if(_guild_position.get_number() >= get_member_full())
	{
		return E_Guild_Member_Full;
	}

	//！ 被邀请者
	Role *p_invitee = g_roleMgr.get_role(dw_invitee_id_);
	if(!VALID_POINT(p_invitee))
	{
		return E_Role_Not_Online;
	}

	//DWORD dw_calculate_time = CalcTimeDiff(g_world.GetWorldTime(), p_invitee->GetLeaveGuildTime());
	//if(dw_calculate_time > 0)
	//{
	//	if(dw_calculate_time < LEAVE_GUILD_TIME_LIMIT)
	//		return E_Guild_LeaveTime_Limit;
	//}

	//! 被邀请人是否已加入帮会
	if(p_invitee->IsInGuild())
	{
		ASSERT(VALID_POINT(g_guild_manager.get_guild(p_invitee->GetGuildID())));

		return E_Guild_Join_AlreadyIn;
	}

	//! 被邀请人等级是否满足
	if(p_invitee->get_level() < g_guild_manager.get_guild_config().nJoinRoleMinLevel)
	{
		return E_Guild_Join_LevelLimit;
	}

	////！ 放入待加入列表
	//if(b_insert_)
	//{
	//	if(map_invite.is_exist(dw_invitee_id_))
	//	{
	//		return E_Guild_Join_BeInvited;
	//	}
	//	
	//	s_invite *p_invite = new s_invite(dw_inviter_id_);
	//	ASSERT(VALID_POINT(p_invite));
	//	map_invite.add(dw_invitee_id_, p_invite);
	//}
	//else
	//{
	//	if(!map_invite.is_exist(dw_invitee_id_))
	//	{
	//		return E_Guild_Join_NotBeInvited;
	//	}

	//	s_invite *p_inviter = map_invite.find(dw_invitee_id_);
	//	ASSERT(VALID_POINT(p_inviter));
	//	delete p_inviter;
	//	
	//	map_invite.erase(dw_invitee_id_);
	//}

	if(pp_invitee_ != NULL)
	{
		*pp_invitee_ = p_invitee;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 删除指定的邀请帖
//-------------------------------------------------------------------------------
BOOL guild::erase_invite_join(DWORD dw_invitee_id_)
{
	return map_invite.erase(dw_invitee_id_);
}

//-------------------------------------------------------------------------------
//! 添加帮会成员
//-------------------------------------------------------------------------------
DWORD guild::invite_join(DWORD dw_inviter_id_, DWORD dw_invitee_id_)
{
	Role *p_invitee = NULL;

	DWORD dw_error_code = can_invite_join(dw_inviter_id_, dw_invitee_id_, &p_invitee, FALSE);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	//! 添加到帮会成员列表，设置职位为帮众
	add_member(dw_invitee_id_, EGMP_BangZhong);

	//！设置角色帮会id
	p_invitee->SetGuildID(guild_att.dwID);

	DWORD dw_guild_instance_id = get_guild_instance_id();
	if( VALID_POINT(dw_guild_instance_id) )
	{
		map_instance_guild* p_guild_instance = get_guild_map();
		if( VALID_POINT(p_guild_instance) )
		{
			p_guild_instance->on_role_enter_guild(dw_invitee_id_, this);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 担任帮会职位
//-------------------------------------------------------------------------------
DWORD guild::appoint(DWORD dw_appointor_id_, DWORD dw_appointee_id_, EGuildMemberPos e_guild_position_)
{
	tagGuildMember *p_appointor = get_member(dw_appointor_id_);
	if(!VALID_POINT(p_appointor))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	tagGuildMember *p_appointee = get_member(dw_appointee_id_);
	if(!VALID_POINT(p_appointee))
	{
		return E_Guild_MemberNotIn;
	}

	if(is_in_guild_state(EGDSS_Delate))
	{
		if(dw_appointee_id_ == get_delate().get_guild_delate().dwInitiateRoleID)
			return E_Guild_ChangePos_DelateRole;
	}

	//! 如果职位相同
	if(p_appointee->eGuildPos == e_guild_position_)
	{
		return E_Guild_Appoint_SamePos;
	}

	//！ 判断职位权限
	if(!get_guild_power(p_appointor->eGuildPos).bAppointMember)
	{
		return E_Guild_Power_NotEnough;
	}

	//! 授予的职位不能比自己的职位高
	if(e_guild_position_!=EGMP_BangZhong && e_guild_position_ < p_appointor->eGuildPos)
	{
		return E_Guild_Power_NotEnough;
	}
	//！不能向平级的和比自己职位高的人授予职位
	if(p_appointee->eGuildPos!=EGMP_BangZhong &&  p_appointee->eGuildPos <= p_appointor->eGuildPos)
	{
		return E_Guild_Power_NotEnough;
	}


	//！检查此职位是否已满
	if(_guild_position.is_position_full(e_guild_position_))
	{
		return E_Guild_Pos_Full;
	}

	//！任命职位
	return change_guild_position(p_appointee, e_guild_position_);
}

//-------------------------------------------------------------------------------
//!  踢出帮会成员
//-------------------------------------------------------------------------------
DWORD guild::kick_member(DWORD dw_role_id_, DWORD dw_kick_id_)
{
	tagGuildMember *p_member = get_member(dw_role_id_);
	if(!VALID_POINT(p_member))
	{
		return INVALID_VALUE;
	}

	tagGuildMember *p_kicked = get_member(dw_kick_id_);
	if(!VALID_POINT(p_kicked))
	{
		return INVALID_VALUE;
	}

	if(is_in_guild_state(EGDSS_Delate))
	{
		if(dw_kick_id_ == get_delate().get_guild_delate().dwInitiateRoleID)
			return E_Guild_Kick_DelateRole;
	}

	if (get_guild_att().dwLeaderRoleID == dw_kick_id_)
		return E_Guild_Power_NotEnough;

	//! 判断职位权限
	if(!get_guild_power(p_member->eGuildPos).bKickMember)
	{
		return E_Guild_Power_NotEnough;
	}

	//！删除帮会成员
	remove_member(dw_kick_id_, p_kicked->eGuildPos);

	//！ 如果角色在线
	Role *p_role_kicled = g_roleMgr.get_role(dw_kick_id_);
	if(VALID_POINT(p_role_kicled))
	{
		p_role_kicled->SetGuildID(INVALID_VALUE);

		//! 设置角色离开帮会时间
		p_role_kicled->SetLeaveGuildTime();
	}

	DWORD dw_guild_instance_id = get_guild_instance_id();
	if( VALID_POINT(dw_guild_instance_id) )
	{
		map_instance_guild* p_guild_instance = get_guild_map();
		if( VALID_POINT(p_guild_instance) )
		{
			p_guild_instance->on_role_leave_guild(dw_kick_id_, this);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 转交帮主
//-------------------------------------------------------------------------------
DWORD guild::change_leader(DWORD dw_old_leader_id_, DWORD dw_new_leader_id_)
{
	tagGuildMember *p_old_leader = get_member(dw_old_leader_id_);
	if(!VALID_POINT(p_old_leader))
	{
		return INVALID_VALUE;
	}

	tagGuildMember *p_new_leader = get_member(dw_new_leader_id_);
	if(!VALID_POINT(p_new_leader))
	{
		return INVALID_VALUE;
	}

	if(is_in_guild_state(EGDSS_Delate))
		return E_Guild_State_Delate;

	if (p_old_leader->eGuildPos != EGMP_BangZhu)
	{
		return E_Guild_Power_NotEnough;
	}
	//! 判断职位权限
	//if(!get_guild_power(p_old_leader->eGuildPos).bTurnoverLeader)
	//{
	//	return E_Guild_Power_NotEnough;
	//}
	
	change_guild_position(p_old_leader, EGMP_BangZhong);
	change_guild_position(p_new_leader, EGMP_BangZhu);

	// ! 设置新帮主
	set_guild_bangzhu(dw_new_leader_id_);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 辞去职位
//-------------------------------------------------------------------------------
DWORD guild::demiss_postion(DWORD dw_role_id_, OUT INT8 &n_old_guild_pos_)
{
	tagGuildMember *p_member = get_member(dw_role_id_);
	if(!VALID_POINT(p_member))
	{
		return INVALID_VALUE;
	}

	//！判断职位权限
	if(!get_guild_power(p_member->eGuildPos).bDemissPostion)
	{
		return E_Guild_CanNot_Demiss;
	}

	n_old_guild_pos_ = p_member->eGuildPos;
	
	//! 辞去职务
	change_guild_position(p_member, EGMP_BangZhong);
	
	return E_Success;
}

//-------------------------------------------------------------------------------
//! 离开帮会
//-------------------------------------------------------------------------------
DWORD guild::leave_guild(Role* p_role_)
{
	ASSERT(VALID_POINT(p_role_));

	tagGuildMember *p_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_member))
	{
		return INVALID_VALUE;
	}

	if (p_member->eGuildPos == EGMP_BangZhu)
	{
		return E_Guild_CanNot_Leave;
	}
	//! 判断职位权限
	//if(!get_guild_power(p_member->eGuildPos).bLeaveGuild)
	//{
	//	return E_Guild_CanNot_Leave;
	//}

	//！ 设置角色帮会id
	p_role_->SetGuildID(INVALID_VALUE);

	//！ 设置角色离开帮会时间
	p_role_->SetLeaveGuildTime();
	
	//！ 删除成员
	remove_member(p_role_->GetID(), p_member->eGuildPos);

	DWORD dw_guild_instance_id = get_guild_instance_id();
	if( VALID_POINT(dw_guild_instance_id) )
	{
		map_instance_guild* p_guild_instance = get_guild_map();
		if( VALID_POINT(p_guild_instance) )
		{
			p_guild_instance->on_role_leave_guild(p_role_->GetID(), this);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//！ 修改帮会职位名称
//-------------------------------------------------------------------------------
DWORD guild:: modify_postion_name(DWORD dw_role_id_, TCHAR * sz_position_name_)
{
	tagGuildMember *p_member = get_member(dw_role_id_);
	if(!VALID_POINT(p_member))
	{
		return INVALID_VALUE;
	}

	if(p_member->eGuildPos!=EGMP_BangZhu)
	{
		return E_Guild_Power_NotEnough;
	}

	if(!get_guild_power(p_member->eGuildPos).bModifyPosName)
	{
		return E_Guild_Power_NotEnough;
	}

	memcpy((void *)&guild_att.szPosName[0][0],sz_position_name_,sizeof(guild_att.szPosName));

	INT32 n_message_size = sizeof(NET_DB2C_change_guild_pos_name) ;

	CREATE_MSG(p_send, n_message_size, NET_DB2C_change_guild_pos_name);
	p_send->dw_guild_id = guild_att.dwID;
	memcpy((void *)&p_send->sz_pos_name[0][0],sz_position_name_,sizeof(p_send->sz_pos_name));

	g_dbSession.Send(p_send, p_send->dw_size);

	MDEL_MSG(pSend);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 修改帮会职位权限
//-------------------------------------------------------------------------------
DWORD guild:: modify_postion_power(DWORD dw_role_id_, DWORD * p_position_power_)
{
	tagGuildMember *pMember = get_member(dw_role_id_);
	if(!VALID_POINT(pMember))
	{
		return INVALID_VALUE;
	}

	if(pMember->eGuildPos!=EGMP_BangZhu)
	{
		return E_Guild_Power_NotEnough;
	}

	if(!get_guild_power(pMember->eGuildPos).bModifyPosPower)
	{
		return E_Guild_Power_NotEnough;
	}


	memcpy((void *)&guild_att.dwPosPower[0],p_position_power_,sizeof(guild_att.dwPosPower));

	INT32 n_message_size = sizeof(NET_DB2C_change_guild_pos_power) ;

	CREATE_MSG(p_send, n_message_size, NET_DB2C_change_guild_pos_power);
	p_send->dw_guild_id = guild_att.dwID;
	memcpy((void *)&p_send->dw_pos_power[0],p_position_power_,sizeof(p_send->dw_pos_power));

	g_dbSession.Send(p_send, p_send->dw_size);

	MDEL_MSG(p_send);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 修改帮会标志
//-------------------------------------------------------------------------------
DWORD guild::modify_symbol(DWORD dw_role_id_, LPCTSTR str_tenet_, INT32 n_string_length_)
{
	tagGuildMember *pMember = get_member(dw_role_id_);
	if(!VALID_POINT(pMember))
	{
		return INVALID_VALUE;
	}

	//! 判断职位权限
	//if(!get_guild_power(pMember->eGuildPos).bModifySgin)
	//{
	//	return E_Guild_Power_NotEnough;
	//}
	if (pMember->eGuildPos == EGMP_BangZhong)
	{
		return E_Guild_Power_NotEnough;
	}
	
	tstring str(str_tenet_, n_string_length_);

	if(guild_att.str_symbol == str)
	{
		return E_Return_NotNeedFeed;
	}

	if(AttRes::GetInstance()->GetVariableLen().nGuildTenet < n_string_length_)
	{
		return E_Filter_Text_TooLong;
	}

	guild_att.str_symbol = str;

	INT32 n_message_size = sizeof(NET_DB2C_change_guild_symbol) + n_string_length_ * sizeof(TCHAR);

	CREATE_MSG(pSend, n_message_size, NET_DB2C_change_guild_symbol);
	pSend->dw_guild_id = guild_att.dwID;
	memcpy(pSend->by_guild_symbol, str_tenet_, n_string_length_ * sizeof(TCHAR));
	pSend->by_guild_symbol[n_string_length_] = _T('\0');

	g_dbSession.Send(pSend, pSend->dw_size);

	MDEL_MSG(pSend);


	return E_Success;
}

//-------------------------------------------------------------------------------
//! 修改帮会公告
//-------------------------------------------------------------------------------
DWORD guild::modify_tenet(Role* pRole, LPCTSTR str_tenet_, INT32 n_string_length_)
{
	tagGuildMember *p_member = get_member(pRole->GetID());
	if(!VALID_POINT(p_member))
	{
		return INVALID_VALUE;
	}

	//! 判断职位权限
	//if(!get_guild_power(p_member->eGuildPos).bModifyTenet)
	//{
	//	return E_Guild_Power_NotEnough;
	//}
	
	if (p_member->eGuildPos != EGMP_BangZhu && p_member->eGuildPos != EGMP_FuBangZhu)
	{
		return E_Guild_Power_NotEnough;
	}

	tstring str(str_tenet_, n_string_length_);

	if(guild_att.str_tenet == str)
	{
		return E_Return_NotNeedFeed;
	}

	if(AttRes::GetInstance()->GetVariableLen().nGuildTenet < n_string_length_)
	{
		return E_Filter_Text_TooLong;
	}

	guild_att.str_tenet = str;

	INT32 n_message_size = sizeof(NET_DB2C_change_guild_tenet) + n_string_length_ * sizeof(TCHAR);
	
	CREATE_MSG(pSend, n_message_size, NET_DB2C_change_guild_tenet);
	pSend->dw_guild_id = guild_att.dwID;
	memcpy(pSend->sz_new_tenet, str_tenet_, n_string_length_ * sizeof(TCHAR));
	pSend->sz_new_tenet[n_string_length_] = _T('\0');

	g_dbSession.Send(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 修改帮会团购指数
//-------------------------------------------------------------------------------
DWORD guild::modify_group_exponent(BOOL b_increase_)
{
	INT32 n_group_exponect = guild_att.nGroupPurchase;

	if (b_increase_)
	{
		if(guild_att.nGroupPurchase < MAX_GUILD_GROUP_PURCHASE)
			guild_att.nGroupPurchase++;
		else
			guild_att.nGroupPurchase = MAX_GUILD_GROUP_PURCHASE;
	}
	else
	{
		if(guild_att.nGroupPurchase > 0)
			guild_att.nGroupPurchase--;
		else
			guild_att.nGroupPurchase = 0;
	}

	if (n_group_exponect != guild_att.nGroupPurchase)
	{
		save_guild_att_to_db();
	}
	return E_Success;
}

//-------------------------------------------------------------------------------
//! 邀请签名
//-------------------------------------------------------------------------------
DWORD guild::invite_sign(Role* p_invite_role_, Role* p_beinvite_role_, INT64 n64_item_id_)
{
	if(!VALID_POINT(p_invite_role_) || !VALID_POINT(p_beinvite_role_))
		return INVALID_VALUE;

	//! 自己正在签名中
	if(p_invite_role_->GetSignID() != INVALID_VALUE)
		return E_Guild_Own_Signing;

	//! 对方正在签名中
	if(p_beinvite_role_->GetSignID() != INVALID_VALUE)
		return E_Guild_Other_Signing;

	//! 签名距离不合法
	if(!p_beinvite_role_->IsInDistance(*p_invite_role_, MAX_SIGN_DISTANCE))
		return E_Guild_Distance_NotCan;

	//! 已经是正式帮派
	if(guild_att.bFormal)
		return E_Guild_Already_Formal;

	//! 如果邀请者不是帮主
	if(guild_att.dwLeaderRoleID != p_invite_role_->GetID())
		return E_Guild_Power_NotEnough;

	//! 签名者等级不足
	if(p_beinvite_role_->get_level() < g_guild_manager.get_guild_config().nSignRoleMinLevel)
		return E_Guild_Sign_Level_NotEnough;

	//! 被邀请人离开帮派时间不足
	DWORD dw_calculate_time = CalcTimeDiff(g_world.GetWorldTime(), p_beinvite_role_->GetLeaveGuildTime());
	if(dw_calculate_time > 0)
	{
		if(dw_calculate_time < LEAVE_GUILD_TIME_LIMIT)
			return E_Guild_LeaveTime_Limit;
	}

	if(p_beinvite_role_->GetGuildID() != INVALID_VALUE)
		return E_Guild_Join_AlreadyIn;
	
	//! 帮主在签名者的黑名单中
	for(INT i = 0; i < MAX_BLACKLIST; ++i)
	{
		if(p_beinvite_role_->GetBlackList(i) == p_invite_role_->GetID())
			return E_Guild_SignIn_BlackList;
	}

	for(INT i = 0; i < MAX_GUILD_SIGN_NUM; i++)
	{
		if(guild_att.dwSignRoleID[i] != INVALID_VALUE)
		{
			if(guild_att.dwSignRoleID[i] == p_beinvite_role_->GetID())
				return E_Guild_SignRole_HaveExist;
		}
	}

	if(guild_att.bySignatoryNum >= MAX_GUILD_SIGN_NUM)
		return E_Guild_SignRole_Full;

	tagItem* p_item = p_invite_role_->GetItemMgr().GetBagItem(n64_item_id_);
	if(!VALID_POINT(p_item))
		return E_Guild_Sign_Item_Not;

	if(p_item->pProtoType->eSpecFunc != EIST_Sing)
		return E_Guild_Sign_Item_Not;

	//! 设置双方签名对象
	p_invite_role_->SetSignID(p_beinvite_role_->GetID());
	p_beinvite_role_->SetSignID(p_invite_role_->GetID());

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 确认签名
//-------------------------------------------------------------------------------
DWORD guild::affirmance_sign(Role* p_affirm_role_, Role* p_leader_role_)
{
	if(!VALID_POINT(p_affirm_role_) || !VALID_POINT(p_leader_role_))
		return INVALID_VALUE;

	guild_att.bySignatoryNum++;
	guild_att.dwSignRoleID[guild_att.bySignatoryNum-1] = p_affirm_role_->GetID();
	save_guild_att_to_db();

	p_affirm_role_->SetSignID(INVALID_VALUE);
	p_leader_role_->SetSignID(INVALID_VALUE);

	NET_SIS_affirmance_sign send;
	send.dwSignID = p_affirm_role_->GetID();
	p_leader_role_->SendMessage(&send, send.dw_size);

	NET_SIS_affirmance_sign_res send_res;
	p_affirm_role_->SendMessage(&send_res, send_res.dw_size);
	return E_Success;
}

//-------------------------------------------------------------------------------
//! 提交签名
//-------------------------------------------------------------------------------
DWORD guild::refer_sign(Role* p_role_, DWORD dw_npc_id_, INT64 n64_item_id_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	//! 已经是正式家族
	if(guild_att.bFormal)
		return E_Guild_Already_Formal;

	//! 不是帮主不能提交
	if(p_role_->GetID() != guild_att.dwLeaderRoleID)
		return E_Guild_Power_NotEnough;

	//! 签名人数不足
	if(guild_att.bySignatoryNum < MAX_GUILD_SIGN_NUM)
		return E_Guild_SignNum_NotEnough;

	tagItem* p_item = p_role_->GetItemMgr().GetBagItem(n64_item_id_);
	if(!VALID_POINT(p_item))
		return E_Guild_Sign_Item_Not;

	if(p_item->pProtoType->eSpecFunc != EIST_Sing)
		return E_Guild_Sign_Item_Not;

	//! 现金是否足够
	/*INT64 n16_cost = MGold2Silver(g_guild_manager.get_guild_config().nGoldCreateNeeded);
	if(p_role_->GetCurMgr().GetBagSilver() < n16_cost)
	{
		return E_BagSilver_NotEnough;
	}*/

	//! 扣除金钱
	//p_role_->GetCurMgr().DecBagSilver(n16_cost, elcid_guild_create);

	//p_role_->GetItemMgr().DelFromBag(n64_item_id_, elcid_guild_create);

	//p_role_->GetTitleMgr()->SigEvent(ete_create_faction, ete_create_faction, INVALID_VALUE);

	//! 全服广播
	s_role_info *p_role_info = g_roleMgr.get_role_info(p_role_->GetID());
	if (VALID_POINT(p_role_info))
	{
		INT32 n_role_name_count = _tcslen(p_role_info->sz_role_name);

		TCHAR sz_buff[X_LONG_NAME];
		ZeroMemory(sz_buff, sizeof(sz_buff));
		_tcsncpy(sz_buff, p_role_info->sz_role_name, n_role_name_count);
		_tcsncpy((sz_buff + n_role_name_count + 1), guild_att.str_name.c_str(), guild_att.str_name.size());


		INT n_size = guild_att.str_name.size();
		INT n_length = guild_att.str_name.length();
		INT32 n_buffer_size = (n_role_name_count + 1 + guild_att.str_name.length() + 1)*sizeof(TCHAR);

		HearSayHelper::SendMessage(EHST_CREATEGUILD, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, 
			INVALID_VALUE, INVALID_VALUE, NULL, FALSE, sz_buff, n_buffer_size);
		/*INT32 n_message_size = sizeof(NET_SIC_guild_create_broad) + (guild_att.str_name.length() + n_role_name_count + 1) * sizeof(TCHAR);

		CREATE_MSG(p_send, n_message_size, NET_SIC_guild_create_broad);
		p_send->dw_role_id		= p_role_->GetID();
		p_send->dwRoleNameID	= p_role_->GetNameID();
		p_send->dwGuildID	= guild_att.dwID;

		_tcscpy_s(p_send->szName, n_role_name_count + 1, p_role_info->sz_role_name);
		_tcscpy_s((p_send->szName + n_role_name_count + 1), guild_att.str_name.length() + 1, guild_att.str_name.c_str());

		g_roleMgr.send_world_msg(p_send, p_send->dw_size);
		MDEL_MSG(pSend);*/
	}

	//! 同步到周围玩家
	tagGuildMember* p_member = get_member(p_role_->GetID());
	Map*			p_map	= p_role_->get_map();
	if (VALID_POINT(p_member) && VALID_POINT(p_map))
	{
		NET_SIS_remote_role_guild_info_change send;
		send.dwGuildID		= guild_att.dwID;
		send.dw_role_id		= p_role_->GetID();
		send.n8GuildPos		= p_member->eGuildPos;

		p_map->send_big_visible_tile_message(p_role_, &send, send.dw_size);
	}

	change_guild_to_formal();
	
	NET_SIS_synchronize_guild_info send;
	send.sGuildInfo = (tagGuildBase)get_guild_att();
	p_role_->SendMessage(&send, send.dw_size);

	if( VALID_POINT( p_role_->GetScript( ) ) )
	{
		p_role_->GetScript( )->OnCreateGuild( p_role_, guild_att.dwID );
	}

	for(INT i = 0; i < MAX_GUILD_SIGN_NUM; i++)
	{
		if(guild_att.dwSignRoleID[i] == INVALID_VALUE)
			continue;

		Role *pInvitee = NULL;
		DWORD dw_error_code = can_invite_join(p_role_->GetID(), guild_att.dwSignRoleID[i], &pInvitee);
		
		if(VALID_POINT(pInvitee))
		{
			if(dw_error_code == E_Success)
			{
				NET_SIS_guild_join_request send;
				send.dwSrcRoleID	= p_role_->GetID();
				send.dwGuildID		= p_role_->GetGuildID();
				send.b_sign			= TRUE;
				pInvitee->SendMessage(&send, send.dw_size);
			}
		}
		else
		{
			tagMailBase st_mail_base;
			ZeroMemory(&st_mail_base, sizeof(st_mail_base));
			st_mail_base.dwSendRoleID = INVALID_VALUE;
			st_mail_base.dwRecvRoleID = guild_att.dwSignRoleID[i];
			TCHAR sz_content[50];
			ZeroMemory(sz_content, sizeof(sz_content));

			char sz_date[X_DATATIME_LEN + 1];
			ZeroMemory(sz_date, sizeof(sz_date));
			DwordTime2DataTime(sz_date, X_DATATIME_LEN + 1, guild_att.dwCreateTime);

			TCHAR	szName[X_SHORT_NAME];
			ZeroMemory(szName, sizeof(szName));
			g_roleMgr.get_role_name(p_role_->GetID(), szName);

			TCHAR szGuildName[MAX_GUILD_NAME_LEN];
			ZeroMemory(szGuildName, sizeof(szGuildName));
			_tcsncpy(szGuildName, get_guild_att().str_name.c_str(), MAX_GUILD_NAME_LEN);

			_stprintf(sz_content, _T("%s_%s_%s"), szGuildName, szName, sz_date);
			g_mailmgr.CreateMail(st_mail_base, _T("&gbuild&"), sz_content);
		}

	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 设置敌对帮会
//-------------------------------------------------------------------------------
DWORD guild::set_enemy_guild(Role* p_role_, DWORD dw_enemy_guild_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	guild* p_enemy_guild = g_guild_manager.get_guild(dw_enemy_guild_);
	if(!VALID_POINT(p_enemy_guild))
		return E_Guild_NoExist;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(!get_guild_power(p_guild_member->eGuildPos).bEnemy)
		return E_Guild_Power_NotEnough;

	if(dw_enemy_guild_ == get_guild_att().dwLeagueID)
		return E_Guild_League_NoCan_Enemy;

	if(get_enemy_num() >= MAX_ENEMY_NUM)
		return E_Guild_Enemy_Limit;

	if(is_enemy(p_enemy_guild->get_guild_att().dwID))
		return E_Guild_AlreadyIs_Enemy;

	set_enemy_num(dw_enemy_guild_);

	save_guild_att_to_db();

	NET_SIS_add_enemy_guild send;
	send.dwGuildEnemyID = dw_enemy_guild_;
	send.bAdd = TRUE;
	send_guild_message(&send, send.dw_size);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 删除敌对帮会
//-------------------------------------------------------------------------------
DWORD guild::delete_enemy_guild(Role* p_role_, DWORD dw_enemy_guild_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	guild* p_enemy_guild = g_guild_manager.get_guild(dw_enemy_guild_);
	if(!VALID_POINT(p_enemy_guild))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(!get_guild_power(p_guild_member->eGuildPos).bEnemy)
		return E_Guild_Power_NotEnough;

	delete_enemy_num(dw_enemy_guild_);

	save_guild_att_to_db();

	NET_SIS_add_enemy_guild send;
	send.dwGuildEnemyID = dw_enemy_guild_;
	send.bAdd = FALSE;
	send_guild_message(&send, send.dw_size);

	return E_Success;
}

VOID guild::setSignUpAttack(BOOL bSet, BOOL bSave)
{
	guild_att.bSignUpAttact = bSet;

	if (bSave)
	{
		save_guild_att_to_db();
	}
}

//-------------------------------------------------------------------------------
//! 通知指定成员扩展信息
//-------------------------------------------------------------------------------
DWORD guild::send_special_memberex_to_client(Role *p_role_, DWORD dw_special_role_id_)
{
	ASSERT(VALID_POINT(p_role_));

	tagGuildMember *p_special_member = get_member(dw_special_role_id_);
	if(!VALID_POINT(p_special_member))
	{
		return INVALID_VALUE;
	}

	NET_SIS_guild_get_member_extend send;
	send.dw_role_id						= dw_special_role_id_;
	send.sGuildMemInfoEx.nExploit		= p_special_member->nExploit;
	send.sGuildMemInfoEx.nTotalContrib	= p_special_member->nTotalContribution;
	send.sGuildMemInfoEx.nKnowledge		= INVALID_VALUE;

	Role *p_special_role = g_roleMgr.get_role(dw_special_role_id_);
	if(VALID_POINT(p_special_role))
	{
		send.sGuildMemInfoEx.nKnowledge = p_special_role->GetAttValue(ERA_Knowledge);
	}

	p_role_->SendMessage(&send, send.dw_size);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 更新指定帮会成员的详细信息
//-------------------------------------------------------------------------------
DWORD guild::send_special_member_to_client(Role *p_role_, DWORD dw_special_role_id_)
{
	ASSERT(VALID_POINT(p_role_));

	tagGuildMember *p_special_member = get_member(dw_special_role_id_);
	if(!VALID_POINT(p_special_member))
	{
		return INVALID_VALUE;
	}

	NET_SIS_guild_refresh_member send;
	send.sGuildMemInfo.dw_role_id		= dw_special_role_id_;
	send.sGuildMemInfo.n8GuildPos		= p_special_member->eGuildPos;
	send.sGuildMemInfo.nCurContrib		= p_special_member->nContribution;
	send.sGuildMemInfo.nDKP				= p_special_member->nDKP;

	send.sGuildMemInfoEx.nTotalContrib	= p_special_member->nTotalContribution;
	send.sGuildMemInfoEx.nExploit		= p_special_member->nExploit;
	send.sGuildMemInfo.dwJoinTime		= p_special_member->dwJoinTime;
	send.sGuildMemInfo.nTotalFund		= p_special_member->nTotalFund;

	Role *p_special_role = g_roleMgr.get_role(dw_special_role_id_);
	if(VALID_POINT(p_special_role))		
	{
		send.sGuildMemInfo.byLevel			= (BYTE)p_special_role->get_level();
		send.sGuildMemInfo.bySex			= p_special_role->GetSex();
		send.sGuildMemInfo.byClass			= p_special_role->GetClass();
		send.sGuildMemInfo.dwTimeLastLogout	= INVALID_VALUE;
		send.sGuildMemInfo.bOnline			= true;

		send.sGuildMemInfoEx.nKnowledge		= p_special_role->GetAttValue(ERA_Knowledge);
	}
	else	
	{
		s_role_info *p_special_role_info = g_roleMgr.get_role_info(dw_special_role_id_);
		if(!VALID_POINT(p_special_role_info))
		{
			ASSERT(0);
			return INVALID_VALUE;
		}

		send.sGuildMemInfo.byLevel			= p_special_role_info->by_level;
		send.sGuildMemInfo.bySex			= p_special_role_info->by_sex_;
		send.sGuildMemInfo.byClass			= p_special_role_info->e_class_type_;		
		send.sGuildMemInfo.dwTimeLastLogout	= p_special_role_info->dw_time_last_logout_;
		send.sGuildMemInfo.bOnline			= false;

		send.sGuildMemInfoEx.nKnowledge		= INVALID_VALUE;
	}

	p_role_->SendMessage(&send, send.dw_size);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 获取帮会名称
//-------------------------------------------------------------------------------
DWORD guild::send_guild_name_to_client(PlayerSession *p_session_)
{
	ASSERT(VALID_POINT(p_session_));
	
	INT32 n_name_size = get_guild_att().str_name.size();
	ASSERT(n_name_size > 0);

	INT32 n_message_size = sizeof(NET_SIS_get_guild_name) + n_name_size * sizeof(TCHAR);

	CREATE_MSG(p_send, n_message_size, NET_SIS_get_guild_name);
	p_send->dwGuildID	= get_guild_att().dwID;
	_tcscpy(p_send->szGuildName, get_guild_att().str_name.c_str());
	p_session_->SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(pSend);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 获取帮派公告
//-------------------------------------------------------------------------------
DWORD guild::send_guild_tenet_to_client(PlayerSession *p_session_)
{
	ASSERT(VALID_POINT(p_session_));
	
	INT32 n_name_size = get_guild_att().str_tenet.size();
	if(n_name_size <=0)
	{
		return E_Success;
	}

	INT32 n_message_size = sizeof(NET_SIS_get_guild_tenet) + n_name_size * sizeof(TCHAR);

	CREATE_MSG(p_send, n_message_size, NET_SIS_get_guild_tenet);
	_tcscpy(p_send->szGuildTenet, get_guild_att().str_tenet.c_str());
	p_session_->SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(p_send);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 获取帮会标志
//-------------------------------------------------------------------------------
DWORD guild::send_guild_symbol_to_client(PlayerSession* p_session_)
{
	ASSERT(VALID_POINT(p_session_));

	INT32 n_name_size = get_guild_att().str_symbol.size();
	if(n_name_size <=0)
	{
		return E_Success;
	}

	INT32 n_message_size = sizeof(NET_SIS_get_guild_symbol) + n_name_size * sizeof(TCHAR);

	CREATE_MSG(p_send, n_message_size, NET_SIS_get_guild_symbol);

	//NET_SIS_get_guild_symbol send;
	p_send->dwGuildID = get_guild_att().dwID;
	_tcscpy(p_send->bySymbol, get_guild_att().str_symbol.c_str());
	p_session_->SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(p_send);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 获取帮会设施升级信息
//-------------------------------------------------------------------------------
DWORD guild::send_facilities_info_to_client(Role* p_role_, EFacilitiesType e_facilities_type_)
{
	ASSERT(VALID_POINT(p_role_));

	tagGuildMember* p_member = _member_manager.get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}

	/*if (!get_guild_power(p_member->eGuildPos).bUpgrade)
	{
		return E_Guild_Power_NotEnough;
	}*/

	NET_SIS_get_guild_upgreade_info send;

	DWORD dw_error_code = _guild_upgrade.GetGuildFacilitiesInfo(send.sFacilitiesInfo, e_facilities_type_);

	if (E_Success == dw_error_code)
	{
		p_role_->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//-------------------------------------------------------------------------------
//! 角色删除时清理帮会数据
//-------------------------------------------------------------------------------
DWORD guild::clear_role_remove(DWORD dw_role_id_)
{
	tagGuildMember *p_member = get_member(dw_role_id_);
	if(!VALID_POINT(p_member))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	if(!get_guild_power(p_member->eGuildPos).bLeaveGuild)
	{
		return E_Guild_CanNot_Leave;
	}

	remove_member(dw_role_id_, p_member->eGuildPos);

	return E_Success;
}

//-------------------------------------------------------------------------------
//！ 保存帮会属性到数据库
//-------------------------------------------------------------------------------
VOID guild::save_guild_att_to_db()
{
	NET_DB2C_save_guild_att send;
	send.p_guild_base = (tagGuildBase)guild_att;

	g_dbSession.Send(&send, send.dw_size);
}

VOID	guild::save_guild_plant_to_db()
{
	NET_DB2C_save_plant send;
	send.p_guild_plant.dw_guild_id = guild_att.dwID;
	memcpy(send.p_guild_plant.s_data, m_plantData, sizeof(m_plantData));
	g_dbSession.Send(&send, send.dw_size);
}
//-------------------------------------------------------------------------------
//! 设置帮会仓库使用权限
//-------------------------------------------------------------------------------
DWORD guild::set_guild_ware_use_privilege( Role* p_role_, DWORD dw_role_id_, BOOL b_can_use_ )
{
	ASSERT(VALID_POINT(p_role_));

	if (!VALID_VALUE(dw_role_id_))
	{
		return INVALID_VALUE;
	}

	tagGuildMember* p_member = get_member(p_role_->GetID());
	if (!VALID_POINT(p_member))
	{
		ASSERT(p_member);
		return E_Guild_MemberNotIn;
	}

	if (!get_guild_power(p_member->eGuildPos).bSetWareRights)
	{
		return E_Guild_Power_NotEnough;
	}

	p_member = get_member(dw_role_id_);
	if (!VALID_POINT(p_member))
	{
		return E_Guild_MemberNotIn;
	}
	else if (get_guild_power(p_member->eGuildPos).bSetWareRights)
	{
		return E_Guild_Power_NotEnough;
	}

	_member_manager.set_guild_ware_use_privilege(dw_role_id_, b_can_use_);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 修改帮会金钱
//-------------------------------------------------------------------------------
BOOL guild::increase_guild_fund( DWORD dw_role_id_, INT32 n_fund_, DWORD dw_commond_id_ )
{
	if (n_fund_ <= 0)
	{
		return FALSE;
	}

	guild_att.nFund += n_fund_;
	//INT32 n_max_fund = _guild_upgrade.GetMaxFund();
	//if (guild_att.nFund > n_max_fund)
	//{
	//	n_fund_ = guild_att.nFund - n_max_fund;
	//	guild_att.nFund = n_max_fund;
	//}

	log_guild_fund(dw_role_id_, n_fund_, guild_att.nFund, dw_commond_id_);

	if (is_in_guild_state(EGDSS_Distress))
	{
		unset_guild_state(EGDSS_Distress);
		if (guild_att.nFund < guild_att.nDailyCost*3)
		{
			set_guild_state(EGDSS_Shortage);
		}
	}
	else if (is_in_guild_state(EGDSS_Shortage))
	{
		if (guild_att.nFund >= guild_att.nDailyCost*3)
		{
			unset_guild_state(EGDSS_Shortage);
		}
	}

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::increase_guild_material( DWORD dw_role_id_, INT32 n_material_, DWORD dw_command_id_ )
{
	if (n_material_ <= 0)
	{
		return FALSE;
	}

	guild_att.nMaterial += n_material_;

	INT32 n_max_material = _guild_upgrade.GetMaxMaterial();
	if (guild_att.nMaterial > n_max_material)
	{
		n_material_ = guild_att.nMaterial - n_max_material;
		guild_att.nMaterial = n_max_material;
	}
	log_guild_material(dw_role_id_, n_material_, guild_att.nMaterial, dw_command_id_);

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::increase_guild_reputation( DWORD dw_role_id_, INT32 n_reputation_, DWORD dw_command_id_ )
{
	if (n_reputation_ <= 0)
	{
		return FALSE;
	}

	guild_att.nReputation += n_reputation_;

	if (guild_att.nReputation > MAX_GUILD_REP)
	{
		n_reputation_ = guild_att.nReputation - MAX_GUILD_REP;
		guild_att.nReputation = MAX_GUILD_REP;
	}
	log_guild_reputation(dw_role_id_, n_reputation_, guild_att.nReputation, dw_command_id_);

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::increase_prosperity(DWORD dw_role_id_, INT32 n_prosperity_, DWORD dw_command_id_)
{
	if(n_prosperity_ <= 0)
		return FALSE;

	guild_att.nProsperity += n_prosperity_;

	if(guild_att.nProsperity > MAX_GUILD_PROSPERITY)
	{
		guild_att.nProsperity = MAX_GUILD_PROSPERITY;
	}

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::increase_guild_peace( DWORD dw_role_id_, INT16 n_peace_, DWORD dw_command_id_ )
{
	if (n_peace_ <= 0)
	{
		return FALSE;
	}

	guild_att.n16Peace += n_peace_;

	if (guild_att.n16Peace > MAX_GUILD_PEACE)
	{
		n_peace_ = guild_att.n16Peace - MAX_GUILD_PEACE;
		guild_att.n16Peace = MAX_GUILD_PEACE;
	}

	//guild_att.nDailyCost = (INT32)MGuildDailyCost(guild_att.byLevel, guild_att.n16Peace);


	//if (is_in_guild_state(EGDSS_Chaos))
	//{
	//	if (guild_att.n16Peace >= 500)
	//	{
	//		unset_guild_state(EGDSS_Chaos);
	//	}
	//}

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::decrease_guild_fund( DWORD dw_role_id_, INT32 n_fund_, DWORD dw_command_id_ )
{
	if (n_fund_ <= 0)
	{
		return FALSE;
	}

	guild_att.nFund -= n_fund_;
	if (guild_att.nFund < 0)
	{
		n_fund_ += guild_att.nFund;
		guild_att.nFund = 0;
	}
	log_guild_fund(dw_role_id_, -n_fund_, guild_att.nFund, dw_command_id_);

	if (!is_in_guild_state_any(EGDSS_Distress | EGDSS_Shortage))
	{
		if (guild_att.nFund == 0)
		{
			set_guild_state(EGDSS_Distress);
		}
		else if (guild_att.nFund < guild_att.nDailyCost*3)
		{
			set_guild_state(EGDSS_Shortage);
		}
	}
	else if (is_in_guild_state(EGDSS_Shortage))
	{
		if (guild_att.nFund == 0)
		{
			unset_guild_state(EGDSS_Shortage);
			set_guild_state(EGDSS_Distress);
		}
	}

	save_guild_att_to_db();
	
	return TRUE;
}

BOOL guild::decrease_guild_material( DWORD dw_role_id_, INT32 n_material_, DWORD dw_command_id_ )
{
	if (n_material_ <= 0)
	{
		return FALSE;
	}

	guild_att.nMaterial -= n_material_;
	if (guild_att.nMaterial < 0)
	{
		n_material_ += guild_att.nMaterial;
		guild_att.nMaterial = 0;
	}
	log_guild_material(dw_role_id_, -n_material_, guild_att.nMaterial, dw_command_id_);

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::decrease_guild_reputation( DWORD dw_role_id_, INT32 n_reputation_, DWORD dw_command_id_ )
{
	if (n_reputation_ <= 0)
	{
		return FALSE;
	}

	guild_att.nReputation -= n_reputation_;
	if (guild_att.nReputation < 0)
	{
		n_reputation_ += guild_att.nReputation;
		guild_att.nReputation = 0;
	}
	log_guild_reputation(dw_role_id_, -n_reputation_, guild_att.nReputation, dw_command_id_);

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::decrease_guild_peace( DWORD dw_role_id_, INT16 n_peace_, DWORD dw_command_id_ )
{
	if (n_peace_ <= 0)
	{
		return FALSE;
	}

	guild_att.n16Peace -= n_peace_;
	if (guild_att.n16Peace < 0)
	{
		n_peace_ += guild_att.n16Peace;
		guild_att.n16Peace = 0;
	}

	//guild_att.nDailyCost = (INT32)MGuildDailyCost(guild_att.byLevel, guild_att.n16Peace);

	//if (!is_in_guild_state(EGDSS_Chaos))
	//{
	//	if (guild_att.n16Peace < 500)
	//	{
	//		set_guild_state(EGDSS_Chaos);
	//	}
	//}

	save_guild_att_to_db();

	return TRUE;
}

BOOL guild::decrease_prosperity(DWORD dw_role_id_, INT32 n_prosperity_, DWORD dw_command_id_)
{
	if (n_prosperity_ <= 0)
	{
		return FALSE;
	}

	guild_att.nProsperity -= n_prosperity_;
	if (guild_att.nProsperity < 0)
	{
		guild_att.nProsperity = 0;
	}

	save_guild_att_to_db();

	return TRUE;
}

VOID guild::set_script_data(INT nIndex, INT n_data)
{
	guild_att.n32ScriptData[nIndex] = n_data;

	save_guild_att_to_db();
}

VOID guild::log_guild_fund( DWORD dw_role_id_, INT32 n_fund_, INT32 n_total_fund_, DWORD dw_command_id_ )
{
	NET_DB2C_log_fund send;
	
	s_role_info* p_role_info = g_roleMgr.get_role_info(dw_role_id_);
	if (!VALID_POINT(p_role_info))
	{
		return;
	}

	send.s_log_fund_.dw_account_id	= p_role_info->dw_account_id;
	send.s_log_fund_.dw_cmd_id		= dw_command_id_;
	send.s_log_fund_.dw_guild_id		= guild_att.dwID;
	send.s_log_fund_.dw_role_id		= dw_role_id_;
	send.s_log_fund_.n_fund			= n_fund_;
	send.s_log_fund_.n_total_funk	= n_total_fund_;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild::log_guild_material( DWORD dw_role_id_, INT32 n_material_, INT32 n_total_material_, DWORD dw_command_id_ )
{
	NET_DB2C_log_material send;

	s_role_info* p_role_info = g_roleMgr.get_role_info(dw_role_id_);
	if (!VALID_POINT(p_role_info))
	{
		return;
	}

	send.s_log_material_.dw_account_id		= p_role_info->dw_account_id;
	send.s_log_material_.dw_cmd_id			= dw_command_id_;
	send.s_log_material_.dw_guild_id			= guild_att.dwID;
	send.s_log_material_.dw_role_id			= dw_role_id_;
	send.s_log_material_.n_material			= n_material_;
	send.s_log_material_.n_total_material	= n_total_material_;

	g_dbSession.Send(&send, send.dw_size);
}

VOID guild::log_guild_reputation( DWORD dw_role_id_, INT32 n_reputation_, INT32 n_total_reputation_, DWORD dw_command_id_ )
{
	NET_DB2C_log_reputation send;

	s_role_info* p_role_info = g_roleMgr.get_role_info(dw_role_id_);
	if (!VALID_POINT(p_role_info))
	{
		return;
	}

	send.s_log_reputation_.dw_account_id			= p_role_info->dw_account_id;
	send.s_log_reputation_.dw_cmd_id				= dw_command_id_;
	send.s_log_reputation_.dw_guild_id			= guild_att.dwID;
	send.s_log_reputation_.dw_role_id			= dw_role_id_;
	send.s_log_reputation_.n_reputation			= n_reputation_;
	send.s_log_reputation_.n_total_reputation	= n_total_reputation_;

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------
// ! 每日重置帮会状态
//-------------------------------------------------------------------------------
VOID guild::daily_guild_reset()
{
	//！ 扣除帮会每日资金和安定消耗
	//decrease_guild_fund(INVALID_VALUE, guild_att.nDailyCost, ELCLD_Guild_DailyCost);
	//decrease_guild_peace(INVALID_VALUE, MGuildPeaceCost(guild_att.byLevel), ELCLD_Guild_DailyCost);

	//！ 奖励跑商贡献度大的玩家
	//_guild_commerce.extend_commendation();

	//！ 重置帮会发布次数
	//m_GuildAffair.reset_affair_count();

	//guild_att.bSignUpAttact = FALSE;

	//guild_att.n32ScriptData[5] = 0;

	save_guild_att_to_db();
}

VOID guild::guild_quest_reset( )
{
	guild_att.n32ScriptData[5] = 0;
	save_guild_att_to_db();
}

//-------------------------------------------------------------------------------
//! 帮会升级
//-------------------------------------------------------------------------------
VOID guild::reinit_guild_upgrade( BYTE by_new_level_)
{
	if (by_new_level_ > MAX_GUILD_LEVEL)
	{
		return;
	}

	if (guild_att.byLevel == by_new_level_)
	{
		return;
	}

	BYTE byOldLevel = guild_att.byLevel;

	//! 设置帮会新等级
	if (by_new_level_ == 0)
	{
		dismiss_guild();
		return;
	}
	else
	{
		guild_att.byLevel = by_new_level_;
	}

	//! 计算帮会新等级的相关属性
	_guild_position.init(guild_att.byLevel);
	_guild_warehouse.reset_init();
	_guild_upgrade.ReSetFacilitiesMaxLevel();

	guild_att.nDailyCost = (INT32)MGuildDailyCost(guild_att.byLevel, guild_att.n16Peace);
	re_calculate_affair_remain_num(MGuildAffairTimes(byOldLevel) - guild_att.byAffairRemainTimes);

	save_guild_att_to_db();
}

//-------------------------------------------------------------------------------
//! 重新计算帮务剩余发布次数
//-------------------------------------------------------------------------------
VOID guild::re_calculate_affair_remain_num( BYTE by_times )
{
	BYTE by_times_ = guild_att.byAffairRemainTimes;
	guild_att.byAffairRemainTimes	= MGuildAffairTimes(guild_att.byLevel) - by_times;

	if (by_times_ != guild_att.byAffairRemainTimes)
	{
		save_guild_att_to_db();
	}
}

//-------------------------------------------------------------------------------
//! 帮会是否占领了指定城市
//-------------------------------------------------------------------------------
BOOL guild::is_occupy_city( BYTE by_city_index_ )
{
	if (by_city_index_ == 0)
	{
		return TRUE;
	}

	for (int n=0; n<MAX_GUILD_HOLDCITY; n++)
	{
		if (guild_att.byHoldCity[n] == by_city_index_)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
//！ 帮会转正
//-------------------------------------------------------------------------------
VOID guild::change_guild_to_formal()
{
	guild_att.bFormal = TRUE;

	tagGuildPower power;
	memset(&power,0,sizeof(tagGuildPower));

	//！帮主权限	
	power.bDismissGuild			= 1;
	power.bModifyTenet			= 1;
	power.bInviteJoin			= 1;
	power.bTurnoverLeader		= 1;
	power.bModifySgin			= 1;
	power.bWareRights			= 1;
	power.bUpgrade				= 1;
	power.bCommerce				= 1;
	power.bSetCommend			= 1;
	power.bAffair				= 1;
	power.bSetSkill				= 1;
	power.bAdvSkill				= 1;
	power.bModifyPosName		= 1;
	power.bModifyPosPower		= 1;
	power.bKickMember			= 1;
	power.bAppointMember		= 1;
	power.bDeclareWar			= 1;
	power.bAcceptWar			= 1;       
	power.bModifyDKP			= 1;       
	power.bUnSay				= 1;		
	power.bLeague				= 1;	
	power.bUnLeague				= 1;		
	power.bEnemy				= 1;			
	power.bWar					= 1;

	guild_att.dwPosPower[EGMP_BangZhu]= *(DWORD*)&power;

	//！ 副帮主权限
	memset(&power,0,sizeof(tagGuildPower));

	power.bDismissGuild			= 1;
	power.bModifyTenet			= 1;
	power.bInviteJoin			= 1;
	power.bLeaveGuild			= 1;
	power.bDemissPostion		= 1;
	power.bModifySgin			= 1;
	power.bWareRights			= 1;
	power.bUpgrade				= 1;
	power.bCommerce				= 1;
	power.bSetCommend			= 1;
	power.bAffair				= 1;
	power.bSetSkill				= 1;
	power.bAdvSkill				= 1;
	power.bModifyPosName		= 1;
	power.bModifyPosPower		= 1;
	power.bKickMember			= 1;
	power.bAppointMember		= 1;
	power.bDeclareWar			= 1;
	power.bAcceptWar			= 1;  
	power.bUnSay				= 1;		
	power.bEnemy				= 1;		
	power.bWar					= 1;

	guild_att.dwPosPower[EGMP_FuBangZhu]= *(DWORD*)&power;

	//！ 帮众权限
	memset(&power,0,sizeof(tagGuildPower));

	power.bLeaveGuild			= 1;
	power.bWar					= 1;

	guild_att.dwPosPower[EGMP_BangZhong] = *(DWORD*)&power;

	for(INT i = EGMP_OFFICER_1; i <= EGMP_End; i++)
	{
		guild_att.dwPosPower[i] = *(DWORD*)&power;
	}

	//set_guild_state(EGDSS_UpLevel);
	
	reinit_guild_upgrade(1);
	
	for (int i = EFT_Lobby; i < EFT_End; i++)
	{
		get_upgrade().ChangeFacilitesLevel((EFacilitiesType)i);
	}

	//get_upgrade().ChangeFacilitesLevel(EFT_Lobby);

	save_guild_att_to_db();
}

//-------------------------------------------------------------------------------
//! 获得敌对帮会数据
//-------------------------------------------------------------------------------
DWORD guild::get_enemy_data(Role* p_role_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	INT n_message_size = sizeof(NET_SIS_get_enemy_data) + sizeof(tagGuildEnemyData) * (MAX_ENEMY_NUM - 1);
	CREATE_MSG(p_enemy_data, n_message_size, NET_SIS_get_enemy_data);

	//! 如果是被宣战状态
	if(_guild_war.get_guild_war_state() == EGWS_BeDeclare)
	{
		p_enemy_data->dwDeclareGuildID = _guild_war.get_declare_guild_id();
		p_enemy_data->dwDeclareBeginTime = _guild_war.get_begin_time();
	}
	else
	{
		p_enemy_data->dwDeclareGuildID = INVALID_VALUE;
	}

	//! 如果是宣战状态
	if(_guild_war.get_guild_war_state() == EGWS_Declare)
	{
		p_enemy_data->dwBeDeclareGuildID = _guild_war.get_bedeclare_guild_id();
		p_enemy_data->dwDeclareBeginTime = _guild_war.get_begin_time();
	}
	else
	{
		p_enemy_data->dwBeDeclareGuildID = INVALID_VALUE;
	}

	//! 如果是战斗准备状态
	if(_guild_war.get_guild_war_state() == EGWS_Prepare)
	{
		p_enemy_data->dwWarGuildID = _guild_war.get_war_guild_id();
		p_enemy_data->dwPrepareBeginTime = _guild_war.get_begin_time();
	}
	else
	{
		p_enemy_data->dwWarGuildID = INVALID_VALUE;
	}

	//! 如果是战斗状态
	if(_guild_war.get_guild_war_state() == EGWS_WAR)
	{
		p_enemy_data->dwWarGuildID = _guild_war.get_war_guild_id();
		p_enemy_data->dwWarBeginTime = _guild_war.get_begin_time();
	}
	else
	{
		p_enemy_data->dwWarGuildID = INVALID_VALUE;
	}

	p_enemy_data->dwCurrTime = g_world.GetWorldTime();
	p_enemy_data->eGuildWarState = _guild_war.get_guild_war_state();

	INT n_num = 0;
	for(int i = 0; i < MAX_ENEMY_NUM; i++)
	{
		if(guild_att.dwEnemyID[i] == INVALID_VALUE)
			continue;

		p_enemy_data->st_EnemyData[n_num].dwGuildID = guild_att.dwEnemyID[i];
		guild* p_guild = g_guild_manager.get_guild(guild_att.dwEnemyID[i]);
		if(VALID_POINT(p_guild))
		{
			p_enemy_data->st_EnemyData[n_num].eGuildWarState = p_guild->get_guild_war().get_guild_war_state();
		}
		n_num++;
	}

	p_enemy_data->n_num = n_num;

	if(n_num > 0)
	{
		p_enemy_data->dw_size = sizeof(NET_SIS_get_enemy_data) + sizeof(tagGuildEnemyData) * (n_num - 1);
	}
	else
	{
		p_enemy_data->dw_size = sizeof(NET_SIS_get_enemy_data);
	}
	
	p_role_->SendMessage(p_enemy_data, p_enemy_data->dw_size);

	MDEL_MSG(p_enemy_data);
	return E_Success;

}

//-------------------------------------------------------------------------------
//! 帮会宣战
//-------------------------------------------------------------------------------
DWORD guild::declare_war(Role* p_role_, DWORD dw_enemy_guild_id_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;
	
	if (p_role_->GetGuildID() == dw_enemy_guild_id_)
		return INVALID_VALUE;

	guild* p_enemy_guild = g_guild_manager.get_guild(dw_enemy_guild_id_);
	if(!VALID_POINT(p_enemy_guild))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;
	
	// 对方帮主是否在线
	//Role* p_enemy_leader = g_roleMgr.get_role(p_enemy_guild->get_guild_att().dwLeaderRoleID);
	//if(!VALID_POINT(p_enemy_leader))
	//	return E_Guild_Leader_Not_Line;
	
	//不到时间
	DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
	if (dwWeek != 4 && dwWeek != 5 && dwWeek != 6)
		return E_Guild_Declare_Time_Limit;

	//tagDWORDTime dw_current_time = g_world.GetWorldTime();
	//if(dw_current_time.hour < 20 || dw_current_time.hour > 22)
	//	return E_Guild_Declare_Time_Limit;
	//
	//// 已经向对方宣战过了
	//if (listEnemyGuild.find(dw_enemy_guild_id_) != listEnemyGuild.end())
	//{
	//	return E_Guild_war_Cant_Again;
	//}

	if(guild_att.byLevel < 3)
		return E_Guild_Declare_Level_Limit;

	if(p_enemy_guild->get_guild_att().byLevel < 3)
		return E_Guild_Enemy_Level_Limit;

	if(guild_att.byLevel - p_enemy_guild->get_guild_att().byLevel > 2 )
		return E_Guild_Declare_Level_Limit;
	
	// 安定度判断
	if (get_guild_att().n16Peace < 50 || 
		p_enemy_guild->get_guild_att().n16Peace < 50)
	{
		return E_Guild_NotEnough_peace;
	}

	//if(!get_guild_power(p_guild_member->eGuildPos).bDeclareWar)
	//	return E_Guild_Power_NotEnough;

	if(guild_att.nFund < GuildHelper::getGuildWarFund(guild_att.byLevel))
		return E_Guild_Fund_NotEnough;
	
	if(p_enemy_guild->get_guild_att().nFund < GuildHelper::getGuildWarFund(p_enemy_guild->get_guild_att().byLevel))
		return E_Guild_Fund_NotEnough;

	//if(!is_enemy(dw_enemy_guild_id_))
	//	return E_Guild_NoIs_Enemy;

	//if(get_upgrade().IsFacilitesUpLevel())
	//	return E_Guild_UpLevel;

	//if(p_enemy_guild->get_upgrade().IsFacilitesUpLevel())
	//	return E_Guild_Enemy_UpLevel;

	//if(get_upgrade().IsFacilitesDestory())
	//	return E_Guild_Grade_Dead;

	//if(p_enemy_guild->get_upgrade().IsFacilitesDestory())
	//	return E_Guild_Enemy_Grade_Dead;

	if(_guild_war.get_guild_war_state() != EGWS_NULL)
	{
		//if(_guild_war.get_guild_war_state() != EGWS_Avoidance)
			return E_Guild_WarState_Limit;
	}

	if(p_enemy_guild->get_guild_war().get_guild_war_state() != EGWS_NULL)
	{
		return E_Guild_WarState_Limit;
	}
		
	//免战
	if (isMianzhan())
	{
		return E_Guild_League_Not_Exist;
	}

	//对方免战
	if (p_enemy_guild->isMianzhan())
	{
		return E_Guild_Have_League;
	}

	//if(p_enemy_guild->get_guild_att().nFund < (6000000 + 6000000 * p_enemy_guild->get_guild_att().byLevel))
	//	return E_Guild_Enemy_Fund_NotEnough;

	//decrease_guild_fund(INVALID_VALUE, g_guild_manager.get_guild_config().nGoldGuildWarNeeded, elcid_guild_declarewar);
	
	_guild_war.set_bedeclare_guild_id(p_enemy_guild->get_guild_att().dwID);
	p_enemy_guild->get_guild_war().set_declare_guild_id(guild_att.dwID);

	//_guild_war.set_guild_war_state(EGWS_Declare);
	get_guild_war().set_guild_war_state(EGWS_Prepare,p_enemy_guild->get_guild_att().str_name, TRUE);
	get_guild_war().set_war_guild_id(get_guild_war().get_bedeclare_guild_id());
	get_guild_war().set_enemy_guild_level(p_enemy_guild->get_guild_att().byLevel);
	get_guild_member().set_guild_war(get_guild_att().dwLeaderRoleID, true);
	get_guild_war().m_dw_baoming_num++;
	NET_SIS_guild_war_member_change send;
	send.dw_role_id = get_guild_att().dwLeaderRoleID;
	send.m_bAdd = TRUE;
	send_guild_message(&send, send.dw_size);

	//p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_BeDeclare);
	p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_Prepare,get_guild_att().str_name, FALSE);
	p_enemy_guild->get_guild_war().set_war_guild_id(p_enemy_guild->get_guild_war().get_declare_guild_id());
	p_enemy_guild->get_guild_war().set_enemy_guild_level(guild_att.byLevel);
	p_enemy_guild->get_guild_member().set_guild_war(p_enemy_guild->get_guild_att().dwLeaderRoleID, true);
	p_enemy_guild->get_guild_war().m_dw_baoming_num++;
	NET_SIS_guild_war_member_change send1;
	send1.dw_role_id = p_enemy_guild->get_guild_att().dwLeaderRoleID;
	send1.m_bAdd = TRUE;
	p_enemy_guild->send_guild_message(&send1, send1.dw_size);
	
	// 给对方帮主发邮件
	tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_enemy_guild->get_guild_att().dwLeaderRoleID;

	TCHAR szPetMailContent[X_SHORT_NAME] = _T("");
	_stprintf(szPetMailContent, _T("%s"), get_guild_att().str_name.c_str());

	g_mailmgr.CreateMail(st_mail_base, _T("&GuildWar&"), szPetMailContent, NULL, 0);
	

	//p_enemy_guild->get_guild_war().get_declare_broadcast().SetBroadcast(300, 1, 1500);


	//listEnemyGuild.insert(dw_enemy_guild_id_);

	//_guild_war.set_begin_time();
	//p_enemy_guild->get_guild_war().set_begin_time();
	

	// 发送给被宣战方
	//NET_SIS_guild_bedeclare_war send;
	//send.dwGuildID = get_guild_att().dwID;
	//_tcsncpy(send.szGuildName, get_guild_att().str_name.c_str(), get_guild_att().str_name.size());

	//s_role_info* pRoleInfo = g_roleMgr.get_role_info(p_enemy_leader->GetID());
	//if (VALID_POINT(pRoleInfo))
	//{	
	//	_tcsncpy(send.szLeaderName, pRoleInfo->sz_role_name, X_SHORT_NAME);
	//}
	//send.dwLevel = get_guild_att().byLevel;
	//p_enemy_leader->SendMessage(&send, send.dw_size);
	

	return E_Success;
}


DWORD guild::declare_war_gm(Role* p_role_, DWORD dw_enemy_guild_id_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	if (p_role_->GetGuildID() == dw_enemy_guild_id_)
		return INVALID_VALUE;

	guild* p_enemy_guild = g_guild_manager.get_guild(dw_enemy_guild_id_);
	if(!VALID_POINT(p_enemy_guild))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;


	if(_guild_war.get_guild_war_state() != EGWS_NULL)
	{
		//if(_guild_war.get_guild_war_state() != EGWS_Avoidance)
		return E_Guild_WarState_Limit;
	}

	if(p_enemy_guild->get_guild_war().get_guild_war_state() != EGWS_NULL)
	{
		return E_Guild_WarState_Limit;
	}

	//对方免战
	if (p_enemy_guild->isMianzhan())
	{
		return E_Guild_Have_League;
	}

	_guild_war.set_bedeclare_guild_id(p_enemy_guild->get_guild_att().dwID);
	p_enemy_guild->get_guild_war().set_declare_guild_id(guild_att.dwID);

	//_guild_war.set_guild_war_state(EGWS_Declare);
	get_guild_war().set_guild_war_state(EGWS_Prepare,p_enemy_guild->get_guild_att().str_name, TRUE);
	get_guild_war().set_war_guild_id(get_guild_war().get_bedeclare_guild_id());
	get_guild_war().set_enemy_guild_level(p_enemy_guild->get_guild_att().byLevel);
	get_guild_member().set_guild_war(get_guild_att().dwLeaderRoleID, true);
	get_guild_war().m_dw_baoming_num++;
	NET_SIS_guild_war_member_change send;
	send.dw_role_id = get_guild_att().dwLeaderRoleID;
	send.m_bAdd = TRUE;
	send_guild_message(&send, send.dw_size);

	//p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_BeDeclare);
	p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_Prepare,get_guild_att().str_name, FALSE);
	p_enemy_guild->get_guild_war().set_war_guild_id(p_enemy_guild->get_guild_war().get_declare_guild_id());
	p_enemy_guild->get_guild_war().set_enemy_guild_level(guild_att.byLevel);
	p_enemy_guild->get_guild_member().set_guild_war(p_enemy_guild->get_guild_att().dwLeaderRoleID, true);
	p_enemy_guild->get_guild_war().m_dw_baoming_num++;
	NET_SIS_guild_war_member_change send1;
	send1.dw_role_id = p_enemy_guild->get_guild_att().dwLeaderRoleID;
	send1.m_bAdd = TRUE;
	p_enemy_guild->send_guild_message(&send1, send1.dw_size);


	tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_enemy_guild->get_guild_att().dwLeaderRoleID;

	TCHAR szMailContent[X_SHORT_NAME] = _T("");
	_stprintf(szMailContent, _T("%s"), get_guild_att().str_name.c_str());

	g_mailmgr.CreateMail(st_mail_base, _T("&GuildWar&"), szMailContent, NULL, 0);

	return E_Success;
}
//-------------------------------------------------------------------------------
//! 帮会宣战回复
//-------------------------------------------------------------------------------
DWORD guild::declare_war_res(Role* p_role_, DWORD dw_enemy_guild_id_, BOOL b_accept_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	guild* p_enemy_guild = g_guild_manager.get_guild(dw_enemy_guild_id_);
	if(!VALID_POINT(p_enemy_guild))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(get_guild_war().get_guild_war_state() != EGWS_Prepare)
		return E_Guild_AcceptWar_Limit;

	if(!get_guild_power(p_guild_member->eGuildPos).bAcceptWar)
		return E_Guild_Power_NotEnough;

	if(get_guild_war().get_declare_guild_id() == INVALID_VALUE || p_enemy_guild->get_guild_war().get_bedeclare_guild_id() == INVALID_VALUE)
		return E_Guild_AcceptWar_Limit;

	if(p_enemy_guild->get_guild_war().get_bedeclare_guild_id() != guild_att.dwID)
		return E_Guild_AcceptWar_Limit;

	if(get_guild_war().get_declare_guild_id() != p_enemy_guild->get_guild_att().dwID)
		return E_Guild_AcceptWar_Limit;

	if(b_accept_)
	{
		get_guild_war().set_guild_war_state(EGWS_Prepare,p_enemy_guild->get_guild_att().str_name, TRUE);
		get_guild_war().set_war_guild_id(get_guild_war().get_declare_guild_id());
		//get_guild_war().set_begin_time();
		get_guild_war().get_prepare_broadcast().SetBroadcast(300, 3, 2700);
		get_guild_war().set_enemy_guild_level(p_enemy_guild->get_guild_att().byLevel);
		get_guild_member().set_guild_war(get_guild_att().dwLeaderRoleID, true);
		get_guild_war().m_dw_baoming_num++;
		NET_SIS_guild_war_member_change send;
		send.dw_role_id = get_guild_att().dwLeaderRoleID;
		send.m_bAdd = TRUE;
		send_guild_message(&send, send.dw_size);

		DWORD dw_guild_instance_id = get_guild_instance_id();
		if( VALID_POINT(dw_guild_instance_id) )
		{
			map_instance_guild* p_guild_instance = get_guild_map();
			if( VALID_POINT(p_guild_instance) )
			{
				p_guild_instance->on_guild_prepare_war(this);
				p_guild_instance->set_in_war(true);
				p_guild_instance->set_guild(this, p_enemy_guild);
			}
		}

		p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_Prepare,get_guild_att().str_name, FALSE);
		p_enemy_guild->get_guild_war().set_war_guild_id(p_enemy_guild->get_guild_war().get_bedeclare_guild_id());
		//p_enemy_guild->get_guild_war().set_begin_time();
		p_enemy_guild->get_guild_war().get_prepare_broadcast().SetBroadcast(300, 3, 2700);
		p_enemy_guild->get_guild_war().set_enemy_guild_level(guild_att.byLevel);
		p_enemy_guild->get_guild_member().set_guild_war(p_enemy_guild->get_guild_att().dwLeaderRoleID, true);
		p_enemy_guild->get_guild_war().m_dw_baoming_num++;
		NET_SIS_guild_war_member_change send1;
		send1.dw_role_id = p_enemy_guild->get_guild_att().dwLeaderRoleID;
		send1.m_bAdd = TRUE;
		p_enemy_guild->send_guild_message(&send1, send1.dw_size);

		dw_guild_instance_id = p_enemy_guild->get_guild_instance_id();
		if( VALID_POINT(dw_guild_instance_id) )
		{
			map_instance_guild* p_guild_instance = get_guild_map();
			if( VALID_POINT(p_guild_instance) )
			{
				p_guild_instance->on_guild_prepare_war(p_enemy_guild);
			}
		}
	}
	else
	{
		tagGuildWarHistory sWarHistory;

		sWarHistory.dw_time = GetCurrentDWORDTime();
		sWarHistory.dw_guild_id = get_guild_att().dwID;
		sWarHistory.dw_enemy_id = p_enemy_guild->get_guild_att().dwID;
		sWarHistory.dw_enemy_leader_id = p_enemy_guild->get_guild_att().dwLeaderRoleID;
		sWarHistory.e_war_history_type = EWHT_JUJUE;
		add_war_history(sWarHistory);

		sWarHistory.dw_guild_id = p_enemy_guild->get_guild_att().dwID;
		sWarHistory.dw_enemy_id = get_guild_att().dwID;
		sWarHistory.dw_enemy_leader_id = get_guild_att().dwLeaderRoleID;
		sWarHistory.e_war_history_type = EWHT_DUIFANG_JUJUE;
		p_enemy_guild->add_war_history(sWarHistory);
		

		// 比目标帮派等级高,惩罚下
		if (get_guild_att().byLevel >= p_enemy_guild->get_guild_att().byLevel)
		{
			//INT32 n_prosperity = 1000 * get_guild_att().byLevel;
			//decrease_prosperity(INVALID_VALUE, n_prosperity, elcid_guild_declarewar);
			
			INT64 n64Silver = 100000 * get_guild_att().byLevel;
			p_role_->GetCurMgr().DecBagBindSilver(n64Silver, elcid_guild_declarewar);
		}

		get_guild_war().reset();
		get_guild_war().set_guild_war_state(EGWS_NULL);
		p_enemy_guild->get_guild_war().reset();
		p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_NULL, _T(""), false);
			
		guild_att.dwJujueTime = g_world.GetWorldTime();
		decrease_guild_peace(INVALID_VALUE, 10, elcid_guild_declarewar);
		p_enemy_guild->decrease_guild_peace(INVALID_VALUE, 10, elcid_guild_declarewar);
		//p_enemy_guild->increase_guild_fund(INVALID_VALUE, g_guild_manager.get_guild_config().nGoldGuildWarNeeded, elcid_guild_declarewar);
		get_guild_member().reset_in_war();
		p_enemy_guild->get_guild_member().reset_in_war();


		NET_SIS_refuse_declare_broad send;
		send.dwDeclareGuildID = p_enemy_guild->get_guild_att().dwID;
		send.dwRefuseGuildID = get_guild_att().dwID;

		_tcsncpy(send.szDeclareGuildName, get_guild_att().str_name.c_str(), get_guild_att().str_name.size());
		_tcsncpy(send.szRefuseGuildName, p_enemy_guild->get_guild_att().str_name.c_str(), p_enemy_guild->get_guild_att().str_name.size());

		g_roleMgr.send_world_msg(&send, send.dw_size);
	}
	

	return E_Success;
}


DWORD	guild::guild_qualify_war(Role* p_role, BOOL bJoin)
{
	if(!VALID_POINT(p_role))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;
	
	//if (p_role->get_level() < 20)
	//	return E_Guild_Join_LevelLimit;

	if(get_guild_war().get_guild_war_state() != EGWS_Prepare)
		return E_Guild_War_Not_Relay;

	//if(!get_guild_power(p_guild_member->eGuildPos).bWar)
	//	return E_Guild_War_Not_Accept;

	if (get_guild_war().m_dw_baoming_num >= get_guild_war().get_max_number() && bJoin)
		return E_Guild_War_num_max;
	
	if (p_guild_member->bWar != bJoin)
	{
		if (bJoin)
		{
			get_guild_war().m_dw_baoming_num++;
		}
		else
		{
			get_guild_war().m_dw_baoming_num--;
		}
		change_member_war(p_role->GetID(), bJoin, false);
	}
	
	return E_Success;

}

DWORD	guild::guild_qualify_war_dis(Role* p_role, DWORD dwMemberID)
{
	if(!VALID_POINT(p_role))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(dwMemberID);
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if (get_guild_att().dwLeaderRoleID != p_role->GetID())
		return E_Guild_War_Not_Leader;
	
	if(get_guild_war().get_guild_war_state() != EGWS_Prepare)
		return E_Guild_War_Not_Relay;
	
	if (p_guild_member->bWar)
	{
		get_guild_war().m_dw_baoming_num--;
		change_member_war(dwMemberID, FALSE, false);
	}


	return E_Success;
}

DWORD	guild::materialReceive(Role* pRole, DWORD dwID, INT nNumber)
{
	const tagGuildMaterialReceive* pProto = AttRes::GetInstance()->GetMaterialReceive(dwID);
	if (!VALID_POINT(pProto))
		return INVALID_VALUE;
	
	INT nAddNum = GET_VIP_EXTVAL(pRole->GetTotalRecharge(), GUILD_JUANCAILAO, INT);

	if (pRole->GetDayClearData(0) + nNumber > MAX_MATERIMAL_NUM + nAddNum)
	{
		return E_Guild_Material_Not_time;
	}
	
	// 判断物品个数
	if (pRole->GetItemMgr().GetBagSameItemCount(dwID) < nNumber)
	{
		return E_Guild_Material_Not_Enough;
	}
	
	if (pProto->byLevel > get_guild_att().byLevel)
	{
		return INVALID_VALUE;
	}

	package_list<tagItem*> itemList;
	itemList.clear();
	pRole->GetItemMgr().GetBag().GetSameItemList(itemList, dwID, nNumber);
	pRole->GetItemMgr().DelBagSameItem(itemList, nNumber, elcid_guild_materimal);
	
	increase_guild_material(pRole->GetID(), nNumber*pProto->n_material, elcid_guild_materimal);

	//change_member_contribution(pRole->GetID(), nNumber*pProto->n_contribution, TRUE);
	pRole->ChangeBrotherhood(nNumber*pProto->n_contribution);

	//pRole->ModRoleDayClearDate(ERDCT_GuildMaterial, nNumber);
	
	pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_guild_cailiao, nNumber);
	
	if(VALID_POINT(pRole->GetScript()))
		pRole->GetScript()->OnJuanMate(pRole, nNumber);
	
	s_role_info* pRoleInfo = g_roleMgr.get_role_info(pRole->GetID());
	if (VALID_POINT(pRoleInfo))
	{
		NET_SIS_Guild_msg send;
		//_tcsncpy(send.szName, pRoleInfo->sz_role_name, X_SHORT_NAME);
		send.byType = 2;
		send.nChangeValue = nNumber*pProto->n_material;
		send_guild_message(&send, send.dw_size);
	}
	
	return E_Success;
}
DWORD	guild::donateFund(Role* pRole, BYTE byType)
{
	//gx modify vip 2013.8.14
	INT vipDelta = GET_VIP_EXTVAL(pRole->GetVIPLevel(),HANGHUISHAOXIANG_ADD, INT);
	if (pRole->GetDayClearData(1) >= (1+vipDelta))
	{
		return E_Guild_Material_Not_time;
	}
	INT32 nNeedFund = 0;
	INT32 nNeedYuanbao = 0;

	INT32 nAddGuildFund = 0;
	INT32 nAddContribution = 0;
	INT32 nAddExp = 0;
	INT32 nShengWang = 0;
	if (byType == 0)
	{
		nNeedFund = 10000;
		nAddExp = 300000;
		nAddGuildFund = 100;
		nAddContribution = 10;
		nShengWang = 100;
	}
	else if( byType == 1)
	{
		nNeedYuanbao = 10;
		nAddExp = 680000;
		nAddGuildFund = 500;
		nAddContribution = 50;
		nShengWang = 200;
	}
	else
	{
		nNeedYuanbao = 66;
		nAddExp = 1680000;
		nAddGuildFund = 3000;
		nAddContribution = 300;
		nShengWang = 500;
	}

	if (pRole->GetCurMgr().GetBagSilver() < nNeedFund)
	{
		return E_Guild_Material_Not_Money;
	}
	if (pRole->GetCurMgr().GetBaiBaoYuanBao() < nNeedYuanbao)
	{
		return E_Guild_Material_Not_Money;
	}

	increase_guild_fund(pRole->GetID(), nAddGuildFund, elcid_guild_inc_fund);
	
	pRole->ExpChange(nAddExp);
	pRole->GetCurMgr().DecBagSilver(nNeedFund, elcid_guild_inc_fund);
	pRole->GetCurMgr().DecBaiBaoYuanBao(nNeedYuanbao, elcid_guild_inc_fund);
	pRole->ModAttValue(ERA_Knowledge, nShengWang);

	change_member_contribution(pRole->GetID(), nAddContribution, TRUE);
	//increase_member_total_fund(pRole->GetID(), nFund/10000);

	pRole->ModRoleDayClearDate(ERDCT_GuildFund);
	

	//pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_guild_zijin, nFund);
	


	if(VALID_POINT(pRole->GetScript()))
		pRole->GetScript()->OnShangxiang(pRole);


	s_role_info* pRoleInfo = g_roleMgr.get_role_info(pRole->GetID());
	if (VALID_POINT(pRoleInfo))
	{
		NET_SIS_Guild_msg send;
		//_tcsncpy(send.szName, pRoleInfo->sz_role_name, X_SHORT_NAME);
		send.byType = 1;
		send.nChangeValue = nAddGuildFund;
		send_guild_message(&send, send.dw_size);
	}
	return E_Success;
}

DWORD guild::useMianzhan(Role* pRole, INT64 n64ItemID)
{

	tagGuildMember* pMember = get_member(pRole->GetID());
	if (!VALID_POINT(pMember))
		return INVALID_VALUE;

	if (pMember->eGuildPos != EGMP_BangZhu && pMember->eGuildPos != EGMP_FuBangZhu)
		return INVALID_VALUE;

	tagItem* pItem = pRole->GetItemMgr().GetBagItem(n64ItemID);
	if (!VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EIST_Guild_mianzhan)
		return INVALID_VALUE;
	
	guild_att.dwMianzhanTime = g_world.GetWorldTime();

	// 取消帮会战
	if (get_guild_war().get_guild_war_state() == EGWS_Prepare && get_guild_war().isDefenter())
	{
		guild* p_enemy_guild = g_guild_manager.get_guild(get_guild_war().get_declare_guild_id());
		if (VALID_POINT(p_enemy_guild))
		{
			get_guild_war().reset();
			get_guild_war().set_guild_war_state(EGWS_NULL, _T(""), true, true);
			p_enemy_guild->get_guild_war().reset();
			p_enemy_guild->get_guild_war().set_guild_war_state(EGWS_NULL, _T(""), FALSE, true);
			get_guild_member().reset_in_war();
			p_enemy_guild->get_guild_member().reset_in_war();

		}

	}

	pRole->GetItemMgr().ItemUsedFromBag(n64ItemID, 1, (DWORD)elcid_guild_declarewar);


	return E_Success;
}

DWORD guild::ModifyApplyLevel(Role* pRole, INT32 nLevel)
{
	tagGuildMember* pMember = get_member(pRole->GetID());
	if (!VALID_POINT(pMember))
		return INVALID_VALUE;

	if (pMember->eGuildPos == EGMP_BangZhong)
		return INVALID_VALUE;

	guild_att.nApplyLevel = nLevel;

	save_guild_att_to_db();

	return E_Success;
}

DWORD	guild::SignUpAttact(Role* pRole)
{
	DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
	if (dwWeek == 2 || dwWeek == 5)
		return E_SBK_ERROR_6;

	tagGuildMember* pMember = get_member(pRole->GetID());
	if (!VALID_POINT(pMember))
		return E_SBK_ERROR_1;

	if (pMember->eGuildPos == EGMP_BangZhong)
		return E_SBK_ERROR_1;
	
	if (guild_att.bSignUpAttact)
		return E_SBK_ERROR_2;

	// 沙巴克公会不用报名
	if (g_guild_manager.get_SBK_guild() == guild_att.dwID)
		return E_SBK_ERROR_3;
	
	// 检查帮会资金
	if (guild_att.nFund < 10000)
		return INVALID_VALUE;


	guild_att.bSignUpAttact = true;

	save_guild_att_to_db();

	// 扣除帮派资金
	decrease_guild_fund(pRole->GetID(), 10000, elcid_guild_declarewar);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 帮会设置DKP
//-------------------------------------------------------------------------------
DWORD guild::set_guild_dkp(Role* p_role_, INT16* p_n16_dkp_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	//if(guild_att.bSignUpAttact)
	//	return E_Guild_ChangeDKP_Limit;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(!get_guild_power(p_guild_member->eGuildPos).bModifyDKP)
		return E_Guild_Power_NotEnough;

	for(INT i = 0; i < EDKP_END; i++)
	{
		if(p_n16_dkp_[i] > 100)
			p_n16_dkp_[i] = 100;
		if(p_n16_dkp_[i] < 0)
			p_n16_dkp_[i] = 0;
	}

	memcpy(guild_att.n16DKP, p_n16_dkp_, sizeof(guild_att.n16DKP));

	//guild_att.bSignUpAttact = TRUE;

	save_guild_att_to_db();

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 帮会DKP确认
//-------------------------------------------------------------------------------
DWORD guild::set_dkp_affirmance(Role* p_role_, INT16 n16_dkp_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	if(n16_dkp_ <= 0)
		return INVALID_VALUE;

	tagGuildMember* pGuildMember = get_member(p_role_->GetID());
	if(!VALID_POINT(pGuildMember))
		return INVALID_VALUE;

	if(pGuildMember->eGuildPos != EGMP_BangZhu)
	{
		if(pGuildMember->eGuildPos != EGMP_FuBangZhu)
			return E_Guild_Power_NotEnough;
	}

	if(n16_dkp_ > 100)
		n16_dkp_ = 100;

	tagVisTile* p_visual_field[EUD_end] = {0};
	p_role_->get_map()->get_visible_tile(p_role_->GetVisTileIndex(), p_visual_field);

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(p_visual_field[n]) ) continue;

		package_map<DWORD, Role*>& map_role = p_visual_field[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = map_role.begin();
		Role* p_temp_role = NULL;

		while(map_role.find_next(it, p_temp_role))
		{
			if(!VALID_POINT(p_temp_role))
				continue;
			if(p_temp_role->GetGuildID() != p_role_->GetGuildID())
				continue;

			tagGuildMember* p_guild_member = get_member(p_temp_role->GetID());
			if(!VALID_POINT(p_guild_member))
				continue;

			change_member_dkp(p_temp_role->GetID(), n16_dkp_);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 帮会邀请联盟
//-------------------------------------------------------------------------------
DWORD guild::invite_league(Role* p_role_, DWORD dw_league_id_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	guild* p_league_guild = g_guild_manager.get_guild(dw_league_id_);
	if(!VALID_POINT(p_league_guild))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(!get_guild_power(p_guild_member->eGuildPos).bLeague)
		return E_Guild_Power_NotEnough;

	//DWORD dw_calculate_time = CalcTimeDiff(g_world.GetWorldTime(), guild_att.dwUnLeagueBeginTime);
	//if(dw_calculate_time > 0)
	//{
	//	if(dw_calculate_time < MAX_UNLEAGUE_TIME)
	//		return E_Guild_League_Time_Limit;
	//}
	
	//dw_calculate_time = CalcTimeDiff(g_world.GetWorldTime(), p_league_guild->get_guild_att().dwUnLeagueBeginTime);
	//if(dw_calculate_time > 0)
	//{
	//	if(dw_calculate_time < MAX_UNLEAGUE_TIME)
	//		return E_Guild_TargetLeague_Time_Limit;
	//}
	
	if(guild_att.dwLeagueID != INVALID_VALUE)
		return E_Guild_Have_League;

	if(is_enemy(dw_league_id_))
		return E_Guild_Enemy_NoCan_League;
	
	Role* p_league_leader = g_roleMgr.get_role(p_league_guild->get_guild_att().dwLeaderRoleID);
	if(!VALID_POINT(p_league_leader))
		return E_Guild_Leader_Not_Line;

	if(p_league_guild->get_guild_att().dwLeagueID != INVALID_VALUE)
		return E_Guild_Have_League;

	NET_SIS_send_invite_league send;
	send.dwInviteGuildID = guild_att.dwID;
	p_league_leader->SendMessage(&send, send.dw_size);

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 帮会邀请联盟回复
//-------------------------------------------------------------------------------
DWORD guild::invite_league_res(Role* p_role_, DWORD dw_invite_guild_id_, BOOL b_agree_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	guild* p_invite_guild = g_guild_manager.get_guild(dw_invite_guild_id_);
	if(!VALID_POINT(p_invite_guild))
		return INVALID_VALUE;	

	if(p_invite_guild->get_guild_att().dwLeagueID != INVALID_VALUE)
		return E_Guild_Have_League;

	if(b_agree_)
	{
		set_league_id(p_invite_guild->get_guild_att().dwID);
		p_invite_guild->set_league_id(guild_att.dwID);

		NET_SIS_send_invite_league_result send;
		send.dwInviteGuildID = p_invite_guild->get_guild_att().dwID;
		send.dwResGuildID = guild_att.dwID;
		send.bAgree = b_agree_;
		send_guild_message(&send, send.dw_size);

		p_invite_guild->send_guild_message(&send, send.dw_size);
	}
	else
	{
		Role* p_role = g_roleMgr.get_role(p_invite_guild->get_guild_att().dwLeaderRoleID);
		if(VALID_POINT(p_role))
		{
			NET_SIS_send_invite_league_result send;
			send.dwInviteGuildID = p_invite_guild->get_guild_att().dwID;
			send.dwResGuildID = guild_att.dwID;
			send.bAgree = b_agree_;
			p_role->SendMessage(&send, send.dw_size);
		}
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
//! 解除联盟
//-------------------------------------------------------------------------------
DWORD guild::relieve_league(Role* p_role_, DWORD dw_guild_id_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	guild* p_berelieve_guild = g_guild_manager.get_guild(dw_guild_id_);
	if(!VALID_POINT(p_berelieve_guild))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(guild_att.dwLeagueID == INVALID_VALUE)
		return E_Guild_League_Not_Exist;

	if(p_berelieve_guild->get_guild_att().dwLeagueID == INVALID_VALUE)
		return E_Guild_League_Not_Exist;

	if(guild_att.dwID != p_berelieve_guild->get_guild_att().dwLeagueID)
		return E_Guild_No_League;

	if(!get_guild_power(p_guild_member->eGuildPos).bUnLeague)
		return E_Guild_Power_NotEnough;

	set_league_id(INVALID_VALUE);
	p_berelieve_guild->set_league_id(INVALID_VALUE);
	set_unleague_time();

	NET_SIS_send_relieve_league send;
	send.dwRelieveID = guild_att.dwID;
	send.dwBeRelieveID = p_berelieve_guild->get_guild_att().dwID;
	send_guild_message(&send, send.dw_size);

	p_berelieve_guild->send_guild_message(&send, send.dw_size);

	return E_Success;
}

BOOL guild::IsLobbyPracitec(Role* pRole)
{
	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return FALSE;

	Creature* pCreature = NULL;
	Unit* pUnit = NULL;

	if(!get_upgrade().IsLobbyUsing())
		return FALSE;


	if(pMap->get_map_type() == EMT_Guild)
	{
		map_instance_guild* pInst = (map_instance_guild*)pMap;

		pCreature = pInst->find_creature(pInst->get_lobby_id());
		if(!VALID_POINT(pCreature))
			return FALSE;

		pUnit = (Unit*)pCreature;
	}
	else
	{
		if(dw_guild_tripod_id == INVALID_VALUE || dw_tripod_map_id == INVALID_VALUE)
			return FALSE;

		Map* pNormalMap = g_mapCreator.get_map(dw_tripod_map_id, INVALID_VALUE);
		if(!VALID_POINT(pNormalMap))
			return FALSE;

		pCreature = pNormalMap->find_creature(dw_guild_tripod_id);
		if(!VALID_POINT(pCreature))
			return FALSE;

		pUnit = (Unit*)pCreature;
	}

	if(!pRole->IsInDistance(*pUnit, MAX_LOBBY_DIS))
		return FALSE;

	return TRUE;
}

FLOAT guild::LobbyPractice(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return 0;

	if(!IsLobbyPracitec(pRole))
		return 0;
	
	INT nNum = GetDaogaoNumber();

	return 1.2 + 0.2 * nNum;
	
}

INT guild::get_member_full()
{
	return MGuildMaxMember(get_guild_att().byLevel);
}

FLOAT guild::LevelToMultiple(INT byLevel)
{
	switch(byLevel)
	{
	case 1:
		return 2;
	case 2:
		return 2.5;
	case 3:
		return 3;
	case 4:
		return 3.5;
	case 5:
		return 4;
	case 6:
		return 4.5;
	case 7:
		{
			if(get_tripod_id() != INVALID_VALUE)
			{
				return 7;
			}
			return 5;
		}
	default:
		return 0;
	}
}

VOID guild::send_tripod_message()
{
	NET_SIS_Guild_Tripod_Info send;
	send.dw_tripod_id = dw_guild_tripod_id;
	send.dw_map_id = dw_tripod_map_id;
	send_guild_message(&send, send.dw_size);
}

VOID guild::add_war_history(const tagGuildWarHistory& gwh, BOOL bSaveDB) 
{ 
	vecWarHistory.push_back(gwh); 

	if (bSaveDB)
	{
		NET_C2DB_insert_guild_war_history send;
		send.st_guild_war_history = gwh;

		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID guild::synchronize_guild_war_info()
{
	guild* pEnemyGuild = g_guild_manager.get_guild(get_guild_war().get_war_guild_id());
	if (!VALID_POINT(pEnemyGuild))
		return ;

	NET_SIS_synchronize_guild_war_info send;
	send.dwMaxWarNum = get_guild_war().get_max_number();
	send.dwCurWarNum = get_guild_war().m_dw_baoming_num;
	send.fMoneyParam = get_guild_war().get_param();
	send.eSatae = get_guild_war().get_guild_war_state();
	send.dwBeginTime = (DWORD)get_guild_war().get_begin_time();

	_tcsncpy(send.szGuildName, pEnemyGuild->get_guild_att().str_name.c_str(), pEnemyGuild->get_guild_att().str_name.size());
	send.byEnemyLevel = pEnemyGuild->get_guild_att().byLevel;
	send.dwSymbolValue = get_guild_att().dwValue;
	send.bDefenter = (get_guild_war().get_declare_guild_id() != INVALID_VALUE);
	send.bRelay = get_guild_war().isRelay();
	memcpy(send.szText, get_guild_att().szText, sizeof(send.szText));

	send_guild_message(&send, send.dw_size);
}

// 帮会升级
DWORD	guild::handleLevelUp(Role* p_role)
{
	// 上层需要保证参数的合法性
	ASSERT(VALID_POINT(p_role));


	tagGuildMember* pMember = get_member(p_role->GetID());
	if (!VALID_POINT(pMember))
	{
		return E_Guild_MemberNotIn;
	}

	// 判断该玩家的地位
	//if (!get_guild_power(pMember->eGuildPos).bUpgrade)
	//{
	//	return E_Guild_Power_NotEnough;
	//}
	if (pMember->eGuildPos != EGMP_BangZhu)
	{
		return E_Guild_MemberNotIn;
	}

	if(get_guild_war().get_guild_war_state() != EGWS_NULL)
		return E_Guild_Waring_NotCan_UpLevel;


	// 判断等级上限
	if (get_guild_att().byLevel >= MAX_GUILD_LEVEL)
	{
		return E_GuildUpgrade_Level_Limit;
	}
	
	INT32 nNeedFund = GuildHelper::getLevelUpNeedFund(get_guild_att().byLevel);
	if(get_guild_att().nFund < nNeedFund)
	{
		return E_Guild_Prosperity_NotEnouth;
	}

	// 扣除帮派资金和资材
	decrease_guild_fund(p_role->GetID(), nNeedFund, elcid_guild_upgrade);
	//decrease_prosperity(p_role->GetID(), nNeedProsperity, elcid_guild_upgrade);

	reinit_guild_upgrade(get_guild_att().byLevel + 1);

	return E_Success;
}

map_instance_guild* guild::get_guild_map()
{
	DWORD dw_guild_instance_id = get_guild_instance_id();
	if( VALID_POINT(dw_guild_instance_id) )
	{
		DWORD dw_guild_map_id = get_tool()->crc32(szGuildMapName);
		Map* p_map = g_mapCreator.get_map(dw_guild_map_id, dw_guild_instance_id);

		if( VALID_POINT(p_map) )
		{
			map_instance_guild* p_guild_instance = dynamic_cast<map_instance_guild*>(p_map);
			return p_guild_instance;
		}

	}

	return NULL;
}

// 激活祷告者
VOID guild::SetDaogao(INT32 nIndex, BOOL bActive)
{
	if (nIndex < 0 || nIndex >= MAX_DAOGA_NUM)
		return;

	//guild_att.bDaogao[nIndex] = bActive;

	save_guild_att_to_db();
}

//激活或取消特定个数祷告者
VOID guild::ActiveDaogaoNumber(DWORD dwNum, BOOL bActive)
{
	if ( dwNum <= 0)
		return;

	DWORD dwCurNum = 0;
	if (bActive)
	{
		for (int i = 8; i < MAX_DAOGA_NUM; i++)
		{
			if (dwCurNum == dwNum)
				break;

			if (!IsDaogaoActive(i))
			{
				SetDaogao(i, true);
				dwCurNum++;
			}
		}
	}
	else
	{
		for (int i = MAX_DAOGA_NUM; i >= 0; i--)
		{
			if (dwCurNum == dwNum)
				break;

			if (IsDaogaoActive(i))
			{
				SetDaogao(i, false);
				dwCurNum++;
			}
		}
	}
	save_guild_att_to_db();
}
// 某个祷告者是否激活
BOOL guild::IsDaogaoActive(INT32 nIndex)
{
	//if (nIndex < 0 || nIndex >= MAX_DAOGA_NUM)
	//	return false;

	//return guild_att.bDaogao[nIndex];
	return false;
}

// 获取祷告者数量
DWORD guild::GetDaogaoNumber(BOOL bQiang)
{
	DWORD dwNum = 0;
	int nbegin = 0;
	if (bQiang )
	{
		nbegin = 8;
	}
	
	for (int i = nbegin; i < MAX_DAOGA_NUM; i++)
	{
		if (IsDaogaoActive(i))
		{
			dwNum++;
		}
	}
	


	return dwNum;
}

BOOL guild::isMianzhan()
{
	if (guild_att.dwMianzhanTime == 0)
		return false;

	if (CalcTimeDiff(g_world.GetWorldTime(), get_guild_att().dwMianzhanTime) > 7*24*60*60)
		return false;
	
	return true;

}

DWORD guild::getGuildSBKReward( Role* pRole )
{
	tagGuildMember* pMember = get_member(pRole->GetID());
	if (VALID_POINT(pMember))
	{
		if (pMember->bBallot)
			return E_SBK_ERROR_5;


		if (pRole->GetItemMgr().GetBagFreeSize() <= 0)
			return E_SBK_ERROR_7;

		pMember->bBallot = true;

		if (pRole->GetScript())
		{
			pRole->GetScript()->OnSBKLottery(pRole, pMember->eGuildPos);
		}

		NET_DB2C_change_ballot send;
		send.dw_role_id = pRole->GetID();
		send.b_ballot = true;
		g_dbSession.Send(&send, send.dw_size);

	}
	return E_Success;
}
