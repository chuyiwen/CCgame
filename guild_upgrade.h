/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//帮会设施

#pragma once

#include "../../common/WorldDefine/guild_define.h"

#pragma pack(push, 1)

//-----------------------------------------------------------------------------
// 帮派设施结构
//-----------------------------------------------------------------------------
struct tagGuildFacilities : public tagGuildFacilitiesInfo
{
	INT16	nStep;
	INT32	nBaseExploit;

	tagGuildFacilities()
	{
		ZeroMemory(this, sizeof(*this));
	}
};

struct s_facilities_load;
class guild;

class GuildUpgrade
{
public:
	GuildUpgrade();
	~GuildUpgrade();

	VOID	Init(guild* pGuild, BOOL bRequest = FALSE);
	BOOL	is_init_ok()	{ return m_bInit; }

	VOID	UpDate();

	// 重新创建缺失设施
	VOID	CreateFacilities();

	// 载入设施信息
	BOOL	LoadFacilitiesInfo(const s_facilities_load* pInfo, INT n_num);

	// 获取设施信息接口
	DWORD	GetGuildFacilitiesInfo(tagGuildFacilitiesInfo* pInfo, EFacilitiesType eType = EFT_End);

	// 上缴物品接口
	DWORD	HandInItems(Role* pRole, EFacilitiesType eType);

	// 保存到数据库
	VOID	SaveUpgradeInfo2DB(EFacilitiesType eType);

	// 清除数据库中所有设施信息
	VOID	RemoveAllFacilitiesInfo();
	
	// 调整帮派设施的等级
	VOID	ChangeFacilitiesLevel(EFacilitiesType eType, BYTE byValue);

	// 调整帮会设施使用次数
	VOID	ChangeFacilitiesUseNum(EFacilitiesType eType, BYTE byValue);

	// 调整帮派设施最大等级
	VOID	ReSetFacilitiesMaxLevel();

	// 每日扣除的设施繁荣度
	VOID	DayDecProsperity();

	// 清空帮会圣兽使用次数
	VOID   ClearHolinessNum();

	// 上限获取
	INT32	GetMaxFund()		{ return 2000000*m_Facilities[EFT_Lobby].byLevel/* + 2000000*m_Facilities[EFT_Fund].byLevel*/; }
	INT32	GetMaxMaterial()	{ return 10000000/* + 20000*m_Facilities[EFT_Material].byLevel*/; }
	BYTE	GetMaxSkillLevel()	{ return 20/* + (2 * m_Facilities[EFT_Academy].byLevel)*/; }

	INT		GetFacilitiesLevel(BYTE Type);

	VOID	SetFacilitiesID(BYTE Type, DWORD dwID, INT nIndex = 0);

	VOID    CleanFacilitiesID()
	{
		for(INT i = EFT_Lobby; i < EFT_End; i++)
		{
			for (int j = 0; j < MAX_GUILD_UPGRADE_NUM; j++)
			{
				m_Facilities[i].dwFacilitiesID[j] = INVALID_VALUE;
			}
		}
	}

	DWORD	GetFacilitiesID(BYTE Type, INT nIndex = 0)
	{
		if(Type < EFT_Lobby || Type > EFT_Holiness)
			return INVALID_VALUE;
		
		if (nIndex < 0 || nIndex >= MAX_GUILD_UPGRADE_NUM)
			return INVALID_VALUE;

		return m_Facilities[Type].dwFacilitiesID[nIndex];
	}

	VOID	FacilitesDestory(EFacilitiesType eType);

	VOID	SetFacilitesReliveTime(BOOL bWin);

	VOID	FacilitesRelive(EFacilitiesType eType);

	VOID	ChangeFacilitesLevel(EFacilitiesType eType, BOOL bAdd = TRUE);

	BOOL	IsFacilitesUpLevel();

	BOOL	IsFacilitesDestory();

	BOOL	IsFacilitesDestory(EFacilitiesType eType);

	VOID	SetFacilitesUpLevel(BYTE byType);

	DWORD	StartGuildPractice(Role* pRole);

	VOID	ResetGuildPracitceUseNum();

	BOOL	IsLobbyUsing();

private:
	BOOL					m_bInit;
	guild*					m_pGuild;

	tagGuildFacilities		m_Facilities[EFT_End];
};

//-----------------------------------------------------------------------------
#pragma pack(pop)