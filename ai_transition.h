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
*	@file		ai_transition.h
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		AI״̬ת����
*/

#pragma once
enum AIStateType;
class Creature;
class AIController;

//-----------------------------------------------------------------------------
// ״̬ת�����ӿ�
//-----------------------------------------------------------------------------
class AITransition
{
public:
	AITransition() {}
	virtual ~AITransition() {}

	virtual AIStateType Transition(AIController* pAI, AIStateType eCurState) = 0;

	AIStateType TransitionToFlee(AIController* pAI);
};

//-----------------------------------------------------------------------------
// ������״̬ת����
//-----------------------------------------------------------------------------
class AggressiveTransition : public AITransition
{
public:
	AggressiveTransition() {}
	~AggressiveTransition() {}

public:
	AIStateType	Transition(AIController* pAI, AIStateType eCurState);
	static AITransition* Instance();

private:
	AIStateType IdleTransition(AIController* pAI);
	AIStateType PursueTransition(AIController* pAI);
	AIStateType AttackTransition(AIController* pAI);
	AIStateType ReturnTransition(AIController* pAI);
	AIStateType CallForHelpTransition(AIController* pAI);	
	AIStateType FleeTransition(AIController* pAI);
	
};

//-----------------------------------------------------------------------------
// ������״̬ת����
//-----------------------------------------------------------------------------
class GuardTransition : public AITransition
{
public:
	GuardTransition() {}
	~GuardTransition() {}

public:
	AIStateType Transition(AIController* pAI, AIStateType eCurState);
	static AITransition* Instance();

private:
	AIStateType IdleTransition(AIController* pAI);
	AIStateType PursueTransition(AIController* pAI);
	AIStateType AttackTransition(AIController* pAI);
	AIStateType ReturnTransition(AIController* pAI);
	AIStateType CallForHelpTransition(AIController* pAI);	
	AIStateType FleeTransition(AIController* pAI);
	
};

//-----------------------------------------------------------------------------
// ������״̬ת����
//-----------------------------------------------------------------------------
class BarbetteTransition : public AITransition
{
public:
	BarbetteTransition() {}
	~BarbetteTransition() {}

public:
	AIStateType Transition(AIController* pAI, AIStateType eCurState);
	static AITransition* Instance();

private:
	AIStateType IdleTransition(AIController* pAI);
	AIStateType AttackTransition(AIController* pAI);
	AIStateType ReturnTransition(AIController* pAI);
};

//-----------------------------------------------------------------------------
// ������״̬ת����
//-----------------------------------------------------------------------------
class SpaceOutTransition : public AITransition
{
public:
	SpaceOutTransition() {}
	~SpaceOutTransition() {}

public:
	AIStateType Transition(AIController* pAI, AIStateType eCurState);
	static AITransition* Instance();

private:
	AIStateType IdleTransition(AIController* pAI);
	AIStateType PursueTransition(AIController* pAI);
	AIStateType AttackTransition(AIController* pAI);
	AIStateType ReturnTransition(AIController* pAI);
	AIStateType SpaceTransition(AIController* pAI);
};

//-----------------------------------------------------------------------------
// ���﹥����״̬ת����
// 
//-----------------------------------------------------------------------------
class PetATTransition : public AITransition
{
public:
	PetATTransition() {}
	~PetATTransition() {}

public:
	AIStateType	Transition(AIController* pAI, AIStateType eCurState);
	static AITransition* Instance();

private:

	AIStateType IdleTransition(AIController* pAI);
	AIStateType PursueTransition(AIController* pAI);
	AIStateType AttackTransition(AIController* pAI);
	
};


//------------------------------------------------------------------------------
// ������״̬״̬ת����
// 
//------------------------------------------------------------------------------
class AlertAITransition : public AITransition
{

public:

	AlertAITransition() {}
	~AlertAITransition(){}

public:

	AIStateType Transition( AIController* pAI, AIStateType eCurState );
	static AITransition* Instance( );

private:

	AIStateType IdleTransition(AIController* pAI);
	AIStateType PursueTransition(AIController* pAI);
	AIStateType AttackTransition(AIController* pAI);
	AIStateType ReturnTransition(AIController* pAI);
	AIStateType AlertTransition(AIController* pAI);
	AIStateType CallForHelpTransition(AIController* pAI);	
	AIStateType FleeTransition(AIController* pAI);

};


//-----------------------------------------------------------------------------
// ����NPC����״̬�л���
//-----------------------------------------------------------------------------
class HuchengAITransition : public AITransition
{

public:
	
	 HuchengAITransition() {}
	~HuchengAITransition() {}

private:

	AIStateType		IdleTransition(AIController* pAI);
	AIStateType		PursueTransition(AIController* pAI);
	AIStateType		AttackTransition(AIController* pAI);
	AIStateType		ReturnTransition(AIController* pAI);
	AIStateType		CallForHelpTransition(AIController* pAI);	


public:

	AIStateType Transition( AIController* pAI, AIStateType eCurState );
	static AITransition* Instance( );

};