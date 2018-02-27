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
*	@brief		非玩家生物类,怪物,npc,宠物,地物
*/

#pragma once

#include "unit.h"

class Unit;
class Role;
class AIController;
class Skill;
class Team;
class CreatureScript;
class PathFinder;
class NPCTeam;
struct tagCreatureProto;
struct tagCreatureAI;
struct tagMapCreatureInfo;
struct tagPlantData;

//-----------------------------------------------------------------------------------
// 常量
//-----------------------------------------------------------------------------------
const INT	CREATURE_PATROL_REST_TICK_MIN				=	5 * TICK_PER_SECOND;		// 生物巡逻过程中休息的最小时间——5秒
const INT	CREATURE_PATROL_REST_TICK_INTERVAL			=	10 * TICK_PER_SECOND;		// 生物巡逻过程中休息的最大差——10秒
const INT	CREATURE_LOOK_FOR_ENEMY_REST_TICK_MIN		=	1 * TICK_PER_SECOND;		// 生物锁敌的间隔的最小时间——1秒
const INT	CREATURE_LOOK_FOR_ENEMY_REST_TICK_INTERVAL	=	2 * TICK_PER_SECOND;		// 生物索敌的间隔的最大时间差——2秒
const INT	CREATURE_CHECK_AI_EVENT_TICK				=	5;							// 生物检查AI事件的时间间隔——5个心跳
const INT	CREATURE_RETURN_TICK						=	10 * TICK_PER_SECOND;		// 生物在多少秒仇恨没有改变的情况下返回——10秒
const INT	CREATURE_LOCK_TARGET_MIN_TICK				=	6 * TICK_PER_SECOND;		// 生物攻击时锁定目标的最小时间——6秒
const INT	CREATURE_LOCK_TARGET_TICK_INTERVAL			=	4 * TICK_PER_SECOND;		// 生物攻击时锁定目标的最大时间——4秒
const INT	CREATURE_CALLHELP_TICK						=	4 * TICK_PER_SECOND;		// 怪物求助的时间 -- 4秒

const FLOAT	MAX_NPC_TALK_DISTANCE						=	20 * 50.0f;					// 与NPC说话的最小距离为1000个世界单位(20个格子)
const FLOAT	CREATURE_PURSUE_NEAR_DIST					=	50.0f;						// 怪物追击到目标附近的距离
const INT	CREATURE_SPACE_OUT_MIN						=	15 * TICK_PER_SECOND;		// 触发怪物与玩家拉开距离行为的最短时间间隔
const INT	CREATURE_SPACE_OUT_MAX						=	30 * TICK_PER_SECOND;		// 触发怪物与玩家拉开距离行为的最长时间间隔
const FLOAT CREATURE_SPACE_OUT_DIST						=	16 * 50.0f;					// 拉开的距离
const FLOAT CREATURE_SPACE_OUT_CHANGE_DIST				=	4 * 50.0f;					// 小于该距离时触发拉开距离行为

const INT	CREATURE_ONTALE_PAUSE_TIME					=	10 * TICK_PER_SECOND;		// 怪物被说话时,巡逻暂停时间
//-----------------------------------------------------------------------------------
// 怪物的生成方式
//-----------------------------------------------------------------------------------
enum ECreatureSpawnedType
{
	ECST_Load			=	0,			// 地图内摆放加载生成
	ECST_Nest			=	1,			// 巢穴生成
	ECST_Dynamic		=	2,			// 动态生成
	ECST_SpawnPoint		=	3,			// 刷怪点生成
	ECST_ByRole			=	4,			// 由角色生成
	ECST_Team			=	5,			// 小队队长生成
	ECST_Guild			=   6,			// 帮会生成
	ECST_Maidan			=   7,			// 帮会兵工厂生成
	ECST_GuildSentinel	=	8,			// 帮会守卫生成	
	ECST_SpawnList		=	9,			// 刷怪点组生成
};

//----------------------------------------------------------------------------------
// 怪物复活结果
//----------------------------------------------------------------------------------
enum ECreatureReviveResult
{
	ECRR_Success		=	0,			// 重生成功
	ECRR_Failed			=	1,			// 重生失败
	ECRR_NeedDestroy	=	2,			// 动态生成，需要删除内存
	ECRR_NeedReplace	=	3,			// 刷新点怪，需要替换
	ECRR_NeedRepos		=	4,			// 刷怪点组, 需要重新生成位置
};

//-----------------------------------------------------------------------------
// 生物类
//-----------------------------------------------------------------------------
class Creature : public Unit, public ScriptData<ESD_Creature>
{
	friend class AIController;

public:
	//-------------------------------------------------------------------------
	// 静态函数，生成怪物
	//-------------------------------------------------------------------------
	static Creature* Create(DWORD dwID, DWORD dwMapID, Map* pMap, const tagCreatureProto* pProto, 
									Vector3& vPos, Vector3& vFaceTo, ECreatureSpawnedType eSpawnedType, 
									DWORD dwSpawnerID, DWORD dwSpawnGroupID, BOOL bCollide, const tag_map_way_point_info_list* patrolList, 
									DWORD dwTeamID=INVALID_VALUE, DWORD dwGuildID = INVALID_VALUE);

	static VOID		Delete(Creature* &pCreature);

	//--------------------------------------------------------------------------
	// Constructor&Destructor
	//--------------------------------------------------------------------------
protected:
	Creature(DWORD dwID, DWORD dwMapID, const Vector3& vPos, const Vector3& vFaceTo, ECreatureSpawnedType eSpawnedType, 
			 DWORD dwSpawnerID, DWORD dwSpawnGroupID, BOOL bCollide, DWORD dwTeamID=INVALID_VALUE, DWORD dwGuildID = INVALID_VALUE);
	virtual ~Creature();
public:
	//--------------------------------------------------------------------------
	// 初始化和销毁函数
	//--------------------------------------------------------------------------
	BOOL					Init(const tagCreatureProto* pProto, const tag_map_way_point_info_list* patrolList);
	VOID					Destroy();

	VOID					SetDelMap(BOOL	bDel) { m_bDelMap = bDel; }
	BOOL					IsDelMap() { return m_bDelMap; }
	
	VOID					OnScriptLoad();

	// 重置朝向
	VOID					ResetFaceTo();
	//--------------------------------------------------------------------------
	// 所属相关
	//--------------------------------------------------------------------------
	BOOL					IsTagged() const			{ return m_bTagged; }
	VOID					SetTagged(DWORD dwOwner);

	//--------------------------------------------------------------------------
	// 小队相关
	//--------------------------------------------------------------------------
	DWORD					GetTeamID()					{ return m_dwTeamID; }
	VOID					SetTeamID(DWORD dwID)		{ m_dwTeamID = dwID; }
	NPCTeam*				GetTeamPtr();

	//--------------------------------------------------------------------------
	// 得到属性
	//--------------------------------------------------------------------------
	const tagCreatureProto*	GetProto() const		{ return m_pProto; }
	AIController*			GetAI()	const			{ return m_pAIController; }
	ECreatureType			GetType() const			{ return m_pProto->eType; }
	DWORD					GetSpawnPtID() const	{ return m_dwSpawnerID;}
	ECreatureSpawnedType    GetSpawnType() const	{ return m_eSpawnedType; }
	DWORD					GetSpawnGroupID() const	{ return m_dwSpawnGroupID; }
	VOID					SetSpawnPtID(DWORD dwSpawnerID) { m_dwSpawnerID = dwSpawnerID; }
	VOID					SetSpawnGroupID(DWORD dwSpawnGroupID) { m_dwSpawnGroupID = dwSpawnGroupID; }
	ECreatureTaggedType		GetTaggedType()	const	{ return m_pProto->eTaggedType; }
	DWORD					GetTaggedOwner() const	{ return m_dwTaggedOwner; }
	DWORD					GetLootID()	const		{ return m_pProto->dwLootID; } 
	DWORD					GetShopID() const		{ return m_pProto->uFunctionID.dwShopID; }
	DWORD					GetDakID()	const		{ return m_pProto->uFunctionID.dwDakID; }
	
	VOID					SetBronPos(const Vector3& pos) { m_vBornPos = pos; }
	const Vector3&			GetBornPos() const		{ return m_vBornPos; }
	const Vector3&			GetBornFace() const		{ return m_vBornFace; }

	//-----------------------------------------------------------------------------
	// 类型判断
	//-----------------------------------------------------------------------------
	BOOL					IsMonster()	const		{ return m_eCreatureType == ECT_Monster; }
	BOOL					IsNPC()	const			{ return m_eCreatureType == ECT_NPC; }
	BOOL					IsGameObject() const	{ return GetType() == ECT_GameObject; }
	BOOL					IsPet()	const			{ return GetType() == ECT_Pet; }

	BOOL					IsNormalMonster() const	{ return IsMonster() && (EMTT_Normal == m_pProto->nType2 || EMTT_Team == m_pProto->nType2); }
	BOOL					IsNest() const			{ return IsMonster() && EMTT_Nest == m_pProto->nType2; }
	BOOL					IsTeam() const			{ return IsMonster() && EMTT_Team == m_pProto->nType2; }
	BOOL					IsBoss() const			{ return IsNormalMonster() && (m_pProto->is_boss()); }

	BOOL					IsRes()	const			{ return IsGameObject() && EGOT_Gather == m_pProto->nType2; }
	BOOL					IsInves() const			{ return IsGameObject() && (EGOT_QuestInves == m_pProto->nType2 || 
															EGOT_CommonInves == m_pProto->nType2 ); }

	BOOL					IsInves_two() const			{ return IsGameObject() && EGOT_CommonInves_T == m_pProto->nType2; }

	BOOL					IsGuildInves() const	{ return IsGameObject() && (EGOT_Relive == m_pProto->nType2); }
	BOOL                    IsDoor() const          { return IsGameObject() && EGOT_Door == m_pProto->nType2; }

	BOOL					IsNatuRes()	const		{ return IsRes() && (EGT_Mine == m_pProto->nType3); }
	BOOL					IsGuildNatuRes() const	{ return IsRes() && (EGT_Guild_Mine == m_pProto->nType3); }
	BOOL					IsManRes()	const		{ return IsRes() && (EGT_Herb == m_pProto->nType3); }
	BOOL					IsGuildManRes() const	{ return IsRes() && (EGT_Guild_Herb == m_pProto->nType3); }
	BOOL					IsCarry() const			{ return IsInves() && (EGT_Carry == m_pProto->nType3); }

	//-----------------------------------------------------------------------------
	// NPC职能
	//-----------------------------------------------------------------------------
	BOOL					IsFunctionNPC(EFunctionNPCType eFuncType) const	{ return eFuncType == m_pProto->eFunctionType; }

	BOOL					IsVipPriceOff( ) const { return m_pProto->bVipPriceOff; }

	//-----------------------------------------------------------------------------
	// 碰撞
	//-----------------------------------------------------------------------------
	BOOL					NeedCollide() const			{ return m_bCollide; }
	VOID					SetCollide(BOOL bCollide)	{ m_bCollide = bCollide; }

	//-----------------------------------------------------------------------------
	// 脚本
	//-----------------------------------------------------------------------------
	const CreatureScript*	GetScript() const		{ return m_pScript; }

	//--------------------------------------------------------------------------
	// 更新
	//--------------------------------------------------------------------------
	virtual VOID			Update();

	//--------------------------------------------------------------------------
	// 得到属性ID，宠物需要多态实现
	//--------------------------------------------------------------------------
	virtual DWORD			GetTypeID()						{ return m_pProto->dw_data_id; }

	//--------------------------------------------------------------------------
	// 得到性别，玩家和生物走的不是同一套
	//--------------------------------------------------------------------------
	virtual BYTE			GetSex() const					{ return m_pProto->bySex; }

	// 获取帮派ID
	DWORD					GetGuildID() const				{ return m_dwGuildID; }
	VOID					SetGuildID(DWORD dwGuildID)		{ m_dwGuildID = dwGuildID; }					

	//--------------------------------------------------------------------------
	// 技能
	//--------------------------------------------------------------------------
	//Skill*					GetMeleeSkill()					{ return m_pMeleeSkill; }
	//Skill*					GetRangedSkill()				{ return m_pRangedSkill; }
	
	Skill*					GetNormalSKill(INT index)		{ if(m_pNormalSkill.size() <= 0 || index > m_pNormalSkill.size()) return NULL/*ASSERT(index>=0&&index<m_pNormalSkill.size())*/; return m_pNormalSkill[index]; }
	INT						GetNormalNumber() { return m_pNormalSkill.size(); }
	//--------------------------------------------------------------------------
	// 状态相关
	//--------------------------------------------------------------------------
	virtual BOOL			IsInStateCantMove()				{ return Unit::IsInStateCantMove() || GetCombatHandler().IsValid(); }
	virtual BOOL			IsInStateCantCastSkill()		{ return Unit::IsInStateCantCastSkill() || (FALSE == m_pProto->bCanAttack); }
	virtual BOOL			IsInStateCantBeSkill()			{ return !m_pProto->bCanBeAttack; }
	virtual BOOL			IsInStateInvisible()			{ return Unit::IsInStateInvisible() || (FALSE == m_pProto->bVisble); }
	//BOOL					IsCanBeAttack()					{ return m_bCanBeAttack; }
	//VOID					SetCanBeAttack(BOOL b);		
	ECreatureType			GetCreatureType()				{ return m_eCreatureType; }
	VOID					SetCreatureType(ECreatureType et);
	//--------------------------------------------------------------------------
	// 距离相关
	//--------------------------------------------------------------------------
	BOOL					IsExpShareDistance(Role* pReward);
	BOOL					IsLootShareDistance(Role* pReward);
	INT						CalExpShareRole(const Team* pTeam, std::vector<Role*>  &vecRole);
	BOOL					CheckNPCTalkDistance(Role* pRole);

	//--------------------------------------------------------------------------
	// 战斗状态相关
	//--------------------------------------------------------------------------
	virtual VOID			OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited);
	virtual VOID			OnDead(Unit* pSrc, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=0, BOOL bCrit = FALSE);
	virtual VOID			OnKill(Unit* pTarget);
	VOID					OnInvest(Role* pSrc);
	VOID					OnDisappear();
	VOID					OnAIAttack();
	VOID					OnBuffInjury(Unit* pSrc, INT dmg);
	
	//--------------------------------------------------------------------------
	// 战斗判定
	//--------------------------------------------------------------------------
	virtual DWORD			TargetTypeFlag(Unit* pTarget);
	virtual DWORD			FriendEnemy(Unit* pTarget);

	//--------------------------------------------------------------------------
	// 战斗辅助
	//--------------------------------------------------------------------------
	INT						GetAroundCreature(std::vector<DWORD> &vecCreature, FLOAT fOPRadius, FLOAT fHigh);
	INT						GetAroundRole(std::vector<DWORD> &vecRole, FLOAT fOPRadius, FLOAT fHigh);	

	//--------------------------------------------------------------------------
	// 复活相关
	//--------------------------------------------------------------------------
	ECreatureReviveResult	TryRevive();
	VOID					OnRevive();
	ECreatureRespawnType	GetRespawnType()					{ return m_pProto->eRespawnType; }
	INT						GetRespawnTick()					{ return get_tool()->rand_in_range(m_pProto->nRespawnTick, m_pProto->nRespawnTick + m_pProto->nRespawnTickAdd);}
	//INT						GetRespawnCountDown()				{ return m_nRespawnTickCountDown; }
	//VOID					SetRespawnCountDown(INT nTick)		{ m_nRespawnTickCountDown = nTick; }
	//VOID					CountDownRespawnTick()				{ m_nRespawnTickCountDown -= 1; }

	//--------------------------------------------------------------------------
	// 表现相关
	//--------------------------------------------------------------------------
	VOID					Say(DWORD dwSayID);
	VOID					PlayerAction(DWORD dwActionID);

	//-------------------------------------------------------------------------
	// 战备状态
	//-------------------------------------------------------------------------
	//void					SetZhanbei( bool bZhanbei )			{ m_bBeiZhan = bZhanbei; }
	VOID					OnQuestEndbyPath( );
	VOID					OnReachEndPath( );

	//-------------------------------------------------------------------------
	// 副本限制
	//-------------------------------------------------------------------------
	VOID			SetInstLimit(Unit* pSrc, INT nPoint);
	VOID			SetInstProcess();
	VOID			SaveRoleProcess();
	
	//VOID					SetUsed(BOOL bUsed) { m_bUsed = bUsed; }
	//BOOL					IsUsed() { return m_bUsed; }

	//INT						GetPlantDataIndex() { return n_guildPlantIndex;}
	//VOID					SetPlantDataIndex(INT nIndex) { n_guildPlantIndex = nIndex; }
	
	//DWORD					GetTuDui() { return m_dwTuDui; }
	//VOID					SetTuDui(DWORD dwid) { m_dwTuDui = dwid;}

	//DWORD					GetMaxYield() { return m_dwMaxTield; }
	//VOID					SetMaxYield(DWORD dwYield) { m_dwMaxTield = dwYield; }

	//DWORD					GetCurYield();
	//VOID					GetPlantRoleName(TCHAR* szName);

	//DWORD					GetPlantTime();

	//VOID					SetPlantData(tagPlantData* pData){ m_pPlantdata = pData; }
	//tagPlantData*			GetPlantData();
	//VOID					ResetPlantData();
	// 是否成熟
	//BOOL					IsMatrue();
	// 是否自动收获
	//BOOL					IsAotoReceive();

	Role*					GetMarster();
	DWORD					GetMarsterID();
	//VOID                    SetVisible(BOOL val, DWORD delay = 0) { m_bVisibleSet = val;  m_dwVisibleDelayTick = delay; }
private:
	//--------------------------------------------------------------------------
	// 初始化
	//--------------------------------------------------------------------------
	BOOL					InitAtt(const tagCreatureProto* pProto);
	BOOL					InitAI(const tagCreatureProto* pProto, const tag_map_way_point_info_list* patrolList);

	//-------------------------------------------------------------------------
	// 计算初始生物当前属性
	//--------------------------------------------------------------------------
	VOID					CalInitAtt();

	//--------------------------------------------------------------------------
	// 复活时的刷新函数
	//--------------------------------------------------------------------------
	VOID					RefreshAtt();
	VOID					RefreshAI();

	//--------------------------------------------------------------------------
	// 各种更新
	//--------------------------------------------------------------------------
	VOID					UpdateAI();
	VOID					UpdateLiveTime();
	VOID					UpdateCombat();
	VOID					UpdateDoor();

	//--------------------------------------------------------------------------
	// 属性改变
	//--------------------------------------------------------------------------
	virtual VOID			OnAttChange(INT nIndex);
	virtual VOID			OnAttChange(bool bRecalFlag[ERA_End]);

	//--------------------------------------------------------------------------
	// 战斗判定
	//--------------------------------------------------------------------------
	DWORD					TargetTypeFlag(Role* pTarget);
	DWORD					TargetTypeFlag(Creature* pTarget);
	DWORD					FriendEnemy(Role* pTarget);
	DWORD					FriendEnemy(Creature* pTarget);

	//--------------------------------------------------------------------------
	// 奖励
	//--------------------------------------------------------------------------
	Role*					FindRewardRole(Unit* pSrc);
	BOOL					ExpReward(Role* pReward, vector<Role*> &team_vec,BOOL bKill=FALSE, const Team *pTeam=(Team*)INVALID_VALUE, INT nShareNum=1,bool bSelf=true);
	FLOAT					CalLevelCoefficient(INT nRoleLevel, INT nCreatureLevel);
	
private:
	//---------------------------------------------------------------------------
	// 基本属性
	//---------------------------------------------------------------------------
	const tagCreatureProto*					m_pProto;					// 静态属性
	AIController*							m_pAIController;			// AI控制器
	BOOL									m_bTagged;					// 怪物是否已被所有
	DWORD									m_dwTaggedOwner;			// 所有者

	Vector3									m_vBornPos;					// 出生点
	Vector3									m_vBornFace;				// 出生朝向
	BOOL									m_bCollide;					// 是否计算碰撞
	
	INT										m_nLiveTick;				// 生存时间
	INT										m_nRespawnTickCountDown;	// 死亡复活倒计时

	ECreatureSpawnedType					m_eSpawnedType;				// 生成方式（加载，巢穴，动态）
	DWORD									m_dwSpawnerID;				// 如果是巢穴或者动态创建的，则记录创建者ID（世界ID），如果是刷新点创建的，则记录刷新点id
	DWORD									m_dwSpawnGroupID;	
	//BOOL									m_bCanBeAttack;				// 是否可被攻击

	ECreatureType							m_eCreatureType;			// 单位类型
	//----------------------------------------------------------------------------
	// 脚本
	//----------------------------------------------------------------------------
	const CreatureScript*					m_pScript;

	BOOL                                    m_bDoorOpened;              // 如果是门类型 则为开启状态

	//----------------------------------------------------------------------------
	// 怪物小队
	//----------------------------------------------------------------------------
	DWORD									m_dwTeamID;				// 怪物小队ID
	//bool									m_bBeiZhan;
	bool									m_bDelSelf;					// 删除自己

	//----------------------------------------------------------------------------
	// 帮会ID
	//----------------------------------------------------------------------------
	DWORD									m_dwGuildID;			// 

	//----------------------------------------------------------------------------
	// 怪物技能
	//----------------------------------------------------------------------------
	//Skill*									m_pMeleeSkill;			// 近身攻击普通技能
	//Skill*									m_pRangedSkill;			// 远程攻击普通技能
		
	std::vector<Skill*>						m_pNormalSkill;	
	//Skill*									m_pNormalSkill[NORAML_SKILL_NUMBER];

	INT32									m_DoorOpenTime;			// 门开启时间

	DWORD									m_DoorObjID;

	BOOL									m_bDelMap;				// 是否需要从Map中删除

	// 种植数据
	//BOOL									m_bUsed;				// 是否被使用
	//INT										n_guildPlantIndex;		// 所属种植数据索引
	//DWORD									m_dwTuDui;				// 植物对应的土堆
	//DWORD									m_dwMaxTield;			// 最大产量
	//tagPlantData*							m_pPlantdata;			
};


