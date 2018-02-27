
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
#pragma once

#include "map.h"

class map_instance;

class map_instance_battle : public map_instance
{
public:
	map_instance_battle();
	virtual ~map_instance_battle();


	virtual BOOL		init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator=NULL, DWORD dwMisc=INVALID_VALUE);
	virtual VOID		update();
	virtual VOID		destroy();

	virtual VOID		add_role(Role* pRole, Map* pSrcMap = NULL);
	virtual	VOID		role_leave_map(DWORD dwID, BOOL b_leave_online = FALSE);
	virtual INT			can_enter(Role *pRole);
	virtual BOOL		can_destroy();
	virtual VOID		on_destroy();
	virtual	 DWORD		FriendEnemy(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_);

	virtual VOID		on_creature_die(Creature* p_creature_, Unit* p_killer_);
	virtual	VOID		on_role_die(Role* p_role_, Unit* p_killer_);

	VOID				remove_all_player();
protected:
	virtual VOID		on_delete();
	virtual BOOL		init_all_spawn_point_creature(DWORD dwGuildID = INVALID_VALUE);

private:
	VOID				update_time_issue();

private:

	package_map<DWORD, INT>		m_map_will_out_role_id;				// 不再属于这个副本等待传输出去的玩家列表

	const tagInstance*		m_p_instance;					// 副本静态属性

	DWORD					m_dw_end_tick;					// 副本开始重置倒计时	

};