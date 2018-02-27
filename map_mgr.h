/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ͼ������

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
	// ��ʼ�������º�����
	//-------------------------------------------------------------------------------
	BOOL				Init(tag_map_info* pInfo);
	UINT				thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	VOID				Destroy();

	//--------------------------------------------------------------------------------------------------------
	// �̹߳���
	//--------------------------------------------------------------------------------------------------------
	static VOID			StopThread()					{ Interlocked_Exchange((LPLONG)&m_bTerminate, TRUE); }
	tstring&			GetThreadName()					{ return m_strThreadName; }

	//--------------------------------------------------------------------------------------------------------
	// ����Get
	//--------------------------------------------------------------------------------------------------------
	Map*				GetSingleMap()					{ return m_pSingleMap; }
	map_instance*		GetInstance(DWORD dwInstanceID) { return m_mapInstance.find(dwInstanceID); }
	const tag_map_info*	get_map_info()					{ return m_pInfo; }

	BOOL		IsNormalMap()	{ return EMT_Normal == m_pInfo->e_type; }
	BOOL		IsInstanceMap()	{ return EMT_Instance == m_pInfo->e_type; }

	//--------------------------------------------------------------------------------------------------------
	// �����Ĵ�����ɾ������
	//--------------------------------------------------------------------------------------------------------
	map_instance*		CreateInstance(Role* pCreator, DWORD dwMisc);
	BOOL				CreateScriptInstance(DWORD dwInstanceID);
	VOID				DestroyInstance(map_instance* pInstance);

	//--------------------------------------------------------------------------------------------------------
	// ��������ڼ������ж�
	//--------------------------------------------------------------------------------------------------------
	Map*				CanEnter(Role* pRole, DWORD dwMisc=INVALID_VALUE);
	Map*				CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);

	//--------------------------------------------------------------------------------------------------------
	// ��Ҵ�������
	//--------------------------------------------------------------------------------------------------------
	VOID				RoleInstanceOut(Role* pRole);

	INT				get_instance_role_num();
	INT				get_instance_leave_role_num();

	BOOL			CanUseSkill();

private:
	
	tstring						m_strThreadName;					// �߳�����
	static volatile BOOL		m_bTerminate;						// �߳�ֹͣ��־

	const tag_map_info*			m_pInfo;							// ��ͼ�������Զ���
	Map*						m_pSingleMap;						// �������ͨ��ͼ�����ָ��ָ���Ψһ��ͼ
	package_map<DWORD, map_instance*>   m_mapInstance;						// ����Ǹ�����ͼ���������ڸõ�ͼ���������и���

	MapRestrict*				m_pRestrict;						// ���Զ���
};
