/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//地图管理器

#pragma once

#include "mutex.h"
#include "map.h"

struct tag_map_info;
struct tagInstance;
class Map;
class map_instance;
class map_creator;
class Role;
class MapRestrict;

class MapMgr
{
	friend class MapRestrictNormal;
	friend class MapRestrictInstance;
	friend class MapRestrictStable;
	friend class MapRestrictGuild;
	friend class MapRestrictPvP;
	friend class MapRestrict1v1;

public:
	MapMgr();
	~MapMgr();

	//-------------------------------------------------------------------------------
	// 初始化，更新和销毁
	//-------------------------------------------------------------------------------
	BOOL				Init(tag_map_info* pInfo);
	UINT				thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	VOID				Destroy();

	//--------------------------------------------------------------------------------------------------------
	// 线程管理
	//--------------------------------------------------------------------------------------------------------
	static VOID			StopThread()					{ Interlocked_Exchange((LPLONG)&m_bTerminate, TRUE); }
	tstring&			GetThreadName()					{ return m_strThreadName; }

	//--------------------------------------------------------------------------------------------------------
	// 各种Get
	//--------------------------------------------------------------------------------------------------------
	Map*				GetSingleMap()					{ return m_pSingleMap; }
	map_instance*		GetInstance(DWORD dwInstanceID) { return m_mapInstance.find(dwInstanceID); }
	const tag_map_info*	get_map_info()					{ return m_pInfo; }

	BOOL		IsNormalMap()	{ return EMT_Normal == m_pInfo->e_type; }
	BOOL		IsInstanceMap()	{ return EMT_Instance == m_pInfo->e_type; }

	//--------------------------------------------------------------------------------------------------------
	// 副本的创建和删除副本
	//--------------------------------------------------------------------------------------------------------
	map_instance*		CreateInstance(Role* pCreator, DWORD dwMisc);
	BOOL				CreateScriptInstance(DWORD dwInstanceID);
	VOID				DestroyInstance(map_instance* pInstance);

	//--------------------------------------------------------------------------------------------------------
	// 副本的入口及出口判断
	//--------------------------------------------------------------------------------------------------------
	Map*				CanEnter(Role* pRole, DWORD dwMisc=INVALID_VALUE);
	Map*				CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);

	//--------------------------------------------------------------------------------------------------------
	// 玩家传出副本
	//--------------------------------------------------------------------------------------------------------
	VOID				RoleInstanceOut(Role* pRole);

	INT				get_instance_role_num();
	INT				get_instance_leave_role_num();

	BOOL			CanUseSkill();

private:
	
	tstring						m_strThreadName;					// 线程名称
	static volatile BOOL		m_bTerminate;						// 线程停止标志

	const tag_map_info*			m_pInfo;							// 地图基本属性定义
	Map*						m_pSingleMap;						// 如果是普通地图，则该指针指向该唯一地图
	package_map<DWORD, map_instance*>   m_mapInstance;						// 如果是副本地图，则管理基于该地图创建的所有副本

	MapRestrict*				m_pRestrict;						// 策略对象
};
