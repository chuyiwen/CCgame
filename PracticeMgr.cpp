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
*	@file		role_ride
*	@author		lc
*	@date		2010/12/2	initial
*	@version	0.0.1.0
*	@brief		武器修炼
*/

#include "StdAfx.h"
#include "PracticeMgr.h"
#include "role.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../common/ServerDefine/log_server_define.h"
#include "hearSay_helper.h"
#include "guild_manager.h"
#include "guild.h"
#include "map_creator.h"

PracticeMgr::PracticeMgr(Role* pRole):
m_pOwner(pRole),
m_n64EquipID(INVALID_VALUE),
m_bPracticing(FALSE),
m_nTickCountDown(PRICTICE_TICK)
{
	m_nLeavePracticeTime = 0;
	m_nLeavePracitceMul = 1;
	m_nLeavePraciteceLove = 0;
	m_nLeaveCountDown = 0;
	m_nLeaveLoveCountDown = 0;
	dw_LoveTotal = 0;
}

PracticeMgr::~PracticeMgr()
{

}
//是否可以修炼
BOOL PracticeMgr::IsInCanPracticeState()
{
	ASSERT(VALID_POINT(m_pOwner));

	BOOL bCannotPractice = m_pOwner->IsInState(ES_Dead) || 
		m_pOwner->IsInState(ES_Dizzy) || 
		m_pOwner->IsInState(ES_Tie) ||
		m_pOwner->IsInState(ES_Spor) ||
		m_pOwner->IsInRoleStateAny(ERS_Mount | ERS_Mount2 | ERS_Combat | ERS_Hang | ERS_Stall | ERS_Carry | ERS_KongFu | ERS_Comprehend | ERS_Dancing) || 
		m_pOwner->GetMoveData().GetCurMoveState() != EMS_Stand;
	return !bCannotPractice;
}

//开始修炼
VOID PracticeMgr::BeginPractice()
{
	ASSERT(VALID_POINT(m_pOwner));
	m_bPracticing = TRUE;
	m_pOwner->SetRoleState(ERS_Prictice);
}

//结束修炼
VOID PracticeMgr::EndPractice(DWORD dw_error_code)
{
	ASSERT(VALID_POINT(m_pOwner));
	m_nTickCountDown = PRICTICE_TICK;
	m_bPracticing = FALSE;
	m_pOwner->UnsetRoleState(ERS_Prictice);
	//m_itemList.clear();
	m_item_serial.clear();

	NET_SIS_parctice_end send;
	send.dw_error_code = dw_error_code;
	m_pOwner->SendMessage(&send, send.dw_size);

}

VOID PracticeMgr::Update()
{
	ASSERT(VALID_POINT(m_pOwner));

	if (!IsInCanPracticeState())
	{
		// 添加离线修炼逻辑
		if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
		{
			g_mapCreator.role_logout_leave_role(m_pOwner->GetID());
		}
		EndPractice(E_Prictice_Not_State);
		return;
	}

	if (!m_bPracticing) return;
	if (!VALID_POINT(m_pOwner)) return;

	//tagEquip* pEquip = (tagEquip*)m_pOwner->GetItem(m_n64EquipID);
	//if (!VALID_POINT(pEquip)) return;
	//if (!M_is_weapon(pEquip)) return;
	
	//义气值不够了
	if (m_pOwner->GetAttValue(ERA_Brotherhood) <= 0)
	{
		// 添加离线修炼逻辑
		if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
		{
			if(VALID_POINT(m_pOwner->GetScript()))
			{
				m_pOwner->GetScript()->OnLeavePractice(m_pOwner, m_nLeaveCountDown, dw_LoveTotal, 1);
			}
			g_mapCreator.role_logout_leave_role(m_pOwner->GetID());
		}
		EndPractice(E_Prictice_Brotherhood_Not);
		return;
	}

	
	//武器等级满了
	//if (pEquip->equipSpec.nLevel >= pEquip->pEquipProto->byMaxLevel ||
	//	pEquip->equipSpec.nLevel >= m_pOwner->get_level())
	//{
	//	// 添加离线修炼逻辑
	//	if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
	//	{
	//		if(VALID_POINT(m_pOwner->GetScript()))
	//		{
	//			m_pOwner->GetScript()->OnLeavePractice(m_pOwner, m_nLeaveCountDown, dw_LoveTotal, 3);
	//		}

	//		g_mapCreator.role_logout_leave_role(m_pOwner->GetID());
	//	}
	//	EndPractice(E_Prictice_Level_Max);
	//	return;
	//}

	// 离线修炼爱心值不足
	/*if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()) && m_pOwner->GetAttValue(ERA_Love) < m_nLeavePraciteceLove)
	{
		if(VALID_POINT(m_pOwner->GetScript()))
		{
			m_pOwner->GetScript()->OnLeavePractice(m_pOwner, m_nLeaveCountDown, dw_LoveTotal, 2);
		}
		g_mapCreator.role_logout_leave_role(m_pOwner->GetID());
		return;
	}*/

	// 超过修炼时间
	if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
	{
		if(m_nLeaveCountDown < m_nLeavePracticeTime)
		{
			m_nLeaveCountDown += TICK_TIME;
		}
		else
		{
			if(VALID_POINT(m_pOwner->GetScript()))
			{
				m_pOwner->GetScript()->OnLeavePractice(m_pOwner, m_nLeaveCountDown, dw_LoveTotal, 5);
			}
			g_mapCreator.role_logout_leave_role(m_pOwner->GetID());
			return;
		}
	}

	// 离线修炼扣费
	if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
	{
		if(m_nLeaveLoveCountDown >= 60000)
		{
			if(m_pOwner->GetAttValue(ERA_Love) < m_nLeavePraciteceLove)
			{
				if(VALID_POINT(m_pOwner->GetScript()))
				{
					m_pOwner->GetScript()->OnLeavePractice(m_pOwner, m_nLeaveCountDown, dw_LoveTotal, 2);
				}
				g_mapCreator.role_logout_leave_role(m_pOwner->GetID());
				return;
			}

			m_nLeaveLoveCountDown = 0;
			m_pOwner->ModAttValue(ERA_Love, -m_nLeavePraciteceLove, FALSE);
			m_pOwner->SaveLove2DB();
			SendLeavePraciticeLog();
			dw_LoveTotal += m_nLeavePraciteceLove;
		}
		else
		{
			m_nLeaveLoveCountDown += TICK_TIME;
		}
	}
	
	if( --m_nTickCountDown <= 0 )	
	{
		m_nTickCountDown = PRICTICE_TICK;

		INT nPv = PRICTICE_VALUE;
		//义气值不够一次
		if (m_pOwner->GetAttValue(ERA_Brotherhood) < nPv)
		{
			nPv = m_pOwner->GetAttValue(ERA_Brotherhood);
			
			//减少人物义气值
			m_pOwner->ChangeBrotherhood(-nPv);
			m_pOwner->ChangeWuhuen(nPv);
			////增加武器经验
			//if( pEquip->ExpChange(nPv) )
			//{
			//	// 重算攻击
			//	m_pOwner->ResetWeaponDmg(*pEquip, TRUE);
			//	m_pOwner->RecalAtt();
			//	if( VALID_POINT( m_pOwner->GetScript( ) ) )
			//		m_pOwner->GetScript( )->OnWeaponLevelUp( m_pOwner, pEquip->equipSpec.nLevel );

			//	m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ete_xiulian, pEquip->equipSpec.nLevel);
			//}
			//m_pOwner->GetItemMgr().UpdateEquipSpec(*pEquip);
			m_pOwner->SaveBrotherhood2DB();
			m_pOwner->SaveWuhuen2DB();
			/*if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
			{
				m_pOwner->ModAttValue(ERA_Love, -nPv, FALSE);
				m_pOwner->SaveLove2DB();
			}*/
			return;

		}

		//判断是否有提速道具和增加值的道具
		INT nAddspeed = 0;
		BOOL bUseItem = TRUE;
		FLOAT f_Multiple = 1;
		// m_itemList在增速道具用完之后会出现野指针现象，没有维护列表，地址可能会被覆盖 lc 2011-4-13
		/*tagItem* pSpeedItem = NULL;
		package_list<tagItem*>::list_iter it = m_itemList.begin();
		while(m_itemList.find_next(it, pSpeedItem))
		{
			if (VALID_POINT(pSpeedItem))
				break;
		}*/

		if(!m_pOwner->is_leave_pricitice() && VALID_POINT(m_pOwner->GetSession()))
		{
			guild* pGuild = g_guild_manager.get_guild(m_pOwner->GetGuildID());
			if(VALID_POINT(pGuild))
			{
				FLOAT fMul = pGuild->LobbyPractice(m_pOwner);
				if(fMul > 0)
				{
					f_Multiple = fMul;
					bUseItem = FALSE;
				}
			}


			if(bUseItem)
			{
				tagItem* pSpeedItem = NULL;
				INT64	n64_item_serial = 0;
				package_list<INT64>::list_iter iter = m_item_serial.begin();
				while(m_item_serial.find_next(iter, n64_item_serial))
				{
					pSpeedItem = m_pOwner->GetItemMgr().GetBag().GetItem(n64_item_serial);
					if (VALID_POINT(pSpeedItem))
						break;
				}
				if (VALID_POINT(pSpeedItem) && pSpeedItem->pProtoType->eSpecFunc == EIST_PRACTICE && pSpeedItem->pProtoType->nSpecFuncVal1 == 0)
				{
					nAddspeed = pSpeedItem->pProtoType->nSpecFuncVal2;
					m_pOwner->GetItemMgr().ItemUsedFromBag(pSpeedItem->n64_serial, 1, elcid_compose_prictice);
				}
			}
		}
	
		//INT nAddValue = 0;
		//tagItem* pItem1 = m_pOwner->GetItem(m_n64ItemID1);
		//if (VALID_POINT(pItem1) && pItem1->pProtoType->eSpecFunc == EIST_PRACTICE && pItem1->pProtoType->nSpecFuncVal1 == 0)
		//{
		//	nAddspeed = pItem1->pProtoType->nSpecFuncVal2;
		//	m_pOwner->GetItemMgr().ItemUsedFromBag(m_n64ItemID1, 1, elcid_compose_prictice);
		//}

		//tagItem* pItem2 = m_pOwner->GetItem(m_n64ItemID2);
		//if (VALID_POINT(pItem2) && pItem2->pProtoType->eSpecFunc == EIST_PRACTICE && pItem2->pProtoType->nSpecFuncVal1 == 1)
		//{
		//	nAddValue = pItem2->pProtoType->nSpecFuncVal2;
		//	m_pOwner->GetItemMgr().ItemUsedFromBag(m_n64ItemID2, 1, elcid_compose_prictice);
		//}


		//mwh 2011-07-29 服务器修炼加速
		FLOAT fExpDelta = 1.0f;
		float fEarnRate	= m_pOwner->GetEarnRate() / 10000.0f;
		float fLeave = 1.0f;
		if(VALID_POINT(m_pOwner->get_map( ))) fExpDelta =  m_pOwner->get_map()->get_practice_rate( );

		if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
		{
			fLeave = m_nLeavePracitceMul;
		}

		INT nChangeValue = ((PRICTICE_VALUE + nAddspeed)*f_Multiple*fExpDelta)*fEarnRate*fLeave;
		//减少人物义气值
		m_pOwner->ChangeBrotherhood(-nChangeValue);
		m_pOwner->ChangeWuhuen(nChangeValue);
		//增加武器经验
		//if( pEquip->ExpChange(((PRICTICE_VALUE + nAddspeed)*f_Multiple * fExpDelta*fEarnRate*fLeave/*+ nAddValue*/)) )
		//{
		//	// 重算攻击
		//	m_pOwner->ResetWeaponDmg(*pEquip, TRUE);
		//	m_pOwner->RecalAtt();
		//	if( VALID_POINT( m_pOwner->GetScript( ) ) )
		//		m_pOwner->GetScript( )->OnWeaponLevelUp( m_pOwner, pEquip->equipSpec.nLevel );

		//	if( pEquip->equipSpec.nLevel == 60 ||
		//		 pEquip->equipSpec.nLevel == 70)
		//	{
		//		HearSayHelper::SendMessage(EHST_EQUIPPRACTICE,
		//			m_pOwner->GetID(), pEquip->dw_data_id, pEquip->equipSpec.nLevel, INVALID_VALUE, INVALID_VALUE, pEquip);
		//	}

		//	m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ete_xiulian, pEquip->equipSpec.nLevel);
		//}
		//m_pOwner->GetItemMgr().UpdateEquipSpec(*pEquip);
		m_pOwner->SaveBrotherhood2DB();
		m_pOwner->SaveWuhuen2DB();
		/*if(m_pOwner->is_leave_pricitice() && !VALID_POINT(m_pOwner->GetSession()))
		{
			m_pOwner->ModAttValue(ERA_Love, -(((PRICTICE_VALUE + nAddspeed)*f_Multiple*fExpDelta)*fEarnRate), FALSE);
			m_pOwner->SaveLove2DB();
		}*/
	}
	
}

VOID PracticeMgr::SetPracticeData(INT64 n64EquipId, package_list<tagItem*>& itemList)
{
	m_n64EquipID = n64EquipId;
	package_list<tagItem*>::list_iter iter = itemList.begin();
	tagItem* pItem = NULL;
	while(itemList.find_next(iter, pItem))
	{
		if(!VALID_POINT(pItem))
			continue;
		m_item_serial.push_back(pItem->n64_serial);
	}
	//m_itemList = itemList;
}

// 设置离线修炼时间
VOID PracticeMgr::SetLeavePracticeTime(BYTE byTimeType)
{
	switch(byTimeType)
	{
	case 1:
		m_nLeavePracticeTime = 3600 * 1000;
		break;
	case 2:
		m_nLeavePracticeTime = 3 * 3600 * 1000;
		break;
	case 3:
		m_nLeavePracticeTime = 8 * 3600 * 1000;
		break;
	case 4:
		m_nLeavePracticeTime = 12 * 3600 * 1000;
		break;
	default:
		m_nLeavePracticeTime = 3600 * 1000;
		break;
	}
}

// 设置离线修炼倍数及消耗
VOID PracticeMgr::SetLeavePraciticeMul(BYTE byMulType)
{
	switch(byMulType)
	{
	case 1:
		m_nLeavePracitceMul = 1;
		m_nLeavePraciteceLove = 50;
		break;
	case 2:
		m_nLeavePracitceMul = 2;
		m_nLeavePraciteceLove = 150;
		break;
	case 3:
		m_nLeavePracitceMul = 4;
		m_nLeavePraciteceLove = 400;
		break;
	case 4:
		m_nLeavePracitceMul = 6;
		m_nLeavePraciteceLove = 800;
		break;
	case 5:
		m_nLeavePracitceMul = 8;
		m_nLeavePraciteceLove = 1500;
		break;
	default:
		m_nLeavePracitceMul = 1;
		m_nLeavePraciteceLove = 50;
		break;
	}
}

// 通过离线修炼倍数类型获取实际倍数
INT	PracticeMgr::GetLeavePraciticeLoveFromType(BYTE byMulType)
{
	switch(byMulType)
	{
	case 1:
		return 100;
	case 2:
		return 300;
	case 3:
		return 900;
	case 4:
		return 1800;
	case 5:
		return 6000;
	default:
		return 100;
	}
}

VOID PracticeMgr::SendLeavePraciticeLog()
{
	NET_DB2C_log_leave_practice send;
	send.s_log_leave_pracitce.dw_role_id = m_pOwner->GetID();
	send.s_log_leave_pracitce.dw_cur_love = m_pOwner->GetAttValue(ERA_Love);
	send.s_log_leave_pracitce.dw_drop_love = m_nLeavePraciteceLove;

	g_dbSession.Send(&send, send.dw_size);
}

VOID PracticeMgr::SetAddLoveTotal(DWORD	dw_love)
{
	dw_LoveTotal += dw_love;
}


