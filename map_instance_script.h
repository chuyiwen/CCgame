/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//脚本副本

#pragma once

#include "map.h"

class map_instance;

//------------------------------------------------------------------------------
// 脚本创建的副本类
//------------------------------------------------------------------------------
class MapInstanceScript : public map_instance
{
public:
	MapInstanceScript();
	virtual ~MapInstanceScript();

	virtual BOOL		init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator=NULL, DWORD dwMisc=INVALID_VALUE);
	virtual VOID		update();
	virtual VOID		destroy();

	virtual VOID		add_role(Role* pRole,  Map* pSrcMap = NULL);
	virtual	VOID		role_leave_map(DWORD dwID, BOOL b_leave_online = FALSE);
	virtual INT			can_enter(Role *pRole);
	virtual BOOL		can_destroy();
	virtual VOID		on_destroy();

protected:
	virtual VOID		on_delete();
	virtual BOOL		init_all_spawn_point_creature(DWORD dwGuildID = INVALID_VALUE);
};