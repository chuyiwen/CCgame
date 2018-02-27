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
*	@file		role_fishing
*	@author		mwh
*	@date		2011/06/27	initial
*	@version	0.0.1.0
*	@brief		钓鱼
*/

#include "StdAfx.h"
#include "player_session.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/fishing_protocol.h"
#include "../../common/WorldDefine/creature_define.h"

#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/fishing_server_define.h"

#include "att_res.h"
#include "role.h"
#include "creature.h"


//! 开始钓鱼
INT Role::StartFishing(DWORD dwNpc,DWORD dwSkill, INT64 n64FishRod, INT64 n64Bait, BOOL bAutoFish)
{
	if(IsInRoleState(ERS_Fishing))
		return E_Success;

	INT nRet = FishingCheckBasic(n64FishRod, n64Bait, dwNpc, FISHINGSKILLID);
	if(E_Success != nRet) return nRet;

	InitFishing(dwNpc, n64FishRod, n64Bait, bAutoFish);

	SetRoleState(ERS_Fishing);
	return E_Success;
}

//! 结束钓鱼
INT Role::StopFishing()
{
	if(IsInRoleState(ERS_Fishing))
		UnsetRoleState(ERS_Fishing);

	this->InitFishing(); return E_Success;
}

//! 钓鱼更新
VOID Role::UpdateFishing()
{
	if(!IsInRoleState(ERS_Fishing)) return;

	--mFishDecVigourTick;
	if(--mFishingTick > 0) return;


	SetFishingCD( );

	if (VALID_POINT(GetSession()) && GetSession()->GetFatigueGuarder().GetEarnRate() < 10000)
	{
		return;
	}

	INT nRet = FishingCheckBasic(mFishingRod, mBait, mFishNpc, FISHINGSKILLID);
	// 注意判断顺序<不能改变>
	if(nRet != E_Success || !ProbGetFish( ) || !mAutoFish) 
	{
		// notice
		NET_SIS_Stop_Fishing send;
		send.dwError = nRet;
		SendMessage(&send, send.dw_size);

		StopFishing(); return; 
	}
}

//! 鱼饵产出
BOOL Role::ProbBaitGet()
{
	if(!VALID_POINT(mBait)) return FALSE;

	tagItem* pItem = GetItemMgr().GetBagItem(mBait);
	if(!VALID_POINT(pItem) || !VALID_POINT(pItem->pProtoType))
		return FALSE;

	GetItemMgr().ItemUsedFromBag(mBait, 1, elcid_fishing);

	BOOL bSuccess  = get_tool()->probability(pItem->pProtoType->nSpecFuncVal2);

	if( bSuccess && VALID_POINT(pItem->pProtoType->nSpecFuncVal1))
	{
		tagItem* pNewItem = CreateItem(EICM_Fishing, m_dwID, pItem->pProtoType->nSpecFuncVal1, 1, INVALID_VALUE);
		if(VALID_POINT(pNewItem) && E_Success != GetItemMgr().Add2Bag(pNewItem, elcid_fishing, TRUE)) 
		{
			::Destroy(pNewItem); return FALSE;
		}
	}

	if (bSuccess)
	{
		GetAchievementMgr().UpdateAchievementCriteria(eta_fish_sucess, 1);
	}
	else
	{
		GetAchievementMgr().UpdateAchievementCriteria(eta_fish_field, 1);
	}
	return bSuccess;
}

//! 钓鱼产出
BOOL Role::ProbGetFish( )
{
	// 计算概率，得物品
	Skill* pSkill = GetSkill(FISHINGSKILLID);
	Creature* pNpc = get_map()->find_creature(mFishNpc);
	DWORD dw_npc_data_id = (VALID_POINT(pNpc) && VALID_POINT(pNpc->GetProto())) ?
												pNpc->GetProto()->dw_data_id : 0;

	const tagNpcFishingProto* pProto = AttRes::GetInstance()->GetNpcFishingProto(dw_npc_data_id);
	if(!VALID_POINT(pProto) || !VALID_POINT(pSkill) ||
		!VALID_POINT(pSkill->GetProto()) ||
		pSkill->GetProto()->nType3 != ESSTE2_Fishing) return FALSE;


	tagItem* pFishingRod = GetItemMgr().GetBagItem(mFishingRod);
	if(!VALID_POINT(pFishingRod) || !VALID_POINT(pFishingRod->pProtoType))
		return FALSE;

	INT nCommonFishProb =10000 - pFishingRod->pProtoType->nSpecFuncVal2;
	INT nProficiencyDelta =  pFishingRod->pProtoType->nSpecFuncVal1;

	// 技能升级
	ChangeSkillExp(pSkill, 1);

	// 注意此此处一下不能再使用pFishingRod
	GetItemMgr( ).ItemUsedFromBag(pFishingRod->n64_serial, 1, elcid_fishing);

	if(VALID_POINT(GetScript()))
	{
		GetScript()->OnFishing(this);
	}


	// 钓空
	if(get_tool()->probability(pProto->dwNullGetProb / 100))
	{
		GetAchievementMgr().UpdateAchievementCriteria(eta_fish_field, 1);
		NET_SIS_Fishing_NullGet send;
		SendMessage(&send, send.dw_size);
		return TRUE;
	}

	// 鱼饵掉落
	if(VALID_POINT(mBait))
	{
		if(!ProbBaitGet()) 
		{
			NET_SIS_Fishing_NullGet send;
			SendMessage(&send, send.dw_size);
		}
		return TRUE;
	}

	// 随机钓鱼类型
	INT nRadPct = get_tool()->tool_rand() % 10000;
	INT nRate = 0, index = -1, nProficiency = pSkill->GetProficiency( );

	// 提升品质
	nProficiency += nProficiencyDelta; 
	const std::vector<tagFishItem>&  fish_outs = nRadPct <= nCommonFishProb ? (pProto->outs) : (pProto->outs_sp);

	//! 最后在随机可得鱼概率
	nRadPct = get_tool()->tool_rand() % 10000;

	// 开始找鱼
	INT nLastIndex = -1; 
	for(INT n = 0; n < (INT)fish_outs.size(); ++n)
	{
		//! mwh 2011-08-22 论坛提示钓鱼bug
		if(fish_outs[n].dwSkillProficiency <= (DWORD)nProficiency ||  
			pProto->nSkillLevel < pSkill->get_level( ))
		{
			nLastIndex = n; 
			nRate += fish_outs[n].dwItemProb;
			if(nRadPct < nRate) {index = n; break;}
		}
	}

	// 超过最大概率，得最后一个
	if(nRadPct > nRate && index < 0 && nLastIndex != -1) index = nLastIndex;

	if(index >= 0 && fish_outs[index].dwItemID)
	{
		tagItem* pNewItem = CreateItem(EICM_Fishing, m_dwID, fish_outs[index].dwItemID, 1, INVALID_VALUE);
		if(VALID_POINT(pNewItem) && E_Success != GetItemMgr().Add2Bag(pNewItem, elcid_fishing, TRUE)) 
		{
			::Destroy(pNewItem); return FALSE;
		}
	}
	
	GetAchievementMgr().UpdateAchievementCriteria(eta_fish_sucess, 1);

	return TRUE;
}

//! 钓鱼检查道具以及背包
INT Role::FishingCheckBasic(INT64 n64FishRod, INT64 n64Bait, DWORD dwNpc, DWORD dwSkill)
{
	if(mFishDecVigourTick <= 0)
	{
		if( this->GetAttValue(ERA_Fortune) <=0)
			return E_CantFishingOutOfVigour;

		this->ModAttValue(ERA_Fortune, -1);
		this->SetDecVigourCD( );
	}

	tagItem* pFishingRod = GetItemMgr().GetBagItem(n64FishRod);
	if( !VALID_POINT(pFishingRod) ||
		!VALID_POINT(pFishingRod->pProtoType) ||
		pFishingRod->pProtoType->eSpecFunc != EIST_FishingRod)
	{
		return E_CantFishingNoRod;
	}

	if( VALID_POINT(n64Bait) )
	{
		tagItem* pBait = GetItemMgr().GetBagItem(n64Bait); 
		if( !VALID_POINT(pBait) ||
			!VALID_POINT(pBait->pProtoType) ||
			pBait->pProtoType->eSpecFunc != EIST_Bait)
		{
			return E_CantFishingNoBait;
		}
	}

	if(!GetItemMgr().GetBagFreeSize())
		return E_CantFishingOutOfSpace;

	if(IsInRoleStateAny(ERS_Combat | ERS_Stall | ERS_StallSet | ERS_Prictice | ERS_Mount | ERS_Carry)) 
		return E_CantFishingStateLimit;

	Creature* pNpc = get_map()->find_creature(dwNpc);
	if(!VALID_POINT(pNpc) ||
		!VALID_POINT(pNpc->GetProto()) ||
		!pNpc->IsFunctionNPC(EFNPCT_FISH))
	{
		return E_CantFishingNpcError;
	}

	const tagNpcFishingProto* pProto = AttRes::GetInstance()->GetNpcFishingProto(pNpc->GetProto()->dw_data_id);
	if(!VALID_POINT(pProto)) return E_CantFishingNpcError;
	if(this->get_level() < pProto->nMinLevel) return E_CantFishingOutOfLevel;

	Skill* pSkill = GetSkill(FISHINGSKILLID);
	if( !VALID_POINT(pSkill) ||
		!VALID_POINT(pSkill->GetProto()) ||
		pSkill->GetProto()->nType3 != ESSTE2_Fishing)
	{
		return E_CantFishingNoSkill;
	}

	//	INT nProficiency = pSkill->GetProficiency() +  pFishingRod->pProtoType->nSpecFuncVal1;
	if( pSkill->get_level( ) < pProto->nSkillLevel) return E_CantFishingSkillLevel;

	return E_Success;
}

//! 初始化钓鱼数据
VOID Role::InitFishing()
{
	SetFishingCD();
	mFishDecVigourTick = 0;
	mFishNpc = 0;
	mAutoFish = FALSE;
	mBait = INVALID_VALUE;
	mFishingRod = INVALID_VALUE;
}

//!  初始化钓鱼数据
VOID Role::InitFishing(DWORD dwNpc, INT64 n64FishRod, INT64 n64Bait,  BOOL bAutoFish)
{
	SetFishingCD();
	mFishDecVigourTick = 0;
	mBait = n64Bait;
	mFishNpc = dwNpc;
	mFishingRod = n64FishRod;
	mAutoFish = bAutoFish;
}
//! 钓鱼CD
VOID Role::SetFishingCD()
{
	mFishingTick = FISHINGCDTICK;
}

VOID Role::SetDecVigourCD()
{
	mFishDecVigourTick = AttRes::GetInstance()->GetVariableLen( ).n_fishing_vigour_cost_second * TICK_PER_SECOND;
}
