/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//成就系统

#pragma once

// project
#include "../../common/WorldDefine/achievement_define.h"
#include "att_res.h"

// stl
#include <list>
#include <hash_map>

typedef stdext::hash_map<DWORD, CriteriaProgress> CriteriaProgressMap;
typedef stdext::hash_map<DWORD, CompletedAchievementData> CompletedAchievementMap;

typedef std::map<DWORD,tagDWORDTime> AchievementCriteriaFailTimeMap;

class AchievementMgr
{

public:
	AchievementMgr();
	~AchievementMgr(){	Destroy();	}

	// 1初始化选项
	void InitOpts(Role* pRole);

	// 初始化完成的成就
	void InitComplate(const BYTE* &pData, const INT32 n_num);

	// 初始化条件
	void InitCriteria(const BYTE* &pData, const INT32 n_num);
	// 销毁数据
	void Destroy();
	
	// 值的变化类型
	enum ProgressType { PROGRESS_SET, PROGRESS_ACCUMULATE, PROGRESS_HIGHEST, PROGRESS_LOWEST};

public:
	
	// 发送初始数据给客户端
	VOID SendAllAchievementData();

	// 完成的成就存到数据库
	void SaveComplateToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	// 条件存到数据库
	void SaveCriteriaToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	// 更新条件
	VOID UpdateAchievementCriteria(e_achievement_event titleEvent, DWORD dwArg1 = 0, DWORD dwArg2 = 0);


	// 是否完成进度
	bool IsCompletedCriteria(AchievementCriteriaEntry const* criteria, AchievementEntry const* achievement) const;
	

	// 完成进度所需最大值
	static UINT32 GetCriteriaProgressMaxCounter(AchievementCriteriaEntry const* entry);
	
	// 设置条件进度
	void SetCriteriaProgress(AchievementCriteriaEntry const* criteria, AchievementEntry const* achievement, UINT32 changeValue, ProgressType ptype);

	// 更新计时列表
	void DoFailedTimedAchievementCriterias();


	INT GetComplateNumber() { return m_completedAchievements.size(); }
private:

	
	// 完成进度
	void CompletedCriteriaFor(AchievementEntry const* achievement);
	
	// 完成成就
	void CompletedAchievement(AchievementEntry const* entry);

	// 发送条件更新消息
	void SendCriteriaUpdate(UINT32 id, CriteriaProgress const* progress);
	
	// 发送成就完成消息
	void SendAchievementEarned(AchievementEntry const* achievement);

	// 成就是否完成
	bool IsCompletedAchievement(AchievementEntry const* entry);

	// 看纹章是否升级了
	VOID OnSignetCheng(e_achievement_signet e_signet_type);
	
	// 获取天启徽章等级
	int GetBigSignetLevel();

	// 纹章升级
	VOID OnSignetLevelUp(e_achievement_signet e_signet_type, int nLevel);
private:

	Role*								m_pRole;							// 角色指针

	CriteriaProgressMap					m_criteriaProgress;			// 进行中的条件
	CompletedAchievementMap				m_completedAchievements;	// 完成的成就
	AchievementCriteriaFailTimeMap		m_criteriaFailTimes;		// 计时类的时间列表

	// 各种类型成就完成数量
	INT									m_nAchievementSignetNum[eas_totle];	
};

