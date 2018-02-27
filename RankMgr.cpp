/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "StdAfx.h"
#include "RankMgr.h"
#include "role.h"

#include "../common/ServerDefine/rank_server_define.h"
#include "../common/ServerDefine/rank_server_define.h"
#include "../common/ServerDefine/title_server_define.h"
#include "../../common/WorldDefine/rank_protocol.h"
#include "../../common/WorldDefine/rank_define.h"
#include "script_mgr.h"
#include "title_mgr.h"
#include "guild_manager.h"
#include "guild.h"


RankMgr*	RankMgr::m_pInstance = NULL;

RankMgr::RankMgr(void)
{
	Interlocked_Exchange((LONG*)&m_bLock, TRUE);

	memset(m_zhanliName, 0, sizeof(m_zhanliName));
	memset(m_nLevel, 0, sizeof(m_nLevel));

	memset(m_dwZhanLiRole, 0, sizeof(m_dwZhanLiRole));
	memset(m_dwLevelRole, 0, sizeof(m_dwLevelRole));

	m_dwMeiliRole[0] = INVALID_VALUE;
	m_dwMeiliRole[1] = INVALID_VALUE;

}

RankMgr::~RankMgr(void)
{
	CleanRank();
}


RankMgr* RankMgr::GetInstance()
{
	if(!VALID_POINT(m_pInstance))
	{
		m_pInstance = new RankMgr;
	}

	return m_pInstance;
}

VOID RankMgr::Destory()
{
	SAFE_DELETE(m_pInstance);
}


// 清空排行榜
VOID RankMgr::CleanRank()
{
	LIST_LEVELRANK::list_iter leveliter = m_listLevelRank.begin();
	s_level_rank* pLevelRank = NULL;
	while(m_listLevelRank.find_next(leveliter, pLevelRank))
	{
		if(VALID_POINT(pLevelRank))
			SAFE_DELETE(pLevelRank);
	}
	m_listLevelRank.clear();

	LIST_EQUIPRANK::list_iter equipiter = m_listEquipRank.begin();
	tagEquip* pEquip = NULL;
	while(m_listEquipRank.find_next(equipiter, pEquip))
	{
		if(VALID_POINT(pEquip))
			SAFE_DELETE(pEquip);
	}
	m_listEquipRank.clear();

	LIST_GUILDRANK::list_iter guilditer = m_listGuildRank.begin();
	tagGuildRank* pGuildRank = NULL;
	while(m_listGuildRank.find_next(guilditer, pGuildRank))
	{
		if(VALID_POINT(pGuildRank))
			SAFE_DELETE(pGuildRank);
	}
	m_listGuildRank.clear();

	LIST_KILLRANK::list_iter killiter = m_listKillRank.begin();
	tagKillRank* pKillRank = NULL;
	while(m_listKillRank.find_next(killiter, pKillRank))
	{
		if(VALID_POINT(pKillRank))
		{
			SAFE_DELETE(pKillRank);
		}
	}
	m_listKillRank.clear();

	LIST_1V1RANK::list_iter rank1v1iter = m_list1v1Rank.begin();
	tag1v1ScoreRank* p1v1Rank = NULL;
	while(m_list1v1Rank.find_next(rank1v1iter, p1v1Rank))
	{
		if(VALID_POINT(p1v1Rank))
		{
			SAFE_DELETE(p1v1Rank);
		}
	}
	m_list1v1Rank.clear();

	LIST_SHIHUNRANK::list_iter rankshihuniter = m_listShihunRank.begin();
	tagShihunRank* pShihunRank = NULL;
	while(m_listShihunRank.find_next(rankshihuniter, pShihunRank))
	{
		if(VALID_POINT(pShihunRank))
		{
			SAFE_DELETE(pShihunRank);
		}
	}
	m_listShihunRank.clear();

	LIST_JUSTICERANK::list_iter justiceiter = m_listJusticeRank.begin();
	tagJusticeRank* pJustice = NULL;
	while(m_listJusticeRank.find_next(justiceiter, pJustice))
	{
		if(VALID_POINT(pJustice))
		{
			SAFE_DELETE(pJustice);
		}
	}
	m_listJusticeRank.clear();

	
	LIST_ACHPOINTRANK::list_iter achpointiter = m_listAchPointRank.begin();
	tagAchievementPointRank* pAchievement = NULL;
	while(m_listAchPointRank.find_next(achpointiter, pAchievement))
	{
		if(VALID_POINT(pAchievement))
		{
			SAFE_DELETE(pAchievement);
		}
	}
	m_listAchPointRank.clear();

	LIST_ACHNUMBRRANK::list_iter achnumberiter = m_listAchNumberRank.begin();
	tagAchievementNumberRank* pAchnumber = NULL;
	while(m_listAchNumberRank.find_next(achnumberiter, pAchnumber))
	{
		if(VALID_POINT(pAchnumber))
		{
			SAFE_DELETE(pAchnumber);
		}
	}
	m_listAchNumberRank.clear();

	LIST_MASTERRANK::list_iter masteriter = m_listMasterRank.begin();
	tagMasterGraduateRank* pMaster = NULL;
	while(m_listMasterRank.find_next(masteriter,pMaster))
	{
		if (VALID_POINT(pMaster))
		{
			SAFE_DELETE(pMaster);
		}
	}
	m_listMasterRank.clear();

	LIST_MOUNTSRANK::list_iter mountsiter = m_listMountsRank.begin();
	tagMountsRank* pMounts = NULL;
	while(m_listMountsRank.find_next(mountsiter, pMounts))
	{
		if(VALID_POINT(pMounts))
		{
			SAFE_DELETE(pMounts);
		}
	}
	m_listMountsRank.clear();


	LIST_REACHRANK::list_iter reachiter = m_listReachRank.begin();
	tagtotalReachRank* pReach = NULL;
	while(m_listReachRank.find_next(reachiter, pReach))
	{
		if(VALID_POINT(pReach))
		{
			SAFE_DELETE(pReach);
		}
	}
	m_listReachRank.clear();

}

// 初始化魅力排行
VOID RankMgr::InitShihunRank(NET_DB2S_load_shihun_rank* rank)
{

	//DWORD dwRoleID[2];
	m_dwMeiliRole[0] = INVALID_VALUE;
	m_dwMeiliRole[1] = INVALID_VALUE;
	
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagShihunRank* pShihunRank = new tagShihunRank;
		memcpy(pShihunRank, &rank->s_shihun_rank[i], sizeof(tagShihunRank));
		m_listShihunRank.push_back(pShihunRank);

		if (pShihunRank->by_sex == 0 && m_dwMeiliRole[0] == INVALID_VALUE)
		{
			m_dwMeiliRole[0] = pShihunRank->dw_role_id;
		}


		if (pShihunRank->by_sex == 1 && m_dwMeiliRole[1] == INVALID_VALUE)
		{
			m_dwMeiliRole[1] = pShihunRank->dw_role_id;
		}
	}
	

	for (int i = 0; i < 2; i++)
	{
		if (m_dwMeiliRole[i] != INVALID_VALUE)
		{
			Role* pRole = g_roleMgr.get_role(m_dwMeiliRole[i]);
			if (VALID_POINT(pRole))
			{
				pRole->GetTitleMgr()->SetTitle(FIRSTNAN + i);
			}
			else
			{
				NET_DB2C_title_insert send;
				send.dw_role_id = m_dwMeiliRole[i];
				send.s_title_save_.n_title_id_ = FIRSTNAN + i;
				send.s_title_save_.dw_time = GetCurrentDWORDTime();
				g_dbSession.Send( &send, send.dw_size );
			}
		}
	
	}




}

DWORD RankMgr::SendShihunRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_meili_rank send;
	send.n_num = 0;

	LIST_SHIHUNRANK::list_iter iter = m_listShihunRank.begin();
	tagShihunRank* pShihunRank = NULL;
	while(m_listShihunRank.find_next(iter, pShihunRank))
	{
		if(!VALID_POINT(pShihunRank))
			continue;

		memcpy(&send.st_ShihunRank[send.n_num], pShihunRank, sizeof(tagmeiliRankInfo));
		send.n_num++;
	}

	send.dw_shihun = pRole->get_shihun();

	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}


VOID RankMgr::InitAchPointRank(NET_DB2S_load_achievement_point_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagAchievementPointRank* pRank = new tagAchievementPointRank;
		memcpy(pRank, &rank->s_point_rank[i], sizeof(tagAchievementPointRank));
		m_listAchPointRank.push_back(pRank);
	}
}

DWORD RankMgr::SendAchPointRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_ach_point_rank send;
	send.n_num = 0;

	LIST_ACHPOINTRANK::list_iter iter = m_listAchPointRank.begin();
	tagAchievementPointRank* pRank = NULL;
	while(m_listAchPointRank.find_next(iter, pRank))
	{
		if(!VALID_POINT(pRank))
			continue;

		memcpy(&send.st_achpointRank[send.n_num], pRank, sizeof(tagAchPointRankInfo));
		send.n_num++;
	}

	send.dw_point = pRole->GetAchievementPoint();

	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}
VOID RankMgr::InitAchNumberRank(NET_DB2S_load_achievement_number_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagAchievementNumberRank* pRank = new tagAchievementNumberRank;
		memcpy(pRank, &rank->s_number_rank[i], sizeof(tagAchievementNumberRank));
		m_listAchNumberRank.push_back(pRank);
	}
}

DWORD RankMgr::SendAchNumberRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_ach_number_rank send;
	send.n_num = 0;

	LIST_ACHNUMBRRANK::list_iter iter = m_listAchNumberRank.begin();
	tagAchievementNumberRank* pRank = NULL;
	while(m_listAchNumberRank.find_next(iter, pRank))
	{
		if(!VALID_POINT(pRank))
			continue;

		memcpy(&send.st_achnumberRank[send.n_num], pRank, sizeof(tagAchNumberRankInfo));
		send.n_num++;
	}

	send.dw_number = pRole->GetAchievementMgr().GetComplateNumber();

	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

VOID RankMgr::InitMountsRank(NET_DB2S_load_mounts_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagMountsRank* pRank = new tagMountsRank;
		memcpy(pRank, &rank->s_mounts_rank[i], sizeof(tagMountsRank));
		m_listMountsRank.push_back(pRank);
	}

}

VOID RankMgr::InitreachRank(NET_DB2S_load_reach_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagtotalReachRank* pRank = new tagtotalReachRank;
		memcpy(pRank, &rank->s_reach_rank[i], sizeof(tagtotalReachRank));
		m_listReachRank.push_back(pRank);
	}

}

VOID RankMgr::InitKillRank(NET_DB2S_load_kill_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagKillRank* pKillRank = new tagKillRank;
		memcpy(pKillRank, &rank->s_kill_rank_[i], sizeof(tagKillRank));
		m_listKillRank.push_back(pKillRank);
	}
}

// 发送恶人排行榜
DWORD RankMgr::SendKillRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_skill_rank send;
	send.n_num = 0;

	LIST_KILLRANK::list_iter iter = m_listKillRank.begin();
	tagKillRank* pKillRank = NULL;
	while(m_listKillRank.find_next(iter, pKillRank))
	{
		if(!VALID_POINT(pKillRank))
			continue;

		memcpy(&send.st_KillRank[send.n_num], pKillRank, sizeof(tagKillRank));
		send.n_num++;
	}

	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 初始化战力排行
VOID RankMgr::InitJustice(NET_DB2S_load_justice_rank* rank)
{

	//DWORD m_dwZhanLiRole[3][10];
	memset(m_dwZhanLiRole, 0, sizeof(m_dwZhanLiRole));

	int nzhanshiNum = 0;
	int nfashiNum = 0;
	int nDaoshiNum = 0;

	DWORD dwRoleName[3][2];
	memset(dwRoleName, 0, sizeof(dwRoleName));

	for(INT i = 0; i < rank->n_count; i++)
	{

		tagJusticeRank* pJustice = new tagJusticeRank;
		memcpy(pJustice, &rank->s_justice_rank_[i], sizeof(tagJusticeRank));
		m_listJusticeRank.push_back(pJustice);

		//if (nzhanshiNum >= 10 || nfashiNum >= 10 || nDaoshiNum >= 10)
		//	continue;

		if (pJustice->e_class == EV_Warrior && nzhanshiNum < 10 && m_dwZhanLiRole[0][nzhanshiNum] == 0 )
		{
			m_dwZhanLiRole[0][nzhanshiNum] = pJustice->dw_role_id;
			nzhanshiNum++;
		}
		if (pJustice->e_class == EV_Mage && nfashiNum < 10 && m_dwZhanLiRole[1][nfashiNum] == 0 )
		{
			m_dwZhanLiRole[1][nfashiNum] = pJustice->dw_role_id;
			nfashiNum++;
		}
		if (pJustice->e_class == EV_Taoist && nDaoshiNum < 10 && m_dwZhanLiRole[2][nDaoshiNum] == 0 )
		{
			m_dwZhanLiRole[2][nDaoshiNum] = pJustice->dw_role_id;
			nDaoshiNum++;
		}

		// 处理雕像名字用
		if (dwRoleName[pJustice->e_class - 1][pJustice->by_Sex] == 0)
		{
			dwRoleName[pJustice->e_class - 1][pJustice->by_Sex] = pJustice->dw_role_id;
			
			int nIndex = (pJustice->e_class - 1) * 2 + pJustice->by_Sex;

			g_roleMgr.get_role_name_by_name_id(pJustice->dw_role_id, &m_zhanliName[nIndex][0]);
			m_nLevel[pJustice->e_class - 1][pJustice->by_Sex] = pJustice->n_level_;
		}
	}



	// 给排行第一的人称号
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (m_dwZhanLiRole[i][j] != 0)
			{
				DWORD dwID = (j == 0 ? FIRSTZHANSHI:OTHERZHANSHI);
		
				Role* pRole = g_roleMgr.get_role(m_dwZhanLiRole[i][j]);
				if (VALID_POINT(pRole))
				{
					pRole->GetTitleMgr()->SetTitle(dwID + i);
				}
				else
				{
					NET_DB2C_title_insert send;
					send.dw_role_id = m_dwZhanLiRole[i][j];
					send.s_title_save_.n_title_id_ = dwID + i;
					send.s_title_save_.dw_time = GetCurrentDWORDTime();
					g_dbSession.Send( &send, send.dw_size );
				}
			}
		}

	}
	


}

DWORD RankMgr::SendJueticeRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_justice_rank send;
	send.n_num = 0;

	LIST_JUSTICERANK::list_iter iter = m_listJusticeRank.begin();
	tagJusticeRank* pJusticeRank = NULL;
	while(m_listJusticeRank.find_next(iter, pJusticeRank))
	{
		if(!VALID_POINT(pJusticeRank))
			continue;

		memcpy(&send.st_JusticeRank[send.n_num], pJusticeRank, sizeof(tagJusticeRank));
		send.n_num++;
	}

	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

VOID RankMgr::Init1v1ScoreRank(NET_DB2S_load_1v1_score_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tag1v1ScoreRank* p = new tag1v1ScoreRank;
		memcpy(p, &rank->st_1v1ScoreRank[i], sizeof(tag1v1ScoreRank));
		m_list1v1Rank.push_back(p);
	}
}

DWORD RankMgr::Send1v1ScoreRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_1v1_rank send;
	send.n_num = 0;

	LIST_1V1RANK::list_iter iter = m_list1v1Rank.begin();
	tag1v1ScoreRank* p1v1ScoreRank = NULL;
	while(m_list1v1Rank.find_next(iter, p1v1ScoreRank))
	{
		if(!VALID_POINT(p1v1ScoreRank))
			continue;

		memcpy(&send.st_1v1ScoreRank[send.n_num], p1v1ScoreRank, sizeof(tag1v1RankInfo));
		send.n_num++;
	}
	pRole->SendMessage(&send, send.dw_size);
}

// 初始等级排行榜
VOID RankMgr::InitLevelRank(NET_DB2S_load_level_rank* rank)
{
	memset(m_dwLevelRole, 0, sizeof(m_dwLevelRole));

	for(INT i = 0; i < rank->n_count; i++)
	{
		s_level_rank* pLevelRank = new s_level_rank;
		memcpy(pLevelRank, &rank->s_level_rank_[i], sizeof(s_level_rank));
		m_listLevelRank.push_back(pLevelRank);
		
		if (m_dwLevelRole[pLevelRank->e_class - 1] == 0)
		{
			m_dwLevelRole[pLevelRank->e_class - 1] = pLevelRank->dw_role_id;
		}


	}

	// 给排行第一的人称号
	if (rank->n_count > 0 && rank->s_level_rank_[0].n_level_ >= 40)
	{
		Role* pRole = g_roleMgr.get_role(rank->s_level_rank_[0].dw_role_id);
		if (VALID_POINT(pRole))
		{
			pRole->GetTitleMgr()->SetTitle(FIRSTLEVLETITLE);
		}
		else
		{
			NET_DB2C_title_insert send;
			send.dw_role_id = rank->s_level_rank_[0].dw_role_id;
			send.s_title_save_.n_title_id_ = FIRSTLEVLETITLE;
			send.s_title_save_.dw_time = GetCurrentDWORDTime();
			g_dbSession.Send( &send, send.dw_size );
		}
	}
	

	SetLock(FALSE);

	// 第一次执行脚本事件
	/*static bool bFirst = TRUE;
	if (bFirst)
	{
		const RankScript* pScript = g_ScriptMgr.GetRankScript();
		if (VALID_POINT(pScript)  )
		{
			pScript->OnInitRoleLevel();
		}
		bFirst = FALSE;
	}*/

}

VOID RankMgr::GetLevelRank(std::vector<DWORD>& vec)
{
	if (IsLock())
		return;

	m_listLevelRank.reset_iterator();
	s_level_rank* pLevelRank = NULL;
	int nNum = 0;
	while(m_listLevelRank.find_next(pLevelRank) && nNum < 100)
	{
		if (VALID_POINT(pLevelRank))
		{
			vec.push_back(pLevelRank->dw_role_id);
			nNum++;
		}
	}
}
// 发送等级排行榜
DWORD RankMgr::SendLevelRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_level_rank send;
	send.n_num = 0;

	LIST_LEVELRANK::list_iter iter = m_listLevelRank.begin();
	s_level_rank* pLevelRank = NULL;
	while(m_listLevelRank.find_next(iter, pLevelRank))
	{
		if(!VALID_POINT(pLevelRank))
			continue;

		memcpy(&send.st_LevelRank[send.n_num], pLevelRank, sizeof(s_level_rank));

		send.n_num++;
	}
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 初始化装备排行榜
VOID RankMgr::InitEquipRank(tagEquip* pEquip)
{
	tagEquip* pTempEquip = new tagEquip;
	get_fast_code()->memory_copy(pTempEquip, pEquip, sizeof(tagEquip));
	pTempEquip->pEquipProto = AttRes::GetInstance()->GetEquipProto(pEquip->dw_data_id);
	if(!VALID_POINT(pTempEquip->pEquipProto))
	{
		ASSERT(VALID_POINT(pTempEquip->pEquipProto));
		SAFE_DELETE(pTempEquip);
		return;
	}


	//开光属性的异常数据恢复
	//if (pTempEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Null && 
	//	pTempEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Physique)
	//{
	//	pTempEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
	//	pTempEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
	//}

	//if (pTempEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt == ERA_Physique)
	//{
	//	if (pTempEquip->equipSpec.byAddTalentPoint == 0 || 
	//		pTempEquip->equipSpec.EquipAttitionalAtt[7].nValue < 0 || 
	//		pTempEquip->equipSpec.EquipAttitionalAtt[7].nValue > 30)
	//	{
	//		pTempEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
	//		pTempEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
	//	}
	//}

	m_listEquipRank.push_back(pTempEquip);
}

// 发送装备排行榜
DWORD RankMgr::SendEquipRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_equip_rank send;
	send.n_num = 0;

	LIST_EQUIPRANK::list_iter iter = m_listEquipRank.begin();
	tagEquip* pEquip = NULL;
	while(m_listEquipRank.find_next(iter, pEquip))
	{
		if(!VALID_POINT(pEquip))
			continue;

		send.st_EquipRank[send.n_num].dw_data_id = pEquip->dw_data_id;
		//send.st_EquipRank[send.n_num].nLevel = pEquip->equipSpec.nLevel;
		send.st_EquipRank[send.n_num].nScore = pEquip->equipSpec.nRating;
		send.st_EquipRank[send.n_num].dw_role_id = pEquip->dwOwnerID;

		send.n_num++;
	}

	pRole->SendMessage(&send, send.dw_size);

	SendEquipInfo(pRole);

	return E_Success;
}

// 发送装备信息
VOID RankMgr::SendEquipInfo(Role* pRole)
{
	NET_SIS_equip_view_info send;
	LIST_EQUIPRANK::list_iter iter = m_listEquipRank.begin();
	tagEquip* pEquip = NULL;
	while(m_listEquipRank.find_next(iter, pEquip))
	{
		if(!VALID_POINT(pEquip))
			continue;

		send.st_EquipViewInfo[send.n_num].dw_data_id = pEquip->dw_data_id;
		//send.st_EquipViewInfo[send.n_num].nLevel = pEquip->equipSpec.nLevel;
		send.st_EquipViewInfo[send.n_num].byBind = pEquip->byBind;
		send.st_EquipViewInfo[send.n_num].byConsolidateLevel = pEquip->equipSpec.byConsolidateLevel;
		//send.st_EquipViewInfo[send.n_num].byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevelStar;
		send.st_EquipViewInfo[send.n_num].nUseTimes = pEquip->nUseTimes;
		send.st_EquipViewInfo[send.n_num].byHoldNum = pEquip->equipSpec.byHoleNum;
		//send.st_EquipViewInfo[send.n_num].n16MinDmg = pEquip->equipSpec.n16MinDmg;
		//send.st_EquipViewInfo[send.n_num].n16MaxDmg = pEquip->equipSpec.n16MaxDmg;
		//send.st_EquipViewInfo[send.n_num].n16Armor = pEquip->equipSpec.n16Armor;
		memcpy(send.st_EquipViewInfo[send.n_num].EquipAttitionalAtt, pEquip->equipSpec.EquipAttitionalAtt, sizeof(pEquip->equipSpec.EquipAttitionalAtt));
		memcpy(send.st_EquipViewInfo[send.n_num].dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);

		send.n_num++;
	}

	pRole->SendMessage(&send, send.dw_size);
}

// 初始帮会排行榜
VOID RankMgr::InitGuildRank(NET_DB2S_load_guild_rank* rank)
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagGuildRank* pGuildRank = new tagGuildRank;
		memcpy(pGuildRank, &rank->s_guild_rank_[i], sizeof(tagGuildRank));
		m_listGuildRank.push_back(pGuildRank);

		guild* pGuild = g_guild_manager.get_guild(pGuildRank->dw_guild_id);
		if (VALID_POINT(pGuild))
		{
			pGuild->setRank(i);
		}
	}
}

// 发送帮会排行榜
DWORD RankMgr::SendGulilRank(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_guild_rank send;
	send.n_num = 0;

	LIST_GUILDRANK::list_iter iter = m_listGuildRank.begin();
	tagGuildRank* pGuildRank = NULL;
	while(m_listGuildRank.find_next(iter, pGuildRank))
	{
		guild* pGuild = g_guild_manager.get_guild(pGuildRank->dw_guild_id);
		if (VALID_POINT(pGuild))
		{
			send.st_GuildRank[send.n_num].nProsperity = pGuild->get_guild_member_num();
		}
		send.st_GuildRank[send.n_num].dwGuildID = pGuildRank->dw_guild_id;
		send.st_GuildRank[send.n_num].dwLeaderID = pGuildRank->dw_leader_id_;
		send.st_GuildRank[send.n_num].nLevel = pGuildRank->n_level_;
		
		send.n_num++;
	}
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 整点更新排行榜
VOID RankMgr::UpdateRank()
{
	SetLock(TRUE);

	CleanRank();

	// 获取装备排行榜
	//NET_DB2C_load_equip_rank LoadEquipRank;
	//g_dbSession.Send(&LoadEquipRank, LoadEquipRank.dw_size);

	// 获取帮会排行榜
	NET_DB2C_load_guild_rank LoadGuildRank;
	g_dbSession.Send(&LoadGuildRank, LoadGuildRank.dw_size);

	// 获取恶人帮
	//NET_DB2C_load_kill_rank LoadKillRank;
	//g_dbSession.Send(&LoadKillRank, LoadKillRank.dw_size);

	// 获取正义榜
	NET_DB2C_load_justice_rank LoadJusticeRank;
	g_dbSession.Send(&LoadJusticeRank, LoadJusticeRank.dw_size);

	//NET_DB2C_load_1v1_score_rank Load1v1ScoreRank;
	//g_dbSession.Send(&Load1v1ScoreRank, Load1v1ScoreRank.dw_size);

	// 获取等级排行榜
	NET_DB2C_load_level_randk LoadLevelRank;
	g_dbSession.Send(&LoadLevelRank, LoadLevelRank.dw_size);

	// 获取噬魂排行榜
	NET_DB2C_load_shihun_rank LoadShihunRank;
	g_dbSession.Send(&LoadShihunRank, LoadShihunRank.dw_size);

	// 获取师徒排行榜 gx add 2013.12.06
	NET_DB2C_load_MasterGraduate_randk LoadMasterRank;
	g_dbSession.Send(&LoadMasterRank,LoadMasterRank.dw_size);
	// end

	// 获取坐骑排行榜
	NET_DB2C_load_mounts_rank LoadmountsRank;
	g_dbSession.Send(&LoadmountsRank, LoadmountsRank.dw_size);

	// 获取充值排行榜
	NET_DB2C_load_reach_rank LoadreachRank;
	g_dbSession.Send(&LoadreachRank, LoadreachRank.dw_size);


	// 获取成就点数行榜
	//NET_DB2C_load_achievement_point_rank LoadAchPointRank;
	//g_dbSession.Send(&LoadAchPointRank, LoadAchPointRank.dw_size);

	// 获取成就数量排行榜
	//NET_DB2C_load_achievement_number_rank LoadAchNumberRank;
	//g_dbSession.Send(&LoadAchNumberRank, LoadAchNumberRank.dw_size);
}

// 1v1积分竞技发奖
VOID RankMgr::Role1v1GiveAward()
{
	package_list<tagAward> list_award;

	LIST_1V1RANK::list_iter iter = m_list1v1Rank.begin();
	tag1v1ScoreRank* pScoreRank = NULL;
	INT	n_index = 1;
	INT n_score = 0;
	while(m_list1v1Rank.find_next(iter, pScoreRank))
	{
		if(!VALID_POINT(pScoreRank))
			continue;

		if(pScoreRank->dw_score <= 50)
			continue;

		if(list_award.size() == 0)
		{
			tagAward st_award;
			ZeroMemory(&st_award, sizeof(st_award));
			st_award.dw_role_id = pScoreRank->dw_role_id;
			st_award.n_index = n_index;
			n_score = pScoreRank->dw_score;
			list_award.push_back(st_award);
		}
		else
		{
			if(pScoreRank->dw_score == n_score)
			{
				tagAward st_award;
				ZeroMemory(&st_award, sizeof(st_award));
				st_award.dw_role_id = pScoreRank->dw_role_id;
				st_award.n_index = n_index;
				n_score = pScoreRank->dw_score;
				list_award.push_back(st_award);
			}
			else
			{
				n_index++;
				tagAward st_award;
				ZeroMemory(&st_award, sizeof(st_award));
				st_award.dw_role_id = pScoreRank->dw_role_id;
				st_award.n_index = n_index;
				n_score = pScoreRank->dw_score;
				list_award.push_back(st_award);
			}
		}
	}

	package_list<tagAward>::list_iter iter_award = list_award.begin();
	tagAward st_temp;
	while(list_award.find_next(iter_award, st_temp))
	{
		if(st_temp.n_index == 1)
		{
			Role* pRole = g_roleMgr.get_role(st_temp.dw_role_id);
			if(VALID_POINT(pRole))
			{
				pRole->get_1v1_score().n16_score_award = st_temp.n_index;

				pRole->Send1v1ScoreInfo();
			}

			NET_C2DB_update_1v1_award send;
			send.dw_role_id = st_temp.dw_role_id;
			send.n16_award = st_temp.n_index;
			g_dbSession.Send(&send, send.dw_size);
		}
		else
		{
			break;
		}
	}
}

// 噬魂给奖
VOID RankMgr::RoleShihunGiveAward()
{
	const RankScript* pRankScript = g_ScriptMgr.GetRankScript();
	if(VALID_POINT(pRankScript))
	{
		pRankScript->OnShihunGiveReward();
	}
}



DWORD RankMgr::GetRankPos(ERank_Type eType, DWORD dwRoleID)
{
	switch (eType)
	{
	case ERT_LEVELRANK:
		return GetRankPos<s_level_rank>(dwRoleID, m_listLevelRank);
	case ERT_KILLRANK:
		return GetRankPos<tagKillRank>(dwRoleID, m_listKillRank);
	case ERT_JUSTICERANK:
		return GetRankPos<tagJusticeRank>(dwRoleID, m_listJusticeRank);
	case ERT_1V1RANK:
		return GetRankPos<tag1v1ScoreRank>(dwRoleID, m_list1v1Rank);
	case ERT_SHIHUNRANK:
		return GetRankPos<tagShihunRank>(dwRoleID, m_listShihunRank);
	case ERT_ACHPOINTRANK:
		return GetRankPos<tagAchievementPointRank>(dwRoleID, m_listAchPointRank);
	case ERT_ACHNUMBRRANK:
		return GetRankPos<tagAchievementNumberRank>(dwRoleID, m_listAchNumberRank);
	case ERT_MOUNTS:
		return GetRankPos<tagMountsRank>(dwRoleID, m_listMountsRank);
	case ERT_CHONGZHI:
		return GetRankPos<tagtotalReachRank>(dwRoleID, m_listReachRank);
	}

	return 0;
}

DWORD RankMgr::GetRankPosClass(ERank_Type eType, DWORD dwRoleID, EClassType eClass)
{
	switch (eType)
	{
	case ERT_LEVELRANK:
		return GetRankPosClass<s_level_rank>(dwRoleID, eClass, m_listLevelRank);
	//case ERT_KILLRANK:
	//	return GetRankPos<tagKillRank>(dwRoleID, m_listKillRank);
	case ERT_JUSTICERANK:
		return GetRankPosClass<tagJusticeRank>(dwRoleID, eClass, m_listJusticeRank);
	//case ERT_1V1RANK:
	//	return GetRankPos<tag1v1ScoreRank>(dwRoleID, m_list1v1Rank);
	//case ERT_SHIHUNRANK:
	//	return GetRankPos<tagShihunRank>(dwRoleID, m_listShihunRank);
	//case ERT_ACHPOINTRANK:
	//	return GetRankPos<tagAchievementPointRank>(dwRoleID, m_listAchPointRank);
	//case ERT_ACHNUMBRRANK:
	//	return GetRankPos<tagAchievementNumberRank>(dwRoleID, m_listAchNumberRank);
	//case ERT_MOUNTS:
	//	return GetRankPos<tagMountsRank>(dwRoleID, m_listMountsRank);
	//case ERT_CHONGZHI:
	//	return GetRankPos<tagtotalReachRank>(dwRoleID, m_listReachRank);
	}

	return 0;
}
VOID RankMgr::sendDiaoxiangName(Role* pRole)
{
	NET_SIS_get_diaoxiang_name send;
	memcpy(send.szDiaoName, m_zhanliName, sizeof(send.szDiaoName));
	pRole->SendMessage(&send, send.dw_size);
}

INT32 RankMgr::getZhanliLevel(DWORD dwclass, DWORD dwSex)
{
	if (dwclass >= 0 && dwclass < 3 && dwSex >= 0 && dwSex < 2)
	{
		return m_nLevel[dwclass][dwSex];
	}

	return 0;
}

VOID RankMgr::InitMasterRank( NET_DB2S_load_MasterGraduate_rank* rank )
{
	for(INT i = 0; i < rank->n_count; i++)
	{
		tagMasterGraduateRank* pMasterRank = new tagMasterGraduateRank;
		memcpy(pMasterRank, &rank->s_master_rank_[i], sizeof(tagMasterGraduateRank));
		m_listMasterRank.push_back(pMasterRank);
	}

	// 给排行第一的人称号
	if (rank->n_count > 0 && rank->s_master_rank_[0].n_level_ >= 60)
	{
		Role* pRole = g_roleMgr.get_role(rank->s_master_rank_[0].dw_role_id);
		if (VALID_POINT(pRole))
		{
			pRole->GetTitleMgr()->SetTitle(MASTERRANK1);
		}
		else
		{
			NET_DB2C_title_insert send;
			send.dw_role_id = rank->s_master_rank_[0].dw_role_id;
			send.s_title_save_.n_title_id_ = MASTERRANK1;
			send.s_title_save_.dw_time = GetCurrentDWORDTime();
			g_dbSession.Send( &send, send.dw_size );
		}
	}

	// 给排行第二的人称号
	if (rank->n_count > 1 && rank->s_master_rank_[1].n_level_ >= 60)
	{
		Role* pRole = g_roleMgr.get_role(rank->s_master_rank_[1].dw_role_id);
		if (VALID_POINT(pRole))
		{
			pRole->GetTitleMgr()->SetTitle(MASTERRANK2);
		}
		else
		{
			NET_DB2C_title_insert send;
			send.dw_role_id = rank->s_master_rank_[1].dw_role_id;
			send.s_title_save_.n_title_id_ = MASTERRANK2;
			send.s_title_save_.dw_time = GetCurrentDWORDTime();
			g_dbSession.Send( &send, send.dw_size );
		}
	}

	// 给排行第三的人称号
	if (rank->n_count > 2 && rank->s_master_rank_[2].n_level_ >= 60)
	{
		Role* pRole = g_roleMgr.get_role(rank->s_master_rank_[2].dw_role_id);
		if (VALID_POINT(pRole))
		{
			pRole->GetTitleMgr()->SetTitle(MASTERRANK3);
		}
		else
		{
			NET_DB2C_title_insert send;
			send.dw_role_id = rank->s_master_rank_[2].dw_role_id;
			send.s_title_save_.n_title_id_ = MASTERRANK3;
			send.s_title_save_.dw_time = GetCurrentDWORDTime();
			g_dbSession.Send( &send, send.dw_size );
		}
	}
	////排行榜4-10，不考虑前三位
	//for (int i = 3;i < min(rank->n_count,9);i++)
	//{
	//	/*if (rank->s_master_rank_[i].n_Graduates >= 10)*/
	//	{
	//		Role* pRole = g_roleMgr.get_role(rank->s_master_rank_[i].dw_role_id);
	//		if (VALID_POINT(pRole))
	//		{
	//			pRole->GetTitleMgr()->SetTitle(MASTERRANK4);
	//		}
	//		else
	//		{
	//			NET_DB2C_title_insert send;
	//			send.dw_role_id = rank->s_master_rank_[i].dw_role_id;
	//			send.s_title_save_.n_title_id_ = MASTERRANK4;
	//			send.s_title_save_.dw_time = GetCurrentDWORDTime();
	//			g_dbSession.Send( &send, send.dw_size );
	//		}
	//	}
	//}
}

DWORD RankMgr::SendMasterRank( Role* pRole )
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(IsLock())
		return e_rank_lock;

	NET_SIS_get_master_rank send;
	send.n_num = 0;

	LIST_MASTERRANK::list_iter iter = m_listMasterRank.begin();
	tagMasterGraduateRank* pMasterRank = NULL;
	while(m_listMasterRank.find_next(iter, pMasterRank))
	{
		if(!VALID_POINT(pMasterRank))
			continue;

		memcpy(&send.st_MasterRank[send.n_num], pMasterRank, sizeof(tagMasterGraduateRank));

		send.n_num++;
	}
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 是否是职业等级第一
BOOL	RankMgr::IsClessOne(DWORD dwRoleID, int nCless)
{
	return (m_dwLevelRole[nCless] == dwRoleID);
}
// 是否是职业战力第一
BOOL	RankMgr::IsZhanLiClassOne(DWORD dwRoleID, int nClass)
{
	
	return (m_dwZhanLiRole[nClass][0] == dwRoleID);
}
// 是否坐骑等级第一
BOOL	RankMgr::IsMountsOne(DWORD dwRoleID)
{
	tagMountsRank* p = *(m_listMountsRank.begin());
	return (p->dw_role_id == dwRoleID);
}
// 是否充值排第一
BOOL	RankMgr::IsChongZhiOne(DWORD dwRoleID)
{
	tagtotalReachRank* p = *(m_listReachRank.begin());

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (VALID_POINT(pRole) && VALID_POINT(pRole->GetSession()))
	{
		return (p->dw_role_id == pRole->GetSession()->GetSessionID());
	}
	
	return false;
}
// 是否总战力第一
BOOL	RankMgr::IsZhanliOne(DWORD dwRoleID)
{
	tagJusticeRank* p = *(m_listJusticeRank.begin());
	return (p->dw_role_id == dwRoleID);
}
// 是否魅力第一
BOOL	RankMgr::IsMeiLiOne(DWORD dwRoleID, int nSex)
{
	return dwRoleID == m_dwMeiliRole[nSex];

}

// 设置头奖三人
VOID RankMgr::SetThreeFirstPeople(DWORD dwRoleID, DWORD dwType)
{
	tagFirstThreePepop data;
	data.dwRoleID = dwRoleID;
	data.dwType = dwType;

	if (m_listFirstThree.size() >= 3)
	{
		m_listFirstThree.pop_front();
	}
	
	m_listFirstThree.push_back(data);


}

// 获取头奖数据
VOID RankMgr::getOpenServerData(Role* pRole, DWORD* pData)
{

	// 等级
	(*pData) = m_dwLevelRole[0];
	(*(pData + 1)) = m_dwLevelRole[1];
	(*(pData + 2)) = m_dwLevelRole[2];

	// 战力
	(*(pData + 3)) = m_dwZhanLiRole[0][0];
	(*(pData + 4)) = m_dwZhanLiRole[0][1];
	(*(pData + 5)) = m_dwZhanLiRole[0][2];

	// 坐骑
	tagMountsRank* p1 = *(m_listMountsRank.begin());
	(*(pData + 6)) = p1->dw_role_id;

	// 沙巴克公会
	(*(pData + 9)) = g_guild_manager.get_SBK_guild();

	// 充值排行
	tagtotalReachRank* p2 = *(m_listReachRank.begin());
	(*(pData + 12)) = p2->dw_role_id;
	

	// 魅力
	(*(pData + 15)) = m_dwMeiliRole[0];
	(*(pData + 16)) = m_dwMeiliRole[1];

	// 战力
	tagJusticeRank* p3 = *(m_listJusticeRank.begin());
	(*(pData + 18)) = p3->dw_role_id;


}
//RankMgr g_RankMgr;
