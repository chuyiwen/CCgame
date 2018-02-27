

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
*	@file		guild_war.h
*	@author		mmz
*	@date		2010/10/14	initial
*	@version	0.0.1.0
*	@brief		����ս��
*/

#pragma once

#include "world.h"

#include "../../common/WorldDefine/guild_define.h"

class guild;

struct guild_war_broadcast_time
{
	tagDWORDTime	dw_broadcast_time;
	INT16			n_time;
	INT				n_space_time;

	VOID SetBroadcast(INT nSpaceTime, INT16 nTime, INT nBeginTime)
	{
		dw_broadcast_time = GetCurrentDWORDTime();
		dw_broadcast_time = IncreaseTime(dw_broadcast_time, nBeginTime);
		this->n_time = nTime;
		this->n_space_time = nSpaceTime;
	}
};

class guild_war
{
public:
	guild_war(void);
	~guild_war(void);

public:
	VOID	update();

	VOID	reset();

	VOID	init(guild* p_guild_);

	VOID	set_relive_id(DWORD dw_creature_id_, BOOL b_add_ = TRUE);

	VOID	zero_relive_id();

	VOID	reset_avlidance_state();

	DWORD	get_declare_guild_id() const { return dw_declare_guild_id; }

	DWORD	get_bedeclare_guild_id() const { return dw_bedeclare_guild_id; }

	DWORD	get_war_guild_id() const { return dw_war_guild_id; }

	tagDWORDTime	get_begin_time() { return dw_begin_time; }

	//tagDWORDTime	get_prepare_begin_time() { return dw_prepare_begin_time; }

	//tagDWORDTime	get_war_begin_time()	{ return dw_war_begin_time; }

	EGuildWarState	get_guild_war_state() const { return e_guild_war_state; }

	DWORD			get_relive_id(INT n_index_);

	VOID			remove_guild_war();
	
	VOID			set_member_number(DWORD dwNum) { m_dw_member_number = dwNum; }
	
	DWORD			get_member_number() { return m_dw_member_number; }
		
	VOID			add_member_number() { m_dw_member_number++; }

	VOID			sub_member_number() { 
		m_dw_member_number--; 
		if (m_dw_member_number == INVALID_VALUE)
			m_dw_member_number = 0;
	}

	VOID			set_max_number(DWORD dwNum) { m_dw_Max_num = dwNum; }
	
	DWORD			get_max_number();

	VOID			set_param(double f) { m_fParam = f; }

	double			get_param() { return m_fParam; }
public:

	VOID set_declare_guild_id(DWORD dw_guild_id_) { dw_declare_guild_id = dw_guild_id_; }

	VOID set_bedeclare_guild_id(DWORD dw_guild_id_) { dw_bedeclare_guild_id = dw_guild_id_; }

	VOID set_war_guild_id(DWORD dw_guild_id_) { dw_war_guild_id = dw_guild_id_; }

	VOID set_guild_war_state(EGuildWarState e_guild_war_state_, const tstring& name = _T(""), BOOL bDefenter=true, BOOL bMianzhan = false);

	//VOID set_begin_time() { dw_begin_time = g_world.GetWorldTime(); }

	//VOID set_prepare_begin_time() { dw_prepare_begin_time = g_world.GetWorldTime(); }

	VOID set_achievement(DWORD dw_add_) { dw_achievement += dw_add_; }

	VOID set_enemy_achievement(DWORD dw_add_) { dw_enemy_achievement += dw_add_; }

	guild_war_broadcast_time& get_declare_broadcast() { return st_declare_broadcast; }

	guild_war_broadcast_time& get_prepare_broadcast() { return st_prepare_broadcast; } 

	DWORD get_war_achievement() { return dw_achievement; }

	DWORD get_enemy_war_achievement() { return dw_enemy_achievement; }

	VOID  war_outcome(BOOL bWin);

	VOID set_enemy_guild_level(BYTE byLevel) { by_enemy_level = byLevel; }
	
	VOID setRelay(BOOL bRelay) { m_bRealy = bRelay;}
	BOOL isRelay() { return m_bRealy; }
	
	BOOL isDefenter() { return dw_declare_guild_id != INVALID_VALUE; }

	BYTE getRebornTime() { return m_byRebornTime; }
	VOID decRebornTime() { m_byRebornTime--; }

	VOID send_reborn_time_to_guild();
public:
	BOOL				m_bWin;			// �Ƿ�Ӯ��

	BOOL				m_bLost;		// �Ƿ�����

	DWORD				m_dw_baoming_num;					// ��������
private:

	EGuildWarState		e_guild_war_state;

	DWORD				dw_declare_guild_id;		//��ս��ID

	DWORD				dw_bedeclare_guild_id;	// ����ս��ID

	DWORD				dw_war_guild_id;			// ս����ID

	tagDWORDTime		dw_begin_time;			// ��ǰ״̬��ʼʱ��

	//tagDWORDTime		dw_prepare_begin_time;	// ս��׼����ʼʱ��

	//tagDWORDTime		dw_war_begin_time;		// ս����ʼʱ��

	DWORD				dw_achievement;		// ս���ɼ�

	DWORD				dw_enemy_achievement;	// ���˳ɼ�

	BYTE				by_enemy_level;			// �ж԰��ȼ�

	DWORD				dw_relive_id[MAX_GUILD_RELIVE_NUM];	//���ս�����ID
	
	DWORD				m_dw_Max_num;						// ��ս�������
	
	DWORD				m_dw_member_number;					// ��ս����
	
	DOUBLE				m_fParam;							// ��������

	guild*				p_guild;

	guild_war_broadcast_time	st_declare_broadcast;			// ��ս�㲥����

	guild_war_broadcast_time	st_prepare_broadcast;			// ��ս�㲥����

	guild_war_broadcast_time	st_war_broadcast;				// ս���㲥����

	BOOL				m_bRealy;							// �Ƿ�׼������
	
	BOOL				m_bInitDoor;						// ���Ƿ��ʼ����

	BYTE				m_byRebornTime;						// ��Ѹ������
};
