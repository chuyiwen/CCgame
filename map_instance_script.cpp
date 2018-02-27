/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ű�����

#include "StdAfx.h"
#include "map_instance_script.h"
#include "map_creator.h"
#include "role.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"

MapInstanceScript::MapInstanceScript() : map_instance()
{
}

MapInstanceScript::~MapInstanceScript()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// ��ʼ���������߲���Ҫ��
//------------------------------------------------------------------------------------------------------
BOOL MapInstanceScript::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_ScriptCreate == pInfo->e_type );

	// ��ͼ�������
	p_map_info = pInfo;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
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
VOID MapInstanceScript::update()
{
	Map::update();
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID MapInstanceScript::destroy()
{

}

//---------------------------------------------------------------------------------
// ��ʽ����һ����ң���ֻ���ɹ���õ�ͼ��MapMgr����
//---------------------------------------------------------------------------------
VOID MapInstanceScript::add_role(Role* pRole, Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);
}

//---------------------------------------------------------------------------------
// ����뿪��ͼ��ֻ���������߳��������
//---------------------------------------------------------------------------------
VOID MapInstanceScript::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);
}
//---------------------------------------------------------------------------------
// �Ƿ��ܽ��븱��
//---------------------------------------------------------------------------------
INT MapInstanceScript::can_enter(Role *pRole)
{
	return E_Success;
}

//---------------------------------------------------------------------------------
// �Ƿ����ɾ��
//---------------------------------------------------------------------------------
BOOL MapInstanceScript::can_destroy()
{
	return map_instance::can_destroy();
}


//---------------------------------------------------------------------------------
// ��ʼ��ˢ�ֵ����
//---------------------------------------------------------------------------------
BOOL MapInstanceScript::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------------------
VOID MapInstanceScript::on_delete()
{
	// �Ƴ������ڵ�ͼ�ڵ����
	MapMgr* pMapMgr = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(pMapMgr) ) return;

	Role* pRole = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, pRole) )
	{
		pMapMgr->RoleInstanceOut(pRole);
	}
}

//-----------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------
VOID MapInstanceScript::on_destroy()
{
}