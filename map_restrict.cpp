/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//地图限制

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
//const INT MAX_STABLE_INSTANCE_NUM = 10;			// 稳定副本开的最大个数

//-------------------------------------------------------------------------------------------------------
// 抽象类
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
// 普通地图
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


		// 普通副本进入次数限制判断(矿洞需要)
		s_enter_map_limit* pEnterMapLimit = pRole->GetMapLimitMap().find(pProto->dwMapID);
		if(VALID_POINT(pEnterMapLimit))
		{

			if(pEnterMapLimit->dw_enter_num_ >= pProto->dwEnterNumLimit)
			{
				//pEnterMapLimit->dw_enter_num_ = 0;
				//gx add 2013.8.12 针对矿洞进入次数满后，无提示
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
// 副本地图
//----------------------------------------------------------------------------------------------------------
VOID MapRestrictInstance::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);
}

//----------------------------------------------------------------------------------------------------------
// 某个副本是否能够进入
//----------------------------------------------------------------------------------------------------------
Map* MapRestrictInstance::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_Instance != pInfo->e_type ) return NULL;

	// 副本销毁中不能进入
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

	// 首先判断副本静态属性
	INT nRet = CanEnterByInstanceInfo(pRole, dwMisc);

	if( E_Success == nRet )
	{
		if( !VALID_POINT(pInstance) )
		{
			// 副本实例不存在
			//nRet = E_Instance_Not_Exit;
			pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{
				nRet = E_Instance_Full;
			}
		}
		else
		{
			// 副本实例存在，检测实例本身是否可以进入
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

	if( E_Success != nRet )		// 如果副本不允许进入，则发送给玩家消息
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// 将副本返回
}

//-----------------------------------------------------------------------------------------------------------
// 根据副本静态属性判断是否能够进入
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

	// 玩家或其所在队伍是不是已经创建了其它
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
		// 等级限制
		if( pProto->nLevelDownLimit > pRole->get_level() )
			return E_Instance_Level_Down_Limit;

		if( pProto->nLevelUpLimit < pRole->get_level() )
			return E_Instance_Level_Up_Limit;
	}
	else if(dwMisc == 1)
	{
		// 等级限制
		if( pProto->nLevelEliteDownLimit > pRole->get_level() )
			return E_Instance_Level_Down_Limit;

		if( pProto->nLevelEliteUpLimit < pRole->get_level() )
			return E_Instance_Level_Up_Limit;
	}
	else if(dwMisc == 2)
	{
		// 等级限制
		if( pProto->nLevelDevilDownLimit > pRole->get_level() )
			return E_Instance_Level_Down_Limit;

		if( pProto->nLevelDevilUpLimit < pRole->get_level() )
			return E_Instance_Level_Up_Limit;
	}

NoLevel:

	// 人数判断
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
	// 判断权值限制
	//s_enter_map_limit* pEnterMapLimit = pRole->GetMapLimitMap().find(pProto->dwMapID);
	//if(VALID_POINT(pEnterMapLimit))
	//{
	//	if(pProto->eInstanceEnterLimit == EEL_Week && pProto->bClearNumLimit)
	//	{
	//		pEnterMapLimit->dw_enter_num_ = 0;
	//	}
	//	else
	//	{
	//		//针对焰火屠魔增加VIP接口
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
	// 如果有队伍
	if(pRole->GetTeamID() != INVALID_VALUE)
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
		if(VALID_POINT(pTeam))
		{
			if(pTeam->get_own_instance_mapid() != INVALID_VALUE && pProto->dwMapID != pTeam->get_own_instance_mapid())
				return E_Instance_team_error;
		}
		
	}
	
	// 单人镇妖塔
	if (pProto->eInstanceMapType == EIMT_Single)
	{
		// 扫荡时进步了
		if (pRole->isSaodangIng())
			return E_Instance_Single_saodanging;

		// 看前置是否打通
		if (pProto->nIndex > 0)
		{
			if (!pRole->isInstancePass(pProto->nIndex - 1))
				return E_Instance_Single_qianzhinot;
		}

		// 达到每天挑战上限
		if (pRole->GetDayClearData(ERDCT_BUYLINGQI) >= 3)
			return E_Instance_Single_EnterMax;

	}
	//大小乔副本 gx add 2013.11.06
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
			//针对焰火屠魔增加VIP接口
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

	// 需要道具的副本要在最后判定
	// gx modify 2013.9.22 进入副本消耗道具要优先消耗绑定的
	if( pProto->dwItemID != INVALID_VALUE && bItem)
	{
		//package_list<tagItem*> list_item;
		//INT32 n_num = pRole->GetItemMgr().GetBagSameItemList(list_item, pProto->dwItemID, 1);
		//if(n_num < 1)
		//{
		//	if (VALID_POINT(pEnterMapLimit))
		//	{
		//		pEnterMapLimit->dw_enter_num_--;//次数回滚
		//	}
		//	return E_Instance_not_item;
		//}
		//package_list<tagItem*>::list_iter iter = list_item.begin();

		//tagItem* pItem = *iter;

		//if(!VALID_POINT(pItem))
		//{
		//	if (VALID_POINT(pEnterMapLimit))
		//	{
		//		pEnterMapLimit->dw_enter_num_--;//次数回滚
		//	}
		//	return E_Instance_not_item;
		//}

		//pRole->GetItemMgr().DelFromBag(pItem->n64_serial, elcid_instance, 1);
		INT nTotalNum = pRole->GetItemMgr().GetBagSameItemCount(pProto->dwItemID);//获得背包中的道具总数，包括绑定的
		INT nBindNum = pRole->GetItemMgr().GetBagSameBindItemCount(pProto->dwItemID,TRUE);//绑定的道具总数
		if (nTotalNum < 1)//进入这里是异常，客户端已做过判断
		{
			if (VALID_POINT(pEnterMapLimit))
			{
				pEnterMapLimit->dw_enter_num_--;//次数回滚
			}
			return E_Instance_not_item;
		}
		//优先消耗绑定的
		if (nBindNum >= 1)//若绑定的够
		{
			pRole->GetItemMgr().RemoveFromRole(pProto->dwItemID,1,(DWORD)elcid_instance,1);
		}
		else//绑定的不够
		{
			pRole->GetItemMgr().RemoveFromRole(pProto->dwItemID,1,(DWORD)elcid_instance);
		}
	}
	//大小乔副本次数增加 gx add 2013.11.06
	if (EIMT_Rand == pProto->eInstanceMapType)
	{
		pRole->ModRoleDayClearDate(ERDCT_DXQ_INSTANCE_LIMIT,1,FALSE);//不必通知客户端次数增加
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
// 玩家上线后的接管处理
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictInstance::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 玩家在副本中下线，再上线的话不会再停留在副本内，而要将玩家传出
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);

			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			// 得到目标地图的导航点
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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
// 得到出口地图和坐标
//------------------------------------------------------------------------------------------------------------
BOOL MapRestrictInstance::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return FALSE;
	}

	//如果玩家上线时有队伍且队伍在副本中，直接传到副本地图中
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);

			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			// 得到目标地图的导航点
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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
// 初始化
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictStable::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	// 生成稳定副本
	INT n_num = g_mapCreator.get_stable_instance_num();

	for(INT n = 0; n < n_num; ++n)
	{
		pMapMgr->CreateInstance(NULL, INVALID_VALUE);
	}
}

//-------------------------------------------------------------------------------------------------------------------
// 是否可以进入
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictStable::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_System != pInfo->e_type ) return NULL;

	// 返回一个合适的map给上层
	map_instance* pInstance = GetOnePerfectMap();

	return pInstance;
}

//------------------------------------------------------------------------------------------------------------
// 玩家上线后的接管处理
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictStable::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 可以接管，找到一个人数最少的
	return GetOnePerfectMap();
}

BOOL MapRestrictStable::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------
// 得到出口地图和坐标
//-------------------------------------------------------------------------------------------------------------
BOOL MapRestrictStable::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//--------------------------------------------------------------------------------------------------------------------
// 找到一个最合适的副本实例
//--------------------------------------------------------------------------------------------------------------------
map_instance* MapRestrictStable::GetOnePerfectMap()
{
	map_instance* pInstance = NULL;	// 人数最少的副本
	INT nMinRoleNum = INT_MAX;		// 最少人数

	// 从mapmgr中找到一个人数最少的副本
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
// 初始化
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictScript::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	// 脚本也在这里进行初始化
	m_pScript = g_ScriptMgr.GetMapScript(pMapMgr->get_map_info()->dw_id);
}

//-------------------------------------------------------------------------------------------------------------------
// 是否可以进入
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
			// 返回一个合适的map给上层
			pInstance = GetOnePerfectMap(pRole);
		}
	}

	return pInstance;
}

//------------------------------------------------------------------------------------------------------------
// 玩家上线后的接管处理
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictScript::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 调用脚本
	if( VALID_POINT(m_pScript) )
	{
		 m_pScript->CanTakeOverWhenOnline(pRole, dwOutMapID, vOut);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------------------
// 得到出口地图和坐标
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
// 找到一个最合适的副本实例
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
// 帮派地图
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

	// 首先判断副本静态属性
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		// 再找到副本实例
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
				// 副本实例不存在
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// 副本实例存在，检测实例本身是否可以进入
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
				// 帮派都没有副本，则创建一个
				pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
				if( !VALID_POINT(pInstance) )
				{
					nRet = E_Instance_Full;
				}
				// 副本实例存在，检测实例本身是否可以进入
				nRet = ((map_instance_guild*)pInstance)->can_enter(pRole, dwMisc);
			}
			
		}
	}

	if( E_Success != nRet )		// 如果副本不允许进入，则发送给玩家消息
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}else{
		// 如果成功进入调整PK状态
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

	return pInstance;		// 将副本返回
}

Map* MapRestrictGuild::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 玩家在副本中下线，再上线的话不会再停留在副本内，而要将玩家传出
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);

			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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
// 根据副本静态属性判断是否能够进入
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictGuild::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// 玩家或其所在队伍是不是已经创建了其它
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// 等级限制
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// 人数下限
	//if( pProto->nNumDownLimit > 1 )
	//{
	//	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	//	if( !VALID_POINT(pTeam) || pTeam->get_member_number() < pProto->nNumDownLimit ) 
	//		return E_Instance_Role_Lack;
	//}

	return E_Success;
}

//---------------------------------------------------------------------------------------------------------
// PVP地图
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

	// 首先判断副本静态属性
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		// 再找到副本实例
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
				// 副本实例不存在
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// 副本实例存在，检测实例本身是否可以进入
				nRet = pInstance->can_enter(pRole);
			}
		}
		else
		{
			// 玩家或队伍都没有副本，则创建一个
			/*pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{*/
			nRet = E_Instance_Full;
			/*}*/
		}
	}

	if( E_Success != nRet )		// 如果副本不允许进入，则发送给玩家消息
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// 将副本返回
}

Map* MapRestrictPvP::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 玩家在副本中下线，再上线的话不会再停留在副本内，而要将玩家传出
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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
// 获取
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
// 根据副本静态属性判断是否能够进入
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictPvP::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// 玩家或其所在队伍是不是已经创建了其它
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// 等级限制
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// 人数下限
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

	// 首先判断副本静态属性
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
				// 副本实例不存在
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// 副本实例存在，检测实例本身是否可以进入
				nRet = pInstance->can_enter(pRole);
			}
		}
		else
		{
			// 玩家或队伍都没有副本，则创建一个
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

	if( E_Success != nRet )		// 如果副本不允许进入，则发送给玩家消息
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// 将副本返回
}

Map* MapRestrict1v1::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 玩家在副本中下线，再上线的话不会再停留在副本内，而要将玩家传出
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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

	// 玩家或其所在队伍是不是已经创建了其它
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// 等级限制
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// 人数下限
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
// 初始化
//------------------------------------------------------------------------------------------------------------------
VOID MapRestrictSBK::Init(MapMgr* pMapMgr)
{
	MapRestrict::Init(pMapMgr);

	pInstance = pMapMgr->CreateInstance(NULL, INVALID_VALUE);

	if(!VALID_POINT(pInstance))
		ERROR_CLUE_ON(_T("sbk instance init faild!!!"));

	
}

//-------------------------------------------------------------------------------------------------------------------
// 是否可以进入
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictSBK::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_SBK != pInfo->e_type ) return NULL;


	return pInstance;
}

//------------------------------------------------------------------------------------------------------------
// 玩家上线后的接管处理
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictSBK::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 可以接管，找到一个人数最少的
	return pInstance;
}

BOOL MapRestrictSBK::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------
// 得到出口地图和坐标
//-------------------------------------------------------------------------------------------------------------
BOOL MapRestrictSBK::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	return FALSE;
}



//------------------------------------------------------------------------------------------------------------------
// 初始化
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

	// 不是活动时间不能进入
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

	// 等级不足不能进入
	if (pRole->get_level() < BATTLE_ROLE_LEVEL)
	{
		return E_Instance_battle_not_level;
	}
	
	// 人数满了不能进入
	if (BattleGround::get_singleton().getRoleNum() >= BATTLE_MAX_ROLE_NUMBER)
		return E_Instance_battle_max_num;


	return E_Success;
}
//-------------------------------------------------------------------------------------------------------------------
// 是否可以进入
//-------------------------------------------------------------------------------------------------------------------
Map* MapRestrictBattle::CanEnter(Role* pRole, DWORD dwMisc)
{
	const tag_map_info* pInfo = m_pMgr->get_map_info();
	if( !VALID_POINT(pInfo) || EMT_Battle != pInfo->e_type ) return NULL;


	// 首先判断副本静态属性
	INT nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{

		
		if( !VALID_POINT(pInstance) )
		{
			// 副本实例不存在
			//nRet = E_Instance_Not_Exit;
			pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{
				nRet = E_Instance_Full;
			}
		}
		else
		{
			// 副本实例存在，检测实例本身是否可以进入
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

	if( E_Success != nRet )		// 如果副本不允许进入，则发送给玩家消息
	{
		pRole->set_temp_role_camp(ECA_Null);

		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// 将副本返回


}

//------------------------------------------------------------------------------------------------------------
// 玩家上线后的接管处理
//------------------------------------------------------------------------------------------------------------
Map* MapRestrictBattle::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 可以接管，找到一个人数最少的
	return NULL;
}

BOOL MapRestrictBattle::GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw)
{


	return FALSE;
}

//-------------------------------------------------------------------------------------------------------------
// 得到出口地图和坐标
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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
// PVP比武地图
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

	//1.先判断 比武活动，判断活动是否已经开始，活动开始后不允许再进入比武副本地图。
	const DWORD activity_id = 19;
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(activity_id);	
	if (pActivity == NULL)
	{
		return NULL;
		//nRet = E_Instance_pvp_biwu_active_not_exit;
	}
	else
	{
		//活动开始后5分钟内可以进入，5分钟后不可进入
		if ( !pActivity->is_start() )
		{
			nRet = E_Instance_pvp_biwu_active_not_started;
		}
		else if (pActivity->get_minute_update_count() >= 5)
		{
			nRet = E_Instance_pvp_biwu_active_is_started;			
		}
	}

	//2.判断副本静态属性
	if( E_Success == nRet )
		nRet = CanEnterByInstanceInfo(pRole);

	if( E_Success == nRet )
	{
		DWORD dwInstanceID = INVALID_VALUE;
		// 再找到副本实例
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
				// 副本实例不存在
				nRet = E_Instance_Not_Exit;
			}
			else
			{
				// 副本实例存在，检测实例本身是否可以进入
				nRet = pInstance->can_enter(pRole);
			}
		}
		else
		{
			// 玩家或队伍都没有副本，则创建一个
			/*pInstance = m_pMgr->CreateInstance(pRole, dwMisc);
			if( !VALID_POINT(pInstance) )
			{*/
			nRet = E_Instance_Full;
			/*}*/
		}
	}

	if( E_Success != nRet )		// 如果副本不允许进入，则发送给玩家消息
	{
		NET_SIS_enter_instance send;
		send.dwTimeLimit = INVALID_VALUE;
		send.dw_error_code = nRet;

		pRole->SendMessage(&send, send.dw_size);

		pInstance = NULL;
	}

	return pInstance;		// 将副本返回
}

Map* MapRestrictPvPBiWu::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	// 玩家在副本中下线，再上线的话不会再停留在副本内，而要将玩家传出
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
	case EEM_Born:			// 出生点
		{
			dwOutMapID = g_mapCreator.get_born_map_id();
			INT nIndex = 0;
			vOut = g_mapCreator.rand_get_one_born_position(nIndex);
			fYaw = g_mapCreator.get_born_yaw(nIndex);
			return TRUE;
		}
		break;

	case EEM_Reborn:		// 复活点
	case EEM_Current:		// 当前点
		{
			dwOutMapID = pRole->GetRebornMapID();
			const tag_map_info* pInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pInfo) ) return FALSE;
			vOut = pInfo->v_reborn_positon;

			return TRUE;
		}
		break;

	case EEM_Appoint:		// 指定
		{
			dwOutMapID = pProto->dwExportMapID;
			//vOut = pProto->vExportPos;
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwOutMapID);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pProto->dwExportWayPoint);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
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
// 获取
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
// 根据副本静态属性判断是否能够进入
//-----------------------------------------------------------------------------------------------------------
INT MapRestrictPvPBiWu::CanEnterByInstanceInfo(Role* pRole)
{
	const tagInstance* pProto = AttRes::GetInstance()->get_instance_proto(m_pInfo->dw_id);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Can't find Instance Proto %u\r\n"), m_pInfo->dw_id);
		return INVALID_VALUE;
	}

	// 玩家或其所在队伍是不是已经创建了其它
	//DWORD dwOwnInstanceMapID = pRole->GetOwnInstanceMapID();
	//if( VALID_POINT(dwOwnInstanceMapID) && dwOwnInstanceMapID != m_pInfo->dw_id )
	//	return E_Instance_Already;

	// 等级限制
	if( pProto->nLevelDownLimit > pRole->get_level() )
		return E_Instance_Level_Down_Limit;

	if( pProto->nLevelUpLimit < pRole->get_level() )
		return E_Instance_Level_Up_Limit;

	// 人数下限
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
	//1.先判断 比武活动，判断活动是否已经开始，活动开始后不允许再进入比武副本地图。
	const DWORD activity_id = 19;
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(activity_id);	
	if (pActivity == NULL)
	{
		return FALSE;
	}
	else
	{
		//活动开始后5分钟内可以进入，5分钟后不可进入
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
