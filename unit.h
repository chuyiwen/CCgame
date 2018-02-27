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
*	@file		unit.h
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		�������NPC���������Ļ���
*/
#pragma once

#include "../../common/WorldDefine/move_define.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/combat_protocol.h"

#include "move_data.h"
#include "world.h"
#include "map.h"
#include "skill.h"
#include "buff.h"
#include "combat_handler.h"
#include "state_count_mgr.h"

//------------------------------------------------------------------------------
// ��Ŀ��ĵ���������ϵ
//------------------------------------------------------------------------------
enum ETargetFriendEnemy
{
	ETFE_Friendly		=	0x0001,		// �ѷ�
	ETFE_Hostile		=	0x0002,		// �з�
	ETFE_Independent	=	0x0004,		// ����
};

//-------------------------------------------------------------------------------
// Buff�����͵��Ӽ����Ľ��
//-------------------------------------------------------------------------------
enum EBuffCounteractAndWrapResult
{
	EBCAWR_CanNotAdd		=	0,		// �����������������
	EBCAWR_CanAdd			=	1,		// ������һ��Buff�����߲���Ҫ�������������
	EBCAWR_Wraped			=	2,		// �����ӽ�һ��Buff��Ѿ����
};

//---------------------------------------------------------------------------------
// Buff����
//---------------------------------------------------------------------------------
const INT MAX_BENEFIT_BUFF_NUM		=	35;		// ���е�����buff����
const INT MAX_DEBUFF_NUM			=	15;		// ���е��к�buff����
const INT MAX_BUFF_NUM				=	MAX_BENEFIT_BUFF_NUM + MAX_DEBUFF_NUM;

//--------------------------------------------------------------------------------
// ��Ѫ�������ػ����س־����ļ��ʱ��
//--------------------------------------------------------------------------------
const INT HP_MP_REGAIN_INTER_TICK		=	5 * TICK_PER_SECOND;	// ��Ѫ�����ļ��ʱ��
const INT VITALITY_REGAIN_INTER_TICK	=	60* TICK_PER_SECOND;	// �ػ����ļ��ʱ��
const INT ENDURANCE_REGAIN_INTER_TICK	=	TICK_PER_SECOND;		// �س־�����ʱ����
const INT RAGE_GEGAIN_INTER_TICK		=	TICK_PER_SECOND;		// ŭ�����¼��
const INT POINT_GEGAIN_INTER_TICK		=	3 * TICK_PER_SECOND;	// �������¼��
const INT ENERGY_GEGAIN_INTER_TICK		=	TICK_PER_SECOND;		// �������¼��
const INT FOCUS_GEGAIN_INTER_TICK		=	2 * TICK_PER_SECOND;	// ���е������¼��

//--------------------------------------------------------------------------------
// �ص��¼��ʱ��ͻָ�ֵ
//--------------------------------------------------------------------------------
const INT INPRISON_MORALE_REGAIN_INTER_TICK	= 60 * TICK_PER_SECOND;	// �ص���TICK
const INT INPRISON_MORAL_INCREASE_VAL						= 8;					// �ص���ֵ

//--------------------------------------------------------------------------------
// �����˼��ʱ��ͻָ�ֵ
//--------------------------------------------------------------------------------
const INT INJURY_REGAIN_INTER_TICK		= TICK_PER_SECOND * 60;				// �ָ�����TICK
const INT INJURY_INCREASE_VAL			= 10;								//�����ָ�ֵ

//--------------------------------------------------------------------------------
// ���������ĵ���ʱ
//--------------------------------------------------------------------------------
const INT PRICTICE_TICK		=	2 * TICK_PER_SECOND;
const INT PRICTICE_VALUE	=	10;//����ʱ����ֵ�ı仯��

class Skill;
class Buff;
class Unit;

//-----------------------------------------------------------------------------
// ����������
//-----------------------------------------------------------------------------
struct tagEnmity
{
	DWORD			dwID;						// ��˭�ĳ��
	INT				nEnmity;					// �������
	INT				nEnmityMod;					// ��������ӳ�
	BOOL			bActive;					// �Ƿ񼤻�
	tagEnmity()	{ dwID = INVALID_VALUE; nEnmity = 0; nEnmityMod = 0; bActive=TRUE; }
};
class Unit
{
public:
	//----------------------------------------------------------------------------
	// Constructor&Destructor
	//----------------------------------------------------------------------------
	Unit(DWORD dwID, DWORD dwMapID, const Vector3& vPos, const Vector3& vFaceTo);
	virtual ~Unit();

	//-------------------------------------------------------------------------------
	// ����Update����
	//-------------------------------------------------------------------------------
	virtual VOID	Update() {}
	VOID			update_time_issue();
	VOID			UpdateState();
	//----------------------------------------------------------------------------
	// �����ʲô�Ӷ���
	//----------------------------------------------------------------------------
	BOOL			IsRole()					{ return IS_PLAYER(m_dwID); }
	BOOL			IsCreature()				{ return IS_CREATURE(m_dwID) || IS_PET(m_dwID); }
	BOOL			IsPet()						{ return IS_PET(m_dwID); }
	//-------------------------------------------------------------------------------
	// ����Get
	//-------------------------------------------------------------------------------
	DWORD			GetID()	const				{ return m_dwID; }
	VOID			SetID(DWORD dwID)			{ m_dwID = dwID; }
	virtual BYTE	GetSex() const				{ return 0; }
	DWORD			GetMapID()					{ return m_dwMapID; }
	Map*			get_map()					{ return m_pMap; }
	INT				get_level()					{ return m_nLevel; }
	INT				GetCurLevelExp()			{ return m_nCurLevelExp; }
	MoveData&		GetMoveData()				{ return m_MoveData; }
	CombatHandler&	GetCombatHandler()			{ return m_CombatHandler; }

	//-------------------------------------------------------------------------------
	// ����Set
	//-------------------------------------------------------------------------------
	VOID			SetMap(Map* pMap)			{ m_pMap = pMap; }
	VOID			SetMapID(DWORD dwMapID)		{ m_dwMapID = dwMapID; }
	VOID			SetLevel(INT nLevel)		{ m_nLevel = nLevel; }
	VOID			SetCurLevelExp(INT nExp)	{ m_nCurLevelExp = nExp; }
	VOID			SetSize(Vector3 vSize)					{ m_Size = vSize;	}

	//--------------------------------------------------------------------------------
	// �ƶ�״̬���
	//--------------------------------------------------------------------------------
	EMoveState		GetMoveState()				{ return m_MoveData.m_eCurMove; }
	Vector3&		GetSize()					{ return m_Size; }
	Vector3&		GetCurPos()					{ return m_MoveData.m_vPos; }
	Vector3&		GetStartPos()				{ return m_MoveData.m_vPosStart; }
	Vector3&		GetDestPos()				{ return m_MoveData.m_vDest; }
	Vector3&		GetMoveDir()				{ return m_MoveData.m_vDir; }
	Vector3&		GetFaceTo()					{ return m_MoveData.m_vFace; }
	FLOAT			GetMovePassTime()			{ return m_MoveData.m_Timer.GetElapse() - m_MoveData.m_fStartTime; }
	Vector3			GetCurPosTop()				{ Vector3 vec = m_MoveData.m_vPos; vec.y = vec.y + m_Size.y; return vec; }
	INT				GetVisTileIndex() const		{ return m_MoveData.m_nVisTileIndex; }
	VOID			SetVisTileIndex(INT nIndex)	{ m_MoveData.m_nVisTileIndex = nIndex; }
	VOID			StartFeatRun();
	//---------------------------------------------------------------------------------
	// �ٶ����
	//---------------------------------------------------------------------------------
	FLOAT			GetXZSpeed();			//{ return m_fXZSpeed; }
	FLOAT			GetYSpeed()	const			{ return m_fYSpeed; }
	FLOAT			GetMountXZSpeed() const		{ return m_fMountXZSpeed; }
	FLOAT			GetSwimXZSpeed() const		{ return m_fSwimXZSpeed; }
	FLOAT			GetDropXZSpeed() const		{ return m_fDropXZSpeed; }
	FLOAT			GetSlideSpeed() const		{ return m_fSlideSpeed; }
	FLOAT			GetCarryXZSpeed() const		{ return m_fCarryXZSpeed; }

	//--------------------------------------------------------------------------------
	// �����ж�
	//--------------------------------------------------------------------------------
	BOOL			IsInDistance(Unit& target, FLOAT fDis);
	BOOL			IsInCombatDistance(Unit& target, FLOAT fDis, FLOAT fDiffDis = 72);
	BOOL			IsInFrontOfTarget(Unit& target);

	//--------------------------------------------------------------------------------
	// ���Զ�ȡ�����ü��޸�
	//--------------------------------------------------------------------------------
	INT				GetAttValue(INT nIndex);
	INT				GetBaseAttValue(INT nIndex);
	INT				GetAttModValue(INT nIndex);
	INT				GetAttModValuePct(INT nIndex);
	VOID			SetAttValue(INT nIndex, INT nValue, BOOL bSendMsg=TRUE);
	VOID			SetBaseAttValue(INT nIndex, INT nValue);
	VOID			SetAttModValue(INT nIndex, INT nValue);
	VOID			SetAttModValuePct(INT nIndex, INT nValuePct);
	VOID			ModAttValue(INT nIndex, INT nValueMod, BOOL bSendMsg=TRUE);
	VOID			ModBaseAttValue(INT nIndex, INT nValueMod);
	VOID			ModAttModValue(INT nIndex, INT nValueMod);
	VOID			ModAttModValuePct(INT nIndex, INT nValuePctMod);

	ERemoteRoleAtt	ERA2ERRA(ERoleAttribute eRA);
	ERoleAttribute	SkillDmgType2ERA(ESkillDmgType eSKillDmgType);
	ERoleAttribute	BuffResistType2ERA(EBuffResistType eBuffResistType);

	BOOL			IsTeamRemoteAtt(ERemoteRoleAtt eType);

	//------------------------------------------------------------------------------
	// ĳЩ���Ե��޸�
	//------------------------------------------------------------------------------
	VOID			ChangeHP(INT nValue, Unit* pSrc=NULL, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, bool bCrit=false, bool bBlock=false, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=INVALID_VALUE, INT nOtherDmg = 0);
	VOID			ChangeMP(INT nValue);
	//VOID			ChangeVitality(INT nValue);
	VOID			ChangeBrotherhood(INT nValue);
	VOID			ChangeWuhuen(INT nValue);
	VOID			ChangeRage(INT nValue);
	
	//------------------------------------------------------------------------------
	// ��ǰ״̬ -- ��״̬���ת������Ҫ�ֶ���ɡ���SetState()�����Զ��������״̬λ��
	//------------------------------------------------------------------------------
	DWORD			GetState()							{ return m_StateMgr.GetStateFlags(); }
	BOOL			IsInState(EState eState)			{ return m_StateMgr.IsInState(eState); }

	BOOL			IsDead()							{ return IsInState(ES_Dead); }
	virtual BOOL	IsInStateCantMove()					{ return IsInState(ES_Dead) || IsInState(ES_Dizzy) || IsInState(ES_Tie) || IsInState(ES_Spor); }
	virtual BOOL	IsInStateCantBeSkill()				{ return FALSE; }
	virtual BOOL	IsInStateCantCastSkill()			{ return IsInState(ES_Dead); }
	virtual BOOL	IsInStateInvisible()				{ return IsInState(ES_Lurk); }

	VOID			ClearState()						{ m_StateMgr.Reset(); }

	VOID			SetState(EState eState, BOOL bSendMsg=TRUE);
	VOID			UnsetState(EState eState, BOOL bSendMsg=TRUE);

	VOID			OnSetState(EState eState);
	VOID			OnUnSetState(EState eState);

	//----------------------------------------------------------------------------------
	// ս��ϵͳĿ�����ͣ�״̬�����ҹ�ϵ�ж�
	//----------------------------------------------------------------------------------
	virtual DWORD	TargetTypeFlag(Unit* pTarget) = 0;
	virtual DWORD	GetStateFlag();
	virtual DWORD	FriendEnemy(Unit* pTarget) = 0;

	//----------------------------------------------------------------------------------
	// ս��ϵͳ�¼����
	//----------------------------------------------------------------------------------
	virtual VOID	OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited) = 0;
	virtual VOID	OnDead(Unit* pSrc, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=0, BOOL bCrit = FALSE) = 0;
	virtual VOID	OnKill(Unit* pTarget) = 0;

	//----------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------
	Skill*			GetSkill(DWORD dwSkillID)			{ return m_mapSkill.find(dwSkillID); }
	INT				GetProduceSkillNum()				{ return m_nProduceSkillNum; }
	VOID			UpdateSkill();
	virtual VOID	StartSkillCoolDown(Skill* pSkill);
	virtual VOID	ClearSkillCoolDown(DWORD dwSkillID);
	VOID			ClearAllSkillCoodDown(ETalentType eType, DWORD dwExceptSkillID=INVALID_VALUE);

	//----------------------------------------------------------------------------------
	// Buff����
	//----------------------------------------------------------------------------------
	package_list<DWORD>*	GetBuffModifier(DWORD dwBuffID)		{ return m_mapBuffModifier.find(dwBuffID); }
	VOID			RegisterBuffModifier(DWORD dwBuffID, Skill* pSkill);
	VOID			UnRegisterBuffModifier(DWORD dwBuffID, Skill* pSkill);
	VOID			RegisterTriggerModifier(Skill* pSkill);
	VOID			UnRegisterTriggerModifier(Skill* pSkill);

	BOOL			OnActiveSkillBuffTrigger(Skill* pSkill, package_list<DWORD>& listTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);
	BOOL			OnBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);
	
	VOID			SetAbsorbDmg(BOOL bDmg) { m_bDmgAbs = bDmg;}
	BOOL			ISAbsorbDmg() { return m_bDmgAbs; }

	BOOL			IsDmgReturn() { return m_bDmgReturn; }
	VOID			SetDmgReturn(BOOL b) { m_bDmgReturn = b; }
	
	INT				OnAbsorDmg(INT nDmgValue);
	//-----------------------------------------------------------------------------------
	// Buff���
	//-----------------------------------------------------------------------------------
	INT				GetBuffNum()						{ return m_mapBuff.size(); }
	Buff*			GetBuffPtr(DWORD dwBuffID)			{ return m_mapBuff.find(dwBuffID); }
	BOOL			IsHaveBuff(DWORD dwBuffID)			{ return m_mapBuff.is_exist(dwBuffID); }

	VOID			GetAllBuffMsgInfo(tagBuffMsgInfo* const pBuffInfo, INT nMaxNum);
	BOOL			TryAddBuff(Unit* pSrc, const tagBuffProto* pBuffProto, const tagTriggerProto* pTriggerProto, Skill* pSkill, tagItem* pItem, BOOL bReCalAtt=TRUE, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);
	BOOL			GMTryAddBuff(Unit* pSrc, const tagBuffProto* pBuffProto, const tagTriggerProto* pTriggerProto, Skill* pSkill, tagItem* pItem, BOOL bReCalAtt=TRUE, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);
	BOOL			CancelBuff(DWORD dwBuffID);
	VOID			DispelBuff(BOOL bBenefit);
	VOID			DispelBuff(EBuffResistType eType);
	VOID			DispelBuff(DWORD dwBuffID);
	VOID			DispelBuff(INT nType, BOOL bLastOne);
	VOID			AddBuff(const tagBuffProto* pBuffProto, Unit* pSrc, DWORD dwSrcSkillID, const tagItem* pItem, INT nIndex, BOOL bRecalAtt);
	INT				RemoveBuff(DWORD dwBuffID, BOOL bRecalAtt, BOOL bSelf=FALSE);
	INT				RemoveBuff(Buff* pBuff, BOOL bRecalAtt, BOOL bSelf = FALSE);
	VOID			RemoveAllBuffBelongSkill(DWORD dwSkillID, BOOL bRecalAtt);
	VOID			RemoveAllBuff();
	VOID			RemoveAllBuff(BOOL bBenefit);
	VOID			UpdateBuff();
	VOID			OnInterruptBuffEvent(EBuffInterruptFlag eFlag, INT nMisc=INVALID_VALUE);
	INT				BuffCounteractAndWrap(Unit* pSrc, DWORD dwBuffID, INT nBuffLevel, INT nGroupLevel, DWORD dwGroupFlag, BOOL bBenefit, INT& nIndex);

	//VOID			CalBuffInstantEffect(Unit* pSrc, EBuffEffectMode eMode, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes=1, Unit* pTarget=NULL);
	//VOID			CalBuffPersistEffect(Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes=1, BOOL bSet=TRUE);
	VOID			WrapBuffPersistEffect(Buff* pBuff, Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod);
	VOID			CalBuffTargetList(Unit* pSrc, EBuffOPType eOPType, FLOAT fOPDist, FLOAT fOPRadius, EBuffFriendEnemy eFriendEnemy, DWORD dwTargetLimit, DWORD dwTargetStateLimit, std::vector<Unit*>& listTarget, Unit* pTarget=NULL);
	BOOL			IsBuffTargetValid(Unit* pTarget, EBuffFriendEnemy eFriendEnemy, DWORD dwTargetLimit, DWORD dwTargetStateLimit);
	Buff*			GetRelativeSkillBuff(DWORD dwSkillID, BOOL bBenefit = TRUE);
	Buff*			GetFiretBenifitBuff();
	//------------------------------------------------------------------------------------
	// ���������
	//------------------------------------------------------------------------------------
	VOID			AddEnmityCreature(Unit *pUnit);
	VOID			DelEnmityCreature(Unit *pUnit);
	VOID			ClearEnmityCreature();
	VOID			ChangeEnmityCreatureValue(DWORD dwRate);
	INT				GetEnmityCreatureSize() { return m_mapEnmityCreature.size(); }
	package_map<DWORD, DWORD>&	GetEnmityCreature() { return m_mapEnmityCreature; }

	//------------------------------------------------------------------------------------
	// Ǳ��
	//------------------------------------------------------------------------------------
	VOID			Lurk(INT nLurkLevel);
	VOID			UnLurk();
	
	//------------------------------------------------------------------------------------
	// ͬ���ƶ���Ϣ���
	//------------------------------------------------------------------------------------
	BOOL			HasDectiveSkill()			{ return FALSE; }//??
	BOOL			IsInVisDist(DWORD dwUnitID)	{ return FALSE; }//??
	BOOL			IsInVisList(DWORD dwUnitID);
	VOID			Add2VisList(DWORD dwUnitID);
	VOID			RemoveFromVisList(DWORD dwUnitID);

	//-----------------------------------------------------------------------------------
	// ���������
	//-----------------------------------------------------------------------------------
	BOOL			TestTrigger(Unit* pTarget, const tagTriggerProto* pProto, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);
	BOOL			TestEventTrigger(Unit* pTarget, const tagTriggerProto* pProto, const tagTriggerMod* pMod, DWORD dwEventMisc1, DWORD dwEventMisc2);
	BOOL			TestStateTrigger(Unit* pTarget, const tagTriggerProto* pProto, const tagTriggerMod* pMod);
	VOID			SetTriggerMode(BOOL bTrigger){ m_bTriggerOff = bTrigger; }
	
	DWORD			GetRingBuffID() { return m_nRingBuffID; }
	VOID			SetRingBuffID(DWORD nID) { m_nRingBuffID = nID;}

	std::vector<DWORD>&		GetRingBuffTargetList() { return m_ListRingBuffTarget; }
	VOID			ClearTargetRingBuff();
	
	VOID			RecalAtt(BOOL bSendMsg=TRUE, BOOL bHp = TRUE);

	DWORD			GetTargetID(){ return m_dwTargetID; }
	VOID			SetTargetID(DWORD dwID);

	virtual VOID	SetFlowUnit(DWORD dwID) { m_dwFlowUnit = dwID; }
	DWORD			GetFlowUnit() { return m_dwFlowUnit; }

protected:
	//----------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------
	VOID			SetAttRecalFlag(INT nIndex);
	bool			GetAttRecalFlag(INT nIndex);
	VOID			ClearAttRecalFlag();
	BOOL			NeedRecalAtt()							{ return m_bNeedRecal; }	
	INT				CalAttMod(INT nBase, INT nIndex);
	VOID			SendAttChange(INT nIndex);
	VOID			SendAttChangeMsg();

	virtual VOID	ReAccAtt();
	virtual VOID	OnAttChange(INT nIndex) = 0;
	virtual VOID	OnAttChange(bool bRecalFlag[ERA_End]) = 0;		// �ǵ�Ҫ�������浱ǰ���Ժ����
	//! mwh-2011-07-22 Ĭ��ʲôҲ����
	virtual VOID OnAttChange(INT nIndex, INT nDelta){}

	//----------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------
	VOID			AddSkillMod(Skill* pSkill);
	VOID			AddSkillBeMod(Skill* pSkill);
	VOID			RemoveSkillMod(Skill* pSkill);
	virtual VOID	AddSkill(Skill* pSkill, BOOL bSendMsg=TRUE);
	virtual VOID	RemoveSkill(DWORD dwSkillID);
	virtual VOID	ChangeSkillLevel(Skill* pSkill, ESkillLevelChange eType, INT nChange=1);
	virtual VOID	ChangeSkillExp(Skill *pSkill, INT nValue);

public:
	// ��ʱ�����ã������װ��move_data��
	FLOAT						m_fXZSpeed;							// ��ǰXZ�����ٶ�
	FLOAT						m_fYSpeed;							// ��ǰY�����ٶ�
	FLOAT						m_fSwimXZSpeed;						// ��Ӿ��XZ�����ٶ�
	FLOAT						m_fJumpXZSpeed;						// ��Ծ��XZ�����ٶȣ�����Ծ��ʼʱ��ֵ����Ծ�����в��ı�
	FLOAT						m_fJumpYSpeed;						// ��Ծ��Y�����ٶȣ�����Ծ��ʼʱ��ֵ������Ծ�����в��ı�
	FLOAT						m_fDropXZSpeed;						// ����ʱ��XZ�����ٶ�
	FLOAT						m_fSlideSpeed;						// �����ٶ�
	FLOAT						m_fMountXZSpeed;					// ��ǰ���XZ�����ٶ�
	FLOAT						m_fCarryXZSpeed;					// �����ƶ��ٶ�

protected:
	DWORD						m_dwID;								// ID
	INT							m_nLevel;							// �ȼ�
	INT							m_nCurLevelExp;						// ��ǰ��������
	DWORD						m_dwMapID;							// ��ͼID
	DWORD						m_dwInstanceID;						// ����ID
	Map*						m_pMap;								// ��ͼָ��

	MoveData					m_MoveData;							// �ƶ�״̬
	Vector3						m_Size;								// �ߴ�
	Vector3						m_SizeMount;						// ���ʱ�ĳߴ�
	
	DWORD						m_dwTargetID;						// Ŀ��ID
	//------------------------------------------------------------------------------------
	// ״̬
	//------------------------------------------------------------------------------------
	CountStateMgr<ES_End>		m_StateMgr;							// ����״̬��������ѣ�Σ��⼼������ȵȣ�
	BOOL						m_bIsFeating;						// �־�������
	DWORD						m_nFeatRestTick;					// �־���Ϣ��Tick
	//------------------------------------------------------------------------------------
	// ����
	//------------------------------------------------------------------------------------
	INT							m_nAtt[ERA_End];					// ��ǰ����
	INT							m_nBaseAtt[ERA_End];				// �������ԣ������������ܣ�װ���Լ�״̬Ч���������ڣ�
	INT							m_nAttMod[ERA_End];					// ��ֵ�ӳɣ��������ܣ�װ���Լ�״̬Ч���ܵĶ�ֵ�ӳ�ֵ��
	INT							m_nAttModPct[ERA_End];				// �ٷֱȼӳɣ��������ܣ�װ���Լ�״̬Ч���ܵİٷֱȼӳ�ֵ��

	bool						m_bAttRecalFlag[ERA_End];			// ������Ҫ���¼���ı�־λ
	BOOL						m_bNeedRecal;						// �Ƿ������Ըı�

	//-------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------
	package_map<DWORD, Skill*>			m_mapSkill;					// �����б�
	package_list<DWORD>				m_listSkillCoolDown;		// ��ȴ�����б�
	CombatHandler				m_CombatHandler;			// ��ǰ����ʩչ�ļ���
	INT							m_nProduceSkillNum;			// ��ǰ�����������ܵ�����

	package_map<DWORD, package_list<DWORD>*>	m_mapBuffModifier;			// �������ܶ�buff��Ӱ���б�
	package_map<DWORD, package_list<DWORD>*>	m_mapTriggerModifier;		// �������ܶ�trigger��Ӱ���б�

	BOOL						m_bTriggerOff;			// �Ƿ���Ҫ�жϴ�����

	//-------------------------------------------------------------------------------------
	// Buff
	//-------------------------------------------------------------------------------------
	Buff						m_Buff[MAX_BUFF_NUM];		// ��ǰ���ϵ�����Buff
	package_map<DWORD, Buff*>			m_mapBuff;					// ��ǰ���ϵ�buff�б�ע�⣺���ܶԸ�map������ѯ������
	package_list<DWORD>				m_listDestroyBuffID;		// Ҫɾ����Buff�б�
	
	DWORD						m_nRingBuffID;				//�⻷buff��ID
	std::vector<DWORD>			m_ListRingBuffTarget;		//�⻷buffӰ��Ŀ��

	BOOL						m_bDmgAbs;					//�Ƿ����˺�����
	BOOL						m_bDmgReturn;				//�Ƿ��˺�������

	//-------------------------------------------------------------------------------------
	// ӵ�и���ҳ�޵Ĺ����б�
	//-------------------------------------------------------------------------------------
	package_map<DWORD, DWORD>			m_mapEnmityCreature;

	//-------------------------------------------------------------------------------------
	// ���ӵ�Ǳ��״̬��λ�б�
	//-------------------------------------------------------------------------------------
	package_list<DWORD>				m_listLurkUnit;

	//-------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------
	INT							m_nHPMPRegainTickCountDown;				// ��Ѫ�����ĵ���ʱ
	INT							m_nVitalityRegainTickCountDown;			// �ػ����ĵ���ʱ
	INT							m_nEnduranceRegainTickCountDown;		// �س־����ĵ���ʱ
	INT							m_nInPrisonMoraleRegainTickCountDown;	// ����ֵ������ʱ
	INT							m_nInjuryRegainTickCountDown;			// �ָ����˵���ʱ
	INT							m_nRageRegainTickCountDown;				// ŭ�����µ���ʱ
	INT							m_nPointRegainTickCountDown;			// �������µ���ʱ
	INT							m_nEnergyRegainTickCountDown;			// �������µ���ʱ
	INT							m_nFocusRegainTickCountDown;			// ���е�������ʱ


	//����Ĺ���
	DWORD						m_dwFlowUnit;
};




//----------------------------------------------------------------------------------
// �ж�Ŀ���Ƿ���������Χ
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInDistance(Unit& target, FLOAT fDis) 
{
	const Vector3& vSrc = GetCurPos();
	const Vector3& vDest = target.GetCurPos();

	FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

	return fDis * fDis >= fDistSq;
}

//----------------------------------------------------------------------------------
// �ж���Ŀ���ս�������Ƿ����㣬fDisΪ���ܻ���Ʒ��ʹ�þ���
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInCombatDistance(Unit& target, FLOAT fDis, FLOAT fDiffDis)
{
	const Vector3& vSrc = GetCurPos();
	const Vector3& vDest = target.GetCurPos();

	FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

	// ս������Ҫ����˫�����Եİ뾶���ټ���һ���������Ա��������ӳ�
	FLOAT fRealDist = fDis + fDiffDis;

	return fRealDist * fRealDist >= fDistSq;
}

//----------------------------------------------------------------------------------
// �ж��Լ��Ƿ���Ŀ����ǰ������������
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInFrontOfTarget(Unit& target)
{
	Vector3& vSelfPos = GetCurPos();
	Vector3& vTargetPos = target.GetCurPos();

	// ���Ŀ���泯����ʸ��A
	Vector3 vTargetFace = target.GetFaceTo();

	// ���Ŀ�굽�Լ���ʸ��B
	Vector3 vDir = vSelfPos - vTargetPos;

	// ���ǵĵ������|A|*|B|*cos��,cos��Ϊ�Ǹ���ʾ�н�С�ڵ���90��
	// ����ʾ������Ŀ������
	if( vTargetFace.x * vDir.x + vTargetFace.z * vDir.z >= 0 )
		return TRUE;
	else
		return FALSE;
}


//-----------------------------------------------------------------------------------
// ��ȡ��ǰ����
//-----------------------------------------------------------------------------------
inline INT Unit::GetAttValue(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nAtt[nIndex];
}

//-----------------------------------------------------------------------------------
// ��ȡ��������
//-----------------------------------------------------------------------------------
inline INT Unit::GetBaseAttValue(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nBaseAtt[nIndex];
}

//-----------------------------------------------------------------------------------
// ��ȡ����ƽֱ�ӳ�
//-----------------------------------------------------------------------------------
inline INT Unit::GetAttModValue(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nAttMod[nIndex];
}

//-----------------------------------------------------------------------------------
// ��ȡ���԰ٷֱȼӳ�
//-----------------------------------------------------------------------------------
inline INT Unit::GetAttModValuePct(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nAttModPct[nIndex];
}

//------------------------------------------------------------------------------------
// ���õ�ǰ����
//------------------------------------------------------------------------------------
inline VOID Unit::SetAttValue(INT nIndex, INT nValue, BOOL bSendMsg)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAtt[nIndex] = nValue;

	// ȡ������
	if( m_nAtt[nIndex] < AttRes::GetInstance()->GetAttMin(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMin(nIndex);
	if( m_nAtt[nIndex] > AttRes::GetInstance()->GetAttMax(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMax(nIndex);

	OnAttChange(nIndex);

	if( bSendMsg ) SendAttChange(nIndex);
}

//-----------------------------------------------------------------------------------
// ���û�������
//-----------------------------------------------------------------------------------
inline VOID Unit::SetBaseAttValue(INT nIndex, INT nValue)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nBaseAtt[nIndex] = nValue;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// ��������ƽֱ�ӳ�
//-----------------------------------------------------------------------------------
inline VOID Unit::SetAttModValue(INT nIndex, INT nValue)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAttMod[nIndex] = nValue;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// �������԰ٷֱȼӳ�
//-----------------------------------------------------------------------------------
inline VOID Unit::SetAttModValuePct(INT nIndex, INT nValuePct)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAttModPct[nIndex] = nValuePct;
	SetAttRecalFlag(nIndex);
}

//------------------------------------------------------------------------------------
// �޸ĵ�ǰ����
//------------------------------------------------------------------------------------
inline VOID Unit::ModAttValue(INT nIndex, INT nValueMod, BOOL bSendMsg)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );

	INT nOriginValue = m_nAtt[nIndex];		// �õ���ʼֵ

	m_nAtt[nIndex] += nValueMod;			// ���õ�ǰֵ

	// ȡ������
	if( m_nAtt[nIndex] < AttRes::GetInstance()->GetAttMin(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMin(nIndex);
	if( m_nAtt[nIndex] > AttRes::GetInstance()->GetAttMax(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMax(nIndex);

	OnAttChange(nIndex);
	OnAttChange(nIndex, nValueMod);

	// �����ֵȷʵ�ı䲢����Ҫ������Ϣ������Ϣ
	if( nOriginValue != m_nAtt[nIndex] && bSendMsg ) SendAttChange(nIndex);
}

//-----------------------------------------------------------------------------------
// �޸Ļ�������
//-----------------------------------------------------------------------------------
inline VOID Unit::ModBaseAttValue(INT nIndex, INT nValueMod)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nBaseAtt[nIndex] += nValueMod;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// �޸�����ƽֱ�ӳ�
//-----------------------------------------------------------------------------------
inline VOID Unit::ModAttModValue(INT nIndex, INT nValueMod)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	if (nValueMod == 0) return;
	m_nAttMod[nIndex] += nValueMod;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// �޸����԰ٷֱȼӳ�
//-----------------------------------------------------------------------------------
inline VOID Unit::ModAttModValuePct(INT nIndex, INT nValuePctMod)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAttModPct[nIndex] += nValuePctMod;
	SetAttRecalFlag(nIndex);
}

//----------------------------------------------------------------------------------
// �������¼����ʶλ����
//----------------------------------------------------------------------------------
inline VOID Unit::SetAttRecalFlag(INT nIndex)
{
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );
	m_bAttRecalFlag[nIndex] = true;
	if( !m_bNeedRecal ) m_bNeedRecal = TRUE;
}

//----------------------------------------------------------------------------------
// �õ�ĳ�������Ƿ���Ҫ����
//----------------------------------------------------------------------------------
inline bool Unit::GetAttRecalFlag(INT nIndex)
{
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );
	return m_bAttRecalFlag[nIndex];
}

//----------------------------------------------------------------------------------
// ������������־λ
//----------------------------------------------------------------------------------
inline VOID Unit::ClearAttRecalFlag()
{
	ZeroMemory(m_bAttRecalFlag, sizeof(m_bAttRecalFlag));
	m_bNeedRecal = FALSE;
}

//----------------------------------------------------------------------------------
// ����������Զ������ӳ��
//----------------------------------------------------------------------------------
inline ERemoteRoleAtt Unit::ERA2ERRA(ERoleAttribute eRA)
{
	switch(eRA)
	{
	case ERA_MaxHP:
		return ERRA_MaxHP;
		break;

	case ERA_HP:
		return ERRA_HP;
		break;

	case ERA_MaxMP:
		return ERRA_MaxMP;
		break;

	case ERA_MP:
		return ERRA_MP;
		break;

	case ERA_Love:
		return ERRA_Rage;
		break;

	case ERA_Speed_XZ:
		return ERRA_Speed_XZ;
		break;

	case ERA_Speed_Mount:
		return ERRA_Speed_Mount;
		break;

	case ERA_Speed_Y:
		return ERRA_Speed_Y;
		break;

	case ERA_Shape:
		return ERRA_Shape;
		break;

	default:
		return ERRA_Null;
		break;
	}

	return ERRA_Null;
}

//----------------------------------------------------------------------------------
// ���ݼ����˺�������ȷ����Ӧ�������˺���������
//----------------------------------------------------------------------------------
inline ERoleAttribute Unit::SkillDmgType2ERA(ESkillDmgType eSKillDmgType)
{
	switch(eSKillDmgType)
	{
	case ESDGT_Ordinary:
		return ERA_Derate_Ordinary;
		break;

	case ESDGT_Soil:
		return ERA_Derate_Soil;
		break;

	case ESDGT_Gold:
		return ERA_Derate_Gold;
		break;

	case ESDGT_Wood:
		return ERA_Derate_Wood;
		break;

	case ESDGT_Fire:
		return ERA_Derate_Fire;
		break;

	case ESDGT_Water:
		return ERA_Derate_Water;
		break;

	//case ESDGT_Injury:
	//	return ERA_Derate_Injury;
	//	break;

	//case ESDGT_Stunt:
	//	return ERA_Derate_Stunt;
	//	break;

	default:
		return ERA_Derate_Ordinary;
		break;
	}
}

//----------------------------------------------------------------------------------
// ����Buff�Ŀ������͵õ���Ӧ�����￹������
//----------------------------------------------------------------------------------
inline ERoleAttribute Unit::BuffResistType2ERA(EBuffResistType eBuffResistType)
{
	switch(eBuffResistType)
	{
	case EBRT_Bleeding:
		return ERA_Resist_Bleeding;
		break;

	case EBRT_Weak:
		return ERA_Resist_Weak;
		break;

	case EBRT_Choas:
		return ERA_Resist_Choas;
		break;

	case EBRT_Special:
		return ERA_Resist_Special;
		break;

	case EBRT_Regain:
		return ERA_Regain_Addtion;
		break;

	default:
		return ERA_Null;
		break;
	}
}

//----------------------------------------------------------------------------------------
// ���ĳ�������ǲ��Ƕ���ͬ��������
//----------------------------------------------------------------------------------------
inline BOOL Unit::IsTeamRemoteAtt(ERemoteRoleAtt eType)
{
	if(eType == ERRA_MaxHP || eType == ERRA_HP || eType == ERRA_MaxMP || eType == ERRA_MP)
		return TRUE;
	else
		return FALSE;
}


//-----------------------------------------------------------------------------------------
// �ı�����
//-----------------------------------------------------------------------------------------
inline VOID Unit::ChangeMP(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_MP, nValue);

	if( nValue < 0 )
	{
		OnInterruptBuffEvent(EBIF_MPLower);		// ���Buff�¼�
	}
}

//-----------------------------------------------------------------------------------------
// �ı䰮��
//-----------------------------------------------------------------------------------------
inline VOID Unit::ChangeRage(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_Love, nValue);

	if( nValue < 0 )
	{
		OnInterruptBuffEvent(EBIF_RageLower);		// ���Buff�¼�
	}
}

//-----------------------------------------------------------------------------------------
// �ı�����
//-----------------------------------------------------------------------------------------
//inline VOID Unit::ChangeVitality(INT nValue)
inline VOID Unit::ChangeBrotherhood(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_Brotherhood, nValue);

	//if( nValue < 0 )
	//{
	//	OnInterruptBuffEvent(EBIF_VitalityLower);		// ���Buff�¼�
	//}
}

//-----------------------------------------------------------------------------------------
// �ı����
//-----------------------------------------------------------------------------------------
inline VOID Unit::ChangeWuhuen(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_Wuhuen, nValue);

	//if( nValue < 0 )
	//{
	//	OnInterruptBuffEvent(EBIF_EnduranceLower);		// ���Buff�¼�
	//}
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
inline VOID Unit::SetState(EState eState, BOOL bSendMsg)
{
	BOOL bIn = IsInState(eState);

	m_StateMgr.SetState(eState);
	OnSetState(eState);

	if( !bIn && bSendMsg )
	{
		NET_SIS_set_state send;
		send.dw_role_id = GetID();
		send.eState = eState;
		if( get_map() )
		{
			get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
		}

		NET_SIS_monster_enter_combat send_combat;
		send.dw_role_id = GetID();
		if( get_map() )
		{
			get_map()->send_big_visible_tile_message(this, &send_combat, send_combat.dw_size);
		}
	}
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
inline VOID Unit::UnsetState(EState eState, BOOL bSendMsg)
{
	if( !IsInState(eState) ) return;

	m_StateMgr.UnsetState(eState);

	if( !IsInState(eState) )
	{
		OnUnSetState(eState);
		if( bSendMsg )
		{
			NET_SIS_unset_state send;
			send.dw_role_id = GetID();
			send.eState = eState;
			if( get_map() )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
		}
	}
}

//-----------------------------------------------------------------------------------------
// ��״̬������ʱ
//-----------------------------------------------------------------------------------------
inline VOID Unit::OnSetState(EState eState)
{
	switch(eState)
	{
		// ѣ��
	case ES_Dizzy:
		{
			GetMoveData().StopMoveForce();	// ǿ��ֹͣ�ƶ�
			GetCombatHandler().End();		// ֹͣ��ǰ����
		}
		break;

		// ����
	case ES_Tie:
		{
			GetMoveData().StopMoveForce();	// ǿ��ֹͣ�ƶ�
		}
		break;

		// ��˯
	case ES_Spor:
		{
			GetMoveData().StopMoveForce();	// ǿ��ֹͣ�ƶ�
			GetCombatHandler().End();		// ֹͣ��ǰ����
		}
		break;

		// �޵� 
	case ES_Invincible:
		{
			RemoveAllBuff(FALSE);			// �����������debuff
		}
		break;

		// ����
	case ES_Lurk:
		{
			
		}
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------------------
// ��״̬������ʱ
//-----------------------------------------------------------------------------------------
inline VOID Unit::OnUnSetState(EState eState)
{
	switch(eState)
	{
	case ES_Lurk:
		{

		}
		break;

	default:
		break;
	}
}

//----------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------
inline VOID Unit::UpdateSkill()
{
	if( m_listSkillCoolDown.empty() ) return;

	DWORD dwSkillID = INVALID_VALUE;
	package_list<DWORD>::list_iter it = m_listSkillCoolDown.begin();
	while( m_listSkillCoolDown.find_next(it, dwSkillID) )
	{
		Skill* pSkill = m_mapSkill.find(dwSkillID);
		if( VALID_POINT(pSkill) && pSkill->CountDownCoolDown() )
		{
			m_listSkillCoolDown.erase(dwSkillID);
		}
	}
}

//----------------------------------------------------------------------------------
// ��ռ���CD
//----------------------------------------------------------------------------------
inline VOID Unit::ClearAllSkillCoodDown(ETalentType eType, DWORD dwExceptSkillID)
{
	if( m_listSkillCoolDown.empty() ) return;

	if( ETT_Null == eType )
	{
		// ���м�����ȴ
		DWORD dwSkillID = INVALID_VALUE;
		package_list<DWORD>::list_iter it = m_listSkillCoolDown.begin();
		while( m_listSkillCoolDown.find_next(it, dwSkillID) )
		{
			if( dwSkillID == dwExceptSkillID ) continue;

			ClearSkillCoolDown(dwSkillID);
		}
	}
	else
	{
		if( eType >= ETT_End ) return;

		// ĳ�����ʼ�����ȴ
		DWORD dwSkillID = INVALID_VALUE;
		package_list<DWORD>::list_iter it = m_listSkillCoolDown.begin();
		while( m_listSkillCoolDown.find_next(it, dwSkillID) )
		{
			if( dwSkillID == dwExceptSkillID ) continue;

			Skill* pSkill = GetSkill(dwSkillID);
			if( pSkill->GetTalentType() != eType ) continue;

			ClearSkillCoolDown(dwSkillID);
		}
	}
}

//----------------------------------------------------------------------------------
// ������Buffд���ⲿ��Ϣ
//----------------------------------------------------------------------------------
inline VOID Unit::GetAllBuffMsgInfo(tagBuffMsgInfo* const pBuffInfo, INT nMaxNum)
{
	if( nMaxNum <= 0 ) return;

	INT nIndex = 0;	// д���Index

	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		pBuffInfo[nIndex].dwBuffTypeID = m_Buff[n].GetTypeID();
		pBuffInfo[nIndex].nWarpTimes = m_Buff[n].GetWrapTimes();
		pBuffInfo[nIndex].nMaxPersistTime = m_Buff[n].GetPersistTime();
		pBuffInfo[nIndex].nPersistTimeLeft = m_Buff[n].GetPersistTimeLeft();

		if( ++nIndex > nMaxNum ) break;
	}
}

//----------------------------------------------------------------------------------
// Ǳ��
//----------------------------------------------------------------------------------
inline VOID Unit::Lurk(INT nLurkLevel)
{
	Map *pMap = get_map();
	if( !VALID_POINT(pMap) )
	{
		return;
	}
	
	// ���õ�λ״̬
	SetState(ES_Lurk);

	// Ǳ�еȼ�����//??

	// ͬ��
	pMap->lurk(this);
}

//----------------------------------------------------------------------------------
// ����Ǳ��״̬
//----------------------------------------------------------------------------------
inline VOID Unit::UnLurk()
{
	Map *pMap = get_map();
	if( !VALID_POINT(pMap) )
	{
		return;
	}

	// ���õ�λ״̬
	UnsetState(ES_Lurk);

	// ͬ��
	if( !IsInState(ES_Lurk) )
	{
		pMap->unlurk(this);
	}
}

//----------------------------------------------------------------------------------
// �Ƿ��ڿ���Ǳ�е�λ�б�
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInVisList(DWORD dwUnitID)
{
	return m_listLurkUnit.is_exist(dwUnitID);
}

//----------------------------------------------------------------------------------
// �������Ǳ�е�λ�б�
//----------------------------------------------------------------------------------
inline VOID Unit::Add2VisList(DWORD dwUnitID)
{
	ASSERT(!m_listLurkUnit.is_exist(dwUnitID));

	m_listLurkUnit.push_back(dwUnitID);
}

//----------------------------------------------------------------------------------
// �ӿ���Ǳ�е�λ�б����޳�
//----------------------------------------------------------------------------------
inline VOID Unit::RemoveFromVisList(DWORD dwUnitID)
{
	ASSERT(m_listLurkUnit.is_exist(dwUnitID));
	
	m_listLurkUnit.erase(dwUnitID);
}
