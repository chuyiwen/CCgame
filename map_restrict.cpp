/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ͼ����

#include "StdAfx.h"
#include "map_restrict.h"
#include "map.h"
#include "map_mgr.h"
#include "map_creator.h"
#include "role.h"
#include "script_mgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "activity_mgr.h"
#include "map_instance.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "map_instance_guild.h"
#include "pvp_mgr.h"
#include "BattleGround.h"
#include "../common/WorldDefine/battle_ground_protocol.h"

//-------------------------------------------------------------------------------------------------------
// CONSTS
//-------------------------------------------------------------------------------------------------------
//const INT MAX_STABLE_INSTANCE_NUM = 10;			// �ȶ���������������

//-------------------------------------------------------------------------------------------------------
// ������
//-------------------------------------------------------------------------------------------------------
MapRestrict::MapRestrict() : m_pMgr(NULL)
{

}

MapRestrict::~MapRestrict()
{
	//m_pScript = NULL;
}

VOID MapRestrict::Init(MapMgr* pMapMgr)
{
	ASSERT(VALID_POINT(pMapMgr));

	m_pMgr	= pMapMgr;
	m_pInfo	= pMapMgr->get_map_info();
}

//---------------------------------------------------------------------------------------------------------
// ��ͨ��ͼ
//---------------------------------------------------------------------------------------------------------
VOID MapRestrictNormal::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);
}

Map* MapRestrictNormal::CanEnter(Role* pRole, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pRole) );
	
	if (m_pInfo->e_normal_map_type == ENMT_Activity)
	{
		const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
			return NULL;
		}


		// ��ͨ����������������ж�(����Ҫ)
		s_enter_map_limit* pEnterMapLimit = pRole->GetMapLimitMap().find(pProto->dwMapID);
		if(VALID_POINT(pEnterMapLimit))
		{

			if(pEnterMapLimit->dw_enter_num_ >= pProto->dwEnterNumLimit)
			{
				//pEnterMapLimit->dw_enter_num_ = 0;
				//gx add 2013.8.12 ��Կ󶴽��������������ʾ
				NET_SIS_enter_instance send;
				send.dwTimeLimit = INVALID_VALUE;
				send.dw_error_code = E_Instance_EnterNum_Limit;

				pRole->SendMessage(&send, send.dw_size);
				//end
				return NULL;
			}
			else
			{
				pEnterMapLimit->dw_enter_num_++;
			}

		}
		else
		{
			pEnterMapLimit = new s_enter_map_limit;
			pEnterMapLimit->dw_map_id_ = pProto->dwMapID;
			pEnterMapLimit->dw_enter_num_ = 1;
			pEnterMapLimit->e_enter_limit_ = pProto->eInstanceEnterLimit;
			pRole->GetMapLimitMap().add(pProto->dwMapID, pEnterMapLimit);

			NET_DB2C_insert_role_map_limit send;
			send.dw_role_id = pRole->GetID();
			send.st_enter_map_limit_.dw_map_id_ = pProto->dwMapID;
			send.st_enter_map_limit_.dw_enter_num_ = 1;
			send.st_enter_map_limit_.e_enter_limit_ = pProto->eInstanceEnterLimit;
			g_dbSession.Send(&send, send.dw_size);
		}
	}



	return m_pMgr->GetSingleMap();
}

Map* MapRestrictNormal::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	ASSERT( VALID_POINT(pRole) );

	return m_pMgr->GetSingleMap();
}

BOOL MapRestrictNormal::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

BOOL MapRestrictNormal::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//----------------------------------------------------------------------------------------------------------
// ������ͼ
//----------------------------------------------------------------------------------------------------------
VOID MapRestrictInstance::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);
}

//----------------------------------------------------------------------------------------------------------
// ĳ�������Ƿ��ܹ�����
//----------------------------------------------------------------------------------------------------------
Map* MapRestrictInstance::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_Instance != pInfo->e_type ) return NULL;

	// ���������в��ܽ���
	DWORD dwInstanceID = pRole->GetOwnInstanceID();
	DWORD dwMapID = pRole->GetOwnInstanceMapID();

	map_instance* pInstance = m_pMgr->GetInstance(dwInstanceID);
	if (dwMapID != m_pMgr->get_map_info()->dw_id)
	{
		pInstance = (map_instance*)g_mapCreator.get_map(dwMapID, dwInstanceID);
	}
	
	
	if (VALID_POINT(pInstance))
	{
		if (pInstance->is_end())
		{
			pInstance = NULL;

			const Team *pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
			if( VALID_POINT(pTeam) )
			{
				pTeam->set_own_instanceid(INVALID_VALUE);
				pTeam->set_own_instance_mapid(INVALID_VALUE);
			}

		}
	}

	// �����жϸ�����̬����
	INT nRet = CanEnterByInstanceInfo(pRole, dwMisc);

	if( E_Success == nRet )
	{
		if( !VALID_POINT(pInstance) )
		{
			// ����ʵ��������
			//nRet = E_Instance_Not_Exit;
			pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{
				nRet = E_Instance_Full;
			}
		}
		else
		{
			// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
			nRet = ((map_instance_normal*)pInstance)->can_enter(pRole, dwMisc);
			/*if(nRet != E_Success)
			{
				pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
				if( !VALID_POINT(pInstance) )
				{
					nRet = E_Instance_Full;
				}
			}*/
		}

	}

	if( E_Success != nRet )		// ���������������룬���͸������Ϣ
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// ����������
}

//-----------------------------------------------------------------------------------------------------------
// ���ݸ�����̬�����ж��Ƿ��ܹ�����
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictInstance::CanEnterByInstanceInfo(Role* pRole, DWORD dwMisc)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	BOOL bItem = TRUE;

	/*package_list<s_inst_process*>::list_iter iter = pRole->get_list_inst_process().begin();
	s_inst_process* p = NULL;
	while(pRole->get_list_inst_process().find_next(iter, p))
	{
		if(!VALID_POINT(p))
			continue;

		if(p->dw_map_id != pProto->dwMapID || p->n_mode != dwMisc)
			continue;

		bItem = FALSE;
		goto NoLevel;
	}*/

	// ��һ������ڶ����ǲ����Ѿ�����������
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dwID )
	//	return E_Instance_Already;

	tagItem* pLgnoreItem = NULL;
	if(pProto->dwLgnoreItemID != INVALID_VALUE)
	{
		package_list<tagItem*> list_item;
		INT32 n_num = pRole->GetItemMgr().GetBagSameItemList(list_item, pProto->dwLgnoreItemID, 1);
		if(n_num < 1)
			goto Level;

		package_list<tagItem*>::list_iter iter = list_item.begin();

		pLgnoreItem = *iter;

		if(!VALID_POINT(pLgnoreItem))
			goto Level;

		bItem = FALSE;
		goto NoLevel;
	}

Level:
	if(dwMisc == 0)
	{
		// �ȼ�����
		if( pProto->nLevelDownLimit > pRole->get_level() )
			return E_Instance_Level_Down_Limit;

		if( pProto->nLevelUpLimit < pRole->get_level() )
			return E_Instance_Level_Up_Limit;
	}
	else if(dwMisc == 1)
	{
		// �ȼ�����
		if( pProto->nLevelEliteDownLimit > pRole->get_level() )
			return E_Instance_Level_Down_Limit;

		if( pProto->nLevelEliteUpLimit < pRole->get_level() )
			return E_Instance_Level_Up_Limit;
	}
	else if(dwMisc == 2)
	{
		// �ȼ�����
		if( pProto->nLevelDevilDownLimit > pRole->get_level() )
			return E_Instance_Level_Down_Limit;

		if( pProto->nLevelDevilUpLimit < pRole->get_level() )
			return E_Instance_Level_Up_Limit;
	}

NoLevel:

	// �����ж�
	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	if( VALID_POINT(pTeam))
	{
		if (pProto->nNumDownLimit > 1 && pTeam->get_member_number() < pProto->nNumDownLimit )
		{
			return E_Instance_Role_Lack;
		}

		if (pTeam->get_member_number() > pProto->nNumUpLimit)
		{
			return E_Instance_Role_Full;
		}
	}
			
	
	
/*****************************************************/	
	// �ж�Ȩֵ����
	//s_enter_map_limit* pEnterMapLimit = pRole->GetMapLimitMap().find(pProto->dwMapID);
	//if(VALID_POINT(pEnterMapLimit))
	//{
	//	if(pProto->eInstanceEnterLimit == EEL_Week && pProto->bClearNumLimit)
	//	{
	//		pEnterMapLimit->dw_enter_num_ = 0;
	//	}
	//	else
	//	{
	//		//��������ħ����VIP�ӿ�
	//		if (pProto->dwMapID == get_tool()->crc32(_T("d140")))
	//		{
	//			if(pEnterMapLimit->dw_enter_num_ >= pProto->dwEnterNumLimit+GET_VIP_EXTVAL(pRole->GetVIPLevel(),YANHUOTUMO_ADD,INT))
	//			{
	//				//pEnterMapLimit->dw_enter_num_ = 0;
	//				return E_Instance_EnterNum_Limit;
	//			}
	//			else
	//			{
	//				pEnterMapLimit->dw_enter_num_ ++;
	//			}
	//		}
	//		else
	//		{
	//			if(pEnterMapLimit->dw_enter_num_ >= pProto->dwEnterNumLimit)
	//			{
	//				//pEnterMapLimit->dw_enter_num_ = 0;
	//				return E_Instance_EnterNum_Limit;
	//			}
	//			else
	//			{
	//				pEnterMapLimit->dw_enter_num_ ++;
	//			}
	//		}
	//		
	//	}
	//}
	//else
	//{
	//	pEnterMapLimit = new s_enter_map_limit;
	//	pEnterMapLimit->dw_map_id_ = pProto->dwMapID;
	//	pEnterMapLimit->dw_enter_num_ = 1;
	//	pEnterMapLimit->e_enter_limit_ = pProto->eInstanceEnterLimit;
	//	pRole->GetMapLimitMap().add(pProto->dwMapID, pEnterMapLimit);

	//	NET_DB2C_insert_role_map_limit send;
	//	send.dw_role_id = pRole->GetID();
	//	send.st_enter_map_limit_.dw_map_id_ = pProto->dwMapID;
	//	send.st_enter_map_limit_.dw_enter_num_ = 1;
	//	send.st_enter_map_limit_.e_enter_limit_ = pProto->eInstanceEnterLimit;
	//	g_dbSession.Send(&send, send.dw_size);
	//}
/*****************************************************/	
	// ����ж���
	if(pRole->GetTeamID() != INVALID_VALUE)
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
		if(VALID_POINT(pTeam))
		{
			if(pTeam->get_own_instance_mapid() != INVALID_VALUE && pProto->dwMapID != pTeam->get_own_instance_mapid())
				return E_Instance_team_error;
		}
		
	}
	
	// ����������
	if (pProto->eInstanceMapType == EIMT_Single)
	{
		// ɨ��ʱ������
		if (pRole->isSaodangIng())
			return E_Instance_Single_saodanging;

		// ��ǰ���Ƿ��ͨ
		if (pProto->nIndex > 0)
		{
			if (!pRole->isInstancePass(pProto->nIndex - 1))
				return E_Instance_Single_qianzhinot;
		}

		// �ﵽÿ����ս����
		if (pRole->GetDayClearData(ERDCT_BUYLINGQI) >= 3)
			return E_Instance_Single_EnterMax;

	}
	//��С�Ǹ��� gx add 2013.11.06
	if (EIMT_Rand == pProto->eInstanceMapType)
	{
		if (pRole->GetDayClearData(ERDCT_DXQ_INSTANCE_LIMIT) >= DXQ_INSTANCE_LIMIT_TIMES)
			return E_Instance_EnterNum_Limit;
	}
	//////////////////////////////////////////////////
	s_enter_map_limit* pEnterMapLimit = pRole->GetMapLimitMap().find(pProto->dwMapID);
	if(VALID_POINT(pEnterMapLimit))
	{
		if(pProto->eInstanceEnterLimit == EEL_Week && pProto->bClearNumLimit)
		{
			pEnterMapLimit->dw_enter_num_ = 0;
		}
		else
		{
			//��������ħ����VIP�ӿ�
			if (pProto->dwMapID == get_tool()->crc32(_T("d140")))
			{
				if(pEnterMapLimit->dw_enter_num_ >= pProto->dwEnterNumLimit+GET_VIP_EXTVAL(pRole->GetVIPLevel(),YANHUOTUMO_ADD,INT))
				{
					//pEnterMapLimit->dw_enter_num_ = 0;
					return E_Instance_EnterNum_Limit;
				}
				else
				{
					pEnterMapLimit->dw_enter_num_ ++;
				}
			}
			else
			{
				if(pEnterMapLimit->dw_enter_num_ >= pProto->dwEnterNumLimit)
				{
					//pEnterMapLimit->dw_enter_num_ = 0;
					return E_Instance_EnterNum_Limit;
				}
				else
				{
					pEnterMapLimit->dw_enter_num_ ++;
				}
			}

		}
	}
	else
	{
		pEnterMapLimit = new s_enter_map_limit;
		pEnterMapLimit->dw_map_id_ = pProto->dwMapID;
		pEnterMapLimit->dw_enter_num_ = 1;
		pEnterMapLimit->e_enter_limit_ = pProto->eInstanceEnterLimit;
		pRole->GetMapLimitMap().add(pProto->dwMapID, pEnterMapLimit);

		NET_DB2C_insert_role_map_limit send;
		send.dw_role_id = pRole->GetID();
		send.st_enter_map_limit_.dw_map_id_ = pProto->dwMapID;
		send.st_enter_map_limit_.dw_enter_num_ = 1;
		send.st_enter_map_limit_.e_enter_limit_ = pProto->eInstanceEnterLimit;
		g_dbSession.Send(&send, send.dw_size);
	}
	/////////////////////////////////////////////////

	// ��Ҫ���ߵĸ���Ҫ������ж�
	// gx modify 2013.9.22 ���븱�����ĵ���Ҫ�������İ󶨵�
	if( pProto->dwItemID != INVALID_VALUE && bItem)
	{
		//package_list<tagItem*> list_item;
		//INT32 n_num = pRole->GetItemMgr().GetBagSameItemList(list_item, pProto->dwItemID, 1);
		//if(n_num < 1)
		//{
		//	if (VALID_POINT(pEnterMapLimit))
		//	{
		//		pEnterMapLimit->dw_enter_num_--;//�����ع�
		//	}
		//	return E_Instance_not_item;
		//}
		//package_list<tagItem*>::list_iter iter = list_item.begin();

		//tagItem* pItem = *iter;

		//if(!VALID_POINT(pItem))
		//{
		//	if (VALID_POINT(pEnterMapLimit))
		//	{
		//		pEnterMapLimit->dw_enter_num_--;//�����ع�
		//	}
		//	return E_Instance_not_item;
		//}

		//pRole->GetItemMgr().DelFromBag(pItem->n64_serial, elcid_instance, 1);
		INT nTotalNum = pRole->GetItemMgr().GetBagSameItemCount(pProto->dwItemID);//��ñ����еĵ��������������󶨵�
		INT nBindNum = pRole->GetItemMgr().GetBagSameBindItemCount(pProto->dwItemID,TRUE);//�󶨵ĵ�������
		if (nTotalNum < 1)//�����������쳣���ͻ����������ж�
		{
			if (VALID_POINT(pEnterMapLimit))
			{
				pEnterMapLimit->dw_enter_num_--;//�����ع�
			}
			return E_Instance_not_item;
		}
		//�������İ󶨵�
		if (nBindNum >= 1)//���󶨵Ĺ�
		{
			pRole->GetItemMgr().RemoveFromRole(pProto->dwItemID,1,(DWORD)elcid_instance,1);
		}
		else//�󶨵Ĳ���
		{
			pRole->GetItemMgr().RemoveFromRole(pProto->dwItemID,1,(DWORD)elcid_instance);
		}
	}
	//��С�Ǹ����������� gx add 2013.11.06
	if (EIMT_Rand == pProto->eInstanceMapType)
	{
		pRole->ModRoleDayClearDate(ERDCT_DXQ_INSTANCE_LIMIT,1,FALSE);//����֪ͨ�ͻ��˴�������
	}
	//end by gx
	DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	{
		pRole->SetMyOwnInstanceID(INVALID_VALUE);
		pRole->SetMyOwnInstanceMapID(INVALID_VALUE);
		tagDWORDTime st_Time;
		ZeroMemory(&st_Time, sizeof(st_Time));
		pRole->SetMyOwnInstanceCreateTime(st_Time);
	}

	if(VALID_POINT(pLgnoreItem))
	{
		pRole->GetItemMgr().DelFromBag(pLgnoreItem->n64_serial, elcid_instance, 1);
	}
	

	
	return E_Success;
}

//------------------------------------------------------------------------------------------------------------
// ������ߺ�Ľӹܴ���
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictInstance::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ����ڸ��������ߣ������ߵĻ�������ͣ���ڸ����ڣ���Ҫ����Ҵ���
	DWORD dwInstanceID = INVALID_VALUE;
	BOOL bRet = GetExportMapAndCoord(pRole, dwOutMapID, dwInstanceID, vOut, fYaw);

	if(bRet && dwInstanceID != INVALID_VALUE)
	{
		Map* pMap = g_mapCreator.get_map(dwOutMapID, dwInstanceID);
		if(VALID_POINT(pMap))
			return pMap;
	}

	return NULL;
}

BOOL MapRestrictInstance::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);

			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			// �õ�Ŀ���ͼ�ĵ�����
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);

			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

//------------------------------------------------------------------------------------------------------------
// �õ����ڵ�ͼ������
//------------------------------------------------------------------------------------------------------------
BOOL MapRestrictInstance::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	//����������ʱ�ж����Ҷ����ڸ����У�ֱ�Ӵ���������ͼ��
	if(pRole->GetTeamID() != INVALID_VALUE)
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
		if(VALID_POINT(pTeam))
		{
			if(pTeam->get_own_instanceid() != INVALID_VALUE && pTeam->get_own_instance_mapid() != INVALID_VALUE)
			{
				map_instance_normal* pInstance = (map_instance_normal*)(m_pMgr->GetInstance(pTeam->get_own_instanceid()));
				if(VALID_POINT(pInstance))
				{
					BOOL bRet = pInstance->can_enter(pRole, pInstance->get_instance_hard());
					if(bRet == E_Success)
					{
						dwOutMapID = pTeam->get_own_instance_mapid();
						vOut = pRole->GetCurPos();
						dwInstanceID = pTeam->get_own_instanceid();

						return TRUE;
					}
				}
			}
		}
	}
	else
	{
		if(pRole->GetMyOwnInstanceID() != INVALID_VALUE && pRole->GetMyOwnInstanceMapID())
		{
			map_instance_normal* pInstance = (map_instance_normal*)(m_pMgr->GetInstance(pRole->GetMyOwnInstanceID()));
			if(VALID_POINT(pInstance))
			{
				if(pInstance->get_create_time() == pRole->GetMyOwnInstanceCreateTime())
				{
					BOOL bRet = pInstance->can_enter(pRole, pInstance->get_instance_hard());
					if(bRet == E_Success)
					{
						dwOutMapID = pRole->GetOwnInstanceMapID();
						vOut = pRole->GetCurPos();

						dwInstanceID = pRole->GetMyOwnInstanceID();
						return TRUE;
					}
				}
			}
		}
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);

			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			// �õ�Ŀ���ͼ�ĵ�����
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);
			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

//------------------------------------------------------------------------------------------------------------------
// ��ʼ��
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictStable::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	// �����ȶ�����
	INT n_num = g_mapCreator.get_stable_instance_num();

	for(INT n = 0; n < n_num; ++n)
	{
		pMapMgr->CreateInstance(NULL, INVALID_VALUE);
	}
}

//-------------------------------------------------------------------------------------------------------------------
// �Ƿ���Խ���
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictStable::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_System != pInfo->e_type ) return NULL;

	// ����һ�����ʵ�map���ϲ�
	map_instance* pInstance = GetOnePerfectMap();

	return pInstance;
}

//------------------------------------------------------------------------------------------------------------
// ������ߺ�Ľӹܴ���
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictStable::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ���Խӹܣ��ҵ�һ���������ٵ�
	return GetOnePerfectMap();
}

BOOL MapRestrictStable::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------
// �õ����ڵ�ͼ������
//-------------------------------------------------------------------------------------------------------------
BOOL MapRestrictStable::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//--------------------------------------------------------------------------------------------------------------------
// �ҵ�һ������ʵĸ���ʵ��
//--------------------------------------------------------------------------------------------------------------------
map_instance* MapRestrictStable::GetOnePerfectMap()
{
	map_instance* pInstance = NULL;	// �������ٵĸ���
	INT nMinRoleNum = INT_MAX;		// ��������

	// ��mapmgr���ҵ�һ���������ٵĸ���
	map_instance* pTemp = NULL;
	package_map<DWORD, map_instance*>::map_iter it = m_pMgr->m_mapInstance.begin();

	while( m_pMgr->m_mapInstance.find_next(it, pTemp) )
	{
		INT n_num = pTemp->get_role_num();
		if( n_num < nMinRoleNum )
		{
			nMinRoleNum = n_num;
			pInstance = pTemp;
		}
	}

	return pInstance;
}

//------------------------------------------------------------------------------------------------------------------
// ��ʼ��
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictScript::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	// �ű�Ҳ��������г�ʼ��
	m_pScript = g_ScriptMgr.GetMapScript(pMapMgr->get_map_info()->dw_id);
}

//-------------------------------------------------------------------------------------------------------------------
// �Ƿ���Խ���
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictScript::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_ScriptCreate != pInfo->e_type ) return NULL;

	map_instance* pInstance = NULL;

	if ( VALID_POINT(m_pScript) )
	{
		if(m_pScript->CanEnter(pRole))
		{
			// ����һ�����ʵ�map���ϲ�
			pInstance = GetOnePerfectMap(pRole);
		}
	}

	return pInstance;
}

//------------------------------------------------------------------------------------------------------------
// ������ߺ�Ľӹܴ���
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictScript::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ���ýű�
	if( VALID_POINT(m_pScript) )
	{
		 m_pScript->CanTakeOverWhenOnline(pRole, dwOutMapID, vOut);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------------------
// �õ����ڵ�ͼ������
//-------------------------------------------------------------------------------------------------------------
BOOL MapRestrictScript::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	if( VALID_POINT(m_pScript) )
	{
		BOOL  bSuccess = FALSE; 
		bSuccess =  m_pScript->GetExportMapAndCoord(pRole, dwOutMapID, vOut);

		return bSuccess;
	}

	return FALSE;
}

BOOL MapRestrictScript::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//--------------------------------------------------------------------------------------------------------------------
// �ҵ�һ������ʵĸ���ʵ��
//--------------------------------------------------------------------------------------------------------------------
map_instance* MapRestrictScript::GetOnePerfectMap(Role* pRole)
{
	DWORD  dwInstanceID = INVALID_VALUE;
	if( VALID_POINT(m_pScript) )
	{
		m_pScript->GetOnePerfectMap(pRole, dwInstanceID);
	}

	return m_pMgr->GetInstance(dwInstanceID);
}

//---------------------------------------------------------------------------------------------------------
// ���ɵ�ͼ
//---------------------------------------------------------------------------------------------------------
VOID MapRestrictGuild::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);
}

Map* MapRestrictGuild::CanEnter(Role* pRole, DWORD dwMisc)
{
	
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_Guild != pInfo->e_type ) return NULL;

	map_instance* pInstance = NULL;

	// �����жϸ�����̬����
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		// ���ҵ�����ʵ��
		guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
		if(!VALID_POINT(pGuild))
			return pInstance;

		if(dwMisc == 1)
		{
			if (pGuild->get_guild_war().isDefenter())
			{
				nRet = E_Instance_Not_Exit;

				NET_SIS_enter_instance send;
				send.dwTimeLimit = INVALID_VALUE;
				send.dw_error_code = nRet;

				pRole->SendMessage(&send, send.dw_size);

				return pInstance;
			}

			guild* pEnemyGuild = g_guild_manager.get_guild(pGuild->get_guild_war().get_war_guild_id());
			if(!VALID_POINT(pEnemyGuild))
				return pInstance;

			if(pEnemyGuild->get_guild_war().get_guild_war_state() == EGWS_WAR_relay ||
				pEnemyGuild->get_guild_war().get_guild_war_state() == EGWS_WAR)
			{
				dwInstanceID = pEnemyGuild->get_guild_instance_id();
			}
			else
			{
				return pInstance;
			}
		}
		else
		{
			dwInstanceID = pGuild->get_guild_instance_id();
		}
		

		if( VALID_POINT(dwInstanceID) )
		{
			pInstance =  m_pMgr->GetInstance(dwInstanceID);
			if( !VALID_POINT(pInstance) )
			{
				// ����ʵ��������
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
				nRet = ((map_instance_guild*)pInstance)->can_enter(pRole, dwMisc);
			}
		}
		else
		{
			if(pGuild->is_in_guild_state(EGDSS_UpLevel))
			{
				nRet = E_Instance_GuildUp;
			}
			else if(!pGuild->get_guild_att().bFormal)
			{
				nRet = E_Instance_Guild_NotFormal;
			}
			//else if(pGuild->get_guild_war().get_guild_war_state() == EGWS_WAR)
			//{
			//	nRet = E_Instance_Act_NoBegin;
			//}
			else if(pGuild->get_upgrade().IsFacilitesDestory(EFT_Lobby))
			{
				nRet = E_Instance_Lobby_Repair;
			}
			else
			{
				// ���ɶ�û�и������򴴽�һ��
				pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
				if( !VALID_POINT(pInstance) )
				{
					nRet = E_Instance_Full;
				}
				// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
				nRet = ((map_instance_guild*)pInstance)->can_enter(pRole, dwMisc);
			}
			
		}
	}

	if( E_Success != nRet )		// ���������������룬���͸������Ϣ
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}else{
		// ����ɹ��������PK״̬
		if(pRole->GetPKState() != ERolePK_Peace){
		//	DWORD dwError = pRole->SetPKStateLimit(ERolePK_Peace);
			if(!pRole->HasPKValue()){
				NET_SIS_change_pk_state send;
				send.dw_role_id = pRole->GetID();
				send.ePKState = ERolePK_Peace;
				send.dwError = E_Success;
				pRole->SendMessage(&send, send.dw_size);
				pRole->SetPKState(ERolePK_Peace);
			}else{
				print_message(_T("GuildInstance CanEnter LogicError:role[%d],PK:[%d]"),
					pRole->GetID( ), pRole->GetPKValue());
			}
		}
	}

	return pInstance;		// ����������
}

Map* MapRestrictGuild::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ����ڸ��������ߣ������ߵĻ�������ͣ���ڸ����ڣ���Ҫ����Ҵ���
	GetExportMapAndCoord(pRole, dwOutMapID, vOut, fYaw);

	return NULL;
}

BOOL MapRestrictGuild::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

BOOL MapRestrictGuild::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);

			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);
			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------------------------------------
// ���ݸ�����̬�����ж��Ƿ��ܹ�����
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictGuild::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// ��һ������ڶ����ǲ����Ѿ�����������
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// �ȼ�����
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// ��������
	//if( pProto->nNumDownLimit > 1 )
	//{
	//	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	//	if( !VALID_POINT(pTeam) || pTeam->get_member_number() < pProto->nNumDownLimit ) 
	//		return E_Instance_Role_Lack;
	//}

	return E_Success;
}

//---------------------------------------------------------------------------------------------------------
// PVP��ͼ
//---------------------------------------------------------------------------------------------------------
VOID MapRestrictPvP::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	memset(pInstance, NULL, sizeof(pInstance));

	for(INT16 i = 0; i < PVP_NUM; i++)
	{
		pInstance[i] = pMapMgr->CreateInstance(NULL, INVALID_VALUE);
		if(!VALID_POINT(pInstance[i]))
			ERROR_CLUE_ON(_T("pvp instance init faild!!!"));
	}
}

Map* MapRestrictPvP::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_PVP != pInfo->e_type ) return NULL;

	map_instance* pInstance = NULL;

	// �����жϸ�����̬����
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		// ���ҵ�����ʵ��
		/*s_guild_pvp_data* pGuildPvPData = g_guild_manager.get_pvp_data(pInfo->n_act_id);
		if(VALID_POINT(pGuildPvPData))
		{
		dwInstanceID = pGuildPvPData->dw_instance_id;
		}*/
		dwInstanceID = get_instance_id(pRole->get_level());

		if( VALID_POINT(dwInstanceID) )
		{
			pInstance =  m_pMgr->GetInstance(dwInstanceID);
			if( !VALID_POINT(pInstance) )
			{
				// ����ʵ��������
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
				nRet = pInstance->can_enter(pRole);
			}
		}
		else
		{
			// ��һ���鶼û�и������򴴽�һ��
			/*pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{*/
			nRet = E_Instance_Full;
			/*}*/
		}
	}

	if( E_Success != nRet )		// ���������������룬���͸������Ϣ
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// ����������
}

Map* MapRestrictPvP::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ����ڸ��������ߣ������ߵĻ�������ͣ���ڸ����ڣ���Ҫ����Ҵ���
	GetExportMapAndCoord(pRole, dwOutMapID, vOut, fYaw);

	return NULL;
}

BOOL MapRestrictPvP::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);
			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

BOOL MapRestrictPvP::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//-----------------------------------------------------------------------------------------------------------
// ��ȡ
//-----------------------------------------------------------------------------------------------------------
DWORD MapRestrictPvP::get_instance_id(INT nLevel)
{
	INT nIndex = 0;
	if(nLevel >= 30 && nLevel <=39)
		nIndex = 0;
	else if(nLevel >= 40 && nLevel <= 49)
		nIndex = 1;
	else if(nLevel >= 50 && nLevel <= 59)
		nIndex = 2;
	else if(nLevel >= 60 && nLevel <= 65)
		nIndex = 3;
	else if(nLevel >= 66 && nLevel <= 70)
		nIndex = 4;

	return pInstance[nIndex]->get_instance_id();

}

//-----------------------------------------------------------------------------------------------------------
// ���ݸ�����̬�����ж��Ƿ��ܹ�����
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictPvP::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// ��һ������ڶ����ǲ����Ѿ�����������
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// �ȼ�����
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// ��������
	/*if( pProto->nNumDownLimit > 1 )
	{
	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) || pTeam->get_member_number() < pProto->nNumDownLimit ) 
	return E_Instance_Role_Lack;
	}*/

	/*guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	return E_Instance_NoGuild;

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(m_pInfo->n_act_id);
	if(!VALID_POINT(pActivity))
	return INVALID_VALUE;

	if(!pActivity->is_start())
	return E_Instance_Act_NoBegin;*/

	return E_Success;
}

VOID MapRestrict1v1::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);
}

Map* MapRestrict1v1::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_1v1 != pInfo->e_type ) return NULL;

	map_instance* pInstance = NULL;

	// �����жϸ�����̬����
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		
		tag1v1* p = NULL;
		if(pRole->get_role_pvp().b1v1)
		{
			p = g_pvp_mgr.get_1v1_info(pRole);
		}
		else
		{
			p = g_pvp_mgr.get_reservation_info(pRole);
		}
		
		if(VALID_POINT(p))
		{
			dwInstanceID = p->dw_instance_id;
			dwMisc = pRole->get_role_pvp().dw_pvp_id;
		}

		if( VALID_POINT(dwInstanceID) )
		{
			pInstance =  m_pMgr->GetInstance(dwInstanceID);
			if( !VALID_POINT(pInstance) )
			{
				// ����ʵ��������
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
				nRet = pInstance->can_enter(pRole);
			}
		}
		else
		{
			// ��һ���鶼û�и������򴴽�һ��
			pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{
				nRet = E_Instance_Full;
			}

			if(nRet == E_Success)
			{
				p->dw_instance_id = pInstance->get_instance_id();
			}
		}
	}

	if( E_Success != nRet )		// ���������������룬���͸������Ϣ
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// ����������
}

Map* MapRestrict1v1::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ����ڸ��������ߣ������ߵĻ�������ͣ���ڸ����ڣ���Ҫ����Ҵ���
	GetExportMapAndCoord(pRole, dwOutMapID, vOut, fYaw);

	return NULL;
}

BOOL MapRestrict1v1::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);
			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

BOOL MapRestrict1v1::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

INT MapRestrict1v1::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// ��һ������ڶ����ǲ����Ѿ�����������
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// �ȼ�����
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// ��������
	/*if( pProto->nNumDownLimit > 1 )
	{
	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) || pTeam->get_member_number() < pProto->nNumDownLimit ) 
	return E_Instance_Role_Lack;
	}*/

	/*guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	return E_Instance_NoGuild;

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(m_pInfo->n_act_id);
	if(!VALID_POINT(pActivity))
	return INVALID_VALUE;

	if(!pActivity->is_start())
	return E_Instance_Act_NoBegin;*/

	return E_Success;
}


//------------------------------------------------------------------------------------------------------------------
// ��ʼ��
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictSBK::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	pInstance = pMapMgr->CreateInstance(NULL, INVALID_VALUE);

	if(!VALID_POINT(pInstance))
		ERROR_CLUE_ON(_T("sbk instance init faild!!!"));

	
}

//-------------------------------------------------------------------------------------------------------------------
// �Ƿ���Խ���
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictSBK::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_SBK != pInfo->e_type ) return NULL;


	return pInstance;
}

//------------------------------------------------------------------------------------------------------------
// ������ߺ�Ľӹܴ���
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictSBK::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ���Խӹܣ��ҵ�һ���������ٵ�
	return pInstance;
}

BOOL MapRestrictSBK::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------
// �õ����ڵ�ͼ������
//-------------------------------------------------------------------------------------------------------------
BOOL MapRestrictSBK::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}



//------------------------------------------------------------------------------------------------------------------
// ��ʼ��
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictBattle::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	pInstance = pMapMgr->CreateInstance(NULL, INVALID_VALUE);

	if(!VALID_POINT(pInstance))
		ERROR_CLUE_ON(_T("battle instance init faild!!!"));


}

INT MapRestrictBattle::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// ���ǻʱ�䲻�ܽ���
	activity_fix* pActive = activity_mgr::GetInstance()->get_activity(pProto->nActID);
	if (!VALID_POINT(pActive))
	{
		print_message(_T("Can't find Battle Active %u\r\n"), pProto->nActID);
		return INVALID_VALUE;
	}

	if (!pActive->is_start())
	{
		return E_Instance_battle_not_start;
	}

	if (BattleGround::get_singleton().getState() != EBS_BEGIN)
	{
		return E_Instance_battle_not_start;
	}

	// �ȼ����㲻�ܽ���
	if (pRole->get_level() < BATTLE_ROLE_LEVEL)
	{
		return E_Instance_battle_not_level;
	}
	
	// �������˲��ܽ���
	if (BattleGround::get_singleton().getRoleNum() >= BATTLE_MAX_ROLE_NUMBER)
		return E_Instance_battle_max_num;


	return E_Success;
}
//-------------------------------------------------------------------------------------------------------------------
// �Ƿ���Խ���
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictBattle::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_Battle != pInfo->e_type ) return NULL;


	// �����жϸ�����̬����
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{

		
		if( !VALID_POINT(pInstance) )
		{
			// ����ʵ��������
			//nRet = E_Instance_Not_Exit;
			pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{
				nRet = E_Instance_Full;
			}
		}
		else
		{
			// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
			nRet = ((map_instance_normal*)pInstance)->can_enter(pRole, dwMisc);
			/*if(nRet != E_Success)
			{
				pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
				if( !VALID_POINT(pInstance) )
				{
					nRet = E_Instance_Full;
				}
			}*/
		}

	}

	if( E_Success != nRet )		// ���������������룬���͸������Ϣ
	{
		pRole->set_temp_role_camp(ECA_Null);

		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// ����������


}

//------------------------------------------------------------------------------------------------------------
// ������ߺ�Ľӹܴ���
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictBattle::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ���Խӹܣ��ҵ�һ���������ٵ�
	return NULL;
}

BOOL MapRestrictBattle::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{


	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------
// �õ����ڵ�ͼ������
//-------------------------------------------------------------------------------------------------------------
BOOL MapRestrictBattle::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);
			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}


//---------------------------------------------------------------------------------------------------------
// PVP�����ͼ
//---------------------------------------------------------------------------------------------------------
VOID MapRestrictPvPBiWu::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	memset(pInstance, NULL, sizeof(pInstance));

	for(INT16 i = 0; i < PVP_BIWU_NUM; i++)
	{
		pInstance[i] = pMapMgr->CreateInstance(NULL, INVALID_VALUE);
		if(!VALID_POINT(pInstance[i]))
			ERROR_CLUE_ON(_T("pvp instance init faild!!!"));
	}
}

Map* MapRestrictPvPBiWu::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_PVP_BIWU != pInfo->e_type ) return NULL;

	map_instance* pInstance = NULL;
	INT nRet = E_Success;

	//1.���ж� �������жϻ�Ƿ��Ѿ���ʼ�����ʼ�������ٽ�����丱����ͼ��
	const DWORD activity_id = 19;
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(activity_id);	
	if (pActivity == NULL)
	{
		return NULL;
		//nRet = E_Instance_pvp_biwu_active_not_exit;
	}
	else
	{
		//���ʼ��5�����ڿ��Խ��룬5���Ӻ󲻿ɽ���
		if ( !pActivity->is_start() )
		{
			nRet = E_Instance_pvp_biwu_active_not_started;
		}
		else if (pActivity->get_minute_update_count() >= 5)
		{
			nRet = E_Instance_pvp_biwu_active_is_started;			
		}
	}

	//2.�жϸ�����̬����
	if( E_Success == nRet )
		nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		// ���ҵ�����ʵ��
		/*s_guild_pvp_data* pGuildPvPData = g_guild_manager.get_pvp_data(pInfo->n_act_id);
		if(VALID_POINT(pGuildPvPData))
		{
		dwInstanceID = pGuildPvPData->dw_instance_id;
		}*/
		dwInstanceID = get_instance_id(pRole->get_level());

		if( VALID_POINT(dwInstanceID) )
		{
			pInstance =  m_pMgr->GetInstance(dwInstanceID);
			if( !VALID_POINT(pInstance) )
			{
				// ����ʵ��������
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// ����ʵ�����ڣ����ʵ�������Ƿ���Խ���
				nRet = pInstance->can_enter(pRole);
			}
		}
		else
		{
			// ��һ���鶼û�и������򴴽�һ��
			/*pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{*/
			nRet = E_Instance_Full;
			/*}*/
		}
	}

	if( E_Success != nRet )		// ���������������룬���͸������Ϣ
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// ����������
}

Map* MapRestrictPvPBiWu::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// ����ڸ��������ߣ������ߵĻ�������ͣ���ڸ����ڣ���Ҫ����Ҵ���
	GetExportMapAndCoord(pRole, dwOutMapID, vOut, fYaw);

	return NULL;
}

BOOL MapRestrictPvPBiWu::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	switch( pProto->eExportMode )
	{
	case EEM_Born:			// ������
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// �����
	case EEM_Current:		// ��ǰ��
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// ָ��
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// ��Ŀ�굼�����б�����ȡһ��������
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);
			vOut = wayPoint.v_pos;

			return TRUE;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

BOOL MapRestrictPvPBiWu::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//-----------------------------------------------------------------------------------------------------------
// ��ȡ
//-----------------------------------------------------------------------------------------------------------
DWORD MapRestrictPvPBiWu::get_instance_id(INT nLevel)
{
	INT nIndex = 0;
	//if(nLevel >= 30 && nLevel <=39)
	//	nIndex = 0;
	//else if(nLevel >= 40 && nLevel <= 49)
	//	nIndex = 1;
	//else if(nLevel >= 50 && nLevel <= 59)
	//	nIndex = 2;
	//else if(nLevel >= 60 && nLevel <= 65)
	//	nIndex = 3;
	//else if(nLevel >= 66 && nLevel <= 70)
	//	nIndex = 4;

	return pInstance[nIndex]->get_instance_id();

}

//-----------------------------------------------------------------------------------------------------------
// ���ݸ�����̬�����ж��Ƿ��ܹ�����
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictPvPBiWu::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// ��һ������ڶ����ǲ����Ѿ�����������
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// �ȼ�����
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// ��������
	/*if( pProto->nNumDownLimit > 1 )
	{
	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) || pTeam->get_member_number() < pProto->nNumDownLimit ) 
	return E_Instance_Role_Lack;
	}*/

	/*guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	return E_Instance_NoGuild;

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(m_pInfo->n_act_id);
	if(!VALID_POINT(pActivity))
	return INVALID_VALUE;

	if(!pActivity->is_start())
	return E_Instance_Act_NoBegin;*/

	return E_Success;
}
BOOL MapRestrictPvPBiWu::CanUseSkill()
{
	//1.���ж� �������жϻ�Ƿ��Ѿ���ʼ�����ʼ�������ٽ�����丱����ͼ��
	const DWORD activity_id = 19;
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(activity_id);	
	if (pActivity == NULL)
	{
		return FALSE;
	}
	else
	{
		//���ʼ��5�����ڿ��Խ��룬5���Ӻ󲻿ɽ���
		if ( !pActivity->is_start() )
		{
			return FALSE;
		}
		else if (pActivity->get_minute_update_count() >= 5)
		{
			return TRUE;
		}
	}
	return FALSE; 
}
