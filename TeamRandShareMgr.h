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
 *	@file		TeamRandShareMgr
 *	@author		mwh
 *	@date		2012/05/23	initial
 *	@version	0.0.1.0
 *	@brief		<所谓的战神同盟系统>
*/
#ifndef __TEAM_RAND_SHARE_QUEST_H_
#define __TEAM_RAND_SHARE_QUEST_H_

class Team;
struct RandShare_t;


#include "event_mgr.h"
class TeamRandSahreQuestMgr : public EventMgr<TeamRandSahreQuestMgr>
{
	typedef EventMgr<TeamRandSahreQuestMgr> Super;
	DWORD mLogicThreadID;
	INT mNotifyActvieTick;
	package_map<DWORD, RandShare_t*> mAllShare;
public:
	static TeamRandSahreQuestMgr& GetInstance(){
		static TeamRandSahreQuestMgr __innerobj;
		return __innerobj;
	}

public:
	/** Init */
	BOOL Init( );
	VOID Destroy();
	VOID ForceAllDelDB( );

	BOOL AddNew(DWORD teamID);
	VOID DelOne(DWORD teamID);

	/** <主线程>update */
	VOID Update();

	VOID MemberNext(DWORD dwNPCID, DWORD roleID, BOOL sysauto = FALSE, BOOL Compelte = FALSE);
	VOID MemberExit(DWORD teamID, DWORD roleID);
	VOID MemberJoin(DWORD teamID, DWORD roleID);
public:
	/** AddEvent */
	VOID EvtMemberNext(DWORD dwRoleID, VOID* pMsg);
public:
	BOOL CanActive(DWORD teamID);
	BOOL CanRefresh(RandShare_t *pShare);
	VOID Refresh(DWORD teamID, RandShare_t *pShare);
private:
	VOID CheckThread( );
	VOID RandQuestList(const Team *pTeam, RandShare_t *pShare);
	VOID SendToOneDelQuest(DWORD dwRoleID, RandShare_t *pShare);
	VOID register_event( );//注册所有事件
private:
	TeamRandSahreQuestMgr(){}
	~TeamRandSahreQuestMgr(){Destroy();}
};

#define sTeamShareMgr TeamRandSahreQuestMgr::GetInstance()
#endif /** __TEAM_RAND_SHARE_QUEST_H_ */