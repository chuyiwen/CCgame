/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//NPCְ�ܴ���

#include "StdAfx.h"
#include "role.h"
#include "creature.h"
#include "map_creator.h"
#include "../../common/WorldDefine/function_npc_protocol.h"
#include "../../common/WorldDefine/func_npc_define.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/WorldDefine/role_att_protocol.h"
//---------------------------------------------------------------------------------
// ����ְ��NPC����
//---------------------------------------------------------------------------------
DWORD Role::ProcDak(DWORD dwNPCID, INT32 nIndex, DWORD dwMapID, DWORD dwMisc, DWORD dwPosID)
{
	// �������Ƿ��ڲ����ƶ�״̬
	if(IsInStateCantMove() || IsInRoleState(ERS_Commerce))
	{
		return E_Role_CantMove;
	}
	
	// ��õ�ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}
	
	DWORD _dwPosID = dwPosID;
	if ( _dwPosID == INVALID_VALUE)
	{
		// �ҵ�NPC�������Ϸ���
		Creature* pNPC = pMap->find_creature(dwNPCID);
		if(!VALID_POINT(pNPC))
		{
			return E_Dak_NPCNotFound;
		}

		if(!pNPC->IsFunctionNPC(EFNPCT_Dak))
		{
			return E_Dak_NPCNotValid;
		}

		if(!pNPC->CheckNPCTalkDistance(this))
		{
			return E_Dak_TooFar;
		}

		_dwPosID = pNPC->GetDakID();
	}


	// �ҵ���վ
	const tagDakProto *pDak = AttRes::GetInstance()->GetDakProto(_dwPosID);
	if(!VALID_POINT(pDak))
	{
		ASSERT(VALID_POINT(pDak));
		return E_Dak_NotExist;
	}

	DWORD dwDestMapID = pDak->dakSite[nIndex].dwMapID;
	//// ���Ŀ�ĵ�
	//if(pDak->dakSite[nIndex].dwMapID != dwMapID)
	//{
	//	return E_Dak_TargetMap_Invalid;
	//}

	// �������Ʒ
	if(pDak->dakSite[nIndex].eCostType != ECCT_Null)
	{
		if(!GetCurMgr().IsEnough(pDak->dakSite[nIndex].eCostType, pDak->dakSite[nIndex].nCostVal))
		{
			return E_Dak_NotEnough_Money;
		}
	}

	// �õ�Ŀ���ͼ�ĵ�����
	const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwDestMapID);
	if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;


	const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pDak->dakSite[nIndex].dwWayPointID);
	if( !VALID_POINT(pList) ) return INVALID_VALUE;

	// ��Ŀ�굼�����б�����ȡһ��������
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	Vector3 vFace;
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

	// ����
	if(!GotoNewMap(dwDestMapID, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, 
		vFace.x, vFace.y, vFace.z, dwMisc))
	{
		return E_Dak_ChangeMap_Failed;
	}

	// �۳�����Ʒ
	if(pDak->dakSite[nIndex].eCostType != ECCT_Null)
	{
		GetCurMgr().DecCurrency(pDak->dakSite[nIndex].eCostType, pDak->dakSite[nIndex].nCostVal, elcid_dak);
	}

	return E_Success;
}

void	Role::setInstancePass(int nIndex)
{
	m_nInstanceData |= (1 << nIndex);
	//��ͻ��˷���Ϣ gx add 2013.8.28
	NET_SIS_set_instance_data send;
	send.dw_role_id = GetID();
	send.nInstanceData = m_nInstanceData;
	SendMessage(&send, send.dw_size);
}

bool	Role::isInstancePass(int nIndex)
{
	return m_nInstanceData & (1 << nIndex);
}

bool	Role::isSaodangIng()
{
	if (m_nInstanceShaodangTime != 0 || m_nShaodangIndex != -1)
		return true;

	return false;
}

void	Role::sendSaoDangData()
{
	NET_SIS_get_saodang_data send;
	send.nIndex = m_nShaodangIndex;
	send.dwBeginTime = m_nInstanceShaodangTime;
	SendMessage(&send, send.dw_size);
}

DWORD	Role::InstanceSaodang(int nIndex)
{
	// �Ƿ��Ѿ�ͨ��
	if (!isInstancePass(nIndex))
		return E_Instance_Saodang_nopass;
	
	// ɨ�������ﵽ����
	if (GetDayClearData(ERDCT_BUYLINGQI) >= 3)
		return E_Instance_Saodang_full;

	// ����ɨ����,�����ٴ�ɨ��
	if (m_nInstanceShaodangTime != 0 || m_nShaodangIndex != -1)
		return E_Instance_Saodang_ing;


	m_nInstanceShaodangTime = GetCurrentDWORDTime();
	m_nShaodangIndex = nIndex;
	ModRoleDayClearDate(ERDCT_BUYLINGQI);
	

	return E_Success;
}

DWORD	Role::InstanceSaodangOver()
{
	// �Ƿ��Ѿ�ͨ��
	if (!isInstancePass(m_nShaodangIndex))
		return INVALID_VALUE;

	// û��ɨ���еĸ���
	if (m_nShaodangIndex == -1 || m_nInstanceShaodangTime == 0)
		return INVALID_VALUE;
		
	DWORD dwPassTime = CalcTimeDiff(GetCurrentDWORDTime(), m_nInstanceShaodangTime);
	if ( dwPassTime < 10 * 60)
		return INVALID_VALUE;

	if (GetScript())
	{
		GetScript()->OnInstanceSaodang(this, m_nShaodangIndex);
	}
	
	m_nInstanceShaodangTime = 0;
	m_nShaodangIndex = -1;

	return E_Success;
}
//------------------------------------------------------------------------------
// ���ø������
//------------------------------------------------------------------------------
DWORD Role::SetRebornMap( DWORD dwNPCID, DWORD &dwBornMapID, Vector3 &vBornPos )
{
	// ��õ�ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	if(pMap->get_map_info()->e_type != EMT_Normal)
	{
		if(pMap->get_map_info()->e_type != EMT_System)
		{
			//ASSERT(pMap->get_map_info()->e_type == EMT_Normal);
			return E_BindRebornMap_MapInvalid;
		}
	}

	// �ҵ�NPC�������Ϸ���
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC))
	{
		return E_BindRebornMap_NPCNotFound;
	}

	if(!pNPC->IsFunctionNPC(EFNPCT_Revive))
	{
		return E_BindRebornMap_NPCInvalid;
	}

	if(!pNPC->CheckNPCTalkDistance(this))
	{
		return E_BindRebornMap_TooFar;
	}

	if(GetRebornMapID() == pMap->get_map_id())
	{
		return E_BindRebornMap_Already;
	}

	// ����
	m_dwRebornMapID = pMap->get_map_id();

	dwBornMapID = m_dwRebornMapID;
	vBornPos = g_mapCreator.get_reborn_point(m_dwRebornMapID);

	if(VALID_POINT(GetScript()))
		GetScript()->OnSetRebornMap(this, pMap->get_map_id(), pMap->get_instance_id());

	return E_Success;
}

//---------------------------------------------------------------------------------
// ʹ��ĥʯ
//---------------------------------------------------------------------------------
DWORD Role::AbraseWeapon(INT64 n64AbraserSerial)
{
	tagItem *pItem = GetItemMgr().GetBagItem(n64AbraserSerial);
	if(!VALID_POINT(pItem))
	{
		return INVALID_VALUE;
	}

	if(pItem->pProtoType->eSpecFunc != EISF_Grind)
	{
		return INVALID_VALUE;
	}

	int nRepaitCount = 0;
	for (int i = 0; i < EEP_MaxEquip; i++)
	{
		tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)i);
		if (!VALID_POINT(pEquip)) continue;

		if (pEquip->GetNewess() == 0)
			continue;

		EquipRepair(pEquip);
		nRepaitCount++;
	}

	if (nRepaitCount == 0)
		return INVALID_VALUE;


	// ����ĥʯ
	DWORD dw_error_code = GetItemMgr().ItemUsedFromBag(n64AbraserSerial, 1, elcid_item_use);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}



	return E_Success;
}