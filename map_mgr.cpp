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

#include "StdAfx.h"
#include "map_mgr.h"
#include "map.h"
#include "map_creator.h"
#include "map_restrict.h"
#include "role.h"
//#include "mem_map.h"

volatile BOOL MapMgr::m_bTerminate = FALSE;
//-------------------------------------------------------------------------------------------------------
// constructor
//-------------------------------------------------------------------------------------------------------
MapMgr::MapMgr() : m_pInfo(NULL), m_pSingleMap(NULL), m_pRestrict(NULL)
{
	m_mapInstance.clear();
}

//-------------------------------------------------------------------------------------------------------
// destructor
//-------------------------------------------------------------------------------------------------------
MapMgr::~MapMgr()
{
	Destroy();
}

//-------------------------------------------------------------------------------------------------------
// 初始化地图信息，副本的初始化不在这里进行
//-------------------------------------------------------------------------------------------------------
BOOL MapMgr::Init(tag_map_info* pInfo)
{
	ASSERT( VALID_POINT(pInfo) );

	m_pInfo = pInfo;

	// 生成地图策略
	m_pRestrict = g_mapCreator.create_factory_map_restrict(m_pInfo);
	m_pRestrict->Init(this);

	// 根据地图信息是不是副本决定是否生成NavMap
	if( EMT_Normal == pInfo->e_type )
	{
		m_pSingleMap = g_mapCreator.create_factory_map(pInfo);
		if( !VALID_POINT(m_pSingleMap) )
			return FALSE;

		if( FALSE == m_pSingleMap->init(pInfo) )
		{
			g_mapCreator.destroy_factory_map(m_pSingleMap);
			return FALSE;
		}
	}

	// 生成Update线程
	m_strThreadName = _T("Thread_");
	m_strThreadName += m_pInfo->sz_map_name;
	World::p_thread->create_thread(m_strThreadName.c_str(), &MapMgr::static_thread_update, this);

	while( FALSE == World::p_thread->is_thread_active(m_strThreadName.c_str()) )
		continue;

	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// 线程更新函数
//-------------------------------------------------------------------------------------------------------
UINT MapMgr::thread_update()
{
	//modify mmz at 2010.9.17 release也记dump
//#ifdef _DEBUG
	_set_se_translator(serverdump::si_translation);

	try
	{
//#endif
		INT nWhichEvent = 0;

		while(TRUE)
		{
			g_mapCreator.get_all_map_start_event(nWhichEvent).Wait();

			if( m_bTerminate )
				break;

			// 普通地图
			if( EMT_Normal == m_pInfo->e_type )
			{
				// added by mmz 如果地图没有人不刷新
				//if(m_pSingleMap->GetRoleNum() > 0)
				{
					// 调用地图的更新
					m_pSingleMap->update();
				}
			}

			// 副本地图
			else
			{
				// 更新每一个副本
				m_mapInstance.reset_iterator();
				map_instance* pInstance = NULL;
				while( m_mapInstance.find_next(pInstance) )
				{
					pInstance->update();

					// 副本可以删除，删除副本
					if( pInstance->can_destroy() )
					{
						g_mapCreator.instance_destroyed(m_pInfo->dw_id, pInstance->get_instance_id());
					}
				}
			}

			// 通知mapcreator已经执行完毕
			g_mapCreator.one_map_manager_end();

			nWhichEvent = ( (nWhichEvent == 0) ? 1 : 0 );

		}
//#ifdef _DEBUG
	}
	catch(serverdump::throw_exception)
	{
#ifdef _DEBUG
		__asm int 3;
#endif
		if( get_tool()->is_debug_present() )
			throw;
		else
		{
			//WriteMem();
			
			PlayerSession::UnRegisterALL();

			SI_LOG->write_log(_T("map_mgr::thread_update shut down"));	
			// 设置ShutDown
			g_world.ShutDown();

			Sleep(3000);

			// 通知mapcreator已经执行完毕
			g_mapCreator.one_map_manager_end();
		}
	}
//#endif
	_endthreadex(0);

	return 0;
}

UINT MapMgr::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	MapMgr* p_this = (MapMgr*)p_data;
	return p_this->thread_update();
}
//-------------------------------------------------------------------------------------------------------
// 销毁
//-------------------------------------------------------------------------------------------------------
VOID MapMgr::Destroy()
{
	// 先删除地图
	if( EMT_Normal == m_pInfo->e_type )
	{
		g_mapCreator.destroy_factory_map(m_pSingleMap);
	}
	else
	{
		m_mapInstance.reset_iterator();
		map_instance* pInstance = NULL;

		while( m_mapInstance.find_next(pInstance) )
		{
			g_mapCreator.destroy_factory_map(pInstance);
		}
		m_mapInstance.clear();
	}

	// 再删除策略
	g_mapCreator.destroy_factory_map_restrict(m_pRestrict);

	//SAFE_DELETE(m_pSparseGraph);
}

//--------------------------------------------------------------------------------------------------------
// 传出副本
//--------------------------------------------------------------------------------------------------------
VOID MapMgr::RoleInstanceOut(Role *pRole)
{
	Vector3 vDestPos = pRole->GetCurPos();
	DWORD	dwDestMapID = INVALID_VALUE;
	FLOAT	fYaw = 0.0f;

	// 得到出口地图和坐标
	if( FALSE == m_pRestrict->GetExportMapAndCoord(pRole, dwDestMapID, vDestPos, fYaw) )
	{
		SI_LOG->write_log(_T("Try to out of the instance failed, mapid=%d\n"), m_pInfo->dw_id);
		return;
	}

	pRole->GotoNewMap(dwDestMapID, vDestPos.x, vDestPos.y, vDestPos.z, cosf((270-fYaw) * 3.1415927f / 180.0f), 0.0f, sinf((270-fYaw) * 3.1415927f / 180.0f));
}

//--------------------------------------------------------------------------------------------------------
// 获取副本内人数
//--------------------------------------------------------------------------------------------------------
INT MapMgr::get_instance_role_num()
{
	map_instance* pTemp = NULL;
	INT n_num = 0;
	package_map<DWORD, map_instance*>::map_iter it = m_mapInstance.begin();

	while( m_mapInstance.find_next(it, pTemp) )
	{
		if(VALID_POINT(pTemp))
			n_num += pTemp->get_role_num();
	}
	return n_num;
}

//--------------------------------------------------------------------------------------------------------
// 获取副本内离线修炼人数
//--------------------------------------------------------------------------------------------------------
INT MapMgr::get_instance_leave_role_num()
{
	map_instance* pTemp = NULL;
	INT n_num = 0;
	package_map<DWORD, map_instance*>::map_iter it = m_mapInstance.begin();

	while( m_mapInstance.find_next(it, pTemp) )
	{
		if(VALID_POINT(pTemp))
			n_num += pTemp->get_leave_role_num();
	}
	return n_num;
}

//--------------------------------------------------------------------------------------------------------
// 创建副本
//--------------------------------------------------------------------------------------------------------
map_instance* MapMgr::CreateInstance(Role* pCreator, DWORD dwMisc)
{
	if( !g_mapCreator.can_create_instance(m_pInfo) ) return NULL;

	map_instance* pInstance = g_mapCreator.create_factory_map_instance(m_pInfo);
	if( !VALID_POINT(pInstance) ) return NULL;

	g_mapCreator.add_instance(m_pInfo);

	DWORD dwInstanceID = g_mapCreator.create_new_instance_id();

	if( FALSE == pInstance->init(m_pInfo, dwInstanceID, pCreator, dwMisc) )
	{
		DestroyInstance(pInstance);
		return NULL;
	}

	m_mapInstance.add(pInstance->get_instance_id(), pInstance);
	
	pInstance->init_creature_script();

	return pInstance;
}

//--------------------------------------------------------------------------------------------------------
// 脚本创建副本
//--------------------------------------------------------------------------------------------------------
BOOL MapMgr::CreateScriptInstance(DWORD dwInstanceID)
{
	if( !g_mapCreator.can_create_instance(m_pInfo) ) return FALSE;

	if( EMT_ScriptCreate != m_pInfo->e_type ) return FALSE;

	map_instance* pInstance = g_mapCreator.create_factory_map_instance(m_pInfo);
	if( !VALID_POINT(pInstance) ) return FALSE;

	g_mapCreator.add_instance(m_pInfo);

	if( FALSE == pInstance->init(m_pInfo, dwInstanceID, NULL, INVALID_VALUE) )
	{
		DestroyInstance(pInstance);
		return FALSE;
	}

	m_mapInstance.add(pInstance->get_instance_id(), pInstance);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
// 删除副本
//---------------------------------------------------------------------------------------------------------
VOID MapMgr::DestroyInstance(map_instance* pInstance)
{
	if( !VALID_POINT(pInstance) ) return;

	pInstance->on_destroy();

	m_mapInstance.erase(pInstance->get_instance_id());
	g_mapCreator.remove_instance(m_pInfo);
	g_mapCreator.destroy_factory_map(pInstance);
}

//---------------------------------------------------------------------------------------------------------
// 是否能够进入
//---------------------------------------------------------------------------------------------------------
Map* MapMgr::CanEnter(Role* pRole, DWORD dwMisc/* =INVALID_VALUE */)
{
	return m_pRestrict->CanEnter(pRole, dwMisc);
}

//---------------------------------------------------------------------------------------------------------
// 上线后是否能够进入，否则返回出口地图及坐标
//---------------------------------------------------------------------------------------------------------
Map* MapMgr::CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw)
{
	return m_pRestrict->CanTakeOverWhenOnline(pRole, dwOutMapID, vOut, fYaw);
}

BOOL MapMgr::CanUseSkill()
{
	return m_pRestrict->CanUseSkill();
}