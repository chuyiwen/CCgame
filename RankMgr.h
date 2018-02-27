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
*	@brief		���а����
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
	ERT_LEVELRANK		,		// ����ȼ���
	ERT_EQUIPRANK		,		// װ�����а�
	ERT_GUILDRANK		,		// ������а�
	ERT_KILLRANK		,		// �������а�
	ERT_JUSTICERANK		,		// ս�����а�
	ERT_1V1RANK			,		// 1v1���а�
	ERT_SHIHUNRANK		,		// �������а�
	ERT_ACHPOINTRANK	,		// �ɾ͵�����
	ERT_ACHNUMBRRANK	,		// �ɾ�������
	ERT_MOUNTS			,		// ����
	ERT_CHONGZHI		,		// ��ֵ
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
	typedef package_list<tagMasterGraduateRank*> LIST_MASTERRANK;//ʦͽ
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

	// ��ȡ�����ĳ���а�����
	DWORD	GetRankPos(ERank_Type eType, DWORD dwRoleID);

	DWORD	GetRankPosClass(ERank_Type eType, DWORD dwRoleID, EClassType eClass);
	// ��ȡ�������ʾ������
	VOID	sendDiaoxiangName(Role* pRole);

	INT32	getZhanliLevel(DWORD dwclass, DWORD dwSex);

	// �Ƿ���ְҵ�ȼ���һ
	BOOL	IsClessOne(DWORD dwRoleID, int nCless);
	// �Ƿ���ְҵս����һ
	BOOL	IsZhanLiClassOne(DWORD dwRoleID, int nClass);
	// �Ƿ�����ȼ���һ
	BOOL	IsMountsOne(DWORD dwRoleID);
	// �Ƿ��ֵ�ŵ�һ
	BOOL	IsChongZhiOne(DWORD dwRoleID);
	// �Ƿ���ս����һ
	BOOL	IsZhanliOne(DWORD dwRoleID);
	// �Ƿ�������һ
	BOOL	IsMeiLiOne(DWORD dwRoleID, int nSex);

	// ����ͷ������
	VOID	SetThreeFirstPeople(DWORD dwRoleID, DWORD dwType);
	// ��ȡͷ������
	VOID	getOpenServerData(Role* pRole, DWORD* pData);
	static RankMgr* GetInstance();
	static VOID	   Destory();

private:

	volatile BOOL				m_bLock;
	LIST_LEVELRANK		m_listLevelRank;				// ����ȼ���
	LIST_EQUIPRANK		m_listEquipRank;				// װ�����а�
	LIST_GUILDRANK		m_listGuildRank;				// ������а�
	LIST_KILLRANK		m_listKillRank;					// �������а�
	LIST_JUSTICERANK	m_listJusticeRank;				// ս�����а�
	LIST_1V1RANK		m_list1v1Rank;					// 1v1���а�
	LIST_SHIHUNRANK		m_listShihunRank;				// �������а�
	LIST_ACHPOINTRANK	m_listAchPointRank;				// �ɾ͵�����
	LIST_ACHNUMBRRANK	m_listAchNumberRank;			// �ɾ�������
	LIST_MASTERRANK		m_listMasterRank;				// ʦͽ�� gx add
	LIST_MOUNTSRANK		m_listMountsRank;				// ��������
	LIST_REACHRANK		m_listReachRank;				// ��ֵ����


	std::list<tagFirstThreePepop>	m_listFirstThree;	// ����鵽ͷ������
	TCHAR				m_zhanliName[6][X_SHORT_NAME];	// ��ְҵս����һ
	INT32				m_nLevel[3][2];					// ս����һְҵ�ȼ�

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
