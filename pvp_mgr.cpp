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
*	@file		pvp_mgr.cpp
*	@author		lc
*	@date		2011/10/20	initial
*	@version	0.0.1.0
*	@brief		1v1管理
*/

#include "StdAfx.h"
#include "pvp_mgr.h"
#include "role.h"
#include "map.h"
#include "map_creator.h"

pvp_mgr g_pvp_mgr;

pvp_mgr::pvp_mgr(void)
{
	for(INT i = 0; i < 3; i++)
	{
		map_1v1[i].clear();
	}
	dw_pvp_id = 0;
}

pvp_mgr::~pvp_mgr(void)
{
}

VOID pvp_mgr::destroy()
{
	for(INT i = 0; i < 3; i++)
	{
		MAP_1V1::map_iter iter = map_1v1[i].begin();
		tag1v1* p1v1 = NULL;
		DWORD	dw_id = INVALID_VALUE;
		while(map_1v1[i].find_next(iter, dw_id, p1v1))
		{
			if(VALID_POINT(p1v1))
			{
				SAFE_DELETE(p1v1);
			}
			map_1v1[i].erase(dw_id);
		}
	}

	MAP_1V1::map_iter iter_reservation = map_reservation.begin();
	tag1v1* pReservation = NULL;
	DWORD	dw_id = NULL;
	while(map_reservation.find_next(iter_reservation, dw_id, pReservation))
	{
		if(VALID_POINT(pReservation))
		{
			SAFE_DELETE(pReservation);
		}
		map_reservation.erase(dw_id);
	}
}

VOID pvp_mgr::update()
{
	for(INT i = 0; i < 3; i++)
	{
		MAP_1V1::map_iter iter = map_1v1[i].begin();
		tag1v1* p1v1 = NULL;
		DWORD	dw_id = INVALID_VALUE;
		while(map_1v1[i].find_next(iter, dw_id, p1v1))
		{
			if(!VALID_POINT(p1v1))
				continue;

			// 检查是否存在异常数据
			if(p1v1->dw_role_id[0] == INVALID_VALUE && p1v1->dw_role_id[1] == INVALID_VALUE)
			{
				p1v1->b_delete = TRUE;
			}

			if(p1v1->dw_role_id[0] != INVALID_VALUE && p1v1->dw_role_id[1] != INVALID_VALUE)
			{
				Role* pRole = g_roleMgr.get_role(p1v1->dw_role_id[0]);
				Role* pPairRole = g_roleMgr.get_role(p1v1->dw_role_id[1]);
				if(!VALID_POINT(pRole) && !VALID_POINT(pPairRole))
				{
					p1v1->b_delete = TRUE;
				}
			}

			if(p1v1->dw_role_id[0] != INVALID_VALUE && p1v1->dw_role_id[1] == INVALID_VALUE)
			{
				Role* pRole = g_roleMgr.get_role(p1v1->dw_role_id[0]);
				if(!VALID_POINT(pRole))
				{
					p1v1->b_delete = TRUE;
				}
			}

			if(p1v1->b_delete)
			{
				Role* pRole = g_roleMgr.get_role(p1v1->dw_role_id[0]);
				if(VALID_POINT(pRole))
				{
					pRole->get_role_pvp().Reset();
					NET_SIS_1v1_leave_queue send;
					send.dw_error = E_Success;
					pRole->SendMessage(&send, send.dw_size);
				}

				Role* pPairRole = g_roleMgr.get_role(p1v1->dw_role_id[1]);
				if(VALID_POINT(pPairRole))
				{
					pPairRole->get_role_pvp().Reset();
					NET_SIS_1v1_leave_queue send;
					send.dw_error = E_Success;
					pPairRole->SendMessage(&send, send.dw_size);
				}
				SAFE_DELETE(p1v1);
				map_1v1[i].erase(dw_id);
			}
		}
	}

	MAP_1V1::map_iter iter_reservation = map_reservation.begin();
	tag1v1* pReservation = NULL;
	DWORD	dw_id = NULL;
	while(map_reservation.find_next(iter_reservation, dw_id, pReservation))
	{
		if(!VALID_POINT(pReservation))
			continue;

		// 检查是否存在异常数据
		if(pReservation->dw_role_id[0] == INVALID_VALUE && pReservation->dw_role_id[1] == INVALID_VALUE)
		{
			pReservation->b_delete = TRUE;
		}

		if(pReservation->dw_role_id[0] != INVALID_VALUE && pReservation->dw_role_id[1] != INVALID_VALUE)
		{
			Role* pRole = g_roleMgr.get_role(pReservation->dw_role_id[0]);
			Role* pPairRole = g_roleMgr.get_role(pReservation->dw_role_id[1]);
			if(!VALID_POINT(pRole) && !VALID_POINT(pPairRole))
			{
				pReservation->b_delete = TRUE;
			}
		}

		if(pReservation->b_delete)
		{
			Role* pRole = g_roleMgr.get_role(pReservation->dw_role_id[0]);
			if(VALID_POINT(pRole))
			{
				pRole->get_role_pvp().Reset();
			}

			Role* pPairRole = g_roleMgr.get_role(pReservation->dw_role_id[1]);
			if(VALID_POINT(pPairRole))
			{
				pPairRole->get_role_pvp().Reset();
			}
			SAFE_DELETE(pReservation);
			map_reservation.erase(dw_id);
		}
	}
}

VOID pvp_mgr::create_pvp_id(DWORD& dw_pvp_id_)
{
	m_lock.Acquire();
	dw_pvp_id_ = ++dw_pvp_id;
	m_lock.Release();
}

INT pvp_mgr::get_level_index(INT nLevel)
{
	INT nIndex = 0;

	if(nLevel >= 40 && nLevel <= 49)
	{
		nIndex = 0;
	}
	else if(nLevel >= 50 && nLevel <= 59)
	{
		nIndex = 1;
	}
	else if(nLevel >= 60)
	{
		nIndex = 2;
	}

	return nIndex;
}

INT	pvp_mgr::get_level_yuanbao(INT nLevel)
{
	INT nYuanBao = 0;

	if(nLevel >= 40 && nLevel <= 49)
	{
		nYuanBao = 15;
	}
	else if(nLevel >= 50 && nLevel <= 59)
	{
		nYuanBao = 20;
	}
	else if(nLevel >= 60)
	{
		nYuanBao = 30;
	}

	return nYuanBao * GOLD2SILVER;
}

BOOL pvp_mgr::is_cost_ticket(INT nScore, INT nLevel)
{
	if(nLevel >= 40 && nLevel <= 49)
	{
		if(nScore < 1)
			return TRUE;
	}
	else if(nLevel >= 50 && nLevel <=59)
	{
		if(nScore < 2)
			return TRUE;
	}
	else if(nLevel >= 60)
	{
		if(nScore < 3)
			return TRUE;
	}
	return FALSE;
}



INT	pvp_mgr::get_level_pumping_into(INT nLevel)
{
	INT nYuanBao = 0;

	if(nLevel >= 40 && nLevel <= 49)
	{
		nYuanBao =  5;
	}
	else if(nLevel >= 50 && nLevel <= 59)
	{
		nYuanBao = 8;
	}
	else if(nLevel >= 60)
	{
		nYuanBao = 10;
	}

	return nYuanBao * GOLD2SILVER;
}

INT	pvp_mgr::get_level_update_score(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return 3;

	if(pRole->get_level() >= 40 && pRole->get_level() <= 49)
	{
		return 1;
	}
	else if(pRole->get_level() >= 50 && pRole->get_level() <= 59)
	{
		return 2;
	}
	else if(pRole->get_level() >= 60)
	{
		return 3;
	}

	return 3;
}

VOID pvp_mgr::updata_role_score(DWORD dw_role_id, BOOL bAdd)
{
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	INT nScore = get_level_update_score(pRole);

	if(VALID_POINT(pRole))
	{
		pRole->get_1v1_score().n_day_scroe_num++;
		pRole->get_1v1_score().n_cur_score += (bAdd == TRUE) ? nScore : -nScore;

		if(pRole->get_1v1_score().n_cur_score > pRole->get_1v1_score().n_day_max_score)
		{
			pRole->get_1v1_score().n_cur_score = pRole->get_1v1_score().n_day_max_score;
		}

		if(pRole->get_1v1_score().n_cur_score < 0)
		{
			pRole->get_1v1_score().n_cur_score = 0;
		}

		NET_DB2C_update_role_1v1_score send;
		send.dw_role_id = dw_role_id;
		send.nScroe = pRole->get_1v1_score().n_cur_score;
		send.nJoinNum = pRole->get_1v1_score().n_day_scroe_num;
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_update_noline_role_1v1_score send;
		send.nScore += (bAdd == TRUE) ? nScore : -nScore;
		send.dw_role_id = dw_role_id;
		g_dbSession.Send(&send, send.dw_size);
	}
}

tag1v1* pvp_mgr::get_1v1_info(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return NULL;

	if(pRole->get_role_pvp().dw_pvp_id == INVALID_VALUE)
		return NULL;

	if(pRole->get_role_pvp().nIndex < 0 || pRole->get_role_pvp().nIndex > 3)
		return NULL;

	tag1v1* p = map_1v1[pRole->get_role_pvp().nIndex].find(pRole->get_role_pvp().dw_pvp_id);

	return p;
}

tag1v1*	pvp_mgr::get_reservation_info(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return NULL;

	if(pRole->get_role_pvp().dw_pvp_id == INVALID_VALUE)
		return NULL;

	tag1v1* p = map_reservation.find(pRole->get_role_pvp().dw_pvp_id);

	return p;
}

DWORD pvp_mgr::set_1v1_pair_id(Role* pRole, INT nIndex)
{
	if(map_1v1[nIndex].size() <= 0)
	{
		tag1v1* p = new tag1v1;
		if(!VALID_POINT(p))
			return INVALID_VALUE;

		DWORD dw_1v1_id = INVALID_VALUE;
		create_pvp_id(dw_1v1_id);

		pRole->get_role_pvp().Init(dw_1v1_id, nIndex);

		p->dw_role_id[0] = pRole->GetID();
		map_1v1[nIndex].add(dw_1v1_id, p);

		return E_Success;
		
	}

	MAP_1V1::map_iter iter = map_1v1[nIndex].begin();
	tag1v1* p1v1 = NULL;
	DWORD dw_1v1_id = INVALID_VALUE;
	while(map_1v1[nIndex].find_next(iter, dw_1v1_id, p1v1))
	{
		if(p1v1->dw_role_id[0] != INVALID_VALUE && p1v1->dw_role_id[1] == INVALID_VALUE)
		{
			Role* pPairRole = g_roleMgr.get_role(p1v1->dw_role_id[0]);
			if(!VALID_POINT(pPairRole))
			{
				p1v1->b_delete = TRUE;
				continue;
			}

			if(pPairRole->IsInRoleStateAny(ERS_Stall | ERS_PrisonArea | ERS_Carry) || pPairRole->IsDead())
			{
				p1v1->b_delete = TRUE;
				continue;
			}

			Map* pMap = pPairRole->get_map();
			if(!VALID_POINT(pMap))
			{
				p1v1->b_delete = TRUE;
				continue;
			}

			if(pMap->get_map_type() == EMT_Instance)
			{
				p1v1->b_delete = TRUE;
				continue;
			}

			if(pPairRole->GetCurMgr().GetBagSilver() < get_level_yuanbao(pPairRole->get_level()))
			{
				p1v1->b_delete = TRUE;
				continue;
			}

			if(pPairRole->get_1v1_score().n_day_scroe_num >= 15 || is_cost_ticket(pPairRole->get_1v1_score().n_cur_score, pPairRole->get_level()))
			{
				if(pPairRole->GetCurMgr().GetBaiBaoYuanBao() < 10)
				{
					p1v1->b_delete = TRUE;
					continue;
				}
			}

			if(pPairRole->IsDuel() || pPairRole->GetAutoKill() || pPairRole->is_leave_pricitice())
			{
				p1v1->b_delete = TRUE;
				continue;
			}

			if(pPairRole->IsInRoleState(ERS_Prictice))
			{
				pPairRole->PracticeEnd();
			}

			if(pPairRole->IsInRoleState(ERS_Fishing))
			{
				pPairRole->StopFishing();
			}

			if(pPairRole->IsInRoleState(ERS_Hang))
			{
				pPairRole->CancelHang();
			}

			p1v1->dw_role_id[1] = pRole->GetID();
			p1v1->dw_yuanbao = get_level_yuanbao(pPairRole->get_level());

			DWORD	dw_map_id = get_tool()->crc32(_T("j02"));
			
			const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(dw_map_id);
			if(!VALID_POINT(pInstance))
				return INVALID_VALUE;

			// 得到目标地图的导航点
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dw_map_id);
			if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;

			const tag_map_way_point_info_list* pList = NULL;
			pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
			if( !VALID_POINT(pList) ) return INVALID_VALUE;


			// 从目标导航点列表中任取一个导航点
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);

			Vector3 vFace;
			vFace.y = 0;
			vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

			pRole->GotoNewMap(dw_map_id, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

			pList->list.rand_find(wayPoint);
			vFace.y = 0;
			vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			pPairRole->GotoNewMap(dw_map_id, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

			pRole->GetCurMgr().DecBagSilver(p1v1->dw_yuanbao, elcid_1v1);
			if(pRole->get_1v1_score().n_day_scroe_num >= 15 || is_cost_ticket(pRole->get_1v1_score().n_cur_score, pRole->get_level()))
			{
				pRole->GetCurMgr().DecBaiBaoYuanBao(10, elcid_1v1);
			}
			pPairRole->GetCurMgr().DecBagSilver(p1v1->dw_yuanbao, elcid_1v1);
			if(pPairRole->get_1v1_score().n_day_scroe_num >= 15 || is_cost_ticket(pPairRole->get_1v1_score().n_cur_score, pPairRole->get_level()))
			{
				pPairRole->GetCurMgr().DecBaiBaoYuanBao(10, elcid_1v1);
			}

			pRole->get_role_pvp().Init(dw_1v1_id, nIndex);

			return E_Success;
		}
	}

	tag1v1* p = new tag1v1;
	if(!VALID_POINT(p))
		return INVALID_VALUE;

	dw_1v1_id = INVALID_VALUE;
	create_pvp_id(dw_1v1_id);

	pRole->get_role_pvp().Init(dw_1v1_id, nIndex);

	p->dw_role_id[0] = pRole->GetID();
	map_1v1[nIndex].add(dw_1v1_id, p);

	return E_Success;
}

DWORD pvp_mgr::pair_1v1(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	INT nIndex = get_level_index(pRole->get_level());
	if(nIndex < 0 || nIndex > 3)
		return INVALID_VALUE;

	DWORD dw_error = set_1v1_pair_id(pRole, nIndex);

	return dw_error;
}

DWORD pvp_mgr::apply_1v1(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(GetCurrentDWORDTime().hour < 19 || GetCurrentDWORDTime().hour > 22)
		return E_Instance_1v1_no_start;

	if(pRole->get_level() < 40)
		return E_Instance_1v1_level_limit;

	if(pRole->GetCurMgr().GetBagSilver() < get_level_yuanbao(pRole->get_level()))
		return E_Instance_1v1_yuanbao_limit;

	if(pRole->get_1v1_score().n_day_scroe_num >= 15 || is_cost_ticket(pRole->get_1v1_score().n_cur_score, pRole->get_level()))
	{
		if(pRole->GetCurMgr().GetBaiBaoYuanBao() < 10)
			return E_Instance_1v1_yuanbao_limit;
	}

	if(pRole->get_role_pvp().dw_pvp_id != INVALID_VALUE)
		return E_Instance_1v1_have_apply;

	if(pRole->IsReservationPvP())
		return E_Instance_1v1_have_apply;

	if(pRole->IsInRoleStateAny(ERS_Stall | ERS_PrisonArea | ERS_Carry) || pRole->IsDuel() ||  pRole->GetAutoKill() || pRole->IsDead())
		return E_Instance_1v1_state_limit;

	if(pMap->get_map_type() == EMT_Instance || pMap->get_map_type() == EMT_PVP || pMap->get_map_type() == EMT_1v1)
		return E_Instance_1v1_map_limit;
	
	if (pRole->is_in_guild_war())
	{
		return E_Instance_1v1_Is_guild_war;
	}

	DWORD dw_error = pair_1v1(pRole);

	return dw_error;
}

VOID pvp_mgr::role_offline(Role* pRole)
{
	if(pRole->get_role_pvp().dw_pvp_id == INVALID_VALUE)
		return;

	if(pRole->get_role_pvp().nIndex < 0 || pRole->get_role_pvp().nIndex > 3)
		return;

	tag1v1* p = map_1v1[pRole->get_role_pvp().nIndex].find(pRole->get_role_pvp().dw_pvp_id);
	if(VALID_POINT(p) && (p->dw_role_id[1] == INVALID_VALUE || p->dw_role_id[0] == INVALID_VALUE))
	{
		p->b_delete = TRUE;
	}
}

DWORD pvp_mgr::role_leave_queue(Role* pRole)
{
	if(pRole->get_role_pvp().dw_pvp_id == INVALID_VALUE)
		return E_Instance_1v1_nohane_apply;

	if(pRole->get_role_pvp().nIndex < 0 || pRole->get_role_pvp().nIndex > 3)
		return INVALID_VALUE;

	tag1v1* p = map_1v1[pRole->get_role_pvp().nIndex].find(pRole->get_role_pvp().dw_pvp_id);
	if(VALID_POINT(p) && (p->dw_role_id[1] == INVALID_VALUE || p->dw_role_id[0] == INVALID_VALUE))
	{
		p->b_delete = TRUE;
	}
	else
	{
		return INVALID_VALUE;
	}

	return E_Success;
}

DWORD pvp_mgr::pair_reservation(Role* pRole, Role* pReservation, INT nYuanbao)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(!VALID_POINT(pReservation))
		return INVALID_VALUE;

	DWORD	dw_pair_id = INVALID_VALUE;
	create_pvp_id(dw_pair_id);

	tag1v1* p = new tag1v1;
	if(!VALID_POINT(p))
		return INVALID_VALUE;

	p->dw_role_id[0] = pRole->GetID();
	p->dw_role_id[1] = pReservation->GetID();
	p->dw_yuanbao = nYuanbao;

	map_reservation.add(dw_pair_id, p);

	pRole->GetCurMgr().DecBagSilver(nYuanbao, elcid_reservation);
	pReservation->GetCurMgr().DecBagSilver(nYuanbao, elcid_reservation);

	pRole->get_role_pvp().Init(dw_pair_id, 0, FALSE);
	pReservation->get_role_pvp().Init(dw_pair_id, 0, FALSE);

	if(pRole->IsInRoleState(ERS_Prictice))
	{
		pRole->PracticeEnd();
	}

	if(pRole->IsInRoleState(ERS_Fishing))
	{
		pRole->StopFishing();
	}

	if(pRole->IsInRoleState(ERS_Hang))
	{
		pRole->CancelHang();
	}

	if(pReservation->IsInRoleState(ERS_Prictice))
	{
		pReservation->PracticeEnd();
	}

	if(pReservation->IsInRoleState(ERS_Fishing))
	{
		pReservation->StopFishing();
	}

	if(pReservation->IsInRoleState(ERS_Hang))
	{
		pReservation->CancelHang();
	}

	if(pReservation->IsInRoleState(ERS_Carry))
	{
		pReservation->UnsetRoleState(ERS_Carry);
	}

	DWORD	dw_map_id = get_tool()->crc32(_T("j02"));

	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(dw_map_id);
	if(!VALID_POINT(pInstance))
		return INVALID_VALUE;

	// 得到目标地图的导航点
	const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dw_map_id);
	if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;

	const tag_map_way_point_info_list* pList = NULL;
	pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
	if( !VALID_POINT(pList) ) return INVALID_VALUE;


	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	Vector3 vFace;
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

	pRole->GotoNewMap(dw_map_id, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

	pList->list.rand_find(wayPoint);
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	pReservation->GotoNewMap(dw_map_id, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

	pRole->SetReservationPvP(FALSE);
	pRole->SetReservationID(INVALID_VALUE);

	pReservation->SetReservationPvP(FALSE);
	pReservation->SetReservationID(INVALID_VALUE);

	return E_Success;
}

// 领取1v1奖励
DWORD pvp_mgr::get_1v1_award(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(pRole->get_1v1_score().n16_score_award < 1)
		return E_Instance_1v1_no_have_award;

	if(VALID_POINT(pRole->GetScript()))
	{
		pRole->GetScript()->OnGet1v1Award(pRole);
	}

	pRole->get_1v1_score().n16_score_award = 0;

	NET_C2DB_update_1v1_award send;
	send.dw_role_id = pRole->GetID();
	send.n16_award = 0;
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}
