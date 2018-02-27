
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//处理GM命令
#include "StdAfx.h"
#include "../../common/WorldDefine/gm_protocol.h"
#include "../../common/WorldDefine/currency_define.h"
#include "../common/ServerDefine/vcard_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/reputation.h"
#include "../../common/WorldDefine/quest_protocol.h"
#include "../../common/WorldDefine/combat_protocol.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../../common/WorldDefine/guerdon_quest_protocol.h"
#include "../../common/WorldDefine/role_god_level.h"

#include "role.h"
#include "player_session.h"
#include "gm_net_cmd_mgr.h"
#include "item_creator.h"
#include "map_creator.h"
#include "role_mgr.h"
#include "world_session.h"
#include "pet_pocket.h"
#include "db_session.h"
#include "gm_policy.h"
#include "title_mgr.h"
#include "quest_mgr.h"
#include "mall.h"
#include "vip_netbar.h"
#include "guild.h"
#include "guild_manager.h"
#include "guild_commodity.h"
#include "world_session.h"
#include "creature.h"
#include "map_mgr.h"
//added by mmz at 2010.9.19
#include "role_mgr.h"
#include "paimai_manager.h"
#include "bank_manager.h"
#include "master_prentice_mgr.h"
#include "../../common/WorldDefine/master_prentice_protocol.h"
#include "../common/ServerDefine/paimai_server_define.h"
#include "../common/ServerDefine/bank_server_define.h"
#include "guerdon_quest_mgr.h"
#include "pvp_mgr.h"
#include "login_session.h"
#include "RankMgr.h"
//-----------------------------------------------------------------------------
// 处理GM通用命令
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGMCommand(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_gm_command);
	NET_SIC_gm_command * p_receive = ( NET_SIC_gm_command * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	//m_GMCommandMgr.SetGMCmdID(pRole->GetID());
	
	// 调用相应控制台命令接口
	return m_GMCommandMgr.Excute(p_receive->szCommand, pRole);
}

//-----------------------------------------------------------------------------
// 通过TypeID获得物品(item dw_data_id nItemNum nQuality)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleCreateItem(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_data_id	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	INT16 n16Num	= (INT16)_ttoi((LPCTSTR)vectParam[1]);
	INT32 nQlty		= (INT32)_ttoi((LPCTSTR)vectParam[2]);
	BOOL bBind = (BOOL)_ttoi((LPCTSTR)vectParam[3]);

	if(n16Num <= 0)
	{
		n16Num = 1;
	}
	
	tagItem *pNewItem = ItemCreator::Create(EICM_GM, pGM->GetID(), dw_data_id, n16Num);
	if(!VALID_POINT(pNewItem))
	{
		return INVALID_VALUE;
	}

	if(MIsEquipment(pNewItem->dw_data_id))
	{
		if(nQlty == INVALID_VALUE)
		{
			// 不鉴定	
		}
		else
		{
			ItemCreator::IdentifyEquip((tagEquip*)pNewItem, (EItemQuality)nQlty);
		}
	}

	//if (bBind)
	//{
	//	pNewItem->byBind = EBS_SYSTEM_Bind;
	//	pNewItem->bCreateBind = bBind;
	//}

	pGM->GetItemMgr().Add2Bag(pNewItem, elcid_gm_create_item, TRUE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 通过TypeID在线所有人获得物品并装备(allitem dw_data_id nQuality)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleAllRoleEquip(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_data_id	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	INT32 nQlty		= (INT32)_ttoi((LPCTSTR)vectParam[1]);
	INT16 n16Num	= 1;

	INT nIndex = -1;
	Role* pRole = NULL;
	role_mgr::RoleMap::map_iter iter = g_roleMgr.get_role_map().begin();
	while(g_roleMgr.get_role_map().find_next(iter, nIndex))
	{
		pRole = g_roleMgr.get_local_role_mgr().get_role(nIndex);
		if(VALID_POINT(pRole))
		{
			tagItem *pNewItem = ItemCreator::Create(EICM_GM, pGM->GetID(), dw_data_id, n16Num);
			if(!VALID_POINT(pNewItem))
			{
				return INVALID_VALUE;
			}

			if(MIsEquipment(pNewItem->dw_data_id))
			{
				if(nQlty == INVALID_VALUE)
				{
					// 不鉴定	
				}
				else
				{
					ItemCreator::IdentifyEquip((tagEquip*)pNewItem, (EItemQuality)nQlty);
				}
			}

			pRole->GetItemMgr().Add2Bag(pNewItem, elcid_gm_create_item, TRUE);
			pRole->Equip(pNewItem->n64_serial, pNewItem->pEquipProto->eEquipPos);
		}	
	}

	print_message(_T("add allitem end \n"));
	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得金钱(silver nGold nSilver nCopper)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetSilver(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT32 nGold		= (INT32)_ttoi((LPCTSTR)vectParam[0]);
	INT32 nSilver	= (INT32)_ttoi((LPCTSTR)vectParam[1]);
	INT32 nCopper	= (INT32)_ttoi((LPCTSTR)vectParam[2]);

	if(nGold < 0 || nSilver < 0)
	{
		return INVALID_VALUE;
	}

	INT64 nTotalSilver = MGold2Silver(nGold) + MSilver2Copper(nSilver) + (INT64)nCopper;
	pGM->GetCurMgr().IncBagSilver(nTotalSilver, elcid_gm_get_money);
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得绑定金钱(silver nGold nSilver nCopper)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandGetBindSilver(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT32 nGold		= (INT32)_ttoi((LPCTSTR)vectParam[0]);
	INT32 nSilver	= (INT32)_ttoi((LPCTSTR)vectParam[1]);
	INT32 nCopper	= (INT32)_ttoi((LPCTSTR)vectParam[2]);

	if(nGold < 0 || nSilver < 0)
	{
		return INVALID_VALUE;
	}

	INT64 nTotalSilver = MGold2Silver(nGold) + MSilver2Copper(nSilver) + (INT64)nCopper;
	pGM->GetCurMgr().IncBagBindSilver(nTotalSilver, elcid_gm_get_money);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得元宝(yuanbao nYuanBao)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetYuanBao(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT32 nYuanBao = (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if(nYuanBao < 0)
	{
		return INVALID_VALUE;
	}

	pGM->GetCurMgr().IncBaiBaoYuanBao(nYuanBao, elcid_gm_get_yuanbao);
	
	pGM->GetSession()->SetTotalRecharge(nYuanBao);

	if (pGM->GetScript())
	{
		pGM->GetScript()->OnRecharge(pGM, nYuanBao, pGM->GetTotalRecharge());
	}

	NET_DB2C_update_yuanbao_recharge send;
	send.dw_account_id = pGM->GetSession()->GetSessionID();
	send.n32_yuanbao = nYuanBao;
	g_dbSession.Send(&send, send.dw_size);


	NET_DB2C_update_total_recharge send_total;
	send_total.dw_account_id = pGM->GetSession()->GetSessionID();
	send_total.n32_yuanbao = nYuanBao;
	g_dbSession.Send(&send_total, send_total.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得赠点(exvolume nExVolume)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetExVolume(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT32 nExVolume = (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if(nExVolume < 0)
	{
		return INVALID_VALUE;
	}

	pGM->GetCurMgr().IncExchangeVolume(nExVolume, elcid_gm_get_exvolume);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 通过TypeID获得物品(clearbag)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleClearBag(const std::vector<DWORD>& vectParam, Role* pGM)
{
	ItemMgr			&itemMgr	= pGM->GetItemMgr();

	tagItem *pItem = NULL;
	INT16 n16SzBag = itemMgr.GetBagCurSize();
	for(INT16 i=0; i<n16SzBag; ++i)
	{
		if(itemMgr.GetBagFreeSize() == n16SzBag)
		{
			break;
		}

		pItem = itemMgr.GetBagItem(i);
		if(!VALID_POINT(pItem))
		{
			continue;
		}

		itemMgr.DelFromBag(pItem->n64_serial, elcid_gm_clear_bag);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 移动到指定地点(goto mapName x z)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGoto(const std::vector<DWORD>& vectParam, Role* pGM)
{
	Map *pMap = pGM->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}
	
	DWORD dwMapID = (DWORD)get_tool()->crc32((LPCTSTR)vectParam[0]);
	FLOAT fX = (FLOAT)_ttoi((LPCTSTR)vectParam[1]) * TILE_SCALE;
	FLOAT fZ = (FLOAT)_ttoi((LPCTSTR)vectParam[2]) * TILE_SCALE;

	if( FALSE == g_mapCreator.is_map_exist(dwMapID) )
		dwMapID = pGM->GetMapID();

	if( pGM->GotoNewMap(dwMapID, fX, GET_MAX_HEIGHT_Y, fZ) )
	{
		return E_Success;
	}
	else
	{
		return INVALID_VALUE;
	}
}

//-----------------------------------------------------------------------------
// GM移动到玩家身边(gotorole szRoleName)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGotoRole(const std::vector<DWORD>& vectParam, Role* pGM)
{
	//Map *pMap = pGM->get_map();
	//if(!VALID_POINT(pMap))
	//{
	//	return INVALID_VALUE;
	//}

	DWORD dwRoleNameCrc = (DWORD)get_tool()->crc32((LPCTSTR)vectParam[0]);

	Role *pRole = g_roleMgr.get_role(g_roleMgr.get_role_id(dwRoleNameCrc));
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Vector3& vectGMPos = pRole->GetCurPos();

	if(pGM->GotoNewMap(pRole->GetMapID(), vectGMPos.x, GET_MAX_HEIGHT_Y, vectGMPos.z))
	{
		return E_Success;
	}

	return INVALID_VALUE;
}

//-----------------------------------------------------------------------------
// GM移动到副本玩家身边(gotorole szRoleName)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGotoRoleInst(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwRoleNameCrc = (DWORD)get_tool()->crc32((LPCTSTR)vectParam[0]);
	BOOL bTeamOrRole = (BOOL)_ttoi((LPCTSTR)vectParam[1]);

	Role *pRole = g_roleMgr.get_role(g_roleMgr.get_role_id(dwRoleNameCrc));
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dwMapID = INVALID_VALUE;
	DWORD dwInstanceID = INVALID_VALUE;

	if(bTeamOrRole)
	{
		DWORD dwTeamID = pRole->GetTeamID();
		if(dwTeamID == INVALID_VALUE)
			return INVALID_VALUE;
		const Team* pTeam = g_groupMgr.GetTeamPtr(dwTeamID);
		if(!VALID_POINT(pTeam))
			return INVALID_VALUE;
		dwMapID = pTeam->get_own_instance_mapid();
		dwInstanceID = pTeam->get_own_instanceid();
	}
	else
	{
		dwMapID = pRole->GetMyOwnInstanceMapID();
		dwInstanceID = pRole->GetMyOwnInstanceID();
	}

	if(dwMapID == INVALID_VALUE || dwInstanceID == INVALID_VALUE)
		return INVALID_VALUE;

	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dwMapID);
	if( !VALID_POINT(p_map_manager) ) return NULL;

	if(p_map_manager->IsNormalMap())
		return INVALID_VALUE;

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	pGM->set_gm_instance_id(dwInstanceID);

	Vector3& vectGMPos = pRole->GetCurPos();

	if(pGM->GotoNewMap(pRole->GetMapID(), vectGMPos.x, vectGMPos.y, vectGMPos.z))
	{
		return E_Success;
	}

	return INVALID_VALUE;
}

//-----------------------------------------------------------------------------
// 把角色拉到GM身边(moverole szRoleName)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleMoveRole(const std::vector<DWORD>& vectParam, Role* pGM)
{
	//Map *pMap = pGM->get_map();
	//if(!VALID_POINT(pMap))
	//{
	//	return INVALID_VALUE;
	//}

	DWORD dwRoleNameCrc = (DWORD)get_tool()->crc32((LPCTSTR)vectParam[0]);

	Role *pRole = g_roleMgr.get_role(g_roleMgr.get_role_id(dwRoleNameCrc));
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Vector3& vectGMPos = pGM->GetCurPos();

	if(pRole->GotoNewMap(pGM->GetMapID(), vectGMPos.x, GET_MAX_HEIGHT_Y, vectGMPos.z))
	{
		return E_Success;
	}

	return INVALID_VALUE;
}

//-----------------------------------------------------------------------------
// 充经验(fillexp nExp)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleFillExp(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->ExpChange(_ttoi((LPCTSTR)vectParam[0]), FALSE, TRUE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 升级(fill nLevel)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleFillLevel(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nLevel = _ttoi((LPCTSTR)vectParam[0]);
	
	// 判断指定等级是否有效，不能指定0级
	if ( nLevel > 0 && nLevel <= ROLE_LEVEL_LIMIT)
		pGM->LevelChange(_ttoi((LPCTSTR)vectParam[0]), FALSE, TRUE);

	// 如果角色处于非死亡状态，返回
	if( !pGM->IsInState(ES_Dead) )
		return E_Success;
	else  // 如果角色处于死亡状态，原地复活
	{
		pGM->Revive( ERRT_Locus, 0, FALSE);
		
		NET_SIS_role_revive send;
		send.dw_role_id		= pGM->GetID();
		send.dw_error_code	= 0;

		if( VALID_POINT(pGM->get_map()) )
		{
			pGM->get_map()->send_big_visible_tile_message(pGM, &send, send.dw_size);
		}
	
		// 复活到指定地点
		pGM->GotoNewMap(pGM->m_Revive.dwMapID, pGM->m_Revive.fX, pGM->m_Revive.fY, pGM->m_Revive.fZ);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 增加装备潜力值(equippot n16Index nValue) n16Index背包位置索引(从0开始）  nValue潜力值增加量
//-----------------------------------------------------------------------------
//DWORD GMCommandMgr::HandleEquipPotInc(const std::vector<DWORD>& vectParam, Role* pGM)
//{
//	INT16	n16Index = (INT16)_ttoi((LPCTSTR)vectParam[0]);
//	INT		nValue = (INT)_ttoi((LPCTSTR)vectParam[1]);
//
//	tagEquip *pEquip = 	(tagEquip*)pGM->GetItemMgr().GetBagItem(n16Index);
//
//	if(!VALID_POINT(pEquip) || !MIsEquipment(pEquip->dw_data_id))
//		return INVALID_VALUE;
//
//	pEquip->ChangePotVal(nValue);
//	pGM->GetItemMgr().UpdateEquipSpec(*pEquip);
//
//	return E_Success;
//}

//-----------------------------------------------------------------------------
// 踢人
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleKickRole(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwRoleNameCrc = (DWORD)get_tool()->crc32((LPCTSTR)vectParam[0]);

	Role *pRole = g_roleMgr.get_role(g_roleMgr.get_role_id(dwRoleNameCrc));
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	PlayerSession *pSession = pRole->GetSession();
	if(VALID_POINT(pSession))
	{
		g_worldSession.Kick(pSession->GetInternalIndex());
		pSession->SetKicked();
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 踢人角色ID
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleKickRoleID(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_role_id = _ttoi((LPCTSTR)vectParam[0]);

	Role *pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	PlayerSession *pSession = pRole->GetSession();
	if(VALID_POINT(pSession))
	{
		g_worldSession.Kick(pSession->GetInternalIndex());
		pSession->SetKicked();
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 调整可登陆玩家人数
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleResizeOnlineNum(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT	nValue = (INT)_ttoi((LPCTSTR)vectParam[0]);

	if(nValue < 100)
	{
		return INVALID_VALUE;
	}

	g_worldSession.SetPlayerNumLimit(nValue);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 重新加载脚本
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleReloadScripts(const std::vector<DWORD>& vectParam, Role* pGM)
{
	g_ScriptMgr.AddEvent(INVALID_VALUE, EVT_Script_Reload, 0, NULL);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 修改脚本使用的RoleData的值
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleChangeScriptData(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nIndex 	= _ttoi((LPCTSTR)vectParam[0]);
	DWORD dwValue= (DWORD)_ttoi((LPCTSTR)vectParam[1]);

	if (nIndex < ESD_Role && nIndex >= 0)
	{
		pGM->SetScriptData(nIndex, dwValue);
		return E_Success;
	}

	return E_SystemError;
}
//-----------------------------------------------------------------------------
// 开双倍
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleDouble( const std::vector<DWORD>& vectParam, Role* pGM )
{
	if (vectParam.size() < 4)
		return INVALID_VALUE;

	INT32 nPara1	= (INT32)_ttoi((LPCTSTR)vectParam[0]);
	INT32 nPara2	= (INT32)_ttoi((LPCTSTR)vectParam[1]);
	INT32 nPara3	= (INT32)_ttoi((LPCTSTR)vectParam[2]);
	INT32 nPara4	= (INT32)_ttoi((LPCTSTR)vectParam[3]);

	g_GMPolicy.SetRate((EGMDoubleType)nPara1, (DWORD)nPara2, (DWORD)nPara3, (DWORD)nPara4);

	return E_Success;
}

// 测试氏族数据用
DWORD GMCommandMgr::HandleClanData(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 3)
		return INVALID_VALUE;

	INT32 nPara1	= (INT32)_ttoi((LPCTSTR)vectParam[0]);
	INT32 nPara2	= (INT32)_ttoi((LPCTSTR)vectParam[1]);
	INT32 nPara3	= (INT32)_ttoi((LPCTSTR)vectParam[2]);

	if (!VALID_POINT(pGM))
		return INVALID_VALUE;

	return pGM->GetClanData().DynamicTest(nPara1, (ECLanType)nPara2, nPara3);
}

// 测试氏族数据用
DWORD GMCommandMgr::HandleVipNetBar(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 3)
		return INVALID_VALUE;

	DWORD dwPara0	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	DWORD dwPara1	= (DWORD)_ttoi((LPCTSTR)vectParam[1]);
	LPCTSTR szPara2	= (LPCTSTR)vectParam[2];

	if (!VALID_POINT(pGM))
		return INVALID_VALUE;

	return g_VipNetBarMgr.DynamicTest(dwPara0, dwPara1, szPara2);
}

// 测试宠物
DWORD GMCommandMgr::HandlePet(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 3)
		return INVALID_VALUE;

	INT32 nPara1	= (INT32)_ttoi((LPCTSTR)vectParam[0]);
	INT32 nPara2	= (INT32)_ttoi((LPCTSTR)vectParam[1]);
	INT32 nPara3	= (INT32)_ttoi((LPCTSTR)vectParam[2]);

	if (!VALID_POINT(pGM))
		return INVALID_VALUE;

	return pGM->GetPetPocket()->DynamicTest(nPara1, (ECLanType)nPara2, nPara3);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleAddSkill( const std::vector<DWORD>& vectParam, Role* pGM )
{
	//9001201
	if (vectParam.size() < 1 || !VALID_POINT(pGM))
		return INVALID_VALUE;
	INT32 dw_data_id = (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if (!VALID_POINT(AttRes::GetInstance()->GetSkillProto(dw_data_id)))
	{
		return INVALID_VALUE;
	}
	
	Skill* pCurSkill = pGM->GetSkill(Skill::GetIDFromTypeID(dw_data_id));
	if (VALID_POINT(pCurSkill))
	{
		INT nChangeLevel = Skill::GetLevelFromTypeID(dw_data_id)-pCurSkill->get_level();
		pGM->ChangeSkillLevel(pCurSkill, ESLC_Self, nChangeLevel);
	}
	else
	{
		Skill* pSkill = new Skill(Skill::GetIDFromTypeID(dw_data_id), 0, Skill::GetLevelFromTypeID(dw_data_id), 0, 0, 0);
		pGM->AddSkill(pSkill);
	}

	if( pGM->NeedRecalAtt() )
		pGM->RecalAtt();

	return E_Success;
}

// 帮派测试
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetFund(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 1)
	{
		return INVALID_VALUE;
	}

	INT32 nFund		= (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	pGuild->increase_guild_fund(pGM->GetID(), nFund, elcid_gm_getfund);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetMaterial(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 1)
	{
		return INVALID_VALUE;
	}

	INT32 nMaterial	= (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	pGuild->increase_guild_material(pGM->GetID(), nMaterial, elcid_gm_get_material);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetContribute(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 1)
	{
		return INVALID_VALUE;
	}

	INT32 nContribute	= (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	pGuild->change_member_contribution(pGM->GetID(), nContribute, TRUE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleResetAffairTimes(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	pGuild->get_guild_affair().reset_affair_count();

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGetTael(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 1)
	{
		return INVALID_VALUE;
	}

	INT32 nTael	= (INT32)_ttoi((LPCTSTR)vectParam[0]);

	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}
	guild_commodity* pCommodity = pGuild->get_guild_commerce().get_commodity(pGM->GetID());
	if (!VALID_POINT(pCommodity))
	{
		return INVALID_VALUE;
	}

	pCommodity->increase_tael(nTael);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGuildStatus(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nType = _ttoi((LPCTSTR)vectParam[0]);
	LONG lValue = (LONG)_ttoi((LPCTSTR)vectParam[1]); // 调整的值，不是设置的值

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
		return INVALID_VALUE;

	// 安定度
	else if (nType == 0)
	{
		if (lValue > 0)
		{
			pGuild->increase_guild_peace(pGM->GetID(), lValue, 0);
		}
		else
		{
			pGuild->decrease_guild_peace(pGM->GetID(), -lValue, 0);
		}
	}
	// 声望值
	else if (nType == 1)
	{
		if (lValue > 0)
		{
			pGuild->increase_guild_reputation(pGM->GetID(), lValue, 0);
		}
		else
		{
			pGuild->decrease_guild_reputation(pGM->GetID(), -lValue, 0);
		}
	}
	else
		return E_SystemError;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleGuildFacility(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nType = _ttoi((LPCTSTR)vectParam[0]);
	BYTE byValue = (BYTE)_ttoi((LPCTSTR)vectParam[1]);

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if (!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->change_facility_level((EFacilitiesType)(nType), byValue);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleRoleGuild(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nType = _ttoi((LPCTSTR)vectParam[0]);
	INT32 n32Value = (INT32)_ttoi((LPCTSTR)vectParam[1]);//调整的值，不是设置的值
	
	// 阅历
	if (nType == 0)
	{
		INT32 n32Temp = pGM->GetAttValue(ERA_Knowledge) + n32Value; 
		if (n32Temp > MAX_GUILDMEM_EXP)
			n32Temp = MAX_GUILDMEM_EXP;
		if (n32Temp < 0)
			n32Temp = 0;
		
		pGM->SetAttValue(ERA_Knowledge, n32Temp);
	} 
	// 功勋
	else if (nType == 1)
	{
		guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
		if (!VALID_POINT(pGuild))
			return INVALID_VALUE;

		pGuild->change_member_exploit(pGM->GetID(), n32Value);
	}
	else
		return E_SystemError;

	return E_Success;
}

// 帮派团购测试
DWORD GMCommandMgr::HandleLaunchGP(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 4)
	{
		return INVALID_VALUE;
	}

	DWORD nPara1	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	INT nPara2		= (INT)_ttoi((LPCTSTR)vectParam[1]);
	BYTE nPara3		= (BYTE)_ttoi((LPCTSTR)vectParam[2]);
	BYTE nPara4		= (BYTE)_ttoi((LPCTSTR)vectParam[3]);


	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	return g_mall.lauch_guild_buy(pGM, nPara1, nPara4, nPara2, nPara3);
}

DWORD GMCommandMgr::HandleRespondGP(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if (vectParam.size() < 3)
	{
		return INVALID_VALUE;
	}

	DWORD nPara1	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	DWORD nPara2	= (DWORD)_ttoi((LPCTSTR)vectParam[1]);
	INT nPara3		= (INT)_ttoi((LPCTSTR)vectParam[2]);

	if (!VALID_POINT(pGM))
	{
		return INVALID_VALUE;
	}

	return g_mall.respond_guild_buy(pGM, pGM->GetGuildID(), nPara1, nPara2, nPara3);
}

// 测试名帖
DWORD GMCommandMgr::HandleVCard( const std::vector<DWORD>& vectParam, Role* pGM )
{
	if (vectParam.size() < 1)
		return INVALID_VALUE;

	DWORD dw_role_id	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	if(!VALID_POINT(g_roleMgr.get_role_info(dw_role_id)))
	{
		return INVALID_VALUE;
	}

	NET_DB2C_get_off_line_vcard send;

	send.dw_role_id = dw_role_id;
	send.dw_query_id = pGM->GetID();
	
	g_dbSession.Send(&send, send.dw_size);

	return TRUE;
}

DWORD GMCommandMgr::HandleTitle( const std::vector<DWORD>& vectParam, Role* pGM )
{
	if (vectParam.size() < 3)
		return INVALID_VALUE;

	e_achievement_event	eEvent	= (e_achievement_event)_ttoi((LPCTSTR)vectParam[0]);
	DWORD			dwPara1	= (DWORD)_ttoi((LPCTSTR)vectParam[1]);
	DWORD			dwPara2	= (DWORD)_ttoi((LPCTSTR)vectParam[2]);

	pGM->GetAchievementMgr().UpdateAchievementCriteria(eEvent, dwPara1, dwPara2);

	return TRUE;
}


DWORD GMCommandMgr::HandleSetTitle( const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwTitleID	= (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	
	pGM->SetTitle(dwTitleID);

	return TRUE;
}


//-----------------------------------------------------------------------------
// 添加新的任务
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleAddQuest(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT	nQuestID = _ttoi((LPCTSTR)vectParam[0]);

	const tagQuestProto* pProto = g_questMgr.get_protocol(nQuestID);
	if( !VALID_POINT(pProto) )	return E_QUEST_NOT_EXIST;

	INT nIndex;
	for(nIndex = 0; nIndex < QUEST_MAX_COUNT; ++nIndex)
	{
		if( FALSE == pGM->quest_valid(nIndex) )
		{
			break;
		}
	}
	if(pGM->is_have_quest(nQuestID))
	{
		return E_CanTakeQuest_ALREADY_TAKE;
	}
	if( nIndex >= QUEST_MAX_COUNT )
	{
		return E_CanTakeQuest_FAILED_QUEST_NUM_FULL;
	}
	
	pGM->add_quest(pProto, nIndex);

	return TRUE;
}

//-----------------------------------------------------------------------------
// 清除指定天资类型技能冷却(skillcd talenttype exceptsillid)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleClearSkillCD(const std::vector<DWORD>& vectParam, Role* pGM)
{
	ETalentType	eTalentType		= (ETalentType)_ttoi((LPCTSTR)vectParam[0]);
	DWORD		dwExceptSkillID	= (DWORD)_ttoi((LPCTSTR)vectParam[1]);

	if(eTalentType >= ETT_End || eTalentType < ETT_Null)
	{
		eTalentType = ETT_Null;
	}

	pGM->ClearAllSkillCoodDown(eTalentType, dwExceptSkillID);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 物品冷却开关
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleCoolOff(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->SetObjectCoolMode(TRUE);
	return E_Success;
}

DWORD GMCommandMgr::HandleCoolOn(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->SetObjectCoolMode(FALSE);
	return E_Success;
}

//-----------------------------------------------------------------------------
// 修改角色属性(att eroleatt nval)
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleChangeRoleAtt(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT	nIndex	= _ttoi((LPCTSTR)vectParam[0]);
	INT	nVal	= _ttoi((LPCTSTR)vectParam[1]);

	if(nIndex <= ERA_Null || nIndex >= ERA_End || nIndex == ERA_Pneuma || nIndex == ERA_Technique)
	{
		return INVALID_VALUE;
	}
	
	if ((nIndex == ERA_HP)||(nIndex == ERA_MP)||(nIndex == ERA_Brotherhood)||(nIndex == ERA_Wuhuen)
		||(nIndex == ERA_Knowledge)||(nIndex == ERA_Injury)||(nIndex == ERA_Morale)||(nIndex == ERA_Morality)
		||(nIndex == ERA_Culture)||(nIndex == ERA_AttPoint)||(nIndex == ERA_TalentPoint))
	{
		pGM->SetAttValue(nIndex, nVal);
	}
	else
	{
		pGM->SetBaseAttValue(nIndex, nVal);
		pGM->RecalAtt();
	}

	return E_Success;
}

// 修改速度
DWORD GMCommandMgr::HandleChangSpeed(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT	nVal	= _ttoi((LPCTSTR)vectParam[0]);
	
	pGM->SetBaseAttValue(ERA_Speed_XZ, nVal);
	pGM->RecalAtt();

	return E_Success;
}
//-----------------------------------------------------------------------------
// 修改角色声望值
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleSetReputation(const std::vector<DWORD>& vectParam, Role* pGM)
{
	ECLanType eClan	= (ECLanType)_ttoi((LPCTSTR)vectParam[0]);
	INT	nReputation	= _ttoi((LPCTSTR)vectParam[1]);

	if( eClan <= ECLT_BEGIN || eClan >= ECLT_END )
	{
		return INVALID_VALUE;
	}

	pGM->GetClanData().RepSetVal(eClan, nReputation, TRUE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 在玩家当前坐标刷出怪物　
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleCreateMonster( const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_data_id = _ttoi((LPCTSTR)vectParam[0]);
	BOOL  bCollide = FALSE;
	INT	  dw_num = _ttoi((LPCTSTR)vectParam[1]);
	if(dw_num == 0)
		dw_num = 1;
	
	if(vectParam.size() == 3)
		bCollide = _ttoi((LPCTSTR)vectParam[2]);

	Map* pMap = pGM->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	INT	n_dis = 480;

	Vector3 vPos = pGM->GetCurPos();
	Vector3 vFace = pGM->GetFaceTo();

	if(dw_num == 1)
	{
		Creature* pCreature = pMap->create_creature(dw_data_id, vPos, vFace, INVALID_VALUE, bCollide);
		if(VALID_POINT(pCreature))
			pCreature->SetTagged(pGM->GetID());

		return E_Success;
	}

	for(INT i = 0; i < dw_num; i++)
	{
		Vector3 vTemp = vPos;
		//随机选择一个角度
		FLOAT fAngle	= (get_tool()->tool_rand() % 360) / 360.0f * 3.14f;
		//随机选择一个合理的范围
		FLOAT fDist		= get_tool()->tool_rand() % n_dis;
		
		vTemp.x += sin(fAngle) * fDist;
		vTemp.z += cos(fAngle) * fDist;
		Creature* pCreature = pMap->create_creature(dw_data_id, vTemp, vFace, INVALID_VALUE, bCollide);
		if(VALID_POINT(pCreature))
			pCreature->SetTagged(pGM->GetID());
	}
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// 不计算技能的触发率
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleTriggerOff(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->SetTriggerMode(TRUE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 使用原有的触发率计算
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleTriggerOn(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->SetTriggerMode(FALSE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 清空该角色所有已完成任务
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleClearRoleQuest(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->clear_done_quest();
	return E_Success;
}

//-----------------------------------------------------------------------------
// 指定某任务物的完成状态
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleRoleQuest(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT	nQuestID = _ttoi((LPCTSTR)vectParam[0]);
	BOOL bDone = (BOOL)_ttoi((LPCTSTR)vectParam[1]);

	pGM->add_del_complete_quest(nQuestID, bDone);
	return E_Success;
}

//-----------------------------------------------------------------------------
// 装备的镌刻，使用该命令一次，则镌刻该装备成功一次 gm engrave n64ItemID dwFormulaID 
//-----------------------------------------------------------------------------
//DWORD GMCommandMgr::HandleEngrave(const std::vector<DWORD>& vectParam, Role* pGM)
//{
//	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
//	DWORD dwFormulaID = (DWORD)_ttoi((LPCTSTR)vectParam[1]);
//
//	pGM->GMEngraveEquip(dwFormulaID, n16ItemIndex);
//	return E_Success;
//}

//-----------------------------------------------------------------------------
// 铭文，使用该命令一次，则铭文该装备成功一次
//-----------------------------------------------------------------------------
//DWORD GMCommandMgr::HandlePosy(const std::vector<DWORD>& vectParam, Role* pGM)
//{
//	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
//	DWORD dwFormulaID = (DWORD)_ttoi((LPCTSTR)vectParam[1]);
//	
//	pGM->GMPosyEquip(dwFormulaID, n16ItemIndex);
//	return E_Success;
//}


//-----------------------------------------------------------------------------
// 为装备增加特殊属性
//-----------------------------------------------------------------------------
//DWORD GMCommandMgr::HandleAddAtt2Weapon(const std::vector<DWORD>& vectParam, Role* pGM)
//{
//	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
//	INT nAttType = (INT)_ttoi((LPCTSTR)vectParam[1]); // 0:龙魂属性表 1:龙魂属性里 2:特殊属性
//	DWORD dwAttID = (DWORD)_ttoi((LPCTSTR)vectParam[2]);
//
//	// 找到被强化装备
//	tagEquip *pEquip = (tagEquip*)pGM->GetItemMgr().GetBagItem(n16ItemIndex);
//	if(!VALID_POINT(pEquip))
//		return E_Consolidate_Equip_Not_Exist;
//
//	if(!MIsEquipment(pEquip->dw_data_id))
//		return E_Consolidate_NotEquipment;
//	// 判断要添加的属性是龙魂属性还是特殊属性
//	if (nAttType == 2)
//		pEquip->equipSpec.bySpecAtt = dwAttID;
//
//
//	ItemCreator::ProcEquipAttBySpecAtt(pEquip);
//	pGM->GetItemMgr().UpdateEquipSpec(*pEquip);
//	return E_Success;
//}

//-----------------------------------------------------------------------------
// 调整生产技能熟练度
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleProficiency(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwSkillID = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	INT nValue = _ttoi((LPCTSTR)vectParam[1]);
	
	Skill* pSkill = pGM->GetSkill(dwSkillID);
	
	if(VALID_POINT(pSkill))
	{
		// 判断是否是角色的生产技能

		if ((pSkill->GetType() == ESST_Role) &&(pSkill->GetTypeEx() == ESSTE_Produce))
			pGM->ChangeSkillExp(pSkill, nValue);
		else
			return E_SystemError;

		return E_Success;
	}
	else
		return E_SystemError;
}

//-----------------------------------------------------------------------------
// 添加buff
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleAddBuff(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwBuffID = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	const tagBuffProto* pProto = AttRes::GetInstance()->GetBuffProto(dwBuffID);
	if( !VALID_POINT(pProto) ) return E_SystemError;

	if(pGM->GMTryAddBuff(pGM, pProto, NULL, NULL, NULL))
		return E_Success;
	else
		return E_SystemError;
}

//-----------------------------------------------------------------------------
// 添加Suit
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleAddSuit(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwSuitID = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	INT nQlty = _ttoi((LPCTSTR)vectParam[1]);

	if( TRUE == pGM->AddSuit(dwSuitID, nQlty))
		return E_Success;
	else
		return E_SystemError;
}

//-----------------------------------------------------------------------------
// 添加指定等级区间（包括低5级的）可用的武器或其他装备
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleAddEquip(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nType = _ttoi((LPCTSTR)vectParam[0]);
	INT nLevel = _ttoi((LPCTSTR)vectParam[1]);
	INT nQlty = _ttoi((LPCTSTR)vectParam[2]);

	if ( TRUE == pGM->AddEquip(nType, nLevel, nQlty) )
		return E_Success;
	else
		return E_SystemError;
}

//-----------------------------------------------------------------------------
// 设定角色的职业 nType 0：专精职业 1：英雄职业
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleSetVocation(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nType = _ttoi((LPCTSTR)vectParam[0]);
	INT nVocation = _ttoi((LPCTSTR)vectParam[1]);

	if(nType == 0)
	{
		if(!(nVocation > EV_Null && nVocation < EV_End)) return E_SystemError;
		pGM->SetClass((EClassType)nVocation);
	}
	else if(nType == 1)
	{
		if(!(nVocation > EHV_Begin && nVocation < EHV_End)) return E_SystemError;
		pGM->SetClassEx((EClassTypeEx)nVocation);
	}
	else
		return E_SystemError;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 设置角色开启宝箱数
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleChangeRoleChestSum(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nSum = _ttoi((LPCTSTR)vectParam[0]);

	pGM->SetTreasureSum(nSum);
	return E_Success;
}

//-----------------------------------------------------------------------------
// 设置服务器开启宝箱数
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleChangeServerChestSum(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nSum = _ttoi((LPCTSTR)vectParam[0]);

	g_worldSession.SetTreasureSum(nSum);
	return E_Success;
}

//-----------------------------------------------------------------------------
// 禁止、解禁玩家一切聊天频道
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleNoSpeak(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_role_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole)) return E_SystemError;

	pRole->SetSpeakOff(TRUE);

	return E_Success;
}

DWORD GMCommandMgr::HandleCancelNoSpeak(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_role_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole)) return E_SystemError;

	pRole->SetSpeakOff(FALSE);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 进入、退出隐身状态
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleLurk(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nLevel = _ttoi((LPCTSTR)vectParam[0]);

	if( !pGM->IsInState(ES_Lurk) )
	{
		pGM->Lurk(nLevel);
	}
	else
	{
		pGM->UnLurk();
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 进入、退出无敌状态
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleInvincible(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if( !pGM->IsInState(ES_Invincible) )
	{
		pGM->SetState(ES_Invincible);
	}
	else
	{
		pGM->UnsetState(ES_Invincible);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得指定玩家的位置（地图，坐标）
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleRolePosition(const std::vector<DWORD>& vectParam, Role* pGM)
{
	return E_Success;
}

//-----------------------------------------------------------------------------
// 杀死指定怪物
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleKillMonster(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwMonsterID = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	Unit* pTarget = pGM->get_map()->find_unit(dwMonsterID);

	if( !VALID_POINT(pTarget)) return E_SystemError;

	if(pTarget->IsCreature())
	{
		Creature* pMonster = (Creature*)pTarget;
		pMonster->OnDead(pGM);
		
		return E_Success;
	}
	else
		return E_SystemError;
}

//-----------------------------------------------------------------------------
// 杀死视野范围内的怪
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleKillAll(const std::vector<DWORD>& vectParam, Role* pGM)
{
	Map* pMap = pGM->get_map();

	if(!VALID_POINT(pMap))
		return -1;
	// 计算玩家所在的可视地砖格子
	INT nVisIndex = pMap->world_position_to_visible_index(pGM->GetCurPos());

	// 得到九宫格
	tagVisTile* pVisTile[EUD_end] = {0};
	pMap->get_visible_tile(nVisIndex, pVisTile);

	for(INT i = EUD_center; i < EUD_end; i++)
	{
		package_map<DWORD, Creature*>::map_iter iter = pVisTile[i]->map_creature.begin();
		Creature* pCreature = NULL;
		while(pVisTile[i]->map_creature.find_next(iter, pCreature))
		{
			if(VALID_POINT(pCreature))
			{
				if(pCreature->IsPet() || pCreature->GetProto()->eType != ECT_Monster)
					continue;

				pCreature->SetTagged(pGM->GetID());
				pCreature->OnDead(pGM);
			}
		}
	}


	return E_Success;
}

//-----------------------------------------------------------------------------
// 摊位获得经验
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::HandleStallExp(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nExp = _ttoi((LPCTSTR)vectParam[0]);

	return pGM->GainStallExp(nExp);
}

DWORD GMCommandMgr::HandleSetPKState(const std::vector<DWORD>& vectParam,	Role* pGM)
{
	ERolePKState ePKState = (ERolePKState)_ttoi((LPCTSTR)vectParam[0]);

	pGM->SetPKState(ePKState);

	return 0;
}

DWORD GMCommandMgr::HandleGMCreateGuild(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring str = (LPCTSTR)vectParam[0];
	g_guild_manager.create_guild_gm(pGM, str.c_str(), str.size()*sizeof(TCHAR));
	return 0;
}

DWORD GMCommandMgr::HandleGMGuildSign(const std::vector<DWORD>& vectParam, Role* pGM)
{
	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	DWORD dwNPCID = INVALID_VALUE;
	pGuild->refer_sign(pGM, dwNPCID, 1111);

	return 0;
}

DWORD GMCommandMgr::HandleDeclareWar(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring str = (LPCTSTR)vectParam[0];

	DWORD dwEnemyGuildID = g_world.LowerCrc32(str.c_str());

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->declare_war_gm(pGM, dwEnemyGuildID);

	return 0;
}

DWORD GMCommandMgr::HandelDeclareWarRes(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring EnemyGuildID = (LPCTSTR)vectParam[0];
	wstring Accept		 = (LPCTSTR)vectParam[1];

	DWORD dwEnemyGuildID = g_world.LowerCrc32(EnemyGuildID.c_str());
	BOOL  bAccept		 =  _ttoi((LPCTSTR)vectParam[1]);

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->declare_war_res(pGM, dwEnemyGuildID, bAccept);

	return 0;
}

DWORD GMCommandMgr::HandleChangeSymbol(const std::vector<DWORD>& vectParam, Role* pGM)
{
	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	BYTE b[20];
	for(INT i = 0; i < 20; i++)
	{
		b[i] = (BYTE)i;
	}

	//pGuild->modify_symbol(pGM->GetID(), b, 20);

	return 0;
}

DWORD GMCommandMgr::HandleGuildGradeUp(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring Type = (LPCTSTR)vectParam[0];

	EFacilitiesType eType = (EFacilitiesType)_ttoi((LPCTSTR)vectParam[0]);

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->get_guild_facilities().HandInItems(pGM, eType);

	return 0;
}

DWORD GMCommandMgr::HandleMissGuild(const std::vector<DWORD>& vectParam, Role* pGM)
{
	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->dismiss_guild();

	return 0;
}

DWORD GMCommandMgr::handleincguildpro(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nPro = _ttoi((LPCTSTR)vectParam[0]);

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->increase_prosperity(pGM->GetID(), nPro, elcid_gm_getfund);

	return 0;
}


DWORD GMCommandMgr::HandleRoleRevive(const std::vector<DWORD>& vectParam,	Role* pGM)
{
	NET_SIS_role_revive send;
	send.dw_role_id		= pGM->GetID();
	send.dw_error_code	= pGM->Revive(ERRT_ReturnCity, 0);

	if( VALID_POINT(pGM->get_map()) )
	{
		pGM->get_map()->send_big_visible_tile_message(pGM, &send, send.dw_size);
	}

	if(E_Success == send.dw_error_code)
	{
		// 复活到指定地点
		pGM->GotoNewMap(pGM->m_Revive.dwMapID, 
			pGM->m_Revive.fX, pGM->m_Revive.fY, pGM->m_Revive.fZ);
	}


	return 0;
}

DWORD GMCommandMgr::HandleCompleteQuest(const std::vector<DWORD>& vectParam, Role* pGM )
{
	UINT16 u16 = 0;
	NET_SIS_complete_quest send;
	send.u16QuestID = (UINT16)_ttoi((LPCTSTR)vectParam[0]);
	send.dw_error_code = pGM->complete_quest((UINT16)_ttoi((LPCTSTR)vectParam[0]),
										 (INT)_ttoi((LPCTSTR)vectParam[1]),
										 INVALID_VALUE, u16 );
	pGM->SendMessage(&send, send.dw_size);
	return 0;
}
DWORD GMCommandMgr::HandlePutQuest(const std::vector<DWORD>& vectParam, Role* pGM)
{
	NET_SIC_PutGDQuest send;
	send.bGuildFix = FALSE;
	send.byHour = 1;
	send.byYuanBao = 0;
	send.n64Item0 =  send.n64Item1 = 0;
	send.u16QuestID = (UINT16)(UINT16)_ttoi((LPCTSTR)vectParam[0]);
	//g_GuerdonQuestMgr.AddEvent(pGM->GetID(), EVT_PutQuest, send.dw_size, &send);
	return 0;
}
DWORD GMCommandMgr::HandleGetQuest(const std::vector<DWORD>& vectParam, Role* pGM)
{
	NET_SIC_GetGDQuest send;
	send.n64Serial = (INT64)_ttoi64((LPCTSTR)vectParam[0]);
	//g_GuerdonQuestMgr.AddEvent(pGM->GetID(), EVT_GetQuest, send.dw_size, &send);
	return 0;
}
DWORD GMCommandMgr::HandlebeginRide(const std::vector<DWORD>& vectParam, Role* pGM )
{
	//pGM->begin_ride();
	return 0;
}

DWORD GMCommandMgr::HandleCancelRide(const std::vector<DWORD>& vectParam, Role* pGM )
{
	pGM->cancel_ride();
	return 0;
}

DWORD GMCommandMgr::HandleUpgradeRide(const std::vector<DWORD>& vectParam, Role* pGM )
{
// 	pGM->upgrade_ride((DWORD)_ttoi((LPCTSTR)vectParam[0]), 
// 					 (INT64)_ttoi64((LPCTSTR)vectParam[1]), 
// 					 (INT64)_ttoi64((LPCTSTR)vectParam[2]), 
// 					 (BYTE)_ttoi((LPCTSTR)vectParam[3]), 
// 					 (INT64)_ttoi64((LPCTSTR)vectParam[4]));
	return 0;
}

DWORD GMCommandMgr::HandleRemoveRideInlay(const std::vector<DWORD>& vectParam, Role* pGM )
{
	pGM->remove_ride_inlay((BYTE)_ttoi((LPCTSTR)vectParam[0]),
						 (INT64)_ttoi64((LPCTSTR)vectParam[1]), 
						 (INT64)_ttoi64((LPCTSTR)vectParam[2]), 
						 (BYTE)_ttoi((LPCTSTR)vectParam[3]) );
	return 0;
}
DWORD GMCommandMgr::HandleMakeMaster(const std::vector<DWORD>& vectParam, Role* pGM )
{
	NET_SIC_make_master send;
	send.dwMaster = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	g_MasterPrenticeMgr.make_master(pGM->GetID(), &send );
	return 0;
}
DWORD GMCommandMgr::HandleMakeMasterEx(const std::vector<DWORD>& vectParam, Role* pGM )
{
	NET_SIC_make_master_extend send;
	send.dwPrentice = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	send.byAck = (BYTE) _ttoi((LPCTSTR)vectParam[1]);
	g_MasterPrenticeMgr.make_master_ex(pGM->GetID(), &send );
	return 0;
}
DWORD GMCommandMgr::HandleMakePrentice(const std::vector<DWORD>& vectParam, Role* pGM )
{
	NET_SIC_make_prentice send;
	send.dwPrentice = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	g_MasterPrenticeMgr.make_prentice( pGM->GetID(), &send);
	return 0;
}
DWORD GMCommandMgr::HandleMakePrenticeEx(const std::vector<DWORD>& vectParam, Role* pGM )
{
	NET_SIC_make_prentice_extend send;
	send.dwMaster = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	send.byAck = (BYTE) _ttoi((LPCTSTR)vectParam[1]);
	g_MasterPrenticeMgr.make_prentice_ex( pGM->GetID(), &send );
	return 0;
}
DWORD GMCommandMgr::HandleMasterPrenticeBreak(const std::vector<DWORD>& vectParam, Role* pGM )
{
	NET_SIC_master_prentice_break send;
	send.dw_role_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	g_MasterPrenticeMgr.master_prentice_break( pGM->GetID(), &send );
	return 0;
}
DWORD GMCommandMgr::HandleGetMasters(const std::vector<DWORD>& vectParam, Role* pGM )
{
	DWORD dwReserved = 0;
	g_MasterPrenticeMgr.get_master_prentices( pGM->GetID( ), &dwReserved );
	return 0;
}
DWORD GMCommandMgr::HandleGetMasterPlacard(const std::vector<DWORD>& vectParam, Role* pGM )
{
	NET_SIC_get_master_placard send;
	g_MasterPrenticeMgr.get_master_placard( pGM->GetID( ), &send);
	return 0;
}

DWORD GMCommandMgr::HandleAddWeaponSkill(const std::vector<DWORD>& vectParam, Role* pGM)
{
	//DWORD dwSkillTypeID	= (INT32)_ttoi((LPCTSTR)vectParam[0]);

	//tagEquip* pEquip = pGM->GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	//if (VALID_POINT(pEquip))
	//{
	//	//pEquip->AddSkill(dwSkillTypeID);
	//	pGM->GetItemMgr().UpdateEquipSpec(*pEquip);
	//	pGM->ProcWeaponSkill(pEquip, TRUE);
	//}
	return 0;
}

DWORD GMCommandMgr::HandStartHang(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->StartHang();
	return 0;
}

DWORD GMCommandMgr::HandSetHang(const std::vector<DWORD>& vectParam, Role* pGM)
{
	BOOL bExp = (BOOL)_ttoi((LPCTSTR)vectParam[0]);
	BOOL bBrother = (BOOL)_ttoi((LPCTSTR)vectParam[1]);
	pGM->SetUseLeaveExp(bExp);
	pGM->SetUseLeaveBrother(bBrother);
	return 0;
}

DWORD GMCommandMgr::HandlePickUpExp(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 nType = (INT16)_ttoi((LPCTSTR)vectParam[0]);
	// 提取离线经验经验
	if(nType == 1)
	{
		if(pGM->GetLeaveExp() <= 0)
			return 0;

		pGM->ExpChange(pGM->GetLeaveExp());
		pGM->CleanLeaveExp();
		NET_DB2C_clean_role_leave_exp send;
		send.dw_role_id = pGM->GetID();
		g_dbSession.Send(&send, send.dw_size);
	}

	// 提取义气值
	if(nType == 2)
	{
		if(pGM->GetLeaveBrother() <= 0)
			return 0;

		pGM->ChangeBrotherhood(pGM->GetLeaveBrother());
		pGM->ReAccAtt();
		pGM->CleanLeaveBrother();
		NET_DB2C_clean_role_leave_brother send;
		send.dw_role_id = pGM->GetID();
		g_dbSession.Send(&send, send.dw_size);
	}
	return 0;
}

DWORD GMCommandMgr::HandleCancelHang(const std::vector<DWORD>& vectParam, Role* pGM)
{
	if(!pGM->IsInRoleState(ERS_Hang))
		return E_Hang_NoHang_State;

	pGM->UnsetRoleState(ERS_Hang);

	pGM->RemoveBuff(30003, TRUE);

	return 0;
}

DWORD GMCommandMgr::HandleUseKillBadge(const std::vector<DWORD>& vectParam, Role* pGM)
{
	NET_SIC_use_skill_badge_item send;
	send.dwTarget =  _ttoi64((LPCTSTR)vectParam[0]);
	//pGM->GetSession( )->HandleRoleUseKillBadage( &send );
	return 0;
}

DWORD GMCommandMgr::HandStartGift(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->m_stNewRoleGift.n_step_++;

	return 0;
}

DWORD GMCommandMgr::HandleGetGift(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->GetNewRoleGift();
	return 0;
}

// 装备强化
DWORD GMCommandMgr::HandleConsolidateEquip(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
	INT level = (DWORD)_ttoi((LPCTSTR)vectParam[1]);

	pGM->GMConsolidateEquip(n16ItemIndex, level);
	return 0;
}

// 修炼
DWORD GMCommandMgr::HandlexiulianEquip(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
	INT level = (DWORD)_ttoi((LPCTSTR)vectParam[1]);

	pGM->GMPractice(n16ItemIndex, level);
	return 0;
}

// 打孔
DWORD GMCommandMgr::HandleDaKongEquip(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
	INT nHoldNum = (DWORD)_ttoi((LPCTSTR)vectParam[1]);

	pGM->GMChiselEquip(n16ItemIndex, nHoldNum);
	return 0;
}

// 开光
DWORD GMCommandMgr::HandlekaiguangEquip(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);

	tagEquip *pEquip = (tagEquip*)(pGM->GetItemMgr().GetBagItem(n16ItemIndex));
	if(!VALID_POINT(pEquip))
		return 0;
	
	pGM->EquipKaiguang(0, pEquip->n64_serial);
	return 0;
}

// 洗礼
DWORD GMCommandMgr::Handlexili(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
	DWORD dwType = (DWORD)_ttoi((LPCTSTR)vectParam[1]);

	tagEquip *pEquip = (tagEquip*)(pGM->GetItemMgr().GetBagItem(n16ItemIndex));
	if(!VALID_POINT(pEquip))
		return 0;

	pGM->EquipXili(0, pEquip->n64_serial, dwType);

	return 0;
}

// 洗礼替换
DWORD GMCommandMgr::HandlexiliTihuan(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);

	tagEquip *pEquip = (tagEquip*)(pGM->GetItemMgr().GetBagItem(n16ItemIndex));
	if(!VALID_POINT(pEquip))
		return 0;

	pGM->EquipXiliChange(pEquip->n64_serial);

	return 0;
}

// 开始拍卖
DWORD GMCommandMgr::handle_begin_paimai(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT64	n64_item_id = (INT64)_ttoi64((LPCTSTR)vectParam[0]);
	DWORD	dw_bidup_price = (DWORD)_ttoi((LPCTSTR)vectParam[1]);			
	DWORD	dw_chaw_price = (DWORD)_ttoi((LPCTSTR)vectParam[2]);			
	BYTE	by_time_type = (DWORD)_ttoi((LPCTSTR)vectParam[3]);

	NET_SIC_begin_paimai send;
	send.n64_item_id = n64_item_id;
	send.dw_bidup_price = dw_bidup_price;
	send.dw_chaw_price = dw_chaw_price;
	send.by_time_type = by_time_type;

	g_paimai.create_paimai(&send, pGM);

	return 0;
}

DWORD GMCommandMgr::handle_cancel_paimai(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD	dw_paimai_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	NET_SIC_cancel_paimai send;
	send.dw_paimai_id = dw_paimai_id;
	g_paimai.cancel_paimai(&send, pGM);

	return 0;
}

DWORD GMCommandMgr::handle_begin_jing(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD	dw_paimai_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	NET_SIC_jingpai send;
	send.dw_paimai_id = dw_paimai_id;
	g_paimai.jingpai(&send, pGM);

	return 0;
}

DWORD GMCommandMgr::handle_paimai_chaw(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD	dw_paimai_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	NET_SIC_chaw_buy send;
	send.dw_paimai_id = dw_paimai_id;
	g_paimai.chaw_buy(&send, pGM);

	return 0;
}

DWORD GMCommandMgr::handle_get_all_paimai(const std::vector<DWORD>& vectParam, Role* pGM)
{
	E_Query_Type e_query_type = (E_Query_Type)_ttoi((LPCTSTR)vectParam[0]);

	NET_SIC_paimai_query send;
	send.st_paimai_query.e_query_type = e_query_type;
	send.st_paimai_query.dw_type_id = INVALID_VALUE;

	g_paimai.query_paimai(pGM, &send);
	return 0;
}

DWORD GMCommandMgr::handle_begin_bank(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD			dw_bidup		= (DWORD)_ttoi((LPCTSTR)vectParam[0]);			// 竞拍价格
	BYTE			by_type			= (BYTE)_ttoi((LPCTSTR)vectParam[1]);			// 拍卖类型 0 游戏币 1 元宝
	DWORD			dw_sell			= (DWORD)_ttoi((LPCTSTR)vectParam[2]);			// 卖出数量
	DWORD			dw_chaw			= (DWORD)_ttoi((LPCTSTR)vectParam[3]);			// 一口价
	BYTE			by_time_type	= (DWORD)_ttoi((LPCTSTR)vectParam[4]);			// 时间类型

	NET_SIC_begin_bank_paimai send;
	send.by_time_type = by_time_type;
	send.by_type = by_type;
	send.dw_bidup = dw_bidup;
	send.dw_chaw = dw_chaw;
	send.dw_sell = dw_sell;

	g_bankmgr.begin_bank_paimai(pGM, &send);
	return E_Success;

}

DWORD GMCommandMgr::handle_bank_jing(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD	dw_bank_id		= (DWORD)_ttoi((LPCTSTR)vectParam[0]);	

	NET_SIC_begin_bank_jing send;
	send.dw_bank_id = dw_bank_id;
	g_bankmgr.begin_bank_jing(pGM, &send);

	return E_Success;
}

DWORD GMCommandMgr::handle_reset_instance(const std::vector<DWORD>& vectParam, Role* pGM)
{
	//pGM->reset_instance();
	return E_Success;
}

DWORD GMCommandMgr::handle_join_recruit(const std::vector<DWORD>& vectParam, Role* pGM)
{
	g_guild_manager.join_guild_recruit(pGM, INVALID_VALUE);
	return E_Success;
}

DWORD GMCommandMgr::handle_leave_recruit(const std::vector<DWORD>& vectParam, Role* pGM)
{
	g_guild_manager.leave_guild_recruit(pGM);
	return E_Success;
}

DWORD GMCommandMgr::handle_query_recruit(const std::vector<DWORD>& vectParam, Role* pGM)
{
	g_guild_manager.query_guild_recruit(pGM);
	return E_Success;
}

DWORD GMCommandMgr::handle_stack_item(const std::vector<DWORD>& vectParam, Role* pGm)
{
	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	pGm->GetItemMgr().Stack(EICT_Bag);

	dw_new_time = timeGetTime();

	print_message(_T("stack item %d\r\n"), dw_new_time - dw_time);

	return E_Success;
}

// 让角色变得nb
DWORD GMCommandMgr::handle_whosyourdaddy(const std::vector<DWORD>& vectParam, Role* pGm)
{
	bool bAttack = _ttoi((LPCTSTR)vectParam[0]);

	pGm->LevelChange(ROLE_LEVEL_LIMIT, FALSE, TRUE);

	pGm->SetBaseAttValue(ERA_MaxHP, AttRes::GetInstance()->GetAttMax(ERA_MaxHP));
	pGm->SetBaseAttValue(ERA_Speed_XZ, AttRes::GetInstance()->GetAttMax(ERA_Speed_XZ));

	if (bAttack)
	{
		pGm->SetBaseAttValue(ERA_ExAttackMin, AttRes::GetInstance()->GetAttMax(ERA_ExAttackMin));
		pGm->SetBaseAttValue(ERA_ExAttackMax, AttRes::GetInstance()->GetAttMax(ERA_ExAttackMax));
		pGm->SetBaseAttValue(ERA_InAttackMin, AttRes::GetInstance()->GetAttMax(ERA_InAttackMin));
		pGm->SetBaseAttValue(ERA_InAttackMax, AttRes::GetInstance()->GetAttMax(ERA_InAttackMax));
		pGm->SetBaseAttValue(ERA_ArmorEx, AttRes::GetInstance()->GetAttMax(ERA_ArmorEx));
		pGm->SetBaseAttValue(ERA_ArmorIn, AttRes::GetInstance()->GetAttMax(ERA_ArmorIn));
	}
	pGm->RecalAtt();
	pGm->SetAttValue(ERA_HP, pGm->GetAttValue(ERA_MaxHP));
	

	return E_Success;
}
// 让角色变得nb
DWORD GMCommandMgr::handle_chengjiupoint(const std::vector<DWORD>& vectParam, Role* pGm)
{
	INT nPoint = (INT)_ttoi((LPCTSTR)vectParam[0]);	

	pGm->ModAchievementPoint(nPoint);

	return E_Success;
}

// 创建副本
DWORD GMCommandMgr::handle_create_instance(const std::vector<DWORD>& vectParam, Role* pGm)
{
	wstring str = (LPCTSTR)vectParam[0];
	INT n_level = (INT)_ttoi((LPCTSTR)vectParam[1]);
	INT n_num	= (INT)_ttoi((LPCTSTR)vectParam[2]);

	MapMgr* pMapMgr = g_mapCreator.get_map_manager(get_tool()->crc32(str.c_str()));

	if(VALID_POINT(pMapMgr))
	{
		for(INT i = 0; i < n_num; i++)
		{
			pMapMgr->CreateInstance(pGm, n_level);
		}
	}

	return E_Success;
}


DWORD GMCommandMgr::handle_change_newness(const std::vector<DWORD>& vectParam, Role* pGm)
{
	INT16 n16ItemIndex = (INT16)_ttoi((LPCTSTR)vectParam[0]);
	INT nNewness = (INT)_ttoi((LPCTSTR)vectParam[1]);
	
	// 找到被强化装备
	tagEquip *pEquip = (tagEquip*)pGm->GetItemMgr().GetBagItem(n16ItemIndex);
	if(VALID_POINT(pEquip) && MIsEquipment(pEquip->dw_data_id))
	{
		pEquip->ChangeNewness(nNewness);

		NET_SIS_newess_change send;
		send.n64EquipSerial = pEquip->n64_serial;
		send.nAttackTimes = pEquip->nUseTimes;
		pGm->SendMessage(&send, send.dw_size);

	}
	return E_Success;
}

DWORD GMCommandMgr::handle_enter_pvp(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->EnterPVP();

	return E_Success;
}

DWORD GMCommandMgr::handle_enter_1v1(const std::vector<DWORD>& vectParam, Role* pGM)
{
	g_pvp_mgr.apply_1v1(pGM);

	return E_Success;
}

DWORD GMCommandMgr::handle_apply_1v1(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring str = (LPCTSTR)vectParam[0];

	DWORD dw_role_id = g_roleMgr.get_role_id(g_world.LowerCrc32(str.c_str()));

	pGM->reservation_apply(dw_role_id);

	return E_Success;
}

DWORD GMCommandMgr::handle_result_1v1(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring str = (LPCTSTR)vectParam[0];

	DWORD dw_role_id = g_roleMgr.get_role_id(g_world.LowerCrc32(str.c_str()));

	//pGM->reservation_result(dw_role_id);

	return E_Success;
}

DWORD GMCommandMgr::handle_leave_prictice(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD	dw_error = E_Success;

	BYTE byTimeType = (BYTE)_ttoi((LPCTSTR)vectParam[0]);
	BYTE byMulType = (BYTE)_ttoi((LPCTSTR)vectParam[1]);

	dw_error = pGM->SetLeaveParcitice(byTimeType, byMulType);

	NET_SIS_leave_parcitice send;
	send.dw_error = dw_error;
	pGM->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 清除僵尸连线（一般情况下不要用）
DWORD GMCommandMgr::handle_clearsession(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_account_id = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	g_worldSession.RemoveSession(dw_account_id);
	g_worldSession.RemoveGlobalSession(dw_account_id);
	g_loginSession.SendPlayerLogout(dw_account_id);

	return E_Success;
}

// 关闭开启验证码功能
DWORD GMCommandMgr::handle_openver(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_open = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	AttRes::GetInstance()->GetVariableLen().n_verification = dw_open;

	return E_Success;
}

// 领取序列号奖励
DWORD GMCommandMgr::handle_serial_reward(const std::vector<DWORD>& vectParam, Role* pGM)
{
	wstring str = (LPCTSTR)vectParam[0];

	char s_str[X_SHORT_NAME];
	ZeroMemory(s_str, sizeof(*s_str));

	strncpy(s_str, get_tool()->unicode_to_ansi((LPCTSTR)(str.c_str())), X_SHORT_NAME);

	NET_DB2C_load_serial_reward send_reward;
	send_reward.dw_role_id = pGM->GetID();
	strncpy(send_reward.sz_serial, s_str, X_SHORT_NAME);
	g_dbSession.Send(&send_reward, send_reward.dw_size);


	/*string sz_str(s_str);

	DWORD dw_error = E_Success;

	if(!pGM->is_serial_init())
	return INVALID_VALUE;

	s_serial_reward* p = pGM->get_map_serial_reward().find(sz_str);
	if(!VALID_POINT(p))
	return INVALID_VALUE;

	if(dw_error == E_Success)
	{
	if(pGM->GetScript())
	{
	pGM->GetScript()->OnReceiveSerialReward(pGM, p->n_type);
	}

	NET_DB2C_delete_serial_reward send;
	send.dw_account_id = pGM->GetSession()->GetSessionID();
	strncpy(send.sz_serial, p->sz_serial, X_SHORT_NAME);
	g_dbSession.Send(&send, send.dw_size);

	NET_DB2C_log_serial_reward log_send;
	log_send.s_log_serial_reward.dw_account_id = pGM->GetSession()->GetSessionID();
	log_send.s_log_serial_reward.n_type = p->n_type;
	strncpy(log_send.s_log_serial_reward.sz_serial, p->sz_serial, X_SHORT_NAME);
	g_dbSession.Send(&log_send, log_send.dw_size);

	pGM->get_map_serial_reward().erase(sz_str);
	SAFE_DELETE(p);
	}

	NET_SIS_receive_serial_reward send;
	send.dw_error = dw_error;
	pGM->SendMessage(&send, send.dw_size);*/

	return E_Success;
}

// 获得战功
DWORD GMCommandMgr::handle_GetExploits(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nExploit = (INT)_ttoi((LPCTSTR)vectParam[0]);
	pGM->GetCurMgr().IncExploits(nExploit, elcid_gm_get_money);
	return E_Success;
}

// 添加帮贡
DWORD GMCommandMgr::handle_add_banggong(const std::vector<DWORD>& vectParam, Role* pGM )
{
	DWORD dw_banggong = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	guild* pGuild = g_guild_manager.get_guild(pGM->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->change_member_contribution(pGM->GetID(), dw_banggong, TRUE);
	
	return E_Success;
}

// 关闭开启移动过快踢人
DWORD GMCommandMgr::handle_KickFaster(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dw_open = (DWORD)_ttoi((LPCTSTR)vectParam[0]);

	AttRes::GetInstance()->GetVariableLen().n_kick_fast_move = dw_open;

	if(dw_open)
	{
		g_roleMgr.reset_delay_time();
	}

	return E_Success;
}

DWORD GMCommandMgr::handle_strengthcheck(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT n_time = (INT)_ttoi((LPCTSTR)vectParam[0]);
	INT n_num = (INT)_ttoi((LPCTSTR)vectParam[1]);

	g_world.SetStrengthCheck(n_time, n_num);

	return E_Success;
}

// 移动距离限制
DWORD GMCommandMgr::handle_move_limit(const std::vector<DWORD>& vectParam, Role* pGM)
{
	FLOAT fLaxCheat = (FLOAT)_ttoi((LPCTSTR)vectParam[0]);
	FLOAT fLaxJumpCheat = (FLOAT)_ttoi((LPCTSTR)vectParam[1]);
	INT	  nLimitNum = (INT)_ttoi((LPCTSTR)vectParam[2]);

	g_world.SetLaxCheatDistanceSQ(fLaxCheat);
	g_world.SetLaxJumpCheatDistanceSQ(fLaxJumpCheat);
	g_world.SetCELimitNum(nLimitNum);

	return E_Success;
}


// 设置系统时间
DWORD GMCommandMgr::hanle_set_time(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwDay = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	DWORD dwHour = (DWORD)_ttoi((LPCTSTR)vectParam[1]);
	DWORD dwMin = (DWORD)_ttoi((LPCTSTR)vectParam[2]);
	
	SYSTEMTIME CurTime;
	GetLocalTime(&CurTime);
	
	CurTime.wDay = dwDay;
	CurTime.wHour = dwHour;
	CurTime.wMinute = dwMin;

	SetLocalTime(&CurTime);

	return E_Success;
}

DWORD GMCommandMgr::HandleClearPKValue(const std::vector<DWORD>& vectParam, Role* pGM )
{
	pGM->SetPKValueMod(0 - pGM->GetPKValue());
	return E_Success;
}

DWORD GMCommandMgr::hanle_cleardaydata(const std::vector<DWORD>& vectParam, Role* pGM )
{
	pGM->ResetDayClearData();
	return E_Success;
}

DWORD GMCommandMgr::hanle_set_pet_ernie_level(const std::vector<DWORD>& vectParam, Role* pGM )
{
	INT level = (INT)_ttoi((LPCTSTR)vectParam[0]);
	AttRes::GetInstance()->GetVariableLen().pet_ernie_level = level;
	return E_Success;
}


// 签到
DWORD GMCommandMgr::handle_sign(const std::vector<DWORD>& vectParam, Role* pGM)
{
	BOOL b_buqian = (BOOL)_ttoi((LPCTSTR)vectParam[0]);

	pGM->role_sign(b_buqian, 0);

	return E_Success;
}

// 签到领奖
DWORD GMCommandMgr::handle_sign_reward(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT n_index = (INT)_ttoi((LPCTSTR)vectParam[0]);

	pGM->role_sign_reward(n_index);

	return E_Success;
}

// 获取签到数据
DWORD GMCommandMgr::handle_get_sign(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->get_sign_data();
	return E_Success;
}

DWORD GMCommandMgr::handle_huenlian(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->Huenlian(0);
	return E_Success;
}
DWORD GMCommandMgr::handle_godLevel(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nLevel = (INT)_ttoi((LPCTSTR)vectParam[0]);

	//pGM->setGodLevel(nLevel);

	NET_SIS_god_level_up send;
	send.nLevel = pGM->getGodLevel();
	pGM->SendMessage(&send, send.dw_size);

	return E_Success;
}
DWORD GMCommandMgr::handle_reward_Item(const std::vector<DWORD>& vectParam, Role* pGM)
{
	DWORD dwItemID = (DWORD)_ttoi((LPCTSTR)vectParam[0]);
	DWORD dwNubmer = (DWORD)_ttoi((LPCTSTR)vectParam[1]);
	INT nType = (INT)_ttoi((LPCTSTR)vectParam[2]);

	pGM->addRewardItem(dwItemID, dwNubmer, (E_REWARDFROM)nType);


	return E_Success;
}

DWORD GMCommandMgr::handle_open_SBK(const std::vector<DWORD>& vectParam, Role* pGM)
{
	BOOL bBegin = g_guild_manager.is_begin_SBK();
	g_guild_manager.set_begin_SBK(!bBegin);
	return E_Success;
}

DWORD GMCommandMgr::handle_exp_raid(const std::vector<DWORD>& vectParam, Role* pGM)
{
	INT nExp = _ttoi((LPCTSTR)vectParam[0]);

	pGM->GetRaidMgr().ExpChange(nExp, TRUE);
	return E_Success;
}

DWORD GMCommandMgr::handle_open_server(const std::vector<DWORD>& vectParam, Role* pGM)
{
	
	DWORD dwData[7][3];
	memset(dwData, 0, sizeof(dwData));
	RankMgr::GetInstance()->getOpenServerData(pGM, &dwData[0][0]);
	return E_Success;
}

DWORD GMCommandMgr::handle_enter_battle(const std::vector<DWORD>& vectParam, Role* pGM)
{
	pGM->EnterBattle();
	return E_Success;
}