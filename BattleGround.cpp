#include "stdafx.h"

#include "BattleGround.h"
#include "activity_mgr.h"
#include "map_instance_battle.h"
#include "creature.h"
#include "role.h"
#include "role_mgr.h"
#include "../../common/WorldDefine/battle_ground_protocol.h"

BattleGround::BattleGround():
m_eBattleState(EBS_END),
m_pMapBattle(NULL)
{
	m_dwLeaderID[EBT_A] = INVALID_VALUE;
	m_dwLeaderID[EBT_B] = INVALID_VALUE;
}

BattleGround::~BattleGround()
{

}

VOID BattleGround::setState( EBATTLEGROUNDSTATE eState )
{
	m_eBattleState = eState;
}

EBATTLEGROUNDSTATE BattleGround::getState()
{
	return m_eBattleState;
}

VOID BattleGround::setMap(map_instance_battle* pMap)
{
	m_pMapBattle = pMap;
}

VOID BattleGround::addRole( DWORD dwRoleID, INT nLevel, EBATTLENTEAM eTeam)
{
	//	EBATTLENTEAM eTeam = getTeam();

	if (m_setTeam[eTeam].find(dwRoleID) == m_setTeam[eTeam].end())
	{
		tagBattleRoleData data;
		data.dwRoleID = dwRoleID;
		data.nLevel = nLevel;
		data.byBattleType = (BYTE)eTeam;
		m_setTeam[eTeam].insert(make_pair(dwRoleID, data));
	}
	
}

VOID BattleGround::delRole( DWORD dwRoleID )
{
	for (int i = EBT_A; i != EBT_NUM; i++)
	{
		std::map<DWORD, tagBattleRoleData>::iterator it = m_setTeam[i].find(dwRoleID);
		if (it != m_setTeam[i].end())
		{
			m_setTeam[i].erase(it);

			return;
		}
	}

	
}

EBATTLENTEAM BattleGround::getTeam()
{
	INT nTeamA = getTeamNum(EBT_A);
	INT nTeamB = getTeamNum(EBT_B);

	if (nTeamA <= nTeamB)
	{
		return EBT_A;
	}
	
	return EBT_B;
}

INT BattleGround::getTeamNum( EBATTLENTEAM eTeamType )
{
	return m_setTeam[eTeamType].size();
}

INT BattleGround::getRoleNum()
{
	return m_setTeam[EBT_A].size() + m_setTeam[EBT_B].size();
}

VOID BattleGround::update(tagDWORDTime time)
{
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(BATTLE_ACTIVE_ID);
	if(!VALID_POINT(pActivity)) 
		return;

	
	if( m_eBattleState == EBS_END && pActivity->is_start() )
	{
		m_eBattleState = EBS_BEGIN;

		return;
	}
	


	if (m_eBattleState == EBS_BEGIN)
	{
		if (pActivity->get_minute_update_count() >= BATLLE_REAL_TIME)
		{
			m_eBattleState = EBS_ING;
		}

		return;
	}
		
	if (m_eBattleState == EBS_ING && !pActivity->is_start())
	{
		m_eBattleState = EBS_END;

		return;
	}

}

VOID BattleGround::killUnit( DWORD dwRoleID, BYTE byKillType )
{
	for (int i = EBT_A; i != EBT_NUM; i++)
	{
		std::map<DWORD, tagBattleRoleData>::iterator it = m_setTeam[i].find(dwRoleID);
		if (it != m_setTeam[i].end())
		{
			// 击杀玩家
			if (byKillType == 0)
			{
				it->second.nKillNum1++;
				it->second.nScore += 2;
				
			}// 击杀卫兵
			else if (byKillType == 1)
			{
				it->second.nKillNum2++;
				it->second.nScore += 10;
			}// 击杀首领
			else if (byKillType == 2)
			{
				it->second.nKillNum3++;
				it->second.nScore += 50;
			}

			return;
		}
	}
}

VOID BattleGround::BeKill(DWORD dwRoleID, DWORD dwKiller)
{
	int nKillNum = 0;
	for (int i = EBT_A; i != EBT_NUM; i++)
	{
		// 连杀归零
		std::map<DWORD, tagBattleRoleData>::iterator it = m_setTeam[i].find(dwRoleID);
		if (it != m_setTeam[i].end())
		{
			nKillNum = it->second.nKillSum;
			it->second.nKillSum = 0;
			break;
		}
	}

	for (int i = EBT_A; i != EBT_NUM; i++)
	{
		// 累计连杀数
		std::map<DWORD, tagBattleRoleData>::iterator it = m_setTeam[i].find(dwKiller);
		if (it != m_setTeam[i].end())
		{
			it->second.nKillSum ++;

			// 根据杀人的连杀数增加荣誉
			if (nKillNum >= 3 && nKillNum <= 6)
			{
				it->second.nScore += 3;
			}
			else if(nKillNum > 6 && nKillNum <= 9)
			{
				it->second.nScore += 4;
			}
			else if(nKillNum >= 10)
			{
				it->second.nScore += 5;
			}
			break;
		}
	}

}

VOID BattleGround::end( DWORD dwActivityID )
{
	if (dwActivityID != BATTLE_ACTIVE_ID)
		return;

	Creature* pTeamA = m_pMapBattle->find_creature(m_dwLeaderID[EBT_A]);
	Creature* pTeamB = m_pMapBattle->find_creature(m_dwLeaderID[EBT_B]);

	if (!VALID_POINT(pTeamA) || !VALID_POINT(pTeamB))
	{
		reset();
		return;
	}
	
	if (pTeamA->GetAttValue(ERA_HP) > pTeamB->GetAttValue(ERA_HP))
	{
		setWiner(EBT_A, FALSE);
	}
	else
	{
		setWiner(EBT_B, FALSE);
	}

	
}

VOID BattleGround::setWiner( EBATTLENTEAM eTeam, BOOL bKillBoss )
{
	// 发送给客户端
	sendToClient();

	// 获得奖励
	for (int i = 0; i < EBT_NUM; i++)
	{
		std::map<DWORD, tagBattleRoleData>::iterator it = m_setTeam[i].begin();
		for (; it != m_setTeam[i].end(); it++)
		{
			Role* pRole = g_roleMgr.get_role(it->first);
			
			if (VALID_POINT(pRole))
			{
				pRole->GetCurMgr().IncExploits(it->second.nScore, elcid_pvp);
				if (VALID_POINT(pRole->GetScript()))
				{
					BOOL bWin = (eTeam == i);
					pRole->GetScript()->OnGetBattleGift(pRole, bWin, it->second.nRank);
				}
				
			}
			
		}
		
	}


	reset();
}

VOID BattleGround::setTeamLeader( EBATTLENTEAM eTeam, DWORD dwID )
{
	m_dwLeaderID[eTeam] = dwID;
}

VOID BattleGround::reset()
{
	// 结束副本
	m_pMapBattle->remove_all_player();

	for (int i = 0; i < EBT_NUM; i++)
	{
		m_setTeam[i].clear();
		//m_dwLeaderID[i] = INVALID_VALUE;
	}
	
}


bool LessBattleResule(const tagBattleResule & m1, const tagBattleResule & m2) 
{
	int a1 = m1.dwKillBoss + m1.dwKillNum + m1.dwKillSorder;
	int a2 = m2.dwKillBoss + m2.dwKillNum + m2.dwKillSorder;
	return a1 < a2;
}

VOID BattleGround::sendToClient()
{
	// 计算排名
	std::vector<tagBattleResule> vecBattleResule;

	for (int i = 0; i < EBT_NUM; i++)
	{
		std::map<DWORD, tagBattleRoleData>::iterator it = m_setTeam[i].begin();
		for (; it != m_setTeam[i].end(); it++)
		{
			tagBattleResule sBattle;
			memset(&sBattle, 0, sizeof(sBattle));
			sBattle.byType = i;
			sBattle.dwRoleID = it->second.dwRoleID;
			sBattle.dwKillNum = it->second.nKillNum1;
			sBattle.dwKillBoss = it->second.nKillNum2;
			sBattle.dwKillSorder = it->second.nKillNum3;
			sBattle.dwScore = it->second.nScore;
			sBattle.dwLevel = it->second.nLevel;

			vecBattleResule.push_back(sBattle);
		}
	}
	

	std::sort(vecBattleResule.begin(), vecBattleResule.end(), LessBattleResule);

	// 发送消息给客户端
	DWORD dwNumber = m_setTeam[EBT_A].size() + m_setTeam[EBT_B].size();
	
	INT	nMsgSize = sizeof(NET_SIS_battle_ground_end) + (dwNumber - 1) * sizeof(tagBattleResule);

	CREATE_MSG(pSend, nMsgSize, NET_SIS_battle_ground_end);

	pSend->dwNumber = dwNumber;
	
	for (int i = 0; i < dwNumber; i++)
	{
		memcpy(&pSend->sBattleData[i], &vecBattleResule[i], sizeof(tagBattleResule));
		if (i < 10)
		{
			BYTE type = vecBattleResule[i].byType;
			DWORD dwRoleID = vecBattleResule[i].dwRoleID;
			m_setTeam[type][dwRoleID].nRank = i + 1;
		}
	}

	for (int i = 0; i < dwNumber; i++)
	{
		
		DWORD dwRoleID = vecBattleResule[i].dwRoleID;
		Role* pRole = g_roleMgr.get_role(dwRoleID);
		if (VALID_POINT(pRole))
		{
			pRole->SendMessage(pSend, pSend->dw_size);
		}
	}

	MDEL_MSG(pSend);
}




