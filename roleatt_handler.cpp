
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//移动消息处理类

#include "StdAfx.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/role_info_protocol.h"

#include "pet_pocket.h"
#include "pet_soul.h"
#include "player_session.h"
#include "role.h"
#include "creature.h"
#include "role_mgr.h"
#include "map.h"
#include "pet.h"
#include "guild.h"
#include "guild_manager.h"
#include "title_mgr.h"
#include "master_prentice_mgr.h"
#include "RankMgr.h"

//------------------------------------------------------------------------------
// 获取人物属性
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetRoleInitAtt(tag_net_message* pCmd)
{
	NET_SIC_get_role_init_state* p_receive = (NET_SIC_get_role_init_state*)pCmd;

	if( p_receive->eType <= ERIT_Null || p_receive->eType >= ERIT_End )
		return INVALID_VALUE;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;


	switch(p_receive->eType)
	{
	case ERIT_Att:
		pRole->SendInitStateAtt();
		break;
	case ERIT_Skill:
		pRole->SendInitStateSkill();
		break;
	case ERIT_Item:
		pRole->SendInitStateItem();
		pRole->SendInitStateSuit();
		break;
	case ERIT_CompleteQuest:
		pRole->send_init_complete_quest();
		break;
	case ERIT_IncompleteQuest:
		pRole->send_init_incomplete_quest();
		break;
	case ERIT_Money:
		pRole->SendInitStateMoney();
		break;
	case ERIT_Reputation:
		pRole->SendInitStateReputation();
		break;
	case ERIT_FrindAndEnemy:
		pRole->SendFriend();
		pRole->SendBlack();
		pRole->SendEnemy();
		break;
	case ERIT_Pet_Sns_info:
		pRole->SendPetSNSInfo();
		break;
	case ERIT_Guild:
		pRole->SendInitStateGuild();
		break;
	case ERIT_MasterPrentice:
		g_MasterPrenticeMgr.get_master_prentices( pRole->GetID( ), pCmd );
		break;
	case ERIT_paimai:
		pRole->send_paimai_info();
		pRole->send_jingpai_info();
		break;
	case ERIT_bank:
		pRole->send_bank_paimai_info();
		break;
	case ERIT_Other:
		//pRole->SendRoleHelp();
		pRole->SendRoleTalk();
		pRole->SendKeyInfo();
		pRole->SendGiftInfo();
		pRole->send_vigour_reward(TRUE);
		pRole->SendReceiveType();
		pRole->Send1v1ScoreInfo();
		pRole->SendActiveInfo();
		pRole->SendGuildActiveInfo();
		pRole->SendRoleDayClear();
		pRole->ShowConsumUI( );
		pRole->SendConsumeReward();
		pRole->sendSaoDangData();
		RankMgr::GetInstance()->sendDiaoxiangName(pRole);
		break;
	case ERIT_Achievement:
		pRole->GetAchievementMgr().SendAllAchievementData();
		break;
	default:
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 获取远程人物或生物属性（是不是要做些处理，如果玩家一直在发怎么办）
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetRemoteUnitAtt(tag_net_message* pCmd)
{
	NET_SIC_get_remote_role_state* p_receive = (NET_SIC_get_remote_role_state*)pCmd;

	Role* pSelf = GetRole();
	if( !VALID_POINT(pSelf) || INVALID_VALUE == pSelf->GetVisTileIndex() ) return INVALID_VALUE;

	Map* pMap = pSelf->get_map();
	if( !VALID_POINT(pMap) ) return INVALID_VALUE;

	if( p_receive->nRoleNum > 100 ) return INVALID_VALUE;	// 最多发50个

	package_list<DWORD>	list_error;

	// 顺序的轮询地图中的ID
	for(INT n = 0; n < p_receive->nRoleNum; n++)
	{
		if( !VALID_POINT(p_receive->dw_role_id[n]) )
			break;

		// 检查是人物还是生物
		if( IS_PLAYER(p_receive->dw_role_id[n]) )
		{
			Role* pRemoteRole = pMap->find_role(p_receive->dw_role_id[n]);
			if(!VALID_POINT(pRemoteRole))
			{
				pRemoteRole = pMap->find_leave_role(p_receive->dw_role_id[n]);
			}
			if( VALID_POINT(pRemoteRole) )
			{
				INT nBuffNum = pRemoteRole->GetBuffNum();
				DWORD dw_size = sizeof(NET_SIS_get_remote_role_state) + ((nBuffNum > 0) ? (nBuffNum - 1)*sizeof(tagBuffMsgInfo) : 0);

				// 发送远程人物属性同步消息
				CREATE_MSG(pSend, dw_size, NET_SIS_get_remote_role_state);

				pSend->RoleData.dwID = pRemoteRole->GetID();
				pSend->RoleData.nLevel = pRemoteRole->get_level();

				//pSend->RoleData.byStallLevel = pRemoteRole->GetStallModeLevel();
				pSend->RoleData.eClassType = pRemoteRole->GetClass();
				//pSend->RoleData.eClassTypeEx = pRemoteRole->GetClassEx();
				
				// 位置
				Vector3 vPos = pRemoteRole->GetCurPos();
				pSend->RoleData.fPos[0] = vPos.x;
				pSend->RoleData.fPos[1] = vPos.y;
				pSend->RoleData.fPos[2] = vPos.z;

				// 朝向
				Vector3 vFace = pRemoteRole->GetFaceTo();
				pSend->RoleData.fFaceTo[0] = vFace.x;
				pSend->RoleData.fFaceTo[1] = vFace.y;
				pSend->RoleData.fFaceTo[2] = vFace.z;

				// 状态
				pSend->RoleData.dwState = pRemoteRole->GetState();
				pSend->RoleData.n64RoleState = pRemoteRole->GetRoleState();
				pSend->RoleData.ePKState = pRemoteRole->GetPKState();
				pSend->RoleData.iPKValue = pRemoteRole->GetClientPKValue();
				//pSend->RoleData.bIsPurpureDec = pRemoteRole->GetIsPurpureDec( );

				// 帮派
				pSend->RoleData.dwGuildID = pRemoteRole->GetGuildID();
				pSend->RoleData.dwTeamID = pRemoteRole->GetTeamID();
				pSend->RoleData.byLeader = pRemoteRole->GetIsLeader();
				pSend->RoleData.dwTargetID = pRemoteRole->GetTargetID();
				pSend->RoleData.dwEquipRating = pRemoteRole->GetEquipTeamInfo().n32_Rating;
				
				//pSend->RoleData.n8GuildPos = EGMP_Null;
				//if(pRemoteRole->IsInGuild())
				//{
					//guild *pGuild = g_guild_manager.get_guild(pRemoteRole->GetGuildID());
					//if(VALID_POINT(pGuild))
					//{
						//pSend->RoleData.dwSymbolValue = pGuild->get_guild_att().dwValue;
						//memcpy(pSend->RoleData.szText, pGuild->get_guild_att().szText, sizeof(pSend->RoleData.szText));
						/*tagGuildMember *pMember = pGuild->get_member(pRemoteRole->GetID());
						if(VALID_POINT(pMember))
						{
							pSend->RoleData.n8GuildPos = pMember->eGuildPos;
						}*/
						//pSend->RoleData.dwChangeSymbolTime = pGuild->get_guild_att().dwChangeSymbolTime;
					//}
				//}

				// 骑乘宠物属性
				//pSend->RoleData.dwMountPetID = INVALID_VALUE;
				//pSend->RoleData.dwMountPetTypeID = INVALID_VALUE;

				//坐骑
				pSend->RoleData.dwMountRideTypeID = pRemoteRole->GetRaidMgr().getRaidMode();
				pSend->RoleData.bySolidateLevel = pRemoteRole->GetRaidMgr().getLevel();

				//pSend->RoleData.dwCarryID = pRemoteRole->GetCarryID( );

				//师傅ID
				//pSend->RoleData.dwMasterID = pRemoteRole->get_master_id( );

				// 属性
				pSend->RoleData.nAtt[ERRA_MaxHP]		= pRemoteRole->GetAttValue(ERA_MaxHP);
				pSend->RoleData.nAtt[ERRA_HP]			= pRemoteRole->GetAttValue(ERA_HP);
				pSend->RoleData.nAtt[ERRA_MaxMP]		= pRemoteRole->GetAttValue(ERA_MaxMP);
				pSend->RoleData.nAtt[ERRA_MP]			= pRemoteRole->GetAttValue(ERA_MP);
				pSend->RoleData.nAtt[ERRA_Rage]			= pRemoteRole->GetAttValue(ERA_Love);
				pSend->RoleData.nAtt[ERRA_Speed_XZ]		= pRemoteRole->GetAttValue(ERA_Speed_XZ);
				pSend->RoleData.nAtt[ERRA_Speed_Y]		= pRemoteRole->GetAttValue(ERA_Speed_Y);
				pSend->RoleData.nAtt[ERRA_Speed_Swim]	= pRemoteRole->GetAttValue(ERA_Speed_Swim);
				pSend->RoleData.nAtt[ERRA_Speed_Mount]	= pRemoteRole->GetAttValue(ERA_Speed_Mount);
				pSend->RoleData.nAtt[ERRA_Shape]		= pRemoteRole->GetAttValue(ERA_Shape);
				//gx add 2013.5.31 查看远程玩家基本属性、
				pSend->RoleData.nAtt[ERRA_HitRate] = pRemoteRole->GetAttValue(ERA_HitRate);
				pSend->RoleData.nAtt[ERRA_Dodge] = pRemoteRole->GetAttValue(ERA_Dodge);
				pSend->RoleData.nAtt[ERRA_Crit_Rate] = pRemoteRole->GetAttValue(ERA_Crit_Rate);
				pSend->RoleData.nAtt[ERRA_UnCrit_Rate] = pRemoteRole->GetAttValue(ERA_UnCrit_Rate);
				pSend->RoleData.nAtt[ERRA_ExAttackMin] = pRemoteRole->GetAttValue(ERA_ExAttackMin);
				pSend->RoleData.nAtt[ERRA_ExAttackMax] = pRemoteRole->GetAttValue(ERA_ExAttackMax);
				pSend->RoleData.nAtt[ERRA_InAttackMin] = pRemoteRole->GetAttValue(ERA_InAttackMin);
				pSend->RoleData.nAtt[ERRA_InAttackMax] = pRemoteRole->GetAttValue(ERA_InAttackMax);
				pSend->RoleData.nAtt[ERRA_ArmorEx] = pRemoteRole->GetAttValue(ERA_ArmorEx);
				pSend->RoleData.nAtt[ERRA_ArmorIn] = pRemoteRole->GetAttValue(ERA_ArmorIn);
				pSend->RoleData.nAtt[ERRA_ExAttack] = pRemoteRole->GetAttValue(ERA_ExAttack);
				pSend->RoleData.nAtt[ERRA_ExDefense] = pRemoteRole->GetAttValue(ERA_ExDefense);
				pSend->RoleData.nAtt[ERRA_InAttack] = pRemoteRole->GetAttValue(ERA_InAttack);
				pSend->RoleData.nAtt[ERRA_InDefense] = pRemoteRole->GetAttValue(ERA_InDefense);
				pSend->RoleData.nAtt[ERRA_Luck] = pRemoteRole->GetAttValue(ERA_Luck);
				//end
				// 战功
				//pSend->RoleData.n32CurExploits = pRemoteRole->GetCurExploits( );

				// 对远端玩家公开信息设置
				//pSend->RoleData.sRemoteOpenSet			= pRemoteRole->GetRemoteOpenSet();

				// 当前称号 gx modify 2013.10.31
				pSend->RoleData.u16CurActTitleID[0]		= pRemoteRole->GetTitleMgr()->GetActiviteTitle(0);
				pSend->RoleData.u16CurActTitleID[1]		= pRemoteRole->GetTitleMgr()->GetActiviteTitle(1);
				pSend->RoleData.u16CurActTitleID[2]		= pRemoteRole->GetTitleMgr()->GetActiviteTitle(2);
				for (INT i = 0;i < 3;i++)
				{
					if (0 == pRemoteRole->GetTitleMgr()->GetShowActiviteTitle(i))
					{
						pSend->RoleData.u16CurActTitleID[i] = INVALID_VALUE;
					}
				}
				//end
				// 角色阵营
				//pSend->RoleData.e_role_camp		=	pRemoteRole->get_role_camp();
				//pSend->RoleData.e_temp_role_camp =  pRemoteRole->get_temp_role_camp();

				//双修对象
				pSend->RoleData.dwCompracticePartner = pRemoteRole->GetComPracticePartner();
				pSend->RoleData.nVIPLevel = pRemoteRole->GetVIPLevel();
				pSend->RoleData.dwRedZuiFlag = pRemoteRole->GetScriptData(REDZUI_FLAG_INDEX);
				// 外观
				pSend->RoleData.sDisplaySet				= pRemoteRole->GetDisplaySet();
				get_fast_code()->memory_copy(&pSend->RoleData.Avatar, pRemoteRole->GetAvatar(), sizeof(tagAvatarAtt));
				get_fast_code()->memory_copy(&pSend->RoleData.AvatarEquip, &pRemoteRole->GetAvatarEquip(), sizeof(tagAvatarEquip));
				
				//pSend->RoleData.n_god_level		=	pRemoteRole->getGodLevel();

				// 状态列表
				pSend->RoleData.nBuffNum = nBuffNum;
				if( nBuffNum > 0 )
				{
					pRemoteRole->GetAllBuffMsgInfo(pSend->RoleData.Buff, nBuffNum);
				}

				SendMessage(pSend, pSend->dw_size);

				MDEL_MSG(pSend);
			}
			else
			{
				list_error.push_back(p_receive->dw_role_id[n]);
			}
		}

		else if( IS_CREATURE(p_receive->dw_role_id[n]))
		{
			Creature* pCreature = pMap->find_creature(p_receive->dw_role_id[n]);
			if( VALID_POINT(pCreature) )
			{
				INT nBuffNum = pCreature->GetBuffNum();
				DWORD dw_size = sizeof(NET_SIS_get_remote_creature_state) + ((nBuffNum > 0) ? (nBuffNum - 1)*sizeof(tagBuffMsgInfo) : 0);

				// 发送远程生物属性同步消息
				CREATE_MSG(pSend, dw_size, NET_SIS_get_remote_creature_state);
				
				pSend->CreatureData.dwID = pCreature->GetID();
				pSend->CreatureData.dw_data_id = pCreature->GetTypeID();
				pSend->CreatureData.nLevel = pCreature->get_level();
			
				// 位置
				Vector3 vPos = pCreature->GetCurPos();
				pSend->CreatureData.fPos[0] = vPos.x;
				pSend->CreatureData.fPos[1] = vPos.y;
				pSend->CreatureData.fPos[2] = vPos.z;

				// 状态
				pSend->CreatureData.dwState = pCreature->GetState();

				// 所属
				pSend->CreatureData.dwTaggedOwner = pCreature->GetTaggedOwner();

				// 朝向
				Vector3 vFace = pCreature->GetFaceTo();
				pSend->CreatureData.fFaceTo[0] = vFace.x;
				pSend->CreatureData.fFaceTo[1] = vFace.y;
				pSend->CreatureData.fFaceTo[2] = vFace.z;

				// 属性
				pSend->CreatureData.nAtt[ERRA_MaxHP]		= pCreature->GetAttValue(ERA_MaxHP);
				pSend->CreatureData.nAtt[ERRA_HP]			= pCreature->GetAttValue(ERA_HP);
				pSend->CreatureData.nAtt[ERRA_MaxMP]		= pCreature->GetAttValue(ERA_MaxMP);
				pSend->CreatureData.nAtt[ERRA_MP]			= pCreature->GetAttValue(ERA_MP);
				pSend->CreatureData.nAtt[ERRA_Rage]			= pCreature->GetAttValue(ERA_Love);
				pSend->CreatureData.nAtt[ERRA_Speed_XZ]		= pCreature->GetAttValue(ERA_Speed_XZ);
				pSend->CreatureData.nAtt[ERRA_Speed_Y]		= pCreature->GetAttValue(ERA_Speed_Y);
				pSend->CreatureData.nAtt[ERRA_Shape]		= pCreature->GetAttValue(ERA_Shape);
				
				//pSend->CreatureData.bCanBeAttack = pCreature->IsCanBeAttack();
				pSend->CreatureData.eCreType = pCreature->GetCreatureType();
			
				pSend->CreatureData.dwTargetID = pCreature->GetMarsterID();
				
				
				// 所属帮会
				pSend->CreatureData.dwGuildID	= pCreature->GetGuildID();
				
				//pSend->CreatureData.dwPlantYield = pCreature->GetCurYield();
				//pSend->CreatureData.dwPlantMaxYield = pCreature->GetMaxYield();
				//pSend->CreatureData.dwPlantTime = pCreature->GetPlantTime();
				pSend->CreatureData.bDynamic = (pCreature->GetSpawnType() == ECST_Dynamic);
				//pCreature->GetPlantRoleName(pSend->CreatureData.szPlantRole);

				// 状态列表
				pSend->CreatureData.nBuffNum = nBuffNum;
				if( nBuffNum > 0 )
				{
					pCreature->GetAllBuffMsgInfo(pSend->CreatureData.Buff, nBuffNum);
				}

				// 门类型加上打开状态信息
				//if ( IS_DOOR(p_receive->dw_role_id[n]) )
				//{
				//	pSend->CreatureData.bOpen = true;
				//	pSend->CreatureData.dwMapObjID = 0;
				//}

				SendMessage(pSend, pSend->dw_size);

				MDEL_MSG(pSend);
			}
			else
			{
				list_error.push_back(p_receive->dw_role_id[n]);
			}
		}
		else if (IS_PET(p_receive->dw_role_id[n]))
		{
			Pet* pPet = pMap->find_pet(p_receive->dw_role_id[n]);
			if (VALID_POINT(pPet))
			{
				NET_SIS_get_remote_pet_state send;
				send.PetData.dwID		= pPet->GetID();
				send.PetData.dwProtoID	= pPet->GetTypeID();
				send.PetData.uState.byPetState		= pPet->GetPetState();
				Role* pMaster			= pPet->GetMaster();
				send.PetData.dw_role_id	= VALID_POINT(pMaster) ? pMaster->GetID() : INVALID_VALUE;
				
				send.PetData.nShape	= pPet->GetAttValue(ERA_Shape);
				ASSERT(VALID_POINT(pPet->GetSoul()));
				send.PetData.nColor = pPet->GetSoul()->GetPetAtt().GetAttVal(epa_color);
				send.PetData.nLevel = pPet->GetSoul()->GetPetAtt().GetVLevel();
				send.PetData.nQuality = pPet->GetSoul()->GetPetAtt().GetAttVal(epa_quality);
				SendMessage(&send, send.dw_size);
			}
			else
			{
				list_error.push_back(p_receive->dw_role_id[n]);
			}
		}
	}

	if(list_error.size())
	{
		DWORD dw_message_size = sizeof(NET_SIS_get_remote_state_error) + (list_error.size()-1) * sizeof(DWORD);

		CREATE_MSG(pSend, dw_message_size, NET_SIS_get_remote_state_error);

		pSend->n_num = list_error.size();

		package_list<DWORD>::list_iter iter = list_error.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT n_num = 0;
		while(list_error.find_next(iter, dw_role_id))
		{
			pSend->dw_role_id[n_num] = dw_role_id;
			n_num++;
		}

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 处理玩家得到其他人的RoleID
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetID(tag_net_message *pCmd)
{
	NET_SIC_role_get_id* p_receive = (NET_SIC_role_get_id*)pCmd;
	DWORD dwNameCrc = p_receive->dwRoleNameCrc;
	DWORD dw_role_id = g_roleMgr.get_role_id(dwNameCrc);

	NET_SIS_role_get_id send;
	send.dwRoleNameCrc = dwNameCrc;
	send.dw_role_id = dw_role_id;
	if (VALID_VALUE(dw_role_id))
		g_roleMgr.get_role_name(dw_role_id, send.szName);

	SendMessage(&send, send.dw_size);
	return 0;
}

//-----------------------------------------------------------------------------
// 通过NameID得到玩家的名字
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetNameByNameID(tag_net_message* pCmd)
{
	
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_name_by_nameid);
	NET_SIC_get_name_by_nameid * p_receive = ( NET_SIC_get_name_by_nameid * ) pCmd ;  
	NET_SIS_get_name_by_nameid send;
	send.bResult = TRUE;
	send.dwNameID = p_receive->dwNameID;
	g_roleMgr.get_role_name_by_name_id(p_receive->dwNameID, send.szName);

	SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 客户端获取多个玩家名字
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetSomeName(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_role_set_some_name);
	NET_SIC_role_set_some_name * p_receive = ( NET_SIC_role_set_some_name * ) pCmd ;  
	INT		n_num = p_receive->n_num;

	if(n_num <= 0)
		return 0;

	if(n_num > 50)
		return 0;

	DWORD	dw_size = sizeof(NET_SIS_role_get_some_name) + (n_num - 1) * sizeof(tagRoleIDName);
	CREATE_MSG(pSend, dw_size, NET_SIS_role_get_some_name);

	pSend->nUserData = p_receive->nUserData;
	pSend->n_num = n_num;

	for(INT n = 0; n < n_num; ++n)
	{
		pSend->IdName[n].dwID = p_receive->dwAllID[n];
		g_roleMgr.get_role_name(p_receive->dwAllID[n], pSend->IdName[n].szName);
	}

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return 0;
}

//------------------------------------------------------------------------------
// 投点
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBidAttPoint(tag_net_message* pCmd)
{
	NET_SIC_role_att_point* p_receive = (NET_SIC_role_att_point*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->BidAttPoint(p_receive->nAttPointAdd);

	NET_SIS_role_att_point send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------
// 洗点
//-------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleClearAttPoint(tag_net_message* pCmd)
{
	NET_SIC_clear_att_point* p_receive = (NET_SIC_clear_att_point*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->ClearAttPoint(p_receive->n64ItemID);

	NET_SIS_clear_att_point send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------
// 角色外观显示模式设置
//-------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetFashion(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_fashion);
	NET_SIC_fashion * p = ( NET_SIC_fashion * ) pCmd ;  
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获取地图
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}
	
	// 检查是否和当前显示模式相同
	if(pRole->GetDisplaySet().bFashionDisplay == p->bFashion)
	{
		return 0;
	}

	// 设置显示模式
	pRole->SetFashionMode(p->bFashion);

	// 向周围玩家发送新的外观信息
	NET_SIS_avatar_equip send;
	send.dw_role_id		= pRole->GetID();
	send.bFashion		= p->bFashion;
	send.sAvatarEquip	= pRole->GetAvatarEquip();
	pMap->send_big_visible_tile_message(pRole, &send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------
// 角色外观显示部位设置
//-------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetDisplay(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_role_set_display);
	NET_SIC_role_set_display * p = ( NET_SIC_role_set_display * ) pCmd ;  
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获取地图
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 检查是否和当前部位设置属性相同
	const tagDisplaySet& sDisplaySet = pRole->GetDisplaySet();
	if(sDisplaySet.bHideBack == p->sDisplaySet.bHideBack
		&& sDisplaySet.bHideFace == p->sDisplaySet.bHideFace
		&& sDisplaySet.bHideHead == p->sDisplaySet.bHideHead
		&& sDisplaySet.bFlyBack == p->sDisplaySet.bFlyBack)
	{
		return 0;
	}

	// 设置部位显示属性
	pRole->SetDisplaySet(p->sDisplaySet.bHideHead, p->sDisplaySet.bHideFace, p->sDisplaySet.bHideBack, p->sDisplaySet.bFlyBack);

	// 向周围玩家发送消息
	NET_SIS_role_set_display send;
	send.dw_role_id		= pRole->GetID();
	send.sDisplaySet	= sDisplaySet;
	pMap->send_big_visible_tile_message(pRole, &send, send.dw_size);

	return 0;
}

// 获取玩家签名
DWORD PlayerSession::HandleGetRoleSignature(tag_net_message* pCmd)
{
	NET_SIC_role_get_signature_name* p_receive = (NET_SIC_role_get_signature_name*)pCmd;
	
	Role* pRole = g_roleMgr.get_role(p_receive->dw_role_id);
	if (!VALID_POINT(pRole)) return 0;

	NET_SIS_role_get_signature_name send;

	g_roleMgr.get_role_signature_name(p_receive->dw_role_id, send.szName);

	send.dw_role_id = p_receive->dw_role_id;
	send.by_sex = pRole->GetSex();
	send.nLevel = pRole->get_level();
	send.nClass = pRole->GetClass();
	SendMessage(&send, send.dw_size);
	return 0;
}

// 修改玩家签名
DWORD PlayerSession::HandleChangeSignature(tag_net_message* pCmd)
{
	NET_SIC_role_set_signature_name* p_receive = (NET_SIC_role_set_signature_name*)pCmd;
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;
	m_pRole->SetSignature(p_receive->szName);

	NET_SIS_role_get_signature_name send;
	send.dw_role_id = m_pRole->GetID();
	_tcsncpy(send.szName, p_receive->szName, X_SHORT_NAME);
	send.by_sex = m_pRole->GetSex();
	send.nLevel = m_pRole->get_level();
	send.nClass = m_pRole->GetClass();
	SendMessage(&send, send.dw_size);
	return 0;
}

DWORD PlayerSession::HandleConsumeReward(tag_net_message* p_cmd)
{
	NET_SIC_get_consume_reward *pProtocol = (NET_SIC_get_consume_reward*)p_cmd;

	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;
	m_pRole->ConsumeReward( pProtocol->index );

	return 0;
}


