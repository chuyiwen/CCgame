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
*	@file		creature.h
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		��Ϸ������AI�࣬һ������״̬������������AI״̬
*/

#pragma once

#include "ai_transition.h"
#include "ai_trigger.h"
#include "script_mgr.h"
#include "creature.h"
#include "creature_order.h"
#include "pet_tracker.h"

class Unit;
class Creature;
class AIState;
class AITransition;
class AITriggerMgr;
class CVector2D;
struct tagCreatureAI;
struct tagTriggerProto;
enum  EFLEETIME;

const FLOAT CREATURE_RETURN_HEIGH = 100.0;

//-----------------------------------------------------------------------------
// AI״̬����
//-----------------------------------------------------------------------------
enum AIStateType
{
	AIST_Idle			= 0,			// ����״̬
	AIST_Pursue			= 1,			// ׷��״̬
	AIST_Attack			= 2,			// ����״̬
	AIST_Flee			= 3,			// ����״̬
	AIST_SOS			= 4,			// ����״̬
	AIST_Return			= 5,			// ����״̬
	AIST_Follow			= 6,			// ����״̬
	AIST_SpaceOut		= 7,			// ��������
	//AIST_								// ս������״̬
	AIST_PetIdle		= 8,			// ��������״̬
	AIST_PetAttack		= 9,			// ����ս��״̬
	AIST_Pet_Pursue		= 10,			// ����׷��״̬
	AIST_Alert			= 11,			// ����״̬
	AISI_Script			= 12,			// �ű�״̬
	AIST_Talk			= 13,			// �Ի�״̬
};

//-----------------------------------------------------------------------------
// AI״̬������
//-----------------------------------------------------------------------------
class AIState
{
public:
	virtual ~AIState() {}

	// ����״̬����
	virtual VOID OnEnter(AIController* pAI) = 0;
	// �뿪״̬����
	virtual VOID OnExit(AIController* pAI) = 0;
	// ���º���
	virtual VOID Update(AIController* pAI) = 0;
	// �¼���Ӧ
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1=0, DWORD dwEventMisc2=0) = 0;
};

//-----------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------
class AIStateIdle : public AIState
{
public:
	virtual ~AIStateIdle() {}

public:
	AIStateIdle() {}
	static AIStateIdle* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);	// �����᲻��̫����

private:
	AIStateIdle(const AIStateIdle&);
	AIStateIdle& operator=(const AIStateIdle&);
};

//-----------------------------------------------------------------------------
// ׷��״̬
//-----------------------------------------------------------------------------
class AIStatePursue: public AIState
{
public:
	AIStatePursue() {}
	virtual ~AIStatePursue() {}

public:
	static AIStatePursue* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStatePursue(const AIStatePursue&);
	AIStatePursue& operator=(const AIStatePursue&);
};

//-----------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------
class AIStateAttack: public AIState
{
public:
	AIStateAttack() {}
	virtual ~AIStateAttack() {}

public:
	static AIStateAttack* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateAttack(const AIStateAttack&);
	AIStateAttack& operator=(const AIStateAttack&);
};

//-----------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------
class AIStateFlee: public AIState
{
public:
	AIStateFlee() {}
	virtual ~AIStateFlee() {}

public:
	static AIStateFlee* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateFlee(const AIStateFlee&);
	AIStateFlee& operator=(const AIStateFlee&);
};

//-----------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------
class AIStateCallHelp : public AIState
{
public:
	AIStateCallHelp() {}
	virtual ~AIStateCallHelp() {}

public:
	static AIStateCallHelp* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateCallHelp(const AIStateCallHelp&);
	AIStateCallHelp& operator=(const AIStateCallHelp&);
};

//-----------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------
class AIStateReturn : public AIState
{
public:
	AIStateReturn() {}
	virtual ~AIStateReturn() {}

public:
	static AIStateReturn* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateReturn(const AIStateReturn&);
	AIStateReturn& operator=(const AIStateReturn&);
};

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
class AIStateSpaceOut : public AIState
{
public:
	AIStateSpaceOut() {}
	virtual ~AIStateSpaceOut() {}

public:
	static AIStateSpaceOut* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc/* =0 */, DWORD dwEventMisc2/* = 0 */);

private:
	AIStateSpaceOut(const AIStateSpaceOut&);
	AIStateSpaceOut& operator=(const AIStateSpaceOut&);
};

//-----------------------------------------------------------------------------
// ����״̬��һ����Ϊȫ��״̬
//-----------------------------------------------------------------------------
class AIStateFollow: public AIState
{
public:
	AIStateFollow() {}
	virtual ~AIStateFollow() {}

public:
	static AIStateFollow* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateFollow(const AIStateFollow&);
	AIStateFollow& operator=(const AIStateFollow&);
};

//-----------------------------------------------------------------------------
// �ű�״̬���������нű�
//-----------------------------------------------------------------------------
class AIStateScript: public AIState
{
public:
	AIStateScript() {}
	virtual ~AIStateScript() {}

public:
	static AIStateScript* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateScript(const AIStateScript&);
	AIStateScript& operator=(const AIStateScript&);
};

//-----------------------------------------------------------------------------
// �Ի�״̬
//-----------------------------------------------------------------------------
class AIStateTalk: public AIState
{
public:
	AIStateTalk() {}
	virtual ~AIStateTalk() {}

public:
	static AIStateTalk* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStateTalk(const AIStateTalk&);
	AIStateTalk& operator=(const AIStateTalk&);
};



//-----------------------------------------------------------------------------
// ��������״̬
// 
//-----------------------------------------------------------------------------
class AIStatePetIdle : public AIState
{
public:
	AIStatePetIdle() {}
	virtual ~AIStatePetIdle() {}

public:
	static AIStatePetIdle* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStatePetIdle(const AIStatePetIdle&);
	AIStatePetIdle& operator=(const AIStatePetIdle&);
};

//-----------------------------------------------------------------------------
// ����ս��״̬
// 
//-----------------------------------------------------------------------------
class AIStatePetAttack: public AIState
{
public:
	AIStatePetAttack() {}
	virtual ~AIStatePetAttack() {}

public:
	static AIStatePetAttack* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStatePetAttack(const AIStatePetAttack&);
	AIStatePetAttack& operator=(const AIStatePetAttack&);
};


//-----------------------------------------------------------------------------
// �����׷��״̬
// 
//-----------------------------------------------------------------------------
class AIStatePetPursue : public AIState
{

public:
	AIStatePetPursue() {}
	virtual ~AIStatePetPursue() {}

public:
	static AIStatePetPursue* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:
	AIStatePetPursue(const AIStatePetPursue&);
	AIStatePetPursue& operator=(const AIStatePetPursue&);

};


//-----------------------------------------------------------------------------
// ���ﾯ��״̬
// 
//-----------------------------------------------------------------------------
class AIStateAlert : public AIState
{

public:
	AIStateAlert() {}
	virtual ~AIStateAlert() {}

public:

	static AIStateAlert* Instance();

	virtual VOID OnEnter(AIController* pAI);
	virtual VOID OnExit(AIController* pAI);
	virtual VOID Update(AIController* pAI);
	virtual BOOL OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */);

private:

	AIStateAlert( const AIStateAlert& );
	AIStateAlert& operator=( const AIStateAlert& );

};


//-----------------------------------------------------------------------------
// AI��������Ϊһ����״̬��
//-----------------------------------------------------------------------------
class AIController
{
public:
	
	explicit AIController(Creature* pCreature, const tag_map_way_point_info_list* patrolList);
	~AIController() { Destroy(); }

	//--------------------------------------------------------------------------
	// ��ʼ���͸���
	//--------------------------------------------------------------------------
	VOID			InitPatrol(const tag_map_way_point_info_list* patrolList);
	VOID			InitAIProto();
	VOID			InitTriggerMgr();
	VOID			InitAITransition();
	VOID			InitPathFinder();

	VOID			Update();
	VOID			Destroy();
	VOID			Refresh();

	//--------------------------------------------------------------------------
	// �¼�����
	//--------------------------------------------------------------------------
	VOID			OnEvent(Unit* pSrc, INT nEventType, DWORD dwEventMisc1=0, DWORD dwEventMisc2=0);

	//--------------------------------------------------------------------------
	// �õ�����
	//--------------------------------------------------------------------------
	Creature*		GetOwner()								{ return m_pOwner; }

	//--------------------------------------------------------------------------
	// ״̬���
	//--------------------------------------------------------------------------
	AIStateType		GetCurrentStateType()					{ return m_eCurAIState; }
	AIState*		GetCurrentState()						{ return m_pCurrentState; }
	AIState*		GetPreviousState()						{ return m_pPreState; }
	AIState*		GetGlobalState()						{ return m_pGlobalState; }

	VOID			SetCurrentStateType(AIStateType eType)	{ m_eCurAIState = eType; }
	VOID			SetCurrentState(AIState* pState)		{ m_pCurrentState = pState; }
	VOID			SetPreviousState(AIState* pState)		{ m_pPreState = pState; }
	VOID			SetGlobalState(AIState* pState)			{ m_pGlobalState = pState; }

	VOID			ChangeState(AIStateType eState);

	//---------------------------------------------------------------------------
	// ս���߼����
	//---------------------------------------------------------------------------

	// ͨ��
	DWORD			GetEnterCombatTick() const			{ return m_dwEnterCombatTick; }
	const Vector3&	GetEnterCombatPos() const			{ return m_vPosEnterCombat; }
	VOID			SetEnterCombatPos(Vector3& vPos)	{ m_vPosEnterCombat = vPos; }
	VOID			SetEnterCombatTick(DWORD dwTick)	{ m_dwEnterCombatTick = dwTick; }
	BOOL			IsTargetValid(DWORD dwID);
	DWORD			GetTargetIDByType(ECreatureTargetFriendEnemy eTargetType);

	// Ѳ��
	BOOL			IsReversePatrol() const				{ return m_bReversePatrol; }
	BOOL			IsPatroling() const					{ return m_bIsPatroling; }
	VOID			SetIsPatroling(BOOL bPatrol)		{ m_bIsPatroling = bPatrol; }
	VOID			SetIsReversePatrol(BOOL bReverse)	{ m_bReversePatrol = bReverse; }
	VOID			ReSetPatrolRestTick()				{ m_nPatrolRestTick = get_tool()->tool_rand() % CREATURE_PATROL_REST_TICK_INTERVAL + CREATURE_PATROL_REST_TICK_MIN; }
	VOID			ReSetPatrolRestTick( DWORD dw_time ) { m_nPatrolRestTick = dw_time; }
	VOID			UpdatePatrol();
	VOID			StartPatrol();
	VOID			PausePatrol(const Vector3& vFace, DWORD dw_time);
	BOOL			MoveWayPoint(INT nIndex, EMoveState nMoveType);
	// ����
	VOID			ReSetLookForTargetTick()			{ m_nLookForTargetTick = get_tool()->tool_rand() % CREATURE_LOOK_FOR_ENEMY_REST_TICK_INTERVAL + CREATURE_LOOK_FOR_ENEMY_REST_TICK_MIN; }
	DWORD			UpdateLookForEnemy();
	DWORD			StartLookForEnemy();
	BOOL			UpdateLockTarget();

	// ��������
	DWORD			UpdateLookForEnemyPet();
	DWORD			StartLookForEnemyPet();

	// ���ﾯ��״̬
	DWORD			UpdateLookForEnemyAlert();
	DWORD			StartLookForEnemyAlert();

	// ��������
	VOID			ResetCallHelpTime()					{ m_dwCallHelpTime = CREATURE_CALLHELP_TICK; }
	VOID			UpdateCallHelp();
	VOID			CallHelp();

	// ����NPC�������߼�
	void			StartLookForEnemyHuCheng();

	// ׷��
	const Vector3&	GetPursueTargetPos() const			{ return m_vPursueTargetPos; }
	DWORD			GetTargetUnitID() const				{ return m_dwTargeUnitID; }
	Unit*			GetPursueTarget();
	BOOL			GetPosNearPursueTarget(Unit* pTarget, Vector3& vPos);
	VOID			StartPursue(Unit* pTarget, BOOL bfanwei = true);
	VOID			UpdatePursue(Unit* pTarget);
	VOID			SetPursueTargetPos(Vector3& vPos)	{ m_vPursueTargetPos = vPos; }
	VOID			SetTargetUnitID(DWORD dwID)			{ m_dwTargeUnitID = dwID; m_pOwner->SetTargetID(dwID);}
	VOID			ReSetLockTargetTime()				{ m_nLockTargetTick = get_tool()->tool_rand() % CREATURE_LOCK_TARGET_TICK_INTERVAL + CREATURE_LOCK_TARGET_MIN_TICK; }

	// ����׷��
	VOID			StartPursuePet(Unit* pTarget);
	VOID			UpdatePursuePet(Unit* pTarget);

	// ����
	BOOL			IsInAttackDist(DWORD dwTargetUnitID);
	BOOL			IsInAlertDist( DWORD dwTargetID );
	BOOL			IsInspiration( DWORD dwTargetUnitID );
	INT				GetNextAttackWaitTick()	const		{ return m_nNextAttackWaitTick; }
	VOID			SetNextAttackWaitTick(INT nTick)	{ m_nNextAttackWaitTick = nTick; }
	VOID			CountDownNextAttackWaitTick()		{ --m_nNextAttackWaitTick; }
	INT				GetCheckAIEvnetTick() const			{ return m_nCheckAIEventTick; }
	VOID			SetCheckAIEventTick(INT nTick)		{ m_nCheckAIEventTick = nTick; }
	INT				AIUseSkill(DWORD dwSkillID, DWORD dwTargetUnitID);
	DWORD			GetAIUseSkillID()					{ return m_dwUseSkillID; }

	// ����
	const Vector3&	GetFleePos() const					{ return m_vFleePos; }
	VOID			SetFleePos(Vector3 vPos)			{ m_vFleePos = vPos; }
	BOOL			IsArrivedFleePos()					{ return m_bArrivedFleePos; }
	VOID			SetIsArrivedFleePos(BOOL bArrived)	{ m_bArrivedFleePos = bArrived; }
	VOID			SetFleeTime(EFLEETIME eFleeTime)	{ m_eFleeTime = eFleeTime; }
	EFLEETIME		GetFleeTime()						{ return m_eFleeTime; }
	VOID			CountDownNextFleeTick()				{ --m_dwFleeTick; }
	DWORD			GetFleeTick()						{ return m_dwFleeTick; }
	VOID			SetFleeTick(DWORD dwFleeTick)		{ m_dwFleeTick = dwFleeTick; }                                                                     
	VOID			CalFleePos();
	VOID			StartFlee(Vector3 vFleePos);
	VOID			UpdateFlee();

	// ����
	BOOL			CalHelpPos();
	DWORD			GetHelperID()						{ return m_dwHelperID; }
	VOID			SetHelperID(DWORD dwHelperID)		{ m_dwHelperID = dwHelperID; }
	package_list<DWORD>&	GetHelpList()						{ return m_listHelpID; }
	BOOL			NeedReCalHelpPos();

	// ���־���
	BOOL			CalSpaceOutPos();
	VOID			StartSpaceOut();
	VOID			UpdateSpaceOut();
	DWORD			GetSpaceOutTick()					{ return m_dwSpaceOutTick; }
	BOOL			IsArrivedPos()						{ return m_bArrivedPos; }
	VOID			SetIfArrivedPos(BOOL bArrived)		{ m_bArrivedPos = bArrived; }

	// ����
	VOID			StartReturn();
	VOID			UpdateReturn();
	BOOL			IsArrivedReturnPos()				{ return m_bArrivedReturnPos; }

	//---------------------------------------------------------------------------
	// AIת�������
	//---------------------------------------------------------------------------
	AITransition*	GetTransition()						{ return m_pTransition; }

	//----------------------------------------------------------------------------
	// ���������������
	//----------------------------------------------------------------------------
	AITriggerMgr*	GetAITriggerMgr()					{ return m_pAITrigger; }

	//----------------------------------------------------------------------------
	// ������
	//----------------------------------------------------------------------------
	DWORD			RandRoleInEnmityList();
	DWORD			GetMinEnmityInEnmityList();
	DWORD			RandRoleInEnemityListNotFirst();
	VOID			AddEnmity(Unit *pUnit, INT nValue, BOOL bSyncTeam=TRUE);
	VOID			AddEnmityMod(Unit *pUnit, INT nValue, BOOL bSyncTeam=TRUE);
	VOID			ClearAllEnmity();
	VOID			ClearEnmity(DWORD dw_role_id);
	VOID			GetEnmityList(std::vector<tagEnmity*>& vecEnmityList);
	VOID			SetEnmityActive(DWORD dw_role_id, BOOL bActive);
	BOOL			IsEnmityListEmpty();
	VOID			CalMaxEnmity();
	DWORD			GetCurrentVictim()				{ return m_dwCurrentVictim; }
	INT				GetEnmityValue(DWORD dwID);
	INT				GetBaseEnmityValue(DWORD dwID);
	VOID			ClearEnmityModValue(DWORD dwID);
	VOID			SetCurrentVictim(DWORD dwID)	{ m_dwCurrentVictim = dwID; }
	BOOL			IsEnmityListAllInvincible();

	//----------------------------------------------------------------------------
	// �ű����
	//----------------------------------------------------------------------------
	VOID			SetScriptUpdateTimer(INT nTimer)	{ m_nScriptUpdateTimer = nTimer; }

	//----------------------------------------------------------------------------
	// �Ի����
	//----------------------------------------------------------------------------
	VOID SetTalkTime(DWORD dw_time) { m_dwTalkTime = dw_time; }

	DWORD& GetTalkTime() { return m_dwTalkTime; }
	
	//----------------------------------------------------------------------------
	// ��ø�����
	//----------------------------------------------------------------------------
	PetTracker*		GetTracker() { return m_pTracker; }
	
	const tagCreatureAI*	GetAIProto() { return m_pAIProto; }

protected:
	//----------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------
	VOID			UpdateAIController();
	VOID			UpdateTransition();
	VOID			UpdateCurrentState();
	VOID			UpdateTriggerMgr();

	//----------------------------------------------------------------------------
	// ״̬���
	//----------------------------------------------------------------------------
	AIState*		GetStateByType(AIStateType eStateType);

	//----------------------------------------------------------------------------
	// ״̬ת�������
	//----------------------------------------------------------------------------
	AITransition*	GetTransitionByType();

	//----------------------------------------------------------------------------
	// ��������
	//----------------------------------------------------------------------------
	BOOL			IsInStateCantUpdateAI();
	
protected:
	Creature*				m_pOwner;					// ��Ӧ������

	//----------------------------------------------------------------------------
	// ״̬
	//----------------------------------------------------------------------------
	AIStateType				m_eCurAIState;				// ��ǰAI״̬ö��
	AIState*				m_pCurrentState;			// ��ǰAI״̬
	AIState*				m_pPreState;				// ��һ�ε�״̬
	AIState*				m_pGlobalState;				// ȫ��AI״̬

	//----------------------------------------------------------------------------
	// ״̬ת����
	//----------------------------------------------------------------------------
	AITransition*			m_pTransition;				// AI״̬ת����

	//----------------------------------------------------------------------------
	// ������������
	//----------------------------------------------------------------------------
	AITriggerMgr*			m_pAITrigger;				// AI������������

	//----------------------------------------------------------------------------
	// ����
	//----------------------------------------------------------------------------
	const tagCreatureProto*	m_pProto;					// �������Ӧ�ľ�̬����
	const tagCreatureAI*	m_pAIProto;					// �������Ӧ��AI��̬����

	//----------------------------------------------------------------------------
	// �ű�
	//----------------------------------------------------------------------------
	const CreatureScript*	m_pScript;					// ����ű�
	INT						m_nScriptUpdateTimer;		// ����ű�����AI��ʱ�䣬��ʼ��ʱΪINVALID_VALUE�������£��ɽű����������ʱ��
	INT						m_nScriptUpdateCountDown;	// ����ű�����AI����ʱ

	//-----------------------------------------------------------------------------
	// AIս���߼�����
	//-----------------------------------------------------------------------------

	// ����״̬
	Vector3*				m_pWPArray;					// ·��Ѳ�߹ֵ�Ѳ�ߵ������б�
	DWORD*					m_pWayPointTime;			// ��������ͣ����ʱ��
	INT						m_nWPNum;					// ���м���������
	INT						m_nWPIndex;					// ��ǰѲ�����ĸ�������
	BOOL					m_bReversePatrol;			// ·��Ѳ�߹��Ƿ����������෴·��Ѳ��
	BOOL					m_bIsPatroling;				// ����Ѳ��
	DWORD					m_nPatrolRestTick;			// ������һ��·������Ϣ��Tick
	INT						m_nLookForTargetTick;		// ��һ�����е�Tick


	// ׷���͹���
	Vector3					m_vPursueTargetPos;			// ����׷����ǰ��Ŀ���
	BOOL					m_bPursueFailed;			// ׷��ʧ��
	BOOL					m_bCanNotPursue;			// ��ʱ����׷��
	BOOL					m_bPathFinding;				// Ѱ·��
	INT						m_nCheckPursueTick;			// �����ѯ�ƶ���ʱ����

	DWORD					m_dwTargeUnitID;			// Ŀ�����ID
	INT						m_nCheckAIEventTick;		// ��һ�μ��AI�¼���tick
	DWORD					m_dwEnterCombatTick;		// ����ս����tick

	DWORD					m_dwUseSkillID;				// ʹ�õļ���ID
	INT						m_nNextAttackWaitTick;		// ���ι�����ɺ��´ι�����Ҫ�ȴ���ʱ��
	DWORD					m_dwSerial;					// �������к�

	// ����
	Vector3					m_vFleePos;					// ���ܵ�Ŀ���
	CVector2D				m_vFleeDir;					// ���ܵķ���
	DWORD					m_dwFleeTick;				// ����״̬����ʱ��
	BOOL					m_bArrivedFleePos;			// �Ƿ񵽴����ܵ�Ŀ���
	EFLEETIME				m_eFleeTime;				// ��������״̬�Ĵ���
	DWORD					m_dwHelperID;				// ��ȶ���ID
	package_list<DWORD>			m_listHelpID;				// ��ȶ����б�

	// ����
	Vector3					m_vPosEnterCombat;			// ����ս��ʱ������
	BOOL					m_bArrivedReturnPos;		// �Ƿ񷵻ص�����ս��ʱ������

	// ���־���
	DWORD					m_dwSpaceOutTick;			// ���־��봥�����
	BOOL					m_bArrivedPos;				// �Ƿ񵽴�Ŀ���

	//-----------------------------------------------------------------------
	// ���ϵͳ
	//-----------------------------------------------------------------------
	package_map<DWORD, tagEnmity*>	m_mapEnmity;		// ����б�
	DWORD					m_dwMaxEnmityUnitID;		// �����ID
	DWORD					m_dwCurrentVictim;			// ��ǰ����Ŀ��
	INT						m_nLockTargetTick;			// ����Ŀ�굹��ʱ
	INT						m_nNoAnmityTick;			// û�����ӳ�޵�������
	INT						m_nNoAttackTick;			// û�й�����������

	//-----------------------------------------------------------------------
	// Ѱ·���
	//-----------------------------------------------------------------------
	PathFinder*				m_pPathFinder;
	
	//-----------------------------------------------------------------------
	// �����ɫ���
	//-----------------------------------------------------------------------
	PetTracker*				m_pTracker;	

public:
	//-----------------------------------------------------------------------
	// �����׷����Ϊ
	//-----------------------------------------------------------------------
	ECreaturePursueType		m_ePursueType;
	// ��һ�������´��������Ƴ�����״̬������ΪFALSE,����׷��״̬����ΪTRUE
	// �����ƹ��ﲻ�ܶ�κ���
	BOOL					m_bCalHelp;					// �Ƿ���������
	DWORD					m_dwCallHelpTime;			// ��������������ʱ������

	DWORD					m_dwTalkTime;				// �Ի�ʱ��

	DWORD					dw_help_time;				// ����or���ܳ���ʱ��

};











