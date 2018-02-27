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
*	@file		combat_handler.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		ս��ϵͳ������
*/

#include "stdafx.h"

#include "../../common/WorldDefine/combat_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/ride_protocol.h"

#include "unit.h"
#include "map.h"
#include "creature.h"
#include "role.h"
#include "combat_handler.h"
#include "script_mgr.h"
#include "title_mgr.h"
#include "creature_ai.h"
#include "map_creator.h"
#include "map_mgr.h"
//-----------------------------------------------------------------------------
// ʹ�ü���
//-----------------------------------------------------------------------------
INT CombatHandler::UseSkill(DWORD dwSkillID, DWORD dwTargetUnitID,DWORD dwSerial, Vector3 vDesPos)
{
	Skill* pSkill = m_pOwner->GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) ) return E_UseSkill_SkillNotExist;

	Unit* pTargetUnit = m_pOwner->get_map()->find_unit(dwTargetUnitID);
	
	if (m_pOwner->IsRole())
	{
		((Role*)m_pOwner)->SetCurSkillTarget(dwTargetUnitID);
	}
	

	// ����ܷ�Ը�Ŀ�귢������
	INT nRet = CanCastSkill(pSkill, dwTargetUnitID , vDesPos);
	if( E_Success != nRet )	
		return nRet;
	
	// ��ǽ�༼��
	if (pSkill->GetProto()->bPoint)
	{
		Vector3 vDPos = vDesPos;
		if (VALID_POINT(pTargetUnit))
		{
			vDPos = pTargetUnit->GetCurPos();
		}

		int nTitleX = vDPos.x / TILE_SCALE;
		int nTitleY = vDPos.z / TILE_SCALE;
		
		// �õ�������
		vDPos.x = nTitleX * TILE_SCALE;
		vDPos.z = nTitleY * TILE_SCALE;

		int nTitleIndex = nTitleX + nTitleY * m_pOwner->get_map()->get_map_info()->n_width;
		std::map<DWORD, tagPilotUnit*>::iterator it = m_listPilotUnit.find(nTitleIndex);
		if ( it != m_listPilotUnit.end())
		{
			tagPilotUnit* pPUnit = it->second;
			pPUnit->dwOverTime = pSkill->GetPrepareTime() / 3;
		}
		else
		{
			Creature *pCreature = m_pOwner->get_map()->create_creature(pSkill->GetProto()->dwTargetTriggerID, vDPos, Vector3(0, 0, 0));
			
			if (!VALID_POINT(pCreature))
				return INVALID_VALUE;

			tagPilotUnit* pPUnit = new tagPilotUnit;
			pPUnit->dwMapID = m_pOwner->GetMapID();
			pPUnit->dwInstanceID = m_pOwner->get_map()->get_instance_id();
			pPUnit->dwSkillID = pSkill->GetID();
			pPUnit->dwCretureID = pCreature->GetID();
			pPUnit->dwOverTime = pSkill->GetPrepareTime() / 3;
			pPUnit->fOPRadius = pSkill->GetOPRadius();
			m_listPilotUnit[nTitleIndex] = pPUnit;
		}

		return nRet;
	}


	//// �û������������ƣ��������Ӧ����ո�¶�
	if( m_pOwner->IsRole() )
		((Role*)m_pOwner)->GetItemMgr().ProcEquipNewness();
	
	// �����;öȴ���
	//if(m_pOwner->IsRole())
	//{
	//	((Role*)m_pOwner)->GetItemMgr().ProcArmorNewness();
	//}
	

	// ���ü����Ƿ��ܹ��ƶ�ʩ��
	
	if (pSkill->GetDmgTimes() == 0)
	{
		// û���˺��ļ���ֱ�Ӵ��
		m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);					// ���ʹ�ü��ܴ�ϵ�buff
	}
	
	m_vPersistSkillPos	=	vDesPos;

	if( pSkill->GetProto()->eOPType == ESOPT_Persist)  //����ǳ�������
	{
		m_pOwner->GetMoveData().StopMoveForce(); //ֹͣ
		m_pOwner->GetMoveData().SetFaceTo(vDesPos - m_pOwner->GetCurPos()); //����Ŀ��
		//m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);					// ���ʹ�ü��ܴ�ϵ�buff

		m_dwSkillID			=	dwSkillID;
		m_dwTargetUnitID	=	dwTargetUnitID;
		m_dwSkillSerial		=	dwSerial;
		

		// ����ü�����Ҫ���֣����������ֵ���ʱ��������뼼�����ý׶�
		m_nPersistSkillTimeCD = (INT)(pSkill->GetPilotTime());

		if( m_nPersistSkillTimeCD > 0 ) 
		{
			m_eUseSkillState = EUSS_Piloting;
			//m_bSkillPiloting	=	TRUE;
			//m_bSkillPreparing	=	FALSE;
			//m_bSkillOperating	=	FALSE;
			m_pOwner->StartSkillCoolDown(pSkill);
		}
		else
		{
			return E_UseSkill_SkillNotExist;
		}
		//else
		//{
		//	// ���Է��������ü�����ȴ
		//	m_pOwner->StartSkillCoolDown(pSkill);

		//	m_bSkillPreparing	=	FALSE;
		//	m_bSkillOperating	=	TRUE;
		//	m_nSkillOperateTime	=	0;
		//	m_nSkillCurDmgIndex	=	0;

		//	// ����Ŀ��
		//	CalSkillTargetList(vDesPos);
		//}
	}
	else
	{
		if( pSkill->IsMoveCancel() )
		{
			m_pOwner->GetMoveData().StopMoveForce();
		}

		// ���Ŀ������Ҳ����Լ�����ı�����
		if( VALID_POINT(pTargetUnit) && m_pOwner->GetID() != pTargetUnit->GetID() )
		{
			m_pOwner->GetMoveData().SetFaceTo(pTargetUnit->GetCurPos() - m_pOwner->GetCurPos());
		}


		// ���ʹ�ü��ܴ�ϵ�buff
		//m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);

		// ���ò�����׼������
		m_dwSkillID			=	dwSkillID;
		m_dwTargetUnitID	=	dwTargetUnitID;
		m_dwSkillSerial		=	dwSerial;

		// ����ü�����Ҫ���֣����������ֵ���ʱ��������뼼�����ý׶�
		m_nSkillPrepareCountDown = (INT)(pSkill->GetPrepareTime() * m_fSkillPrepareModPct);

		if(m_pOwner->IsRole())
		{
			if(pSkill->GetProto()->nType2 != ESSTE_Default)
			{
				m_dwPublicCoolDown = 0;
				if(pSkill->GetProto()->bPublicCD)
				{
					EClassType e = ((Role*) m_pOwner)->GetClass();
					m_dwPublicCoolDown = AttRes::GetInstance()->GetVariableLen().n_public_cd_time[e];
					NET_SIS_start_public_cd send;
					send.dwSkillID = m_dwSkillID;
					send.dwCoolTime = m_dwPublicCoolDown;
					((Role*)m_pOwner)->SendMessage(&send, send.dw_size);
				}
			}
		}
		
		if( m_nSkillPrepareCountDown > 0 )
		{
			m_eUseSkillState	=	EUSS_Preparing;
			const SkillScript* pScript = pSkill->GetSkillScript();
			if (VALID_POINT(pScript))
			{
				pScript->PreparingSkill(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), dwTargetUnitID);
			}

			//m_bSkillPreparing	=	TRUE;
			//m_bSkillOperating	=	FALSE;
		}
		else
		{
			// ���Է��������ü�����ȴ �����Ƶ�Ч��ʱcd
			//m_pOwner->StartSkillCoolDown(pSkill);

			m_eUseSkillState	=	EUSS_Operating;
			m_bCD				=	TRUE;
			//m_bSkillPreparing	=	FALSE;
			//m_bSkillOperating	=	TRUE;
			m_nSkillOperateTime	=	0;
			m_nSkillCurDmgIndex	=	0;
			m_bTrigger			=	FALSE;
			// ��������
			CalculateCost(pSkill);
			// ����Ŀ��	

			// ��Ⱥ�����������
			if( pSkill->GetOPRadius() <= 0.0f || pSkill->GetDmgTimes() <= 0)
			{
				CalSkillTargetList(vDesPos, pSkill->GetHitNumber());
			}
			
		}
	}

	return nRet;
}

//------------------------------------------------------------------------------------------
// ����ʹ�ü���
//------------------------------------------------------------------------------------------
VOID CombatHandler::EndUseSkill()
{
	m_dwSkillID					=	INVALID_VALUE;
	m_dwTargetUnitID			=	INVALID_VALUE;
	m_dwTargetEffectFlag		=	0;
	m_dwSkillSerial				=	0;
	
	m_eUseSkillState			=	EUSS_NULL;
	//m_bSkillPreparing			=	false;
	//m_bSkillOperating			=	false;

	m_nSkillPrepareCountDown	=	0;
	m_nSkillOperateTime			=	0;
	m_nSkillCurDmgIndex			=	0;
	m_bTrigger					=	FALSE;

	m_nPersistSkillTimeCD		=	0;
	m_nPersistSkillTime			=   0;
	m_nPersistSkillCnt			=	0;
	m_bCD						=	FALSE;
	//m_bSkillPiloting			=	false;
	//m_vPersistSkillPos			=	Vector3(0,0,0);
	memset(&m_vPersistSkillPos,0,sizeof(Vector3));

}
//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
VOID CombatHandler::Update()
{

	m_dwPublicCoolDown -= TICK_TIME;
	if(m_dwPublicCoolDown <= 0)
	{
		m_dwPublicCoolDown = 0;
	}

	m_dwCombatStateCoolDown -= TICK_TIME;
	if(m_dwCombatStateCoolDown <= 0)
	{
		m_dwCombatStateCoolDown = 0;
		if(m_pOwner->IsRole())
		{
			if(((Role*)m_pOwner)->GetEnmityCreatureSize() <= 0)
			{
				if(((Role*)m_pOwner)->IsInRoleState(ERS_Combat))
				{
					((Role*)m_pOwner)->UnsetRoleState(ERS_Combat);
					((Role*)m_pOwner)->OnInterruptBuffEvent(EBIF_LeaveCombat);
					((Role*)m_pOwner)->SetLeaveCombatTime( );
				}
			}
		}
	}
		
	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	tstring szDesc = _T("");


	DWORD dwcurrSkill = m_dwSkillID;
	// �������ʹ�ü���
	if( IsUseSkill() )
	{
		switch (m_eUseSkillState)
		{
		case EUSS_Preparing:
			UpdateSkillPrepare();
			szDesc = _T("CombatHandler::UpdateSkillPrepare() time %d skill %d\r\n");
			break;
		case EUSS_Operating:
			UpdateSkillOperate();
			szDesc = _T("CombatHandler::UpdateSkillOperate() time %d skill %d\r\n");
			break;
		case EUSS_Piloting:
			UpdateSkillPiloting();
			szDesc = _T("CombatHandler::UpdateSkillPiloting() time %d skill %d\r\n");
			break;
		default:
			EndUseSkill();
			break;

		}
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(szDesc.c_str(), dw_new_time - dw_time, dwcurrSkill);
		}
		//if( IsSkillPreparing() )		UpdateSkillPrepare();
		//else if( IsSkillOperating() )	UpdateSkillOperate();
		//else if( IsSkillPiloting())		UpdateSkillPiloting(); //add by guohui
		//else							EndUseSkill();
	}
	
	//���³����ĵ��＼��
	m_dwChixuCoolDown -= 1;
	if (m_dwChixuCoolDown <= 0)
	{
		UpdatePrepareUnit();
		m_dwChixuCoolDown = 3 * TICK_PER_SECOND;
	}
	

	// �������ʹ����Ʒ
	if( IsUseItem() )
	{
		if( IsItemPreparing() )			UpdateItemPrepare();
		else if( IsItemOperating() )	UpdateItemOperate();
		else							EndUseItem();
	}

	if( IsRide())
	{
		if(IsRidePreparing()) UpdateRidePrepare();
		else if(IsRideOperating())UpdateRideOperate();
		else EndRide();
	}
}

//-------------------------------------------------------------------------------
// ʹ����Ʒ
//-------------------------------------------------------------------------------
INT CombatHandler::UseItem(INT64 n64ItemID, DWORD dwTargetUnitID, DWORD dwSerial, DWORD &dw_data_id, bool& bImmediate)
{
	if( INVALID_VALUE == dwTargetUnitID )
		dwTargetUnitID = m_pOwner->GetID();

	// ����ǲ������
	if( !m_pOwner->IsRole() ) return E_UseItem_TargetInvalid;
	Role* pOwnerRole = static_cast<Role*>(m_pOwner);

	// �����Ʒ�Ƿ��ڱ�����
	tagItem* pItem = pOwnerRole->GetItemMgr().GetBagItem(n64ItemID); 
	if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;

	// ����ܷ�ʹ����Ʒ
	INT nRet = E_Success;
	BOOL bIgnore = FALSE;		// �Ƿ�����ܷ�ʹ����Ʒ��ͨ���ж�

	if(VALID_POINT(pItem->pScript) && VALID_POINT(pOwnerRole->get_map()))
	{
		// ���ű�����Ʒʹ������
		nRet = pItem->pScript->can_use_item(pOwnerRole->get_map(), pItem->dw_data_id, m_pOwner->GetID(), dwTargetUnitID, bIgnore, pItem->n64_serial);

		// ���»�ȡ��Ʒָ��
		pItem = pOwnerRole->GetItemMgr().GetBagItem(n64ItemID); 
		if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;
	}

	// ���ʹ����Ʒ��ͨ���ж�
	if(!bIgnore && E_Success == nRet)
		nRet = can_use_item(pItem);

	if( E_Success != nRet ) return nRet;

	// �������Ʒ�����ƶ�ʹ�ã���ͣ��
	if( !pItem->pProtoType->bMoveable )
	{
		m_pOwner->GetMoveData().StopMoveForce();
	}

	// ���ʹ����Ʒ��ϵ�buff
	//m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);

	// ������ͨ��������������Ӧ�Ĳ�����׼������
	m_n64ItemID			=	n64ItemID;
	m_dwItemSerial		=	dwSerial;
	dw_data_id			=	pItem->dw_data_id;
	m_dwTargetUnitIDItem = dwTargetUnitID;

	// ���ʹ����Ʒ��Ҫ���֣����������ֵ���ʱ������������ý׶�
	m_nItemPrepareCountDown = pItem->pProtoType->nPrepareTime;
	if( m_nItemPrepareCountDown > 0 )
	{
		m_bItemPreparing	=	TRUE;
		m_bItemOperating	=	FALSE;
		bImmediate			=	false;
	}
	else
	{
		m_bItemPreparing	=	FALSE;
		m_bItemOperating	=	TRUE;
		bImmediate			=	true;
	}

	return nRet;
}

//-------------------------------------------------------------------------------
// ʹ������
//-------------------------------------------------------------------------------
DWORD CombatHandler::UseRide()
{
	if( !m_pOwner->IsRole() ) return E_Ride_BeginRide_Failed_Not_Exist;

	Role* pOwnerRole = static_cast<Role*>(m_pOwner);
	tagEquip* pRide = pOwnerRole->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(!VALID_POINT(pRide)) return E_Ride_BeginRide_Failed_Not_Exist;

	DWORD tRet = CheckUseRide(pRide); 
	if(tRet != E_Success) return tRet;

	m_pOwner->GetMoveData().StopMoveForce();

	m_nRidePrepareCountDown = pOwnerRole->GetMountSpellTime( );
	m_bRide = TRUE;
	m_bRidePreparing = TRUE;
	m_bRideOperating = FALSE;


	return E_Success;
}
DWORD CombatHandler::CheckUseRide(tagEquip* pRide)
{
	DWORD dwRet = E_Success;

	if(IsUseItem() || IsUseSkill())
		return E_Ride_BeginRide_Failed_StateLimit;

	dwRet = CheckRoleStateLimit(pRide);
	if(dwRet != E_Success) 
		return E_Ride_BeginRide_Failed_StateLimit;

	dwRet = m_pOwner->IsInStateCantMove();
	if(dwRet != E_Success || 
		static_cast<Role*>(m_pOwner)->IsInRoleStateAny(ERS_Fishing | ERS_Carry))
	{
		return E_Ride_BeginRide_Failed_StateLimit;
	}

	BOOL bLimited = ((Role*)m_pOwner)->ride_limit(pRide);
	if(bLimited) return E_Ride_BeginRide_Failed_StateLimit;

	return E_Success;
}
//-----------------------------------------------------------------------------
// ���¼������֣�������ֽ����ˣ����л�������״̬
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateSkillPrepare()
{
	if( !IsUseSkill() ) return;
	if( !IsSkillPreparing() ) return;
	
	// �����ҵ��������
	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		EndUseSkill();
		return;
	}

	// ��ȥTickʱ��
	m_nSkillPrepareCountDown -= TICK_TIME;

	// ����ʱ��������л�������״̬
	if( m_nSkillPrepareCountDown <= 0 )
	{
		// �����Ƶ��������Ч������ȴ
		//m_pOwner->StartSkillCoolDown(pSkill);
		m_eUseSkillState	= EUSS_Operating;
		m_bCD				= TRUE;
		//m_bSkillPreparing = FALSE;
		//m_bSkillOperating = TRUE;
		m_nSkillOperateTime = 0;
		m_nSkillCurDmgIndex = 0;
		m_bTrigger			= FALSE;
		// ��������
		CalculateCost(pSkill);
		// ����Ŀ��
		CalSkillTargetList(Vector3(0,0,0), pSkill->GetHitNumber());
	}
}

//-----------------------------------------------------------------------------
// �������֣�������ֽ����ˣ����л�������״̬
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateItemPrepare()
{
	if( !IsUseItem() ) return;
	if( !IsItemPreparing() ) return;

	// ��ȥTickʱ��
	m_nItemPrepareCountDown -= TICK_TIME;

	// ����ʱ��������л�������״̬
	if( m_nItemPrepareCountDown <= 0 )
	{
		m_bItemPreparing = FALSE;
		m_bItemOperating = TRUE;
	}
}
//-----------------------------------------------------------------------------
// �������֣�������ֽ����ˣ����л�������״̬
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateRidePrepare()
{
	if( !IsRide() ) return;
	if( !IsRidePreparing() ) return;
	
	m_nRidePrepareCountDown -= TICK_TIME;
	if( m_nRidePrepareCountDown <= 0 )
	{
		m_bRidePreparing = FALSE;
		m_bRideOperating = TRUE;
	}
}

//add by guohui 
//���������ͼ��ܵĲ������µļ����˺�����
VOID CombatHandler::UpdateSkillPiloting()
{
	if( !IsUseSkill() ) return;
	if( !IsSkillPiloting()) return;

	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		EndUseSkill();
		return;
	}

	Map* pMap = m_pOwner->get_map();
	if( !VALID_POINT(pMap) ) return;

	m_nPersistSkillTimeCD -= TICK_TIME;

	INT nDmgTimes = pSkill->GetPilotNum();

	// ����˺�����Ϊ0��˵���ü������˺�����ֱ�ӽ��뵽����buff�׶�
	if( nDmgTimes <= 0 )
	{
		// ����buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listTargetID, ETEE_Use);
		// ��������
		//CalculateCost(pSkill);

		// ����
		EndUseSkill();
		return;
	}

	// ��������
	//CalculateCost(pSkill);
	//m_nPersistSkillTime += TICK_TIME;  //��������ʱ���һ��tick
	//static bool flag = true;

	for(; m_nPersistSkillCnt < nDmgTimes; m_nPersistSkillCnt++)  //��������������
	{
		// ��tick��ɲ�����˶���˺����㣬�ȵ��¸�tick
		if(	pSkill->GetPilotTime() -(m_nPersistSkillCnt+1)* (pSkill->GetPilotTime()/nDmgTimes) < m_nPersistSkillTimeCD )
			break;

		CalSkillTargetList(m_vPersistSkillPos, pSkill->GetHitNumber()); //�����˺��б�
		// ʱ�䵽�ˣ���ʼ�����˺�
		package_list<DWORD>::list_iter it = m_listTargetID.begin();
		DWORD dwTargetID = INVALID_VALUE;

		while( m_listTargetID.find_next(it, dwTargetID) )
		{
			// �ҵ����Ŀ��
			Unit* pTarget = pMap->find_unit(dwTargetID);

			if( !VALID_POINT(pTarget) || pTarget == m_pOwner) continue;

			// �����˺�
			CalculateDmg(pSkill, pTarget);
		}

		// ������������Buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Hit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listDodgedTarget, ETEE_Dodged);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listBlockedTarget, ETEE_Blocked);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listCritedTarget, ETEE_Crit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Use);

		//if(m_bDropMP)
		//{
			//CalculateCost(pSkill);  //ֻ�۵�һ����
		//	m_bDropMP = false;
		//}
	}

	if(m_nPersistSkillCnt >= nDmgTimes )
	{
		EndUseSkill();
		//m_bDropMP = true;
	}
}
//end
//-------------------------------------------------------------------------------
// ���¼��ܲ�����������˼����˺���ʱ��������˺�������˺��������ˣ������buff
//-------------------------------------------------------------------------------
VOID CombatHandler::UpdateSkillOperate()
{
	if( !IsUseSkill() ) return;
	if( !IsSkillOperating() ) return;

	// �����ҵ��������
	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		EndUseSkill();
		return;
	}

	Map* pMap = m_pOwner->get_map();
	if( !VALID_POINT(pMap) ) return;

	// �õ��������˺�����
	INT nDmgTimes = pSkill->GetDmgTimes();

	if (m_pOwner->IsRole())
	{
		((Role*)m_pOwner)->GetAchievementMgr().UpdateAchievementCriteria(ete_use_skill, pSkill->GetTypeID()/100, 1);
	}
	

	// ��ֹ��ε���
	if (m_bCD)
	{
		// ����cd
		m_pOwner->StartSkillCoolDown(pSkill);
		m_bCD = FALSE;
	}


	// ����˺�����Ϊ0��˵���ü������˺�����ֱ�ӽ��뵽����buff�׶�
	if( nDmgTimes <= 0 )
	{
		// ����buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listTargetID, ETEE_Use);
		// ��������
		//CalculateCost(pSkill);
		// ����
		EndUseSkill();
		return;
	}
	
	bool bCaldmg = FALSE;
	// �˺�������Ϊ0�����⵱ǰʱ�䵽���Ĵ��˺�
	m_nSkillOperateTime += TICK_TIME;

	DWORD dwStartTime = timeGetTime();
	for(; m_nSkillCurDmgIndex < nDmgTimes; m_nSkillCurDmgIndex++)
	{
		// ��tick��ɲ�����˶���˺����㣬�ȵ��¸�tick
		if( pSkill->GetDmgTime(m_nSkillCurDmgIndex) > m_nSkillOperateTime )
			break;

		/** Ares Ⱥ���ڼ����˺������¼���Ŀ�꣬��Ҫ����Ч�� ===========> **/
		if( pSkill->GetOPRadius() > 0.0f )
		{
			//if( pSkill->IsMoveable( ) && !m_pOwner->IsInStateCantMove( ) )
			CalSkillTargetList( m_pOwner->GetCurPos( ), pSkill->GetHitNumber() );
		}
		/** <=============== */
		
		bCaldmg = TRUE;
		// ʱ�䵽�ˣ���ʼ�����˺�
		package_list<DWORD>::list_iter it = m_listTargetID.begin();
		DWORD dwTargetID = INVALID_VALUE;

		while( m_listTargetID.find_next(it, dwTargetID) )
		{
			// �ҵ����Ŀ��
			Unit* pTarget = pMap->find_unit(dwTargetID);

			if( !VALID_POINT(pTarget) || pTarget == m_pOwner) continue;

			// �����˺�
			CalculateDmg(pSkill, pTarget);
		}
	}
	DWORD dwEndTime = timeGetTime();

	DWORD dwMaxPcallTime = dwEndTime - dwStartTime;
	if(dwMaxPcallTime > 0)
	{
		g_world.get_log()->write_log(_T("damage time useing %d millisecond.\r\n"), dwMaxPcallTime);
	}

	if (!m_bTrigger && bCaldmg)
	{

		m_bTrigger = TRUE;
		// ������������Buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Hit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listDodgedTarget, ETEE_Dodged);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listBlockedTarget, ETEE_Blocked);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listCritedTarget, ETEE_Crit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Use);

		// �ҵ�Ŀ��
		Unit* pTarget = pMap->find_unit(m_dwTargetUnitID);
		BOOL bhit = m_dwTargetEffectFlag & ETEF_Hited;
		BOOL bCrit = m_dwTargetEffectFlag & ETEF_Crited;
		if (!VALID_POINT(pTarget))
		{
			pTarget = pMap->find_unit(m_listHitedTarget.front());
			bhit = TRUE;
		}

		if( VALID_POINT(pTarget) )
		{	
			Creature* pPetTarget = pTarget->get_map()->find_creature(pTarget->GetFlowUnit());
			if (VALID_POINT(pPetTarget))
			{
				pPetTarget->GetAI()->AddEnmity(m_pOwner, 1);
				pPetTarget->GetAI()->SetTargetUnitID(m_pOwner->GetID());
			}

			// ��ս�����﹥��Ŀ��
			Creature* pPet = m_pOwner->get_map()->find_creature(m_pOwner->GetFlowUnit());
			if (VALID_POINT(pPet))
			{
				pPet->GetAI()->AddEnmity(pTarget, 1);
				pPet->GetAI()->SetTargetUnitID(pTarget->GetID());
			}

			// Buff����
			if( m_dwTargetEffectFlag & ETEF_Hited )
			{
				// ����
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Hit);
			}
			else
			{
				// ������
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Dodged);
			}

			if( m_dwTargetEffectFlag & ETEF_Block )
			{
				// ����
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Blocked);
			}

			if( m_dwTargetEffectFlag & ETEF_Crited )
			{
				// ����
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Crit);
			}
		
			// ������������߼�
			//if (bhit)
			//	OnHit(pSkill, m_pOwner->get_level()-pTarget->get_level(), bCrit);

			// ���㱻�����ܺ�װ��Buff
			if( m_pOwner->IsRole() )
			{
				// ��Ե�һĿ����м���
				Role* pOwnerRole = static_cast<Role*>(m_pOwner);

				if( m_dwTargetEffectFlag & ETEF_Hited )
				{
					// ����
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Hit);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Hit);
				}
				else
				{
					// ������
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Dodged);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Dodged);
				}

				if( m_dwTargetEffectFlag & ETEF_Block )
				{
					// ����
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Blocked);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Blocked);
				}

				if( m_dwTargetEffectFlag & ETEF_Crited )
				{
					// ����
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Crit);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Crit);
				}
			}
		}

	}

	// ��������˺��Ƿ��Ѿ��������
	if( m_nSkillCurDmgIndex >= nDmgTimes )
	{
		m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);					// ���ʹ�ü��ܴ�ϵ�buff
		// ��������
		//CalculateCost(pSkill);
		// ���ܽ���
		EndUseSkill();
	}

}

//-----------------------------------------------------------------------------
// ����ʹ����ƷЧ��
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateItemOperate()
{
	if( !IsUseItem() ) return;
	if( !IsItemOperating() ) return;
	if( !m_pOwner->IsRole() ) return;

	Role* pOwnerRole = static_cast<Role*>(m_pOwner);

	// �����ҵ������Ʒ
	tagItem* pItem = pOwnerRole->GetItemMgr().GetBagItem(m_n64ItemID); 
	if( !VALID_POINT(pItem) )
	{
		EndUseItem();
		return;
	}

	DWORD	dw_data_id = pItem->dw_data_id;
	Map* pMap = pOwnerRole->get_map();
	if( !VALID_POINT(pMap) ) return;

	/* js zhaopeng 2010.03.28 ����ϵͳҪ���ܶ�NPCʹ����Ʒ
	* ֮ǰ��ʹ����Ʒд��ֻ�ܶ��Լ�ʹ��
	* ����ȡ��ǰĿ��ʹ����Ʒ ���ܻ����µ�BUG  
	*/
	Unit* pTargetUnit = pOwnerRole->get_map()->find_unit( m_dwTargetUnitIDItem );

	// ��������Ŀ����ͻ���
	NET_SIS_hit_target send;
	send.dw_role_id = m_dwTargetUnitIDItem;
	send.dwSrcRoleID = m_pOwner->GetID();
	send.eCause = EHTC_Item;
	send.dwMisc = pItem->dw_data_id;
	send.dwSerial = m_dwItemSerial;
	pMap->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);

	// ����buff
	pOwnerRole->OnActiveItemBuffTrigger(pTargetUnit, pItem, ETEE_Use);
	
	// �Ƿ�Ҫɾ��
	BOOL bDelete = TRUE;
	// ������Ʒ�Ľű�ʹ��Ч��
	if(VALID_POINT(pItem->pScript) && VALID_POINT(pOwnerRole->get_map()))
	{
		bDelete = pItem->pScript->UseItem(pOwnerRole->get_map(), pItem->dw_data_id, m_pOwner->GetID(), m_dwTargetUnitIDItem, pItem->n64_serial);
	}

	// �ƺ���Ϣ
	pOwnerRole->GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, dw_data_id, 1);

	// ������Ʒ������ȴʱ��
	pOwnerRole->GetItemMgr().Add2CDTimeMap(dw_data_id);
	
	if (bDelete)
	{
		// ������Ʒ��ʧ
		pOwnerRole->GetItemMgr().ItemUsedFromBag(m_n64ItemID, 1, (DWORD)elcid_item_use);
	}
	
	// ���������¼�
	pOwnerRole->on_quest_event( EQE_UseItem, dw_data_id , m_dwTargetUnitIDItem );

	EndUseItem();
}
//-----------------------------------------------------------------------------
// ��ʼ���
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateRideOperate()
{
	if(!IsRide())return;
	if(!IsRideOperating())return;

	tagEquip* pRide = ((Role*)m_pOwner)->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(VALID_POINT(pRide))
	{
		Role* pOwner = static_cast<Role*>(m_pOwner);
		DWORD dwRet = pOwner->begin_ride_op();
		if(VALID_POINT(pOwner->get_map()) && dwRet != E_Success)
		{
			NET_SIS_begin_ride send;
			send.dwError = dwRet;
			send.dwRoleID = pOwner->GetID();
			//send.nSpellTime = 0;
			send.dwTypeID = 0;
			pOwner->get_map()->send_big_visible_tile_message(pOwner,&send, send.dw_size);
		}
	}

	EndRide();
}
//-----------------------------------------------------------------------------------
// ȡ������ʹ��
//-----------------------------------------------------------------------------------
VOID CombatHandler::CancelSkillUse(DWORD dwSkillTypeID)
{
	if( !IsValid() || !IsUseSkill() ) return;

	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) || pSkill->GetTypeID() != dwSkillTypeID )
		return;

	BOOL bCanCancel = FALSE;

	// ���������������\������һ������
	if( IsSkillPreparing() || IsSkillPiloting() )
	{
		bCanCancel = TRUE;
	}
	// ��������ͷţ���ֻ����ͨ�����ſ���
	else
	{
		if( ESSTE_Default == pSkill->GetTypeEx() )
		{
			bCanCancel = TRUE;
		}
	}

	// �������ȡ��
	if( bCanCancel )
	{
		NET_SIS_skill_interrupt send;
		send.dw_role_id = m_pOwner->GetID();
		send.dwSkillID = dwSkillTypeID;

		if( VALID_POINT(m_pOwner->get_map()) )
		{
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
		}
		EndUseSkill();
	}
}

//-----------------------------------------------------------------------------------
// ȡ����Ʒ�ͷ�
//-----------------------------------------------------------------------------------
VOID CombatHandler::CancelItemUse(INT64 n64ItemSerial)
{
	if( !IsValid() || !IsUseItem() ) return;

	if( m_n64ItemID != n64ItemSerial ) return;

	BOOL bCanCancel = FALSE;

	// ��Ʒֻ��������ʱ����ȡ��
	if( IsItemPreparing() )
	{
		bCanCancel = TRUE;
	}

	if( bCanCancel )
	{
		NET_SIS_use_item_interrupt send;
		send.dw_role_id = m_pOwner->GetID();
		send.n64ItemID = m_n64ItemID;
		send.dw_data_id = INVALID_VALUE;

		if( VALID_POINT(m_pOwner->get_map()) )
		{
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
		}
		EndUseItem();
	}
}

//----------------------------------------------------------------------------
// ȡ�����
//----------------------------------------------------------------------------
VOID CombatHandler::InterruptRide()
{
	if( !IsValid() || !IsRide()) return;
	if(IsRidePreparing())
	{
		NET_SIS_interrupt_begin_ride send;
		send.dwRoleID = m_pOwner->GetID();
		if( VALID_POINT(m_pOwner->get_map()) )
		{
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
		}
		EndRide();
	}
}
//-----------------------------------------------------------------------------------
// �������
//-----------------------------------------------------------------------------------
BOOL CombatHandler::InterruptPrepare(EInterruptType eType, BOOL bOrdinary, BOOL bForce)
{
	if( FALSE == IsValid() || FALSE == IsPreparing() )
		return TRUE;

	BOOL bSkill = FALSE;		// �Ǽ��������ֻ�����Ʒ������
	DWORD dwSkillTypeID = INVALID_VALUE;
	if( IsSkillPreparing() )	bSkill = TRUE;
	else if(IsSkillPiloting())	bSkill = TRUE;
	else						bSkill = FALSE;

	// ͨ����ʹ����Ʒ����ʹ�ü������жϴ��ֵ
	BOOL bMoveInterrupt = FALSE;
	INT nInterruptSkillRate = 0;

	if( bSkill )
	{
		Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
		if( VALID_POINT(pSkill) )
		{
			const tagSkillProto* pProto = pSkill->GetProto();
			if( VALID_POINT(pProto) )
			{
				bMoveInterrupt = pProto->bInterruptMove;
				nInterruptSkillRate = (bOrdinary ? pProto->nInterruptSkillOrdRate : pProto->nInterruptSkillSpecRate);
			}
			dwSkillTypeID = pSkill->GetTypeID();
		}
	}
	else
	{
		Role* pRole = static_cast<Role*>(m_pOwner);
		tagItem* pItem = pRole->GetItemMgr().GetBagItem(m_n64ItemID);
		if( VALID_POINT(pItem) )
		{
			bMoveInterrupt = pItem->pProtoType->bInterruptMove;
			nInterruptSkillRate = pItem->pProtoType->nInterruptSkillOrdRate;
		}

		if(IsRide() || IsRidePreparing())
		{
			bMoveInterrupt = TRUE;
			nInterruptSkillRate = 10000;
		}
	}

	BOOL bCanInterrupt = FALSE;	// �Ƿ��ܹ����

	if( bForce )
	{
		bCanInterrupt = TRUE;
	}
	else
	{
		// ���Դ��
		switch(eType)
		{
		case EIT_Move:
			{
				if( bMoveInterrupt )
				{
					bCanInterrupt = TRUE;
				}
			}
			break;

		case EIT_Skill:
			{
				// ��ͨ������ϼ���
				if( get_tool()->tool_rand() % 10000 < nInterruptSkillRate )
				{
					bCanInterrupt = TRUE;
				}
			}
			break;

		default:
			break;
		}
	}

	if( bCanInterrupt )
	{
		// ���ʹ�ϸ���Χ���
		if( bSkill )
		{
			NET_SIS_skill_interrupt send;
			send.dw_role_id = m_pOwner->GetID();
			send.dwSkillID = dwSkillTypeID;

			if( VALID_POINT(m_pOwner->get_map()) )
			{
				m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
			}
			EndUseSkill();
		}
		else
		{
			if(IsRide() || IsRidePreparing())
			{
				NET_SIS_interrupt_begin_ride send;
				send.dwRoleID = m_pOwner->GetID();
				if( VALID_POINT(m_pOwner->get_map()) )
				{
					m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
				}
				EndRide();
			}
			else
			{
				NET_SIS_use_item_interrupt send;
				send.dw_role_id = m_pOwner->GetID();
				send.n64ItemID = m_n64ItemID;
				send.dw_data_id = INVALID_VALUE;

				if( VALID_POINT(m_pOwner->get_map()) )
				{
					m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
				}
				EndUseItem();
			}
		}

		return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
// ����ͷ�
//-------------------------------------------------------------------------------
BOOL CombatHandler::InterruptOperate(EInterruptType eType, DWORD dwMisc, BOOL bForce/* =FALSE */)
{
	if( FALSE == IsValid() || FALSE == IsSkillOperating() )
		return TRUE;

	if( EIT_Move == eType )
	{
		EMoveState eState = (EMoveState)dwMisc;

		// �ߺ���Ӿ��ص��ƶ�����ֻ���ƶ���ϵ���ͨ�����Ŵ��
		if( EMS_Walk			== eState ||
			EMS_CreaturePatrol	== eState ||
			EMS_CreatureWalk	== eState ||
			EMS_CreatureFlee	== eState ||
			EMS_CreatureRun		== eState)
		{
			Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
			if( VALID_POINT(pSkill) && pSkill->IsMoveCancel() )
			{
				EndUseSkill();
				return TRUE;
			}
		}
		// �����ƶ���ʽ����ֻҪ����ͨ�����ʹ��
		else
		{
			Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
			if( VALID_POINT(pSkill) && ESSTE_Default == pSkill->GetTypeEx() )
			{
				EndUseSkill();
				return TRUE;
			}
		}
	}

	return FALSE;
}


//-------------------------------------------------------------------------------
// �Ƿ����ʹ�ü���
//-------------------------------------------------------------------------------
INT CombatHandler::CanCastSkill(Skill* pSkill, DWORD dwTargetUnitID ,const Vector3& vDesPos)
{
	if( !VALID_POINT(pSkill) )
		return E_SystemError;

	// �����ƶ��ͷ�
	//if( !pSkill->IsMoveable() && m_pOwner->GetMoveData().GetCurMoveState() != EMS_Stand)
	//{
	//	return E_UseSkill_SelfStateLimit;
	//}
	
	//if( IsPreparing() ) return E_UseSkill_Operating;	// ��ǰ�������֣�����ʹ���κμ���

	//if(IsPiloting()) return E_UseSkill_Operating;  //add by guohui ����Ƿ�������״̬�£����������ʹ�ü���

	INT nRet = E_Success;

	nRet = CheckSkillAbility(pSkill);
	if( E_Success != nRet ) return nRet;

	//nRet = CheckOwnerLimitSkill();
	//if( E_Success != nRet ) return nRet;

	nRet = CheckSkillLimit(pSkill);
	if( E_Success != nRet ) return nRet;

	nRet = CheckTargetLimit(pSkill, dwTargetUnitID ,vDesPos);
	if( E_Success != nRet ) return nRet;

	nRet = CheckCostLimit(pSkill);
	if( E_Success != nRet ) return nRet;

	//nRet = CheckVocationLimit(pSkill);
	//if( E_Success != nRet ) return nRet;

	nRet = CheckMapLimit(pSkill);
	if( E_Success != nRet ) return nRet;

	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		nRet = pScript->CanCastSkill(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), dwTargetUnitID);
		if( E_Success != nRet ) return nRet;
	}
	
	if (m_pOwner->IsRole())
	{
		DWORD dwItemID;
		INT nItemNum;
		pSkill->GetCostItem(dwItemID, nItemNum);
		

		if (dwItemID != INVALID_VALUE && dwItemID != 0 && nItemNum > 0)
		{
			if (((Role*)m_pOwner)->GetItemMgr().GetBag().GetSameItemCount(dwItemID) >= nItemNum)
			{
				package_list<tagItem*> itemList;

				((Role*)m_pOwner)->GetItemMgr().GetBag().GetSameItemList(itemList, dwItemID, nItemNum);
				((Role*)m_pOwner)->GetItemMgr().DelBagSameItem(itemList, nItemNum, elcid_skill);
			}
			else
			{
				nRet = E_UseSKill_Not_Item;
			}
		}
		
	}
	
	if( CheckSkillConflict(pSkill) ) 
		return E_UseSkill_Operating;

	return nRet;
}

//-------------------------------------------------------------------------------
// ���Լ��ܱ����Ƿ��ܹ�ʹ��
//-------------------------------------------------------------------------------
INT CombatHandler::CheckSkillAbility(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return E_UseSkill_SkillNotExist;

	// �������ܲ�����ʹ��
	if( pSkill->IsPassive() )
		return E_UseSkill_PassiveSkill;

	// ������ܵ�Ŀ�����Ͳ���ս��Ŀ����ս��Ŀ�꣬�򲻿���ʹ��
	ESkillTargetType eTargetType = pSkill->GetTargetType();
	if( ESTT_Combat != eTargetType && ESTT_NoCombat != eTargetType )
		return E_UseSkill_SkillTargetInvalid;

	// ���ܵ���ȴʱ�仹û�����򲻿���ʹ��
	if( pSkill->GetCoolDownCountDown() > pSkill->GetProto()->nMaxCoolDown )
		return E_UseSkill_CoolDowning;

	if(pSkill->GetProto()->bPublicCDLimit)
	{
		if(m_dwPublicCoolDown > 0)
			return E_UseSkill_CoolDowning;
	}
	
	return E_Success;
}

//-------------------------------------------------------------------------------
// ���Լ���ʹ�����Ƿ��ܹ�ʹ�ü���
//-------------------------------------------------------------------------------
INT CombatHandler::CheckOwnerLimitSkill()
{
	// �Ƿ��ڲ���ʹ�ü��ܵ�״̬
	if( m_pOwner->IsInStateCantCastSkill() )
		return E_UseSkill_UseLimit;

	return E_Success;
}

//-------------------------------------------------------------------------------
// ���Լ��ܱ���ʹ������
//-------------------------------------------------------------------------------
INT CombatHandler::CheckSkillLimit(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return E_UseSkill_SkillNotExist;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;
	
	// �������ڻ�����������
	if( pProto->nUseHPPctLimit > 0 )
	{
		if( m_pOwner->GetAttValue(ERA_MaxHP) <= 0 )
			return E_UseSkill_UseLimit;

		if( (FLOAT)m_pOwner->GetAttValue(ERA_HP) / (FLOAT)m_pOwner->GetAttValue(ERA_MaxHP) * 10000.0f < pProto->nUseHPPctLimit )
			return E_UseSkill_UseLimit;

	}
	if( pProto->nUseMPPctLimit > 0 )
	{
		if( m_pOwner->GetAttValue(ERA_MaxMP) <= 0 )
			return E_UseSkill_UseLimit;

		if( (FLOAT)m_pOwner->GetAttValue(ERA_MP) / (FLOAT)m_pOwner->GetAttValue(ERA_MaxMP) * 10000.0f < pProto->nUseHPPctLimit )
			return E_UseSkill_UseLimit;

	}

	// �Ա�����
	//if( pProto->eSexLimit != ESSL_Null )
	//{
	//	if( ESSL_Man == pProto->eSexLimit )
	//	{
	//		if( 1 != m_pOwner->GetSex() )
	//			return E_UseSkill_SexLimit;
	//	}
	//	else if( ESSL_Woman == pProto->eSexLimit )
	//	{
	//		if( 0 != m_pOwner->GetSex() )
	//			return E_UseSkill_SexLimit;
	//	}
	//	else
	//	{

	//	}
	//}


	//if (m_pOwner->IsRole())
	//{
	//	// ��������
	//	if( EITE_Null != pProto->nWeaponLimit )
	//	{
	//		// �����;öȲ���
	//		Role* pRole = static_cast<Role*>(m_pOwner);
	//		tagEquip* pWeapon = pRole->GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	//		if( VALID_POINT(pWeapon) && pWeapon->GetEquipNewness() <= 0)
	//		{
	//			return E_UseSkill_Not_Newness;
	//		}

	//		if( !VALID_POINT(pWeapon) )
	//		{
	//			return E_UseSkill_WeaponLimit;
	//		}

	//	}

	//	if(static_cast<Role*>(m_pOwner)->IsInRoleState(ERS_Carry))
	//		return E_UseSkill_SelfStateLimit;
	//}


	// ����Buff����
	//if( VALID_POINT(pProto->dwBuffLimitID) )
	//{
	//	if( !m_pOwner->IsHaveBuff(pProto->dwBuffLimitID) )
	//	{
	//		return E_UseSkill_SelfBuffLimit;
	//	}
	//}
	
	if (m_pOwner->IsDead())
	{
		return E_UseSkill_SelfStateLimit;
	}
	// �������״̬����
	if (m_pOwner->IsInState(ES_Dizzy))
	{
		return E_UseSkill_SelfStateLimit;
	}
	//DWORD dwSelfStateFlag = m_pOwner->GetStateFlag();
	//if( (dwSelfStateFlag & pProto->dwSelfStateLimit) != dwSelfStateFlag)
	//{
	//	return E_UseSkill_SelfStateLimit;
	//}

	return E_Success;
}

//-------------------------------------------------------------------------------
// ����Ŀ������
//-------------------------------------------------------------------------------
INT CombatHandler::CheckTargetLimit(Skill* pSkill, DWORD dwTargetUnitID, const Vector3& vdespos)
{
	if( !VALID_POINT(pSkill) )
		return E_UseSkill_SkillNotExist;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;

	// ���TargetUnitID��INVALID_VALUE������Ҫ�����ж�һ��
	if( INVALID_VALUE == dwTargetUnitID )
	{
		if( ESOPT_Explode == pSkill->GetOPType() && 0.0f == pSkill->GetOPDist() )
		{
			return E_Success;
		}
		else if(ESOPT_Sector == pSkill->GetOPType() )  //�������׶����Ϊ����ʩ�� add by guohui
		{
			return E_Success;
		}
		else if (ESOPT_Persist == pSkill->GetOPType()) //����ǳ�������Ϊ���ͷ�
		{
			const Vector3& vSrc = m_pOwner->GetCurPos();
			const Vector3& vDest = vdespos;

			FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

			// ս������Ҫ����˫�����Եİ뾶���ټ���һ���������Ա��������ӳ�
			FLOAT fRealDist = pSkill->GetOPDist() + (m_pOwner->GetSize().z);

			if( fRealDist * fRealDist >= fDistSq )
				return E_Success;
			else
				return E_UseSkill_DistLimit;
		}
		else
		{
			return E_UseSkill_SkillTargetInvalid;
		}
	}

	Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetUnitID);
	if( !VALID_POINT(pTarget) ) return E_UseSkill_SkillTargetInvalid;

	// λ�����ƣ��������ƺͷ�Χ�ж�
	if( m_pOwner != pTarget )
	{
		// λ������
		//if( ESPT_NUll != pProto->ePosType )
		//{
		//	if( ESPT_Front == pProto->ePosType )
		//	{
		//		if( FALSE == m_pOwner->IsInFrontOfTarget(*pTarget) )
		//			return E_UseSkill_PosLimitFront;
		//	}
		//	else if( ESPT_Back == pProto->ePosType )
		//	{
		//		if( TRUE == m_pOwner->IsInFrontOfTarget(*pTarget) )
		//			return E_UseSkill_PosLimitBack;
		//	}
		//}

		// Ŀ������ж�
		if( (FALSE == m_pOwner->IsInCombatDistance(*pTarget, pSkill->GetOPDist(), 250)) &&
			(FALSE == m_pOwner->IsInCombatDistance(*pTarget, pSkill->GetOPRadius())))
			return E_UseSkill_DistLimit;
		
		//// ���߼��
		//if( pProto->bCollide && m_pOwner->IsRayCollide(*pTarget) )
		//	return E_UseSkill_RayLimit;
		
		// Ŀ������߼�����
		INT nRet = CheckTargetLogicLimit(pSkill, pTarget);
		if( nRet != E_Success )	return nRet;

		CheckInCombat(pTarget);
		//��Գ�ײ���ܼ��жϣ�ʹ�ü��ܵ���ҵȼ��������Ŀ����Ҳſ�ʹ�� gx add 2013.8.30
		if ((10107 == pProto->dwID /100) || (10207 == pProto->dwID /100) || (10307 == pProto->dwID /100))
		{
			if (m_pOwner->get_level() <= pTarget->get_level())
			{
				return E_UseSkill_Not_tou_level;
			}
		}
		//end
	}
	
	return E_Success;
}

//-------------------------------------------------------------------------------
// ����Ŀ���߼�����
//-------------------------------------------------------------------------------
INT CombatHandler::CheckTargetLogicLimit(Skill* pSkill, Unit* pTarget)
{
	if( !VALID_POINT(pSkill) || !VALID_POINT(pTarget) )
		return E_UseSkill_SkillNotExist;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;

	// ���Ŀ���Ƿ��ܱ�ʹ�ü���
	if( pTarget->IsInStateCantBeSkill() )
	{
		return E_UseSkill_TargetLimit;
	}

	// ���ȼ����Ŀ������ͱ�־
	DWORD dwTargetFlag = m_pOwner->TargetTypeFlag(pTarget);
	if( !(dwTargetFlag & pProto->dwTargetLimit) )
		return E_UseSkill_TargetLimit;
	
	// Ŀ��ȼ�����
	//if (pProto->nTargetLevelLimit > 0 && pProto->nTargetLevelLimit < pTarget->get_level())
	//	return E_UseSkill_TargetLimit;

	// �ټ��Ŀ���״̬����
	DWORD dwTargetStatFlag = pTarget->GetStateFlag();
	if( (dwTargetStatFlag & pProto->dwTargetStateLimit) != dwTargetStatFlag )
	{
		return E_UseSkill_TargetStateLimit;
	}

	//if(pTarget->IsRole())
	//{
	//	if(static_cast<Role*>(pTarget)->IsInRoleState(ERS_Carry))
	//		return E_UseSkill_TargetStateLimit;
	//}
	// ���Ŀ��Buff����
	if( VALID_POINT(pProto->dwTargetBuffLimitID) )
	{
		if( !pTarget->IsHaveBuff(pProto->dwTargetBuffLimitID) )
		{
			return E_UseSkill_TargetBuffLimit;
		}
	}	
	

	//zhjl changed �����ͼ������Ӫ���ֵ��ѣ����ü������ж�
	if (m_pOwner->get_map() && m_pOwner->get_map()->get_map_type()==EMT_PVP_BIWU)
	{
		return E_Success;
	}
	// �ټ������ж�
	if( m_pOwner != pTarget )
	{
		DWORD dwFriendEnemyFlag = m_pOwner->FriendEnemy(pTarget);

		DWORD dwFriendEnemyLimit = 0;

		if( pProto->bFriendly )		dwFriendEnemyLimit |= ETFE_Friendly;
		if( pProto->bHostile )		dwFriendEnemyLimit |= ETFE_Hostile;
		if( pProto->bIndependent )	dwFriendEnemyLimit |= ETFE_Independent;

		if( !(dwFriendEnemyLimit & dwFriendEnemyFlag) )
		{
			return E_UseSkill_TargetLimit;
		}
		
		/* �����Ƶ�CheckInCombat�������� lc

		//if((dwFriendEnemyFlag & ETFE_Hostile) || (dwFriendEnemyFlag & ETFE_Independent))
		//{
		//	if(m_pOwner->IsRole())
		//	{
		//		if(pTarget->IsRole())
		//		{
		//			m_dwCombatStateCoolDown = 6000;
		//		if(!((Role*)m_pOwner)->IsInRoleState(ERS_Combat))
		//		{
		//			((Role*)m_pOwner)->SetRoleState(ERS_Combat);
		//		}
		//	}
		//	}

		//	if(pTarget->IsRole())
		//	{
		//		if(!((Role*)pTarget)->IsInRoleState(ERS_Combat))
		//		{
		//			((Role*)pTarget)->SetRoleState(ERS_Combat);
		//			pTarget->OnInterruptBuffEvent(EBIF_InterCombat);
		//		}
		//		if(m_pOwner->IsRole())
		//		{
		//			((Role*)pTarget)->GetCombatHandler().SetCombatStateCoolDown();
		//		}
		//	}
		}*/
	}

	// �жϳɹ�
	return E_Success;
}
//----------------------------------------------------------------------------------------
//ս��״̬�ж�
//----------------------------------------------------------------------------------------
VOID	CombatHandler::CheckInCombat(Unit* pTarget)
{
	if(!VALID_POINT(pTarget) )
		return;

	// �ټ������ж�
	if( m_pOwner != pTarget )
	{
		DWORD dwFriendEnemyFlag = m_pOwner->FriendEnemy(pTarget);

		if((dwFriendEnemyFlag & ETFE_Hostile) || (dwFriendEnemyFlag & ETFE_Independent))
		{
			if(m_pOwner->IsRole())
			{
				if(pTarget->IsRole())
				{
					m_dwCombatStateCoolDown = 6000;
					if(!((Role*)m_pOwner)->IsInRoleState(ERS_Combat))
					{
						((Role*)m_pOwner)->SetRoleState(ERS_Combat);
						((Role*)m_pOwner)->StopMount();
					}
				}
			}

			if(pTarget->IsRole())
			{
				if(!((Role*)pTarget)->IsInRoleState(ERS_Combat))
				{
					((Role*)pTarget)->SetRoleState(ERS_Combat);
					((Role*)pTarget)->StopMount();
					pTarget->OnInterruptBuffEvent(EBIF_InterCombat);
				}
				if(m_pOwner->IsRole())
				{
					((Role*)pTarget)->GetCombatHandler().SetCombatStateCoolDown();
				}
			}
		}
	}

}
//----------------------------------------------------------------------------------------
// ����ͼ�м�������
//----------------------------------------------------------------------------------------
INT CombatHandler::CheckMapLimit(Skill* pSkill)
{
	// �жϵ�ͼ����
	if(VALID_POINT(m_pOwner->get_map()))
	{
		//add by zhjl 2013-12-25:��ӱ��丱��δ����ʱ������
		MapMgr* p_dest_map_manager = g_mapCreator.get_map_manager(m_pOwner->get_map()->get_map_id());
		if( VALID_POINT(p_dest_map_manager) )
		{
			if ( !p_dest_map_manager->CanUseSkill() )
				return E_UseSkill_Biwu_Not_Start;
		}

		BOOL bUesAble = m_pOwner->get_map()->can_use_skill(pSkill->GetID());
		if( !bUesAble )	return E_UseSkill_MapLimit;
	}

	return E_Success;
}

//----------------------------------------------------------------------------------------
// ���Լ���ʹ�ó�ͻ������TRUEΪ��ͻ��FALSEΪ�ǳ�ͻ
//----------------------------------------------------------------------------------------
BOOL CombatHandler::CheckSkillConflict(Skill* pSkill)
{
	ASSERT( VALID_POINT(pSkill) );

	if( !IsValid() ) return FALSE;		// ��ǰû��ʹ���κμ��ܺ��κ���Ʒ

	//if( IsPreparing() ) return TRUE;	// ��ǰ�������֣�����ʹ���κμ���

	//if(IsPiloting()) return TRUE;  //add by guohui ����Ƿ�������״̬�£����������ʹ�ü���

	if( IsUseSkill() )
	{
		// ��ǰ����ʹ�ü��ܣ���鿴�ü����Ƿ�����ͨ����
		Skill* pCurSkill = m_pOwner->GetSkill(m_dwSkillID);
		if( VALID_POINT(pSkill) && ESSTE_Default != pCurSkill->GetTypeEx() )
		{
			return TRUE;
		}
		else
		{
			EndUseSkill();
			return FALSE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
// ���Լ�������
//-------------------------------------------------------------------------------
INT CombatHandler::CheckCostLimit(Skill* pSkill)
{
	// �����������
	INT nHPCost = pSkill->GetCost(ESCT_HP);
	if( nHPCost > 0 && m_pOwner->GetAttValue(ERA_HP) < nHPCost )
		return E_UseSkill_CostLimit;


	// �����������
	INT nMPCost = pSkill->GetCost(ESCT_MP);
	if( nMPCost > 0 && m_pOwner->GetAttValue(ERA_MP) < nMPCost )
		return E_UseSkill_CostLimit;


	// ��Ⱞ������
	INT nRageCost = pSkill->GetCost(ESCT_Love);
	if( nRageCost > 0 && m_pOwner->GetAttValue(ERA_Love) < nRageCost )
		return E_UseSkill_CostLimit;


	// ���־�����
	//INT nEnduranceCost = pSkill->GetCost(ESCT_Endurance);
	//if( nEnduranceCost > 0 && m_pOwner->GetAttValue(ERA_Endurance) < nEnduranceCost )
	//	return E_UseSkill_CostLimit;


	// ����������
	//INT nValicityCost = pSkill->GetCost(ESCT_Valicity);
	//if( nValicityCost > 0 && m_pOwner->GetAttValue(ERA_Vitality) < nValicityCost )
	//	return E_UseSkill_CostLimit;

	return E_Success;
}

//-------------------------------------------------------------------------------
// ����ְҵ����
//-------------------------------------------------------------------------------
INT CombatHandler::CheckVocationLimit(Skill* pSkill)
{
	//ASSERT(VALID_POINT(pSkill));
	if (!VALID_POINT(pSkill)) return E_UseSkill_SkillNotExist;

	if (!m_pOwner->IsRole()) return E_Success;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;

	//INT nClass = (INT)((Role*)m_pOwner)->GetClass();
	//INT nClassEx = (INT)((Role*)m_pOwner)->GetClassEx();
	INT nClass = (INT)(static_cast<Role*> (m_pOwner)->GetClass());
	INT nClassEx = (INT)(static_cast<Role*> (m_pOwner)->GetClassEx());

	if (((nClassEx << 9) + nClass) & pProto->dwVocationLimit)
		return E_Success;
	else
		return E_UseSkill_VocationLimit;
}

//-------------------------------------------------------------------------------
// ���㹥��Ŀ�꣬���뵽list��
//-------------------------------------------------------------------------------
VOID CombatHandler::CalSkillTargetList(Vector3 vDesPos, DWORD dwMaxNumber)
{
	m_listTargetID.clear();
	m_listHitedTarget.clear();
	m_listDodgedTarget.clear();
	m_listBlockedTarget.clear();
	m_listCritedTarget.clear();
	m_dwTargetEffectFlag = 0;
	
	DWORD dwCurrNumber = 0;
	// ���ݸü��ܵĹ�������͹�����Χ���ж�
	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	// �õ�Ŀ�����
	Unit* pTarget = NULL;
	if( INVALID_VALUE == m_dwTargetUnitID)	// ���û��ѡĿ�꣬��Ŀ������Լ�
	{
		pTarget = m_pOwner;
	}
	else									// ���ѡ��Ŀ�꣬���ҵ�Ŀ��
	{
		pTarget = m_pOwner->get_map()->find_unit(m_dwTargetUnitID);
	}
	if( !VALID_POINT(pTarget) ) return;

	// �����������ͣ����þ�������ð뾶��ʹ�ü���
	ESkillOPType eOPType = pSkill->GetOPType();
	FLOAT fOPDist = pSkill->GetOPDist();
	FLOAT fOPRadius = pSkill->GetOPRadius();
	
	// ���Ŀ��
	if( E_Success == CheckTargetLogicLimit(pSkill, pTarget ) )
	{
		m_listTargetID.push_back(pTarget->GetID());
		m_dwTargetEffectFlag = CalculateSkillEffect(pSkill, pTarget);
	}
	
	// �Ƚ�Ŀ��ӽ�ȥ
	//if( m_pOwner != pTarget )
	//{
	//	m_listTargetID.push_back(pTarget->GetID());
	//	m_dwTargetEffectFlag = CalculateSkillEffect(pSkill, pTarget);
	//}

	// ��ըЧ��
	if( ESOPT_Explode == eOPType )
	{
		// ���������ΧΪ0����ֱ�ӷ���
		if( 0.0f == fOPRadius )
			return;
		
		m_listTargetID.clear();

		// ���ĵ�
		Unit* pCent = pTarget;
		// ��������Ϊ0�����Լ�Ϊ����
		if (fOPDist == 0)
		{
			pCent = m_pOwner;
		}
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;
		
		Vector3 vecCent;
		if (pSkill->GetProto()->bPoint)
		{
			vecCent = vDesPos;
		}
		else
		{
			vecCent = pCent->GetCurPos();
		}

		tagVisTile* pVisTile[EUD_end] = {0};

		// �õ�������Χ�ڵ�vistile�б�
		m_pOwner->get_map()->get_visible_tile(vecCent, fOPRadius, pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// ���ȼ������
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// ��Ŀ��һ������������
				//if( pRole == pTarget || pRole == m_pOwner ) continue;

				// ���ܾ����ж�
				FLOAT fDistSQ = Vec3DistSq(vecCent, pRole->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;


				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole ) )
					continue;
				
				CheckInCombat(pRole);
				// ���߼��

				// �ж�ͨ��������Ҽ��뵽�б���
				m_listTargetID.push_back(pRole->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// �ټ������
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// ��Ŀ��һ������������
				//if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// ���ܾ����ж�
				FLOAT fDistSQ = Vec3DistSq(vecCent, pCreature->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;

				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);

				// ���߼��

				// �ж�ͨ������������뵽�б���
				m_listTargetID.push_back(pCreature->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}
	}

	// ����
	else if( ESOPT_Rect == eOPType )
	{
		// ���������Χ���߹�������Ϊ0����ֱ�ӷ���
		if( 0.0f == fOPRadius || 0.0f == fOPDist )	return;

		// ���������Χ��Ϊ0������Ŀ��Ϊ���ļ��
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;
		FLOAT fOPDistSQ = fOPDist * fOPDist;

		// ���������Χ�͹����������Ϊ0����������Ϊ��׼���
		FLOAT fTargetX = pTarget->GetCurPos().x;
		FLOAT fTargetZ = pTarget->GetCurPos().z;

		FLOAT fSrcX = m_pOwner->GetCurPos().x;
		FLOAT fSrcZ = m_pOwner->GetCurPos().z;

		// ������һĿ��������
		FLOAT fX2 = fTargetX - fSrcX;
		FLOAT fZ2 = fTargetZ - fSrcZ;

		// ���Ŀ�����������ôֱ��ȡ����ĳ�������
		if( m_pOwner == pTarget )
		{
			fX2 = m_pOwner->GetFaceTo().x;
			fZ2 = m_pOwner->GetFaceTo().z;
		}

		if( abs(fX2) < 0.001f && abs(fZ2) < 0.001f )
			return;

		// ������һĿ���ľ����ƽ��
		FLOAT fDistSQ2 = fX2*fX2 + fZ2*fZ2;

		tagVisTile* pVisTile[EUD_end] = {0};

		// �õ�vistile�б�
		pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// ���ȼ������
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// ��Ŀ��һ������������
				if( pRole == pTarget || pRole == m_pOwner ) continue;

				// ������ǰ�������
				FLOAT fX1 = pRole->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pRole->GetCurPos().z - fSrcZ;

				// �ȼ�鷽λ cos(a) > 0 
				if( fX1*fX2	+ fZ1*fZ2 < 0.0f )
					continue;

				FLOAT fDist1 = fX1*fX2 + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;

				// ���ͶӰ����
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// ���㵽ֱ�߾���
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
					continue;
				
				CheckInCombat(pRole);
				// ���߼��

				// �ж�ͨ��������Ҽ��뵽�б���
				m_listTargetID.push_back(pRole->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// �ټ������  ����߼�2010.3.25��������by guohui
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// ��Ŀ��һ������������
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// ������ǰ�������
				FLOAT fX1 = pCreature->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pCreature->GetCurPos().z - fSrcZ;

				// �ȼ�鷽λ cos(a) > 0 
				if( fX1*fX2	+ fZ1*fZ2 < 0.0f )
					continue;

				// ���ͶӰ����
				FLOAT fDist1 = fX1*fX2  + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// ���㵽ֱ�߾���
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);
				// ���߼��

				// �ж�ͨ��������Ҽ��뵽�б���
				m_listTargetID.push_back(pCreature->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}
	}

	else if (ESOPT_Sector == eOPType)  //��׶ add by guohui
	{
		// ���������Χ���߹�������Ϊ0����ֱ�ӷ���
		if( 0.0f == fOPRadius || 0.0f == fOPDist )	return;

		// ���������Χ��Ϊ0������Ŀ��Ϊ���ļ��
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;
		FLOAT fOPDistSQ = fOPDist * fOPDist;

		// ���������Χ�͹����������Ϊ0����������Ϊ��׼���
		//FLOAT fTargetX = pTarget->GetCurPos().x;
		//FLOAT fTargetY = pTarget->GetCurPos().y;
		//FLOAT fTargetZ = pTarget->GetCurPos().z;
		FLOAT fSrcX = m_pOwner->GetCurPos().x;
		FLOAT fSrcZ = m_pOwner->GetCurPos().z;

		// ������һĿ��������
		//FLOAT fX2 = fTargetX - fSrcX;
		//FLOAT fY2 = fTargetY - fSrcY;
		//FLOAT fZ2 = fTargetZ - fSrcZ;

		FLOAT fX2 = m_pOwner->GetFaceTo().x;
		FLOAT fZ2 = m_pOwner->GetFaceTo().z;

		if( abs(fX2) < 0.001f && abs(fZ2) < 0.001f )
			return;

		// ������һĿ���ľ����ƽ��
		FLOAT fDistSQ2 = fX2*fX2 + fZ2*fZ2;

		tagVisTile* pVisTile[EUD_end] = {0};

		// �õ�vistile�б�
		pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// ���ȼ������
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// ��Ŀ��һ������������
				if( pRole == pTarget || pRole == m_pOwner ) continue;

				// ������ǰ�������
				FLOAT fX1 = pRole->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pRole->GetCurPos().z - fSrcZ;

				// �ȼ�鷽λ cos(a) > 0 
				if(( (fX1*fX2 + fZ1*fZ2)/(sqrt(fX1*fX1 + fZ1*fZ1) * sqrt(fX2*fX2 + fZ2*fZ2))) < 0.5f )
					continue;

				FLOAT fDist1 = fX1*fX2 + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;

				// ���ͶӰ����
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// ���㵽ֱ�߾���
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
					continue;
				
				CheckInCombat(pRole);

				// ���߼��

				// �ж�ͨ��������Ҽ��뵽�б���
				m_listTargetID.push_back(pRole->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// �ټ������  ����߼�2010.3.25��������by guohui
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// ��Ŀ��һ������������
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// ������ǰ�������
				FLOAT fX1 = pCreature->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pCreature->GetCurPos().z - fSrcZ;

				// �ȼ�鷽λ cos(a) > 0 
				if(( (fX1*fX2 + fZ1*fZ2)/(sqrt(fX1*fX1 + fZ1*fZ1) * sqrt(fX2*fX2 + fZ2*fZ2))) < 0.5f )
					continue;


				// ���ͶӰ����
				FLOAT fDist1 = fX1*fX2 + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// ���㵽ֱ�߾���
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);
				// ���߼��

				// �ж�ͨ��������Ҽ��뵽�б���
				m_listTargetID.push_back(pCreature->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}
	}
	else if (ESOPT_Persist == eOPType)
	{
		// ���������ΧΪ0����ֱ�ӷ���
		if( 0.0f == fOPRadius )
			return;

		// ���������Χ��Ϊ0������Ŀ��Ϊ���ļ��
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;

		tagVisTile* pVisTile[EUD_end] = {0};

		// �õ�������Χ�ڵ�vistile�б�
		pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// ���ȼ������
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// ��Ŀ��һ������������
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// ���ܾ����ж�
				FLOAT fDistSQ = Vec3DistSq(vDesPos, pRole->GetCurPos()); //�ô������ĵ�������
				if( fDistSQ > fOPRadiusSQ  ) continue;
				
				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
					continue;
				
				CheckInCombat(pRole);

				// ���߼��
			
				// �ж�ͨ��������Ҽ��뵽�б���
				m_listTargetID.push_back(pRole->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// �ټ������
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// ��Ŀ��һ������������
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;


				// ���ܾ����ж�
				FLOAT fDistSQ = Vec3DistSq(vDesPos, pCreature->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;
				
				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);
				// ���߼��

				// �ж�ͨ������������뵽�б���
				m_listTargetID.push_back(pCreature->GetID());

				// ���㼼�����ý��
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}		
	}
	//{
	//	// ���������Χ���߹�������Ϊ0����ֱ�ӷ���
	//	//fOPRadius���ֵ����׶�����д�������ﳯ��ļн�(�Ƕȱ�ʾ)
	//	if( 0.0f == fOPRadius || 0.0f == fOPDist )	return;
	//	FLOAT angle = fOPRadius * 3.1415927f / 180; //���һ���нǣ������������ļнǵ�1/2

	//	Vector3 vecFaceTo = m_pOwner->GetFaceTo(); //�õ���ҳ��������
	//	if(abs(vecFaceTo.x) < 0.001f &&abs(vecFaceTo.z) < 0.001f) return ; 

	//	D3DXVec3Normalize(&vecFaceTo,&vecFaceTo);

	//	tagVisTile* pVisTile[EUD_end] = {0};
	//	pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);//�õ�vistile�б�
	//	Role*		pRole		= NULL;
	//	Creature*	pCreature	= NULL;


	//	for(INT n = EUD_center; n < EUD_end; n++)
	//	{
	//		if( !VALID_POINT(pVisTile[n]) ) continue;

	//		// ���ȼ������
	//		package_map<DWORD, Role*>& mapRole = pVisTile[n]->mapRole;
	//		package_map<DWORD, Role*>::map_iter it = mapRole.Begin();

	//		while( mapRole.find_next(it, pRole) )
	//		{
	//			//Ŀ������ң���������
	//			if( pRole == m_pOwner ) continue;

	//			// Ŀ����������ж�
	//			if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
	//				continue;

	//			// ������ǰ������������
	//			Vector3 VecToDes = pRole->GetCurPos() - m_pOwner->GetCurPos();
	//			// �ȼ�����ҳ���λ�Ƿ�һ�� cos(a) > 0 �Ƿ������ǰ��
	//			if(VecToDes.x * vecFaceTo.x +VecToDes.z * vecFaceTo.z < 0.0f )
	//				continue;

	//			Vector3 vecToDesNomrliazed;
	//			D3DXVec3Normalize(&vecToDesNomrliazed,&VecToDes);
	//			FLOAT angleD2F = acos(D3DXVec3Dot(&vecToDesNomrliazed,&vecFaceTo));//������������ļн�
	//			if(angleD2F  < 0) angleD2F = -angleD2F;

	//			if(angleD2F > angle)	continue;//������ڸ����ĽǶȾ��޷�������

	//			FLOAT fDis = D3DXVec3Length(&VecToDes); //������
	//			if(fDis > fOPDist )		continue;

	//			// �ж�ͨ��������Ҽ��뵽�б���
	//			m_listTargetID.PushBack(pRole->GetID());

	//			// ���㼼�����ý��
	//			CalculateSkillEffect(pSkill, pRole);
	//		}

	//		// �ټ������
	//		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->mapCreature;
	//		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.Begin();

	//		while( mapCreature.find_next(it2, pCreature) )
	//		{
	//			//Ŀ������ң���������
	//			if( pCreature == m_pOwner ) continue;

	//			// Ŀ����������ж�
	//			if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
	//				continue;

	//			// ������ǰ������������
	//			Vector3 VecToDes = pCreature->GetCurPos() - m_pOwner->GetCurPos();
	//			// �ȼ�����ҳ���λ�Ƿ�һ�� cos(a) > 0 �Ƿ������ǰ��
	//			if(VecToDes.x * vecFaceTo.x +VecToDes.z * vecFaceTo.z < 0.0f )
	//				continue;

	//			Vector3 vecToDesNomrliazed;
	//			D3DXVec3Normalize(&vecToDesNomrliazed,&VecToDes);
	//			FLOAT angleD2F = acos(D3DXVec3Dot(&vecToDesNomrliazed,&vecFaceTo));//������������ļн�
	//			if(angleD2F  < 0) angleD2F = -angleD2F;
	//			if(angleD2F > angle)	continue;//������ڸ����ĽǶȾ��޷�������

	//			FLOAT fDis = D3DXVec3Length(&VecToDes); //�����⣬��������
	//			if(fDis < fOPDist )		continue;

	//			// �ж�ͨ��������Ҽ��뵽�б���
	//			m_listTargetID.PushBack(pCreature->GetID());

	//			// ���㼼�����ý��
	//			CalculateSkillEffect(pSkill, pCreature);
	//		}
	//	}
	//	}
}

//-------------------------------------------------------------------------------
// ���㼼��Ч��
//-------------------------------------------------------------------------------
DWORD CombatHandler::CalculateSkillEffect(Skill* pSkill, Unit* pTarget)
{
	DWORD dwTargetEffectFlag = 0;

	DWORD dwTargetID = pTarget->GetID();

	INT nDmgTimes = pSkill->GetDmgTimes();

	// ���˺����ܣ�������
	if( nDmgTimes <= 0 )
	{
		m_listHitedTarget.push_back(dwTargetID);
		dwTargetEffectFlag |= ETEF_Hited;

		NET_SIS_hit_target send;
		send.dw_role_id = pTarget->GetID();
		send.dwSrcRoleID = m_pOwner->GetID();
		send.eCause = EHTC_Skill;
		send.dwMisc = pSkill->GetTypeID();
		send.dwSerial = m_dwSkillSerial;

		if( VALID_POINT(pTarget->get_map()) )
		{
			pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		}

		pTarget->OnBeAttacked(m_pOwner, pSkill, TRUE, FALSE, FALSE);
		return dwTargetEffectFlag;
	}

	// ��������
	BOOL bHit = CalculateHit(pSkill, pTarget);
	if( FALSE == bHit )
	{
		// δ����
		m_listDodgedTarget.push_back(dwTargetID);

		if (m_pOwner->IsRole())
		{
			((Role*)m_pOwner)->GetAchievementMgr().UpdateAchievementCriteria(eta_combat_miss,1);
		}
		if (pTarget->IsRole())
		{
			((Role*)pTarget)->GetAchievementMgr().UpdateAchievementCriteria(eta_combat_dodge,1);
		}
	}
	else
	{
		if(m_pOwner->IsRole())
		{
			//((Role*)m_pOwner)->KaiGuang(pTarget);
		}
		// ����
		m_listHitedTarget.push_back(dwTargetID);
		dwTargetEffectFlag |= ETEF_Hited;
		
		//lc ������Ϣ�ĵ��˺��������
		//NET_SIS_hit_target send;
		//send.dw_role_id = pTarget->GetID();
		//send.dwSrcRoleID = m_pOwner->GetID();
		//send.eCause = EHTC_Skill;
		//send.dwMisc = pSkill->GetTypeID();
		//send.dwSerial = m_dwSkillSerial;

		//if( VALID_POINT(pTarget->get_map()) )
		//{
		//	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		//}

		// �����
		//BOOL bBlocked = CalculateBlock(pSkill, pTarget);
		//if( TRUE == bBlocked )
		//{
		//	// ����
		//	m_listBlockedTarget.push_back(dwTargetID);
		//	dwTargetEffectFlag |= ETEF_Block;
		//}
		//else
		//{
			// ���㱩��
			BOOL bCrit = CalculateCritRate(pSkill, pTarget);
			if( TRUE == bCrit )
			{
				m_listCritedTarget.push_back(dwTargetID);
				dwTargetEffectFlag |= ETEF_Crited;

				if (m_pOwner->IsRole())
				{
					((Role*)m_pOwner)->GetAchievementMgr().UpdateAchievementCriteria(eta_combat_crit, 1);
				}
			}
		//}
	}

	// ���������ı���������
	pTarget->OnBeAttacked(m_pOwner, pSkill,
		dwTargetEffectFlag & ETEF_Hited, dwTargetEffectFlag & ETEF_Block, dwTargetEffectFlag & ETEF_Crited);

	return dwTargetEffectFlag;
}


//--------------------------------------------------------------------------------
// ��������
//--------------------------------------------------------------------------------
BOOL CombatHandler::CalculateHit(Skill* pSkill, Unit* pTarget)
{
	
	double fHit = 0.0f;
	double fShanbi = 0.0f;

	////����
	int nCrit = m_pOwner->GetAttValue(ERA_HitRate);
	if (nCrit < 3500)
	{
		fHit = nCrit * 2;
	}
	else
	{
		fHit = 3500 * 2 + (nCrit - 3500) * 0.01;
	}

	int nFanCrit = pTarget->GetAttValue(ERA_Dodge);
	if (nFanCrit < 3500)
	{
		fShanbi = nFanCrit * 2;
	}
	else
	{
		fShanbi = 3500 * 2 + (nFanCrit - 3500) * 0.01;
	}

	////����=����*(1-������)
	fHit = 10000 + fHit - fShanbi;

	
	// ��Χ��0����100%
	if( fHit < 0.0f ) fHit = 0.0f;
	if( fHit > 10000.0f ) fHit = 10000.0f;

	// ������Ƿ�������
	return get_tool()->probability(INT(fHit/100.0f));
}

//----------------------------------------------------------------------------
// �����
//----------------------------------------------------------------------------
BOOL CombatHandler::CalculateBlock(Skill* pSkill, Unit* pTarget)
{
	// ֻ�й����Ŵ��ڷ�����ǰ��ʱ���������ſ��Ը�
	if( FALSE == m_pOwner->IsInFrontOfTarget(*pTarget) )
		return FALSE;

	//�м���=�м�/(�м�+�ȼ�*20+180)
	FLOAT fBlock = 0.0f;
	
	INT nBlock = pTarget->GetAttValue(ERA_ShenAttack);

	fBlock = nBlock * 1.0f / (nBlock + pTarget->get_level()*20 + 180);

	//// �⹦����
	//if( pSkill->IsExAttackSkill() )
	//{
	//	// Զ�̹���
	//	if( pSkill->IsMelee() )
	//	{
	//		// ��������=����������ǰ�⹦����-���������⹦����+�������ڹ�������/4��/30000
	//		// ����=[1+����������ǰ��������-��������ǰ�������ɣ�/6000] ����������+�������񵵼��ʼӳ�
	//		FLOAT fBaseBlock = (FLOAT(pTarget->GetAttValue(ERA_ExDefense)) - FLOAT(m_pOwner->GetAttValue(ERA_ExAttack) + m_pOwner->GetAttValue(ERA_InAttack)) / 4.0f) / 30000.0f;
	//		fBlock = (1.0f + FLOAT(pTarget->GetAttValue(ERA_DefenseTec) - m_pOwner->GetAttValue(ERA_AttackTec)) / 6000.0f) * fBaseBlock + (FLOAT)pTarget->GetAttValue(ERA_Block_Rate) / 10000.0f;
	//	}

	//	// Զ�̹���
	//	else if( pSkill->IsRanged() )
	//	{
	//		// ����=0
	//		fBlock = 0.0f;
	//	}		
	//}

	//// �ڹ�����
	//else if( pSkill->IsInAttackSkill() )
	//{
	//	// ��������=����������ǰ�ڹ�����-���������⹦����+�������ڹ�������/4��/30000
	//	// ����=[1+����������ǰ��������-��������ǰ�������ɣ�/6600] ����������+�������񵵼��ʼӳ�
	//	FLOAT fBaseBlock = (FLOAT(pTarget->GetAttValue(ERA_InDefense)) - FLOAT(m_pOwner->GetAttValue(ERA_ExAttack) + m_pOwner->GetAttValue(ERA_InAttack)) / 4.0f) / 30000.0f;
	//	fBlock = (1.0f + FLOAT(pTarget->GetAttValue(ERA_DefenseTec) - m_pOwner->GetAttValue(ERA_AttackTec)) / 6600.0f) * fBaseBlock + (FLOAT)pTarget->GetAttValue(ERA_Block_Rate) / 10000.0f;
	//}

	//// ��������
	//// 	else if( pSkill->IsStuntSkill() )
	//// 	{
	//// 		fBlock = 0.0f;
	//// 	}

	//// else
	//else
	//{

	//}

	// ��Χ��0����100%
	if( fBlock < 0.0f ) fBlock = 0.0f;
	if( fBlock > 1.0f ) fBlock = 1.0f;

	// ������Ƿ�������
	return get_tool()->probability(INT(fBlock*100.0f));
}

//-----------------------------------------------------------------------------
// ����������
//-----------------------------------------------------------------------------
BOOL CombatHandler::CalculateCritRate(Skill* pSkill, Unit* pTarget)
{
	double fCrit = 0.0f;
	double fFanCirt = 0.0f;
	//double fFanCrit = 0.0f;
	////������=����/(����+�ȼ�20+180)
	int nCrit = m_pOwner->GetAttValue(ERA_Crit_Rate);
	if (nCrit < 3500)
	{
		fCrit = nCrit * 2;
	}
	else
	{
		fCrit = 3500 * 2 + (nCrit - 3500) * 0.01;
	}
	
	int nFanCrit = pTarget->GetAttValue(ERA_UnCrit_Rate);
	if (nFanCrit < 3500)
	{
		fFanCirt = nFanCrit * 2;
	}
	else
	{
		fFanCirt = 3500 * 2 + (nFanCrit - 3500) * 0.01;
	}

	////������=����*(1-������)
	fCrit = fCrit - fFanCirt;

	// ��Χ��0����100%
	if( fCrit < 0.0f ) fCrit = 0.0f;
	if( fCrit > 10000.0f ) fCrit = 10000.0f;

	// ������Ƿ�������
	return get_tool()->probability(INT(fCrit/100));
}

//-----------------------------------------------------------------------------
// ����������
//-----------------------------------------------------------------------------
FLOAT CombatHandler::CalculateCritAmount(Skill* pSkill, Unit* pTarget)
{
	//������=����/(����+�ȼ�*10+90)
	double fCritAmount = 1.5f;
	//double nCritAmount = m_pOwner->GetAttValue(ERA_Crit_Amount);
	////fCritAmount = nCritAmount*1.0f / ( nCritAmount + m_pOwner->get_level()*10 + 90);
	////����/(����+140*�ȼ�^(�ȼ�/100)+400+150^(MAX(1,(�ȼ�/42))))
	//fCritAmount = nCritAmount/(nCritAmount+140.0*pow(m_pOwner->get_level(), m_pOwner->get_level()/100.0)+400+pow(150.0, max(1, m_pOwner->get_level()/42.0)));

	////������=����/(����+�ȼ�*10+90)
	//double fFanCritAmount = 0.0f;
	//double nFanCritAmount = pTarget->GetAttValue(ERA_UnCrit_Amount);
	////fFanCritAmount = nFanCritAmount * 1.0f/ (nFanCritAmount + pTarget->get_level()*10 + 90);
	////����*(MAX(41,�ȼ�)/65)/(����+�ȼ�*150^(�ȼ�/70))
	//fFanCritAmount =  nFanCritAmount*(max(41.0, m_pOwner->get_level())/65.0)/(nFanCritAmount+m_pOwner->get_level()*pow(150.0, m_pOwner->get_level()/70.0));


	////�����˺�=2+����-������
	//fCritAmount = 2+fCritAmount-fFanCritAmount;

	// ��Χ��0����100%
	//if( fCritAmount < 1.25f ) fCritAmount = 1.25f;
	//if( fCritAmount > 2.25f ) fCritAmount = 2.25f;

	return (FLOAT)fCritAmount;
}

//-----------------------------------------------------------------------------
// ���㼼���˺�
//-----------------------------------------------------------------------------
VOID CombatHandler::CalculateDmg(Skill* pSkill, Unit* pTarget)
{
	// Ŀ���Ѿ�������ֱ�ӷ���
	if( pTarget->IsDead() ) return;

	DWORD dwTargetID = pTarget->GetID();

	// ��������
	bool bCrit = false;
	bool bBlock = false;
	FLOAT fCrit = 1.0f;

	// �����жϸ�Ŀ���Ƿ�������
	if( m_listDodgedTarget.is_exist(dwTargetID) )
	{
		// ����δ������Ϣ
		NET_SIS_role_hp_change send;
		send.dw_role_id = pTarget->GetID();
		send.dwSrcRoleID = m_pOwner->GetID();
		send.eCause = ERHPCC_SkillDamage;
		send.bMiss = true;
		send.nHPChange = 0;
		send.dwMisc = pSkill->GetTypeID();
		send.dwMisc2 = m_nSkillCurDmgIndex;
		send.dwSerial = m_dwSkillSerial;

		if( VALID_POINT(pTarget->get_map()) )
		{
			pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		}
		return;
	}
	else
	{
		NET_SIS_hit_target send;
		send.dw_role_id = pTarget->GetID();
		send.dwSrcRoleID = m_pOwner->GetID();
		send.eCause = EHTC_Skill;
		send.dwMisc = pSkill->GetTypeID();
		send.dwSerial = m_dwSkillSerial;

		if( VALID_POINT(pTarget->get_map()) )
		{
			pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		}

	}
	// ���ж��Ƿ��м���
	//if( m_listBlockedTarget.is_exist(dwTargetID) )
	//{
	//	bBlock = true;
		// �����м���Ϣ
		//NET_SIS_role_hp_change send;
		//send.dw_role_id = pTarget->GetID();
		//send.dwSrcRoleID = m_pOwner->GetID();
		//send.eCause = ERHPCC_SkillDamage;
		//send.bBlocked = true;
		//send.dwMisc = pSkill->GetTypeID();
		//send.dwMisc2 = m_nSkillCurDmgIndex;
		//send.dwSerial = m_dwSkillSerial;

		//if( VALID_POINT(pTarget->get_map()) )
		//{
		//	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		//}
		//return;
	//}

	// ���ж��Ƿ񱩻�
	if( m_listCritedTarget.is_exist(dwTargetID) )
	{
		// ���㱩������
		bCrit = true;
		fCrit = CalculateCritAmount(pSkill, pTarget);
	}

	FLOAT fBaseDmg		=	CalBaseDmg(pSkill, pTarget);			// �����˺�
	//FLOAT fAttDefCoef	=	CalAttackDefenceCoef(pSkill, pTarget);	// ���׼���
	FLOAT fDerateCoef	=	CalDerateCoef(pSkill, pTarget);			// ����Ӱ��
	//FLOAT fLevelCoef	=	CalLevelCoef(pSkill, pTarget);			// �ȼ�Ӱ��

	// �����˺�
	FLOAT fDmg = fBaseDmg * fDerateCoef/** fAttDefCoef * fMoraleCoef  * fInjuryCoef* fLevelCoef*/ ;
	fDmg = fDmg + (FLOAT)m_pOwner->GetAttValue(ERA_ExDamage) - (FLOAT)pTarget->GetAttValue(ERA_ExDamage_Absorb);
	


	// ���ܽű�
	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		fDmg = pScript->CalculateDmg(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID(), bBlock, bCrit, fDmg);
	}

	// ���㱩������
	INT nDmg = INT(fDmg * fCrit);
	
	if( nDmg < 1 ) nDmg = 1;
	
	// ��Ѫ
	pTarget->ChangeHP(-nDmg, m_pOwner, pSkill, NULL, bCrit, bBlock, m_dwSkillSerial, m_nSkillCurDmgIndex);
	//OnDmg(pTarget, nDmg, pTarget->get_level() - m_pOwner->get_level(), bCrit);

	// ���������
	if(pTarget->IsCreature())
	{
		if(!((Creature*)pTarget)->IsDead())
		{
			AIController* pAI = ((Creature*)pTarget)->GetAI();
			if( VALID_POINT(pAI) )
			{
				INT	nValue = pSkill->GetEnmity() + m_pOwner->GetAttValue(ERA_Enmity_Degree);
				// �����˺���޶�
				nValue += (nDmg /** pSkill->GetEnmityParam()*/);
				pAI->AddEnmity(m_pOwner, nValue);
				pAI->OnEvent(m_pOwner, ETEE_BeAttacked);
			}
		}
	}
	

	// ��ɱĿ��ű�
	if (pTarget->IsDead())
	{
		if (VALID_POINT(pScript))
		{
			pScript->KillUnit(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID());
		}
	}
}

VOID CombatHandler::CalculateDmgNoSpe(Skill* pSkill, Unit* pTarget)
{
	// Ŀ���Ѿ�������ֱ�ӷ���
	if( pTarget->IsDead() ) return;

	DWORD dwTargetID = pTarget->GetID();

	// ��������
	bool bCrit = false;
	bool bBlock = false;
	FLOAT fCrit = 1.0f;

	//// �����жϸ�Ŀ���Ƿ�������
	//if( m_listDodgedTarget.is_exist(dwTargetID) )
	//{
	//	// ����δ������Ϣ
	//	NET_SIS_role_hp_change send;
	//	send.dw_role_id = pTarget->GetID();
	//	send.dwSrcRoleID = m_pOwner->GetID();
	//	send.eCause = ERHPCC_SkillDamage;
	//	send.bMiss = true;
	//	send.nHPChange = 0;
	//	send.dwMisc = pSkill->GetTypeID();
	//	send.dwMisc2 = m_nSkillCurDmgIndex;
	//	send.dwSerial = m_dwSkillSerial;

	//	if( VALID_POINT(pTarget->get_map()) )
	//	{
	//		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
	//	}
	//	return;
	//}
	//else
	//{
	//	NET_SIS_hit_target send;
	//	send.dw_role_id = pTarget->GetID();
	//	send.dwSrcRoleID = m_pOwner->GetID();
	//	send.eCause = EHTC_Skill;
	//	send.dwMisc = pSkill->GetTypeID();
	//	send.dwSerial = m_dwSkillSerial;

	//	if( VALID_POINT(pTarget->get_map()) )
	//	{
	//		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
	//	}

	//}

	//// ���ж��Ƿ񱩻�
	//if( m_listCritedTarget.is_exist(dwTargetID) )
	//{
	//	// ���㱩������
	//	bCrit = true;
	//	fCrit = CalculateCritAmount(pSkill, pTarget);
	//}

	FLOAT fBaseDmg		=	CalBaseDmg(pSkill, pTarget);			// �����˺�
	//FLOAT fAttDefCoef	=	CalAttackDefenceCoef(pSkill, pTarget);	// ���׼���
	FLOAT fDerateCoef	=	CalDerateCoef(pSkill, pTarget);			// ����Ӱ��
	//FLOAT fLevelCoef	=	CalLevelCoef(pSkill, pTarget);			// �ȼ�Ӱ��

	// �����˺�
	FLOAT fDmg = fBaseDmg * fDerateCoef/** fAttDefCoef * fMoraleCoef  * fInjuryCoef* fLevelCoef*/ ;
	fDmg = fDmg + (FLOAT)m_pOwner->GetAttValue(ERA_ExDamage) - (FLOAT)pTarget->GetAttValue(ERA_ExDamage_Absorb);



	// ���ܽű�
	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		fDmg = pScript->CalculateDmg(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID(), bBlock, bCrit, fDmg);
	}

	// ���㱩������
	INT nDmg = INT(fDmg * fCrit);

	if( nDmg < 1 ) nDmg = 1;

	// ��Ѫ
	pTarget->ChangeHP(-nDmg, m_pOwner, pSkill, NULL, bCrit, bBlock, m_dwSkillSerial, m_nSkillCurDmgIndex);
	//OnDmg(pTarget, nDmg, pTarget->get_level() - m_pOwner->get_level(), bCrit);

	// ���������
	if(pTarget->IsCreature())
	{
		if(!((Creature*)pTarget)->IsDead())
		{
			AIController* pAI = ((Creature*)pTarget)->GetAI();
			if( VALID_POINT(pAI) )
			{
				INT	nValue = pSkill->GetEnmity() + m_pOwner->GetAttValue(ERA_Enmity_Degree);
				// �����˺���޶�
				nValue += (nDmg * pSkill->GetEnmityParam());
				pAI->AddEnmity(m_pOwner, nValue);
				pAI->OnEvent(m_pOwner, ETEE_BeAttacked);
			}
		}
	}

	// ��ɱĿ��ű�
	if (pTarget->IsDead())
	{
		if (VALID_POINT(pScript))
		{
			pScript->KillUnit(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID());
		}
	}
}

VOID CombatHandler::ClearPilotList()
{
	std::map<DWORD, tagPilotUnit*>::iterator it = m_listPilotUnit.begin();
	while(it != m_listPilotUnit.end())
	{
		tagPilotUnit* pUnit = it->second;


		Map* pMap = g_mapCreator.get_map(pUnit->dwMapID, pUnit->dwInstanceID);
		Creature* pCreature = NULL;
		if( VALID_POINT(pMap) )
		{
			pCreature = pMap->find_creature(pUnit->dwCretureID);
		}
		
		if (VALID_POINT(pCreature))
		{
			pCreature->OnDisappear();
		}

		SAFE_DELETE(it->second);
		it->second = NULL;
		it++;
	}

	m_listPilotUnit.clear();
}
//------------------------------------------------------------------------------
// ��������˺�
//------------------------------------------------------------------------------
inline FLOAT CombatHandler::CalBaseDmg(Skill* pSkill, Unit* pTarget)
{
	FLOAT	fBaseDmg	=	1.0f;

	INT		nSkillDmg = 0;
	INT		nSkillDmgMod = 0;
	INT		nSKillDmgModPct = 0;
	
	nSkillDmg =	pSkill->GetDmg(m_nSkillCurDmgIndex);
	nSkillDmgMod = pSkill->GetDmgMod(m_nSkillCurDmgIndex);
	nSKillDmgModPct = pSkill->GetDmgModPct(m_nSkillCurDmgIndex);
	
	ESkillTypeEx eSkillTypeEx = pSkill->GetTypeEx();
	INT nMinAttack = 0;
	INT nMaxAttack = 0;
	INT nMinDef = 0;
	INT nMaxDef = 0;
	switch (eSkillTypeEx)
	{
	case ESSTE_Sword:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_ExAttackMin);
			nMaxAttack = m_pOwner->GetAttValue(ERA_ExAttackMax);
			nMinDef = pTarget->GetAttValue(ERA_ExAttack);
			nMaxDef = pTarget->GetAttValue(ERA_ExDefense);
		}
		break;
	case ESSTE_Blade:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_InAttackMin);
			nMaxAttack = m_pOwner->GetAttValue(ERA_InAttackMax);
			nMinDef = pTarget->GetAttValue(ERA_InAttack);
			nMaxDef = pTarget->GetAttValue(ERA_InDefense);
		}
		break;
	case ESSTE_Wand:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_ArmorEx);
			nMaxAttack = m_pOwner->GetAttValue(ERA_ArmorIn);
			nMinDef = pTarget->GetAttValue(ERA_InAttack);
			nMaxDef = pTarget->GetAttValue(ERA_InDefense);
		}
		break;
	default:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_ExAttackMin);
			nMaxAttack = m_pOwner->GetAttValue(ERA_ExAttackMax);
			nMinDef = pTarget->GetAttValue(ERA_ExAttack);
			nMaxDef = pTarget->GetAttValue(ERA_ExDefense);
		}
		break;
	}

	INT	nExAttack = 0;
	if (m_pOwner->GetAttValue(ERA_Luck) >= 9)
	{
		nExAttack = nMaxAttack;
	}
	else if (m_pOwner->GetAttValue(ERA_Luck) <= -9)
	{
		nExAttack = nMinAttack;
	}
	else
	{
		nExAttack = get_tool()->rand_in_range(nMinAttack, nMaxAttack);
	}

	INT nExDef = get_tool()->rand_in_range(nMinDef, nMaxDef);

	FLOAT fExAttack = nExAttack - nExDef;
	if( nSkillDmg > 100000 )
	{
		// ȡ�����˺��ı���
		fBaseDmg = fExAttack * (FLOAT(nSkillDmg - 100000) / 10000.0f);
	}
	else
	{
		// ȡ���Ǽ����˺�
		fBaseDmg =  fExAttack + (FLOAT)nSkillDmg;
	}
	
	fBaseDmg = fBaseDmg + fBaseDmg*nSKillDmgModPct/10000.0f + nSkillDmgMod;


	return get_tool()->fluctuate(fBaseDmg, 5, 5);
}

//-----------------------------------------------------------------------------
// ���㹥��Ӱ��
//-----------------------------------------------------------------------------
inline FLOAT CombatHandler::CalAttackDefenceCoef(Skill* pSkill, Unit* pTarget)
{
	//��������=����/������+�ȼ�*50+60��
	INT nDef = pTarget->GetAttValue(ERA_ExDefense) /*+ pTarget->GetAttValue(ERA_ArmorEx)*/;


	FLOAT fCoef = nDef * 1.0f / (nDef + pTarget->get_level() * 50+ 60);

	return (1 - fCoef);
}


//------------------------------------------------------------------------
// �������Ӱ��
//------------------------------------------------------------------------
inline FLOAT CombatHandler::CalDerateCoef(Skill* pSkill, Unit* pTarget)
{
	FLOAT fDerateCoef = 1.0f;
	

	//�˺����� = Ŀ�깥������+�����˺�����+��Ӧ�����˺�����
	fDerateCoef =  FLOAT( pTarget->GetAttValue(ERA_Derate_ALL) );
	
	fDerateCoef = fDerateCoef / 10000 ;

	// ��������ֵ
	fDerateCoef = 1.0f - fDerateCoef;
	if( fDerateCoef < 0.1f ) fDerateCoef = 0.1f;
	if( fDerateCoef > 1.0f ) fDerateCoef = 1.0f;

	return fDerateCoef;
}


//-------------------------------------------------------------------------
// ����ȼ�Ӱ��
//-------------------------------------------------------------------------
inline FLOAT CombatHandler::CalLevelCoef(Skill* pSkill, Unit* pTarget)
{
	// �⹦�������ڹ�����
	// 1-���������ȼ�-�������ȼ���/75     ȡֵ��0.2~1.8��
	INT nLevelSub = pTarget->get_level() - m_pOwner->get_level();
	//if( pSkill->IsExAttackSkill() || pSkill->IsInAttackSkill() )
	if( nLevelSub > 0)
	{
		FLOAT fDmgCof = 0.0f;
		if (nLevelSub > 20)
		{
			fDmgCof = (FLOAT)(31-(fDmgCof-50)*(fDmgCof-50)/200)/100.0f;		
		}
		else if (nLevelSub > 10)
		{
			fDmgCof = (FLOAT)(nLevelSub*nLevelSub/17+nLevelSub/2-6.8)/100.0f;
		}
		else
		{
			fDmgCof = (FLOAT)(nLevelSub * nLevelSub /25 + 1)/100.0f;
		}
		FLOAT fCoef = 1.0f - fDmgCof;
		if( fCoef < 0.7f ) fCoef = 0.7f;
		if( fCoef > 1.0f ) fCoef = 1.0f;

		return fCoef;
	}
	else
	{
		return 1.0f;
	}
}

//----------------------------------------------------------------------------
// ���㼼������
//----------------------------------------------------------------------------
VOID CombatHandler::CalculateCost(Skill* pSkill)
{
	// ��������
	INT nHPCost = pSkill->GetCost(ESCT_HP);
	if( nHPCost > 0  )
	{
		m_pOwner->ChangeHP(-nHPCost, m_pOwner);
	}

	// ��������
	INT nMPCost = pSkill->GetCost(ESCT_MP);
	if( nMPCost > 0  )
	{
		m_pOwner->ChangeMP(-nMPCost);
	}

	// ŭ������
	//INT nRageCost = pSkill->GetCost(ESCT_Rage);
	//if( nRageCost > 0 )
	//{
	//	m_pOwner->ChangeRage(-nRageCost);
	//}


	// �־�����
	//INT nEnduranceCost = pSkill->GetCost(ESCT_Endurance);
	//if( nEnduranceCost > 0 )
	//{
	//	m_pOwner->ChangeEndurance(-nEnduranceCost);
	//}

	// ��������
	//INT nValicityCost = pSkill->GetCost(ESCT_Valicity);
	//if( nValicityCost > 0  )
	//{
	//	m_pOwner->ChangeVitality(-nValicityCost);
	//}
}

//----------------------------------------------------------------------------
// ��Ʒʹ���ж�
//----------------------------------------------------------------------------
INT	CombatHandler::can_use_item(tagItem *pItem)
{
	if( !VALID_POINT(pItem)  )
		return E_SystemError;

	if( CheckItemConflict(pItem) ) return E_UseItem_Operating;

	INT nRet = E_Success;

	nRet = CheckItemAbility(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckOwnerLimitItem();
	if( E_Success != nRet ) return nRet;

	nRet = CheckRoleProtoLimit(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckRoleStateLimit(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckRoleVocationLimit(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckMapLimit(pItem);
	if( E_Success != nRet ) return nRet;

	return nRet;
}


//----------------------------------------------------------------------------
// �����Ʒ����
//----------------------------------------------------------------------------
INT	CombatHandler::CheckItemAbility(tagItem *pItem)
{
	if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;

	// ��Ʒ�Ƿ��ǿ�ʹ����Ʒ
	if(MIsEquipment(pItem->dw_data_id) || pItem->pProtoType->dwBuffID0 == INVALID_VALUE)
		return E_UseItem_ItemCanNotUse;

	// ��Ʒ����ȴʱ�仹û�����򲻿���ʹ��
	if(((Role*)m_pOwner)->GetItemMgr().IsItemCDTime(pItem->dw_data_id))
		return E_UseItem_CoolDowning;

	return E_Success;
}

//----------------------------------------------------------------------------
// ���ʹ���߱���
//----------------------------------------------------------------------------
INT CombatHandler::CheckOwnerLimitItem()
{
	// �Ƿ��ڲ���ʹ�ü��ܵ�״̬
	if( m_pOwner->IsInStateCantCastSkill() )
		return E_UseItem_UseLimit;

	if (m_pOwner->IsInState(ES_Dizzy))
		return E_UseItem_UseLimit;

	if( IsRide() ) 
		return E_UseItem_UseLimit;

	return E_Success;
}

//----------------------------------------------------------------------------
// ���������������
//----------------------------------------------------------------------------
INT CombatHandler::CheckRoleProtoLimit(tagItem *pItem)
{
	if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;

	// �Ա�����
	if( pItem->pProtoType->eSexLimit != ESL_Null )
	{
		if( ESL_Man == pItem->pProtoType->eSexLimit )
		{
			if( 1 != m_pOwner->GetSex() )
				return E_UseItem_SexLimit;
		}
		else if( ESL_Woman == pItem->pProtoType->eSexLimit )
		{
			if( 0 != m_pOwner->GetSex() )
				return E_UseItem_SexLimit;
		}
		else
		{

		}
	}

	// �ȼ�����
	if(pItem->pProtoType->byMinUseLevel > m_pOwner->get_level() 
		|| pItem->pProtoType->byMaxUseLevel < m_pOwner->get_level())
		return E_UseItem_LevelLimit;

	// ְҵ����

	return E_Success;
}


//----------------------------------------------------------------------------
// �������״̬����
//----------------------------------------------------------------------------
INT CombatHandler::CheckRoleStateLimit(tagItem *pItem)
{
	// ����״̬���ƣ����� ��ѣ�Σ�
	DWORD dwSelfStateFlag = m_pOwner->GetStateFlag();

	if( (dwSelfStateFlag & pItem->pProtoType->dwStateLimit) != dwSelfStateFlag )
	{
		return E_UseItem_SelfStateLimit;
	}

	// ����ڻ�е�����

	// ��������
	return E_Success;
}

//----------------------------------------------------------------------------
// �������ְҵ����
//----------------------------------------------------------------------------
INT CombatHandler::CheckRoleVocationLimit(tagItem *pItem)
{
	if(!VALID_POINT(pItem)) return E_UseItem_ItemNotExist;

	if(!m_pOwner->IsRole()) return E_Success;

	//INT nClass = (INT)((Role*)m_pOwner)->GetClass();
	//INT nClassEx = (INT)((Role*)m_pOwner)->GetClassEx();
	INT nClass = (INT)(static_cast<Role*> (m_pOwner)->GetClass());
	INT nClassEx = (INT)(static_cast<Role*> (m_pOwner)->GetClassEx());

	/*if(pItem->pProtoType->dwVocationLimit != 5)
	{
	if(pItem->pProtoType->dwVocationLimit != nClass)
	return E_UseItem_VocationLimit;
	}*/

	/*if (((nClassEx << 9) + nClass) & pItem->pProtoType->dwVocationLimit)
	return E_Success;
	else
	return E_UseItem_VocationLimit;*/

	return E_Success;
}

//----------------------------------------------------------------------------
// ����ͼ����
//----------------------------------------------------------------------------
INT CombatHandler::CheckMapLimit(tagItem* pItem)
{
	// �жϵ�ͼ����
	if(VALID_POINT(m_pOwner->get_map()))
	{
		BOOL bUesAble = m_pOwner->get_map()->can_use_item(pItem->dw_data_id, pItem->n64_serial);
		if( !bUesAble )	return E_UseItem_MapLimit;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
// ������Ʒʹ�ó�ͻ�������ͻ��ΪTRUE������ͻΪFALSE
//-------------------------------------------------------------------------------
BOOL CombatHandler::CheckItemConflict(tagItem* pItem)
{
	if( IsUseItem() ) return TRUE;	// ��ǰ����ʹ����Ʒ������ʹ��

	if( IsUseSkill() )
	{
		// �����Ʒ��������Ʒ���򲻿���ʹ��
		if( pItem->pProtoType->nPrepareTime > 0 ) return TRUE;
		else return FALSE;
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
// ��ȡbuffӰ����Ŀ�껤��ֵ
//-------------------------------------------------------------------------------
FLOAT CombatHandler::GetTargetArmor(Unit* target, DWORD dwArmorType)
{
	//FLOAT fArmor = (FLOAT)target->GetAttValue(ERA_ArmorIn) * m_fTargetArmorLeftPct;
	//if ( target->IsRole() )
	//{
	//	Role* pRole = (Role*)target;
	//	DWORD dwClass = (DWORD)pRole->GetClass();
	//	if ((dwArmorType == 0 && (dwClass == EV_HuanMo || dwClass == EV_YinWu)) || 
	//		(dwArmorType == 1 && (dwClass == EV_JinGang || dwClass == EV_ShaXing || dwClass == EV_FengYu)))
	//	{
	//		fArmor = fArmor * 0.5f;
	//	}
	//}
	//return fArmor; 
	return 0;
}


VOID CombatHandler::CalSkillTargetList(package_list<DWORD>& targetList, const tagPilotUnit* pPilotUnit, Vector3 vPosDes)
{
	Skill* pSkill = m_pOwner->GetSkill(pPilotUnit->dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	
	FLOAT fOPRadius = pPilotUnit->fOPRadius;
	FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;

		// ���������ΧΪ0����ֱ�ӷ���
		if( 0.0f == fOPRadius )
			return;

		targetList.clear();

		tagVisTile* pVisTile[EUD_end] = {0};

		// �õ�������Χ�ڵ�vistile�б�
		m_pOwner->get_map()->get_visible_tile(vPosDes, fOPRadius, pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// ���ȼ������
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// ��Ŀ��һ������������
				//if( pRole == pTarget || pRole == m_pOwner ) continue;

				// ���ܾ����ж�
				FLOAT fDistSQ = Vec3DistSq(vPosDes, pRole->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;


				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole ) )
					continue;

				CheckInCombat(pRole);
				// ���߼��

				// �ж�ͨ��������Ҽ��뵽�б���
				targetList.push_back(pRole->GetID());

				// ���㼼�����ý��
				//CalculateSkillEffect(pSkill, pRole);
				((Unit*)pRole)->OnBeAttacked(m_pOwner, pSkill, TRUE, FALSE, FALSE);

			}

			// �ټ������
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// ��Ŀ��һ������������
				if( pCreature == m_pOwner ) continue;

				// ���ܾ����ж�
				FLOAT fDistSQ = Vec3DistSq(vPosDes, pCreature->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;

				// Ŀ����������ж�
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;

				CheckInCombat(pCreature);

				// �ж�ͨ������������뵽�б���
				targetList.push_back(pCreature->GetID());

				// ���㼼�����ý��
				//CalculateSkillEffect(pSkill, pCreature);
				((Unit*)pCreature)->OnBeAttacked(m_pOwner, pSkill, TRUE, FALSE, FALSE);

			}
		}
	


}
//-----------------------------------------------------------------------------
// ���³��������ͼ����˺�
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdatePrepareUnit()
{
	std::map<DWORD, tagPilotUnit*>::iterator iter = m_listPilotUnit.begin();
	while(iter != m_listPilotUnit.end())
	{
		tagPilotUnit* pUnit = iter->second;

		Map* pMap = g_mapCreator.get_map(pUnit->dwMapID, pUnit->dwInstanceID);
		Creature* pCreature = NULL;
		if( VALID_POINT(pMap) )
		{
			pCreature = pMap->find_creature(pUnit->dwCretureID);
		}

		// ʱ�䵽�ˣ�ɾ�����������
		if (pUnit->dwOverTime <= 0 || !VALID_POINT(pCreature))
		{

			if (VALID_POINT(pCreature))
			{
				pCreature->OnDisappear();
			}

			m_listPilotUnit.erase(iter++);
			SAFE_DELETE(pUnit);
		}
		else
		{
			package_list<DWORD>	listTargetID;				// ����Ŀ���б�
			CalSkillTargetList(listTargetID, pUnit, pCreature->GetCurPos());
			
			package_list<DWORD>::list_iter it = listTargetID.begin();
			DWORD dwTargetID = INVALID_VALUE;
			
			Skill* pSkill = m_pOwner->GetSkill(pUnit->dwSkillID);
			if( !VALID_POINT(pSkill) ) return;


			while( listTargetID.find_next(it, dwTargetID) )
			{
				// �ҵ����Ŀ��
				Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetID);

				if( !VALID_POINT(pTarget) || pTarget == m_pOwner || pCreature == pTarget) continue;

				// �����˺�
				CalculateDmgNoSpe(pSkill, pTarget);
			}

			pUnit->dwOverTime--;
			iter++;
		}
		
	}
	
}
//-----------------------------------------------------------------------------
// ��������Ч��
//-----------------------------------------------------------------------------
//VOID CombatHandler::OnHit(Skill* pSkill, INT32 nlevelSub, BOOL bCrit)
//{
//	//������Ҳ�����
//	if (!m_pOwner->IsRole()) return;
//
//	Role* pRole = (Role*)m_pOwner;
//	switch (pRole->GetClass())
//	{
//	case EV_Warrior://�ͽ�
//		{
//			//���������������ͼ��ܲ�����
//			if (pSkill->GetCost(ESCT_MP) == -1) 
//			{
//				FLOAT fAddPower = 10;
//
//				/*if (nlevelSub < -5) 
//					fAddPower = 8.0f;
//				else if (nlevelSub > 5) 
//					fAddPower = 4.0f;
//				else 
//					fAddPower = nlevelSub+8.0f;*/
//				//if (bCrit)
//				//	fAddPower *= 1.5f;
//				pRole->ChangeMP((INT)fAddPower);
//			}
//		
//		}
//		break;
//	case EV_Blader://�ٱ�
//		{
//			//�����������ͼ���
//			if (pSkill->GetCost(ESCT_MP) == -1) 
//			{
//				pRole->ChangeMP(1);
//			}
//			//������������
//			else if (pSkill->GetCost(ESCT_MP) == -2)
//			{
//				pRole->ChangeMP(-m_pOwner->GetAttValue(ERA_MP));
//			}
//		}
//		
//		break;
//	case EV_Taoist://����
//		{
//			switch (GetChargeType())
//			{
//			case 0:
//				{
//					//��������
//					if (pSkill->GetCost(ESCT_MP) == -2) 
//					{
//						pRole->ChangeMP(bCrit ? 3:2);
//					}
//	
//				}
//				break;
//			case 1:
//				pRole->SetAttValue(ERA_MP, 5);
//				if (pRole->GetAttValue(ERA_MP) >= pRole->GetAttValue(ERA_MaxMP))
//				{
//					pRole->SetAttValue(ERA_MP, 0);
//				}
//				break;
//			case 2:
//				pRole->SetAttValue(ERA_MP, 9);
//				if (pRole->GetAttValue(ERA_MP) >= pRole->GetAttValue(ERA_MaxMP))
//				{
//					pRole->SetAttValue(ERA_MP, 0);
//				}
//				break;
//			case 3:
//				pRole->SetAttValue(ERA_MP, 11);
//				if (pRole->GetAttValue(ERA_MP) >= pRole->GetAttValue(ERA_MaxMP))
//				{
//					pRole->SetAttValue(ERA_MP, 0);
//				}
//				break;
//			default:
//				break;
//			}
//
//		}
//		break;
//	default:
//		break;
//
//	}
//}



//VOID CombatHandler::OnDmg(Unit* pRole, INT dmg, INT32 nlevelSub, BOOL bCrit)
//{
//	if (dmg <= 0) return;
//	//������Ҳ�����
//	if (!pRole->IsRole()) return;
//		
//	switch (((Role*)pRole)->GetClass())
//	{
//	case EV_Warrior:
//		{
//			//FLOAT fAddPower = 0;
//			//FLOAT fparam = dmg/pRole->GetAttValue(ERA_MaxHP)*1.0f;
//			////�ж��˺��Ƿ񳬹�20%����
//			//if (fparam > 0.2f)
//			//{
//			//	//ŭ��������ͬ����
//			//	fAddPower = fparam * pRole->GetAttValue(ERA_MaxMP);
//			//}
//			//else
//			//{
//			//	if (nlevelSub < -5) 
//			//		fAddPower = 6.0f;
//			//	else if (nlevelSub > 5) 
//			//		fAddPower = 2.0f;
//			//	else 
//			//		fAddPower = nlevelSub+4.0f;
//			//	if (bCrit)
//			//		fAddPower *= 1.5f;
//			//}
//			
//			FLOAT fAddPower = 60;
//			
//			if (bCrit)
//				fAddPower *= 2.0f;
//
//			pRole->ChangeMP((INT)fAddPower);
//		}
//		break;
//	case EV_Taoist:
//		{
//			INT nPower = pRole->GetAttValue(ERA_MP);
//			if (nPower == 0)
//				return;
//			
//			// �����˵ļ���
//			INT nPro = pRole->GetAttValue(ERA_RemoteAttack_DodgeRate);
//			if (nPro > get_tool()->tool_rand() % 10000)
//				return;
//
//			INT subPower = nPower - (bCrit?3:2);
//			if (nPower > 8)
//			{
//				pRole->SetAttValue(ERA_MP, subPower > 9 ? subPower:9);
//			}
//			else if (nPower > 4)
//			{
//				pRole->SetAttValue(ERA_MP, subPower > 5 ? subPower:5);
//			}
//			else
//			{
//				pRole->SetAttValue(ERA_MP, subPower > 1 ? subPower:1);	
//			}
//		}
//		break;
//	default:
//		break;
//
//	}	
//}

//-----------------------------------------------------------------------------
// �����������仯�߼���غ���
//-----------------------------------------------------------------------------
//�õ���������
//DWORD CombatHandler::GetChargeType()
//{
//	INT nPower = m_pOwner->GetAttValue(ERA_MP);
//	
//	INT nChargeValue = 0;
//	if ( nPower>2 && nPower<5 )
//		nChargeValue = 1;
//
//	else if ( nPower>6 && nPower<9 )
//		nChargeValue = 2;
//
//	else if ( nPower == 10 )
//		nChargeValue = 3;
//
//	return nChargeValue;
//}
