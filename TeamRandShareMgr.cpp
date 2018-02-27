/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "StdAfx.h"
#include "static_array.h"
#include "TeamRandShareMgr.h"
#include "role.h"
#include "role_mgr.h"
#include "creature.h"
#include "map.h"
#include "quest_mgr.h"
#include "db_session.h"
#include "att_res.h"
#include "quest_mgr.h"

#include "../../common/WorldDefine/quest_protocol.h"
#include "../../common/WorldDefine/group_define.h"
#include "../../common/WorldDefine/TeamRandShareDefine.h"
#include "../../common/WorldDefine/TeamRandShareProtocol.h"
#include "../common/ServerDefine/TeamShareQuestServerDefine.h"
#include "../common/ServerDefine/quest_server_define.h"

struct RandShareUnit_t
{
	DWORD dwRoleID;
	INT16 nIndex;
	INT16 nCompleted;
	RandShareUnit_t():dwRoleID(0), nIndex(0),nCompleted(0){}
};

struct RandShareUnitPred
{
	DWORD dwRoleID;
public:
	RandShareUnitPred(DWORD id):dwRoleID(id){}
public:
	BOOL operator()(const RandShareUnit_t *pUnit)
	{
		return pUnit->dwRoleID == dwRoleID;
	}
};

struct RandShare_t
{
	DWORD dwTeamID;
	StaticArraySP<TEAMSHAREMAXQUST, UINT16> mRandList;
	StaticArraySP<MAX_TEAM_NUM, RandShareUnit_t> mMembers;
	RandShare_t(DWORD tid):dwTeamID(tid){}

public:
	RandShareUnit_t *GetShareUnit(DWORD dwRoleID)
	{
		INT idx = mMembers.Find(RandShareUnitPred(dwRoleID));

		return (idx == -1) ? (NULL) : mMembers.GetPtr(idx);
	}
};

BOOL TeamRandSahreQuestMgr::Init( )
{
	mLogicThreadID = 0; 
	mNotifyActvieTick = 0;
	Super::Init( );
	this->register_event( );
	return TRUE;
}

VOID TeamRandSahreQuestMgr::EvtMemberNext(DWORD dwRoleID, VOID* pMsg)
{
	MemberNext(INVALID_VALUE, dwRoleID, TRUE, TRUE);
}

VOID TeamRandSahreQuestMgr::register_event( )
{
	Super::RegisterEventFunc(EVT_TeamShareCirlcleNext, &TeamRandSahreQuestMgr::EvtMemberNext);
}
VOID TeamRandSahreQuestMgr::Destroy()
{
	RandShare_t *pShare;
	mAllShare.reset_iterator();
	while(mAllShare.find_next(pShare))
		SAFE_DELETE(pShare);
	mAllShare.clear();
}

VOID TeamRandSahreQuestMgr::Update()
{
	/** 主线程调用 */
	if(mLogicThreadID==0)
		mLogicThreadID = GetCurrentThreadId( );
	Super::Update( );

	if(++mNotifyActvieTick < (10*TICK_PER_SECOND))
		return;

	mNotifyActvieTick = 0;
	RandShare_t *pShare;
	mAllShare.reset_iterator();
	while(mAllShare.find_next(pShare))
	{
		if(CanRefresh(pShare))
		{
			Team *pTeam = g_groupMgr.GetTeamPtrEx(pShare->dwTeamID);
			if(VALID_POINT(pTeam) && pTeam->get_leader_share_circle_quest())
			{
				pTeam->leader_set_share_circle_quest(FALSE);
				Role *pRole = pTeam->get_member(0);
				if(VALID_POINT(pRole)) 
				{
					NET_SIS_Get_Team_Share_Quest send;
					send.bFlag = FALSE;
					send.serial = 0;
					send.u16QuestID = 0;
					send.dwError = ETSQ_Leader_NeedReactive;
					pRole->SendMessage(&send, send.dw_size);
				}
			}
		}
	}

}

VOID TeamRandSahreQuestMgr::CheckThread( )
{
	if(mLogicThreadID && mLogicThreadID != GetCurrentThreadId()){
		print_message(_T("TeamRandSahreQuestMgr MultiThread Fatal Error"));
		INT *p = NULL; *p = 0; /** 让程序生成dump */
	}
}


BOOL TeamRandSahreQuestMgr::CanActive(DWORD teamID)
{
	CheckThread( );
	RandShare_t *pNew = mAllShare.find(teamID);
	if(!VALID_POINT(pNew)) return TRUE;
	
	INT i;
	INT nMebmers = pNew->mMembers.GetSize();
	for(i = 0; i < nMebmers; i++){
		RandShareUnit_t *pUnit = pNew->mMembers.GetPtr(i);
		if(NULL == pUnit || pUnit->nIndex < pNew->mRandList.GetSize())
			return FALSE;
	}

	return TRUE;
}

BOOL TeamRandSahreQuestMgr::CanRefresh(RandShare_t *pShare)
{
	CheckThread( );

	if(!VALID_POINT(pShare))
		return FALSE;

	INT i;
	INT nMebmers = pShare->mMembers.GetSize();
	for(i = 0; i < nMebmers; i++){
		RandShareUnit_t *pUnit = pShare->mMembers.GetPtr(i);
		if(NULL == pUnit || pUnit->nIndex < pShare->mRandList.GetSize())
			return FALSE;
	}
	
	return TRUE;
}

VOID TeamRandSahreQuestMgr::Refresh(DWORD teamID, RandShare_t *pShare)
{
	CheckThread( );
	pShare->mMembers.Clear();

	const Team *pTeam = g_groupMgr.GetTeamPtr(teamID);
	if(!VALID_POINT(pTeam))
		return;


	/** fill member list */
	INT i, cnt = 0;
	INT nTeamMember = pTeam->get_member_number();
	pShare->mMembers.Resize(nTeamMember);
	for(i = 0; i < nTeamMember; i++)
	{
		Role* pRole = pTeam->get_member(i);
		if(VALID_POINT(pRole)){
			pShare->mMembers.GetRef(cnt++).dwRoleID = pRole->GetID();
		}
	}
	pShare->mMembers.Resize(cnt);

	RandQuestList(pTeam, pShare);
}

VOID TeamRandSahreQuestMgr::ForceAllDelDB( )
{
	/** 通知数据库删除所有任务 */
	NET_S2DB_DelAllTeamShareQuest send;
	send.nNewFlag = 1;
	g_dbSession.Send(&send, send.dw_size);
}

BOOL TeamRandSahreQuestMgr::AddNew(DWORD teamID)
{
	CheckThread( );
	RandShare_t *pNew = mAllShare.find(teamID);
	if(VALID_POINT(pNew)){
		if(!CanRefresh(pNew)) return TRUE;
	}else{

		pNew = new RandShare_t(teamID);
		if(!VALID_POINT(pNew)) return FALSE;
		mAllShare.add(teamID, pNew);
	}

	this->Refresh(teamID, pNew);

	return TRUE;
}

VOID TeamRandSahreQuestMgr::DelOne(DWORD teamID)
{
	CheckThread( );
	RandShare_t *pShare = mAllShare.find(teamID);
	if(!VALID_POINT(pShare)) return;

	INT i;
	INT nMebmers = pShare->mMembers.GetSize();
	for(i = 0; i < nMebmers; i++){
		RandShareUnit_t *pUnit = pShare->mMembers.GetPtr(i);
		if(NULL == pUnit){ASSERT(FALSE);continue;}
		SendToOneDelQuest(pUnit->dwRoleID, pShare);
	}

	mAllShare.erase(teamID);
	SAFE_DELETE(pShare);
}

VOID TeamRandSahreQuestMgr::MemberNext(DWORD dwNPCID, DWORD roleID, BOOL sysauto, BOOL Compelte)
{
	CheckThread( );
	Role *pRole = g_roleMgr.get_role(roleID);
	if(!VALID_POINT(pRole)) return;
	Map *pMap = pRole->get_map( );
	if(!VALID_POINT(pMap)) return ;
	
	NET_SIS_Get_Team_Share_Quest send;

	if( !sysauto ){
		Creature* p_npc = pMap->find_creature(dwNPCID);
		if( !VALID_POINT(p_npc) ) return  ;
		else if( !p_npc->IsFunctionNPC(EFNPCT_TeamCircleQst)) return;
		else if( !p_npc->IsInDistance(*pRole, 500.0f) ) return  ;
	}

	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	if(!VALID_POINT(pTeam))
	{
		send.bFlag = FALSE;
		send.dwError = ETSQ_Not_JoinTeam;
		goto end_next;
	}

	send.bFlag = pTeam->get_leader_share_circle_quest();
	RandShare_t *pShare = mAllShare.find(pTeam->get_team_id());
	if(!VALID_POINT(pShare))
	{
		send.dwError = ETSQ_Leader_NotActive;
		goto end_next;
	}

	RandShareUnit_t *pUnit = pShare->GetShareUnit(roleID);
	if(!VALID_POINT(pUnit))
	{
		send.dwError = ETSQ_Not_JoinTeam;
		goto end_next;
	}

	if(Compelte)
		pUnit->nCompleted++;

	if(pUnit->nIndex >= pShare->mRandList.GetSize() && 
		pUnit->nCompleted >= pShare->mRandList.GetSize()){
		send.dwError = !CanRefresh(pShare)? ETSQ_No_More_Quset
			: ETSQ_Leader_NeedReactive;
	}else{

		BOOL bNeedGetNext = TRUE;
		if(pUnit->nIndex > 0){
			UINT16 uPrevQuestID = pShare->mRandList.Get(pUnit->nIndex - 1);
			quest* pPrevQuest = pRole->get_quest(uPrevQuestID);
			if(VALID_POINT(pPrevQuest) || pUnit->nCompleted < pUnit->nIndex){
				send.u16QuestID = uPrevQuestID;
				send.serial = pUnit->nIndex - 1;
				bNeedGetNext = FALSE;
			}
		}

		if(bNeedGetNext){
			send.serial = pUnit->nIndex;
			send.u16QuestID = pShare->mRandList.Get(pUnit->nIndex);
			pUnit->nIndex++;
		}

		send.dwError = E_Success;
	}

	/** 将ID发给客户端 */
end_next:	
	pRole->SendMessage(&send, send.dw_size);
}

VOID TeamRandSahreQuestMgr::MemberExit(DWORD teamID, DWORD roleID)
{
	CheckThread( );
	RandShare_t *pShare = mAllShare.find(teamID);
	if(!VALID_POINT(pShare)) return;

	SendToOneDelQuest(roleID, pShare);
	pShare->mMembers.FastRemove(RandShareUnitPred(roleID));
}

VOID TeamRandSahreQuestMgr::MemberJoin(DWORD teamID, DWORD roleID)
{
	CheckThread( );
	RandShare_t *pShare = mAllShare.find(teamID);
	if(!VALID_POINT(pShare)) return;

	RandShareUnit_t *pUnit = pShare->GetShareUnit(roleID);
	if(!VALID_POINT(pUnit)){
		RandShareUnit_t unit; 
		unit.dwRoleID = roleID;
		pShare->mMembers.Add(unit);
	}
}


VOID TeamRandSahreQuestMgr::RandQuestList(const Team *pTeam, RandShare_t *pShare)
{
	CheckThread( );
	pShare->mRandList.Clear( );
	/** 随机相应的任务 */
	const std::vector<tagTeamShareCirle*>* pTeamShareCircle = \
		g_questMgr.get_team_share_circle_quest(pTeam->get_average_level());

	if(!VALID_POINT(pTeamShareCircle))
		return;


	size_t nCircleNumber = pTeamShareCircle->size();
	for(size_t n = 0; n < nCircleNumber; n++)
	{
		const tagTeamShareCirle* pCircle = (*pTeamShareCircle)[n];
		INT nRand = 0;
		for(size_t n = 0; 
			nRand < pCircle->rand_number && n < pCircle->ids.size();
			n++)
		{
			int idx = rand( ) % pCircle->ids.size();
			const UINT16 u16QuestID = pCircle->ids[idx];
			if(!pShare->mRandList.IsExist(u16QuestID))
			{	
				pShare->mRandList.Add(u16QuestID); ++nRand;
			}
		}
	}
}

VOID TeamRandSahreQuestMgr::SendToOneDelQuest(DWORD dwRoleID, RandShare_t *pShare)
{
	CheckThread( );
	RandShareUnit_t *pUnit = pShare->GetShareUnit(dwRoleID);
	if(pUnit == NULL){
		ASSERT(FALSE);
		return;
	}

	if(pUnit->nIndex < 1)
		return ;

	UINT16 curQuestID = pShare->mRandList.Get(pUnit->nIndex - 1);
	Role *pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole) || pRole->is_leave_pricitice())
	{
		/** 不在线， 通知数据库删除对应任务 */
		NET_DB2C_discard_quest send;
		send.dw_role_id = dwRoleID;
		send.u16_quest_id_ = curQuestID;
		g_dbSession.Send(&send, send.dw_size);
	}else{
		/** 在线 */
		NET_SIS_delete_quest send2;
		send2.u16QuestID = curQuestID;
		send2.dw_error_code = E_Success;
		pRole->SendMessage(&send2, send2.dw_size);

		pRole->remove_quest(curQuestID, FALSE);
	}
}