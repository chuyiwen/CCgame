/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ִ帱��

#include "StdAfx.h"
#include "map_instance_stable.h"
#include "map_creator.h"
#include "role.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"


MapInstanceStable::MapInstanceStable() : map_instance()
{
}

MapInstanceStable::~MapInstanceStable()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// ��ʼ���������߲���Ҫ��
//------------------------------------------------------------------------------------------------------
BOOL MapInstanceStable::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_System == pInfo->e_type );

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
VOID MapInstanceStable::update()
{
	Map::update();
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID MapInstanceStable::destroy()
{

}

//---------------------------------------------------------------------------------
// ��ʽ����һ����ң���ֻ���ɹ���õ�ͼ��MapMgr����
//---------------------------------------------------------------------------------
VOID MapInstanceStable::add_role(Role* pRole, Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);
}

//---------------------------------------------------------------------------------
// ����뿪��ͼ��ֻ���������߳��������
//---------------------------------------------------------------------------------
VOID MapInstanceStable::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);
}
//---------------------------------------------------------------------------------
// �Ƿ��ܽ��븱��
//---------------------------------------------------------------------------------
INT MapInstanceStable::can_enter(Role *pRole)
{
	return E_Success;
}

//---------------------------------------------------------------------------------
// �Ƿ����ɾ��
//---------------------------------------------------------------------------------
BOOL MapInstanceStable::can_destroy()
{
	return map_instance::can_destroy();
}


//---------------------------------------------------------------------------------
// ��ʼ��ˢ�ֵ����
//---------------------------------------------------------------------------------
BOOL MapInstanceStable::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------------------
VOID MapInstanceStable::on_delete()
{
	
}

//-----------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------
VOID MapInstanceStable::on_destroy()
{
}
