/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//VIP摊位

#pragma once
#include "event_mgr.h"
#include "../../common/WorldDefine/stall_define.h"

struct tagVIPStall;
struct s_vip_stall_to_db;

class VIPStall : public EventMgr<VIPStall>
{
public:
	VIPStall();
	~VIPStall();

	BOOL	Init();
	VOID	Update();
	VOID	Destroy();

	// 事件处理函数
	VOID	OnSetStallTitleFunc(DWORD dwSenderID, VOID* pEventMessage);
	VOID	OnSetStallStatusFunc(DWORD dwSenderID, VOID* pEventMessage);
	VOID	OnChangeStallGoodsFunc(DWORD dwSenderID, VOID* pEventMessage);

	// 地图上层消息处理
	DWORD	ApplyVIPStall(Role* pRole, BYTE byIndex, INT32 nRent);
	DWORD	GetAllVIPStallInfo(OUT tagVIPStall* pStallInfo, DWORD& dw_time);
	DWORD	GetUpdatedStallInfo(OUT tagVIPStall* pStallInfo, INT32& n_num, DWORD& dw_time);
	DWORD	GetVIPStallGoods(OUT LPVOID pData, OUT BYTE &byGoodsNum, OUT INT &nGoodsSz, DWORD& dwRequestTime, BYTE byIndex);

public:
	// 摊位信息相关
	tagVIPStall*	GetVIPStallInfo(BYTE index);
	tagVIPStall*	GetVIPStallInfo(DWORD dw_role_id);
	Role*			GetVIPStallRole(BYTE index);
	BOOL			IsInStatus(BYTE index, EVIPStallStatus eStatus);
	DWORD			GetSpecVIPStallGoodsTime(BYTE index);

	// 设置摊位状态
	DWORD			SetVIPStallStatus(BYTE index, EVIPStallStatus eStatus);

	// 角色删除时的清理操作
	VOID			RemoveRoleVIPStall(DWORD dw_role_id);

	// 读取所有VIP摊位信息
	DWORD	LoadAllVIPStallInfo(s_vip_stall_to_db* pVIPStallInfo);

private:
	static	VOID RegisterStallEventFunc();

private:
	vector<tagVIPStall>		m_vecVIPStall;		// VIP摊位信息
	package_map<DWORD, BYTE>		m_mapVIPStall;		// 角色ID与VIP摊位序号对照表
	BOOL					m_bInit;

	DWORD					m_dwVIPStallChange;	// VIP摊位变更时刻
	tagDWORDTime			m_dwSecTimer;		// 更新计时
	INT						m_nMinuteTimer;		// 分钟计时
};

extern VIPStall g_VIPStall;