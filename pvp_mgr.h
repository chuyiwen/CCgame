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
*	@file		pvp_mgr.h
*	@author		lc
*	@date		2011/10/20	initial
*	@version	0.0.1.0
*	@brief		1v1№ЬАн
*/

#ifndef PVP_MGR
#define PVP_MGR

#include "../../common/WorldDefine/map_protocol.h"
#include "mutex.h"

typedef package_map<DWORD, tag1v1*> MAP_1V1;

class pvp_mgr
{
public:
	pvp_mgr(void);
	~pvp_mgr(void);

	VOID	destroy();
	VOID	update();

	VOID	create_pvp_id(DWORD& dw_pvp_id_);

	DWORD	apply_1v1(Role* pRole);

	DWORD	pair_1v1(Role* pRole);

	INT		get_level_index(INT nLevel);

	INT		get_level_yuanbao(INT nLevel);

	BOOL	is_cost_ticket(INT nScore, INT nLevel);

	INT		get_level_pumping_into(INT nLevel);

	INT		get_level_update_score(Role* pRole);

	VOID	updata_role_score(DWORD	dw_role_id, BOOL bAdd);

	DWORD	get_1v1_award(Role* pRole);

	DWORD	set_1v1_pair_id(Role* pRole, INT nIndex);

	tag1v1* get_1v1_info(Role* pRole);

	tag1v1*	get_reservation_info(Role* pRole);

	VOID	role_offline(Role* pRole);

	DWORD	role_leave_queue(Role* pRole);

	DWORD	pair_reservation(Role* pRole, Role* pReservation, INT nYuanbao);

private:
	MAP_1V1		map_reservation;

	MAP_1V1		map_1v1[3];

	DWORD		dw_pvp_id;

	Mutex		m_lock;
};

extern pvp_mgr g_pvp_mgr;

#endif;
