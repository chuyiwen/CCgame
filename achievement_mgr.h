/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ɾ�ϵͳ

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

	// 1��ʼ��ѡ��
	void InitOpts(Role* pRole);

	// ��ʼ����ɵĳɾ�
	void InitComplate(const BYTE* &pData, const INT32 n_num);

	// ��ʼ������
	void InitCriteria(const BYTE* &pData, const INT32 n_num);
	// ��������
	void Destroy();
	
	// ֵ�ı仯����
	enum ProgressType { PROGRESS_SET, PROGRESS_ACCUMULATE, PROGRESS_HIGHEST, PROGRESS_LOWEST};

public:
	
	// ���ͳ�ʼ���ݸ��ͻ���
	VOID SendAllAchievementData();

	// ��ɵĳɾʹ浽���ݿ�
	void SaveComplateToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	// �����浽���ݿ�
	void SaveCriteriaToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	// ��������
	VOID UpdateAchievementCriteria(e_achievement_event titleEvent, DWORD dwArg1 = 0, DWORD dwArg2 = 0);


	// �Ƿ���ɽ���
	bool IsCompletedCriteria(AchievementCriteriaEntry const* criteria, AchievementEntry const* achievement) const;
	

	// ��ɽ����������ֵ
	static UINT32 GetCriteriaProgressMaxCounter(AchievementCriteriaEntry const* entry);
	
	// ������������
	void SetCriteriaProgress(AchievementCriteriaEntry const* criteria, AchievementEntry const* achievement, UINT32 changeValue, ProgressType ptype);

	// ���¼�ʱ�б�
	void DoFailedTimedAchievementCriterias();


	INT GetComplateNumber() { return m_completedAchievements.size(); }
private:

	
	// ��ɽ���
	void CompletedCriteriaFor(AchievementEntry const* achievement);
	
	// ��ɳɾ�
	void CompletedAchievement(AchievementEntry const* entry);

	// ��������������Ϣ
	void SendCriteriaUpdate(UINT32 id, CriteriaProgress const* progress);
	
	// ���ͳɾ������Ϣ
	void SendAchievementEarned(AchievementEntry const* achievement);

	// �ɾ��Ƿ����
	bool IsCompletedAchievement(AchievementEntry const* entry);

	// �������Ƿ�������
	VOID OnSignetCheng(e_achievement_signet e_signet_type);
	
	// ��ȡ�������µȼ�
	int GetBigSignetLevel();

	// ��������
	VOID OnSignetLevelUp(e_achievement_signet e_signet_type, int nLevel);
private:

	Role*								m_pRole;							// ��ɫָ��

	CriteriaProgressMap					m_criteriaProgress;			// �����е�����
	CompletedAchievementMap				m_completedAchievements;	// ��ɵĳɾ�
	AchievementCriteriaFailTimeMap		m_criteriaFailTimes;		// ��ʱ���ʱ���б�

	// �������ͳɾ��������
	INT									m_nAchievementSignetNum[eas_totle];	
};

