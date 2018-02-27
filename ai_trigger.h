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
*	@file		ai_trigger.h
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		����AI������
*/

#pragma once

class Unit;
class Creature;
class AITriggerScript;
struct tagTriggerProto;
enum ECreatureTargetFriendEnemy;


class AITrigger
{
public:
	AITrigger();
	~AITrigger() {}

public:
	BOOL		Init(DWORD dwCreatureTypeID, DWORD dw_data_id, INT dwTargetType, DWORD dwSkillID=INVALID_VALUE);
	VOID		Refresh();
	VOID		SetTriggerActive(INT eEventType);
	VOID		OnEvent(INT eEventType, Creature* pOwner);
	VOID		Update(Creature* pOwner);

private:
	VOID		SetActive(BOOL bActive) { m_bActive = bActive; }
	BOOL		TestTimeIntervalTrigger();
	BOOL		TestEventTrigger(Creature* pOwner);
	BOOL		TestStateTrigger(Creature* pOwner, DWORD& dwTargetID);
	VOID		OnTrigger(Creature* pOwner, DWORD dwTargetID);

	BOOL						m_bActive;			// �������Ƿ񱻼���
	INT							m_nActiveTick;		// ����ʱ��
	DWORD						m_dwTriggeredTimes;	// �Ѵ����Ĵ���
	DWORD						m_dwSkillID;		// ����ID
	ECreatureTargetFriendEnemy	m_eTargeType;		// Ŀ������
	const tagTriggerProto*		m_pProto;
};

class AITriggerMgr
{
public:
	AITriggerMgr():m_pOwner(NULL), m_bPaused(FALSE) {}
	~AITriggerMgr();

public:
	BOOL		Init(Creature* pOwner, const tagCreatureAI* m_pAIProto);
	VOID		SetTriggerActive(INT eEventType);
	VOID		Refresh();
	VOID		Update();
	VOID		OnEvent(INT eEventType);

	VOID		Pause()			{ m_bPaused = TRUE; }
	VOID		UnPause()		{ m_bPaused = FALSE; }
	BOOL		IsPaused()		{ return m_bPaused; }

public:
	Creature*					m_pOwner;			 
	BOOL						m_bPaused;
	std::list<AITrigger*>		m_listTrigger;
};