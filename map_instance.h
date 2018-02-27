/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ͨ������ͼ

#pragma once

#pragma once
#include "map.h"
#include "instance_define.h"

#include "../common/ServerDefine/role_data_server_define.h"

class Team;
struct s_inst_door_state;

//-----------------------------------------------------------------------------
// CONST
//-----------------------------------------------------------------------------
const INT ROLE_LEAVE_INSTANCE_TICK_COUNT_DOWN	= 60 * TICK_PER_SECOND;		// ��ɫ�ڸ������뿪����ʱ�����ͳ������ĵ���ʱ��60��
const INT INSTANCE_COMPLETE_TIME				= 120 * 1000;				// ����������ɣ��رյ���ʱ
const INT SAVE_PROCESS_TIME						= 60 * 10 * TICK_PER_SECOND;	// �������ȴ洢ʱ��
const INT DXQ_INSTANCE_LIMIT_TIMES				= 5;//��С�Ǹ������ƴ����ܺ�Ϊ5	gx add 2013.11.06
typedef package_list<DWORD>		LIST_CREATURE_PRO;					// ���︱������


//-----------------------------------------------------------------------------
// ��ͨ�����࣬������һ���鴴���ĸ��������������ȵ�
//-----------------------------------------------------------------------------
class map_instance_normal : public map_instance
{
public:
	map_instance_normal();
	virtual ~map_instance_normal();

	virtual BOOL		init(const tag_map_info* p_info_, DWORD dw_instance_id_, Role* p_creator_=NULL, DWORD dw_misc_=INVALID_VALUE);
	virtual VOID		update();
	virtual VOID		destroy();

	virtual VOID		add_role(Role* p_role_, Map* pSrcMap = NULL);
	virtual	VOID		role_leave_map(DWORD dw_id_, BOOL b_leave_online = FALSE);
	virtual INT			can_enter(Role *p_role_, DWORD dw_param_ = 0);
	virtual BOOL		can_destroy();
	virtual VOID		on_destroy();

	DWORD				get_creator_id()			{ return m_dw_creator_id; }
	EInstanceHardMode	get_instance_hard()		{ return m_e_instance_hare_mode; }
	DWORD				cal_time_limit();
	const tagInstance*	get_instance_proto()		{ return m_p_instance; }

	VOID				on_team_create(const Team* p_team_);
	VOID				on_team_delete(const Team* p_team_);
	VOID				on_role_leave_team(DWORD dw_role_id_, const Team* p_team_);
	VOID				on_role_enter_team(DWORD dw_role_id_, const Team* p_team_);

	tagDWORDTime		get_create_time() { return m_dw_create_time; }

	VOID				complete_estimate ();

	VOID				set_boss(Creature* p_boss_);

	INT					get_map_role_num() { return map_role.size(); }

	LIST_CREATURE_PRO&	get_list_creature_pro() { return m_list_creature_process; }

	VOID				set_door_state(DWORD	dw_door_id, BOOL b_open);

	VOID	get_door_state(INT nIndex, DWORD& dw_door_id, BOOL& b_open);

	VOID				add_inst_process_to_role(Role* pRole);

protected:
	virtual VOID		on_delete();
	virtual BOOL		init_all_fixed_creature(DWORD dw_guild_id_ = INVALID_VALUE);					// ���ɵ�ͼ������аڷŹ���
	virtual BOOL		init_all_spawn_point_creature(DWORD dw_guild_id_ = INVALID_VALUE);
	
private:
	VOID				update_time_issue();
	BOOL				is_time_limit()		{ return m_p_instance->dwTimeLimit > 0 && m_p_instance->dwTargetLimit != INVALID_VALUE; }

	BOOL				recal_hard_mode();
	BOOL				get_creature_base_level(INT& n_base_level_);
	DWORD				cal_creature_type_id(const tagRandSpawnPointInfo* p_rand_spawn_point_);
	//DWORD				transmit_big_id(INT n_base_level_, tag_map_spawn_point_info* p_map_spawn_info_);

	VOID				init_inst_process(Role* pRole);

private:
	BOOL					m_b_no_enter;						// �����Ƿ�û�˽����
	DWORD					m_dw_creator_id;					// ����������ID
	DWORD					m_dw_team_id;						// ����������С��ID
	DWORD					m_dw_start_tick;					// ��ʼʱ��
	DWORD					m_dw_end_tick;					// ������ʼ���õ���ʱ
	EInstanceHardMode		m_e_instance_hare_mode;			// �����Ѷ�

	package_map<DWORD, INT>		m_map_will_out_role_id;				// ����������������ȴ������ȥ������б�

	const tagInstance*		m_p_instance;					// ������̬����

	tagDWORDTime			m_dw_create_time;					// ��������ʱ��

	INT						m_n_complete_time;				// ������ɵ���ʱ

	Creature*				m_p_boss;						// ����boss

	LIST_CREATURE_PRO		m_list_creature_process;		// �����������
	s_inst_door_state		m_door_state[MAX_INST_DOOR_NUM];	// �����Ž���
	BOOL					b_process;						// �Ƿ��н���

	DWORD					dw_save_process_time;
};


