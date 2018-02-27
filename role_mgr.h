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
 *	@file		role_mgr
 *	@author		mwh
 *	@date		2011/03/10	initial
 *	@version	0.0.1.0
 *	@brief		人物管理器
*/

#ifndef __ROLE_MGR_H__

#define __ROLE_MGR_H__

#include "mutex.h"
#include "world_session.h"
#include "group_mgr.h"

struct s_role_info;
struct s_role_data_load;
class Role;
class PlayerSession;

// 在线角色维护结构
struct role_node
{
	Role* p_role;
	INT next_free;
};

// 维护在线Role指针
class local_role_mgr
{
public:
	VOID reset();
	INT add(Role *pRole);
	Role* remove(INT nIndex);
	Role* get_role(INT nIndex);
	Role* get_save_role();
public:
	INT get_max_used_index() const
	{ return max_used_index_; }

public:
	INT free_index_;
	INT free_number_;
	INT max_used_index_;
	INT last_save_index_;
	role_node role_nodes_[MAX_PLAYER_NUM];
};

inline Role* local_role_mgr::get_role(INT index)
{
	if(index < 0 || index >= MAX_PLAYER_NUM)
		return NULL;

	return role_nodes_[index].p_role;
}

inline INT local_role_mgr::add(Role *p_role)
{
	if(free_index_ < 0 || free_index_ >= MAX_PLAYER_NUM)
		return INVALID_VALUE;

	INT ret_index = free_index_;
	role_node *p_node = &role_nodes_[ret_index];

	free_index_ = p_node->next_free;
	if(max_used_index_ < ret_index)
		max_used_index_ = ret_index;

	p_node->p_role = p_role;
	p_node->next_free = INVALID_VALUE;

	--free_number_;

	return ret_index;
}

inline Role* local_role_mgr::remove(INT index)
{
	if(index < 0 || index >= MAX_PLAYER_NUM)
		return NULL;

	Role *p_role = role_nodes_[index].p_role;

	role_nodes_[index].next_free = free_index_;
	role_nodes_[index].p_role = NULL;
	
	free_index_ = index;
	++free_number_;

	return p_role;
}

// 角色管理器
class role_mgr
{
	friend VOID	GroupMgr::OnAddAllRoleBuff(DWORD dwSenderID, LPVOID pEventMessage);
public:
	typedef package_map<DWORD, s_role_info*> RoleInfoMap;
	typedef package_map<DWORD, INT> RoleMap;
	typedef package_map<DWORD, DWORD> RoleIDMap;

public:
	role_mgr(){};
	~role_mgr();
public:
	BOOL init();
	VOID destroy();
public:
	BOOL insert_role_info(const s_role_info* p_info);
	Role* insert_online_role(DWORD dw_role_id, const s_role_data_load* p_data, PlayerSession* p_session, BOOL& first_in);

	BOOL delete_role_info(const DWORD dw_role_id);
	BOOL delete_role(const DWORD dw_role_id);

	VOID save_one_to_db();
	VOID save_all_role_to_db();

	//added by mmz at 2010.9.19
	Role* get_role(const DWORD dw_role_id);
	Role* get_rand_role(BOOL force_one = FALSE);
	s_role_info* get_role_info(const DWORD dw_role_id);
	DWORD get_role_id(const DWORD name_crc);
	VOID  get_role_name(const DWORD dw_role_id, LPTSTR sz_name);
	VOID  get_role_name_by_name_id(const DWORD name_id, LPTSTR sz_name);
	VOID  get_role_signature_name(const DWORD dw_role_id, LPTSTR sz_signature_name);
	RoleMap& get_role_map(){ return role_id_ref_index_; }
	RoleIDMap& get_name_crc_map() { return namecrc_ref_role_id_; }
	local_role_mgr& get_local_role_mgr() { return local_role_mgr_; }
	
	
	BOOL is_this_world_role(const DWORD dw_role_id)
	{
		return VALID_POINT(get_role_info(dw_role_id));
	}
	template<class OP>
	VOID send_world_msg(OP& op){
		this->for_each(op);
	}

	VOID send_world_msg(LPVOID p_message, DWORD dw_size);
	VOID reset_role_reputation(ECLanType clan_type, EReputationLevel reputation_level, tagDWORDTime dw_time);
	VOID erase_all_role_info();

	template<typename UnitOperation>
	VOID for_each(UnitOperation oper);

	VOID clear_send_mail_number();
	VOID delete_role_map_limit(INT type);
	VOID clear_role_hang_number();
	VOID clear_role_paimai_number();
	//VOID clear_role_luck();
	VOID clear_role_bank_number();
	VOID reset_role_data();
	VOID reset_role_data_six();//每日六点清零
	VOID update_role_sign_level();//零点更新签到奖励等级
	VOID on_sharp_hour(INT hour);
	VOID inc_circle_refresh_number();

	VOID update_1v1_day_score();
	VOID update_week_score();

	VOID update_shihun();
	VOID reset_delay_time();

	VOID create_system_mail();

	VOID reset_sign_data();
private:
	VOID role_online(Role *p_role, BOOL first_in);
	VOID role_offline(Role* p_role);
	VOID delete_friends(DWORD dw_role_id);
	VOID add_buff_to_all(DWORD dw_buff);
private:
	RoleInfoMap role_infos_;
	RoleMap role_id_ref_index_;
	RoleIDMap namecrc_ref_role_id_;
	local_role_mgr local_role_mgr_;

	Mutex role_mutex_;
	Mutex role_info_mutex_;
};

template<typename UnitOperation>
VOID role_mgr::for_each( UnitOperation __op )
{
	Role* p_role = NULL; INT index = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, index) )
	{
		p_role = local_role_mgr_.get_role(index);
		if(VALID_POINT(p_role)) __op(p_role);
	}
}


extern role_mgr g_roleMgr;

#endif //__ROLE_MGR_H__
