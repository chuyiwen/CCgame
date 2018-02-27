/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ͼ������

#include "StdAfx.h"

#include "../../common/WorldDefine/map_protocol.h"

#include "map_creator.h"
#include "map.h"
#include "map_mgr.h"
#include "role.h"
#include "role_mgr.h"
#include "world_session.h"
#include "login_session.h"
#include "map_instance.h"
#include "map_instance_stable.h"
#include "map_restrict.h"
#include "game_guarder.h"
#include "map_instance_script.h"
#include "creature.h"
#include "map_instance_guild.h"
#include "map_instance_pvp.h"
#include "map_instance_pvp_biwu.h"
#include "map_instance_1v1.h"
#include "map_instance_sbk.h"
#include "map_instance_battle.h"
#include "hearSay_helper.h"
#include "..\common\serverbase\zlib\zlib.h"

#define POOL_SIZE 50*1024*1024		// ��ʼ�ش�СΪ50M


map_creator::map_creator()
: m_map_end_event(TRUE, FALSE), m_n_num_map_manager(0), m_n_which_event(1)
{
	
}

map_creator::~map_creator()
{
	destroy();
	
}

//-----------------------------------------------------------------------------
// ��ʼ�����е�ͼ������Ϣ�����е�ͼ������
//-----------------------------------------------------------------------------
BOOL map_creator::init()
{
	
	m_dw_instance_id_gen	=	1000;

	map_guild_tripod.clear();

	register_factory_map();			// ע���ͼ����
	register_factory_map_restrict();	// ע���ͼ���Թ���


	// ��ȡ��ͼ�߼�����xml��
	const TCHAR* sz_map_logic_proto_file = _T("data/config/table/SceneInfo.xml");
	file_container* p_logic = new file_container;
	p_logic->load(g_world.get_virtual_filesys(), sz_map_logic_proto_file, "id");

	// ��ȡ��ͼ�����ļ����������ļ��ж�ȡҪ���صĵ�ͼ�ļ���
	file_container* p_var = new file_container;

	TCHAR t_sz_path[MAX_PATH];
	ZeroMemory(t_sz_path, sizeof(t_sz_path));

	if (!get_file_io_mgr()->get_ini_path(t_sz_path, _T("server_config/world/map_list"))||
		!p_var->load(g_world.get_virtual_filesys(), t_sz_path))
	{
		ERROR_CLUE_ON(_T("�����ļ�δ�ҵ�"));
		return FALSE;
	}

	//p_var->load(g_world.get_virtual_filesys(), _T("server_config/world/map_list.ini"));

	// ���ظ����ļ�Ȩ�ܺ�
	ZeroMemory(m_n_instance_cur_num, sizeof(m_n_instance_cur_num));
	m_n_instance_coef_num_limit = p_var->get_int(_T("coef_num"),	_T("instance"), 1000);
	m_n_instance_coef_cur_num = 0;

	// ���ظ�����ͼ����
	INT n_map_num = p_var->get_int(_T("num"), _T("map"));
	if( n_map_num <= 0 ) return FALSE;

	TCHAR sz_temp[X_SHORT_NAME] = {'\0'};
	for(INT n = 0; n < n_map_num; ++n)
	{
		_stprintf(sz_temp, _T("map%d"), n+1);
		LPCTSTR szMapFileName = (LPCTSTR)p_var->get_string(sz_temp, _T("map"));
		if( FALSE == VALID_POINT(szMapFileName) ) return FALSE;

		if( FALSE == loag_map_info(szMapFileName, p_logic) )
		{
			ASSERT(0);
			return FALSE;
		}
	}

	//mwh-�����е�mapinfo�����ʹ���
	tag_map_info *pInfoEx;
	m_map_map_info.reset_iterator( );
	while( m_map_map_info.find_next(pInfoEx) )
	{
		if(pInfoEx->e_type == EMT_Normal)
			mMapNames.push_back( pInfoEx->sz_map_name );
		else
			mInstanceNames.push_back( pInfoEx->sz_map_name );
	}

	// ��ȡ������ͼ
	LPCTSTR sz_born_map_name = (LPCTSTR)p_var->get_string(_T("name"), _T("born_map"));
	m_dw_born_map_id = get_tool()->crc32(sz_born_map_name);

	const tag_map_info* p_born_map_info = get_map_info(m_dw_born_map_id);
	ASSERT( VALID_POINT(p_born_map_info) );

	// ��ȡ������ͼ������
	m_n_born_position_num = p_var->get_int(_T("waypoint_num"), _T("born_map"));
	ASSERT( m_n_born_position_num > 0 );

	m_pv_born_position = new Vector3[m_n_born_position_num];
	m_p_yaw = new FLOAT[m_n_born_position_num];
	
	int nRebornX = p_var->get_int(_T("x"), _T("born_map"));
	int nRebornY = p_var->get_int(_T("y"), _T("born_map"));
	
	m_pv_born_position[0].x = nRebornX * TILE_SCALE;
	m_pv_born_position[0].z = nRebornY * TILE_SCALE;
	m_p_yaw[0] = 0.0f;

	//TCHAR sz_born_way_point_name[X_SHORT_NAME];
	//for(INT n = 0; n < m_n_born_position_num; n++)
	//{
	//	_stprintf(sz_temp, _T("waypoint_%d"), (n+1));
	//	get_fast_code()->memory_copy(sz_born_way_point_name, p_var->get_string(sz_temp, _T("born_map"), _T("")), sizeof(sz_born_way_point_name));
	//	DWORD dwBornWPID = get_tool()->crc32(sz_born_way_point_name);

	//	const tag_map_way_point_info_list* p_way_point_list = p_born_map_info->map_way_point_list.find(dwBornWPID);
	//	if( VALID_POINT(p_way_point_list) && p_way_point_list->list.size() > 0 )
	//	{
	//		tag_way_point_info info = p_way_point_list->list.front();
	//		m_pv_born_position[n] = info.v_pos;
	//		m_p_yaw[n] = info.f_yaw;
	//	}
	//	else
	//	{
	//		print_message(_T("Can't find the reborn waypoint of the map %s\r\n"), p_born_map_info->sz_map_name);
	//	}
	//}


	// ��ȡ������ͼ
	//LPCTSTR sz_prison_map_name = (LPCTSTR)p_var->get_string(_T("name"), _T("prison_map"));
	//m_dw_prison_map_id = get_tool()->crc32(sz_prison_map_name);

	//const tag_map_info* p_prison_map_info = get_map_info(m_dw_prison_map_id);
	//if( VALID_POINT(p_prison_map_info) )
	//{

	//	TCHAR sz_prison_way_point_name[X_SHORT_NAME];
	//	get_fast_code()->memory_copy(sz_prison_way_point_name, p_var->get_string(_T("inprisonpoint"), _T("prison_map"), _T("")), sizeof(sz_prison_way_point_name));
	//	DWORD dw_prison_way_point_id = get_tool()->crc32(sz_prison_way_point_name);

	//	const tag_map_way_point_info_list* p_way_point_list = p_prison_map_info->map_way_point_list.find(dw_prison_way_point_id);
	//	if( VALID_POINT(p_way_point_list) && p_way_point_list->list.size() > 0 )
	//	{
	//		tag_way_point_info info = p_way_point_list->list.front();
	//		m_putin_prison_point = info.v_pos;
	//	}
	//	else
	//	{
	//		print_message(_T("Can't find the putin prison waypoint of the map %s\r\n"), p_prison_map_info->sz_map_name);
	//	}

	//	get_fast_code()->memory_copy(sz_prison_way_point_name, p_var->get_string(_T("outprisonpoint"), _T("prison_map"), _T("")), sizeof(sz_prison_way_point_name));
	//	dw_prison_way_point_id = get_tool()->crc32(sz_prison_way_point_name);

	//	p_way_point_list = p_prison_map_info->map_way_point_list.find(dw_prison_way_point_id);
	//	if( VALID_POINT(p_way_point_list) && p_way_point_list->list.size() > 0 )
	//	{
	//		tag_way_point_info info = p_way_point_list->list.front();
	//		m_putout_prison_point = info.v_pos;
	//	}
	//	else
	//	{
	//		print_message(_T("Can't find the putout prison waypoint of the map %s\r\n"), p_prison_map_info->sz_map_name);
	//	}

	//}
	//else
	//{
	//	ASSERT(0);
	//	print_message(_T("Can't find the prison map %s\r\n"), sz_prison_map_name);
	//}

	////���ŵ�ͼ
	//LPCTSTR sz_yamun_map_name = (LPCTSTR)p_var->get_string(_T("name"), _T("yamun_map"));
	//m_dw_yamun_map_id = get_tool()->crc32(sz_yamun_map_name);

	//const tag_map_info* p_yamun_map_info = get_map_info(m_dw_yamun_map_id);
	//if( VALID_POINT(p_yamun_map_info) )
	//{
	//	TCHAR sz_yamun_way_point_name[X_SHORT_NAME];
	//	get_fast_code()->memory_copy(sz_yamun_way_point_name, p_var->get_string(_T("inyamunpoint"), _T("yamun_map"), _T("")), sizeof(sz_yamun_way_point_name));
	//	DWORD dw_yamun_way_point_id = get_tool()->crc32(sz_yamun_way_point_name);

	//	const tag_map_way_point_info_list* p_way_point_list = p_yamun_map_info->map_way_point_list.find(dw_yamun_way_point_id);
	//	if( VALID_POINT(p_way_point_list) && p_way_point_list->list.size() > 0 )
	//	{
	//		tag_way_point_info info = p_way_point_list->list.front();
	//		m_putin_yamun_point = info.v_pos;
	//	}
	//	else
	//	{
	//		print_message(_T("Can't find the putin yamun waypoint of the map %s\r\n"), p_yamun_map_info->sz_map_name);
	//	}
	//}


	// ���ִ�ĸ�������
	m_n_max_stable_instance_num = p_var->get_int(_T("num"), _T("initial_map"));


	SAFE_DELETE(p_var);
	SAFE_DELETE(p_logic);

	// �������е�MapInfo�����ɸ���MapMgr
	tag_map_info* pInfo  = NULL;
	m_map_map_info.reset_iterator();
	while( m_map_map_info.find_next(pInfo) )
	{
		MapMgr* p_may_manager = create_map_manager(pInfo);
		if( !VALID_POINT(p_may_manager) )
		{
			ASSERT(0);
			return FALSE;
		}

		m_map_map_manager.add(pInfo->dw_id, p_may_manager);
		if (EMT_Normal == pInfo->e_type)
		{
			ASSERT(VALID_POINT(p_may_manager->GetSingleMap()));
			p_may_manager->GetSingleMap()->init_creature_script();
		}
	}

	return TRUE;
}

//----------------------------------------------------------------------------
// �����ͼ������Ϣ
//----------------------------------------------------------------------------
BOOL map_creator::loag_map_info(LPCTSTR sz_file_map_name_, file_container* p_logic_)
{
	ASSERT( VALID_POINT(sz_file_map_name_) );
	if( FALSE == VALID_POINT(sz_file_map_name_) ) return FALSE;

	// ���ȶ�ȡMB�ļ�����ȡ��Ҫ�ĵ�ͼ����
	//TCHAR sz_full_file_map_name[MAX_PATH] = {_T('\0')};
 //   _sntprintf_s(sz_full_file_map_name, MAX_PATH, MAX_PATH, _T("data\\scene\\%s\\%s.block"), sz_file_map_name_, sz_file_map_name_);

	//HANDLE dw_handle = (HANDLE)g_world.get_virtual_filesys()->open_file(sz_full_file_map_name);
	//if( FALSE == VALID_VALUE(dw_handle) ) 
	//{
	//	
	//	return FALSE;
	//}
	
	tag_map_info* p_info = new tag_map_info;
	
	//if( FALSE == p_info )
	//{
	//	g_world.get_virtual_filesys()->close_file((DWORD)dw_handle);
	//	return FALSE;
	//}
	


	
	//������ײ��Ϣ
	//load_map_block((DWORD)dw_handle, p_info);
	//// ���ص�ͼ������
	//load_map_trigger(&header, (DWORD)dw_handle, p_info);
	//// ���ص�ͼ����
	//load_map_area(&header, (DWORD)dw_handle, p_info);
	//// ���ص�ͼ����
	//load_map_creature(&header, (DWORD)dw_handle, p_info);
	//// ���ص�ͼˢ�ֵ�
	//load_map_spawn_point(&header, (DWORD)dw_handle, p_info);
	//// ���ص�ͼ������Ч
	//load_map_trigger_effect(&header, (DWORD)dw_handle, p_info);
	//// ���ص�ͼ��
	//load_map_door(&header, (DWORD)dw_handle, p_info);

	// ��ֵpInfo�ĸ����߼�����
	p_info->dw_id					=	get_tool()->crc32(sz_file_map_name_);
	p_info->n_width				=	p_logic_->get_int(_T("SceneWidth"), sz_file_map_name_);
	p_info->n_height				=	p_logic_->get_int(_T("SceneHeight"), sz_file_map_name_);
	p_info->n_visit_distance				=	p_logic_->get_int(_T("ViewDistance"), sz_file_map_name_, VIS_DIST);
	p_info->e_type				=	(EMapType)p_logic_->get_dword(_T("SceneType"), sz_file_map_name_);
	p_info->e_normal_map_type		=	(ENormalMapType)p_logic_->get_dword(_T("NormalSceneType"), sz_file_map_name_);
	p_info->e_instance_grade		=	(E_map_instance_grade)p_logic_->get_dword(_T("InstanceRank"), sz_file_map_name_, EMIG_NOLIMIT);
	p_info->dw_weather_id			=	INVALID_VALUE;
	p_info->n_act_id				=	p_logic_->get_int(_T("ActivityID"), sz_file_map_name_, 0);
	p_info->b_raid					=	(BOOL)p_logic_->get_int(_T("raid"), sz_file_map_name_, 1);
	p_info->b_AddCooldownTime		=	(BOOL)p_logic_->get_int(_T("AddCoolDownReviveTime"), sz_file_map_name_, 1);
	_tcsncpy(p_info->sz_map_name, sz_file_map_name_, cal_tchar_array_num(p_info->sz_map_name) - 1);
	//gx add 2013.4.16 
	//��Ҹ����������Ϣ
	p_info->v_reborn_positon.x = (p_logic_->get_int(_T("RebornPosition_x"), sz_file_map_name_)) * TILE_SCALE;
	p_info->v_reborn_positon.y = 0.0f;
	p_info->v_reborn_positon.z = (p_logic_->get_int(_T("RebornPosition_y"), sz_file_map_name_)) * TILE_SCALE;
	//end
	
	//����ĵ�ͼid
	tstring szRebornMapName = p_logic_->get_string(_T("RebornMap"), sz_file_map_name_, _T("m1"));
	p_info->dw_out_map_id = get_tool()->crc32(szRebornMapName.c_str());
	p_info->v_out_map_position.x = (p_logic_->get_int(_T("Reborn_x"), sz_file_map_name_, 14)) * TILE_SCALE;
	p_info->v_out_map_position.y = 0;
	p_info->v_out_map_position.z = (p_logic_->get_int(_T("Reborn_z"), sz_file_map_name_, 15)) * TILE_SCALE;


	p_info->map_block_data.resize(p_info->n_width * p_info->n_height, 0);

	TCHAR sz_map_name[MAX_PATH] = {_T('\0')};
	_sntprintf_s(sz_map_name, MAX_PATH, MAX_PATH, _T("data\\scene\\%s\\%s.tmx"), sz_file_map_name_, sz_file_map_name_);


	XmlDocument doc;
	if( !doc.LoadFile(g_world.get_virtual_filesys(), sz_map_name) )
		return true;

	XmlHandle doc_hangle( &doc );

	//������ײ��Ϣ
	load_map_block(doc_hangle, p_info);
	// ���ص�ͼˢ�ֵ�
	load_map_creature(doc_hangle, p_info);
	// ���ص�ͼ������
	load_map_trigger(doc_hangle, p_info);
	// ���ص�ͼ������
	load_map_way_point(doc_hangle, p_info);
	//// ���ص�ͼ����
	load_map_area(doc_hangle, p_info);

	//! ����
	//p_info->n_weather_change_tick = p_logic_->get_int(_T("WeahterChangeTick"), sz_file_map_name_, INT_MAX);

	//int nWeahterNum = 0;
	//TCHAR szWeatherID[64] = {0};
	//nWeahterNum = p_logic_->get_int(_T("WeahterNum"), sz_file_map_name_, 0);
	//for(int n = 0; n < nWeahterNum; ++n){
	//	_stprintf(szWeatherID, _T("WeahterID%d"), n + 1);
	//	DWORD dwID =  0;
	//	dwID = p_logic_->get_dword(szWeatherID, sz_file_map_name_, 0);
	//	const tagWeatherProto* p =  AttRes::GetInstance()->GetWeather(dwID);
	//	if(VALID_POINT(p)){
	//		p_info->vec_weahter_ids.push_back(dwID);
	//	}
	//}
//gx modify 2013.4.16
	// �����
	/*TCHAR sz_reborn_name[X_SHORT_NAME];
	_tcsncpy(sz_reborn_name, p_logic_->get_string(_T("RebornWayPoint"), sz_file_map_name_), cal_tchar_array_num(sz_reborn_name) - 1);

	DWORD dw_reborn_id = get_tool()->crc32(sz_reborn_name);
	const tag_map_way_point_info_list* p_way_point_list = p_info->map_way_point_list.find(dw_reborn_id);
	if( VALID_POINT(p_way_point_list) && p_way_point_list->list.size() > 0 )
	{
		tag_way_point_info info = p_way_point_list->list.front();
		p_info->v_reborn_positon = info.v_pos;
	}
	else
	{
		print_message(_T("error reborn waypoint of the scene %s\r\n"), p_info->sz_map_name);
	}
	*/
//gx end
	m_map_map_info.add(p_info->dw_id, p_info);
	//DWORD dw_map_size = p_info->GetMemorySize();
	//g_world.get_log()->write_log(_T("Read %d BYTE memory from map<%s>!\n"), dw_map_size, p_info->sz_map_name);


	//g_world.get_virtual_filesys()->close_file((DWORD)dw_handle);

	return TRUE;
}

//----------------------------------------------------------------------------
// ���ص�ͼ������
//----------------------------------------------------------------------------
BOOL map_creator::load_map_way_point(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
	ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );

	if( p_header_->nNumWayPoint <= 0 )
	{
		return TRUE;
	}

	// ���ݵ������ƫ��������λ�ļ�
	g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwWayPointOffset, FILE_SEEK_SET);

	tagMapWayPoint temp;
	for(INT n = 0; n < p_header_->nNumWayPoint; n++)
	{
		g_world.get_virtual_filesys()->read_file(&temp, sizeof(temp), handle_);
		DWORD dw_way_point_id = get_tool()->crc32(temp.szName);

		tag_map_way_point_info_list* p_list = p_info_->map_way_point_list.find(dw_way_point_id);

		if( !VALID_POINT(p_list) )
		{
			p_list = new tag_map_way_point_info_list;
			p_list->dw_id = dw_way_point_id;
			p_info_->map_way_point_list.add(dw_way_point_id, p_list);
		}

		// ����һ��·����
		tag_way_point_info point;
		point.v_pos = temp.vPos;
		point.v_range = temp.vRange;
		point.f_yaw = temp.fYaw;
		point.dw_time = (DWORD)temp.fSuspend;

		p_list->list.push_back(point);
	}

	return TRUE;
}

//----------------------------------------------------------------------------
// ���ص�ͼ������
//----------------------------------------------------------------------------
//BOOL map_creator::load_map_trigger(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
//{
//	ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
//	ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );
//
//	if( p_header_->nNumTrigger <= 0 )
//	{
//		return TRUE;
//	}
//
//	g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwTriggerOffset, FILE_SEEK_SET);
//
//	DWORD dw_obj_id = INVALID_VALUE;
//	INT n_region_point_num = 0;
//	TCHAR sz_map_name[X_LONG_NAME] = {0};
//	TCHAR sz_way_point_name[X_SHORT_NAME] = {0};
//	vector<POINT> region;
//
//	for(INT n = 0; n < p_header_->nNumTrigger; n++)
//	{
//		// ���ȶ�ȡid�����
//		g_world.get_virtual_filesys()->read_file(&dw_obj_id, sizeof(dw_obj_id), handle_);
//
//		tag_map_trigger_info* p_trigger = p_info_->map_trigger.find(dw_obj_id);
//		ASSERT( !VALID_POINT(p_trigger) );
//
//		p_trigger = new tag_map_trigger_info;
//
//		// ����id������
//		p_trigger->dw_att_id = dw_obj_id;
//		g_world.get_virtual_filesys()->read_file(&p_trigger->e_type, sizeof(p_trigger->e_type), handle_);
//
//		// ��ȡ�����
//		g_world.get_virtual_filesys()->read_file(&n_region_point_num, sizeof(n_region_point_num), handle_);
//		ASSERT(n_region_point_num > 0);
//
//		region.clear();
//		region.resize(n_region_point_num);
//		Vector3 vPos;
//		POINT pt;
//		for(INT m = 0; m < n_region_point_num; m++)
//		{
//			g_world.get_virtual_filesys()->read_file(&vPos, sizeof(vPos), handle_);
//			pt.x = LONG(vPos.x / FLOAT(TILE_SCALE));
//			pt.y = LONG(vPos.z / FLOAT(TILE_SCALE));
//			region[m] = pt;		// ��������
//		}
//		p_trigger->h_polygon = CreatePolygonRgn((POINT*)&region[0], n_region_point_num, ALTERNATE);
//		ASSERT(NULL != p_trigger->h_polygon);
//
//		// ��ȡ��Χ��
//		g_world.get_virtual_filesys()->read_file(&p_trigger->box.max, sizeof(p_trigger->box.max), handle_);
//		g_world.get_virtual_filesys()->read_file(&p_trigger->box.min, sizeof(p_trigger->box.min), handle_);
//		// ��ȡ�߶�
//		g_world.get_virtual_filesys()->read_file(&p_trigger->f_height, sizeof(p_trigger->f_height), handle_);
//		
//		// ��ȡ��ͼ����
//		g_world.get_virtual_filesys()->read_file(sz_map_name, sizeof(sz_map_name), handle_);
//
//		// ��ȡ����������
//		g_world.get_virtual_filesys()->read_file(sz_way_point_name, sizeof(sz_way_point_name), handle_);
//
//		// ת����ID
//		p_trigger->dw_map_id = get_tool()->crc32(sz_map_name);
//		p_trigger->dw_way_point_id = get_tool()->crc32(sz_way_point_name);
//
//		// ��תһ��λ�ã�Խ��srciptName��
//		g_world.get_virtual_filesys()->seek_file(handle_, sizeof(TCHAR) * X_SHORT_NAME, FILE_SEEK_CURRENT);
//
//		// ��ȡֵ
//		g_world.get_virtual_filesys()->read_file(&p_trigger->dw_param, sizeof(p_trigger->dw_param), handle_);
//
//		// ��ת����һ��λ�ã�Խ��bFlag, bLock��
//		g_world.get_virtual_filesys()->seek_file(handle_, sizeof(bool)+sizeof(bool), FILE_SEEK_CURRENT);
//
//		// ��ȡ�������к�
//		g_world.get_virtual_filesys()->read_file(&p_trigger->dw_quest_id, sizeof(p_trigger->dw_quest_id), handle_);
//
//		// ��ת����һ��λ�ã�Խ��byReserved��
//		g_world.get_virtual_filesys()->seek_file(handle_, sizeof(BYTE)*124, FILE_SEEK_CURRENT);
//
//		// ��ӵ��б���
//		p_info_->map_trigger.add(p_trigger->dw_att_id, p_trigger);
//	}
//
//	return TRUE;
//}


//----------------------------------------------------------------------------
// ���ص�ͼ������
// gx add 2013.4.17
//----------------------------------------------------------------------------
BOOL map_creator::load_map_trigger_extra(LPCTSTR sz_file_map_name_, tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(sz_file_map_name_) );
	if( FALSE == VALID_POINT(sz_file_map_name_) ) return FALSE;

	TCHAR sz_full_file_map_name[MAX_PATH] = {_T('\0')};
	_sntprintf_s(sz_full_file_map_name, MAX_PATH, _T("data\\scene\\%s\\maptriggers.xml"), sz_file_map_name_);

	file_container* p_creature = new file_container;
	p_creature->load(g_world.get_virtual_filesys(), sz_full_file_map_name, "id");

	int i = 0;
	const int MAXCHARNUM = 10;
	tag_map_trigger_info_extra *pMapTrigger = NULL;
	TCHAR mapTriggerID[MAXCHARNUM] = {_T('\0')};
	DWORD elementNum = p_creature->getXMLRow();
	for (i;i < elementNum;i++)
	{
		pMapTrigger = new tag_map_trigger_info_extra;
		_sntprintf_s(mapTriggerID, MAXCHARNUM, _T("%d"), i);
		pMapTrigger->dw_att_id = p_creature->get_int(_T("id"), mapTriggerID);
		pMapTrigger->e_type = (EMapTrigger)(p_creature->get_int(_T("MapType"), mapTriggerID));

		//pMapTrigger->box.max.x = (EMapTrigger)(p_creature->get_int(_T("BoxMax_x"), mapTriggerID));
		//pMapTrigger->box.max.y = (EMapTrigger)(p_creature->get_int(_T("BoxMax_y"), mapTriggerID));
		//pMapTrigger->box.max.z = (EMapTrigger)(p_creature->get_int(_T("BoxMax_z"), mapTriggerID));
		//
		//pMapTrigger->box.min.x = (EMapTrigger)(p_creature->get_int(_T("BoxMin_x"), mapTriggerID));
		//pMapTrigger->box.min.y = (EMapTrigger)(p_creature->get_int(_T("BoxMin_y"), mapTriggerID));
		//pMapTrigger->box.min.z = (EMapTrigger)(p_creature->get_int(_T("BoxMin_z"), mapTriggerID));

		// ת����ID
		LPCTSTR sz_map_name = (LPCTSTR)(p_creature->get_string(_T("MapName"), mapTriggerID));
		pMapTrigger->dw_map_id = get_tool()->crc32(sz_map_name);
		LPCTSTR sz_waypoint_name = (LPCTSTR)(p_creature->get_string(_T("WayPointName"), mapTriggerID));
		pMapTrigger->dw_way_point_id = get_tool()->crc32(sz_waypoint_name);

		p_info_->map_trigger_extra.add(pMapTrigger->dw_att_id,pMapTrigger);
	}
	SAFE_DELETE(p_creature);
	return TRUE;
}

BOOL map_creator::load_map_trigger(XmlHandle& xmlHandle, tag_map_info* p_info_)
{
	XmlElement* xml_element = xmlHandle.FirstChildElement().FirstChildElement("objectgroup").Element();
	while(xml_element)
	{
		LPCSTR sz = xml_element->Attribute("name");
		if (strcmp( sz, "trigger") == 0)
		{
			break;
		}
		xml_element = xml_element->NextSiblingElement("objectgroup");
	}
	
	if (!VALID_POINT(xml_element))
		return false;

	int nY = 0;
	int nX = 0;
	int nWidth = 0;
	int nHeight = 0;
	std::string sz_map_name;
	std::string sz_way_point_name;

	tag_map_trigger_info *pMapTrigger = NULL;
	xml_element = xml_element->FirstChildElement();
	while(xml_element)
	{
		pMapTrigger = new tag_map_trigger_info;

		xml_element->Attribute("name", (int*)(&pMapTrigger->dw_att_id));
		xml_element->Attribute("type", (int*)(&pMapTrigger->e_type));
		xml_element->Attribute("x", &nX);
		xml_element->Attribute("y", &nY);
		xml_element->Attribute("width", &nWidth);
		xml_element->Attribute("height", &nHeight);

		pMapTrigger->boxSize.x = nWidth;
		pMapTrigger->boxSize.z = nHeight;

		pMapTrigger->boxPos.x = nX;
		pMapTrigger->boxPos.z = p_info_->n_height * TILE_SCALE - nY - pMapTrigger->boxSize.z;
		

		
		if (!VALID_POINT(xml_element->FirstChildElement()))
			return false;

		XmlElement* xmlPropetry = xml_element->FirstChildElement()->FirstChildElement();

		if (!VALID_POINT(xmlPropetry))
			return false;

		sz_map_name = xmlPropetry->Attribute("value");

		if (!VALID_POINT(xmlPropetry->NextSiblingElement()))
			return false;

		sz_way_point_name = xmlPropetry->NextSiblingElement()->Attribute("value");
		
		
		pMapTrigger->dw_map_id = get_tool()->crc32(get_tool()->unicode8_to_unicode(sz_map_name.c_str()));
		pMapTrigger->dw_way_point_id = get_tool()->crc32(get_tool()->unicode8_to_unicode(sz_way_point_name.c_str()));

		
		p_info_->map_trigger.add(pMapTrigger->dw_att_id, pMapTrigger);

		xml_element = xml_element->NextSiblingElement();
	}

	return true;
}


BOOL map_creator::load_map_way_point(XmlHandle& xmlHandle, tag_map_info* p_info_)
{
	XmlElement* xml_element = xmlHandle.FirstChildElement().FirstChildElement("objectgroup").Element();
	while(xml_element)
	{
		LPCSTR sz = xml_element->Attribute("name");
		if (strcmp( sz, "waypoint") == 0)
		{
			break;
		}
		xml_element = xml_element->NextSiblingElement("objectgroup");
	}

	if (!VALID_POINT(xml_element))
		return false;
	

	int nY = 0;
	int nX = 0;

	std::string sz_name;
	xml_element = xml_element->FirstChildElement();
	while(xml_element)
	{
		
		sz_name = xml_element->Attribute("name");
		//TCHAR tName[X_SHORT_NAME] = _T("");
		//get_tool()->ansi_to_unicode(sz_name.c_str());
		DWORD dw_way_point_id = get_tool()->crc32(get_tool()->unicode8_to_unicode(sz_name.c_str()));
	
		tag_map_way_point_info_list* p_list = p_info_->map_way_point_list.find(dw_way_point_id);
		
		if( !VALID_POINT(p_list) )
		{
			p_list = new tag_map_way_point_info_list;
			p_list->dw_id = dw_way_point_id;
			p_info_->map_way_point_list.add(dw_way_point_id, p_list);
		}
			
		tag_way_point_info point;
		xml_element->Attribute("x", &nX);
		xml_element->Attribute("y", &nY);
		
		point.v_pos.x = nX;
		point.v_pos.z = p_info_->n_height * TILE_SCALE - nY;

		p_list->list.push_back(point);
		xml_element = xml_element->NextSiblingElement();

	}
	return true;
}

//----------------------------------------------------------------------------
// ��ȡ��ͼ������Ϣ
//----------------------------------------------------------------------------
BOOL map_creator::load_map_area(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
{
	//ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
	//ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );

	//if( p_header_->nNumMapRect <= 0 )
	//{
	//	return TRUE;
	//}

	//g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwMapRectOffset, FILE_SEEK_SET);

	//DWORD dw_obj_id = INVALID_VALUE;
	//EMapArea e_type = EMA_Null;
	//INT n_region_point_num = 0;
	//vector<POINT> region;

	//for(INT n = 0; n < p_header_->nNumMapRect; n++)
	//{
	//	g_world.get_virtual_filesys()->read_file(&dw_obj_id, sizeof(dw_obj_id), handle_);
	//	g_world.get_virtual_filesys()->read_file(&e_type, sizeof(e_type), handle_);

	//	// ��ʾ�������������Ҫ���أ���ת����һ��
	//	if( e_type == EMA_Null || e_type == EMA_Display )
	//	{
	//		g_world.get_virtual_filesys()->read_file(&n_region_point_num, sizeof(n_region_point_num), handle_);
	//		ASSERT( n_region_point_num > 0 );

	//		INT n_offset = sizeof(Vector3)*n_region_point_num + sizeof(AABBox) + sizeof(FLOAT)
	//						+ sizeof(bool) + sizeof(bool) + sizeof(BYTE)*128;

	//		g_world.get_virtual_filesys()->seek_file(handle_, n_offset, FILE_SEEK_CURRENT);
	//		continue;
	//	}

	//	tag_map_area_info* p_area = new tag_map_area_info;

	//	// ����ID������
	//	p_area->dw_att_id = dw_obj_id;
	//	p_area->e_type = e_type;

	//	// ���������
	//	g_world.get_virtual_filesys()->read_file(&n_region_point_num, sizeof(n_region_point_num), handle_);
	//	ASSERT(n_region_point_num > 0);
	//	
	//	region.clear();
	//	region.resize(n_region_point_num);
	//	Vector3 v_pos;
	//	POINT pt;
	//	for(INT m = 0; m < n_region_point_num; m++)
	//	{
	//		g_world.get_virtual_filesys()->read_file(&v_pos, sizeof(v_pos), handle_);
	//		pt.x = LONG(v_pos.x / FLOAT(TILE_SCALE));
	//		pt.y = LONG(v_pos.z / FLOAT(TILE_SCALE));
	//		region[m] = pt;
	//	}
	//	p_area->h_polygon = CreatePolygonRgn((POINT*)&region[0], n_region_point_num, ALTERNATE);
	//	ASSERT(NULL != p_area->h_polygon);

	//	// ���������
	//	g_world.get_virtual_filesys()->read_file(&p_area->box.max, sizeof(p_area->box.max), handle_);
	//	g_world.get_virtual_filesys()->read_file(&p_area->box.min, sizeof(p_area->box.min), handle_);
	//	// ����߶�
	//	g_world.get_virtual_filesys()->read_file(&p_area->f_height, sizeof(p_area->f_height), handle_);

	//	// ��ת����һ����
	//	g_world.get_virtual_filesys()->seek_file(handle_, sizeof(bool)+sizeof(bool)+sizeof(BYTE)*128, FILE_SEEK_CURRENT);

	//	// �������ͼ��뵽��ͬ���б�
	//	switch (e_type)
	//	{
	//	case EMA_Safe:
	//		p_info_->map_safe_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_PVP:
	//		p_info_->map_pvp_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Stall:
	//		p_info_->map_stall_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Prison:
	//		p_info_->map_prison_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Script:
	//		p_info_->map_script_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Common:
	//		p_info_->map_common_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_RealSafe:
	//		p_info_->map_real_safe_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_GuildBattle:
	//		p_info_->map_guild_battle.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_NoPunish:
	//		p_info_->map_no_punish_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Hang:
	//		p_info_->map_hang_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_KongFu:
	//		p_info_->map_KongFu_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Comprehend:
	//		p_info_->map_Comprehend_area.add(dw_obj_id, p_area);
	//		break;
	//	case EMA_Dancing:
	//		p_info_->map_Dancing_area.add(dw_obj_id, p_area);
	//		break;

	//	default:
	//		SAFE_DELETE(p_area);
	//		break;
	//	}

	//}
	return TRUE;
}
//�����ͼ��
BOOL map_creator::load_map_area(XmlHandle& xmlHandle, tag_map_info* p_info_)
{
	XmlElement* xml_element = xmlHandle.FirstChildElement().FirstChildElement("objectgroup").Element();
	while(xml_element)
	{
		LPCSTR sz = xml_element->Attribute("name");
		if (strcmp( sz, "area") == 0)
		{
			break;
		}
		xml_element = xml_element->NextSiblingElement("objectgroup");
	}

	if (!VALID_POINT(xml_element))
		return false;

	DWORD dw_obj_id = 0;
	//EMapArea e_type = EMA_Null;
	int nY = 0;
	int nX = 0;
	int nWidth = 0;
	int nHeight = 0;

	tag_map_area_info* p_area = NULL;
	xml_element = xml_element->FirstChildElement();
	while(xml_element)
	{
		p_area = new tag_map_area_info;
		
		p_area->dw_att_id = dw_obj_id;

		xml_element->Attribute("type", (int*)(&p_area->e_type));
		xml_element->Attribute("x", &nX);
		xml_element->Attribute("y", &nY);
		xml_element->Attribute("width", &nWidth);
		xml_element->Attribute("height", &nHeight);

		p_area->boxSize.x = nWidth;
		p_area->boxSize.z = nHeight;

		p_area->boxPos.x = nX;
		p_area->boxPos.z = p_info_->n_height * TILE_SCALE - nY - p_area->boxSize.z;


		// �������ͼ��뵽��ͬ���б�
		switch (p_area->e_type)
		{
		case EMA_Safe:
			p_info_->map_safe_area.add(dw_obj_id, p_area);
			break;
		case EMA_PVP:
			p_info_->map_pvp_area.add(dw_obj_id, p_area);
			break;
		case EMA_Stall:
			p_info_->map_stall_area.add(dw_obj_id, p_area);
			break;
		case EMA_Prison:
			p_info_->map_prison_area.add(dw_obj_id, p_area);
			break;
		case EMA_Script:
			p_info_->map_script_area.add(dw_obj_id, p_area);
			break;
		case EMA_Common:
			p_info_->map_common_area.add(dw_obj_id, p_area);
			break;
		case EMA_RealSafe:
			p_info_->map_real_safe_area.add(dw_obj_id, p_area);
			break;
		case EMA_GuildBattle:
			p_info_->map_guild_battle.add(dw_obj_id, p_area);
			break;
		case EMA_NoPunish:
			p_info_->map_no_punish_area.add(dw_obj_id, p_area);
			break;
		case EMA_Hang:
			p_info_->map_hang_area.add(dw_obj_id, p_area);
			break;
		case EMA_KongFu:
			p_info_->map_KongFu_area.add(dw_obj_id, p_area);
			break;
		case EMA_Comprehend:
			p_info_->map_Comprehend_area.add(dw_obj_id, p_area);
			break;
		case EMA_Dancing:
			p_info_->map_Dancing_area.add(dw_obj_id, p_area);
			break;

		default:
			SAFE_DELETE(p_area);
			break;
		}

		xml_element = xml_element->NextSiblingElement();

		dw_obj_id++;
	}


}
//----------------------------------------------------------------------------
// ��ȡ��ͼ����
//----------------------------------------------------------------------------
#if 0
BOOL map_creator::load_map_creature(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
	ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );

	if( p_header_->nNumNPC <= 0 )
	{
		return TRUE;
	}

	// ���ݵ�ͼ�����ƫ��������λ�ļ�
	g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwNpcOffset, FILE_SEEK_SET);

	tagMapNPC temp;
	for(INT n = 0; n < p_header_->nNumNPC; n++)
	{
		g_world.get_virtual_filesys()->read_file(&temp, sizeof(temp), handle_);

		tagMapCreatureInfo* p_creature = p_info_->map_creature_info.find(temp.dwObjID);
		ASSERT( !VALID_POINT(p_creature) );

		p_creature = new tagMapCreatureInfo;
		p_creature->dw_att_id = temp.dwObjID;
		p_creature->dw_type_id = temp.dw_data_id;
		p_creature->v_pos = temp.vPos;
		p_creature->f_yaw = 270 - temp.fYaw;
		p_creature->b_collide = temp.bCollide;
		p_creature->list_patrol = p_info_->map_way_point_list.find(get_tool()->crc32(temp.szName));

		// ���뵽�б���
		p_info_->map_creature_info.add(p_creature->dw_att_id, p_creature);
	}

	return TRUE;
}
#endif
//----------------------------------------------------------------------------
// ��ȡ��ͼ����
// gx add 2013.4.16
//----------------------------------------------------------------------------
BOOL map_creator::load_map_creature(XmlHandle& xmlHandle, tag_map_info* p_info_)
{
	XmlElement* xml_element = xmlHandle.FirstChildElement().FirstChildElement("objectgroup").Element();
	while(xml_element)
	{
		LPCSTR sz = xml_element->Attribute("name");
		if (strcmp( sz, "unit") == 0)
		{
			break;
		}
		xml_element = xml_element->NextSiblingElement("objectgroup");
	}
	
	if (!VALID_POINT(xml_element))
		return false;


	tagMapMonsterInfo *pmonster = NULL;
	xml_element = xml_element->FirstChildElement();
	int i = 0;
	while(xml_element)
	{
		pmonster = new tagMapMonsterInfo;
		
		int nID;
		int nx;
		int ny;
		xml_element->Attribute("name", &nID);
		xml_element->Attribute("x", &nx);
		xml_element->Attribute("y", &ny);

		pmonster->dw_type_id = nID;
		pmonster->v_pos.x = nx;
		pmonster->v_pos.y = 0;
		pmonster->v_pos.z = p_info_->n_height * TILE_SCALE - ny;

		p_info_->map_monster_info.add(i, pmonster);
		xml_element = xml_element->NextSiblingElement();
		i++;
	}
	//ASSERT( VALID_POINT(sz_file_map_name_) );
	//if( FALSE == VALID_POINT(sz_file_map_name_) ) return FALSE;

	//TCHAR sz_full_file_map_name[MAX_PATH] = {_T('\0')};
	//_sntprintf_s(sz_full_file_map_name, MAX_PATH, _T("data\\scene\\%s\\monstrposition.xml"), sz_file_map_name_);

	//file_container* p_creature = new file_container;
	//p_creature->load(g_world.get_virtual_filesys(), sz_full_file_map_name, "id");

	//int i = 0;
	//const int MAXCHARNUM = 10;
	//tagMapMonsterInfo *pmonster = NULL;
	//TCHAR monsterID[MAXCHARNUM] = {_T('\0')};
	//DWORD elementNum = p_creature->getXMLRow();
	//for (i;i < elementNum;i++)
	//{
	//	pmonster = new tagMapMonsterInfo;
	//	_sntprintf_s(monsterID, MAXCHARNUM, _T("%d"), i);
	//	pmonster->dw_type_id = p_creature->get_int(_T("monster_type_id"), monsterID);
	//	pmonster->v_pos.x = (p_creature->get_int(_T("position_x"), monsterID)) * TILE_SCALE;
	//	pmonster->v_pos.y = 0;
	//	pmonster->v_pos.z = (p_creature->get_int(_T("position_y"), monsterID)) * TILE_SCALE;
	//	pmonster->f_yaw = p_creature->get_float(_T("orientation"),monsterID);
	//	p_info_->map_monster_info.add(pmonster->dw_type_id,pmonster);
	//}
	//SAFE_DELETE(p_creature);
	return TRUE;
}
//----------------------------------------------------------------------------
// ��ȡˢ�ֵ���Ϣ
//----------------------------------------------------------------------------
BOOL map_creator::load_map_spawn_point(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
	ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );

	if( p_header_->nNumSpawnPoint <= 0 )
	{
		return TRUE;
	}
	
	

	// �Ȱ�ˢ�ֵ���չ��Ϣ������
	BOOL b_have_ex_info = FALSE;
	std::vector<tagMapSpawnPointExtInfo> vec_map_spawn_ex_info;
	if (p_header_->dwSpawnPointExtInfoOffset != 0)
	{
		b_have_ex_info = TRUE;

		g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwSpawnPointExtInfoOffset, FILE_SEEK_SET);

		tagMapSpawnPointExtInfo temp;
		for(INT n = 0; n < p_header_->nNumSpawnPoint; n++)
		{
			g_world.get_virtual_filesys()->read_file(&temp, sizeof(temp), handle_);
			vec_map_spawn_ex_info.push_back(temp);
		}

	}

	// ���ݵ�ͼ�����ƫ��������λ�ļ�
	g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwSpawnPointOffset, FILE_SEEK_SET);

	tagMapSpawnPoint temp;
	for(INT n = 0; n < p_header_->nNumSpawnPoint; n++)
	{
		g_world.get_virtual_filesys()->read_file(&temp, sizeof(temp), handle_);

		tag_map_spawn_point_info* p_spawn_point = p_info_->map_spawn_point.find(temp.dwObjID);
		ASSERT( !VALID_POINT(p_spawn_point) );

		p_spawn_point = new tag_map_spawn_point_info;
		p_spawn_point->dw_att_id = temp.dwObjID;
		p_spawn_point->dw_spawn_point_id = temp.dwGroupID;
		p_spawn_point->n_level_increase = temp.nLevelInc;
		p_spawn_point->b_collide = temp.bCollide;
		p_spawn_point->v_pos = temp.vPos;
		wstring str = get_tool()->ansi_to_unicode(temp.navName);
		DWORD dw_way_point_id = get_tool()->crc32(str.c_str());
		p_spawn_point->list_patrol = p_info_->map_way_point_list.find(dw_way_point_id);
		
		// ���뵽�б���
		p_info_->map_spawn_point.add(p_spawn_point->dw_att_id, p_spawn_point);
		
		if (b_have_ex_info)
		{
			p_spawn_point->dw_group_id = vec_map_spawn_ex_info[n].dwSpawnPointGroupID;
			p_spawn_point->f_angle = vec_map_spawn_ex_info[n].fAngle;
		}

		if (p_spawn_point->dw_group_id == 0)
			continue; 

		tag_map_spawn_point_list* p_spawn_list = p_info_->map_spawn_point_list.find(p_spawn_point->dw_group_id);
		if (!VALID_POINT(p_spawn_list))
		{
			p_spawn_list = new tag_map_spawn_point_list;
			p_spawn_list->dw_id = p_spawn_point->dw_group_id;
			p_info_->map_spawn_point_list.add(p_spawn_list->dw_id, p_spawn_list);
		}
		p_spawn_list->list.add(p_spawn_point->dw_att_id, p_spawn_point);
	}

	return TRUE;
}

//----------------------------------------------------------------------------
// ��ȡ��ͼ��Ч��Ϣ
//----------------------------------------------------------------------------
BOOL map_creator::load_map_trigger_effect(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
	ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );

	if( p_header_->nNumMapTriggerEffect <= 0 )
	{
		return TRUE;
	}

	// ���ݵ�ͼ�����ƫ��������λ�ļ�
	g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwMapTriggerEffectOffset, FILE_SEEK_SET);

	tagMapTriggerEffect temp;
	for(INT n = 0; n < p_header_->nNumMapTriggerEffect; n++)
	{
		g_world.get_virtual_filesys()->read_file(&temp, sizeof(temp), handle_);

		tagMapTriggerEffect* p_trigger_effect = p_info_->map_trigger_effect.find(temp.dwObjID);
		ASSERT( !VALID_POINT(p_trigger_effect) );

		p_trigger_effect = new tagMapTriggerEffect;
		p_trigger_effect->dwObjID = temp.dwObjID;

		// ���뵽�б���
		p_info_->map_trigger_effect.add(p_trigger_effect->dwObjID, p_trigger_effect);
	}

	return TRUE;
}

BOOL map_creator::load_map_door(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(p_header_) && VALID_VALUE(handle_) && VALID_POINT(p_info_) );
	ASSERT( VALID_POINT(g_world.get_virtual_filesys()) );

	if( p_header_->nNumMapDoor <= 0 )
	{
		return TRUE;
	}

	// ���ݵ�ͼ�����ƫ��������λ�ļ�
	g_world.get_virtual_filesys()->seek_file(handle_, p_header_->dwMapDoorOffset, FILE_SEEK_SET);

	tagMapDoor temp;
	for(INT n = 0; n < p_header_->nNumMapDoor; n++)
	{
		g_world.get_virtual_filesys()->read_file(&temp, sizeof(temp), handle_);

		tag_map_door_info* p_door = p_info_->map_door_info.find(temp.dwObjID);
		ASSERT( !VALID_POINT(p_door) );

		p_door = new tag_map_door_info;
		p_door->dw_att_id = temp.dwObjID;
		p_door->dw_type_id = temp.dw_data_id;
		p_door->v_pos = temp.vPos;
		p_door->f_yaw = 270 - temp.fYaw;
		p_door->b_init_open_state = temp.bInitState;

		// ���뵽�б���
		p_info_->map_door_info.add(p_door->dw_att_id, p_door);
	}

	return TRUE;
}

BOOL map_creator::load_map_block(DWORD handle, tag_map_info* p_info_)
{
	int tiledHNum = 0;
	int tiledVNum = 0;
	int tiledW = 0;
	int tiledH = 0;

	g_world.get_virtual_filesys()->read_file(&tiledHNum, sizeof(p_info_->n_width	), handle);
	g_world.get_virtual_filesys()->read_file(&tiledVNum, sizeof(p_info_->n_height), handle);
	g_world.get_virtual_filesys()->read_file(&tiledW, sizeof(tiledW	), handle);
	g_world.get_virtual_filesys()->read_file(&tiledH, sizeof(tiledH	), handle);

	int nTitleNum = tiledHNum * tiledVNum; 
	for (int i = 0; i < nTitleNum; i++)
	{
		int nData = 0;
		g_world.get_virtual_filesys()->read_file(&nData, sizeof(nData), handle);
		p_info_->map_block_data.push_back(nData);

	}

	return TRUE;
}

BOOL map_creator::load_map_block(XmlHandle& xmlHandle, tag_map_info* p_info_)
{
	XmlElement* xml_element = xmlHandle.FirstChildElement().FirstChildElement("layer").Element();
	while(xml_element)
	{
		LPCSTR sz = xml_element->Attribute("name");
		if (strcmp( sz, "block") == 0)
		{
			break;
		}
		xml_element = xml_element->NextSiblingElement("layer");
	}

	

	BYTE* pData = NULL;
	if (!VALID_POINT(xml_element->FirstChild()))
		return false;

	if(!VALID_POINT(xml_element->FirstChild()->FirstChild()))
		return false;

	LPCSTR strData = xml_element->FirstChild()->FirstChild()->Value();
	int dataLen = strlen(strData);
	int decodeLen = base64Decode((BYTE*)strData, dataLen, &pData);

	DWORD nOutSize = p_info_->n_width * p_info_->n_height * 4;

	BYTE* outbuffer = new BYTE[nOutSize];

	if (Z_OK != uncompress(outbuffer, &nOutSize, pData, decodeLen))
	{
		SAFE_DELETE_ARRAY(outbuffer);
		SAFE_DELETE_ARRAY(pData);
		return false;
	}

	for (int i = 0; i < nOutSize; i+=4)
	{
		unsigned int nData = outbuffer[i] |
					outbuffer[i + 1] << 8 |
					outbuffer[i + 2] << 16 |
					outbuffer[i + 3] << 24;
		
		//int nBlock = nData & 0x1ffff;

		p_info_->map_block_data[i/4] = nData;

	}

	SAFE_DELETE_ARRAY(outbuffer);
	SAFE_DELETE_ARRAY(pData);
	return true;
}

//----------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------
VOID map_creator::destroy()
{
	// ɾ�����е�MapMgr
	m_map_map_manager.reset_iterator();
	MapMgr* p_map_manager = NULL;

	while( m_map_map_manager.find_next(p_map_manager) )
	{
		SAFE_DELETE(p_map_manager);
	}

	m_map_map_manager.clear();

	// ɾ�����еĵ�ͼ������Ϣ
	m_map_map_info.reset_iterator();
	tag_map_info* p_info = NULL;

	while( m_map_map_info.find_next(p_info) )
	{
		// ���ˢ�ֵ�
		tag_map_spawn_point_info* p_spawn_point = NULL;
		p_info->map_spawn_point.reset_iterator();
		while( p_info->map_spawn_point.find_next(p_spawn_point) )
		{
			SAFE_DELETE(p_spawn_point);
		}
		p_info->map_spawn_point.clear();

		// ��յ�ͼ����
		tagMapCreatureInfo* p_map_creature_info = NULL;
		p_info->map_creature_info.reset_iterator();
		while( p_info->map_creature_info.find_next(p_map_creature_info) )
		{
			SAFE_DELETE(p_map_creature_info);
		}
		p_info->map_creature_info.clear();
		//gx add 2013.4.16
		tagMapMonsterInfo* p_map_monster_info = NULL;
		p_info->map_monster_info.reset_iterator();
		while( p_info->map_monster_info.find_next(p_map_monster_info) )
		{
			SAFE_DELETE(p_map_monster_info);
		}
		p_info->map_monster_info.clear();
		//end
		// �������
		tag_map_area_info* p_area = NULL;

		p_info->map_safe_area.reset_iterator();
		while( p_info->map_safe_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_safe_area.clear();

		p_info->map_hang_area.reset_iterator();
		while( p_info->map_hang_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_hang_area.clear();

		p_info->map_pvp_area.reset_iterator();
		while( p_info->map_pvp_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_pvp_area.clear();

		p_info->map_stall_area.reset_iterator();
		while( p_info->map_stall_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_stall_area.clear();

		p_info->map_prison_area.reset_iterator();
		while( p_info->map_prison_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_prison_area.clear();

		p_info->map_script_area.reset_iterator();
		while( p_info->map_script_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_script_area.clear();

		p_info->map_common_area.reset_iterator();
		while( p_info->map_common_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_common_area.clear();

		p_info->map_real_safe_area.reset_iterator();
		while( p_info->map_real_safe_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_real_safe_area.clear();

		p_info->map_guild_battle.reset_iterator();
		while( p_info->map_guild_battle.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_guild_battle.clear();

		p_info->map_no_punish_area.reset_iterator();
		while( p_info->map_no_punish_area.find_next(p_area) )
		{
			SAFE_DELETE(p_area);
		}
		p_info->map_no_punish_area.clear();


		// ��մ�����
		tag_map_trigger_info* p_trigger = NULL;
		p_info->map_trigger.reset_iterator();
		while( p_info->map_trigger.find_next(p_trigger) )
		{
			SAFE_DELETE(p_trigger);
		}
		p_info->map_trigger.clear();

		// ��յ�����
		tag_map_way_point_info_list* pList = NULL;
		p_info->map_way_point_list.reset_iterator();
		while( p_info->map_way_point_list.find_next(pList) )
		{
			SAFE_DELETE(pList);
		}
		p_info->map_way_point_list.clear();

		// ��ճ�����Ч
		tagMapTriggerEffect* p_effect = NULL;
		p_info->map_trigger_effect.reset_iterator();
		while (p_info->map_trigger_effect.find_next(p_effect))
		{
			SAFE_DELETE(p_effect);
		}
		p_info->map_trigger_effect.clear();
		
		// �����
		tag_map_door_info* p_door = NULL;
		p_info->map_door_info.reset_iterator();
		while (p_info->map_door_info.find_next(p_door))
		{
			SAFE_DELETE(p_door);
		}
		p_info->map_door_info.clear();

		// ���ˢ�ֵ���
		tag_map_spawn_point_list* p_point_list = NULL;
		p_info->map_spawn_point_list.reset_iterator();
		while(p_info->map_spawn_point_list.find_next(p_point_list))
		{
			p_point_list->list.clear();
			SAFE_DELETE(p_point_list);
		}
		p_info->map_spawn_point_list.clear();


		// ��յ�ͼ��̬���Խṹ
		SAFE_DELETE(p_info);
	}

	m_map_map_info.clear();

	// ɾ���������ڴ�
	SAFE_DELETE_ARRAY(m_pv_born_position);
	SAFE_DELETE_ARRAY(m_p_yaw);

	// ��ע�����е�ע����
	un_register_all();

	
}

//--------------------------------------------------------------------------------
// ע���ͼ��������
//--------------------------------------------------------------------------------
VOID map_creator::register_factory_map()
{
	factory_map.register_class(wrap<Map>(),					EMT_Normal);
	factory_map.register_class(wrap<map_instance_normal>(),	EMT_Instance);
	factory_map.register_class(wrap<MapInstanceStable>(),	EMT_System);
	factory_map.register_class(wrap<MapInstanceScript>(),	EMT_ScriptCreate);
	factory_map.register_class(wrap<map_instance_guild>(),		EMT_Guild);
	factory_map.register_class(wrap<map_instance_pvp>(),		EMT_PVP);
	factory_map.register_class(wrap<map_instance_1v1>(),		EMT_1v1);
	factory_map.register_class(wrap<MapInstanceSBK>(),			EMT_SBK);
	factory_map.register_class(wrap<map_instance_pvp_biwu>(),			EMT_PVP_BIWU);
	factory_map.register_class(wrap<map_instance_battle>(),		EMT_Battle);
}

//--------------------------------------------------------------------------------
// ע���ͼ������������
//--------------------------------------------------------------------------------
VOID map_creator::register_factory_map_restrict()
{
	factory_restrict.register_class(wrap<MapRestrictNormal>(),		EMT_Normal);
	factory_restrict.register_class(wrap<MapRestrictInstance>(),		EMT_Instance);
	factory_restrict.register_class(wrap<MapRestrictStable>(),		EMT_System);
	factory_restrict.register_class(wrap<MapRestrictScript>(),		EMT_ScriptCreate);
	factory_restrict.register_class(wrap<MapRestrictGuild>(),		EMT_Guild);	
	factory_restrict.register_class(wrap<MapRestrictPvP>(),			EMT_PVP);
	factory_restrict.register_class(wrap<MapRestrict1v1>(),			EMT_1v1);
	factory_restrict.register_class(wrap<MapRestrictSBK>(),			EMT_SBK);
	factory_restrict.register_class(wrap<MapRestrictPvPBiWu>(),			EMT_PVP_BIWU);
	factory_restrict.register_class(wrap<MapRestrictBattle>(),	EMT_Battle);
}

//--------------------------------------------------------------------------------
// ��ע��ӿ�
//--------------------------------------------------------------------------------
VOID map_creator::un_register_all()
{
	factory_map.unregister_class(wrap<Map>(),				EMT_Normal);
	factory_map.unregister_class(wrap<map_instance_normal>(),	EMT_Instance);
	factory_map.unregister_class(wrap<MapInstanceStable>(),	EMT_System);
	factory_map.unregister_class(wrap<MapInstanceScript>(),	EMT_ScriptCreate);
	factory_map.unregister_class(wrap<map_instance_guild>(),	EMT_Guild);
	factory_map.unregister_class(wrap<map_instance_pvp>(),		EMT_PVP);
	factory_map.unregister_class(wrap<map_instance_1v1>(),		EMT_1v1);
	factory_map.unregister_class(wrap<MapInstanceSBK>(),		EMT_SBK);
	factory_map.unregister_class(wrap<map_instance_pvp_biwu>(),		EMT_PVP_BIWU);
	factory_map.unregister_class(wrap<map_instance_battle>(),	EMT_Battle);

	factory_restrict.unregister_class(wrap<MapRestrictNormal>(),		EMT_Normal);
	factory_restrict.unregister_class(wrap<MapRestrictInstance>(),	EMT_Instance);
	factory_restrict.unregister_class(wrap<MapRestrictStable>(),		EMT_System);
	factory_restrict.unregister_class(wrap<MapRestrictScript>(),		EMT_ScriptCreate);
	factory_restrict.unregister_class(wrap<MapRestrictGuild>(),		EMT_Guild);
	factory_restrict.unregister_class(wrap<MapRestrictPvP>(),			EMT_PVP);
	factory_restrict.unregister_class(wrap<MapRestrict1v1>(),			EMT_1v1);
	factory_restrict.unregister_class(wrap<MapInstanceSBK>(),			EMT_SBK);
	factory_restrict.unregister_class(wrap<MapRestrictPvPBiWu>(),			EMT_PVP_BIWU);

	factory_restrict.unregister_class(wrap<MapRestrictBattle>(),		EMT_Battle);
}

//--------------------------------------------------------------------------------
// ������ͨ��ͼ
//--------------------------------------------------------------------------------
Map* map_creator::create_factory_map(const tag_map_info* p_info_)
{
	if( !VALID_POINT(p_info_) || EMT_Normal != p_info_->e_type ) return NULL;

	return factory_map.create_class(p_info_->e_type);
}

//--------------------------------------------------------------------------------
// ���ɸ�����ͼ
//--------------------------------------------------------------------------------
map_instance* map_creator::create_factory_map_instance(const tag_map_info* p_info_)
{
	if( !VALID_POINT(p_info_) || EMT_Normal == p_info_->e_type ) return NULL;

	return static_cast<map_instance*>(factory_map.create_class(p_info_->e_type));
}

//---------------------------------------------------------------------------------
// �ͷŵ�ͼ
//---------------------------------------------------------------------------------
VOID map_creator::destroy_factory_map(Map* p_map_)
{
	if( !VALID_POINT(p_map_) ) return;

	const tag_map_info* p_info = p_map_->get_map_info();
	if( !VALID_POINT(p_info) ) return;

	factory_map.destroy_class(p_map_, p_info->e_type);
}

//--------------------------------------------------------------------------------
// ���ɵ�ͼ����
//--------------------------------------------------------------------------------
MapRestrict* map_creator::create_factory_map_restrict(const tag_map_info* p_info_)
{
	if( !VALID_POINT(p_info_) ) return NULL;

	return factory_restrict.create_class(p_info_->e_type);
}

//--------------------------------------------------------------------------------
// �ͷŵ�ͼ����
//--------------------------------------------------------------------------------
VOID map_creator::destroy_factory_map_restrict(MapRestrict* p_restrict_)
{
	if( !VALID_POINT(p_restrict_) ) return;

	const tag_map_info* p_info = p_restrict_->get_map_info();
	if( !VALID_POINT(p_info) ) return;

	factory_restrict.destroy_class(p_restrict_, p_info->e_type);
}

//--------------------------------------------------------------------------------
// ֹͣ���е�ͼ�������߳�
//--------------------------------------------------------------------------------
VOID map_creator::stop_all_map_manager()
{
	MapMgr::StopThread();
	start_all_map_manager();	// ���������̣߳������������˳�����Ϊ�����Ѿ������˽�����־

	m_map_map_manager.reset_iterator();
	MapMgr* p_map_manager = NULL;

	while( m_map_map_manager.find_next(p_map_manager) )
	{
		tstring& str_thread_name = p_map_manager->GetThreadName();
		World::p_thread->waitfor_thread_destroy(str_thread_name.c_str(), INFINITE);
	}
}

//------------------------------------------------------------------------------
// ����һ����ͼ��Ӧ��MapMgr������MapMgr�߳�
//------------------------------------------------------------------------------
MapMgr* map_creator::create_map_manager(tag_map_info* p_info_)
{
	ASSERT( VALID_POINT(p_info_) );

	MapMgr* p_map_manager = new MapMgr;
	if( !VALID_POINT(p_map_manager) )
		return NULL;

	if( FALSE == p_map_manager->Init(p_info_) )
	{
		SAFE_DELETE(p_map_manager);
		return NULL;
	}

	return p_map_manager;
}

//----------------------------------------------------------------------------------
// ���ݵ�ͼID�͸���IDȷ��һ��Ψһ��ͼ
//----------------------------------------------------------------------------------
Map* map_creator::get_map(DWORD dw_map_id_, DWORD dw_instance_)
{
	MapMgr* p_map_manager = get_map_manager(dw_map_id_);
	if( !VALID_POINT(p_map_manager) ) return NULL;

	Map* p_map = NULL;

	if( p_map_manager->IsNormalMap() )	p_map = p_map_manager->GetSingleMap();
	else							p_map = p_map_manager->GetInstance(dw_instance_);

	return p_map;
}

//------------------------------------------------------------------------------------------
// ��ͼ�������ĸ���
//------------------------------------------------------------------------------------------
VOID map_creator::update()
{
	// �������е�ͼ�߳�
	start_all_map_manager();

	// �ȴ����е�ͼ�̱߳��θ������
	wait_all_map_manager_end();

	// �������֮��ĺ������������в�����
	update_all_delayed_stuff();
}

//-------------------------------------------------------------------------------------------
// ÿ��Tick���е�ͼ�߳���ͣ��Ĳ�����һ��Ҫ����˳�����������õ�˳���ܵߵ�
//-------------------------------------------------------------------------------------------
VOID map_creator::update_all_delayed_stuff()
{
	// ���沢�˳������Ѿ������ߵ����
	logout_all_remove_role_pertick();

	// �˳����������������
	logout_all_leave_role();

	// ��������Ҫ����ѡ�˽�������
	deal_all_return_character_role();

	// ��������Ҫ�л���ͼ�����
	deal_all_change_map_role();

	// ����Ҫ�ߵ������
	deal_all_kick_role();

	// ɾ�����б�TickҪɾ���ĸ���
	deal_all_destroied_instance();


}

//--------------------------------------------------------------------------------------------
// ���沢�˳����������������
//--------------------------------------------------------------------------------------------
VOID map_creator::logout_all_leave_role()
{
	DWORD	dw_role_id = m_list_logout_leave_role.pop_front();

	while(VALID_POINT(dw_role_id))
	{
		Role* p_role = g_roleMgr.get_role(dw_role_id);
		if(!VALID_POINT(p_role))
		{
			dw_role_id = m_list_logout_leave_role.pop_front();
			continue;
		}

		Map* p_map = p_role->get_map();
		if(VALID_POINT(p_map))
		{
			p_map->role_leave_logout(p_role->GetID());
		}

		// �������
		//p_role->SaveToDB();

		g_roleMgr.delete_role(p_role->GetID());

		dw_role_id = m_list_logout_leave_role.pop_front();
	}
}

//--------------------------------------------------------------------------------------------
// ���沢�˳������Ѿ������ߵ����
//--------------------------------------------------------------------------------------------
VOID map_creator::logout_all_remove_role_pertick()
{
	DWORD dw_role_id = m_list_logout_role.pop_front();

	while( VALID_POINT(dw_role_id) )
	{
		Role* p_role = g_roleMgr.get_role(dw_role_id);
		if( !VALID_POINT(p_role) )
		{
			dw_role_id = m_list_logout_role.pop_front();
			continue;
		}

		if(VALID_POINT(p_role) && !VALID_POINT(p_role->GetSession()))
		{
			dw_role_id = m_list_logout_role.pop_front();
			continue;
		}

		// �ҵ�����ҵǳ�ʱ���ڵĵ�ͼ
		Map* p_map = p_role->get_map();
		if( VALID_POINT(p_map) )
		{
			p_map->role_logout(p_role->GetID());
		}

		// �������ֽ���
		//p_role->ResetNewRoleGift();

		// �������
		p_role->SaveToDB();

		// ���Session
		PlayerSession* pSession = p_role->GetSession();
		if( VALID_POINT(pSession) )
		{
			// PRINT_MESSAGE(_T("player logout in mapcreator, sessionid=%u, internalid=%u\r\n"), pSession->GetSessionID(), pSession->GetInternalIndex());
			g_worldSession.RemoveSession(pSession->GetSessionID());
		}

		if(!p_role->is_leave_pricitice() || !p_role->IsInRoleState(ERS_Prictice))
		{
			g_roleMgr.delete_role(p_role->GetID());
		}
		else
		{
			p_role->SetSession(NULL);
		}

		g_loginSession.SendPlayerLogout(pSession->GetSessionID());
		g_worldSession.PlayerLogout();
		g_gameGuarder.Logout(pSession->GetSessionID(), pSession->GetAccount());
		SAFE_DELETE(pSession);

		// �õ���һ�����
		dw_role_id = m_list_logout_role.pop_front();
	}
}

//---------------------------------------------------------------------------------------------
// �������б�tickҪ�л���ͼ�����
//---------------------------------------------------------------------------------------------
VOID map_creator::deal_all_change_map_role()
{
	tag_change_map_info info = m_list_change_map_role.pop_front();

	while( VALID_POINT(info.dw_role_id) )
	{
		Role* p_role = g_roleMgr.get_role(info.dw_role_id);
		if( !VALID_POINT(p_role) )
		{
			info = m_list_change_map_role.pop_front();
			continue;
		}

		MapMgr* p_dest_map_manager = g_mapCreator.get_map_manager(info.dw_dest_map_id);
		if( !VALID_POINT(p_dest_map_manager) )
		{
			info = m_list_change_map_role.pop_front();
			continue;
		}

		Map* pDestMap = NULL;
		if(p_role->IsGM() && p_role->get_gm_instance_id() != INVALID_VALUE)
		{
			pDestMap = g_mapCreator.get_map(info.dw_dest_map_id, p_role->get_gm_instance_id());
			p_role->set_gm_instance_id(INVALID_VALUE);
		}
		else
		{
			pDestMap = p_dest_map_manager->CanEnter(p_role, info.dw_misc);
		}
		
		if( !VALID_POINT(pDestMap) )
		{
			info = m_list_change_map_role.pop_front();
			continue;
		}

		// ��������������
		if(p_role->is_leave_pricitice())
		{
			continue;
		}

		// ��ԭ��ͼ����
		Map* p_src_map = p_role->get_map();
		DWORD	dw_src_map_id = INVALID_VALUE;
		if( VALID_POINT(p_src_map) )
		{
			p_src_map->role_leave_map(p_role->GetID());
			dw_src_map_id = p_src_map->get_map_id();
		}

		// ���뵽�µ�ͼ
		p_role->SetMapID(info.dw_dest_map_id);
		if(!info.v_face.x && !info.v_face.y && !info.v_face.z)
		{
			p_role->GetMoveData().Reset(info.v_position.x, info.v_position.y, info.v_position.z, p_role->GetMoveData().m_vFace.x, p_role->GetMoveData().m_vFace.y, p_role->GetMoveData().m_vFace.z);
		}
		else
		{
			p_role->GetMoveData().Reset(info.v_position.x, info.v_position.y, info.v_position.z, info.v_face.x, info.v_face.y, info.v_face.z);
		}
		

		pDestMap->add_role(p_role, p_src_map);

		if(VALID_POINT(p_role->GetScript()))
		{
			p_role->GetScript()->OnChangeMap(p_role, dw_src_map_id, pDestMap->get_map_id(), pDestMap->get_instance_id());
		}

		if(/*p_role->IsPurpureName() && */pDestMap->get_map_type() == EMT_Normal){
				Vector3 pos = p_role->GetCurPos( );
				HearSayHelper::SendMessage(EHST_PURPUREPOS, p_role->GetID(), pos.x, pos.y, pos.z, p_role->GetMapID());
		}

		// ȡ��һ�����
		info = m_list_change_map_role.pop_front();
	}
}

//---------------------------------------------------------------------------------------------
// �������б�tickҪ�ص�ѡ�˽�������
//---------------------------------------------------------------------------------------------
VOID map_creator::deal_all_return_character_role()
{
	DWORD dw_role_id = m_list_return_character_role.pop_front();

	while( VALID_POINT(dw_role_id) )
	{
		Role* p_role = g_roleMgr.get_role(dw_role_id);

		if( VALID_POINT(p_role) )
		{
			p_role->removeFlowCreature();
			p_role->get_map()->role_leave_map(p_role->GetID());

			// �ҵ�Session�����ø�������
			PlayerSession* p_session = p_role->GetSession();
			if(VALID_POINT(p_session))
			{
				p_session->Refresh();

				// ���뵽ѡ�˽�����
				g_worldSession.AddGlobalSession(p_session);

			}

			//p_role->ResetNewRoleGift();

			// �����ɫ����ɾ����ɫ
			p_role->SaveToDB();
			g_roleMgr.delete_role(p_role->GetID());
		}

		// �õ�һ���µ����
		dw_role_id = m_list_return_character_role.pop_front();
	}
}
VOID map_creator::deal_all_kick_role()
{
	if( !m_map_kick_role.empty() )
	{
		package_map<DWORD, INT>::map_iter it = m_map_kick_role.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT n_tick = INVALID_VALUE;

		while( m_map_kick_role.find_next(it, dw_role_id, n_tick) )
		{
			--n_tick;	// ��һ�µ���ʱ
			if( n_tick <= 0 )
			{
				// ʱ�䵽�ˣ�������ߵ�
				Role* p_role = g_roleMgr.get_role(dw_role_id);
				if( VALID_POINT(p_role) )
				{
					g_worldSession.Kick(p_role->GetSession()->GetInternalIndex());
					p_role->GetSession()->SetKicked();
				}

				m_map_kick_role.erase(dw_role_id);
			}
			else
			{
				m_map_kick_role.change_value(dw_role_id, n_tick);
			}
		}
	}
}
//---------------------------------------------------------------------------------------------
// ɾ�����б�tickҪɾ���ĸ���
//---------------------------------------------------------------------------------------------
VOID map_creator::deal_all_destroied_instance()
{
	tag_instance_destroy_info info = m_list_destroy_instance.pop_front();

	while( VALID_POINT(info.dw_map_id) )
	{
		// �ҵ�MapMgr
		MapMgr* p_map_manager = get_map_manager(info.dw_map_id);
		if( !VALID_POINT(p_map_manager) )
		{
			info = m_list_destroy_instance.pop_front();
			continue;
		}

		// �ҵ�����
		map_instance* p_instance = p_map_manager->GetInstance(info.dw_instance_id);
		if( !VALID_POINT(p_instance) )
		{
			info = m_list_destroy_instance.pop_front();
			continue;
		}

		// ɾ������
		p_map_manager->DestroyInstance(p_instance);

		// ɾ����һ��
		info = m_list_destroy_instance.pop_front();
	}
}

//-------------------------------------------------------------------------------------------------
// �ⲿָ��ĳ������ɾ��
//-------------------------------------------------------------------------------------------------
VOID map_creator::set_instance_delete(DWORD dw_map_id_, DWORD dw_instance_id_)
{
	MapMgr* p_map_manager = get_map_manager(dw_map_id_);
	if( !VALID_POINT(p_map_manager) ) return;

	map_instance* p_instance = p_map_manager->GetInstance(dw_instance_id_);
	if( !VALID_POINT(p_instance) ) return;

	p_instance->set_delete();
}

//-------------------------------------------------------------------------------------------------
// �ҵ�ĳ����ͼ�ӹ����
//-------------------------------------------------------------------------------------------------
BOOL map_creator::take_over_role_when_online(Role* p_role_)
{
	if( !VALID_POINT(p_role_) ) return FALSE;

	DWORD dw_map_id = p_role_->GetMapID();		// �������ʱ�ĵ�ͼID
	Vector3 v_pos = p_role_->GetCurPos();		// �������ʱ������

	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_map_id);
	if( !VALID_POINT(p_map_manager) )
	{
		SI_LOG->write_log(_T("Can not find a map when player online, roleid=%u, mapid=%u\n"), p_role_->GetID(), dw_map_id);
		return FALSE;
	}

	FLOAT f_yaw = 0.0f;
	Map* p_map = p_map_manager->CanTakeOverWhenOnline(p_role_, dw_map_id, v_pos, f_yaw);

	while( !VALID_POINT(p_map) )
	{
		p_map_manager = g_mapCreator.get_map_manager(dw_map_id);
		p_map = p_map_manager->CanTakeOverWhenOnline(p_role_, dw_map_id, v_pos, f_yaw);

		if( !VALID_POINT(p_map) && dw_map_id == p_map_manager->get_map_info()->dw_id )	// ���ܽӹܣ������ڵ�ͼ�������Լ���˵���д���
		{
			SI_LOG->write_log(_T("find a duplicated map when player online, roleid=%u, mapid=%u\n"), p_role_->GetID(), dw_map_id);
			break;
		}
	}

	if( VALID_POINT(p_map) )
	{
		// �ҵ���һ��Ŀ���ͼ
		p_role_->SetMapID(dw_map_id);
		p_role_->GetMoveData().Reset(v_pos.x, v_pos.y, v_pos.z, p_role_->GetFaceTo().x, p_role_->GetFaceTo().y, p_role_->GetFaceTo().z);

		p_map->add_role(p_role_);

		NET_SIS_unset_role_state send;
		send.dw_role_id = p_role_->GetID();
		send.eState = ERS_Prictice;
		p_map->send_big_visible_tile_message(p_role_, &send, send.dw_size);

		return TRUE;
	}
	else
	{
		// û���ҵ�Ŀ���ͼ���򴫻س�����
		dw_map_id = g_mapCreator.get_born_map_id();
		INT nIndex = 0;
		v_pos = g_mapCreator.rand_get_one_born_position(nIndex);

		FLOAT fYaw = 0.0f;

		p_map_manager = g_mapCreator.get_map_manager(dw_map_id);
		p_map = p_map_manager->CanTakeOverWhenOnline(p_role_, dw_map_id, v_pos, fYaw);

		if( !VALID_POINT(p_map) )
		{
			SI_LOG->write_log(_T("Critical Error, role can not enter the born map!!!, roleid=%u\n"), p_role_->GetID());
			return FALSE;
		}

		p_role_->SetMapID(dw_map_id);
		p_role_->GetMoveData().Reset(v_pos.x, v_pos.y, v_pos.z, cosf((270-f_yaw) * 3.1415927f / 180.0f), 0.0f, sinf((270-f_yaw) * 3.1415927f / 180.0f));

		p_map->add_role(p_role_);

		NET_SIS_unset_role_state send;
		send.dw_role_id = p_role_->GetID();
		send.eState = ERS_Prictice;
		p_map->send_big_visible_tile_message(p_role_, &send, send.dw_size);

		return TRUE;
	}
}

const Vector3 map_creator::get_reborn_point( DWORD dw_reborn_map_id_ )
{
	Vector3 v_rt;
	v_rt.x = v_rt.y = v_rt.z = INVALID_VALUE;
	const tag_map_info *p_map_info = g_mapCreator.get_map_info(dw_reborn_map_id_);
	if(VALID_POINT(p_map_info))
	{
		v_rt = p_map_info->v_reborn_positon;
	}
	else
	{
		// ������ͼû�ҵ�
		ASSERT(0);
	}

	return v_rt;
}

//-------------------------------------------------------------------------------------------------
// �Ƿ���Դ�������
//-------------------------------------------------------------------------------------------------
BOOL map_creator::can_create_instance(const tag_map_info* p_info_)
{
	if( EMT_Normal == p_info_->e_type ) return FALSE;
	if( p_info_->e_instance_grade < EMIG_NOLIMIT || p_info_->e_instance_grade >= EMIG_END ) return FALSE;

	if( EMIG_NOLIMIT == p_info_->e_instance_grade ) return TRUE;

	// ����һ�¼�Ȩ�ܺ��Ƿ��������
	if( m_n_instance_coef_cur_num + INSTANCE_POWER[p_info_->e_instance_grade] > m_n_instance_coef_num_limit ) return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
// ����һ������
//-------------------------------------------------------------------------------------------------
VOID map_creator::add_instance(const tag_map_info* p_info_)
{
	if( EMT_Normal == p_info_->e_type ) return;
	if( p_info_->e_instance_grade < EMIG_NOLIMIT || p_info_->e_instance_grade >= EMIG_END ) return;

	m_n_instance_cur_num[p_info_->e_instance_grade] += 1;
	m_n_instance_coef_cur_num += INSTANCE_POWER[p_info_->e_instance_grade];
}

//-------------------------------------------------------------------------------------------------
// �Ƴ�һ������
//-------------------------------------------------------------------------------------------------
VOID map_creator::remove_instance(const tag_map_info* p_info_)
{
	if( EMT_Normal == p_info_->e_type ) return;
	if( p_info_->e_instance_grade < EMIG_NOLIMIT || p_info_->e_instance_grade >= EMIG_END ) return;

	m_n_instance_cur_num[p_info_->e_instance_grade] -= 1;
	m_n_instance_coef_cur_num -= INSTANCE_POWER[p_info_->e_instance_grade];
}

//-------------------------------------------------------------------------------------------------
//��ȡ��ͼ����
//-------------------------------------------------------------------------------------------------
INT	map_creator::get_map_role_num(DWORD dw_map_id)
{
	MapMgr* p_map_manager = m_map_map_manager.find(dw_map_id);
	if(!VALID_POINT(p_map_manager))
		return -1;

	Map* p_map = p_map_manager->GetSingleMap();
	if(!VALID_POINT(p_map))
		return -1;

	return p_map->get_role_num();
}

//-------------------------------------------------------------------------------------------------
//��ȡ��ͼ������������
//-------------------------------------------------------------------------------------------------
INT	map_creator::get_map_leave_role_num(DWORD dw_map_id)
{
	MapMgr* p_map_manager = m_map_map_manager.find(dw_map_id);
	if(!VALID_POINT(p_map_manager))
		return -1;

	Map* p_map = p_map_manager->GetSingleMap();
	if(!VALID_POINT(p_map))
		return -1;

	return p_map->get_leave_role_num();
}

//-------------------------------------------------------------------------------------------------
//��ȡ��ͼ����ʱ��
//-------------------------------------------------------------------------------------------------
DWORD map_creator::get_update_time(DWORD dw_map_id)
{
	MapMgr* p_map_manager = m_map_map_manager.find(dw_map_id);
	if(!VALID_POINT(p_map_manager))
		return -1;

	Map* p_map = p_map_manager->GetSingleMap();
	if(!VALID_POINT(p_map))
		return 0;

	return p_map->get_update_time();
}


// ˢ��ս���̵�
VOID map_creator::onclock(INT nClock)
{
	MapMgrMap::map_iter iter = m_map_map_manager.begin();
	MapMgr* p_map_manager = NULL;
	while(m_map_map_manager.find_next(iter, p_map_manager))
	{
		if(!VALID_POINT(p_map_manager))
			continue;
		Map* p_map = p_map_manager->GetSingleMap();
		if(!VALID_POINT(p_map))
			continue;

		p_map->onclock(nClock);
	}
}

//-------------------------------------------------------------------------------------------------
//��ȡ���ִ�����(ֻ�ܻ�ȡ��������)
//-------------------------------------------------------------------------------------------------
INT map_creator::get_instance_role_num(DWORD dw_map_id)
{
	MapMgr* p_map_manager = m_map_map_manager.find(dw_map_id);
	if(!VALID_POINT(p_map_manager))
		return -1;

	INT n_num = p_map_manager->get_instance_role_num();

	return n_num;
}

//-------------------------------------------------------------------------------------------------
//��ȡ���ִ�������������(ֻ�ܻ�ȡ��������)
//-------------------------------------------------------------------------------------------------
INT	map_creator::get_instance_leave_role_num(DWORD dw_map_id)
{
	MapMgr* p_map_manager = m_map_map_manager.find(dw_map_id);
	if(!VALID_POINT(p_map_manager))
		return -1;

	INT n_num = p_map_manager->get_instance_leave_role_num();

	return n_num;
}

BOOL map_creator::is_sbk_map( DWORD dw_map_id )
{
	if ((dw_map_id == get_tool()->crc32(szSBKHuangGong)) || 
		(dw_map_id == get_tool()->crc32(szGuildMapName)) )
	{
		return true;
	}
	return false;
}


map_creator g_mapCreator;