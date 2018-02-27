/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//地图限制

#pragma once

class Map;
class MapMgr;
class MapScript;
class map_instance;

struct tag_map_info;

#define PVP_NUM		5
#define PVP_BIWU_NUM		1

//------------------------------------------------------------------------------------------------------
// 地图限制抽象类
//------------------------------------------------------------------------------------------------------
class MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc) = 0;
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw) = 0;
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw) = 0;
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw) = 0;

	virtual BOOL CanUseSkill() { return true; }

	const tag_map_info* get_map_info()	{ return m_pInfo; }

protected:
	MapRestrict();
	virtual ~MapRestrict();

private:
	MapRestrict(const MapRestrict& rh);
	MapRestrict& operator=(const MapRestrict& rh);

protected:
	const tag_map_info*	m_pInfo;
	MapMgr*				m_pMgr;
};

//--------------------------------------------------------------------------------------------------------
// 普通地图限制类
//--------------------------------------------------------------------------------------------------------
class MapRestrictNormal : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);
};

//--------------------------------------------------------------------------------------------------------
// 普通副本限制类
//--------------------------------------------------------------------------------------------------------
class MapRestrictInstance : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);

private:
	INT CanEnterByInstanceInfo(Role* pRole, DWORD dwMisc);
};

//---------------------------------------------------------------------------------------------------------
// 稳定副本限制类（新手副本）
//---------------------------------------------------------------------------------------------------------
class MapRestrictStable : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);

private:
	map_instance* GetOnePerfectMap();
};

//---------------------------------------------------------------------------------------------------------
// 脚本创建的副本限制类
//---------------------------------------------------------------------------------------------------------
class MapRestrictScript : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);

private:
	map_instance* GetOnePerfectMap(Role* pRole);

private:
	const MapScript*	m_pScript;
};

//--------------------------------------------------------------------------------------------------------
// 帮派地图限制类
//--------------------------------------------------------------------------------------------------------
class MapRestrictGuild : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);
private:
	INT CanEnterByInstanceInfo(Role* pRole);
};

//--------------------------------------------------------------------------------------------------------
// PVP地图限制类
//--------------------------------------------------------------------------------------------------------
class MapRestrictPvP : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);
private:
	INT CanEnterByInstanceInfo(Role* pRole);

	map_instance* pInstance[PVP_NUM];

	DWORD	get_instance_id(INT nLevel);
};

//--------------------------------------------------------------------------------------------------------
// 1v1地图限制类
//--------------------------------------------------------------------------------------------------------
class MapRestrict1v1 : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);
private:
	INT CanEnterByInstanceInfo(Role* pRole);

};


//---------------------------------------------------------------------------------------------------------
// 沙巴克副本限制类
//---------------------------------------------------------------------------------------------------------
class MapRestrictSBK : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);

private:

	map_instance* pInstance;
};


//---------------------------------------------------------------------------------------------------------
// 战场副本限制类
//---------------------------------------------------------------------------------------------------------
class MapRestrictBattle : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);

private:
	INT CanEnterByInstanceInfo(Role* pRole);

	map_instance* pInstance;
};

//--------------------------------------------------------------------------------------------------------
// PVP比武地图限制类
//--------------------------------------------------------------------------------------------------------
class MapRestrictPvPBiWu : public MapRestrict
{
public:
	virtual VOID Init(MapMgr* pMapMgr);
	virtual Map* CanEnter(Role* pRole, DWORD dwMisc);
	virtual Map* CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, DWORD& dwInstanceID, Vector3& vOut, FLOAT& fYaw);
	virtual BOOL CanUseSkill();
private:
	INT CanEnterByInstanceInfo(Role* pRole);

	map_instance* pInstance[PVP_BIWU_NUM];

	DWORD	get_instance_id(INT nLevel);
};
