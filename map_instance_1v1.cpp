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
*	@file		map_instance_pvp.h
*	@author		mmz
*	@date		2010/11/3	initial
*	@version	0.0.1.0
*	@brief		pvp����
*/


#include "StdAfx.h"
#include "map_instance_1v1.h"

#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"

#include "map_instance.h"
#include "map_creator.h"
#include "att_res.h"
#include "role.h"
#include "role_mgr.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "guild_manager.h"
#include "pvp_mgr.h"


map_instance_1v1::map_instance_1v1(void) : map_instance(), m_pInstance(NULL),
m_dwStartTick(INVALID_VALUE), m_dwEndTick(INVALID_VALUE),
m_bNoEnter(TRUE),dw_pvp_id(INVALID_VALUE)
{
}

map_instance_1v1::~map_instance_1v1(void)
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// ��ʼ��(������Ҫ������)
//------------------------------------------------------------------------------------------------------
BOOL map_instance_1v1::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_1v1 == pInfo->e_type );

	// ��ȡ������̬����
	m_pInstance = AttRes::GetInstance()->get_instance_proto(pInfo->dw_id);
	if( !VALID_POINT(m_pInstance) )	return FALSE;

	// ��ͼ�������
	p_map_info = pInfo;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();

	// �����������
	dw_id = p_map_info->dw_id;
	dw_instance_id = dwInstanceID;
	dw_pvp_id = dwMisc;
	//set_instance_id(dw_instance_id, pInfo->n_act_id);
	//m_eInstanceHardMode = (EInstanceHardMode)dwMisc;
	m_dwStartTick = g_world.GetWorldTick();
	m_dwEndTick = INVALID_VALUE;

	p_npc_team_mgr = new NPCTeamMgr(this);
	if(!VALID_POINT(p_npc_team_mgr))
		return FALSE;

	// ��ʼ��Ѱ·ϵͳ
	//if (!initSparseGraph(pInfo))
	//	return FALSE;

	// ����mapinfo����ʼ����ͼ�Ĺ�������б����Ϣ
	if( FALSE == init_logical_info() )
	{
		return FALSE;
	}

	return TRUE;
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID map_instance_1v1::update()
{
	if( map_role.empty() )
	set_delete();

	Map::update();
	update_time_issue();
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID map_instance_1v1::destroy()
{
	
}

//---------------------------------------------------------------------------------
// ��ʽ����һ����ң���ֻ���ɹ���õ�ͼ��MapMgr����
//---------------------------------------------------------------------------------
VOID map_instance_1v1::add_role(Role* pRole,  Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);

	// ���ùرյȴ�
	if( is_end() )
	{
		m_dwEndTick = INVALID_VALUE;
		b_end = FALSE;
	}

	// ���ͽ��븱����Ϣ
	NET_SIS_enter_instance send;
	send.dw_error_code = E_Success;
	send.dwTimeLimit = cal_time_limit();
	pRole->SendMessage(&send, send.dw_size);
}

//---------------------------------------------------------------------------------
// ����뿪��ͼ��ֻ���������߳��������
//---------------------------------------------------------------------------------
VOID map_instance_1v1::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);

	// �Ƿ����ȴ��ر�
	/*if( map_role.empty() && !is_end() && m_pInstance->dwEndTime != INVALID_VALUE )
	{
	m_dwEndTick = g_world.GetWorldTick();
	b_end = TRUE;
	}*/

	// ����������ڵȴ��뿪���б�����Ƴ�
	m_mapWillOutRoleID.erase(dwID);

	if(map_role.size() <= 1)
		set_delete();
}

//---------------------------------------------------------------------------------
// �Ƿ��ܽ��븱��
//---------------------------------------------------------------------------------
INT	map_instance_1v1::can_enter(Role *pRole, DWORD dwParam)
{
	// �ȼ��ͨ���ж�
	INT nErrorCode = map_instance::can_enter(pRole);
	if( E_Success != nErrorCode ) return nErrorCode;

	// �����������
	if( m_pInstance->nNumUpLimit <= get_role_num() )
		return E_Instance_Role_Full;

	return E_Success;
}

//---------------------------------------------------------------------------------
// �Ƿ����ɾ��
//---------------------------------------------------------------------------------
BOOL map_instance_1v1::can_destroy()
{
	return map_instance::can_destroy();
}

//-----------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------
VOID map_instance_1v1::on_destroy()
{

}

//---------------------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------------------
VOID map_instance_1v1::on_delete()
{
	// �Ƴ������ڵ�ͼ�ڵ����
	MapMgr* pMapMgr = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(pMapMgr) ) return;

	Role* pRole = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, pRole) )
	{
		if(VALID_POINT(pRole))
		{
			pMapMgr->RoleInstanceOut(pRole);

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
				p->b_delete = TRUE;

				if(!pRole->get_role_pvp().b1v1)
				{
					pRole->GetCurMgr().IncBagSilver(p->dw_yuanbao*2, elcid_1v1);

					NET_SIS_1v1_result send;
					g_roleMgr.get_role_name(pRole->GetID(), send.sz_win_name);

					DWORD	dw_faile_id = INVALID_VALUE;
					for(INT i = 0; i < 2; i++)
					{
						if(p->dw_role_id[i] != pRole->GetID())
						{
							dw_faile_id = p->dw_role_id[i];
							g_roleMgr.get_role_name(p->dw_role_id[i], send.sz_faile_name);
						}
					}
					pRole->SendMessage(&send, send.dw_size);

					Role* pFaildRole = g_roleMgr.get_role(dw_faile_id);
					if(VALID_POINT(pFaildRole))
					{
						pFaildRole->SendMessage(&send, send.dw_size);
					}
				}
				else
				{
					INT nPumping = g_pvp_mgr.get_level_pumping_into(pRole->get_level());
					pRole->GetCurMgr().IncBagSilver(p->dw_yuanbao*2-nPumping, elcid_1v1);
					g_pvp_mgr.updata_role_score(pRole->GetID(), TRUE);

					NET_SIS_1v1_score_result send_win;
					send_win.bWin = TRUE;
					send_win.bAward = (pRole->get_1v1_score().n16_score_award > 0) ? TRUE : FALSE;
					send_win.nJoinNum = pRole->get_1v1_score().n_day_scroe_num;
					send_win.nMaxScore = pRole->get_1v1_score().n_day_max_score;
					send_win.nScore = pRole->get_1v1_score().n_cur_score;
					pRole->SendMessage(&send_win, send_win.dw_size);
					pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_win, 1);
					DWORD	dw_faile_id = INVALID_VALUE;
					for(INT i = 0; i < 2; i++)
					{
						if(p->dw_role_id[i] != pRole->GetID())
						{
							dw_faile_id = p->dw_role_id[i];
						}
					}

					Role* pFaildRole = g_roleMgr.get_role(dw_faile_id);
					g_pvp_mgr.updata_role_score(dw_faile_id, FALSE);
					if(VALID_POINT(pFaildRole))
					{
						send_win.bWin = FALSE;
						send_win.bAward = (pFaildRole->get_1v1_score().n16_score_award > 0) ? TRUE : FALSE;
						send_win.nJoinNum = pFaildRole->get_1v1_score().n_day_scroe_num;
						send_win.nMaxScore = pFaildRole->get_1v1_score().n_day_max_score;
						send_win.nScore = pFaildRole->get_1v1_score().n_cur_score;
						pFaildRole->SendMessage(&send_win, send_win.dw_size);
						pFaildRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_lost, 1);
						pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_win_class, pFaildRole->GetClass(), 1);
						pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_win_level, pFaildRole->get_level() - pRole->get_level(), 1);
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------
// ��ʼ��ˢ�ֵ����
//---------------------------------------------------------------------------------
BOOL map_instance_1v1::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// ʱ����صĸ���
//---------------------------------------------------------------------------------
VOID map_instance_1v1::update_time_issue()
{
	// ����Ѿ����ڴ�ɾ��״̬���Ͳ�������
	if( is_delete() ) return;

	// ʱ�޸���
	if( is_time_limit() && !is_end() )
	{
		DWORD dwTick = g_world.GetWorldTick();
		if( (dwTick - m_dwStartTick) >= m_pInstance->dwTimeLimit * TICK_PER_SECOND )
		{
			m_dwEndTick = g_world.GetWorldTick();
			set_end();
		}
	}

	// �رյ���ʱ
	if( is_end() )
	{
		DWORD dwTick = g_world.GetWorldTick();
		if( (dwTick - m_dwEndTick) > m_pInstance->dwEndTime * TICK_PER_SECOND )
		{
			set_delete();
		}
	}

	// �������д��˳��Ľ�ɫ��ʱ��
	if( !m_mapWillOutRoleID.empty() )
	{
		package_map<DWORD, INT>::map_iter it = m_mapWillOutRoleID.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT nTick = INVALID_VALUE;

		while( m_mapWillOutRoleID.find_next(it, dw_role_id, nTick) )
		{
			--nTick;	// ��һ�µ���ʱ
			if( nTick <= 0 )
			{
				// ʱ�䵽�ˣ�����Ҵ��ͳ�ȥ
				Role* pRole = find_role(dw_role_id);
				if( VALID_POINT(pRole) )
				{
					MapMgr* pMapMgr = g_mapCreator.get_map_manager(p_map_info->dw_id);
					if( VALID_POINT(pMapMgr) )
					{
						pMapMgr->RoleInstanceOut(pRole);
					}
				}

				m_mapWillOutRoleID.erase(dw_role_id);
			}
			else
			{
				m_mapWillOutRoleID.change_value(dw_role_id, nTick);
			}
		}
	}
}

//-----------------------------------------------------------------------------------
// ����ʱ�޸�����ʣʱ��
//-----------------------------------------------------------------------------------
DWORD map_instance_1v1::cal_time_limit()
{
	DWORD dwTimeLeft = INVALID_VALUE;
	if(m_pInstance->dwTimeLimit > 0 && m_pInstance->dwTimeLimit != INVALID_VALUE)
	{
		DWORD dwCurrentTick = g_world.GetWorldTick();
		DWORD dwTimePass = (dwCurrentTick - m_dwStartTick) / TICK_PER_SECOND;
		dwTimeLeft = m_pInstance->dwTimeLimit - dwTimePass;
	}

	return dwTimeLeft;
}

VOID map_instance_1v1::on_leave_instance(DWORD dw_role_id)
{
	Role* pRole = map_role.find(dw_role_id);
	if(VALID_POINT(pRole))
	{
		m_mapWillOutRoleID.add(pRole->GetID(), 0);	
	}

	if(map_role.size() <= 1)
		set_delete();
}
