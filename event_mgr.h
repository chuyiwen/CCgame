
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//事件管理

#pragma once

#include "mutex.h"

//-------------------------------------------------------------------------------------------------------
// 事件类型
//-------------------------------------------------------------------------------------------------------
enum  EEventType
{
	EVT_NULL				=	-1,

	//---------------------------------------------------------
	// 好友相关
	//---------------------------------------------------------
	EVT_MakeFriend			=	0,
	EVT_CancelFriend		=	1,
	EVT_FriendGrp			=	2,
	EVT_InsertBlkList		=	3,
	EVT_DeleteBlkList		=	4,
	EVT_SendGift			=	5,
	EVT_SendGiftRpy			=	6,

	//---------------------------------------------------------
	// 队伍相关
	//---------------------------------------------------------
	EVT_MemJoinTeam			=   7,
	EVT_JoinTeam			=	8,
	EVT_JoinTeamRepley		=	9,
	EVT_KickMember			=	10,
	EVT_LeaveTeam			=	11,
	EVT_SetPickMol			=	12,
	EVT_ChangeLeader		=	13,
	EVT_ChangeLevel			=	14,
	EVT_ChangeRein			=	15,

	EVT_SynRoleLevel		=   16,

	EVT_AddAllRoleBuff		=	17,

	EVT_GuildWareMoveTo		=	18,	
	EVT_GuildWareMove2Other	=	19,

	//----------------------------------------------------------	
	// VIP摊位相关	
	//----------------------------------------------------------	
	EVT_SetStallTitle		=	20,		// 设置摊位标题	
	EVT_SetStallStatus		=	21,		// 设置摊位状态	
	EVT_ChangeStallGoods	=	22,		// 变更摊位商品	

	//---------------------------------------------------------
	// 队伍相关
	//---------------------------------------------------------
	EVT_ApplyJoin			=   23,		// 申请加入小队
	EVT_ApplyJoinReply		=   24,		// 申请加入小队反馈
	EVT_CleanApply			=   25,		// 清空申请列表
	EVT_OwnCreate			=   26,     // 自建队伍
	EVT_MemJoinTeamReply	=   27,		// 队员邀请回复
	EVT_Dismiss				=	28,		// 解散队伍
	EVT_ChangeTeamPlacard	=	29,		// 修改队伍公告
	EVT_GetMapTeamInfo		=	30,		// 取得地图内所有队伍信息

	//---------------------------------------------------------
	// 好友相关
	//---------------------------------------------------------
	EVT_DeleteEmList		=	31,		// 删除仇人
	EVT_GetEnemyPos			=   32,		// 获取仇人位置

	//---------------------------------------------------------
	// 组队分配
	//---------------------------------------------------------
	EVT_NoticeAssign		=	33,		//通知队长分配或者队员开始掷骰子
	EVT_LeaderAssign		=	34,		//队长分配物品
	EVT_TeamMemberSice		=	35,		//队员投票
	EVT_TeamSiceFinish		=	36,		//投票结束
	EVT_SetAssignQuality	=	37,		//设置物品分配等级
	
	//----------------------------------------------------------	
	// 双修相关 gx modify 2013.6.27
	//----------------------------------------------------------	
	EVT_ComPractice			=	38,//邀请玩家进行双修
	EVT_ComPracticeReply	=	39,//被邀请玩家双修反馈
	EVT_CancelPractice		=	40,//玩家主动取消双修
	//----------------------------------------------------------	
	// 悬赏相关
	//----------------------------------------------------------	
	//EVT_PutQuest			=	38,
	//EVT_GetQuest			=	39,
	//EVT_GiveUpQuest			=	40,
	EVT_CompGDQuest			=	41,

	//----------------------------------------------------------	
	// 组队环随机
	//----------------------------------------------------------	
	EVT_TeamShareCirlcleNext=	42,

	//----------------------------------------------------------
	// 组队环随机 //mwh 2011-09-06 
	//----------------------------------------------------------
	EVT_SetShareLeaderCircleQuest = 50,
	
	//----------------------------------------------------------	
	// 结婚相关 gx modify 2013.7.3
	//----------------------------------------------------------	
	EVT_Propose			=	51,//向女性玩家求婚
	EVT_ProposeReply	=	52,//女性玩家对求婚的回应
	EVT_Divorce			=	53,//玩家离婚
	EVT_QbjjReward		=	54,//领取情比金坚奖 gx add 2013.10.25

	//----------------------------------------------------------	
	// 脚本相关	
	//----------------------------------------------------------	
	EVT_Script_Reload		=	101,
	EVT_CreateTeam			=	102,
	EVT_AddMember			=	103,

	//---------------------------------------------------------
	// 好友相关
	//---------------------------------------------------------
	EVT_AddEmList		=	104,		// 添加仇人
	EVT_End,
};

//--------------------------------------------------------------------------------------------------------
// 游戏内事件
//--------------------------------------------------------------------------------------------------------
template<class T>
class EventMgr;

class EventObj
{
	template<class T> friend class EventMgr;

private:
	EventObj(DWORD dwSender, VOID* p_message, EEventType eType, DWORD dw_size)
	{
		// 初始化
		m_dwSender = dwSender;
		m_dwSize = dw_size;
		m_eType = eType;

		if( 0 == dw_size )
		{
			m_pMsg = NULL;
		}
		else
		{
			m_pMsg = new BYTE[dw_size];
			memcpy(m_pMsg, p_message, dw_size);
		}
	}

	~EventObj() { SAFE_DELETE_ARRAY(m_pMsg); }

	DWORD		GetSender() { return m_dwSender; }
	VOID*		GetEvent() { return m_pMsg; }
	DWORD		GetEventSize() { return m_dwSize; }
	EEventType	GetEventType() { return m_eType; }

private:
	DWORD				m_dwSender;
	VOID*				m_pMsg;
	EEventType			m_eType;
	DWORD				m_dwSize;
};

//-------------------------------------------------------------------------------------------------------
// 该类的成员函数只能在主线程调用（AddEvent除外）
//-------------------------------------------------------------------------------------------------------
template<class T>
class EventMgr
{
public:
	typedef VOID (T::*EVENTMESSAGEFUNC)(DWORD dwSender, VOID* p_message);
	typedef std::map<EEventType, EVENTMESSAGEFUNC>	EventFuncMap;

public:
	BOOL Init();
	~EventMgr();

	VOID Update();		
	VOID AddEvent(DWORD dwSender, EEventType eType, DWORD dw_size, VOID* p_message);

protected:
	static VOID RegisterEventFunc(EEventType EventType, EVENTMESSAGEFUNC EventMessageFunc);

private:
	package_list<EventObj*>				m_listEventObj;	
	Mutex							m_Lock;
	static EventFuncMap				m_mapEventFunc;	// 事件触发器类型及对应的处理函数
};

template<class T>
typename EventMgr<T>::EventFuncMap EventMgr<T>::m_mapEventFunc; 

//-------------------------------------------------------------------------------------------------------
// 注册事件处理函数
//-------------------------------------------------------------------------------------------------------
template<class T>
VOID EventMgr<T>::RegisterEventFunc(EEventType eEventType, EVENTMESSAGEFUNC EventMessageFunc)
{
	m_mapEventFunc.insert(make_pair(eEventType,	EventMessageFunc));
}

//-------------------------------------------------------------------------------------------------------
// 事件管理类初始化
//-------------------------------------------------------------------------------------------------------
template<class T>
BOOL EventMgr<T>::Init()
{
	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// 析构函数
//-------------------------------------------------------------------------------------------------------
template<class T>
EventMgr<T>::~EventMgr()
{
	EventObj* pEvent = m_listEventObj.pop_front();
	while( VALID_POINT(pEvent) )
	{
		SAFE_DELETE(pEvent);
		pEvent = m_listEventObj.pop_front();
	}
}


//-------------------------------------------------------------------------------------------------------
// 管理类事件处理
//-------------------------------------------------------------------------------------------------------
template<class T>
VOID EventMgr<T>::Update()
{
	EventObj* pOjb = m_listEventObj.pop_front();
	while( VALID_POINT(pOjb) )
	{
		EventFuncMap::iterator it = m_mapEventFunc.find(pOjb->GetEventType());
		if( it != m_mapEventFunc.end() )
		{
			EVENTMESSAGEFUNC handler = it->second;
			(((T*)this)->*handler)(pOjb->GetSender(), pOjb->GetEvent());
		}
		SAFE_DELETE(pOjb);

		pOjb = m_listEventObj.pop_front();
	}
}

//-------------------------------------------------------------------------------------------------------
// 加入事件
//-------------------------------------------------------------------------------------------------------
template<class T>
VOID EventMgr<T>::AddEvent(DWORD dwSender, EEventType eType, DWORD dw_size, VOID* p_message)
{
	ASSERT( eType > EVT_NULL && eType < EVT_End );

	EventObj* pObj = new EventObj(dwSender, p_message, eType, dw_size);

	m_Lock.Acquire();
	m_listEventObj.push_back(pObj);
	m_Lock.Release();
}
