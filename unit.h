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
*	@brief		人物怪物NPC及地物宠物的基类
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
// 与目标的敌我中立关系
//------------------------------------------------------------------------------
enum ETargetFriendEnemy
{
	ETFE_Friendly		=	0x0001,		// 友方
	ETFE_Hostile		=	0x0002,		// 敌方
	ETFE_Independent	=	0x0004,		// 中立
};

//-------------------------------------------------------------------------------
// Buff抵消和叠加计算后的结果
//-------------------------------------------------------------------------------
enum EBuffCounteractAndWrapResult
{
	EBCAWR_CanNotAdd		=	0,		// 被抵消掉，不能添加
	EBCAWR_CanAdd			=	1,		// 抵消掉一个Buff，或者不需要抵消，可以添加
	EBCAWR_Wraped			=	2,		// 被叠加进一个Buff里，已经添加
};

//---------------------------------------------------------------------------------
// Buff数量
//---------------------------------------------------------------------------------
const INT MAX_BENEFIT_BUFF_NUM		=	35;		// 所有的有益buff数量
const INT MAX_DEBUFF_NUM			=	15;		// 所有的有害buff数量
const INT MAX_BUFF_NUM				=	MAX_BENEFIT_BUFF_NUM + MAX_DEBUFF_NUM;

//--------------------------------------------------------------------------------
// 回血回能量回活力回持久力的间隔时间
//--------------------------------------------------------------------------------
const INT HP_MP_REGAIN_INTER_TICK		=	5 * TICK_PER_SECOND;	// 回血回蓝的间隔时间
const INT VITALITY_REGAIN_INTER_TICK	=	60* TICK_PER_SECOND;	// 回活力的间隔时间
const INT ENDURANCE_REGAIN_INTER_TICK	=	TICK_PER_SECOND;		// 回持久力的时间间隔
const INT RAGE_GEGAIN_INTER_TICK		=	TICK_PER_SECOND;		// 怒气更新间隔
const INT POINT_GEGAIN_INTER_TICK		=	3 * TICK_PER_SECOND;	// 点数更新间隔
const INT ENERGY_GEGAIN_INTER_TICK		=	TICK_PER_SECOND;		// 能量更新间隔
const INT FOCUS_GEGAIN_INTER_TICK		=	2 * TICK_PER_SECOND;	// 集中点数更新间隔

//--------------------------------------------------------------------------------
// 回道德间隔时间和恢复值
//--------------------------------------------------------------------------------
const INT INPRISON_MORALE_REGAIN_INTER_TICK	= 60 * TICK_PER_SECOND;	// 回道德TICK
const INT INPRISON_MORAL_INCREASE_VAL						= 8;					// 回道德值

//--------------------------------------------------------------------------------
// 灵气伤间隔时间和恢复值
//--------------------------------------------------------------------------------
const INT INJURY_REGAIN_INTER_TICK		= TICK_PER_SECOND * 60;				// 恢复灵气TICK
const INT INJURY_INCREASE_VAL			= 10;								//灵气恢复值

//--------------------------------------------------------------------------------
// 武器修炼的倒计时
//--------------------------------------------------------------------------------
const INT PRICTICE_TICK		=	2 * TICK_PER_SECOND;
const INT PRICTICE_VALUE	=	10;//修炼时义气值的变化量

class Skill;
class Buff;
class Unit;

//-----------------------------------------------------------------------------
// 怪物仇恨数据
//-----------------------------------------------------------------------------
struct tagEnmity
{
	DWORD			dwID;						// 对谁的仇恨
	INT				nEnmity;					// 仇恨数量
	INT				nEnmityMod;					// 仇恨数量加成
	BOOL			bActive;					// 是否激活
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
	// 各种Update函数
	//-------------------------------------------------------------------------------
	virtual VOID	Update() {}
	VOID			update_time_issue();
	VOID			UpdateState();
	//----------------------------------------------------------------------------
	// 检查是什么子对象
	//----------------------------------------------------------------------------
	BOOL			IsRole()					{ return IS_PLAYER(m_dwID); }
	BOOL			IsCreature()				{ return IS_CREATURE(m_dwID) || IS_PET(m_dwID); }
	BOOL			IsPet()						{ return IS_PET(m_dwID); }
	//-------------------------------------------------------------------------------
	// 各种Get
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
	// 各种Set
	//-------------------------------------------------------------------------------
	VOID			SetMap(Map* pMap)			{ m_pMap = pMap; }
	VOID			SetMapID(DWORD dwMapID)		{ m_dwMapID = dwMapID; }
	VOID			SetLevel(INT nLevel)		{ m_nLevel = nLevel; }
	VOID			SetCurLevelExp(INT nExp)	{ m_nCurLevelExp = nExp; }
	VOID			SetSize(Vector3 vSize)					{ m_Size = vSize;	}

	//--------------------------------------------------------------------------------
	// 移动状态相关
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
	// 速度相关
	//---------------------------------------------------------------------------------
	FLOAT			GetXZSpeed();			//{ return m_fXZSpeed; }
	FLOAT			GetYSpeed()	const			{ return m_fYSpeed; }
	FLOAT			GetMountXZSpeed() const		{ return m_fMountXZSpeed; }
	FLOAT			GetSwimXZSpeed() const		{ return m_fSwimXZSpeed; }
	FLOAT			GetDropXZSpeed() const		{ return m_fDropXZSpeed; }
	FLOAT			GetSlideSpeed() const		{ return m_fSlideSpeed; }
	FLOAT			GetCarryXZSpeed() const		{ return m_fCarryXZSpeed; }

	//--------------------------------------------------------------------------------
	// 距离判断
	//--------------------------------------------------------------------------------
	BOOL			IsInDistance(Unit& target, FLOAT fDis);
	BOOL			IsInCombatDistance(Unit& target, FLOAT fDis, FLOAT fDiffDis = 72);
	BOOL			IsInFrontOfTarget(Unit& target);

	//--------------------------------------------------------------------------------
	// 属性读取、设置及修改
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
	// 某些属性的修改
	//------------------------------------------------------------------------------
	VOID			ChangeHP(INT nValue, Unit* pSrc=NULL, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, bool bCrit=false, bool bBlock=false, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=INVALID_VALUE, INT nOtherDmg = 0);
	VOID			ChangeMP(INT nValue);
	//VOID			ChangeVitality(INT nValue);
	VOID			ChangeBrotherhood(INT nValue);
	VOID			ChangeWuhuen(INT nValue);
	VOID			ChangeRage(INT nValue);
	
	//------------------------------------------------------------------------------
	// 当前状态 -- 各状态间的转换，需要手动完成。即SetState()不会自动清除其他状态位。
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
	// 战斗系统目标类型，状态，敌我关系判定
	//----------------------------------------------------------------------------------
	virtual DWORD	TargetTypeFlag(Unit* pTarget) = 0;
	virtual DWORD	GetStateFlag();
	virtual DWORD	FriendEnemy(Unit* pTarget) = 0;

	//----------------------------------------------------------------------------------
	// 战斗系统事件相关
	//----------------------------------------------------------------------------------
	virtual VOID	OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited) = 0;
	virtual VOID	OnDead(Unit* pSrc, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=0, BOOL bCrit = FALSE) = 0;
	virtual VOID	OnKill(Unit* pTarget) = 0;

	//----------------------------------------------------------------------------------
	// 技能相关
	//----------------------------------------------------------------------------------
	Skill*			GetSkill(DWORD dwSkillID)			{ return m_mapSkill.find(dwSkillID); }
	INT				GetProduceSkillNum()				{ return m_nProduceSkillNum; }
	VOID			UpdateSkill();
	virtual VOID	StartSkillCoolDown(Skill* pSkill);
	virtual VOID	ClearSkillCoolDown(DWORD dwSkillID);
	VOID			ClearAllSkillCoodDown(ETalentType eType, DWORD dwExceptSkillID=INVALID_VALUE);

	//----------------------------------------------------------------------------------
	// Buff触发
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
	// Buff相关
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
	// 怪物仇恨相关
	//------------------------------------------------------------------------------------
	VOID			AddEnmityCreature(Unit *pUnit);
	VOID			DelEnmityCreature(Unit *pUnit);
	VOID			ClearEnmityCreature();
	VOID			ChangeEnmityCreatureValue(DWORD dwRate);
	INT				GetEnmityCreatureSize() { return m_mapEnmityCreature.size(); }
	package_map<DWORD, DWORD>&	GetEnmityCreature() { return m_mapEnmityCreature; }

	//------------------------------------------------------------------------------------
	// 潜行
	//------------------------------------------------------------------------------------
	VOID			Lurk(INT nLurkLevel);
	VOID			UnLurk();
	
	//------------------------------------------------------------------------------------
	// 同步移动信息相关
	//------------------------------------------------------------------------------------
	BOOL			HasDectiveSkill()			{ return FALSE; }//??
	BOOL			IsInVisDist(DWORD dwUnitID)	{ return FALSE; }//??
	BOOL			IsInVisList(DWORD dwUnitID);
	VOID			Add2VisList(DWORD dwUnitID);
	VOID			RemoveFromVisList(DWORD dwUnitID);

	//-----------------------------------------------------------------------------------
	// 触发器相关
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
	// 属性相关
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
	virtual VOID	OnAttChange(bool bRecalFlag[ERA_End]) = 0;		// 记得要在重算玩当前属性后调用
	//! mwh-2011-07-22 默认什么也不做
	virtual VOID OnAttChange(INT nIndex, INT nDelta){}

	//----------------------------------------------------------------------------------
	// 技能相关
	//----------------------------------------------------------------------------------
	VOID			AddSkillMod(Skill* pSkill);
	VOID			AddSkillBeMod(Skill* pSkill);
	VOID			RemoveSkillMod(Skill* pSkill);
	virtual VOID	AddSkill(Skill* pSkill, BOOL bSendMsg=TRUE);
	virtual VOID	RemoveSkill(DWORD dwSkillID);
	virtual VOID	ChangeSkillLevel(Skill* pSkill, ESkillLevelChange eType, INT nChange=1);
	virtual VOID	ChangeSkillExp(Skill *pSkill, INT nValue);

public:
	// 临时测试用，后面封装到move_data中
	FLOAT						m_fXZSpeed;							// 当前XZ方向速度
	FLOAT						m_fYSpeed;							// 当前Y方向速度
	FLOAT						m_fSwimXZSpeed;						// 游泳的XZ方向速度
	FLOAT						m_fJumpXZSpeed;						// 跳跃的XZ方向速度，在跳跃开始时付值，跳跃过程中不改变
	FLOAT						m_fJumpYSpeed;						// 跳跃的Y方向速度，在跳跃开始时付值，在跳跃过程中不改变
	FLOAT						m_fDropXZSpeed;						// 掉落时的XZ方向速度
	FLOAT						m_fSlideSpeed;						// 滑落速度
	FLOAT						m_fMountXZSpeed;					// 当前骑乘XZ方向速度
	FLOAT						m_fCarryXZSpeed;					// 搬运移动速度

protected:
	DWORD						m_dwID;								// ID
	INT							m_nLevel;							// 等级
	INT							m_nCurLevelExp;						// 当前升级经验
	DWORD						m_dwMapID;							// 地图ID
	DWORD						m_dwInstanceID;						// 副本ID
	Map*						m_pMap;								// 地图指针

	MoveData					m_MoveData;							// 移动状态
	Vector3						m_Size;								// 尺寸
	Vector3						m_SizeMount;						// 骑乘时的尺寸
	
	DWORD						m_dwTargetID;						// 目标ID
	//------------------------------------------------------------------------------------
	// 状态
	//------------------------------------------------------------------------------------
	CountStateMgr<ES_End>		m_StateMgr;							// 各种状态（死亡，眩晕，封技，定身等等）
	BOOL						m_bIsFeating;						// 恐惧逃跑中
	DWORD						m_nFeatRestTick;					// 恐惧休息的Tick
	//------------------------------------------------------------------------------------
	// 属性
	//------------------------------------------------------------------------------------
	INT							m_nAtt[ERA_End];					// 当前属性
	INT							m_nBaseAtt[ERA_End];				// 基本属性（不将被动技能，装备以及状态效果加入在内）
	INT							m_nAttMod[ERA_End];					// 定值加成（被动技能，装备以及状态效果总的定值加成值）
	INT							m_nAttModPct[ERA_End];				// 百分比加成（被动技能，装备以及状态效果总的百分比加成值）

	bool						m_bAttRecalFlag[ERA_End];			// 属性需要重新计算的标志位
	BOOL						m_bNeedRecal;						// 是否有属性改变

	//-------------------------------------------------------------------------------------
	// 技能
	//-------------------------------------------------------------------------------------
	package_map<DWORD, Skill*>			m_mapSkill;					// 技能列表
	package_list<DWORD>				m_listSkillCoolDown;		// 冷却技能列表
	CombatHandler				m_CombatHandler;			// 当前正在施展的技能
	INT							m_nProduceSkillNum;			// 当前身上生产技能的数量

	package_map<DWORD, package_list<DWORD>*>	m_mapBuffModifier;			// 被动技能对buff的影响列表
	package_map<DWORD, package_list<DWORD>*>	m_mapTriggerModifier;		// 被动技能对trigger的影响列表

	BOOL						m_bTriggerOff;			// 是否需要判断触发率

	//-------------------------------------------------------------------------------------
	// Buff
	//-------------------------------------------------------------------------------------
	Buff						m_Buff[MAX_BUFF_NUM];		// 当前身上的所有Buff
	package_map<DWORD, Buff*>			m_mapBuff;					// 当前身上的buff列表，注意：不能对该map进行轮询！！！
	package_list<DWORD>				m_listDestroyBuffID;		// 要删除的Buff列表
	
	DWORD						m_nRingBuffID;				//光环buff的ID
	std::vector<DWORD>			m_ListRingBuffTarget;		//光环buff影响目标

	BOOL						m_bDmgAbs;					//是否有伤害吸收
	BOOL						m_bDmgReturn;				//是否伤害变治疗

	//-------------------------------------------------------------------------------------
	// 拥有该玩家仇恨的怪物列表
	//-------------------------------------------------------------------------------------
	package_map<DWORD, DWORD>			m_mapEnmityCreature;

	//-------------------------------------------------------------------------------------
	// 可视的潜行状态单位列表
	//-------------------------------------------------------------------------------------
	package_list<DWORD>				m_listLurkUnit;

	//-------------------------------------------------------------------------------------
	// 其它
	//-------------------------------------------------------------------------------------
	INT							m_nHPMPRegainTickCountDown;				// 回血回蓝的倒计时
	INT							m_nVitalityRegainTickCountDown;			// 回活力的倒计时
	INT							m_nEnduranceRegainTickCountDown;		// 回持久力的倒计时
	INT							m_nInPrisonMoraleRegainTickCountDown;	// 道德值增长计时
	INT							m_nInjuryRegainTickCountDown;			// 恢复内伤倒计时
	INT							m_nRageRegainTickCountDown;				// 怒气更新倒计时
	INT							m_nPointRegainTickCountDown;			// 点数更新倒计时
	INT							m_nEnergyRegainTickCountDown;			// 能量更新倒计时
	INT							m_nFocusRegainTickCountDown;			// 集中点数倒计时


	//跟随的怪物
	DWORD						m_dwFlowUnit;
};




//----------------------------------------------------------------------------------
// 判断目标是否在自身周围
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInDistance(Unit& target, FLOAT fDis) 
{
	const Vector3& vSrc = GetCurPos();
	const Vector3& vDest = target.GetCurPos();

	FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

	return fDis * fDis >= fDistSq;
}

//----------------------------------------------------------------------------------
// 判断与目标的战斗距离是否满足，fDis为技能或物品的使用距离
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInCombatDistance(Unit& target, FLOAT fDis, FLOAT fDiffDis)
{
	const Vector3& vSrc = GetCurPos();
	const Vector3& vDest = target.GetCurPos();

	FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

	// 战斗距离要加上双方各自的半径，再加上一个增量，以避免网络延迟
	FLOAT fRealDist = fDis + fDiffDis;

	return fRealDist * fRealDist >= fDistSq;
}

//----------------------------------------------------------------------------------
// 判断自己是否在目标身前，否则就在身后
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInFrontOfTarget(Unit& target)
{
	Vector3& vSelfPos = GetCurPos();
	Vector3& vTargetPos = target.GetCurPos();

	// 求出目标面朝方向矢量A
	Vector3 vTargetFace = target.GetFaceTo();

	// 求出目标到自己的矢量B
	Vector3 vDir = vSelfPos - vTargetPos;

	// 它们的点积等于|A|*|B|*cosθ,cosθ为非负表示夹角小于等于90度
	// 即表示我们在目标正面
	if( vTargetFace.x * vDir.x + vTargetFace.z * vDir.z >= 0 )
		return TRUE;
	else
		return FALSE;
}


//-----------------------------------------------------------------------------------
// 读取当前属性
//-----------------------------------------------------------------------------------
inline INT Unit::GetAttValue(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nAtt[nIndex];
}

//-----------------------------------------------------------------------------------
// 读取基础属性
//-----------------------------------------------------------------------------------
inline INT Unit::GetBaseAttValue(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nBaseAtt[nIndex];
}

//-----------------------------------------------------------------------------------
// 读取属性平直加成
//-----------------------------------------------------------------------------------
inline INT Unit::GetAttModValue(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nAttMod[nIndex];
}

//-----------------------------------------------------------------------------------
// 读取属性百分比加成
//-----------------------------------------------------------------------------------
inline INT Unit::GetAttModValuePct(INT nIndex)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	return m_nAttModPct[nIndex];
}

//------------------------------------------------------------------------------------
// 设置当前属性
//------------------------------------------------------------------------------------
inline VOID Unit::SetAttValue(INT nIndex, INT nValue, BOOL bSendMsg)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAtt[nIndex] = nValue;

	// 取上下限
	if( m_nAtt[nIndex] < AttRes::GetInstance()->GetAttMin(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMin(nIndex);
	if( m_nAtt[nIndex] > AttRes::GetInstance()->GetAttMax(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMax(nIndex);

	OnAttChange(nIndex);

	if( bSendMsg ) SendAttChange(nIndex);
}

//-----------------------------------------------------------------------------------
// 设置基本属性
//-----------------------------------------------------------------------------------
inline VOID Unit::SetBaseAttValue(INT nIndex, INT nValue)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nBaseAtt[nIndex] = nValue;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// 设置属性平直加成
//-----------------------------------------------------------------------------------
inline VOID Unit::SetAttModValue(INT nIndex, INT nValue)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAttMod[nIndex] = nValue;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// 设置属性百分比加成
//-----------------------------------------------------------------------------------
inline VOID Unit::SetAttModValuePct(INT nIndex, INT nValuePct)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAttModPct[nIndex] = nValuePct;
	SetAttRecalFlag(nIndex);
}

//------------------------------------------------------------------------------------
// 修改当前属性
//------------------------------------------------------------------------------------
inline VOID Unit::ModAttValue(INT nIndex, INT nValueMod, BOOL bSendMsg)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );

	INT nOriginValue = m_nAtt[nIndex];		// 得到初始值

	m_nAtt[nIndex] += nValueMod;			// 设置当前值

	// 取上下限
	if( m_nAtt[nIndex] < AttRes::GetInstance()->GetAttMin(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMin(nIndex);
	if( m_nAtt[nIndex] > AttRes::GetInstance()->GetAttMax(nIndex) ) m_nAtt[nIndex] = AttRes::GetInstance()->GetAttMax(nIndex);

	OnAttChange(nIndex);
	OnAttChange(nIndex, nValueMod);

	// 如果数值确实改变并且需要发送消息则发送消息
	if( nOriginValue != m_nAtt[nIndex] && bSendMsg ) SendAttChange(nIndex);
}

//-----------------------------------------------------------------------------------
// 修改基础属性
//-----------------------------------------------------------------------------------
inline VOID Unit::ModBaseAttValue(INT nIndex, INT nValueMod)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nBaseAtt[nIndex] += nValueMod;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// 修改属性平直加成
//-----------------------------------------------------------------------------------
inline VOID Unit::ModAttModValue(INT nIndex, INT nValueMod)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	if (nValueMod == 0) return;
	m_nAttMod[nIndex] += nValueMod;
	SetAttRecalFlag(nIndex);
}

//-----------------------------------------------------------------------------------
// 修改属性百分比加成
//-----------------------------------------------------------------------------------
inline VOID Unit::ModAttModValuePct(INT nIndex, INT nValuePctMod)
{
	ASSERT( nIndex >= 0 && nIndex < ERA_End );
	m_nAttModPct[nIndex] += nValuePctMod;
	SetAttRecalFlag(nIndex);
}

//----------------------------------------------------------------------------------
// 属性重新计算标识位设置
//----------------------------------------------------------------------------------
inline VOID Unit::SetAttRecalFlag(INT nIndex)
{
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );
	m_bAttRecalFlag[nIndex] = true;
	if( !m_bNeedRecal ) m_bNeedRecal = TRUE;
}

//----------------------------------------------------------------------------------
// 得到某个属性是否需要重算
//----------------------------------------------------------------------------------
inline bool Unit::GetAttRecalFlag(INT nIndex)
{
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );
	return m_bAttRecalFlag[nIndex];
}

//----------------------------------------------------------------------------------
// 清除属性重算标志位
//----------------------------------------------------------------------------------
inline VOID Unit::ClearAttRecalFlag()
{
	ZeroMemory(m_bAttRecalFlag, sizeof(m_bAttRecalFlag));
	m_bNeedRecal = FALSE;
}

//----------------------------------------------------------------------------------
// 本地属性与远程属性映射
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
// 根据技能伤害类型来确定对应的人物伤害减免类型
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
// 根据Buff的抗性类型得到对应的人物抗性属性
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
// 检查某个属性是不是队友同步的属性
//----------------------------------------------------------------------------------------
inline BOOL Unit::IsTeamRemoteAtt(ERemoteRoleAtt eType)
{
	if(eType == ERRA_MaxHP || eType == ERRA_HP || eType == ERRA_MaxMP || eType == ERRA_MP)
		return TRUE;
	else
		return FALSE;
}


//-----------------------------------------------------------------------------------------
// 改变真气
//-----------------------------------------------------------------------------------------
inline VOID Unit::ChangeMP(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_MP, nValue);

	if( nValue < 0 )
	{
		OnInterruptBuffEvent(EBIF_MPLower);		// 检测Buff事件
	}
}

//-----------------------------------------------------------------------------------------
// 改变爱心
//-----------------------------------------------------------------------------------------
inline VOID Unit::ChangeRage(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_Love, nValue);

	if( nValue < 0 )
	{
		OnInterruptBuffEvent(EBIF_RageLower);		// 检测Buff事件
	}
}

//-----------------------------------------------------------------------------------------
// 改变义气
//-----------------------------------------------------------------------------------------
//inline VOID Unit::ChangeVitality(INT nValue)
inline VOID Unit::ChangeBrotherhood(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_Brotherhood, nValue);

	//if( nValue < 0 )
	//{
	//	OnInterruptBuffEvent(EBIF_VitalityLower);		// 检测Buff事件
	//}
}

//-----------------------------------------------------------------------------------------
// 改变武魂
//-----------------------------------------------------------------------------------------
inline VOID Unit::ChangeWuhuen(INT nValue)
{
	if( 0 == nValue ) return;

	ModAttValue(ERA_Wuhuen, nValue);

	//if( nValue < 0 )
	//{
	//	OnInterruptBuffEvent(EBIF_EnduranceLower);		// 检测Buff事件
	//}
}

//-----------------------------------------------------------------------------------------
// 设置状态
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
// 重置状态
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
// 当状态被设置时
//-----------------------------------------------------------------------------------------
inline VOID Unit::OnSetState(EState eState)
{
	switch(eState)
	{
		// 眩晕
	case ES_Dizzy:
		{
			GetMoveData().StopMoveForce();	// 强制停止移动
			GetCombatHandler().End();		// 停止当前攻击
		}
		break;

		// 定身
	case ES_Tie:
		{
			GetMoveData().StopMoveForce();	// 强制停止移动
		}
		break;

		// 昏睡
	case ES_Spor:
		{
			GetMoveData().StopMoveForce();	// 强制停止移动
			GetCombatHandler().End();		// 停止当前攻击
		}
		break;

		// 无敌 
	case ES_Invincible:
		{
			RemoveAllBuff(FALSE);			// 清除所有自身debuff
		}
		break;

		// 隐身
	case ES_Lurk:
		{
			
		}
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------------------
// 当状态被重置时
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
// 更新
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
// 清空技能CD
//----------------------------------------------------------------------------------
inline VOID Unit::ClearAllSkillCoodDown(ETalentType eType, DWORD dwExceptSkillID)
{
	if( m_listSkillCoolDown.empty() ) return;

	if( ETT_Null == eType )
	{
		// 所有技能冷却
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

		// 某类天资技能冷却
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
// 将所有Buff写入外部消息
//----------------------------------------------------------------------------------
inline VOID Unit::GetAllBuffMsgInfo(tagBuffMsgInfo* const pBuffInfo, INT nMaxNum)
{
	if( nMaxNum <= 0 ) return;

	INT nIndex = 0;	// 写入的Index

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
// 潜行
//----------------------------------------------------------------------------------
inline VOID Unit::Lurk(INT nLurkLevel)
{
	Map *pMap = get_map();
	if( !VALID_POINT(pMap) )
	{
		return;
	}
	
	// 设置单位状态
	SetState(ES_Lurk);

	// 潜行等级设置//??

	// 同步
	pMap->lurk(this);
}

//----------------------------------------------------------------------------------
// 脱离潜行状态
//----------------------------------------------------------------------------------
inline VOID Unit::UnLurk()
{
	Map *pMap = get_map();
	if( !VALID_POINT(pMap) )
	{
		return;
	}

	// 设置单位状态
	UnsetState(ES_Lurk);

	// 同步
	if( !IsInState(ES_Lurk) )
	{
		pMap->unlurk(this);
	}
}

//----------------------------------------------------------------------------------
// 是否在可视潜行单位列表
//----------------------------------------------------------------------------------
inline BOOL Unit::IsInVisList(DWORD dwUnitID)
{
	return m_listLurkUnit.is_exist(dwUnitID);
}

//----------------------------------------------------------------------------------
// 加入可视潜行单位列表
//----------------------------------------------------------------------------------
inline VOID Unit::Add2VisList(DWORD dwUnitID)
{
	ASSERT(!m_listLurkUnit.is_exist(dwUnitID));

	m_listLurkUnit.push_back(dwUnitID);
}

//----------------------------------------------------------------------------------
// 从可视潜行单位列表中剔除
//----------------------------------------------------------------------------------
inline VOID Unit::RemoveFromVisList(DWORD dwUnitID)
{
	ASSERT(m_listLurkUnit.is_exist(dwUnitID));
	
	m_listLurkUnit.erase(dwUnitID);
}
