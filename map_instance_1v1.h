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
*	@file		map_instance_pvp.h
*	@author		mmz
*	@date		2010/11/3	initial
*	@version	0.0.1.0
*	@brief		1v1����
*/


#pragma once

const INT ROLE_LEAVE_1V1_INSTANCE_TICK_COUNT_DOWN	= 60 * TICK_PER_SECOND;		// ��ɫ�ڸ������뿪����ʱ�����ͳ������ĵ���ʱ��60��

#include "map.h"
#include "instance_define.h"

class map_instance_1v1 : public map_instance
{
public:
	map_instance_1v1(void);
	virtual ~map_instance_1v1(void);

	virtual BOOL		init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator=NULL, DWORD dwMisc=INVALID_VALUE);
	virtual VOID		update();
	virtual VOID		destroy();

	virtual VOID		add_role(Role* pRole,  Map* pSrcMap = NULL);
	virtual VOID		role_leave_map(DWORD dwID, BOOL b_leave_online = FALSE);
	virtual INT			can_enter(Role *pRole, DWORD dwParam = 0);
	virtual BOOL		can_destroy();
	virtual VOID		on_destroy();

	DWORD				cal_time_limit();

	/*VOID				on_act_end();*/
	VOID				on_leave_instance(DWORD dw_role_id);

protected:
	virtual VOID		on_delete();
	virtual BOOL		init_all_spawn_point_creature(DWORD dwGuildID = INVALID_VALUE);

private:
	VOID				update_time_issue();
	BOOL				is_time_limit()		{ return m_pInstance->dwTimeLimit > 0 && m_pInstance->dwTargetLimit != INVALID_VALUE; }

	//VOID				set_instance_id(DWORD dwInstanceID, INT nActID);

private:

	BOOL					m_bNoEnter;						// �����Ƿ�û�˽����
	DWORD					m_dwStartTick;					// ��ʼʱ��
	DWORD					m_dwEndTick;					// ������ʼ���õ���ʱ

	DWORD					dw_pvp_id;

	package_map<DWORD, INT>		m_mapWillOutRoleID;				// ����������������ȴ������ȥ������б�

	const tagInstance*		m_pInstance;					// ������̬����
};