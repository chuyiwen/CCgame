/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//NPC����

#include "StdAfx.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "creature_ai.h"

#include "NPCTeam_define.h"

NPCTeam::NPCTeam(DWORD dwID, Creature* pLeader):m_dwID(dwID)
{
	// ����С�Ӷӳ�
	m_mapMemPtr.add(pLeader->GetID(), pLeader);
}

NPCTeam::~NPCTeam()
{
	m_mapMemPtr.clear();
}

//----------------------------------------------------------------------------
// ����С�ӳ�Ա
//----------------------------------------------------------------------------
VOID NPCTeam::AddNPCTeamMem(Creature* pCreature)
{
	m_mapMemPtr.add(pCreature->GetID(), pCreature);
}

//----------------------------------------------------------------------------
// �����Ụ��
//----------------------------------------------------------------------------
VOID NPCTeam::DelGuildSentinel(BYTE byType)
{
	Creature* pCreature = (Creature*)INVALID_VALUE;
	package_map<DWORD, Creature*>::map_iter it = m_mapMemPtr.begin();
	while( m_mapMemPtr.find_next(it, pCreature) )
	{
		if(VALID_POINT(pCreature) && !pCreature->IsTeam())
		{
			if(pCreature->GetProto()->eGuildType == (byType + EGT_Holiness))
			{
				pCreature->OnDisappear();
				m_mapMemPtr.erase(pCreature->GetID());
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// ʹ��Ա����
//-----------------------------------------------------------------------------
VOID NPCTeam::OrderTeammateMove(Creature* pLeader, Vector3& vDes)
{
	Creature* pCreature = (Creature*)INVALID_VALUE;
	Vector3 vNew;
	package_map<DWORD, Creature*>::map_iter it = m_mapMemPtr.begin();

	while( m_mapMemPtr.find_next(it, pCreature) )
	{
		if(!VALID_POINT(pCreature) || pCreature->IsDead())
			continue;

		// �ȼ�����ӳ��³������ʼ����ļнǵ�cosֵ��sinֵ
		FLOAT fFaceX = (FLOAT)(vDes.x - pLeader->GetCurPos().x);
		FLOAT fFaceZ = (FLOAT)(vDes.z - pLeader->GetCurPos().z);
		FLOAT fOriFaceX = pLeader->GetFaceTo().x;
		FLOAT fOriFaceZ = pLeader->GetFaceTo().z;
		FLOAT fFaceLen = get_fast_code()->fast_square_root(fFaceX*fFaceX+fFaceZ*fFaceZ);
		FLOAT fOriFaceLen = get_fast_code()->fast_square_root(fOriFaceX*fOriFaceX+fOriFaceZ*fOriFaceZ);
		FLOAT fCos = (fFaceX*fOriFaceX + fFaceZ*fOriFaceZ) / fFaceLen / fOriFaceLen; 
		FLOAT fSin = (fOriFaceX*fFaceZ - fFaceX*fOriFaceZ) / fFaceLen / fOriFaceLen;

		// ����ת
		FLOAT fX = pCreature->GetBornPos().x - pLeader->GetBornPos().x;
		FLOAT fZ = pCreature->GetBornPos().z - pLeader->GetBornPos().z;
		vNew.x = fX*fCos-fZ*fSin;
		vNew.z = fX*fSin+fZ*fCos;

		vNew.x += vDes.x;
		vNew.z += vDes.z;
		vNew.y = vDes.y;

		if( MoveData::IsPosInvalid(vNew))
			return;

		if( MoveData::EMR_Success == pCreature->GetMoveData().StartCreatureWalk(vNew, EMS_CreaturePatrol))
		{
			if( VALID_POINT(pCreature->GetAI()) )
			pCreature->GetAI()->SetIsPatroling(TRUE);
		}		
	}
}

//-----------------------------------------------------------------------------
// С���¼�����
//-----------------------------------------------------------------------------
VOID NPCTeam::OnEvent(ENPCTeamEvent eType, Unit* pUnit, DWORD dwEventMisc1, DWORD dwEventMisc2, DWORD dwEventMisc3)
{
	switch(eType)
	{
	case NTE_SynEnmity:
		{
			OnSynEnmity(pUnit, dwEventMisc1);
		}
	}
}

//-----------------------------------------------------------------------------
// ͬ������С�ӳ��
//-----------------------------------------------------------------------------
VOID NPCTeam::OnSynEnmity(Unit* pUnit, DWORD dwValue)
{
	INT nValue = (INT)dwValue;

	Creature* pCreature = (Creature*)INVALID_VALUE;
	package_map<DWORD, Creature*>::map_iter it = m_mapMemPtr.begin();
	while( m_mapMemPtr.find_next(it, pCreature) )
	{
		if( !VALID_POINT(pCreature) || pCreature->IsDead() || !VALID_POINT(pCreature->GetAI()))
			continue;

		pCreature->GetAI()->AddEnmity(pUnit, nValue, FALSE);
	}
}

//-----------------------------------------------------------------------------
// ���С�Ӷӳ�
//-----------------------------------------------------------------------------
DWORD NPCTeam::GetLeaderID()
{
	DWORD dwID = INVALID_VALUE;
	Creature* pCreature = (Creature*)INVALID_VALUE;
	package_map<DWORD, Creature*>::map_iter it = m_mapMemPtr.begin();
	while( m_mapMemPtr.find_next(it, dwID, pCreature) )
	{
		if(VALID_POINT(pCreature) && pCreature->IsTeam())
		{
			dwID = pCreature->GetID();
			break;
		}
	}
	return dwID;
}

//-----------------------------------------------------------------------------
// ͬ�ӳ�Ա��������һ��ֵ
//-----------------------------------------------------------------------------
Creature* NPCTeam::IsTeamateHPLower(DWORD dwValue)
{
	DWORD dwID = INVALID_VALUE;
	Creature* pCreature = (Creature*)INVALID_VALUE;
	package_map<DWORD, Creature*>::map_iter it = m_mapMemPtr.begin();
	while( m_mapMemPtr.find_next(it, dwID, pCreature) )
	{
		if(VALID_POINT(pCreature) && !pCreature->IsDead())
		{
			INT nMPLimit = (INT)dwValue;

			if( nMPLimit < 100000 )
			{
				 if(pCreature->GetAttValue(ERA_HP) < nMPLimit)
					 return pCreature;
			}
			else
			{
				if( pCreature->GetAttValue(ERA_MaxHP) > 0 )
				{
					if(pCreature->GetAttValue(ERA_HP) * 10000 / pCreature->GetAttValue(ERA_MaxHP) < (nMPLimit - 100000))
						return pCreature;
				}
				else
					return NULL;
			}
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// ͬ�ӳ�Ա��������һ��ֵ
//-----------------------------------------------------------------------------
Creature* NPCTeam::IsTeamateMPLower(DWORD dwValue)
{
	DWORD dwID = INVALID_VALUE;
	Creature* pCreature = (Creature*)INVALID_VALUE;
	package_map<DWORD, Creature*>::map_iter it = m_mapMemPtr.begin();
	while( m_mapMemPtr.find_next(it, dwID, pCreature) )
	{
		if(VALID_POINT(pCreature) && !pCreature->IsDead())
		{
			INT nMPLimit = (INT)dwValue;

			if( nMPLimit < 100000 )
			{
				if(pCreature->GetAttValue(ERA_MP) < nMPLimit)
					return pCreature;
			}
			else
			{
				if( pCreature->GetAttValue(ERA_MaxMP) > 0 )
				{
					if(pCreature->GetAttValue(ERA_MP) * 10000 / pCreature->GetAttValue(ERA_MaxMP) < (nMPLimit - 100000))
						return pCreature;
				}
				else
					return NULL;
			}
		}
	}
	return NULL;
}


