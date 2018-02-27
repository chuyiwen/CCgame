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
*	@file		map_instance_pvp_match.h
*	@author		zhjl
*	@date		2013/12/18	initial
*	@version	0.0.1.0
*	@brief		pvp比武副本
*/


#pragma once

const INT ROLE_LEAVE_PVP_BIWU_INSTANCE_TICK_COUNT_DOWN	= 60 * TICK_PER_SECOND;		// 角色在副本内离开家族时，传送出副本的倒计时，60秒

#include "map.h"
#include "instance_define.h"

class map_instance_pvp_biwu : public map_instance
{
public:
	map_instance_pvp_biwu(void);
	virtual ~map_instance_pvp_biwu(void);

	virtual BOOL		init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator=NULL, DWORD dwMisc=INVALID_VALUE);
	virtual VOID		update();
	virtual VOID		destroy();

	virtual VOID		add_role(Role* pRole,  Map* pSrcMap = NULL);
	virtual VOID		role_leave_map(DWORD dwID, BOOL b_leave_online = FALSE);
	virtual INT			can_enter(Role *pRole, DWORD dwParam = 0);
	virtual BOOL		can_destroy();
	virtual VOID		on_destroy();

	DWORD				cal_time_limit();

	VOID				on_act_end();
	VOID				on_leave_instance(DWORD dw_role_id);

protected:
	virtual VOID		on_delete();
	virtual BOOL		init_all_spawn_point_creature(DWORD dwGuildID = INVALID_VALUE);

private:
	VOID				update_time_issue();
	BOOL				is_time_limit()		{ return m_pInstance->dwTimeLimit > 0 && m_pInstance->dwTargetLimit != INVALID_VALUE; }

	VOID				set_instance_id(DWORD dwInstanceID, INT nActID);

private:

	BOOL					m_bNoEnter;						// 副本是否还没人进入过
	DWORD					m_dwStartTick;					// 开始时间
	DWORD					m_dwEndTick;					// 副本开始重置倒计时

	INT						m_nActID;							//  活动ID

	package_map<DWORD, INT>		m_mapWillOutRoleID;				// 不再属于这个副本等待传输出去的玩家列表

	const tagInstance*		m_pInstance;					// 副本静态属性
};
