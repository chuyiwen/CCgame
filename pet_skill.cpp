/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物技能

#include "StdAfx.h"
#include "pet_skill.h"

#include "pet_skill_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/pet_skill_protocol.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "../../common/WorldDefine/drop_protocol.h"

#include "item_creator.h"
#include "role.h"
#include "pet_server_define.h"
#include "pet_soul.h"
#include "att_res.h"
#include "title_mgr.h"

PetSkill::PetSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2) 
	:m_pProto(NULL), m_pSoul(pSoul), m_nCoolDownTick(nPara1), m_nWorkCountTick(nPara2)
{
	// get proto
	m_pProto = AttRes::GetInstance()->GetPetSkillProto(dwSkillTypeID);
	ASSERT(VALID_POINT(m_pProto));
}

PetSkill* PetSkill::CreatePetSkill( DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2 )
{
	const tagPetSkillProto* pProto = AttRes::GetInstance()->GetPetSkillProto(dwSkillTypeID);
	if (!VALID_POINT(pProto))
	{
		ASSERT(0);
		return NULL;
	}

	PetSkill* pSkill = NULL;

	switch(pProto->eType)
	{
	case EPT_Gather:
		break;
	case EPT_PickUp:
		pSkill = new PetPickUpSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	case EPT_MedicineFeed:
		pSkill = new PetMedicineFeedSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	case EPT_Forage:
		break;
	case EPT_Experience:
		break;
	case EPT_Deliver:
		break;
	case EPT_Sale:
		break;
	case EPT_StoreHouse:
		break;
	case EPT_Strengthen:
		pSkill = new PetStrengthSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	case EPT_Buff:
		//pSkill = new PetBuffSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		pSkill = new PetStrengthSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	case EPT_WuXing:
		pSkill = new PetWuXingSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	case EPT_MountAdd:
		pSkill = new PetMountAddSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	case EPT_Specialty:
		//pSkill = new PetSpecialtySkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		pSkill = new PetStrengthSkill(dwSkillTypeID, pSoul, nPara1, nPara2);
		break;
	default:
		break;
	}

	return pSkill;
}

Role* PetSkill::GetMaster() const
{
	return GetSoul()->GetMaster();
}

VOID PetSkill::FillClientInfo( tagPetSkillMsgInfo* pInfo )
{
	pInfo->dw_data_id			= m_pProto->dw_data_id;
	pInfo->nCurrCoolDown	= m_nCoolDownTick;
	pInfo->nMaxCoolDown		= m_nWorkCountTick;
}

BOOL PetSkill::SaveToDB( s_db_pet_skill* pDBSkill )
{
	pDBSkill->dw_data_id	= m_pProto->dw_data_id;
	pDBSkill->n_para1_	= m_nCoolDownTick;
	pDBSkill->n_para2_	= m_nWorkCountTick;

	return TRUE;
}

VOID PetSkill::Update()
{
	// 更新冷却倒计时
	CoolDowning();

	WorkCounting();
}

PetSkill* PetSkill::CreateDBPetSkill( s_db_pet_skill* pDBPetSkill, PetSoul* pSoul )
{
	PetSkill* pSkill = CreatePetSkill(pDBPetSkill->dw_data_id, pSoul, pDBPetSkill->n_para1_, pDBPetSkill->n_para2_);
	return pSkill;
}

DWORD PetSkill::GetSkillTypeID() const
{
	return m_pProto->dw_data_id;
}

BYTE PetSkill::GetCastCondition() const
{
	return GetProto()->byCast_condition;
}

BOOL PetSkill::CanSetWorking( BOOL bWorking )
{
	return GetSoul()->CanSetWroking();
}

BOOL PetSkill::SetWorking( BOOL bWorking )
{
	return GetSoul()->SetWorking(bWorking);
}

VOID PetSkill::SetWorkCounting( INT nTickAdd /*= 0*/ )
{
	m_nWorkCountTick = GetProto()->nWorkTimeTick + nTickAdd;
	if (m_nWorkCountTick < 0)
	{
		m_nWorkCountTick = INVALID_VALUE;
	}
}
VOID PetSkill::SetCoolDowning( INT nTickAdd /*= 0*/ )
{
	m_nCoolDownTick = GetProto()->nCooldownTick + nTickAdd;
	if (m_nCoolDownTick < 0)
	{
		m_nCoolDownTick = INVALID_VALUE;
	}
}


BOOL PetStrengthSkill::Active(Role* pTarget, INT nAddLevel)
{
	if(m_bInUsing)
		return FALSE;

	// 技能限制
	//FLOAT fRate = GetSoul()->GetPetAtt().GetAttVal(epa_strength_effect) / 10000.0f;

	// 计算加成
	//Role* pMaster = GetMaster();
	if(!VALID_POINT(pTarget))
		return FALSE;
	const tagPetSkillProto* pSkillProto = GetProto();

	//for (INT nIndex = 0; nIndex < MAX_PET_SKILL_MOD_ATT && VALID_VALUE(pSkillProto->AttIndexs[nIndex]); ++nIndex)
	//{
	//	pMaster->ModAttModValue(pSkillProto->AttIndexs[nIndex], INT(pSkillProto->AttMods[nIndex] * fRate + 0.5));
	//}
	
	const tagSkillProto* pRoleSkillProto = AttRes::GetInstance()->GetSkillProto(pSkillProto->dw_data_id);
	if (VALID_POINT(pRoleSkillProto))
	{
		DWORD id = Skill::GetIDFromTypeID(pSkillProto->dw_data_id);
		// 玩家已有该技能
		Skill* pSkill = pTarget->GetSkill(id);
		if(VALID_POINT(pSkill)) 
			return FALSE;

		// 主人学会技能
		pSkill = new Skill(id, 0, Skill::GetLevelFromTypeID(pSkillProto->dw_data_id)+nAddLevel, 0, 0, TRUE);
		pTarget->AddSkill(pSkill);

		pTarget->RecalAtt();
	}



	m_bInUsing = TRUE;

	return TRUE;
}

BOOL PetStrengthSkill::DeActive(Role* pTarget, BOOL bSendMsg)
{
	if(!m_bInUsing)
		return FALSE;

	// 技能限制
	//FLOAT fRate = GetSoul()->GetPetAtt().GetAttVal(epa_strength_effect) / 10000.0f;

	// 取消加成
	//Role* pMaster = GetMaster();
	if(!VALID_POINT(pTarget))
		return FALSE;

	const tagPetSkillProto* pSkillProto = GetProto();

	//for (INT nIndex = 0; nIndex < MAX_PET_SKILL_MOD_ATT && VALID_VALUE(pSkillProto->AttIndexs[nIndex]); ++nIndex)
	//{
	//	pMaster->ModAttModValue(pSkillProto->AttIndexs[nIndex], -INT(pSkillProto->AttMods[nIndex] * fRate + 0.5));
	//}
	
	pTarget->RemoveSkill(Skill::GetIDFromTypeID(pSkillProto->dw_data_id));

	pTarget->RecalAtt(bSendMsg);
	

	m_bInUsing = FALSE;

	return TRUE;

}

BOOL PetSpecialtySkill::Active(Role* pTarget, INT nAddLevel)
{
	if(m_bInUsing)
		return FALSE;

	// 技能限制
	//FLOAT fRate = GetSoul()->GetPetAtt().GetAttVal(epa_strength_effect) / 10000.0f;

	// 计算加成
	//Role* pMaster = GetMaster();
	if(!VALID_POINT(pTarget))
		return FALSE;

	const tagPetSkillProto* pSkillProto = GetProto();

	const tagSkillProto* pRoleSkillProto = AttRes::GetInstance()->GetSkillProto(pSkillProto->dw_data_id);
	if (VALID_POINT(pRoleSkillProto))
	{
		DWORD id = Skill::GetIDFromTypeID(pSkillProto->dw_data_id);
		// 玩家已有该技能
		Skill* pSkill = pTarget->GetSkill(id);
		if(VALID_POINT(pSkill)) 
			return FALSE;

		// 主人学会技能
		pSkill = new Skill(id, 0, Skill::GetLevelFromTypeID(pSkillProto->dw_data_id)+nAddLevel, 0, 0, TRUE);
		pTarget->AddSkill(pSkill);

		pTarget->RecalAtt();
	}



	m_bInUsing = TRUE;

	return TRUE;
}

BOOL PetSpecialtySkill::DeActive(Role* pTarget, BOOL bSendMsg )
{
	if(!m_bInUsing)
		return FALSE;

	// 技能限制
	//FLOAT fRate = GetSoul()->GetPetAtt().GetAttVal(epa_strength_effect) / 10000.0f;

	// 取消加成
	//Role* pMaster = GetMaster();
	if(!VALID_POINT(pTarget))
		return FALSE;

	const tagPetSkillProto* pSkillProto = GetProto();

	pTarget->RemoveSkill(Skill::GetIDFromTypeID(pSkillProto->dw_data_id));

	pTarget->RecalAtt(bSendMsg);


	m_bInUsing = FALSE;

	return TRUE;

}


DWORD PetMedicineFeedSkill::HandleCmdImpl( tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto , INT &nCoolDownAdd, INT &nWorkingAdd )
{
	M_trans_pointer(pMCmd, pCmd->m_pInputBuf, tagPetSupplyData);
	pCmd->m_dwOutSize = 0;

	Role* pOwnerRole = GetMaster();

	// 首先找到这个物品
	tagItem* pItem = pOwnerRole->GetItemMgr().GetBagItem(pMCmd->n64ItemID); 
	if( !VALID_POINT(pItem) )
	{
		return E_Pets_PetSkill_Use_ItemNotExist;
	}

	if (pOwnerRole->GetItemMgr().IsItemCDTime(pItem->dw_data_id))
	{
		return INVALID_VALUE;
	}

	Map* pMap = pOwnerRole->get_map();
	if( !VALID_POINT(pMap) ) return E_Pets_UnkownErr;

	// 计算buff
	pOwnerRole->OnActiveItemBuffTrigger(pOwnerRole, pItem, ETEE_Use);

	// 计算物品的脚本使用效果
	if(VALID_POINT(pItem->pScript) && VALID_POINT(pOwnerRole->get_map()))
		pItem->pScript->UseItem(pOwnerRole->get_map(), pItem->dw_data_id, pOwnerRole->GetID(), pOwnerRole->GetID(), pItem->n64_serial);

	// 称号消息
	pOwnerRole->GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, pItem->dw_data_id, 1);

	// 加入物品公共冷却时间
	pOwnerRole->GetItemMgr().Add2CDTimeMap(pItem->dw_data_id);

	INT nSave = INT(GetSoul()->GetPetAtt().GetAttVal(epa_medicine_saving) / 100);

	if(!get_tool()->probability(nSave))
	{
		// 处理物品消失
		pOwnerRole->GetItemMgr().ItemUsedFromBag(pItem->GetKey(), 1, (DWORD)elcid_item_use);
	}

	return E_Pets_Success;
}

DWORD PetPickUpSkill::HandleCmdImpl( tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd )
{
	M_trans_pointer(p_receive, pCmd->m_pInputBuf, tagPetPickUpData);
	M_trans_pointer(pSerial, pCmd->m_pOutBuf, INT64);
	pCmd->m_dwOutSize	= 0;


	M_trans_else_ret(pMap, GetMaster()->get_map(), Map, E_Pets_Success);
	M_trans_else_ret(pGroundItem, pMap->get_ground_item(p_receive->n64GroundSerial), tag_ground_item, E_Pets_Success);

	if(	(pGroundItem->dw_team_id == INVALID_VALUE && pGroundItem->dw_owner_id != INVALID_VALUE && GetMaster()->GetID() != pGroundItem->dw_owner_id) ||
		(pGroundItem->dw_team_id != INVALID_VALUE && GetMaster()->GetTeamID() != pGroundItem->dw_team_id) ||
		(pGroundItem->dw_team_id != INVALID_VALUE && pGroundItem->dw_owner_id != INVALID_VALUE && GetMaster()->GetID() != pGroundItem->dw_owner_id) ||
		(TYPE_ID_MONEY != pGroundItem->dw_type_id && GetMaster()->GetItemMgr().GetBagFreeSize() <= 0)
		)
	{
		pCmd->m_dwOutSize	= sizeof(INT64);
		*pSerial			= p_receive->n64GroundSerial;
		return E_Pets_UnkownErr;
	}

	GetMaster()->PickUpItem(p_receive->n64GroundSerial);

// 	if(pGroundItem->dw_team_id != INVALID_VALUE &&
// 	   (pGroundItem->dw_owner_id == INVALID_VALUE && 
// 	   (pGroundItem->e_pick_mode == EPUM_Sice || pGroundItem->e_pick_mode == EPUM_Leader)))
// 	{
// 		pCmd->m_dwOutSize	= sizeof(INT64);
// 		*pSerial			= p_receive->n64GroundSerial;
// 		return E_Pets_UnkownErr;
// 	}
	
// 
// 	//发送物品消失消息
// 	NET_SIS_role_ground_item_disappear disappear;
// 	disappear.n64_serial[0] = p_receive->n64GroundSerial;
// 	pMap->send_big_visible_tile_message(GetMaster(), &disappear, disappear.dw_size);
// 
// 	//从Map中删除掉落物品
// 	//将掉落物品从地面删除
// 	pMap->remove_ground_item(pGroundItem);
// 
// 	//注意把掉落物品拾取到背包里,这里tag_ground_item中的pItem指针会被销毁,所以放进背包应该是最后
// 	//注意把掉落物品拾取到背包里,这里tag_ground_item中的pItem指针会被销毁,所以放进背包应该是最后
// 	if ((pGroundItem->dw_type_id == TYPE_ID_MONEY && FALSE == GetMaster()->GetCurMgr().IncBagSilver(pGroundItem->n_num, elcid_pickup_money)) ||
// 		(pGroundItem->dw_type_id != TYPE_ID_MONEY && E_Success != GetMaster()->GetItemMgr().Add2Bag(pGroundItem->p_item, elcid_pickup_item, TRUE))
// 		)
// 	{
// 		pCmd->m_dwOutSize	= sizeof(INT64);
// 		*pSerial			= p_receive->n64GroundSerial;
// 		return E_Pets_UnkownErr;
// 	}
// 	
// 	//将指针恢复为NULL
// 	pGroundItem->p_item=NULL;
// 	SAFE_DELETE(pGroundItem);

	// 节省时间
	nCoolDownAdd = GetSoul()->GetPetAtt().GetAttVal(epa_pick_up_resume);

	return E_Pets_Success;
}

DWORD PetWuXingSkill::HandleCmdImpl( tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd )
{
	M_trans_pointer(pWuXingItem, pCmd->m_pOutBuf, tagPetWuXingItem);
	pCmd->m_dwOutSize = 0;

	FLOAT fRate = GetSoul()->GetPetAtt().GetAttVal(epa_wuxing_consume) / 10000.0f;

	INT nTalentCount	= GetSoul()->GetPetAtt().GetAttVal(epa_talent_count);
	INT nMaxTalentCount	= GetSoul()->GetPetAtt().GetAttVal(epa_talent_count_max);

	if (nTalentCount >= nMaxTalentCount)
	{
		return E_Pets_PetSkill_Use_MaxTalentCount;
	}

	// 判断角色背包是否已满，若无剩余空间，
	if (0 == GetSoul()->GetMaster()->GetItemMgr().GetBagFreeSize())
	{
		//则提示：“背包已满，灵兽无法施展五行凝结”；
		return E_Pets_UnkownErr;
	}
	
	//读灵兽五行表，根据灵兽品质决定随机道具；
	const tagPetWuXingProto* pWuXingProto = AttRes::GetInstance()->GetPetWuXingProto(GetSoul()->GetPetAtt().GetAttVal(epa_quality));
	if (!VALID_POINT(pWuXingProto))
	{
		ASSERT(0);
		return E_Pets_UnkownErr;
	}

	tagItem* pActItem = NULL;

	for (INT i=0; i<MAX_WUXING_ITEM_NUM; ++i)
	{
		if(!get_tool()->probability(pWuXingProto->nProb[i] / 100))
			continue;

		// 生成物品
		pActItem = ItemCreator::Create(EICM_PetSkill, GetSoul()->GetID(), pWuXingProto->dwItemTypeID[i], pWuXingProto->n_num[i]);
		if (VALID_POINT(pActItem))
			break;
	}

	//道具直接奖励到角色背包
	if(!VALID_POINT(pActItem))
		return E_Pets_UnkownErr;
	DWORD dwRtv = GetMaster()->GetItemMgr().Add2Bag(pActItem, elcid_act_treasure, TRUE);
	if(E_Success == dwRtv)
	{
		// 提示：“您的灵兽施展五行之力，为您凝聚XXX（获得物品名称，绿色显示）！”
		pWuXingItem->dwItemTypeID	= pActItem->pProtoType->dw_data_id;
		pWuXingItem->n_num			= pActItem->n16Num;
		pCmd->m_dwOutSize			= sizeof(tagPetWuXingItem);

		// 消耗五行力
		GetSoul()->GetPetAtt().ModAttVal(epa_wuxing_energy,	-INT(pSkillProto->nWuxing_cost * fRate + 0.5));
	}
	else
	{
		::Destroy(pActItem);
	}
	
	return dwRtv;
}

DWORD ActiveSkill::HandleCmd( tagPetSkillCmdParam* pCmd )
{
	const tagPetSkillProto* pPetSkillProto = GetProto();
	if (!VALID_POINT(pPetSkillProto))
	{
		ASSERT(0);
		return E_Pets_PetSkill_Use_SkillProtoInvalid;
	}
	//if (pPetSkillProto->nSpirit_cost > GetSoul()->GetPetAtt().GetAttVal(EPA_Spirit))
	//{
	//	return E_Pets_PetSkill_Use_SpiritNotEnough;
	//}
	//if (pPetSkillProto->nWuxing_cost > GetSoul()->GetPetAtt().GetAttVal(EPA_WuXingEnergy))
	//{
	//	return E_Pets_PetSkill_Use_WuXingEnergyNotEnough;
	//}
	if (IsCoolDowning())
	{
		return E_Pets_PetSkill_Use_CoolingDown;
	}
	if (IsWorkCounting())
	{
		return E_Pets_PetSkill_Use_WorkCounting;
	}

	INT nCoolDownAdd = 0;
	INT nWorkAdd = 0;
	DWORD dwRtv = HandleCmdImpl(pCmd, pPetSkillProto, nCoolDownAdd, nWorkAdd);

	if (E_Pets_Success == dwRtv)
	{
		// 冷却时间
		SetWorkCounting(nWorkAdd);
		SetCoolDowning(nCoolDownAdd);

		//FLOAT fRate = GetSoul()->GetPetAtt().GetAttVal(EPA_SpiritRate) / 10000.0f;

		// 消耗
		//GetSoul()->GetPetAtt().ModAttVal(EPA_Spirit,		-INT(pPetSkillProto->nSpirit_cost * fRate + 0.5));

		// 获取
		//GetSoul()->GetPetAtt().ModAttVal(EPA_WuXingEnergy,	pPetSkillProto->nWuxing_add);
	}
	
	return dwRtv;
}

BOOL PetMountAddSkill::Open()
{
	if (m_bInUsing)
	{
		return FALSE;
	}
	m_bInUsing = TRUE;

	const tagPetSkillProto* pSkillProto = GetProto();

	if (VALID_VALUE(pSkillProto->nPetAttIndex))
	{
		GetSoul()->GetPetAtt().ModAttVal(pSkillProto->nPetAttIndex, pSkillProto->nPetAttMod);
	}

	return TRUE;
}

BOOL PetMountAddSkill::Close()
{
	if (!m_bInUsing)
	{
		return FALSE;
	}
	m_bInUsing = FALSE;

	const tagPetSkillProto* pSkillProto = GetProto();

	if (VALID_VALUE(pSkillProto->nPetAttIndex))
	{
		GetSoul()->GetPetAtt().ModAttVal(pSkillProto->nPetAttIndex, -pSkillProto->nPetAttMod);
	}

	return TRUE;
}

DWORD PetBuffSkill::HandleCmdImpl( tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd )
{
	// 设置命令的返回值
	BOOL bRtv = TRUE;//= *((BOOL*)(pCmd->m_pOutBuf));
	//pCmd->m_dwOutSize = sizeof(bRtv);

	// 添加buff
	DWORD dwBuffID = Buff::GetIDFromTypeID(pSkillProto->nBuffid);
	if (GetMaster()->IsHaveBuff(dwBuffID) && GetMaster()->RemoveBuff(dwBuffID, TRUE) == E_Success 
		||!GetMaster()->IsHaveBuff(dwBuffID))
	{
		bRtv = GetMaster()->TryAddBuff(GetMaster(), AttRes::GetInstance()->GetBuffProto(pSkillProto->nBuffid), NULL, NULL, NULL);
	}

	return bRtv ? E_Success : INVALID_VALUE;
}