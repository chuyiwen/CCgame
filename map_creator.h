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

#pragma once

#include "map.h"
#include "mutex.h"

struct tag_map_info;
class Map;
class Team;
class map_instance;
class MapMgr;
class MapRestrict;

//---------------------------------------------------------------------------------
// �л���ͼ�Ľṹ
//---------------------------------------------------------------------------------
struct tag_change_map_info
{
	DWORD		dw_role_id;			// ��ɫID
	DWORD		dw_dest_map_id;		// Ŀ���ͼID
	Vector3		v_position;				// Ŀ���ͼ����
	Vector3		v_face;				// ����
	DWORD		dw_misc;				// �������������������������Ѷ�

	tag_change_map_info(DWORD)
	{
		dw_role_id = INVALID_VALUE;
	}
	tag_change_map_info()
	{
		dw_role_id = INVALID_VALUE;
	}
};

//-----------------------------------------------------------------------------------
// ����ɾ���ṹ
//-----------------------------------------------------------------------------------
struct tag_instance_destroy_info
{
	DWORD		dw_map_id;			// ��ͼID
	DWORD		dw_instance_id;		// ����ID

	tag_instance_destroy_info(DWORD)
	{
		dw_map_id = INVALID_VALUE;
	}
	tag_instance_destroy_info(DWORD _dwMapID, DWORD _dwInstanceID)
	{
		dw_map_id = _dwMapID;
		dw_instance_id = _dwInstanceID;
	}
	tag_instance_destroy_info()
	{
		dw_map_id = INVALID_VALUE;
	}
};

//-------------------------------------------------------------------------------------------------
// ��ͼ������
//-------------------------------------------------------------------------------------------------
class map_creator
{
public:
	typedef		package_map<DWORD, MapMgr*>		MapMgrMap;
	typedef		package_map<DWORD, tag_map_info*>	MapInfoMap;

public:
	map_creator();
	~map_creator();

	//---------------------------------------------------------------------------------------------
	// ��ʼ�������¼�����
	//---------------------------------------------------------------------------------------------
	BOOL				init();
	VOID				update();
	VOID				destroy();

	//--------------------------------------------------------------------------------------------
	// �������
	//--------------------------------------------------------------------------------------------
	Map*				create_factory_map(const tag_map_info* p_info_);
	map_instance*		create_factory_map_instance(const tag_map_info* p_info_);
	MapRestrict*		create_factory_map_restrict(const tag_map_info* p_info_);

	VOID				destroy_factory_map(Map* p_map_);
	VOID				destroy_factory_map_restrict(MapRestrict* p_restrict_);

	//--------------------------------------------------------------------------------------------
	// �̹߳���
	//--------------------------------------------------------------------------------------------
	VOID				start_all_map_manager();								// ��Tick�������е�ͼ�߳�����
	VOID				wait_all_map_manager_end();								// �ȴ���Tick���е�ͼ�߳̽���
	VOID				update_all_delayed_stuff();						// ÿһTick�������е�ͼ�߳���ͣ��Ĳ���
	VOID				stop_all_map_manager();								// ����ֹͣ���е�ͼ�������߳�
	Event&				get_all_map_start_event(INT n);						// �õ���Tick�������е�ͼ�߳����е��¼�
	VOID				one_map_manager_end();									// һ����ͼ�̱߳�Tickִ�����

	//--------------------------------------------------------------------------------------------
	// ����Get
	//--------------------------------------------------------------------------------------------
	MapMgr*				get_map_manager(DWORD dw_map_id_)			{ return m_map_map_manager.find(dw_map_id_); }
	const tag_map_info*	get_map_info(DWORD dw_map_id_)			{ return m_map_map_info.find(dw_map_id_); }
	DWORD				get_born_map_id()						{ return m_dw_born_map_id; }
	BOOL				is_map_exist(DWORD dw_map_id_)			{ return m_map_map_info.is_exist(dw_map_id_); }
	BOOL				is_sbk_map(DWORD dw_map_id);

	Vector3				rand_get_one_born_position(INT& n_index_);
	FLOAT				get_born_yaw(INT n_index_);
	Map*				get_map(DWORD dw_map_id_, DWORD dw_instance_);
	DWORD				get_prison_map_id()			{ return m_dw_prison_map_id; }
	const Vector3		get_putin_prison_point()		{ return m_putin_prison_point; }
	const Vector3		get_putout_prison_point()		{ return m_putout_prison_point; }
	const Vector3		get_reborn_point(DWORD dw_reborn_map_id_);

	DWORD				get_yamun_map_id( ) const { return m_dw_yamun_map_id; }
	const Vector3&		get_putin_yamun_point( ) const { return m_putin_yamun_point; }
	
	INT					get_stable_instance_num()		{ return m_n_max_stable_instance_num; }

	INT					get_map_role_num(DWORD dw_map_id);
	INT					get_map_leave_role_num(DWORD dw_map_id);
	INT					get_instance_role_num(DWORD dw_map_id);
	INT					get_instance_leave_role_num(DWORD dw_map_id);

	DWORD				get_update_time(DWORD	dw_map_id);

	VOID				onclock(INT nClock);

	//---------------------------------------------------------------------------------------------
	// ��ɫ������
	//---------------------------------------------------------------------------------------------
	VOID				role_logout(DWORD dw_role_id)			{ m_list_logout_role.push_back(dw_role_id); }
	VOID				role_return_character(DWORD dw_role_id)	{ m_list_return_character_role.push_back(dw_role_id); }
	VOID				role_logout_leave_role(DWORD	dw_role_id) { m_list_logout_leave_role.push_back(dw_role_id); }
	VOID				role_change_map(DWORD dw_role_id, DWORD dw_dest_map_id_, Vector3& v_pos_, Vector3& v_face_, DWORD dw_misc_);
	VOID				role_will_kick(DWORD dw_role_id, INT nTime) { m_map_kick_role.add(dw_role_id, nTime); }

	BOOL				take_over_role_when_online(Role* p_role_);
	
	//---------------------------------------------------------------------------------------------
	// ��������
	//---------------------------------------------------------------------------------------------
	DWORD				create_new_instance_id()	{ Interlocked_Exchange_Add((LPLONG)&m_dw_instance_id_gen, 1); return m_dw_instance_id_gen; }
	VOID				instance_destroyed(DWORD dw_map_id_, DWORD dw_instance_id_);

	VOID				set_instance_delete(DWORD dw_map_id_, DWORD dw_instance_id_);

	BOOL				can_create_instance(const tag_map_info* p_info_);
	VOID				add_instance(const tag_map_info* p_info_);
	VOID				remove_instance(const tag_map_info* p_info_);
	INT					get_instance_num(INT n_index_)		{ return m_n_instance_cur_num[n_index_]; }
	INT					get_instance_coef_num()			{ return m_n_instance_coef_cur_num; }
	INT					get_instance_coef_num_limit()		{ return m_n_instance_coef_num_limit; }

	package_map<DWORD, DWORD>&	get_guild_tripod_map() { return map_guild_tripod; }

	const std::vector<tstring>& GetMapNames( ) const{ return mMapNames; }
	const std::vector<tstring>& GetInstanceNames( ) const { return mInstanceNames; }
private:
	//---------------------------------------------------------------------------------------------
	// ��ʼ��
	//---------------------------------------------------------------------------------------------
	BOOL				loag_map_info(LPCTSTR sz_file_map_name_, file_container* p_logic_);				// �����ͼ�Ļ�����Ϣ������ɹ�ʱ������뵽m_map_map_info
	BOOL				load_map_way_point(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);		// �����ͼ�е����е�����
	//BOOL				load_map_trigger(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);			// �����ͼ�е����д�����
	BOOL				load_map_area(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);			// �����ͼ�е���������
	//BOOL				load_map_creature(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);		// �����ͼ�е����г�������
	
	//gx add 2013.4.17
	BOOL				load_map_trigger_extra(LPCTSTR sz_file_map_name_, tag_map_info* p_info_);			
	//end
	BOOL				load_map_spawn_point(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);		// �����ͼ�е�����ˢ�ֵ�
	BOOL				load_map_trigger_effect(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);	// �����ͼ�е�������Ч
	BOOL                load_map_door(tagMapHeader* p_header_, DWORD handle_, tag_map_info* p_info_);
	BOOL				load_map_block(DWORD handle, tag_map_info* p_info_);

	BOOL				load_map_creature(XmlHandle& xmlHandle, tag_map_info* p_info_);//�����ͼ�����е�ˢ�ֵ�
	BOOL				load_map_block(XmlHandle& xmlHandle, tag_map_info* p_info_);
	BOOL				load_map_trigger(XmlHandle& xmlHandle, tag_map_info* p_info_);			// �����ͼ�е����д�����
	BOOL				load_map_way_point(XmlHandle& xmlHandle, tag_map_info* p_info_);		// �����ͼ�е����е�����
	BOOL				load_map_area(XmlHandle& xmlHandle, tag_map_info* p_info_);//�����ͼ����
	//----------------------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------------------
	VOID				register_factory_map();
	VOID				register_factory_map_restrict();
	VOID				un_register_all();

	//----------------------------------------------------------------------------------------------
	// �̹߳���
	//----------------------------------------------------------------------------------------------
	MapMgr*				create_map_manager(tag_map_info* p_info_);

	//----------------------------------------------------------------------------------------------
	// ����������
	//----------------------------------------------------------------------------------------------
	VOID				logout_all_remove_role_pertick();
	VOID				deal_all_change_map_role();
	VOID				deal_all_return_character_role();
	VOID				deal_all_destroied_instance();
	VOID				logout_all_leave_role();
	VOID				logout_one_leave_role(DWORD	dw_role_id_);
	VOID				deal_all_kick_role();
private:
	
	std::vector<tstring>						mMapNames;
	std::vector<tstring>						mInstanceNames;

	class_template_factory<MapRestrict>			factory_restrict;					// ��ͼ�����๤������ͼ���͡������������ࣩ
	class_template_factory<Map>					factory_map;						// ��ͼ�๤������ͼ���͡�����ͼ�����ࣩ

	MapMgrMap							m_map_map_manager;					// ���е�MapMgr�б�����Ƿ�Ҫ��������
	MapInfoMap							m_map_map_info;					// ���е�ͼ���߼�������Ϣ
	volatile DWORD						m_dw_instance_id_gen;				// ����ID������

	INT									m_n_instance_cur_num[EMIG_END];	// ��ͬ��񸱱��ĵ�ǰ��������
	INT									m_n_instance_coef_cur_num;			// ��ǰ���и����ļ�Ȩ�ܺ�
	INT									m_n_instance_coef_num_limit;		// ���и����ļ�Ȩ�ܺ�����

	DWORD								m_dw_born_map_id;					// ��ҳ�����ͼ��ID
	INT									m_n_born_position_num;					// ���������
	Vector3*							m_pv_born_position;					// ��������������
	FLOAT*								m_p_yaw;							// ���������ﳯ��

	Event								m_map_start_event[2];				// �¼������������е�ͼ�߳�ִ�У������¼������е�ͼ�̵߳�Update�߳̽�������������¼���
	INT									m_n_which_event;					// ��ǰʹ�õ����ĸ��¼�
	Event								m_map_end_event;					// �¼��������е�ͼUpdate���
	volatile INT						m_n_num_map_manager;					// MapMgr��������ÿ��Tick����ΪMapMgr����������ÿ��MapMgr������һ�κ���ٸ�ֵ

	package_safe_list<DWORD>					m_list_logout_role;				// ÿ��Tick�ǳ������
	package_safe_list<tag_change_map_info>			m_list_change_map_role;			// ÿ��TickҪ�л���ͼ�����
	package_safe_list<DWORD>					m_list_return_character_role;		// ÿ��TickҪ�ص�ѡ�˽�������
	package_safe_list<DWORD>					m_list_logout_leave_role;		// ÿ��tick�ǳ��������������
	package_safe_map<DWORD, INT>				m_map_kick_role;				// ÿ��ticҪ�߳������
	package_safe_list<tag_instance_destroy_info>	m_list_destroy_instance;			// ÿ��TickҪɾ���ĸ���

	// ������Ϣ
	DWORD								m_dw_prison_map_id;				// ������ͼid
	Vector3								m_putin_prison_point;				// ����������point
	Vector3								m_putout_prison_point;			// ����������point

	//����
	DWORD								m_dw_yamun_map_id;					//���ŵ�ͼID
	Vector3								m_putin_yamun_point;				//����������point

	// ���ִ�
	INT									m_n_max_stable_instance_num;		// ���ִ帱������

	//-------------------------------------------------------------------------------------------
	// ���ﳡ����ᶦ�б�
	//-------------------------------------------------------------------------------------------
	package_map<DWORD, DWORD>		map_guild_tripod;
};

//--------------------------------------------------------------------------------------------
// �������е�ͼ�߳�
//--------------------------------------------------------------------------------------------
inline VOID map_creator::start_all_map_manager()
{
	m_n_which_event = ((m_n_which_event == 0) ? 1 : 0);
	Interlocked_Exchange((LPLONG)&m_n_num_map_manager, m_map_map_manager.size());
	m_map_start_event[m_n_which_event].Set();
}

//--------------------------------------------------------------------------------------------
// �ȴ����е�ͼ�̱߳�tick���
//--------------------------------------------------------------------------------------------
inline VOID	map_creator::wait_all_map_manager_end()
{
	m_map_end_event.Wait();
	m_map_end_event.ReSet();
	m_map_start_event[m_n_which_event].ReSet();
}

//---------------------------------------------------------------------------------------------
// ����õ�һ�����������
//---------------------------------------------------------------------------------------------
inline Vector3 map_creator::rand_get_one_born_position(INT& n_index_)
{
	INT n_rand = get_tool()->tool_rand() % m_n_born_position_num;
	n_index_ = n_rand;
	return m_pv_born_position[n_rand];
}

//---------------------------------------------------------------------------------------------
// �õ������㳯��
//---------------------------------------------------------------------------------------------
inline FLOAT map_creator::get_born_yaw(INT n_index_)
{
	return m_p_yaw[n_index_];
}

//---------------------------------------------------------------------------------------------
// �õ�������ͼ�߳����е��¼�
//---------------------------------------------------------------------------------------------
inline Event& map_creator::get_all_map_start_event(INT n)
{
	return m_map_start_event[n];
}

//---------------------------------------------------------------------------------------------
// ĳ����ͼ�̱߳�Tickִ�����
//---------------------------------------------------------------------------------------------
inline VOID map_creator::one_map_manager_end()
{
	// �����е�ͼ�߳�ִ����Ϻ������¼�֪ͨ
	if( 0 == Interlocked_Decrement((LPLONG)&m_n_num_map_manager ) ) 
	{
		m_map_end_event.Set();
	}
}

//----------------------------------------------------------------------------------------------
// ����л���ͼ
//----------------------------------------------------------------------------------------------
inline VOID map_creator::role_change_map(DWORD dw_role_id, DWORD dw_dest_map_id_, Vector3& v_pos_, Vector3& v_face_, DWORD dw_misc_)
{
	MapMgr* p_map_manager = get_map_manager(dw_dest_map_id_);
	if( !VALID_POINT(p_map_manager) ) return;

	tag_change_map_info info;
	info.dw_role_id		=	dw_role_id;
	info.dw_dest_map_id	=	dw_dest_map_id_;
	info.v_position			=	v_pos_;
	info.v_face			=   v_face_;
	info.dw_misc			=	dw_misc_;

	m_list_change_map_role.push_back(info);
}

//------------------------------------------------------------------------------------------------
// ��������ɾ��
//------------------------------------------------------------------------------------------------
inline VOID map_creator::instance_destroyed(DWORD dw_map_id_, DWORD dw_instance_id_)
{
	MapMgr* p_map_manager = get_map_manager(dw_map_id_);
	if( !VALID_POINT(p_map_manager) ) return;

	tag_instance_destroy_info info(dw_map_id_, dw_instance_id_);

	m_list_destroy_instance.push_back(info);
}

extern map_creator g_mapCreator;

