
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�¼�����

#pragma once

#include "mutex.h"

//-------------------------------------------------------------------------------------------------------
// �¼�����
//-------------------------------------------------------------------------------------------------------
enum  EEventType
{
	EVT_NULL				=	-1,

	//---------------------------------------------------------
	// �������
	//---------------------------------------------------------
	EVT_MakeFriend			=	0,
	EVT_CancelFriend		=	1,
	EVT_FriendGrp			=	2,
	EVT_InsertBlkList		=	3,
	EVT_DeleteBlkList		=	4,
	EVT_SendGift			=	5,
	EVT_SendGiftRpy			=	6,

	//---------------------------------------------------------
	// �������
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
	// VIP̯λ���	
	//----------------------------------------------------------	
	EVT_SetStallTitle		=	20,		// ����̯λ����	
	EVT_SetStallStatus		=	21,		// ����̯λ״̬	
	EVT_ChangeStallGoods	=	22,		// ���̯λ��Ʒ	

	//---------------------------------------------------------
	// �������
	//---------------------------------------------------------
	EVT_ApplyJoin			=   23,		// �������С��
	EVT_ApplyJoinReply		=   24,		// �������С�ӷ���
	EVT_CleanApply			=   25,		// ��������б�
	EVT_OwnCreate			=   26,     // �Խ�����
	EVT_MemJoinTeamReply	=   27,		// ��Ա����ظ�
	EVT_Dismiss				=	28,		// ��ɢ����
	EVT_ChangeTeamPlacard	=	29,		// �޸Ķ��鹫��
	EVT_GetMapTeamInfo		=	30,		// ȡ�õ�ͼ�����ж�����Ϣ

	//---------------------------------------------------------
	// �������
	//---------------------------------------------------------
	EVT_DeleteEmList		=	31,		// ɾ������
	EVT_GetEnemyPos			=   32,		// ��ȡ����λ��

	//---------------------------------------------------------
	// ��ӷ���
	//---------------------------------------------------------
	EVT_NoticeAssign		=	33,		//֪ͨ�ӳ�������߶�Ա��ʼ������
	EVT_LeaderAssign		=	34,		//�ӳ�������Ʒ
	EVT_TeamMemberSice		=	35,		//��ԱͶƱ
	EVT_TeamSiceFinish		=	36,		//ͶƱ����
	EVT_SetAssignQuality	=	37,		//������Ʒ����ȼ�
	
	//----------------------------------------------------------	
	// ˫����� gx modify 2013.6.27
	//----------------------------------------------------------	
	EVT_ComPractice			=	38,//������ҽ���˫��
	EVT_ComPracticeReply	=	39,//���������˫�޷���
	EVT_CancelPractice		=	40,//�������ȡ��˫��
	//----------------------------------------------------------	
	// �������
	//----------------------------------------------------------	
	//EVT_PutQuest			=	38,
	//EVT_GetQuest			=	39,
	//EVT_GiveUpQuest			=	40,
	EVT_CompGDQuest			=	41,

	//----------------------------------------------------------	
	// ��ӻ����
	//----------------------------------------------------------	
	EVT_TeamShareCirlcleNext=	42,

	//----------------------------------------------------------
	// ��ӻ���� //mwh 2011-09-06 
	//----------------------------------------------------------
	EVT_SetShareLeaderCircleQuest = 50,
	
	//----------------------------------------------------------	
	// ������ gx modify 2013.7.3
	//----------------------------------------------------------	
	EVT_Propose			=	51,//��Ů��������
	EVT_ProposeReply	=	52,//Ů����Ҷ����Ļ�Ӧ
	EVT_Divorce			=	53,//������
	EVT_QbjjReward		=	54,//��ȡ��Ƚ�ά gx add 2013.10.25

	//----------------------------------------------------------	
	// �ű����	
	//----------------------------------------------------------	
	EVT_Script_Reload		=	101,
	EVT_CreateTeam			=	102,
	EVT_AddMember			=	103,

	//---------------------------------------------------------
	// �������
	//---------------------------------------------------------
	EVT_AddEmList		=	104,		// ��ӳ���
	EVT_End,
};

//--------------------------------------------------------------------------------------------------------
// ��Ϸ���¼�
//--------------------------------------------------------------------------------------------------------
template<class T>
class EventMgr;

class EventObj
{
	template<class T> friend class EventMgr;

private:
	EventObj(DWORD dwSender, VOID* p_message, EEventType eType, DWORD dw_size)
	{
		// ��ʼ��
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
// ����ĳ�Ա����ֻ�������̵߳��ã�AddEvent���⣩
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
	static EventFuncMap				m_mapEventFunc;	// �¼����������ͼ���Ӧ�Ĵ�����
};

template<class T>
typename EventMgr<T>::EventFuncMap EventMgr<T>::m_mapEventFunc; 

//-------------------------------------------------------------------------------------------------------
// ע���¼�������
//-------------------------------------------------------------------------------------------------------
template<class T>
VOID EventMgr<T>::RegisterEventFunc(EEventType eEventType, EVENTMESSAGEFUNC EventMessageFunc)
{
	m_mapEventFunc.insert(make_pair(eEventType,	EventMessageFunc));
}

//-------------------------------------------------------------------------------------------------------
// �¼��������ʼ��
//-------------------------------------------------------------------------------------------------------
template<class T>
BOOL EventMgr<T>::Init()
{
	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// ��������
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
// �������¼�����
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
// �����¼�
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
