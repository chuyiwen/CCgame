/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�����ʩ


#include "StdAfx.h"

#include "../common/ServerDefine/guild_server_define.h"
#include "../common/ServerDefine/guild_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "../../common/WorldDefine/guild_protocol.h"
#include "../../common/WorldDefine/guild_define.h"

#include "att_res.h"
#include "world.h"
#include "role.h"
#include "guild.h"
#include "guild_upgrade.h"
#include "map.h"
#include "map_creator.h"
#include "map_instance_guild.h"
#include "creature.h"

//-----------------------------------------------------------------------------
// ���������
//-----------------------------------------------------------------------------
GuildUpgrade::GuildUpgrade()
{
}

GuildUpgrade::~GuildUpgrade()
{
}

//-----------------------------------------------------------------------------
// ��ʼ��
//-----------------------------------------------------------------------------
VOID GuildUpgrade::Init(guild* pGuild, BOOL bRequest /*= FALSE*/)
{
	if (!VALID_POINT(pGuild))
	{
		ASSERT(pGuild);
		return;
	}

	m_bInit		= FALSE;
	m_pGuild	= pGuild;

	if (bRequest)
	{
		NET_DB2C_load_facilities_info send;
		send.dw_guild_id = m_pGuild->get_guild_att().dwID;
		g_dbSession.Send(&send, send.dw_size);
	}
}

//-----------------------------------------------------------------------------
// ��ȡ��ʩ������Ϣ
//-----------------------------------------------------------------------------
DWORD GuildUpgrade::GetGuildFacilitiesInfo( tagGuildFacilitiesInfo* pInfo, EFacilitiesType eType /*= EFT_End*/ )
{
	// �ϲ��뱣֤pInfo�ṹ�Ĵ�С
	if (!VALID_POINT(pInfo))
		return INVALID_VALUE;

	if (!m_bInit)
		return INVALID_VALUE;

	if (eType >=EFT_Lobby && eType <= EFT_Holiness)
	{
		get_fast_code()->memory_copy(pInfo, &m_Facilities[eType], sizeof(tagGuildFacilitiesInfo));
	}
	else
	{
		for (int n=EFT_Lobby; n<EFT_End; n++)
		{
			get_fast_code()->memory_copy(&pInfo[n], &m_Facilities[n], sizeof(tagGuildFacilitiesInfo));
		}
	}

	return 0;
}

VOID GuildUpgrade::UpDate()
{
	for(INT i = EFT_Lobby; i < EFT_End; i++)
	{
		if(i == EFT_Lobby)
		{
			if(m_Facilities[i].byUseType == 1)
			{
				if(CalcTimeDiff(g_world.GetWorldTime(), m_Facilities[i].dwUseTime) >= LOBBY_USE_TIME)
				{
					Map* pNormalMap = g_mapCreator.get_map(m_pGuild->get_tripod_map_id(), INVALID_VALUE);
					if(VALID_POINT(pNormalMap))
					{
						Creature* pCreature = pNormalMap->find_creature(m_pGuild->get_tripod_id());
						if(VALID_POINT(pCreature))
						{
							pCreature->UnsetState(ES_Occupied);
							pCreature->SetGuildID(INVALID_VALUE);
						}
					}

			
					map_instance_guild* pGuildInstance = m_pGuild->get_guild_map();
					if( VALID_POINT(pGuildInstance) )
					{
						Creature* pTripod = pGuildInstance->find_creature(m_pGuild->get_upgrade().GetFacilitiesID(EFT_Lobby));
						if(VALID_POINT(pTripod))
							pTripod->UnsetState(ES_Occupied);
					}
						
					m_Facilities[i].byUseType = 0;
					m_pGuild->set_tripod_id(INVALID_VALUE);
					m_pGuild->set_tripod_map_id(INVALID_VALUE);
					SaveUpgradeInfo2DB(EFT_Lobby);

					NET_SIS_Guild_Practice_Broad send;
					send.b_Start = FALSE;
					m_pGuild->send_guild_message(&send, send.dw_size);
				}
			}
		}

		if(m_Facilities[i].IsLevelUp())
		{
			//if(CalcTimeDiff(g_world.GetWorldTime(), m_Facilities[i].dwBeginUpTime) >= m_Facilities[i].nUpLevelLimit)
			{
				m_Facilities[i].bUpLevel = FALSE;

				ChangeFacilitesLevel((EFacilitiesType)i);

			
				/*map_instance_guild* pGuildInstance = m_pGuild->get_guild_map();
				if( VALID_POINT(pGuildInstance) )
				{
					
					DWORD dwCreatureID = GetFacilitiesID(i);
					Creature* pDelCreatrue = pGuildInstance->find_creature(dwCreatureID);
					if(VALID_POINT(pDelCreatrue))
					{
						pDelCreatrue->OnDisappear();
					}

					pGuildInstance->init_guild_build(m_pGuild, (BYTE)i);
				}*/
					
				// �㲥������ʩ������Ϣ
				NET_SIS_guild_upgrade send;
				send.eType		= (EFacilitiesType)i;
				send.byNewLevel = m_Facilities[i].byLevel;
				m_pGuild->send_guild_message(&send, send.dw_size);
			}
		}

		//if(m_pGuild->get_guild_war().get_guild_war_state() != EGWS_WAR)
		//{
		//	if(m_Facilities[i].IsDestory())
		//	{
		//		if(CalcTimeDiff(g_world.GetWorldTime(), m_Facilities[i].dwDeadBeginTime) >= m_Facilities[i].dwReliveTimeLimit)
		//		{
		//			FacilitesRelive((EFacilitiesType)i);

		//			map_instance_guild* pGuildInstance = m_pGuild->get_guild_map();
		//			if( VALID_POINT(pGuildInstance) )
		//			{
		//				pGuildInstance->init_guild_build(m_pGuild, (BYTE)i);
		//			}
		//				
		//		}
		//	}
		//}
	}
}

//-----------------------------------------------------------------------------
//�ı��Ὠ���ȼ�
//-----------------------------------------------------------------------------
VOID GuildUpgrade::ChangeFacilitesLevel(EFacilitiesType eType, BOOL bAdd)
{
	// �����ȡ��һ������������Ʒ
	if(bAdd)
	{
		m_Facilities[eType].byLevel++;
	}
	else
	{
		m_Facilities[eType].byLevel--;

		//if(eType == EFT_Lobby)
		//{
		//	if(m_Facilities[eType].byLevel < 1)
		//	{
		//		m_pGuild->dismiss_guild();
		//		return;
		//	}
		//}

		if(m_Facilities[eType].byLevel < 1)
		{
			m_Facilities[eType].byLevel = 1;
		}
	}

	m_Facilities[eType].nProgress	= 0;
	if(eType == EFT_Lobby)
	{
		if (m_Facilities[eType].byLevel >= MAX_GUILD_LEVEL)
		{
			// ������ʩ����
			m_Facilities[eType].byLevel		= MAX_GUILD_LEVEL;
			m_Facilities[eType].nFulfill	= 0;
			m_Facilities[eType].nNeedFund	= 0;
			m_Facilities[eType].nMaterial	= 0;
			m_Facilities[eType].nStep		= 0;
			m_Facilities[eType].nBaseExploit= 0;
			m_Facilities[eType].nUpLevelLimit = 0;
		}
		else
		{
			AttRes::GetInstance()->GetGuildUpgradeItemInfo(eType, m_Facilities[eType].byLevel, m_Facilities[eType]);
		}
	}
	else
	{
		if (m_Facilities[eType].byLevel >= MAX_GUILD_LEVEL)
		{
			// ������ʩ����
			m_Facilities[eType].byLevel		= MAX_GUILD_LEVEL;
			m_Facilities[eType].nFulfill	= 0;
			m_Facilities[eType].nNeedFund	= 0;
			m_Facilities[eType].nMaterial	= 0;
			m_Facilities[eType].nStep		= 0;
			m_Facilities[eType].nBaseExploit= 0;
			m_Facilities[eType].nUpLevelLimit = 0;
		}
		else
		{
			AttRes::GetInstance()->GetGuildUpgradeItemInfo(eType, m_Facilities[eType].byLevel, m_Facilities[eType]);
		}
	}

	//if(m_Facilities[eType].eType == EFT_Maidan)
	//{
	//	if(bAdd)
	//	{
	//		if(m_Facilities[eType].byLevel == 6)
	//		{
	//			m_Facilities[EFT_Holiness].byLevel = 1;
	//			AttRes::GetInstance()->GetGuildUpgradeItemInfo(EFT_Holiness, m_Facilities[EFT_Holiness].byLevel, m_Facilities[EFT_Holiness]);

	//			// ���浽���ݿ�
	//			SaveUpgradeInfo2DB(EFT_Holiness);

	//			DWORD dwGuildInstanceID = m_pGuild->get_guild_instance_id();
	//			if( VALID_POINT(dwGuildInstanceID) )
	//			{
	//				DWORD dwGuildMapID = get_tool()->crc32(szGuildMapName);
	//				Map* pMap = g_mapCreator.get_map(dwGuildMapID, dwGuildInstanceID);

	//				if( VALID_POINT(pMap) )
	//				{
	//					map_instance_guild* pGuildInstance = dynamic_cast<map_instance_guild*>(pMap);
	//					if( VALID_POINT(pGuildInstance) )
	//					{
	//						pGuildInstance->init_guild_build(m_pGuild, (BYTE)eType);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//if (eType == EFT_Lobby)
	//{
		//m_pGuild->reinit_guild_upgrade(m_Facilities[eType].byLevel);
	//	m_pGuild->unset_guild_state(EGDSS_UpLevel);
		//INT32 n32_DecProsperity =  (m_Facilities[eType].byLevel)*MGuildMaxMember(m_Facilities[eType].byLevel)*MGuildMaxMember(m_Facilities[eType].byLevel);/*pow((double)((m_Facilities[eType].byLevel)*MGuildMaxMember(m_Facilities[eType].byLevel)), 2)*/;
		//n32_DecProsperity /= 10;
		//m_Facilities[eType].nDayDecProsperity = n32_DecProsperity;
	//}
	
	// ���浽���ݿ�
	SaveUpgradeInfo2DB((EFacilitiesType)eType);
	
	map_instance_guild* pGuildInstance = m_pGuild->get_guild_map();
	if( VALID_POINT(pGuildInstance) )
	{
		
		for(int i = 0; i < MAX_GUILD_UPGRADE_NUM; i++)
		{
			DWORD dwCreatureID = GetFacilitiesID(eType, i);
			Creature* pDelCreatrue = pGuildInstance->find_creature(dwCreatureID);
			if(VALID_POINT(pDelCreatrue))
			{
				pDelCreatrue->OnDisappear();
			}
		}


		pGuildInstance->init_guild_build(m_pGuild, (BYTE)eType);
	}

	NET_SIS_update_facilities_info SendInfo;
	m_pGuild->get_guild_facilities().GetGuildFacilitiesInfo(&SendInfo.sFacilitiesInfo, (EFacilitiesType)eType);
	m_pGuild->send_guild_message(&SendInfo, SendInfo.dw_size);
}

//-----------------------------------------------------------------------------
// ��ʩ�Ƿ�����
//-----------------------------------------------------------------------------
BOOL GuildUpgrade::IsFacilitesUpLevel()
{
	for(INT i = EFT_Lobby; i < EFT_End; i++)
	{
		if(m_Facilities[i].bUpLevel)
			return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// ��ʩ�Ƿ�ݻ�
//-----------------------------------------------------------------------------
BOOL GuildUpgrade::IsFacilitesDestory()
{
	for(INT i = EFT_Lobby; i < EFT_End; i++)
	{
		if(m_Facilities[i].bDead)
			return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// ��¯�Ƿ��ѱ�����ʹ��
//-----------------------------------------------------------------------------
BOOL GuildUpgrade::IsLobbyUsing()
{
	if(m_Facilities[EFT_Lobby].byUseType == 1)
		return TRUE;
	return FALSE;
}

//-----------------------------------------------------------------------------
// ��ʩ�Ƿ�ݻ�
//-----------------------------------------------------------------------------
BOOL GuildUpgrade::IsFacilitesDestory(EFacilitiesType eType)
{
	if(eType < EFT_Lobby || eType > EFT_Holiness)
		return TRUE;

	if(m_Facilities[eType].bDead)
		return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
// ������ʩ����״̬
//-----------------------------------------------------------------------------
VOID GuildUpgrade::SetFacilitesUpLevel(BYTE byType)
{
	if(byType < EFT_Lobby || byType > EFT_Holiness)
		return ;

	m_Facilities[byType].dwBeginUpTime = g_world.GetWorldTime();
	m_Facilities[byType].bUpLevel = TRUE;
}

// ��հ��ʥ��ʹ�ô���
VOID GuildUpgrade::ClearHolinessNum()
{
	m_Facilities[EFT_Holiness].byUseNum = 0;
	SaveUpgradeInfo2DB(EFT_Holiness);
}

//-----------------------------------------------------------------------------
// ÿ�տ۳�����ʩ���ٶ�
//-----------------------------------------------------------------------------
VOID GuildUpgrade::DayDecProsperity()
{
	
	// ÿ������20%�ʽ�ת��Ϊ���ٶ�
	INT32 nCurFund = m_pGuild->get_guild_att().nFund;
	if (nCurFund > 1000000)
	{
		m_pGuild->decrease_guild_fund(INVALID_VALUE, nCurFund*GuildHelper::getMoneyToFanlong(m_pGuild->get_guild_att().byLevel), elcid_guild_daily_cost);
		m_pGuild->increase_prosperity(INVALID_VALUE, nCurFund/10000*GuildHelper::getMoneyToFanlong(m_pGuild->get_guild_att().byLevel), elcid_guild_daily_cost);
	}
	else
	{
		m_pGuild->decrease_guild_fund(INVALID_VALUE, nCurFund, elcid_guild_daily_cost);
		m_pGuild->increase_prosperity(INVALID_VALUE, nCurFund/10000, elcid_guild_daily_cost);
	}
	
	// û���ʽ𣬿۳����ٶ�
	if (nCurFund <= 0)
	{
		m_pGuild->decrease_prosperity(INVALID_VALUE, 100, elcid_guild_daily_cost);
	}

	//INT32 nDayDecProsperity = 0;
	//for(INT i = EFT_Lobby; i < EFT_End; i++)
	//{
	//	if(i == EFT_Lobby || i == EFT_Factory || i == EFT_Holiness)
	//	{
	//		if(m_Facilities[i].nDayDecProsperity > 0)
	//		{
	//			nDayDecProsperity += m_Facilities[i].nDayDecProsperity;
	//		}
	//	}
	//}
	// ÿ��ָ�20�㰲��
	m_pGuild->increase_guild_peace(INVALID_VALUE, 20, elcid_guild_daily_cost);

	if(m_pGuild->get_guild_att().nProsperity <= 0)
	{
		byte byNewLevel = m_pGuild->get_guild_att().byLevel - 1;
		m_pGuild->reinit_guild_upgrade(byNewLevel);
		
		for (int i = EFT_Lobby; i < EFT_End; i++)
		{
			if (m_pGuild->get_upgrade().GetFacilitiesLevel(i) > m_pGuild->get_guild_att().byLevel)
			{
				m_pGuild->get_upgrade().ChangeFacilitesLevel((EFacilitiesType)i, FALSE);
			}
		}
		//m_pGuild->get_upgrade().ChangeFacilitesLevel(EFT_Lobby, FALSE);
		//m_pGuild->get_upgrade().SaveUpgradeInfo2DB(EFT_Lobby);
		//m_pGuild->get_upgrade().ChangeFacilitesLevel(EFT_Factory, FALSE);
		//m_pGuild->get_upgrade().SaveUpgradeInfo2DB(EFT_Factory);
		//m_pGuild->get_upgrade().ChangeFacilitesLevel(EFT_Holiness, FALSE);
		//m_pGuild->get_upgrade().SaveUpgradeInfo2DB(EFT_Holiness);

		Map* pNormalMap = g_mapCreator.get_map(m_pGuild->get_tripod_map_id(), INVALID_VALUE);
		if(VALID_POINT(pNormalMap))
		{
			Creature* pCreature = pNormalMap->find_creature(m_pGuild->get_tripod_id());
			if(VALID_POINT(pCreature))
			{
				pCreature->UnsetState(ES_Occupied);
				pCreature->SetGuildID(INVALID_VALUE);
			}
		}

		m_pGuild->set_tripod_id(INVALID_VALUE);
		m_pGuild->set_tripod_map_id(INVALID_VALUE);

		map_instance_guild* p_guild_instance = m_pGuild->get_guild_map();
		if( VALID_POINT(p_guild_instance) )
		{
			p_guild_instance->on_guild_end_war(m_pGuild);
		}
		
		INT32 nProsperity = GuildHelper::getLevelUpNeedFund(m_pGuild->get_guild_att().byLevel);
		m_pGuild->increase_prosperity(INVALID_VALUE, nProsperity, elcid_guild_upgrade);
	}

	//m_pGuild->decrease_prosperity(INVALID_VALUE, nDayDecProsperity, elcid_guild_upgrade);
}

//-----------------------------------------------------------------------------
// �����ʩ����
//-----------------------------------------------------------------------------
DWORD GuildUpgrade::HandInItems( Role* pRole, EFacilitiesType eType )
{
	// �ϲ���Ҫ��֤�����ĺϷ���
	ASSERT(VALID_POINT(pRole));

	if (!m_bInit)
		return INVALID_VALUE;

	if (eType <= EFT_Null || eType >= EFT_End)
	{
		return INVALID_VALUE;
	}
	//if(eType != EFT_Lobby)
	//	return INVALID_VALUE;

	tagGuildMember* pMember = m_pGuild->get_member(pRole->GetID());
	if (!VALID_POINT(pMember))
	{
		return E_Guild_MemberNotIn;
	}

	// �жϸ���ҵĵ�λ
	if (!m_pGuild->get_guild_power(pMember->eGuildPos).bUpgrade)
	{
		return E_Guild_Power_NotEnough;
	}

	if(m_pGuild->get_guild_war().get_guild_war_state() != EGWS_NULL /*&& m_pGuild->get_guild_war().get_guild_war_state() != EGWS_Avoidance*/)
		return E_Guild_Waring_NotCan_UpLevel;

	if(m_Facilities[eType].IsLevelUp())
		return E_Guild_Grade_UpLevel;

	if(m_Facilities[eType].IsDestory())
		return E_Guild_Grade_Dead;

	// �ж���ʩ�Ƿ�ﵽ�ȼ�����
	//if(eType == EFT_Lobby)
	//{
		if (m_Facilities[eType].byLevel >= m_pGuild->get_guild_att().byLevel)
		{
			return E_GuildUpgrade_Level_Limit;
		}
	//}
	//else
	//{
	//	if (m_Facilities[eType].byLevel >= m_Facilities[eType].byMaxLevel)
	//	{
	//		return E_GuildUpgrade_Level_Limit;
	//	}
	//}
	
	const s_guild& sGuildAtt = m_pGuild->get_guild_att();
	/*if (sGuildAtt.nFund < m_Facilities[eType].nNeedFund)
	{
		return E_Guild_Fund_NotEnough;
	}*/

	//INT32 nDayDecProsperity = 0;
	//for(INT i = EFT_Lobby; i < EFT_End; i++)
	//{
	//	if(i == EFT_Lobby || i == EFT_Factory || i == EFT_Holiness)
	//	{
	//		if(m_Facilities[i].nDayDecProsperity > 0)
	//		{
	//			nDayDecProsperity += m_Facilities[i].nDayDecProsperity;
	//		}
	//	}
	//}

	//if(sGuildAtt.nProsperity < nDayDecProsperity * 10)
	//	return E_Guild_Prosperity_NotEnouth;
	
	// ���ϲ���
	if(sGuildAtt.nMaterial < m_Facilities[eType].nMaterial)
	{
		return E_Guild_Prosperity_NotEnouth;
	}
	

	// �۳������ʽ���ʲ�
	//m_pGuild->decrease_guild_fund(pRole->GetID(), m_Facilities[eType].nNeedFund, elcid_guild_upgrade);
	m_pGuild->decrease_guild_material(pRole->GetID(), m_Facilities[eType].nMaterial, elcid_guild_upgrade);
	
	
	//m_Facilities[eType].dwBeginUpTime = g_world.GetWorldTime();
	//m_Facilities[eType].bUpLevel = TRUE;
	//m_Facilities[EFT_Factory].dwBeginUpTime = g_world.GetWorldTime();
	//m_Facilities[EFT_Factory].bUpLevel = TRUE;
	//m_Facilities[EFT_Holiness].dwBeginUpTime = g_world.GetWorldTime();
	//m_Facilities[EFT_Holiness].bUpLevel = TRUE;
	//if(eType == EFT_Lobby)
	//{
	//	m_pGuild->set_guild_state(EGDSS_UpLevel);

		//m_pGuild->get_guild_att().dwUpLevelTime = g_world.GetWorldTime();
	//}

	// Reset��ʩ������Ϣ
	//m_Facilities[eType].ResetItemInfo();
	//m_Facilities[EFT_Factory].ResetItemInfo();
	//m_Facilities[EFT_Holiness].ResetItemInfo();

	//֪ͨ��ḱ������뿪��ͼ
	//if(eType == EFT_Lobby)
	//{
	//	DWORD dwGuildInstanceID = m_pGuild->get_guild_instance_id();
	//	if( VALID_POINT(dwGuildInstanceID) )
	//	{
	//		map_instance_guild* pGuildInstance = m_pGuild->get_guild_map();
	//		if( VALID_POINT(pGuildInstance) )
	//		{
	//			pGuildInstance->on_guild_level_up(m_pGuild);
	//		}
	//	}
	//}

	ChangeFacilitesLevel(eType);
	// ���浽���ݿ�
	SaveUpgradeInfo2DB(eType);

	return 0;
}

//-----------------------------------------------------------------------------
// ���浽���ݿ�
//-----------------------------------------------------------------------------
VOID GuildUpgrade::SaveUpgradeInfo2DB( EFacilitiesType eType )
{
	// ������ͺϷ���
	if (eType >= EFT_End || eType <= EFT_Null)
	{
		return;
	}

	// ���ɴ�������
	NET_DB2C_facilities_upgrade send;
	send.s_facilities_info_.dw_guild_id	= m_pGuild->get_guild_att().dwID;
	send.s_facilities_info_			= m_Facilities[eType];

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ������ʩ��Ϣ
//-----------------------------------------------------------------------------
BOOL GuildUpgrade::LoadFacilitiesInfo( const s_facilities_load* pInfo, INT n_num )
{
	if (!VALID_POINT(pInfo) || n_num != MAX_GUILD_FACILITIES_TYPE)
	{
		return FALSE;
	}

	for (int m=0; m<n_num; m++)
	{
		// ������ͺϷ���
		EFacilitiesType eType = pInfo[m].e_type;
		if (eType >= EFT_End || eType <= EFT_Null)
		{
			return FALSE;
		}

		// �������ݿⱣ������
		m_Facilities[eType].eType		= (EFacilitiesType)eType;
		m_Facilities[eType].byLevel		= pInfo[m].by_level;
		m_Facilities[eType].byMaxLevel	= pInfo[m].by_max_level;
		m_Facilities[eType].dwBeginUpTime = pInfo[m].dw_begin_up_time;
		m_Facilities[eType].bDead		= pInfo[m].b_dead;
		m_Facilities[eType].dwDeadBeginTime = pInfo[m].dw_dead_begin_time;
		m_Facilities[eType].dwReliveTimeLimit= pInfo[m].dw_relive_time_limit;
		m_Facilities[eType].byUseType = pInfo[m].byUseType;
		m_Facilities[eType].byUseNum = pInfo[m].byUseNum;
		m_Facilities[eType].dwUseTime = pInfo[m].dwUseTime;
		m_Facilities[eType].byStep = pInfo[m].byStep;
		m_Facilities[eType].bUpLevel = pInfo[m].b_up_level;
		//if (eType == EFT_Lobby)
		//{
		//	m_pGuild->reinit_guild_upgrade(pInfo[m].by_level);
		//}
		m_Facilities[eType].nProgress	= pInfo[m].n16_progress;
		for (int n=0; n<MAX_UPGRADE_NEED_ITEM_TYPE; n++)
		{
			m_Facilities[eType].dwItemID[n]	= pInfo[m].dw_item_type_id[n];
			m_Facilities[eType].nNeedNum[n]	= pInfo[m].n_item_need[n];
		}

		// ������ʩ��������
		if (!AttRes::GetInstance()->GetGuildUpgradeBaseInfo(eType, pInfo[m].by_level, m_Facilities[eType]))
		{
			m_Facilities[eType].ResetItemInfo();
		}	

		if(eType == EFT_Lobby)
		{
			INT32 n32_DecProsperity =  (m_Facilities[eType].byLevel)*MGuildMaxMember(m_Facilities[eType].byLevel)*MGuildMaxMember(m_Facilities[eType].byLevel);/*pow((double)((m_Facilities[eType].byLevel)*MGuildMaxMember(m_Facilities[eType].byLevel)), 2)*/;
			n32_DecProsperity /= 10;
			m_Facilities[eType].nDayDecProsperity = n32_DecProsperity;
		}
	}

	// ��ʼ�����
	m_bInit = TRUE;

	return TRUE;
}

//-----------------------------------------------------------------------------
// ������ʩ
//-----------------------------------------------------------------------------
VOID GuildUpgrade::CreateFacilities()
{
	for (int n=EFT_Lobby; n<EFT_End; n++)
	{
		// ����World������ʩ
		//if (n == EFT_Lobby)
		//{
		//	m_Facilities[n].byLevel	= 0;
			//m_pGuild->reinit_guild_upgrade(0);

			//m_Facilities[n].dw_begin_up_time = g_world.GetWorldTime();
			//m_Facilities[n].bUpLevel = TRUE;
			
			//m_pGuild->set_guild_state(EGDSS_UpLevel);

		//}
		//else
		//{
			/*if(n != EFT_Holiness)
			{*/
				m_Facilities[n].eType = (EFacilitiesType)n;
				m_Facilities[n].byLevel = 0;
				m_Facilities[n].byMaxLevel = MGuildMaxGradeLevel(1);
			/*}
			else
			{
				m_Facilities[n].byLevel = 0;
				m_Facilities[n].byMaxLevel = 0;
			}*/
		//}
		m_Facilities[n].nProgress	= 0;
		AttRes::GetInstance()->GetGuildUpgradeItemInfo((BYTE)n, m_Facilities[n].byLevel, m_Facilities[n]);

		// ֪ͨ���ݿ�
		NET_DB2C_create_facilities send;
		send.s_facilities_info_.dw_guild_id	= m_pGuild->get_guild_att().dwID;
		send.s_facilities_info_			= m_Facilities[n];

		g_dbSession.Send(&send, send.dw_size);
	}

	// ��ʼ�����
	m_bInit = TRUE;
}

//-----------------------------------------------------------------------------
// ������ݿ�������ʩ��Ϣ
//-----------------------------------------------------------------------------
VOID GuildUpgrade::RemoveAllFacilitiesInfo()
{
	if (!m_bInit)
	{
		return;
	}

	NET_DB2C_remove_all_facilities send;
	send.dw_guild_id = m_pGuild->get_guild_att().dwID;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ������ʩ�ĵȼ�
//-----------------------------------------------------------------------------
VOID GuildUpgrade::ChangeFacilitiesLevel(EFacilitiesType eType, BYTE byValue)
{
	m_Facilities[eType].byLevel = byValue;
	if (m_Facilities[eType].byLevel > MAX_GUILD_LEVEL)
		m_Facilities[eType].byLevel = MAX_GUILD_LEVEL;
	if (m_Facilities[eType].byLevel < 0)
		m_Facilities[eType].byLevel = 0;

	//if (eType == EFT_Lobby)
	//{
	//	m_pGuild->reinit_guild_upgrade(m_Facilities[EFT_Lobby].byLevel);
	//}

	// Reset��ʩ������Ϣ
	m_Facilities[eType].ResetItemInfo();

	// �����ȡ������������Ʒ
	AttRes::GetInstance()->GetGuildUpgradeItemInfo(eType, m_Facilities[eType].byLevel, m_Facilities[eType]);

	// ���浽���ݿ�
	SaveUpgradeInfo2DB(eType);
}

// ���������ʩʹ�ô���
VOID GuildUpgrade::ChangeFacilitiesUseNum(EFacilitiesType eType, BYTE byValue)
{
	m_Facilities[eType].byUseNum = byValue;

	SaveUpgradeInfo2DB(eType);
}

//-----------------------------------------------------------------------------
// ������ʩ�����ȼ�
//-----------------------------------------------------------------------------
VOID GuildUpgrade::ReSetFacilitiesMaxLevel()
{
	for(INT i = EFT_Lobby; i < EFT_End; i++)
	{
		m_Facilities[i].byMaxLevel = MGuildMaxGradeLevel(m_pGuild->get_guild_att().byLevel);

		// ���浽���ݿ�
		SaveUpgradeInfo2DB((EFacilitiesType)i);
	}
	
}

// ��ȡ��ʩ�ȼ�
INT GuildUpgrade::GetFacilitiesLevel(BYTE Type)
{
	if(Type < EFT_Lobby || Type > EFT_Holiness)
		return 0;

	return m_Facilities[Type].byLevel;
}

//-----------------------------------------------------------------------------
// ������ʩ���
//-----------------------------------------------------------------------------
VOID GuildUpgrade::SetFacilitiesID(BYTE Type, DWORD dwID, INT nIndex)
{
	if(Type < EFT_Lobby || Type > EFT_Holiness)
		return;
	
	if (nIndex < 0 || nIndex >= MAX_GUILD_UPGRADE_NUM)
		return;

	m_Facilities[Type].dwFacilitiesID[nIndex] = dwID;
}

//-----------------------------------------------------------------------------
// ������ʩ���ݻ�
//-----------------------------------------------------------------------------
VOID GuildUpgrade::FacilitesDestory(EFacilitiesType eType)
{
	if(eType < EFT_Lobby || eType > EFT_Holiness)
		return;

	
	m_Facilities[eType].bDead = TRUE;

	for (int i = 0; i < MAX_GUILD_UPGRADE_NUM; i++)
	{
		SetFacilitiesID(eType, INVALID_VALUE, i);
	}
	

	if(eType != EFT_Lobby)
	{
		ChangeFacilitesLevel((EFacilitiesType)eType, FALSE);

		SaveUpgradeInfo2DB(eType);

	}
}

//-----------------------------------------------------------------------------
// ������ʩ�޸�ʱ��
//-----------------------------------------------------------------------------
VOID GuildUpgrade::SetFacilitesReliveTime(BOOL bWin)
{
	for(INT i = EFT_Lobby; i < EFT_End; i++)
	{
		if(!m_Facilities[i].bDead)
			continue;

		m_Facilities[i].dwDeadBeginTime = g_world.GetWorldTime();
		if(i == EFT_Holiness)
		{
			m_Facilities[i].dwReliveTimeLimit = 2 * 3600 * m_pGuild->get_guild_att().byLevel;
		}
		else
		{
			m_Facilities[i].dwReliveTimeLimit = 30 * 60 * m_pGuild->get_guild_att().byLevel;
		}

		if(bWin)
			m_Facilities[i].dwReliveTimeLimit = m_Facilities[i].dwReliveTimeLimit / 2;

		SaveUpgradeInfo2DB((EFacilitiesType)i);
	}
}

//-----------------------------------------------------------------------------
// ������ʩ����
//-----------------------------------------------------------------------------
VOID GuildUpgrade::FacilitesRelive(EFacilitiesType eType)
{
	if(eType < EFT_Lobby || eType > EFT_Holiness)
		return;

	m_Facilities[eType].bDead = FALSE;
	m_Facilities[eType].dwDeadBeginTime = 0;
	m_Facilities[eType].dwReliveTimeLimit = 0;

	SaveUpgradeInfo2DB(eType);
}

//-----------------------------------------------------------------------------
// ��ʼ�������
//-----------------------------------------------------------------------------
DWORD GuildUpgrade::StartGuildPractice(Role* pRole)
{
	DWORD dwError = E_Success;

	tagGuildMember* pGuildMember = m_pGuild->get_member(pRole->GetID());
	if(!VALID_POINT(pGuildMember))
		return INVALID_VALUE;
	
	if(!m_pGuild->get_guild_power(pGuildMember->eGuildPos).bLeague)
		return E_Guild_Power_NotEnough;

	// ��������Ѿ�����
	if(m_Facilities[EFT_Lobby].byUseType == 1)
		return E_Guild_Practice_Already_Start;

	// ���������������
	if(m_Facilities[EFT_Lobby].byUseNum <= 0)
		return E_Guild_Practiec_Num_Limit;

	m_Facilities[EFT_Lobby].byUseType = 1;
	m_Facilities[EFT_Lobby].byUseNum -= 1;
	m_Facilities[EFT_Lobby].dwUseTime = GetCurrentDWORDTime();

	SaveUpgradeInfo2DB(EFT_Lobby);

	NET_SIS_Guild_Practice_Broad send;
	send.b_Start = TRUE;
	m_pGuild->send_guild_message(&send, send.dw_size);

	Map* pNormalMap = g_mapCreator.get_map(m_pGuild->get_tripod_map_id(), INVALID_VALUE);
	if(VALID_POINT(pNormalMap))
	{
		Creature* pCreature = pNormalMap->find_creature(m_pGuild->get_tripod_id());
		if(VALID_POINT(pCreature))
			pCreature->SetState(ES_Occupied);
	}

	DWORD dwGuildInstanceID = m_pGuild->get_guild_instance_id();
	if( VALID_POINT(dwGuildInstanceID) )
	{
		map_instance_guild* pGuildInstance = m_pGuild->get_guild_map();
		if( VALID_POINT(pGuildInstance) )
		{
			Creature* pTripod = pGuildInstance->find_creature(m_pGuild->get_upgrade().GetFacilitiesID(EFT_Lobby));
			if(VALID_POINT(pTripod))
				pTripod->SetState(ES_Occupied);
		}
	}

	return dwError;
}

// ˢ�°��û����������
VOID GuildUpgrade::ResetGuildPracitceUseNum()
{
	m_Facilities[EFT_Lobby].byUseNum = 1;
	SaveUpgradeInfo2DB(EFT_Lobby);
}

