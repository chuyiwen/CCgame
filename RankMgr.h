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
*	@file		RankMgr.h
*	@author		mmz
*	@date		2010/12/21	initial
*	@version	0.0.1.0
*	@brief		排行榜管理
*/


#pragma once

class  Role;
struct s_level_rank;
struct tagGuildRank;
struct NET_DB2S_load_level_rank;
struct NET_DB2S_load_guild_rank;
struct tagEquip;
struct tagKillRank;
struct NET_DB2S_load_kill_rank;
struct tag1v1ScoreRank;
struct NET_DB2S_load_1v1_score_rank;
struct tagJusticeRank;
struct NET_DB2S_load_justice_rank;
struct tagShihunRank;
struct NET_DB2S_load_shihun_rank;
struct tagAchievementPointRank;
struct tagAchievementNumberRank;
struct NET_DB2S_load_achievement_point_rank;
struct NET_DB2S_load_achievement_number_rank;
struct tagMasterGraduateRank;//gx add
struct NET_DB2S_load_MasterGraduate_rank;
struct NET_DB2S_load_mounts_rank;
struct NET_DB2S_load_reach_rank;
struct tagMountsRank;
struct tagtotalReachRank;

struct tagAward
{
	INT		n_index;
	DWORD	dw_role_id;
};

struct tagFirstThreePepop
{
	DWORD dwRoleID;
	DWORD dwType;
};

enum ERank_Type
{
	ERT_LEVELRANK		,		// 人物等级榜
	ERT_EQUIPRANK		,		// 装备排行榜
	ERT_GUILDRANK		,		// 帮会排行榜
	ERT_KILLRANK		,		// 恶人排行榜
	ERT_JUSTICERANK		,		// 战力排行榜
	ERT_1V1RANK			,		// 1v1排行榜
	ERT_SHIHUNRANK		,		// 魅力排行榜
	ERT_ACHPOINTRANK	,		// 成就点数榜
	ERT_ACHNUMBRRANK	,		// 成就数量榜
	ERT_MOUNTS			,		// 坐骑
	ERT_CHONGZHI		,		// 充值
};

class RankMgr
{
	typedef package_list<s_level_rank*> LIST_LEVELRANK;
	typedef package_list<tagEquip*> LIST_EQUIPRANK;
	typedef package_list<tagGuildRank*> LIST_GUILDRANK;
	typedef package_list<tagKillRank*> LIST_KILLRANK;
	typedef package_list<tag1v1ScoreRank*> LIST_1V1RANK;
	typedef package_list<tagJusticeRank*> LIST_JUSTICERANK;
	typedef package_list<tagShihunRank*> LIST_SHIHUNRANK;
	typedef package_list<tagAchievementPointRank*> LIST_ACHPOINTRANK;
	typedef package_list<tagAchievementNumberRank*> LIST_ACHNUMBRRANK;
	typedef package_list<tagMasterGraduateRank*> LIST_MASTERRANK;//师徒
	typedef package_list<tagMountsRank*> LIST_MOUNTSRANK;
	typedef package_list<tagtotalReachRank*> LIST_REACHRANK;
public:
	RankMgr(void);
	~RankMgr(void);

	VOID	CleanRank();

	VOID	SetLock(BOOL bLock) { Interlocked_Exchange((LONG*)&m_bLock, bLock); }
	BOOL	IsLock()	{ return m_bLock; }

	VOID	InitLevelRank(NET_DB2S_load_level_rank* rank);
	DWORD	SendLevelRank(Role* pRole);
	VOID	GetLevelRank(std::vector<DWORD>& vec);

	VOID	InitEquipRank(tagEquip* pEquip);
	DWORD	SendEquipRank(Role* pRole);
	VOID	SendEquipInfo(Role* pRole);

	VOID	InitGuildRank(NET_DB2S_load_guild_rank* rank);
	DWORD	SendGulilRank(Role* pRole);

	VOID	InitKillRank(NET_DB2S_load_kill_rank* rank);
	DWORD	SendKillRank(Role* pRole);

	VOID	InitJustice(NET_DB2S_load_justice_rank* rank);
	DWORD	SendJueticeRank(Role* pRole);

	VOID	Init1v1ScoreRank(NET_DB2S_load_1v1_score_rank* rank);
	DWORD   Send1v1ScoreRank(Role* pRole);

	VOID	InitShihunRank(NET_DB2S_load_shihun_rank* rank);
	DWORD	SendShihunRank(Role* pRole);
	
	VOID	InitAchPointRank(NET_DB2S_load_achievement_point_rank* rank);
	DWORD	SendAchPointRank(Role* pRole);

	VOID	InitAchNumberRank(NET_DB2S_load_achievement_number_rank* rank);
	DWORD	SendAchNumberRank(Role* pRole);

	VOID	InitMasterRank(NET_DB2S_load_MasterGraduate_rank* rank);
	DWORD	SendMasterRank(Role* pRole);
	VOID	InitMountsRank(NET_DB2S_load_mounts_rank* rank);

	VOID	InitreachRank(NET_DB2S_load_reach_rank* rank);

	VOID	UpdateRank();

	VOID	Role1v1GiveAward();

	VOID	RoleShihunGiveAward();

	package_list<tagShihunRank*>& GetShiHunRank() { return m_listShihunRank; }
	
	template<class T>
	DWORD	GetRankPos(DWORD dwRoleID, package_list<T*>& list);
	
	template<class T>
	DWORD	GetRankPosClass(DWORD dwRoleID, EClassType eClass, package_list<T*>& list);

	// 获取玩家在某排行榜排名
	DWORD	GetRankPos(ERank_Type eType, DWORD dwRoleID);

	DWORD	GetRankPosClass(ERank_Type eType, DWORD dwRoleID, EClassType eClass);
	// 获取雕像该显示的名字
	VOID	sendDiaoxiangName(Role* pRole);

	INT32	getZhanliLevel(DWORD dwclass, DWORD dwSex);

	// 是否是职业等级第一
	BOOL	IsClessOne(DWORD dwRoleID, int nCless);
	// 是否是职业战力第一
	BOOL	IsZhanLiClassOne(DWORD dwRoleID, int nClass);
	// 是否坐骑等级第一
	BOOL	IsMountsOne(DWORD dwRoleID);
	// 是否充值排第一
	BOOL	IsChongZhiOne(DWORD dwRoleID);
	// 是否总战力第一
	BOOL	IsZhanliOne(DWORD dwRoleID);
	// 是否魅力第一
	BOOL	IsMeiLiOne(DWORD dwRoleID, int nSex);

	// 设置头奖三人
	VOID	SetThreeFirstPeople(DWORD dwRoleID, DWORD dwType);
	// 获取头奖数据
	VOID	getOpenServerData(Role* pRole, DWORD* pData);
	static RankMgr* GetInstance();
	static VOID	   Destory();

private:

	volatile BOOL				m_bLock;
	LIST_LEVELRANK		m_listLevelRank;				// 人物等级榜
	LIST_EQUIPRANK		m_listEquipRank;				// 装备排行榜
	LIST_GUILDRANK		m_listGuildRank;				// 帮会排行榜
	LIST_KILLRANK		m_listKillRank;					// 恶人排行榜
	LIST_JUSTICERANK	m_listJusticeRank;				// 战力排行榜
	LIST_1V1RANK		m_list1v1Rank;					// 1v1排行榜
	LIST_SHIHUNRANK		m_listShihunRank;				// 魅力排行榜
	LIST_ACHPOINTRANK	m_listAchPointRank;				// 成就点数榜
	LIST_ACHNUMBRRANK	m_listAchNumberRank;			// 成就数量榜
	LIST_MASTERRANK		m_listMasterRank;				// 师徒榜 gx add
	LIST_MOUNTSRANK		m_listMountsRank;				// 坐骑排行
	LIST_REACHRANK		m_listReachRank;				// 充值排行


	std::list<tagFirstThreePepop>	m_listFirstThree;	// 最近抽到头奖三人
	TCHAR				m_zhanliName[6][X_SHORT_NAME];	// 各职业战力第一
	INT32				m_nLevel[3][2];					// 战力第一职业等级

	DWORD				m_dwZhanLiRole[3][10];			
	DWORD				m_dwLevelRole[3];
	
	DWORD				m_dwMeiliRole[2];

	static RankMgr*	m_pInstance; 
};

template<class T>
DWORD RankMgr::GetRankPos(DWORD dwRoleID, package_list<T*>& list)
{
	package_list<T*>::list_iter iter = list.begin();
	T* pRank = NULL;
	int pos = 1;
	while(list.find_next(iter, pRank))
	{
		if (pRank->dw_role_id == dwRoleID)
		{
			return pos;
		}
		pos++;
	}

	return 100000;
}

template<class T>
DWORD RankMgr::GetRankPosClass(DWORD dwRoleID, EClassType eClass, package_list<T*>& list)
{
	package_list<T*>::list_iter iter = list.begin();
	T* pRank = NULL;
	int pos = 1;
	while(list.find_next(iter, pRank))
	{
		if (pRank->e_class != eClass)
			continue;

		if (pRank->dw_role_id == dwRoleID)
		{
			return pos;
		}
		pos++;
	}

	return 100000;
}
//extern RankMgr g_RankMgr;
