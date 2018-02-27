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
 *	@file		guerdon_quest_mgr
 *	@author		mwh
 *	@date		2011/04/18	initial
 *	@version	0.0.1.0
 *	@brief		悬赏任务,使用event_mgr来做
*/

#ifndef __GUERDON_QUEST_MGR_H__
#define __GUERDON_QUEST_MGR_H__

#include "../../common/WorldDefine/guerdon_quest_define.h"
#include "event_mgr.h"

struct guerdon_quest;

enum ERwardOpType
{
	EROT_Complete,
	EROT_Timeout,
	EROT_Cancel,
	EROT_GiveUp,
};

// 客户端脚本将根据这个拼接邮件内容
enum ERewardMailOP
{
	ERMOP_Cancel,						// 发布人取消悬赏
	ERMOP_Timeout,					// 悬赏过期奖励返还
	ERMOP_Failed,							// 悬赏被接取后未能完成时，退还悬赏奖励的邮件
	ERMOP_FailedGold,					// 悬赏被接取后未能完成时，给予悬赏押金的邮件
	ERMOP_Complete,					// 悬赏任务完成后，给予悬赏奖励的邮件
	ERMOP_CompleteGold,			// 悬赏任务完成后，退还悬赏押金的邮件
};

struct IDRefHelper
{
	INT nNumber;
	INT64 n64ID[MAXGDQUEST];
	UINT16 u16ID[MAXGDQUEST];
public:
	IDRefHelper(){ZeroMemory(this,sizeof(*this));}
public:
	BOOL Add(UINT16 u16, INT64 n64)
	{
		if(nNumber>=MAXGDQUEST) return FALSE;
		n64ID[nNumber] = n64;
		u16ID[nNumber] = u16;
		++nNumber;
		return TRUE;
	}

	VOID Remove(UINT16 u16)
	{
		INT nIndex = GetIndex(u16);
		if(nIndex != -1)
		{
			for(INT n = nIndex+1; n < nNumber; ++n)
			{
				if(n64ID[n])
				{
					n64ID[nIndex] = n64ID[n];
					u16ID[nIndex] = u16ID[n];
					++nIndex;
				}
			}
			n64ID[nIndex] = 0;
			u16ID[nIndex] = 0;
			--nNumber;
		}
	}

	INT64 Get64ID(UINT16 u16)
	{
		INT nIndex = GetIndex(u16);
		if(nIndex != -1)
			return n64ID[nIndex];
		return 0;
	}

	INT GetIndex(UINT16 u16)
	{
		for(INT n = 0; n < nNumber; ++n)
			if(u16ID[n]==u16) return n;

		return -1;
	}

	INT GetCount() const { return nNumber; }

	BOOL IsFull() const { return nNumber >= MAXGDQUEST; }
};

class guerdon_quest_mgr : public EventMgr<guerdon_quest_mgr>
{
	typedef EventMgr<guerdon_quest_mgr> Super;
public:
	typedef package_map<INT64,guerdon_quest*> QUESTMAP;
	typedef package_map<DWORD,IDRefHelper*> IDREFMAP;
public:
	//--------------------------------------------------------------------
	// 主线程调用
	//--------------------------------------------------------------------
	BOOL init();
	VOID init(const VOID* pData, INT number);
	VOID init_item(const VOID* pData, INT number);
	VOID destroy();
	VOID update();
public:
	//=================================================
	//	 所有地图线程之上
	//=================================================
	VOID EvtPutQuest(DWORD dwRoleID, VOID* pMsg);
	VOID EvtGetQuest(DWORD dwRoleID, VOID* pMsg);
	VOID EvtGetOnePageGDQuest(DWORD dwRoleID, VOID* pMsg);
	VOID EvtUpdateMyPutGDQuest(DWORD dwRoleID, VOID* pMsg);
	VOID EvtUpdateMyGetGDQuest(DWORD dwRoleID, VOID* pMsg);
	VOID EvtCancelPutQuest(DWORD dwRoleID, VOID* pMsg);
	//VOID EvtGiveUpQuest(DWORD dwRoleID, VOID* pMsg);
protected:
	//--------------------------------------------------------------------
	// 需通过AddEvet来处理
	//--------------------------------------------------------------------
	VOID EvtCompGDQuest(DWORD dwRoleID, VOID* pMsg);
private:
	VOID RegisterEvent( );//注册所有事件
private:
	//--------------------------------------------------------------------
	// 内部辅助函数
	//--------------------------------------------------------------------
	VOID PutQuest(Role* pRole, VOID* pMsg);
	VOID GetQuest(Role* pRole, VOID* pMsg);
	VOID CompGDQuest(Role* pRole, VOID* pMsg);
	VOID ComplBeGDQuest(Role* pRole, VOID* pMsg);
	VOID QuestTimeout(guerdon_quest* p);
	VOID SendOnePageToRole(Role* pRole, BYTE byPage);
	VOID UpdateQuest2DB(guerdon_quest* p);
	VOID DeleteQuestDBAndMem(guerdon_quest* p);
	VOID UpdateMyPutGDQuest(Role* pRole);
	VOID UpdateMyGetGDQuest(Role* pRole);
	VOID CancelPutGDQuest(Role* pRole, VOID* pMsg);
	VOID QuestRewardDeal(guerdon_quest* p, ERwardOpType eOp, DWORD dwParam1 = 0);
	BOOL RewardCheck(const tagItem* pItem);
	ERewardMailOP GetMailTitleAndContent(const guerdon_quest* p, ERwardOpType eOp, BOOL dwGoldFlag = FALSE);
private:
	inline guerdon_quest* GetQuest(INT64 n64Serial);
	inline BOOL CanPuts(DWORD dwRole);
	inline BOOL CanGets(DWORD dwRole);
	inline BOOL AddPuts(DWORD dwRole, UINT16 u16, INT64 n64);
	inline BOOL AddGets(DWORD dwRole, UINT16 u16, INT64 n64);
private:
	IDREFMAP mPuts;
	IDREFMAP mGets;
	QUESTMAP mQuests;
};

inline guerdon_quest* guerdon_quest_mgr::GetQuest(INT64 n64Serial)
{
	return mQuests.find(n64Serial);
}

inline BOOL guerdon_quest_mgr::CanPuts(DWORD dwRole)
{
	IDRefHelper* pHelper = mPuts.find(dwRole);
	return VALID_POINT(pHelper) ? !pHelper->IsFull() : TRUE;
}

inline BOOL guerdon_quest_mgr::CanGets(DWORD dwRole)
{
	IDRefHelper* pHelper = mGets.find(dwRole);
	return VALID_POINT(pHelper) ? !pHelper->IsFull() : TRUE;
}

inline BOOL guerdon_quest_mgr::AddPuts(DWORD dwRole, UINT16 u16, INT64 n64)
{
	IDRefHelper* pHelper = mPuts.find(dwRole);
	if(!VALID_POINT(pHelper))
	{
		pHelper = new IDRefHelper;
		mPuts.add(dwRole, pHelper);
	}

	return pHelper->Add(u16, n64);
}

inline BOOL guerdon_quest_mgr::AddGets(DWORD dwRole, UINT16 u16, INT64 n64)
{
	IDRefHelper* pHelper = mGets.find(dwRole);
	if(!VALID_POINT(pHelper))
	{
		pHelper = new IDRefHelper;
		mGets.add(dwRole, pHelper);
	}

	return pHelper->Add(u16, n64);
}


extern guerdon_quest_mgr g_GuerdonQuestMgr;

#endif//__GUERDON_QUEST_MGR_H__