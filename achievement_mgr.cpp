/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//成就系统

#include "stdafx.h"
#include "achievement_mgr.h"
#include "role.h"
#include "../common/ServerDefine/achievement_server_define.h"
#include "../../common/WorldDefine/achievement_protocol.h"



AchievementMgr::AchievementMgr()
{
	memset(m_nAchievementSignetNum, 0, sizeof(m_nAchievementSignetNum));
}
// 1初始化选项
void AchievementMgr::InitOpts(Role* pRole)
{
	ASSERT( VALID_POINT(pRole) );
	m_pRole = pRole;
}

void AchievementMgr::InitComplate(const BYTE* &pData, const INT32 n_num)
{
	m_completedAchievements.clear();

	s_achievement_complate_save* pAchLoad = (s_achievement_complate_save*)pData;

	for( INT nLoadIndex = 0; nLoadIndex < n_num; ++nLoadIndex )
	{
		DWORD id	= pAchLoad[nLoadIndex].dw_id;
		DWORD date	= pAchLoad[nLoadIndex].dw_date;
		
		const AchievementEntry* achievement = AttRes::GetInstance()->GetAchievementProto(id);
		if (!VALID_POINT(achievement))
			continue;
		
		int nOldLevel = GetBigSignetLevel(); 

		CompletedAchievementData& ca = m_completedAchievements[id];
		ca.m_ID = id;
		ca.m_date = date;
		ca.m_changed = false;
		
		m_nAchievementSignetNum[achievement->m_signet]++;
		
		int nCurLevel = GetBigSignetLevel(); 
		
		if (nCurLevel > nOldLevel)
		{
			OnSignetLevelUp(eas_tianqi, nCurLevel);
		}

		OnSignetCheng(achievement->m_signet);
	}

	pData = reinterpret_cast<const BYTE *>( pAchLoad + n_num );
}

void AchievementMgr::InitCriteria(const BYTE* &pData, const INT32 n_num)
{
	m_criteriaProgress.clear();

	s_achievement_process_save* pAchLoad = (s_achievement_process_save*)pData;

	for( INT nLoadIndex = 0; nLoadIndex < n_num; ++nLoadIndex )
	{
		DWORD id	= pAchLoad[nLoadIndex].dw_id;
		DWORD count = pAchLoad[nLoadIndex].dw_count;
		DWORD date	= pAchLoad[nLoadIndex].dw_date;

		if (!VALID_POINT(AttRes::GetInstance()->GetAchievementCriteriaProto(id)))
			continue;

		CriteriaProgress& ca = m_criteriaProgress[id];
		ca.m_ID =  id;
		ca.m_date = date;
		ca.m_counter = count;
		ca.m_changed = false;

	}

	pData = reinterpret_cast<const BYTE *>( pAchLoad + n_num );

}

// 销毁数据
void AchievementMgr::Destroy()
{
	
}


UINT32 AchievementMgr::GetCriteriaProgressMaxCounter(AchievementCriteriaEntry const* achievementCriteria)
{
	return achievementCriteria->m_dwPara2;
}


bool AchievementMgr::IsCompletedCriteria(AchievementCriteriaEntry const* achievementCriteria, AchievementEntry const* achievement) const
{

	CriteriaProgressMap::const_iterator itr = m_criteriaProgress.find(achievementCriteria->m_ID);
	if(itr == m_criteriaProgress.end())
		return false;

	CriteriaProgress const* progress = &itr->second;

	UINT32 maxcounter = GetCriteriaProgressMaxCounter(achievementCriteria);

	if (!maxcounter)
		return false;
	
	if (achievementCriteria->m_Events == ete_rank_pos)
	{
		return progress->m_counter <= maxcounter;
	}
	return progress->m_counter >= maxcounter;
}

// 发信号
VOID AchievementMgr::UpdateAchievementCriteria(e_achievement_event titleEvent, DWORD dwArg1, DWORD dwArg2)
{

	//AchievementCriteriaEntryList listAchievementProto = AttRes::GetInstance()->GetAchievementCriteriaByType(titleEvent);
	//AchievementCriteriaEntryList::iterator titleItr = listAchievementProto.begin();
	//for( ;titleItr != listAchievementProto.end(); titleItr++ )
	//{
	//	const AchievementCriteriaEntry*  achievementCriteria = *titleItr;
	//	const AchievementEntry *achievement = AttRes::GetInstance()->GetAchievementProto(achievementCriteria->m_referredAchievement);

	//	if (!VALID_POINT(achievement))
	//		continue;

	//	//对应成就已经完成了
	//	if (m_completedAchievements.find(achievement->m_ID) != m_completedAchievements.end())
	//		continue;

	//	//  条件完成了
	//	if (IsCompletedCriteria(achievementCriteria,achievement))
	//		continue;

	//	//CriteriaProgress& progress = m_criteriaProgress[achievementCriteria->m_ID];
	//	
	//	ProgressType progressType = PROGRESS_HIGHEST; 
	//	switch(achievementCriteria->m_CondType)
	//	{
	//	case ect_count:
	//		progressType = PROGRESS_ACCUMULATE;
	//		break;
	//	case ect_value:
	//		progressType = PROGRESS_HIGHEST;
	//		break;
	//	}

	//	UINT32 change = 0;
	//	
	//	switch(achievementCriteria->m_Events)
	//	{
	//	case ete_kill_monster:
	//	case ete_quest_complete:
	//	case ete_use_item:
	//	case ete_use_skill:
	//	case eta_roborn:
	//	case eta_bank_buy_success:
	//	case eta_bank_sell_success:
	//	case eta_produce_item:
	//	case eta_be_kill_monster:
	//	case ete_produce_equip_up_quality:
	//	case ete_consolidate_up_pro:
	//	case ete_into_area:
	//	case eta_equip_xili:
	//	case eta_equip_reatt:
	//	case ete_xiangqian_ride_item:
	//	case eta_pet_to_level:
	//	case eta_equip_kaiguang:
	//	case ete_kill_role_class:
	//	case ete_1v1_win_class:
	//	case ete_duel_class:
	//	case ete_add_item:
	//	case ete_title_get:
	//		if(!dwArg2) 
	//			continue;

	//		if (dwArg1 != achievementCriteria->m_dwPara1)
	//			continue;
	//		
	//		change = dwArg2;
	//	
	//		progressType = PROGRESS_ACCUMULATE;
	//		break;
	//	case ete_kill_role:
	//	case ete_role_die:
	//	case ete_role_skilled_by_role:
	//	case ete_decomposite_three_seven:
	//	case eta_decomposite_feil:
	//	case eta_decomposite_equip:
	//	case eta_fish_field:
	//	case eta_fish_sucess:
	//		if(!dwArg1)
	//			continue;
	//		change = dwArg1;
	//		progressType = PROGRESS_ACCUMULATE;
	//		break;
	//	case ete_role_level:
	//		{
	//			change = m_pRole->get_level();
	//			progressType = PROGRESS_HIGHEST;
	//			break;
	//		}
	//	case eta_skill_level_up:
	//	case eta_extendbag:
	//	case eta_equip_kaiguang_get_nai:
	//		if (achievementCriteria->m_dwPara1 != dwArg1)
	//			continue;
	//		change = dwArg2;
	//		progressType = PROGRESS_HIGHEST;
	//		break;
	//	case ete_rank_pos:
	//		if (achievementCriteria->m_dwPara1 != dwArg1)
	//			continue;
	//		change = dwArg2;
	//		progressType = PROGRESS_LOWEST;
	//		break;
	//	case ete_composite_achievement:
	//		if(m_completedAchievements.find(achievementCriteria->m_dwPara1) == m_completedAchievements.end())
	//			continue;

	//		change = 1;
	//		progressType = PROGRESS_ACCUMULATE;
	//		break;
	//	case ete_composite_equip_success:
	//	case ete_strengthen_ride_fail:
	//	case ete_inlay_level:
	//	case ete_1v1_win_level:
	//	case eta_unbeset_level:
	//	case ete_delete_item_level:
	//	case ete_duel_level:
	//	case ete_kill_role_level_sub:
	//		if (dwArg1 >= achievementCriteria->m_dwPara1)
	//		{
	//			change = dwArg2;
	//			progressType = PROGRESS_ACCUMULATE;
	//		}
	//		break;
	//	case ete_kill_role_level:
	//		if (dwArg1 <= achievementCriteria->m_dwPara1)
	//		{
	//			change = dwArg2;
	//			progressType = PROGRESS_ACCUMULATE;
	//		}
	//		break;

	//	case eta_add_friend:
	//	case eta_pet_level:
	//	case ete_strengthen_weapon_success_2:
	//	case ete_strengthen_weapon_success_3:
	//	case ete_strengthen_weapon_success_4:
	//	case ete_strengthen_armor_success_2:
	//	case ete_strengthen_armor_success_3:
	//	case ete_strengthen_armor_success_4:
	//	case ete_strengthen_armor_success:
	//	case ete_strengthen_ride_success:
	//	case ete_xiangqian_ride:
	//	case ete_xiulian:
	//	case ete_graduates_num:
	//	case ete_title_num:
	//	case ete_PKValue:
	//	case ete_pet_have_quality:
	//	case ete_have_prentice:
	//	case eta_shipin_fumo_level:
	//		change = dwArg1;
	//		progressType = PROGRESS_HIGHEST;
	//		break;
	//	case eta_chisel:
	//		change = dwArg1;
	//		progressType = PROGRESS_SET;
	//		break;
	//	default:
	//		change = dwArg1;
	//		progressType = PROGRESS_ACCUMULATE;
	//		break;
	//	}

	//	

	//	SetCriteriaProgress(achievementCriteria, achievement, change, progressType);
	//}

}

void AchievementMgr::SetCriteriaProgress(AchievementCriteriaEntry const* criteria, AchievementEntry const* achievement, UINT32 changeValue, ProgressType ptype)
{

	UINT32 max_value = GetCriteriaProgressMaxCounter(criteria);

	//if (max_value < 0 )
	//	max_value = std::numeric_limits<UINT32>::max();

	if (changeValue > max_value)
		changeValue = max_value;

	CriteriaProgress *progress = NULL;
	UINT32 old_value = 0;
	UINT32 newValue = 0;

	CriteriaProgressMap::iterator iter = m_criteriaProgress.find(criteria->m_ID);

	// 没有进度的条件
	if(iter == m_criteriaProgress.end())
	{
		
		if(changeValue == 0)
			return;


		progress = &m_criteriaProgress[criteria->m_ID];
		progress->m_ID = criteria->m_ID;
		progress->m_date = GetCurrentDWORDTime();
		progress->m_bTimedCriteriaFailed = false;

		// 对于有时间限制的条件
		if (criteria->m_dwTimeLimit)
		{
			m_criteriaFailTimes[criteria->m_ID] = (progress->m_date + criteria->m_dwTimeLimit);
			progress->m_counter = 0;
			SendCriteriaUpdate(criteria->m_ID, progress);
		}


		newValue = changeValue;
	}
	else // 有的进度的条件
	{
		progress = &iter->second;

		old_value = progress->m_counter;

		// 变更值
		switch(ptype)
		{
		case PROGRESS_SET:
			newValue = changeValue;
			break;
		case PROGRESS_ACCUMULATE:
			{
				
				newValue = max_value - progress->m_counter > changeValue ? progress->m_counter + changeValue : max_value;
				break;
			}
		case PROGRESS_HIGHEST:
			newValue = progress->m_counter < changeValue ? changeValue : progress->m_counter;
			break;
		case PROGRESS_LOWEST:
			{
				if (progress->m_counter == 0)
				{
					progress->m_counter = changeValue;
				}
				newValue = progress->m_counter > changeValue ? changeValue : progress->m_counter;
			}
			
			break;
		}

		if(progress->m_counter == newValue)
			return;
	}

	progress->m_counter = newValue;

	// 阀值条件不记录数据库,也不发给客户端
	if (ptype != PROGRESS_HIGHEST)
	{
		progress->m_changed = true;

		// 发送进度更新
		SendCriteriaUpdate(criteria->m_ID,progress);
	}

	
	
	if (old_value < progress->m_counter)
	{
		if(IsCompletedCriteria(criteria, achievement))
			CompletedCriteriaFor(achievement);

	
		if(AchievementEntryList const* achRefList = AttRes::GetInstance()->GetAchievementByReferencedId(achievement->m_ID))
		{
			for(AchievementEntryList::const_iterator itr = achRefList->begin(); itr != achRefList->end(); ++itr)
				if(IsCompletedAchievement(*itr))
					CompletedAchievement(*itr);
		}
	}

}

bool AchievementMgr::IsCompletedAchievement(AchievementEntry const* entry)
{

	UINT32 achievementForTestId = entry->m_refAchievement ? entry->m_refAchievement : entry->m_ID;
	UINT32 achievementForTestCount = entry->m_count;

	AchievementCriteriaEntryList const* cList = AttRes::GetInstance()->GetAchievementCriteriaByAchievement(achievementForTestId);
	if(!VALID_POINT(cList))
		return false;

	UINT32 count = 0;

	bool completed_all = true;
	for(AchievementCriteriaEntryList::const_iterator itr = cList->begin(); itr != cList->end(); ++itr)
	{
		AchievementCriteriaEntry const* criteria = *itr;

		bool completed = IsCompletedCriteria(criteria,entry);

		if(completed)
			++count;
		else
			completed_all = false;

		// 所有条件都完成了
		if(achievementForTestCount > 0 && achievementForTestCount <= count)
			return true;
	}

	// 所有条件都完成了
	if(completed_all && achievementForTestCount==0)
		return true;

	return false;
}



// 发送初始数据给客户端
VOID AchievementMgr::SendAllAchievementData()
{
	INT nNum = m_completedAchievements.size();

	if (nNum > 0)
	{
		tagCompletedAchievementData* pRoleInfo = NULL;

		DWORD dw_size = sizeof(NET_SIS_init_achievement);

		dw_size += (nNum-1) * sizeof(tagCompletedAchievementData);

		CREATE_MSG(pSend, dw_size, NET_SIS_init_achievement);
		
		pSend->n_num = nNum;
		

		CompletedAchievementMap::iterator iter = m_completedAchievements.begin();
		for (int i = 0; iter != m_completedAchievements.end(); iter++, i++)
		{
			pSend->completeAchData[i].ID = iter->first;
			pSend->completeAchData[i].date = iter->second.m_date;
		}
		
		m_pRole->SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);

	}
	
	nNum = m_criteriaProgress.size();
	if (nNum > 0)
	{
		tagCriteriaProgress* pRoleInfo = NULL;

		DWORD dw_size = sizeof(NET_SIS_init_achievement_criteria);

		dw_size += (nNum-1) * sizeof(tagCriteriaProgress);

		CREATE_MSG(pSend, dw_size, NET_SIS_init_achievement_criteria);
		pSend->n_num = nNum;

		CriteriaProgressMap::iterator iter = m_criteriaProgress.begin();

		for(INT i = 0; iter != m_criteriaProgress.end(); iter++,i++)
		{
			//if( INVALID_VALUE == m_criteriaProgress[i].m_ID ) continue;

			pSend->criteriaProgress[i].ID = iter->first;
			pSend->criteriaProgress[i].date = iter->second.m_date;
			pSend->criteriaProgress[i].counter = iter->second.m_counter;
		}
		m_pRole->SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);

	}
	

}
void AchievementMgr::SendCriteriaUpdate(UINT32 id, CriteriaProgress const* progress)
{
	NET_SIS_updata_achievement_progress send;
	send.dw_id = id;
	send.dw_count = progress->m_counter;
	send.b_fail = progress->m_bTimedCriteriaFailed;
	m_pRole->SendMessage(&send, send.dw_size);
}

void AchievementMgr::SendAchievementEarned(AchievementEntry const* achievement)
{

	NET_SIS_complate_achievement send;
	send.dw_id = achievement->m_ID;
	send.dw_date = GetCurrentDWORDTime();

	m_pRole->SendMessage(&send, send.dw_size);
}


void AchievementMgr::CompletedCriteriaFor(AchievementEntry const* achievement)
{
	
	// 已经完成了
	if (m_completedAchievements.find(achievement->m_ID)!=m_completedAchievements.end())
		return;

	if (IsCompletedAchievement(achievement))
		CompletedAchievement(achievement);
}


void AchievementMgr::CompletedAchievement(AchievementEntry const* achievement)
{

	int nOldLevel = GetBigSignetLevel(); 

	// 加入到完成列表
	CompletedAchievementData& ca =  m_completedAchievements[achievement->m_ID];
	ca.m_date = GetCurrentDWORDTime();
	ca.m_changed = true;
	ca.m_ID = achievement->m_ID;

	SendAchievementEarned(achievement);
	
	// 增加成就点数
	m_pRole->ModAchievementPoint(achievement->m_point);

	// 对应纹章+1
	m_nAchievementSignetNum[achievement->m_signet]++;

	int nCurLevel = GetBigSignetLevel(); 

	if (nCurLevel > nOldLevel)
	{
		OnSignetLevelUp(eas_tianqi, nCurLevel);
	}

	OnSignetCheng(achievement->m_signet);

	// 获得对应称号
	const tagTitleProto* pTitle = AttRes::GetInstance()->GetTitleProto(achievement->m_nTitleID);
	if (VALID_POINT(pTitle))
	{
		m_pRole->SetTitle(achievement->m_nTitleID);
	}

	
	UpdateAchievementCriteria(ete_composite_achievement);

}


// 存储到数据库
void AchievementMgr::SaveComplateToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	s_achievement_complate_save* pAchData = (s_achievement_complate_save*)pData;

	INT nSaveNum = 0;
	if(!m_completedAchievements.empty())
	{
		for(CompletedAchievementMap::iterator iter = m_completedAchievements.begin(); iter!=m_completedAchievements.end(); ++iter)
		{
			if(!iter->second.m_changed)
				continue;

			
			pAchData[nSaveNum].dw_id = iter->first;
			pAchData[nSaveNum].dw_date = iter->second.m_date;

			iter->second.m_changed = false;
			
			nSaveNum++;
		}
	}
	pOutPointer = static_cast<BYTE *>(pData) + sizeof(s_achievement_complate_save) * nSaveNum;
	n_num += nSaveNum;
}

void AchievementMgr::SaveCriteriaToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	s_achievement_process_save* pAchData = (s_achievement_process_save*)pData;

	INT nSaveNum = 0;
	if(!m_criteriaProgress.empty())
	{
		for(CriteriaProgressMap::iterator iter = m_criteriaProgress.begin(); iter!=m_criteriaProgress.end(); ++iter)
		{
			if(!iter->second.m_changed)
				continue;


			pAchData[nSaveNum].dw_id = iter->first;
			pAchData[nSaveNum].dw_count = iter->second.m_counter;
			pAchData[nSaveNum].dw_date = iter->second.m_date;

			iter->second.m_changed = false;

			nSaveNum++;
		}
	}
	pOutPointer = static_cast<BYTE *>(pData) + sizeof(s_achievement_process_save) * nSaveNum;
	n_num += nSaveNum;

}


void AchievementMgr::DoFailedTimedAchievementCriterias()
{
	if (m_criteriaFailTimes.empty())
		return;

	tagDWORDTime now = GetCurrentDWORDTime();
	for (AchievementCriteriaFailTimeMap::iterator iter = m_criteriaFailTimes.begin(); iter != m_criteriaFailTimes.end();)
	{
		// 没到时间
		if (iter->second > now)
		{
			++iter;
			continue;
		}


		AchievementCriteriaEntry const* criteria = AttRes::GetInstance()->GetAchievementCriteriaProto(iter->first);
		AchievementEntry const* achievement = AttRes::GetInstance()->GetAchievementProto(criteria->m_referredAchievement);
		
		// 时间到了,还没完成则失败
		if (!IsCompletedCriteria(criteria, achievement))
		{

			CriteriaProgressMap::iterator pro_iter = m_criteriaProgress.find(criteria->m_ID);

			CriteriaProgress* progress = &pro_iter->second;

			progress->m_bTimedCriteriaFailed = true;
			SendCriteriaUpdate(criteria->m_ID, progress);

			// 条件失败
			m_criteriaProgress.erase(pro_iter);
		}

		m_criteriaFailTimes.erase(iter++);
	}
}

// 看纹章是否升级了
VOID AchievementMgr::OnSignetCheng(e_achievement_signet e_signet_type)
{
	if (e_signet_type == eas_tianqi)
	{
		return;
	}

	INT nOldLevel = AchievementHelper::GetSignetLevel(e_signet_type, m_nAchievementSignetNum[e_signet_type] - 1);
	INT nCurLevel = AchievementHelper::GetSignetLevel(e_signet_type, m_nAchievementSignetNum[e_signet_type]);

	// 升级了,学技能
	if (nCurLevel > nOldLevel)
	{
		OnSignetLevelUp(e_signet_type, nCurLevel);	
	}
}

// 纹章升级
VOID AchievementMgr::OnSignetLevelUp(e_achievement_signet e_signet_type, int nLevel)
{
	Skill* pSkill = m_pRole->GetSkill(ACHIEVEMENT_SIGNET_SKILL[e_signet_type]);
	if(!VALID_POINT(pSkill)) 
	{
		pSkill = new Skill(ACHIEVEMENT_SIGNET_SKILL[e_signet_type], 0, nLevel, 0, 0, TRUE);
		m_pRole->AddSkill(pSkill);

		m_pRole->RecalAtt();
	}
	else
	{
		pSkill->IncLevel(ESLC_Self);
	}	
}

int AchievementMgr::GetBigSignetLevel()
{
	int nTotleLevel = AchievementHelper::GetAchievementLevel(GetComplateNumber());
	int nMinSingnetLevel = 10;
	for (int i = 0; i < eas_tianqi; i++)
	{
		INT nLevel = AchievementHelper::GetSignetLevel((e_achievement_signet)i, m_nAchievementSignetNum[i]);
		nMinSingnetLevel = min(nLevel, nMinSingnetLevel);
	}
	return min(nMinSingnetLevel, nTotleLevel);
}