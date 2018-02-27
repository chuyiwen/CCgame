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
*	@brief		�����������,����,npc,����,����
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
// ����
//-----------------------------------------------------------------------------------
const INT	CREATURE_PATROL_REST_TICK_MIN				=	5 * TICK_PER_SECOND;		// ����Ѳ�߹�������Ϣ����Сʱ�䡪��5��
const INT	CREATURE_PATROL_REST_TICK_INTERVAL			=	10 * TICK_PER_SECOND;		// ����Ѳ�߹�������Ϣ�������10��
const INT	CREATURE_LOOK_FOR_ENEMY_REST_TICK_MIN		=	1 * TICK_PER_SECOND;		// �������еļ������Сʱ�䡪��1��
const INT	CREATURE_LOOK_FOR_ENEMY_REST_TICK_INTERVAL	=	2 * TICK_PER_SECOND;		// �������еļ�������ʱ����2��
const INT	CREATURE_CHECK_AI_EVENT_TICK				=	5;							// ������AI�¼���ʱ��������5������
const INT	CREATURE_RETURN_TICK						=	10 * TICK_PER_SECOND;		// �����ڶ�������û�иı������·��ء���10��
const INT	CREATURE_LOCK_TARGET_MIN_TICK				=	6 * TICK_PER_SECOND;		// ���﹥��ʱ����Ŀ�����Сʱ�䡪��6��
const INT	CREATURE_LOCK_TARGET_TICK_INTERVAL			=	4 * TICK_PER_SECOND;		// ���﹥��ʱ����Ŀ������ʱ�䡪��4��
const INT	CREATURE_CALLHELP_TICK						=	4 * TICK_PER_SECOND;		// ����������ʱ�� -- 4��

const FLOAT	MAX_NPC_TALK_DISTANCE						=	20 * 50.0f;					// ��NPC˵������С����Ϊ1000�����絥λ(20������)
const FLOAT	CREATURE_PURSUE_NEAR_DIST					=	50.0f;						// ����׷����Ŀ�긽���ľ���
const INT	CREATURE_SPACE_OUT_MIN						=	15 * TICK_PER_SECOND;		// �����������������������Ϊ�����ʱ����
const INT	CREATURE_SPACE_OUT_MAX						=	30 * TICK_PER_SECOND;		// �����������������������Ϊ���ʱ����
const FLOAT CREATURE_SPACE_OUT_DIST						=	16 * 50.0f;					// �����ľ���
const FLOAT CREATURE_SPACE_OUT_CHANGE_DIST				=	4 * 50.0f;					// С�ڸþ���ʱ��������������Ϊ

const INT	CREATURE_ONTALE_PAUSE_TIME					=	10 * TICK_PER_SECOND;		// ���ﱻ˵��ʱ,Ѳ����ͣʱ��
//-----------------------------------------------------------------------------------
// ��������ɷ�ʽ
//-----------------------------------------------------------------------------------
enum ECreatureSpawnedType
{
	ECST_Load			=	0,			// ��ͼ�ڰڷż�������
	ECST_Nest			=	1,			// ��Ѩ����
	ECST_Dynamic		=	2,			// ��̬����
	ECST_SpawnPoint		=	3,			// ˢ�ֵ�����
	ECST_ByRole			=	4,			// �ɽ�ɫ����
	ECST_Team			=	5,			// С�Ӷӳ�����
	ECST_Guild			=   6,			// �������
	ECST_Maidan			=   7,			// ������������
	ECST_GuildSentinel	=	8,			// �����������	
	ECST_SpawnList		=	9,			// ˢ�ֵ�������
};

//----------------------------------------------------------------------------------
// ���︴����
//----------------------------------------------------------------------------------
enum ECreatureReviveResult
{
	ECRR_Success		=	0,			// �����ɹ�
	ECRR_Failed			=	1,			// ����ʧ��
	ECRR_NeedDestroy	=	2,			// ��̬���ɣ���Ҫɾ���ڴ�
	ECRR_NeedReplace	=	3,			// ˢ�µ�֣���Ҫ�滻
	ECRR_NeedRepos		=	4,			// ˢ�ֵ���, ��Ҫ��������λ��
};

//-----------------------------------------------------------------------------
// ������
//-----------------------------------------------------------------------------
class Creature : public Unit, public ScriptData<ESD_Creature>
{
	friend class AIController;

public:
	//-------------------------------------------------------------------------
	// ��̬���������ɹ���
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
	// ��ʼ�������ٺ���
	//--------------------------------------------------------------------------
	BOOL					Init(const tagCreatureProto* pProto, const tag_map_way_point_info_list* patrolList);
	VOID					Destroy();

	VOID					SetDelMap(BOOL	bDel) { m_bDelMap = bDel; }
	BOOL					IsDelMap() { return m_bDelMap; }
	
	VOID					OnScriptLoad();

	// ���ó���
	VOID					ResetFaceTo();
	//--------------------------------------------------------------------------
	// �������
	//--------------------------------------------------------------------------
	BOOL					IsTagged() const			{ return m_bTagged; }
	VOID					SetTagged(DWORD dwOwner);

	//--------------------------------------------------------------------------
	// С�����
	//--------------------------------------------------------------------------
	DWORD					GetTeamID()					{ return m_dwTeamID; }
	VOID					SetTeamID(DWORD dwID)		{ m_dwTeamID = dwID; }
	NPCTeam*				GetTeamPtr();

	//--------------------------------------------------------------------------
	// �õ�����
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
	// �����ж�
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
	// NPCְ��
	//-----------------------------------------------------------------------------
	BOOL					IsFunctionNPC(EFunctionNPCType eFuncType) const	{ return eFuncType == m_pProto->eFunctionType; }

	BOOL					IsVipPriceOff( ) const { return m_pProto->bVipPriceOff; }

	//-----------------------------------------------------------------------------
	// ��ײ
	//-----------------------------------------------------------------------------
	BOOL					NeedCollide() const			{ return m_bCollide; }
	VOID					SetCollide(BOOL bCollide)	{ m_bCollide = bCollide; }

	//-----------------------------------------------------------------------------
	// �ű�
	//-----------------------------------------------------------------------------
	const CreatureScript*	GetScript() const		{ return m_pScript; }

	//--------------------------------------------------------------------------
	// ����
	//--------------------------------------------------------------------------
	virtual VOID			Update();

	//--------------------------------------------------------------------------
	// �õ�����ID��������Ҫ��̬ʵ��
	//--------------------------------------------------------------------------
	virtual DWORD			GetTypeID()						{ return m_pProto->dw_data_id; }

	//--------------------------------------------------------------------------
	// �õ��Ա���Һ������ߵĲ���ͬһ��
	//--------------------------------------------------------------------------
	virtual BYTE			GetSex() const					{ return m_pProto->bySex; }

	// ��ȡ����ID
	DWORD					GetGuildID() const				{ return m_dwGuildID; }
	VOID					SetGuildID(DWORD dwGuildID)		{ m_dwGuildID = dwGuildID; }					

	//--------------------------------------------------------------------------
	// ����
	//--------------------------------------------------------------------------
	//Skill*					GetMeleeSkill()					{ return m_pMeleeSkill; }
	//Skill*					GetRangedSkill()				{ return m_pRangedSkill; }
	
	Skill*					GetNormalSKill(INT index)		{ if(m_pNormalSkill.size() <= 0 || index > m_pNormalSkill.size()) return NULL/*ASSERT(index>=0&&index<m_pNormalSkill.size())*/; return m_pNormalSkill[index]; }
	INT						GetNormalNumber() { return m_pNormalSkill.size(); }
	//--------------------------------------------------------------------------
	// ״̬���
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
	// �������
	//--------------------------------------------------------------------------
	BOOL					IsExpShareDistance(Role* pReward);
	BOOL					IsLootShareDistance(Role* pReward);
	INT						CalExpShareRole(const Team* pTeam, std::vector<Role*>  &vecRole);
	BOOL					CheckNPCTalkDistance(Role* pRole);

	//--------------------------------------------------------------------------
	// ս��״̬���
	//--------------------------------------------------------------------------
	virtual VOID			OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited);
	virtual VOID			OnDead(Unit* pSrc, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=0, BOOL bCrit = FALSE);
	virtual VOID			OnKill(Unit* pTarget);
	VOID					OnInvest(Role* pSrc);
	VOID					OnDisappear();
	VOID					OnAIAttack();
	VOID					OnBuffInjury(Unit* pSrc, INT dmg);
	
	//--------------------------------------------------------------------------
	// ս���ж�
	//--------------------------------------------------------------------------
	virtual DWORD			TargetTypeFlag(Unit* pTarget);
	virtual DWORD			FriendEnemy(Unit* pTarget);

	//--------------------------------------------------------------------------
	// ս������
	//--------------------------------------------------------------------------
	INT						GetAroundCreature(std::vector<DWORD> &vecCreature, FLOAT fOPRadius, FLOAT fHigh);
	INT						GetAroundRole(std::vector<DWORD> &vecRole, FLOAT fOPRadius, FLOAT fHigh);	

	//--------------------------------------------------------------------------
	// �������
	//--------------------------------------------------------------------------
	ECreatureReviveResult	TryRevive();
	VOID					OnRevive();
	ECreatureRespawnType	GetRespawnType()					{ return m_pProto->eRespawnType; }
	INT						GetRespawnTick()					{ return get_tool()->rand_in_range(m_pProto->nRespawnTick, m_pProto->nRespawnTick + m_pProto->nRespawnTickAdd);}
	//INT						GetRespawnCountDown()				{ return m_nRespawnTickCountDown; }
	//VOID					SetRespawnCountDown(INT nTick)		{ m_nRespawnTickCountDown = nTick; }
	//VOID					CountDownRespawnTick()				{ m_nRespawnTickCountDown -= 1; }

	//--------------------------------------------------------------------------
	// �������
	//--------------------------------------------------------------------------
	VOID					Say(DWORD dwSayID);
	VOID					PlayerAction(DWORD dwActionID);

	//-------------------------------------------------------------------------
	// ս��״̬
	//-------------------------------------------------------------------------
	//void					SetZhanbei( bool bZhanbei )			{ m_bBeiZhan = bZhanbei; }
	VOID					OnQuestEndbyPath( );
	VOID					OnReachEndPath( );

	//-------------------------------------------------------------------------
	// ��������
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
	// �Ƿ����
	//BOOL					IsMatrue();
	// �Ƿ��Զ��ջ�
	//BOOL					IsAotoReceive();

	Role*					GetMarster();
	DWORD					GetMarsterID();
	//VOID                    SetVisible(BOOL val, DWORD delay = 0) { m_bVisibleSet = val;  m_dwVisibleDelayTick = delay; }
private:
	//--------------------------------------------------------------------------
	// ��ʼ��
	//--------------------------------------------------------------------------
	BOOL					InitAtt(const tagCreatureProto* pProto);
	BOOL					InitAI(const tagCreatureProto* pProto, const tag_map_way_point_info_list* patrolList);

	//-------------------------------------------------------------------------
	// �����ʼ���ﵱǰ����
	//--------------------------------------------------------------------------
	VOID					CalInitAtt();

	//--------------------------------------------------------------------------
	// ����ʱ��ˢ�º���
	//--------------------------------------------------------------------------
	VOID					RefreshAtt();
	VOID					RefreshAI();

	//--------------------------------------------------------------------------
	// ���ָ���
	//--------------------------------------------------------------------------
	VOID					UpdateAI();
	VOID					UpdateLiveTime();
	VOID					UpdateCombat();
	VOID					UpdateDoor();

	//--------------------------------------------------------------------------
	// ���Ըı�
	//--------------------------------------------------------------------------
	virtual VOID			OnAttChange(INT nIndex);
	virtual VOID			OnAttChange(bool bRecalFlag[ERA_End]);

	//--------------------------------------------------------------------------
	// ս���ж�
	//--------------------------------------------------------------------------
	DWORD					TargetTypeFlag(Role* pTarget);
	DWORD					TargetTypeFlag(Creature* pTarget);
	DWORD					FriendEnemy(Role* pTarget);
	DWORD					FriendEnemy(Creature* pTarget);

	//--------------------------------------------------------------------------
	// ����
	//--------------------------------------------------------------------------
	Role*					FindRewardRole(Unit* pSrc);
	BOOL					ExpReward(Role* pReward, vector<Role*> &team_vec,BOOL bKill=FALSE, const Team *pTeam=(Team*)INVALID_VALUE, INT nShareNum=1,bool bSelf=true);
	FLOAT					CalLevelCoefficient(INT nRoleLevel, INT nCreatureLevel);
	
private:
	//---------------------------------------------------------------------------
	// ��������
	//---------------------------------------------------------------------------
	const tagCreatureProto*					m_pProto;					// ��̬����
	AIController*							m_pAIController;			// AI������
	BOOL									m_bTagged;					// �����Ƿ��ѱ�����
	DWORD									m_dwTaggedOwner;			// ������

	Vector3									m_vBornPos;					// ������
	Vector3									m_vBornFace;				// ��������
	BOOL									m_bCollide;					// �Ƿ������ײ
	
	INT										m_nLiveTick;				// ����ʱ��
	INT										m_nRespawnTickCountDown;	// ���������ʱ

	ECreatureSpawnedType					m_eSpawnedType;				// ���ɷ�ʽ�����أ���Ѩ����̬��
	DWORD									m_dwSpawnerID;				// ����ǳ�Ѩ���߶�̬�����ģ����¼������ID������ID���������ˢ�µ㴴���ģ����¼ˢ�µ�id
	DWORD									m_dwSpawnGroupID;	
	//BOOL									m_bCanBeAttack;				// �Ƿ�ɱ�����

	ECreatureType							m_eCreatureType;			// ��λ����
	//----------------------------------------------------------------------------
	// �ű�
	//----------------------------------------------------------------------------
	const CreatureScript*					m_pScript;

	BOOL                                    m_bDoorOpened;              // ����������� ��Ϊ����״̬

	//----------------------------------------------------------------------------
	// ����С��
	//----------------------------------------------------------------------------
	DWORD									m_dwTeamID;				// ����С��ID
	//bool									m_bBeiZhan;
	bool									m_bDelSelf;					// ɾ���Լ�

	//----------------------------------------------------------------------------
	// ���ID
	//----------------------------------------------------------------------------
	DWORD									m_dwGuildID;			// 

	//----------------------------------------------------------------------------
	// ���＼��
	//----------------------------------------------------------------------------
	//Skill*									m_pMeleeSkill;			// ��������ͨ����
	//Skill*									m_pRangedSkill;			// Զ�̹�����ͨ����
		
	std::vector<Skill*>						m_pNormalSkill;	
	//Skill*									m_pNormalSkill[NORAML_SKILL_NUMBER];

	INT32									m_DoorOpenTime;			// �ſ���ʱ��

	DWORD									m_DoorObjID;

	BOOL									m_bDelMap;				// �Ƿ���Ҫ��Map��ɾ��

	// ��ֲ����
	//BOOL									m_bUsed;				// �Ƿ�ʹ��
	//INT										n_guildPlantIndex;		// ������ֲ��������
	//DWORD									m_dwTuDui;				// ֲ���Ӧ������
	//DWORD									m_dwMaxTield;			// ������
	//tagPlantData*							m_pPlantdata;			
};


