/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ͼ����

#pragma once

class Map;
class MapMgr;
class MapScript;
class map_instance;

struct tag_map_info;

#define PVP_NUM		5
#define PVP_BIWU_NUM		1

//------------------------------------------------------------------------------------------------------
// ��ͼ���Ƴ�����
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
// ��ͨ��ͼ������
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
// ��ͨ����������
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
// �ȶ����������ࣨ���ָ�����
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
// �ű������ĸ���������
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
// ���ɵ�ͼ������
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
// PVP��ͼ������
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
// 1v1��ͼ������
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
// ɳ�Ϳ˸���������
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
// ս������������
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
// PVP�����ͼ������
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
