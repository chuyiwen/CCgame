/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once


/**
*	@file		role.h
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		人物数据结构
*/
#include "stdafx.h"
#include "../../common/WorldDefine/base_define.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/move_define.h"
#include "../../common/WorldDefine/talent_define.h"
#include "../../common/WorldDefine/QuestDef.h"
#include "../../common/WorldDefine/pk_define.h"
#include "../../common/WorldDefine/SocialDef.h"
#include "../../common/WorldDefine/skill_define.h"
#include "../../common/WorldDefine/compose_define.h"
#include "../../common/WorldDefine/ScriptMsgInfo.h"

#include "../../common/WorldDefine/compose_protocol.h"
#include "../../common/WorldDefine/pk_protocol.h"
#include "../../common/WorldDefine/talent_protocol.h"
#include "../../common/WorldDefine/action_protocol.h"
#include "../../common/WorldDefine/combat_protocol.h"
#include "../../common/WorldDefine/gm_protocol.h"

#include "../../common/WorldDefine/guild_protocol.h"
#include "../../common/WorldDefine/TreasureChest_define.h"
#include "../../common/WorldDefine/rank_protocol.h"

#include "../../common/WorldDefine/Sign_define.h"
#include "../../common/WorldDefine/LianHun_define.h"
#include "../../common/WorldDefine/reward_define.h"

#include "../common/ServerDefine/role_data_server_define.h"


#include "unit.h"
#include "item_mgr.h"
#include "currency.h"
#include "db_session.h"
#include "player_session.h"
#include "exchange.h"
#include "pet_exchange.h"
#include "suit.h"
#include "quest.h"
#include "stall.h"
#include "group_mgr.h"
#include "team.h"
#include "vcard.h"
#include "clandata.h"
#include "state_mgr.h"
#include "mutex.h"
#include "PracticeMgr.h"
#include "achievement_mgr.h"
#include "RaidMgr.h"

#define USE_RIDE_EX 0

class PlayerSession;
class Map;
class quest;
class Skill;
class Creature;
class stall;
class Team;
class RoleScript;
class TitleMgr;
class PetPocket;

struct s_role_data_load;
struct tagQuestDoneSave;
struct tagRoleTalent;
struct NET_DB2C_save_role;
struct s_ime_effect;
//--------------------------------------------------------------------------------
// 被动技能和装备的触发器类型
//--------------------------------------------------------------------------------
enum EPassiveSkillAndEquipTrigger
{
	EPSAET_Null				=	-1,
	EPSAET_BeAttacked		=	0,	// 被攻击
	EPSAET_Hit				=	1,	// 命中
	EPSAET_Hited			=	2,	// 被命中
	EPSAET_Dodged			=	3,	// 被闪避
	EPSAET_Dodge			=	4,	// 闪避
	EPSAET_Blocked			=	5,	// 被格挡
	EPSAET_Block			=	6,	// 格挡
	EPSAET_Crit				=	7,	// 暴击
	EPSAET_Crited			=	8,	// 被暴击
	EPSAET_Die				=	9,	// 死亡
	EPSAET_Random			=	10,	// 随机
	EPSAET_Attack			=	11,	// 攻击
	EPSAET_Kill				=	12,	// 击杀
	EPSAET_Be_Dizzy			=	13, // 被眩晕
	EPSAET_Kill_Player		=	14,	// 击杀玩家
	EPSAET_End			
};



typedef VOID (Role::*pFun_RegTriggerEquipSet)(DWORD, DWORD, INT16);
//------------------------------------------------------------------------------
// 角色类
//------------------------------------------------------------------------------
class Role : public Unit, public ScriptData<ESD_Role>
{
public:
	friend class PlayerSession;
	friend class GMCommandMgr;
	friend class ItemMgr;
	friend class Creature;
public:
	typedef package_map<UINT16, tagQuestDoneSave*> QuestDoneMap;
	typedef package_map<UINT16, quest*>	QuestMap;

	typedef State<INT64, ERoleState>				RoleState;
	typedef State<DWORD, ERoleStateEx>				RoleStateEx;

	typedef package_map<DWORD, s_enter_map_limit*>			MapLimitMap;

	typedef package_list<s_inst_process*>			LIST_INST_PRO;

protected:
	//---------------------------------------------------------------------------
	// Constructor&Destructor
	//---------------------------------------------------------------------------
	Role(DWORD dw_role_id, const s_role_data_load* pData, PlayerSession* pSession);
	virtual ~Role();
public:
	static Role* Create(DWORD dw_role_id, const s_role_data_load* pData, PlayerSession* pSession);
	static VOID	Delete(Role* &pRole);
	static e_role_att_to_change ERA2ERAC(ERoleAttribute erac);
	static ERoleAttribute ERAC2ERA(e_role_att_to_change erac);
	//---------------------------------------------------------------------------
	// 初始化
	//---------------------------------------------------------------------------
	VOID Init(const s_role_data_load* pData);

	//---------------------------------------------------------------------------
	// 上线
	//---------------------------------------------------------------------------
	VOID Online(BOOL bFirst);

	//----------------------------------------------------------------------------
	// 第一次上线
	//----------------------------------------------------------------------------
	VOID VirginOnline();

	//----------------------------------------------------------------------------
	// 各种Update函数
	//----------------------------------------------------------------------------
	virtual VOID Update();
	VOID RegenerAll();
	VOID Regenerate(EPowers power);
	VOID RegenerateHealth();
	//-----------------------------------------------------------------------------
	// 保存数据库
	//-----------------------------------------------------------------------------
	VOID SaveToDB();

	//-----------------------------------------------------------------------------
	// 加入到游戏世界中
	//-----------------------------------------------------------------------------
	BOOL AddToWorld(BOOL bFirst);

	//-----------------------------------------------------------------------------
	// 传送到到某个地图中
	//-----------------------------------------------------------------------------
	BOOL GotoNewMap(DWORD dwDestMapID, FLOAT fX, FLOAT fY, FLOAT fZ, FLOAT fFaceX = 0,FLOAT fFaceY = 0, FLOAT fFaceZ = 0, DWORD dwMisc=0, BOOL bSameInstance=TRUE);

	//------------------------------------------------------------------------------
	// 回城
	//------------------------------------------------------------------------------
	VOID ReturnCity();

	//------------------------------------------------------------------------------
	// 物品是否需要冷却
	//------------------------------------------------------------------------------
	BOOL ObjectCoolOff()
	{
		return m_bObjectCoolOff;
	}

	VOID SetObjectCoolMode(BOOL bMode)
	{
		m_bObjectCoolOff = bMode;
	}

	//------------------------------------------------------------------------------
	// 禁言
	//------------------------------------------------------------------------------
	BOOL SpeakOff() 
	{
		return !SpeakOffTimeOut( ); 
	}

	VOID SetSpeakOff(BOOL bFlag, DWORD dwSeconds = 0) 
	{ 
		if(bFlag)
		{
			dwSeconds = max(dwSeconds, 60);
			m_dwForbidTalkStart = GetCurrentDWORDTime( );
			m_dwForbidTalkEnd = IncreaseTime((tagDWORDTime)m_dwForbidTalkStart, dwSeconds);
		}
		else
		{
			m_dwForbidTalkEnd = m_dwForbidTalkStart = 0;
		}
	}

	BOOL SpeakOffTimeOut(){ return m_dwForbidTalkEnd <= GetCurrentDWORDTime( );}

	//-----------------------------------------------------------------------
	// 检查是否在对应职能NPC附近
	//-----------------------------------------------------------------------
	DWORD CheckFuncNPC(DWORD dwNPCID, EFunctionNPCType eNPCType, OUT Creature **ppNPC = NULL, OUT Map **ppMap = NULL);

	//------------------------------------------------------------------------------
	// 当前状态 -- 各状态间的转换，需要手动完成。即SetState()不会自动清除其他状态位。
	//------------------------------------------------------------------------------
	INT64 GetRoleState() const { return m_RoleState.GetState(); }

	BOOL IsInRoleState(ERoleState eState) const
	{
		return m_RoleState.IsInState(eState);
	}

	BOOL IsInRoleStateAll(INT64 dwState) const
	{
		return m_RoleState.IsInStateAll(dwState);
	}

	BOOL IsInRoleStateAny(INT64 dwState) const
	{
		return m_RoleState.IsInStateAny(dwState);
	}

	VOID SetRoleState(ERoleState eState, BOOL bSendMsg=TRUE)
	{
		RoleState oldState = m_RoleState;

		m_RoleState.SetState(eState);

		if( bSendMsg )
		{
			NET_SIS_set_role_state send;
			send.dw_role_id = GetID();
			send.eState = eState;
			if( VALID_POINT(get_map()) )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
		}

		OnSetRoleState(oldState, eState);
	}


	VOID OnSetRoleState(const RoleState& oldState, ERoleState eState);

	VOID UnsetRoleState(ERoleState eState, BOOL bSendMsg=TRUE)
	{
		if( FALSE == IsInRoleState(eState) ) return;

		RoleState oldState = m_RoleState;
		m_RoleState.UnsetState(eState);

		if( bSendMsg )
		{
			NET_SIS_unset_role_state send;
			send.dw_role_id = GetID();
			send.eState = eState;
			if( VALID_POINT(get_map()) )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
		}

		OnUnsetRoleState(oldState, eState);
	}

	VOID OnUnsetRoleState(const RoleState& oldState, ERoleState eState);


	VOID OnInvestCarryNPC(DWORD dwCarryID);

	//------------------------------------------------------------------------------
	// 扩展状态 -- 该状态只需玩家本人知道就可以
	//------------------------------------------------------------------------------
	const RoleStateEx& GetRoleStateEx() const
	{
		return m_RoleStateEx;
	}
	
	VOID SetRoleStateEx(ERoleStateEx eState, BOOL bSendMsg=TRUE)
	{
		if( TRUE == m_RoleStateEx.IsInState(eState) ) return;

		m_RoleStateEx.SetState(eState);

		if( bSendMsg )
		{
			NET_SIS_set_role_state_extend send;
			send.eState = eState;
			SendMessage(&send, send.dw_size);
		}
	}

	VOID UnsetRoleStateEx(ERoleStateEx eState, BOOL bSendMsg=TRUE)
	{
		if( FALSE == m_RoleStateEx.IsInState(eState) ) return;

		m_RoleStateEx.UnsetState(eState);

		if( bSendMsg )
		{
			NET_SIS_unset_role_state_extend send;
			send.eState = eState;
			SendMessage(&send, send.dw_size);
		}
	}

	//------------------------------------------------------------------------------
	// 是否处在不能移动的状态
	//------------------------------------------------------------------------------
	virtual BOOL IsInStateCantMove()
	{
		return Unit::IsInStateCantMove() || IsInRoleStateAny(ERS_Stall | ERS_Prictice);
	}

	//------------------------------------------------------------------------------
	// 是否处在不能使用技能的状态
	//------------------------------------------------------------------------------
	virtual BOOL IsInStateCantCastSkill()
	{
		return Unit::IsInStateCantCastSkill() || IsInRoleStateAny(ERS_Stall | ERS_Prictice | ERS_Fishing | ERS_ComPractice) \
			/*|| IsPurpureName( )*/;//gx add 双修状态限制
	}

	//------------------------------------------------------------------------------
	// 是否处在不可被攻击状态
	//------------------------------------------------------------------------------
	virtual BOOL IsInStateCantBeSkill()
	{
		return Unit::IsInStateCantBeSkill() || IsInRoleStateAny(ERS_Stall | ERS_Fishing);
	}

	//------------------------------------------------------------------------------
	// 是否处在不可见的状态
	//------------------------------------------------------------------------------
	virtual BOOL IsInStateInvisible()
	{
		return Unit::IsInStateInvisible();
	}

	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
	// 检测所在地图区域
	//------------------------------------------------------------------------------
	VOID CheckMapArea();
	
	//------------------------------------------------------------------------------
	// 金钱和物品
	//------------------------------------------------------------------------------
	CurrencyMgr&	GetCurMgr()		{ return m_CurMgr; }
	ItemMgr&		GetItemMgr()	{ return m_ItemMgr; }

	//-------------------------------------------------------------------------------
	// 人物数据获取
	//-------------------------------------------------------------------------------
	EClassType				GetClass()		const { return m_eClass; }
	EClassTypeEx			GetClassEx()	const { return m_eClassEx; }
	VOID					SetClass(EClassType eVocation)		{m_eClass = eVocation;}
	VOID					SetClassEx(EClassTypeEx eHVocation)		{m_eClassEx = eHVocation;}
	INT						GetCredit()		const { return m_nCredit; }
	INT						GetIdentity()	const { return m_nIdentity; }
	INT						GetVIPPoint()	const { return m_nVIPPoint; }
	PlayerSession*			GetSession()	const { return m_pSession; }
	const tagAvatarAtt*		GetAvatar()		const { return &m_Avatar; }
	const tagDisplaySet&	GetDisplaySet()	const { return m_DisplaySet; }
	ERolePKState			GetPKState()	const { return m_ePKState; }
	INT						GetPKValue()    const { return m_iPKValue; }
	INT						GetClientPKValue() const;
	//BOOL					GetIsPurpureDec() const {return mIsPurpureDec;}
	DWORD					GetRebornMapID()const { return m_dwRebornMapID; }
	DWORD					GetNameID()		const { return GetID(); }			//?? 该接口需进一步处理
	DWORD					GetTeamID()		const { return m_dwTeamID; }
	BOOL					GetIsLeader()	const { return m_bLeader; }
	DWORD					GetGroupID()	const { return m_dwGroupID; }
	DWORD					GetTeamInvite()	const { return m_dwTeamInvite; }
	DWORD					GetEarnRate()	const 
	{ 
		if(VALID_POINT(GetSession())) 
		{
			return GetSession()->GetFatigueGuarder().GetEarnRate(); 
		}
		else 
		{
			return 10000.0f;
		}
	}
	virtual BYTE			GetSex()		const { return m_Avatar.bySex; }
	INT						GetOnlineTime() const { return m_nOnlineTime; }
	tagDWORDTime			GetLogoutTime() const { return m_LogoutTime; }
	DWORD					GetDestoryEquipCount() const { return m_dwDestoryEquipCount;}
	VOID					SetDestoryEquipCount(DWORD dwCount) { m_dwDestoryEquipCount = dwCount; }
	VOID					ModDestoryEquipCount(DWORD dwCount) { m_dwDestoryEquipCount += dwCount; }

	EPowers					GetPowerType();
	BOOL					IsInCombat();
	tagItem*				GetItem(INT64 n64_serial);
	BOOL					IsGM( ) const { return VALID_POINT(m_pSession) ? m_pSession->IsGM( ) : FALSE; }
	BYTE					GetGMPrivilege() const { return VALID_POINT(m_pSession) ? m_pSession->GetPrivilege() : 0;}
	VOID					SetSession(PlayerSession* pSession) { m_pSession = pSession; }
	BOOL					is_today_first_login_after(INT hour) const;
	BOOL					is_effect_sign_login() const;
	const tagRemoteOpenSet& GetRemoteOpenSet() const { return m_sRemoteOpenSet; }
	
	VOID					AccountSignLevel();
	LPCTSTR					GetVNBName() const { return VALID_POINT(GetSession()) ? GetSession()->GetVNBName() : _T(""); }
	
	VOID					GetSignature(LPTSTR sz_signature) { _tcsncpy(sz_signature, m_szSignature_name, X_SHORT_NAME); }
	VOID					SetSignature(LPTSTR sz_signature) { _tcsncpy(m_szSignature_name, sz_signature, X_SHORT_NAME); }

	tagAvatarEquip			GetAvatarEquip() const;

	const tagChestItem&		GetChestItem()	const	{return m_TreasureState.ChestItem;}
	INT64					GetChestSerial() const	{return m_TreasureState.nChestSerial;}
	INT64					GetKeySerial() const	{return m_TreasureState.nKeySerial;}
	DWORD					GetChestTypeID() const	{return m_TreasureState.dwChestTypeID;}
	DWORD					GetKeyTypeID() const	{return m_TreasureState.dwKeyTypeID;}

	MapLimitMap&			GetMapLimitMap()		{ return m_mapMapLimit; }	

	inline void				ResetUseKillbadgeCD()	{ m_dwUseKillbadgeCD = KILLBADGEITEMCD_TICK; }
	BOOL					CanUseKillbadge() const { return m_dwUseKillbadgeCD == 0; }
	//-------------------------------------------------------------------------------
	// 设置人物的战斗模式
	//-------------------------------------------------------------------------------
	EBattleMode				GetBattleMode( ) const	{return m_eBattleMode; }
	void					SetBattleMode( EBattleMode eMode ) { m_eBattleMode = eMode; }

	DWORD					GetSignID()		const	{ return m_dwSignRoleID; }

	tagDWORDTime&		GetLeaveGuildTime()  { return m_LeaveGuildTime; }
	VOID				SetHangBeginTime() { m_dwHangBeginTime = GetCurrentDWORDTime(); }

	//-------------------------------------------------------------------------------
	// 人物数据设置
	//-------------------------------------------------------------------------------
	VOID SetSignID(DWORD dw_role_id)
	{
		m_dwSignRoleID = dw_role_id;
	}

	VOID SetChestTypeID(DWORD dw_data_id)
	{
		m_TreasureState.dwChestTypeID = dw_data_id;
	}

	VOID SetKeyTypeID(DWORD dw_data_id)
	{
		m_TreasureState.dwKeyTypeID = dw_data_id;
	}

	VOID SetChestSerial(INT64 nSerial)
	{
		m_TreasureState.nChestSerial = nSerial;
	}

	VOID SetKeySerial(INT64 nSerial)
	{
		m_TreasureState.nKeySerial = nSerial;
	}

	VOID SetChestItem(tagChestItem item)
	{
		m_TreasureState.ChestItem = item;
	}

	VOID SetFashionMode(bool bDisplay)
	{
		m_DisplaySet.bFashionDisplay = bDisplay;
	}

	VOID SetDisplaySet(bool bHead, bool bFace, bool bBack, bool bFly)
	{
		m_DisplaySet.Set(bHead, bFace, bBack, bFly);
	}

	VOID SetEquipDisplay(BOOL bFashion, INT nDisplayPos, DWORD dw_data_id, BYTE byFlareVal, INT8 n8ColorID)
	{
		if(bFashion)	// 时装
		{
			m_AvatarEquipFashion.Set(nDisplayPos, dw_data_id, byFlareVal, n8ColorID);
		}
		else	// 武器或防具
		{
			m_AvatarEquipEquip.Set(nDisplayPos, dw_data_id, byFlareVal, n8ColorID);

			// 如果是武器，则做特殊处理
			if(M_is_wapon_by_display_pos(nDisplayPos))
			{
				m_AvatarEquipFashion.Set(nDisplayPos, dw_data_id, byFlareVal, n8ColorID);
			}
		}
	}

	VOID SetEquipEffect(INT nDisplayPos, BYTE byEquipEffect)
	{
		m_AvatarEquipEquip.Set(nDisplayPos, byEquipEffect);
		m_AvatarEquipFashion.Set(nDisplayPos, byEquipEffect);
	}

	VOID	ResetWeaponDmg(const tagEquip& Equip, BOOL bEquip);	// 换装时调用


	//-------------------------------------------------------------------------------
	// 帮派相关
	//-------------------------------------------------------------------------------
	BOOL	IsInGuild()	const			{ return m_dwGuildID != INVALID_VALUE; }
	DWORD	GetGuildID() const			{ return m_dwGuildID; }
	VOID	SetGuildID(DWORD dwGuildID);
	VOID	SetLeaveGuildTime();

	//-------------------------------------------------------------------------------
	// 邮件相关
	//-------------------------------------------------------------------------------
	VOID	AddSendMailNum();
	INT		GetSendMainNum(){ return m_nSendMailNum ;}
	VOID	ClearSendMainNum(){ m_nSendMailNum = 0; }

	//-------------------------------------------------------------------------------
	// 地图限制相关
	//-------------------------------------------------------------------------------
	VOID	DelMapLimit(INT nType);
	VOID	DelMapProcess(INT nType);

	//-------------------------------------------------------------------------------
	// 是否需要保存到数据库
	//-------------------------------------------------------------------------------
	BOOL IsNeedSave2DB() const { return m_nNeedSave2DBCountDown <= 0; }

	//-------------------------------------------------------------------------------
	// 发送消息
	//-------------------------------------------------------------------------------
	VOID SendMessage(LPVOID p_message, DWORD dw_size) { if( VALID_POINT(GetSession()) ) { GetSession()->SendMessage(p_message, dw_size); } }

	//---------------------------------------------------------------------------------
	// 投属性点
	//---------------------------------------------------------------------------------
	INT BidAttPoint(const INT nAttPointAdd[X_ERA_ATTA_NUM]);

	//---------------------------------------------------------------------------------
	// 洗属性点
	//---------------------------------------------------------------------------------
	INT ClearAttPoint(INT64& n64ItemID);

	//---------------------------------------------------------------------------------
	// 学技能
	//---------------------------------------------------------------------------------
	INT	LearnSkill(DWORD dwSkillID, DWORD dwNPCID=INVALID_VALUE, INT64 n64ItemID=INVALID_VALUE);

	//---------------------------------------------------------------------------------
	// 升级技能
	//---------------------------------------------------------------------------------
	INT LevelUpSkill(DWORD dwSkillID, DWORD dwNPCID=INVALID_VALUE, INT64 n64ItemID=INVALID_VALUE);

	//---------------------------------------------------------------------------------
	// 遗忘技能
	//---------------------------------------------------------------------------------
	INT ForgetSkill(DWORD dwSkillID, DWORD dwNPCID);

	//---------------------------------------------------------------------------------
	// 洗点
	//---------------------------------------------------------------------------------
	INT	ClearTalent(INT64 n64ItemID);

	//--------------------------------------------------------------------
	//  PK
	//--------------------------------------------------------------------
	VOID UpdatePK();
	//BOOL IsPKPenalty( ) const  { return m_iPKValue > 1; }
	BOOL HasPKValue( ) const {return m_iPKValue> 0 ;}
	//BOOL IsLightYellowName( ) const { return m_iPKValue == 1;}
	//BOOL IsYellowName( ) const { return m_iPKValue >= 2 && m_iPKValue <= 55; }
	BOOL IsRedName( ) const { return  ( m_iPKValue >= ATTACK_WHITENAME_INCPK); }
	//BOOL IsPurpureName( ) const { return (m_iPKValue == 100); }

	VOID SetPKState(ERolePKState ePKState) { m_ePKState = ePKState; }
	DWORD SetPKStateLimit(ERolePKState ePKState);

	VOID SetPKValueMod(INT nValue);
	//VOID SetLastAttacker( DWORD dwAttacker )
	//{
	//	m_mapLastAttacker.add( dwAttacker, GetCurrentDWORDTime( ) + ATTACK_INCPK_INTERVAL );
	//}

	VOID SetLeaveCombatTime( ){ m_dwLeaveCombatTime = GetCurrentDWORDTime( ); m_mapLastAttacker.clear( ); }
	tagDWORDTime GetLeaveCombatTime( ) { return m_dwLeaveCombatTime; }

	VOID DisappearPunisher( );

	//---------------------------------------------------------------------------------
	// 开启/更新PVP状态
	//---------------------------------------------------------------------------------
	VOID SetPVP();
	VOID UpdatePVP();

	//---------------------------------------------------------------------------------
	// 计算PK状态
	//---------------------------------------------------------------------------------
	VOID CalPKState(BOOL bSendMsg=TRUE);

	//---------------------------------------------------------------------------------
	// 更新在线挂机
	//---------------------------------------------------------------------------------
	VOID UpdateHang();
	VOID CalLineHang(tagEquip* pEquip, DWORD dw_data_id);
	VOID CalLeaveLineHang(DWORD dw_time);
	INT  CalLeaveTimeByExp(INT nLeaveExp);//根据离线经验计算对应的离线时间，单位小时
	INT  SetLeaveHangExp(DWORD dw_data_id, INT nHour, INT& nExp);
	VOID CleanLeaveExp() { m_nLeaveExp = 0; }
	VOID CleanLeaveBrother() { m_nLeaveBrotherHood = 0; }
	VOID CleanHangNum();
	VOID CalLeaveLingqi(DWORD dw_time);
	//---------------------------------------------------------------------------------
	// 复活
	//---------------------------------------------------------------------------------
	VOID BeRevived(INT nHP, INT nMP, Unit* pSrc);

	//---------------------------------------------------------------------------------
	// 同步单向好友是否在线
	//---------------------------------------------------------------------------------
	DWORD UpdateFriOnline();

	//---------------------------------------------------------------------------------
	// 目标是否完全可见//??
	//---------------------------------------------------------------------------------
	BOOL CanSeeTargetEntirely(Unit *pUnit)
	{
		return (FriendEnemy(pUnit) & ETFE_Friendly) == ETFE_Friendly;
	}

	//---------------------------------------------------------------------------------
	// 添加，删除和改变技能技能
	//---------------------------------------------------------------------------------
	virtual VOID AddSkill(Skill* pSkill, BOOL bSendMsg=TRUE)
	{
		Unit::AddSkill(pSkill);

		if( bSendMsg )
		{
			// 发送给自己添加一个技能
			NET_SIS_add_skill send;
			pSkill->GenerateMsgInfo(&send.Skill);
			SendMessage(&send, send.dw_size);
			
			if (!pSkill->IsFromeEquip())
			{
				// 保存到数据库
				NET_DB2C_add_skill send1;
				send1.dw_role_id = GetID();
				send1.s_skill_.dw_id = pSkill->GetID();
				send1.s_skill_.n_self_level_ = pSkill->GetSelfLevel();
				send1.s_skill_.n_learn_level_ = pSkill->GetLearnLevel();
				send1.s_skill_.n_proficiency_ = pSkill->GetProficiency();
				send1.s_skill_.n_cool_down_ = pSkill->GetCoolDownCountDown();
				g_dbSession.Send(&send1, send1.dw_size);
			}
			
		}
	};

	virtual VOID RemoveSkill(DWORD dwSkillID)
	{
		Skill* pSkill = m_mapSkill.find(dwSkillID);
		BOOL bFromEquip = FALSE;
		if (VALID_POINT(pSkill))
		{
			bFromEquip = pSkill->IsFromeEquip();
		}

		Unit::RemoveSkill(dwSkillID);

		// 发送给自己删除一个技能
		NET_SIS_remove_skill send;
		send.dwSkillID = dwSkillID;
		SendMessage(&send, send.dw_size);
		
		if (!bFromEquip)
		{
			// 发送到数据库删除
			NET_DB2C_remove_skill send1;
			send1.dw_role_id = GetID();
			send1.dw_skill_id = dwSkillID;
			g_dbSession.Send(&send1, send1.dw_size);
		}
	}

	virtual VOID ChangeSkillLevel(Skill* pSkill, ESkillLevelChange eType, INT nChange=1)
	{
		Unit::ChangeSkillLevel(pSkill, eType, nChange);

		// 发送给自己更新一个技能
		NET_SIS_update_skill send;
		pSkill->GenerateMsgInfo(&send.Skill);
		SendMessage(&send, send.dw_size);
		
		GetAchievementMgr().UpdateAchievementCriteria(eta_skill_level_up, pSkill->GetID(), pSkill->get_level());
		if (!pSkill->IsFromeEquip())
		{
			NET_DB2C_update_skill send1;
			send1.dw_role_id = GetID();
			send1.s_skill_.dw_id = pSkill->GetID();
			send1.s_skill_.n_self_level_ = pSkill->GetSelfLevel();
			send1.s_skill_.n_learn_level_ = pSkill->GetLearnLevel();
			send1.s_skill_.n_proficiency_ = pSkill->GetProficiency();
			send1.s_skill_.n_cool_down_ = pSkill->GetCoolDownCountDown();
			g_dbSession.Send(&send1, send1.dw_size);
		}
		
	}

	virtual VOID ChangeSkillExp(Skill *pSkill, INT nValue)
	{
		Unit::ChangeSkillExp(pSkill, nValue);
		
		// 发送给自己更新一个技能
		NET_SIS_update_skill send;
		pSkill->GenerateMsgInfo(&send.Skill);
		SendMessage(&send, send.dw_size);
	}

	virtual VOID StartSkillCoolDown(Skill* pSkill)
	{
		Unit::StartSkillCoolDown(pSkill);

		// 发送给自己更新技能CD
		NET_SIS_update_skill_cool_down send;
		send.dwSkillID = pSkill->GetID();
		send.nCoolDown = pSkill->GetCoolDownCountDown();
		SendMessage(&send, send.dw_size);
		
	}

	virtual VOID ClearSkillCoolDown(DWORD dwSkillID)
	{
		Unit::ClearSkillCoolDown(dwSkillID);

		// 发送给自己更新技能CD
		NET_SIS_update_skill_cool_down send;
		send.dwSkillID = dwSkillID;
		send.nCoolDown = 0;
		SendMessage(&send, send.dw_size);
	}
	//--------------------------------------------------------------------------------
	// 通过一级属性计算二级属性
	//--------------------------------------------------------------------------------
	VOID			AccountAtt(ERoleAttribute eRAtt);
	virtual VOID	ReAccAtt();
	//--------------------------------------------------------------------------------
	// 属性改变引起的其它内容改变
	//--------------------------------------------------------------------------------
	virtual VOID OnAttChange(INT nIndex);
	virtual VOID OnAttChange(bool bRecalFlag[ERA_End]);
	virtual VOID OnAttChange(INT nIndex, INT nDelta);

	//--------------------------------------------------------------------------------
	// 获得或减少经验
	//--------------------------------------------------------------------------------
	VOID ExpChange(INT nValue, BOOL bKill=FALSE, BOOL bForce=FALSE,INT nSpecial = 0);

	//--------------------------------------------------------------------------------
	// 改变等级
	//--------------------------------------------------------------------------------
	VOID LevelChange(INT nValue, BOOL bKill=FALSE, BOOL bForce=FALSE);

	//--------------------------------------------------------------------------------
	// 根据职业和等级设置基础属性
	//--------------------------------------------------------------------------------
	VOID SetBaseAttByLevel(EClassType eClassType, INT nLevel);

	//--------------------------------------------------------------------------------
	// 设置套装特效
	//--------------------------------------------------------------------------------
	VOID SetSuitEffect(DWORD dwSuitEffect)
	{ 
		m_AvatarEquipEquip.dwSuitEffectID = dwSuitEffect;
		m_AvatarEquipFashion.dwSuitEffectID = dwSuitEffect;
	}
	//所有装备位置上的装备强化等级是否达到规定值
	BOOL IsAllEquipLevelHas(DWORD dwLevel, BOOL bAttack);
	//--------------------------------------------------------------------------------
	// 换装相关
	//--------------------------------------------------------------------------------
	VOID	ProcEquipEffect(tagEquip* pNewEquip, tagEquip* pOldEquip, INT16 n16IndexOld, BOOL bDiscard = FALSE);
	INT32	GetEquipDisplayPos(INT16 n16EquipPos);
	DWORD	Equip(INT64 n64_serial, EEquipPos ePosDst);
	VOID	ResetWeaponDmg(tagEquip& Equip);				// 崭新度变化时调用
	INT		CalConsolidate(INT16 nBaseValue, BYTE byConLevel, float fParam);//计算强化等级影响
	tagEquip*	GetWeapon();//取得武器
	//--------------------------------------------------------------------------------
	// 角色采集技能加成
	//--------------------------------------------------------------------------------
	INT CalGatherRate( Creature* pCreature );

	//--------------------------------------------------------------------------------
	// 角色实际打怪获得经验计算
	//--------------------------------------------------------------------------------
	INT CalRealGainExp( INT nSrcExp, FLOAT fAdd = 0);

	//----------------------------------------------------------------------------------
	// 根据索引得到好友列表的相应位置
	//----------------------------------------------------------------------------------
	tagFriend GetFriend(INT nIndex)
	{
		ASSERT(nIndex < MAX_FILE_NAME && nIndex >= 0);
		return m_Friend[nIndex];
	}

	VOID SetFriend(INT nIndex, DWORD dwFriendID, DWORD dwFriVal = 0, BYTE byGroup = 1)
	{
		ASSERT(nIndex < MAX_FRIENDNUM && nIndex >= 0);
		ASSERT(byGroup < 5 && byGroup > 0);

		if(m_Friend[nIndex].dwFriendID == INVALID_VALUE && dwFriendID != INVALID_VALUE)
		{
			m_mapFriend.add(dwFriendID, &m_Friend[nIndex]);
		}
		else if(m_Friend[nIndex].dwFriendID != INVALID_VALUE && dwFriendID == INVALID_VALUE)
		{
			m_mapFriend.erase(m_Friend[nIndex].dwFriendID);
		}

		m_Friend[nIndex].dwFriendID = dwFriendID; 
		if(dwFriendID == INVALID_VALUE)
			m_Friend[nIndex].dwFriVal = 0;
		else
			m_Friend[nIndex].dwFriVal += dwFriVal;	
		m_Friend[nIndex].byGroup = byGroup;

		m_Friend[nIndex].dwFriVal = (m_Friend[nIndex].dwFriVal > MAX_FRIENDVAL) ? MAX_FRIENDVAL : m_Friend[nIndex].dwFriVal;
	}

	tagFriend*	GetFriendPtr(DWORD dwFriendID) { return m_mapFriend.find(dwFriendID); }
	BOOL		IsFriend(DWORD dwFriendID);
	//----------------------------------------------------------------------------------
	// 根据索引得到黑名单的相应位置
	//----------------------------------------------------------------------------------
	DWORD GetBlackList(INT nIndex)
	{
		ASSERT(nIndex < MAX_BLACKLIST && nIndex >= 0);
		return m_dwBlackList[nIndex];
	}

	VOID SetBlackList(INT nIndex, DWORD dw_role_id)
	{
		ASSERT(nIndex < MAX_BLACKLIST && nIndex >= 0);
		m_dwBlackList[nIndex] = dw_role_id;
	}

	DWORD GetEnemyList(INT nIndex)
	{
		ASSERT(nIndex < MAX_ENEMYNUM && nIndex >= 0);
		return m_dwEnemyList[nIndex];
	}

	VOID SetEnemyList(INT nIndex, DWORD dw_role_id)
	{
		ASSERT(nIndex < MAX_ENEMYNUM && nIndex >= 0);
		m_dwEnemyList[nIndex] = dw_role_id;
	}

	BOOL IsEnemyList(DWORD dw_role_id)
	{
		for(INT i = 0; i < MAX_ENEMYNUM; i++)
		{
			if(m_dwEnemyList[i] == dw_role_id)
				return TRUE;
		}
		return FALSE;
	}

	//----------------------------------------------------------------------------------
	// 设置小队和团队ID
	//----------------------------------------------------------------------------------
	VOID SetTeamID(DWORD dwTeamID)		{ m_dwTeamID = dwTeamID; }
	VOID SetLeader(BOOL bLeader)		{ m_bLeader = bLeader; }
	VOID SetGroupID(DWORD dwGroupID)	{ m_dwGroupID = dwGroupID; }
	VOID SetTeamInvite(DWORD dw_role_id)	{ m_dwTeamInvite = dw_role_id; }

	BOOL IsTeamMate(Role* pRole)		{ return GetTeamID() != INVALID_VALUE && GetTeamID() == pRole->GetTeamID(); }
	BOOL IsGuildMate(Role* pRole)		{ return GetGuildID() != INVALID_VALUE && GetGuildID() == pRole->GetGuildID(); }

	//----------------------------------------------------------------------------------
	// 与某个单位是不是友好的社会关系（队友，结拜，师徒，夫妻等）
	//----------------------------------------------------------------------------------
	BOOL IsFriendlySocial(Role* pRole)	{ return IsTeamMate(pRole); }

	//----------------------------------------------------------------------------------
	// 副本
	//----------------------------------------------------------------------------------
	DWORD		GetOwnInstanceID()	const;
	DWORD		GetOwnInstanceMapID() const;
	DWORD		GetMyOwnInstanceID() const			{ return m_dwOwnInstanceID; }
	DWORD		GetMyOwnInstanceMapID() const		{ return m_dwOwnInstanceMapID; }
	tagDWORDTime		GetMyOwnInstanceCreateTime() const	{ return m_dwInstanceCreateTime; }
	DWORD		GetExportMapID() const				{ return m_dwExportMapID; }
	Vector3&	GetExportPoint()					{ return m_vExport; }
	VOID		SetMyOwnInstanceID(DWORD dwID)		{ m_dwOwnInstanceID = dwID; }
	VOID		SetMyOwnInstanceMapID(DWORD dwID)	{ m_dwOwnInstanceMapID = dwID; }
	VOID		SetMyOwnInstanceCreateTime(tagDWORDTime time) { m_dwInstanceCreateTime = time; }
	VOID		SetExportMapID(DWORD dwMapID)		{ m_dwExportMapID = dwMapID; }
	VOID		SetExportPoint(Vector3 &vExport)	{ m_vExport = vExport; }

	INT32		GetCurExploits( ) const { return m_CurMgr.GetExploits();}

	//----------------------------------------------------------------------------------
	// 商城相关
	//----------------------------------------------------------------------------------
	tagDWORDTime GetLastMallFreeTime() const { return m_LastGetMallFreeTime; }

	VOID SetLastGetMallFreeTime(DWORD dwNewLastGetTime) { m_LastGetMallFreeTime = dwNewLastGetTime; }
	
	//----------------------------------------------------------------------------------
	// 主动技能状态触发
	//----------------------------------------------------------------------------------
	BOOL OnActiveItemBuffTrigger(Unit* pTarget, tagItem* pItem, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);
	//----------------------------------------------------------------------------------
	// 被动技能状态触发
	//----------------------------------------------------------------------------------
	BOOL OnPassiveSkillBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);

	//----------------------------------------------------------------------------------
	// 装备被动触发
	//----------------------------------------------------------------------------------
	BOOL OnEquipmentBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);

	//----------------------------------------------------------------------------------
	// 宠物被动触发
	//----------------------------------------------------------------------------------
	BOOL OnPetBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);

	//----------------------------------------------------------------------------------
	// 注册触发器技能关联表
	//----------------------------------------------------------------------------------
	VOID RegisterTriggerSkillSet(ETriggerEventType eEvent, DWORD dwSkillID);

	//----------------------------------------------------------------------------------
	// 反注册触发器技能关联表
	//----------------------------------------------------------------------------------
	VOID UnRegisterTriggerSkillSet(ETriggerEventType eType, DWORD dwSkillID);

	//----------------------------------------------------------------------------
	// 注册触发器装备关联表
	//----------------------------------------------------------------------------
	VOID RegisterTriggerEquipSet(DWORD dwTriggerID, DWORD dwBuffID, INT16 n16EquipPos)
	{
		ASSERT(MIsInEquipPos(n16EquipPos));

		if( INVALID_VALUE == dwTriggerID || INVALID_VALUE == dwBuffID )
		{
			return;
		}

		EPassiveSkillAndEquipTrigger eTriggerType = PreRegisterTriggerEquipSet(dwTriggerID, dwBuffID, true);
		if( EPSAET_Null == eTriggerType ) return;

		m_bitsetEquipTrigger[eTriggerType][n16EquipPos] = true;
	}

	//----------------------------------------------------------------------------
	// 反注册触发器装备关联表
	//----------------------------------------------------------------------------
	VOID UnRegisterTriggerEquipSet(DWORD dwTriggerID, DWORD dwBuffID, INT16 n16EquipPos)
	{
		ASSERT(MIsInEquipPos(n16EquipPos));

		if( INVALID_VALUE == dwTriggerID || INVALID_VALUE == dwBuffID )
		{
			return;
		}

		EPassiveSkillAndEquipTrigger eTriggerType = PreRegisterTriggerEquipSet(dwTriggerID, dwBuffID, false);
		if( EPSAET_Null == eTriggerType ) return;

		m_bitsetEquipTrigger[eTriggerType][n16EquipPos] = false;
	}


	//----------------------------------------------------------------------------------
	// 注册触发器套装关联表
	//----------------------------------------------------------------------------------
	VOID RegisterTriggerSuitSet(DWORD dwTriggerID, DWORD dwBuffID, DWORD dwSuitID)
	{
		if( INVALID_VALUE == dwTriggerID || INVALID_VALUE == dwBuffID )
		{
			return;
		}

		EPassiveSkillAndEquipTrigger eTriggerType = PreRegisterTriggerEquipSet(dwTriggerID, dwBuffID, true);
		if( EPSAET_Null == eTriggerType ) return;

		m_setSuitTrigger[eTriggerType].insert(dwSuitID);
	}

	//----------------------------------------------------------------------------------
	// 反注册触发器套装关联表
	//----------------------------------------------------------------------------------
	VOID UnRegisterTriggerSuitSet(DWORD dwTriggerID, DWORD dwBuffID, DWORD dwSuitID)
	{
		if( INVALID_VALUE == dwTriggerID || INVALID_VALUE == dwBuffID )
		{
			return;
		}

		EPassiveSkillAndEquipTrigger eTriggerType = PreRegisterTriggerEquipSet(dwTriggerID, dwBuffID, false);
		if( EPSAET_Null == eTriggerType ) return;

		m_setSuitTrigger[eTriggerType].erase(dwSuitID);
	}

	// 重置副本
	DWORD	reset_instance(DWORD dw_map_id, INT n_mode);
	VOID	reset_inst_process(DWORD dw_map_id, INT n_mode);
	BOOL	is_have_inst_process(DWORD dw_map_id, INT n_mode);
	DWORD	reset_instance_limit(DWORD dw_map_id);
	
	//--------------------------------------------------------------------------
	// 战斗辅助
	//--------------------------------------------------------------------------
	INT	GetAroundCreature(std::vector<DWORD> &vecCreature, FLOAT fOPRadius, FLOAT fHigh);
	INT	GetAroundRole(std::vector<DWORD> &vecRole, FLOAT fOPRadius, FLOAT fHigh);
	
	VOID removeFlowCreature();

	virtual VOID	SetFlowUnit(DWORD dwID);
private:
	//----------------------------------------------------------------------------------
	// 装备相关buff预处理，并获取tigger类型
	//----------------------------------------------------------------------------------
	EPassiveSkillAndEquipTrigger PreRegisterTriggerEquipSet(DWORD dwTriggerID, DWORD dwBuffID, BOOL bEquip);

	//----------------------------------------------------------------------------------
	// 装备相关被动trigger触发处理
	//----------------------------------------------------------------------------------
	VOID OnEquipmentBuffTriggerCommon(Unit* pTarget, ETriggerEventType eEvent, EPassiveSkillAndEquipTrigger eTriggerType);
	VOID OnEquipmentBuffTriggerSuit(Unit* pTarget, ETriggerEventType eEvent, EPassiveSkillAndEquipTrigger eTriggerType);

public:
	VOID SendInitStateGuild();				// 帮派
private:
	//------------------------------------------------------------------------------
	// 发送客户端初始属性
	//------------------------------------------------------------------------------
	VOID SendInitStateAtt();
	VOID SendInitStateSkill();
	VOID SendInitStateItem();	// 物品&装备
	VOID SendInitStateSuit();	// 套装
	VOID send_init_complete_quest();
	VOID send_init_incomplete_quest();
	VOID SendInitStateMoney();
	VOID SendInitStateReputation();
	VOID SendFriend();
	VOID SendBlack();
	VOID SendEnemy();
	
	VOID SendFatigueGuardInfo(BYTE byCode);	// 防沉迷
	VOID SendPetSNSInfo();					// 宠物sns
	//------------------------------------------------------------------------------
	// 初始化从数据库中读取的列表数据
	//------------------------------------------------------------------------------
	VOID InitAtt(const s_role_data_save* pData);
	VOID InitSkill(const BYTE* &pData, INT32 n_num);
	VOID InitBuff(const BYTE* &pData, INT32 n_num);
	VOID InitItem(const BYTE* &pData, INT32 n_num);
	VOID InitItemCDTime(const BYTE* &pData, INT32 n_num);
	VOID InitFriend(const BYTE* &pData, INT32 n_num);
	VOID InitFriendValue(const BYTE* &pData, INT32 n_num);
	VOID InitBlackList(const BYTE* &pData, INT32 n_num);
	VOID InitEnemyList(const BYTE* &pData, INT32 n_num);
	VOID InitMapLimit(const BYTE* &pData, INT32 n_num);
	VOID InitInstProcess(const BYTE* &pData, INT32 n_num);

	DWORD Put2Container(tagItem* pItem);

	VOID CalInitAtt();

	VOID CalInitState();

	//------------------------------------------------------------------------------
	// 初始化需保存数据库的数据
	//------------------------------------------------------------------------------
	VOID SaveSkill2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID SaveBuff2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID save_quest_to_db(OUT LPVOID p_data, OUT LPVOID &p_out, OUT INT32 &n_num);
	VOID SaveMapEnterLimit(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID SaveInstProcess(OUT LPVOID pData, OUT LPVOID &p_out, OUT INT &n_num);
	VOID SaveSignData(OUT LPVOID pData, OUT LPVOID &p_out);
	VOID SaveHuenjingData(OUT LPVOID pData, OUT LPVOID &p_out);
	VOID SaveRewardData(OUT LPVOID pData, OUT LPVOID &p_out);
	//------------------------------------------------------------------------------
	// 重置保存到数据库时间倒计时
	//------------------------------------------------------------------------------
	VOID ResetNeedSave2DBCD() { m_nNeedSave2DBCountDown = MIN_ROLE_SAVE2DB_INTERVAL; }
	
	VOID UpdateRoleRewardData();//玩家新增奖励信息后，及时通知数据库更新
	//------------------------------------------------------------------------------
	// 复活
	//------------------------------------------------------------------------------
	DWORD SetRebornMap(DWORD dwNPCID, DWORD &dwBornMapID, Vector3 &vBornPos);
	DWORD Revive(ERoleReviveType eType, INT64 n64ItemID, BOOL bNeedItem=TRUE);
	
	DWORD CoolDownRevive( );
	DWORD CityRevive();
	DWORD LocusRevive(INT64 n64ItemID, BOOL bNeedItem=TRUE);
	DWORD AcceptRevive();
	DWORD PrisonRevive();
	DWORD GuildRevive();
	DWORD BattleRevive();
	VOID  SetReviveMp(INT n = 1);
	DWORD InstRevive();
	DWORD YamunRevive( );
	DWORD PerfectionRevive();
	DWORD NormalRevive();
	DWORD GuildWarRevive();
	//------------------------------------------------------------------------------
	// 换装
	//------------------------------------------------------------------------------
	DWORD	CanEquip(tagEquip* pEquip, EEquipPos ePosDst);
	DWORD	Unequip(INT64 n64_serial, INT16 n16IndexDst);
	//DWORD	SwapWeapon();
	DWORD	MoveRing(INT64 n64SerialSrc, INT16 n16PosDst);
	VOID	ProcEquipEffectPos(tagEquip* pNewEquip, tagEquip* pOldEquip, EItemConType eConTypeNewDst, EItemConType eConTypeNewSrc);
	VOID	ProcEquipEffectAtt(tagEquip* pEquip, bool bEquip, const INT16 n16Index);
	VOID	ProcEquipEffectAvatar(tagEquip* pNewEquip, INT16 n16IndexOld);
	INT32	ResetOneEquipDisplay(tagEquip* pEquip, INT16 n16Index);
	VOID	ChangeRoleAtt(const tagRoleAttEffect Effect[], INT32 nArraySz, INT32 nFactor);
	VOID	ChangeRoleAtt(const INT32 nValue[], INT32 nArraySz, INT32 nAttStart, INT32 nFactor);
	VOID	ChangeRoleAtt(const DWORD dwValue[], INT32 nArraySz, INT32 nFactor);
	VOID	ProcEquipBuffTrigger(DWORD dwBuffID, BOOL bEquip);
	
	// 激活,取消某件装备的属性
	VOID	EquipEffect(tagEquip* pEquip, BOOL bEquip);
	//------------------------------------------------------------------------------
	// 装备武器技能
	//------------------------------------------------------------------------------
	VOID	ProcWeaponSkill(tagEquip* pEquip, bool bEquip);
	VOID	ProcEquipSkill(const tagEquipProto* pEquipProto, bool bEquip);
	//------------------------------------------------------------------------------
	// 装备维修相关计算
	//------------------------------------------------------------------------------
	DWORD	EquipRepair(INT64 n64SerialEquip, DWORD	dwNPCID);
	VOID	EquipRepair(tagEquip* pEquip);
	//------------------------------------------------------------------------------
	// 装备绑定,解绑
	//------------------------------------------------------------------------------
	DWORD	EquipBind(INT64 n64SerialEquip, INT64 n64SerialItem, DWORD dwNPCID, EBindStatus& eBindType);
	DWORD	EquipUnBind(INT64 n64SerialEquip, INT64 n64SerialItem);
	// 设置离线修炼
	DWORD	SetLeaveParcitice(BYTE	byTimeType, BYTE	byMulType);
	//------------------------------------------------------------------------------
	// 重置装备属性
	//------------------------------------------------------------------------------
	DWORD	EquipReAtt(DWORD dwNPCID, INT64 n64SerialEquip, INT64 n64ItemID, BYTE byIndex, BYTE byType);
	// 武器开光
	DWORD	EquipKaiguang(DWORD dwNPCID, INT64 n64SerialEquip);
	// 装备洗礼
	DWORD	EquipXili(DWORD dwNPCID, INT64 n64SerialEquip, DWORD dwType);
	// 洗礼属性替换
	DWORD	EquipXiliChange(INT64 n64SerialEquip);
	// 武魂提取
	DWORD	EquipGetWuhuen(INT64 n64SerialEquip, DWORD dwItemID, int nNum);
	// 武器熔炼
	DWORD	EquipRonglian(INT64 n64SerialEquip);
	// 饰品附魔
	DWORD	Equipfumo(INT64 n64SerialEquip, DWORD dwItemID, int nNum);
	// 鉴定饰品
	DWORD	IdentifyEquip(INT64 n64SerialReel, INT64 n64SerialEquip);
	// 装备变换
	DWORD	EquipChange(INT64 n64SerialEquip, DWORD n64Item1, DWORD n64Item2, DWORD n64Item3, INT64& n64NewEquip);
	// 使用祝福油
	DWORD	EquipUseLuckYou(INT64 n64Item,CHAR &cCurLuck,CHAR &cChangeLuck);
	//------------------------------------------------------------------------------
	// 武器伤害相关计算
	//------------------------------------------------------------------------------
	/* 计算磨损对武器伤害影响.返回值为对武器伤害影响的百分比值.*/
	FLOAT	CalAbrasionEffect(const tagEquip& Equip);

	//------------------------------------------------------------------------------
	// 玩家间交易相关
	//------------------------------------------------------------------------------
	ExchangeMgr& GetExchMgr() { return m_ExchMgr; }
	
	BOOL	IsExchanging()					{ return IsInRoleState(ERS_Exchange); }
	BOOL	CanExchange()
	{
		return !IsInRoleStateAny(ERS_Exchange | ERS_Shop | ERS_Stall) 
				&& (GetExchMgr().GetTgtRoleID() == INVALID_VALUE);
	}
	
	VOID	BeginExchange(DWORD dwTgtRoleID)
	{
		SetRoleState(ERS_Exchange);
		GetExchMgr().CreateData();
		GetExchMgr().SetTgtRoleID(dwTgtRoleID);
	}

	VOID	EndExchange()
	{
		UnsetRoleState(ERS_Exchange);
		GetExchMgr().DeleteData();
		GetExchMgr().SetTgtRoleID(INVALID_VALUE);
	}

	DWORD	ProcExchangeReq(OUT Role* &pTarget, DWORD dwTgtRoleID);
	DWORD	ProcExchangeReqRes(OUT Role* &pApplicant, DWORD dwTgtRoleID, DWORD dw_error_code);
	DWORD	ProcExchangeAdd(OUT Role* &pTarget, OUT tagItem* &pItem, INT32 &nInsIndex, INT64 n64_serial);
	DWORD	ProcExchangeDec(OUT Role* &pTarget, INT64 n64_serial);
	DWORD	ProcExchangeMoney(OUT Role* &pTarget, INT64 n64Silver);
	DWORD	ProcExchangeLock(OUT Role* &pTarget);
	DWORD	ProcExchangeCancel(OUT Role* &pTarget);
	DWORD	ProcExchangeVerify(OUT Role* &pTarget, OUT DWORD &dwFailedRoleID);

	DWORD	VerifyExchangeData(OUT tagItem* pItem[]);
	DWORD	ProcExchange();

	DWORD	ProcPrepareExchange(OUT Role* &pTarget);

	//---------------------------------------------------------------------------------
	// 商店相关
	//---------------------------------------------------------------------------------
	DWORD	GetShopItems(DWORD dwNPCID, BYTE byShelf);
	DWORD	GetShopEquips(DWORD dwNPCID, BYTE byShelf);
	DWORD	BuyShopItem(DWORD dwNPCID, BYTE byShelf, DWORD dw_data_id, INT16 n16ItemNum);
	DWORD	BuyShopEquip(DWORD dwNPCID, BYTE byShelf, DWORD dw_data_id, INT64 n64_serial);
	DWORD	SellToShop(DWORD dwNPCID, int nNUmber, INT64* n64_serial);

	VOID	SendShopFeedbackMsg(DWORD dw_error_code, DWORD dwNPCID);

	//-----------------------------------------------------------------------------
	// 商城相关
	//-----------------------------------------------------------------------------
	DWORD	GetMallAll(OUT DWORD &dwMallTime);
	DWORD	UpdateMallAll(OUT DWORD &dwNewMallTime, IN DWORD dwOldMallTime);
	DWORD	BuyMallItem(DWORD dwID, INT nUnitPrice, INT16 n16BuyNum, BYTE byIndex);
	DWORD	BuyMallPack(DWORD dwID, INT nUnitPrice, BYTE byIndex);
	DWORD	BuyMallItem(DWORD dwTgtRoleID, LPCTSTR szLeaveWord, 
							DWORD dwID, INT nUnitPrice, INT16 n16BuyNum, BYTE byIndex);
	DWORD	BuyMallPack(DWORD dwTgtRoleID, LPCTSTR szLeaveWord,
							DWORD dwID, INT nUnitPrice, BYTE byIndex);
	DWORD	GetMallFreeItem(DWORD dwID);

public:
	DWORD	MallItemExchange(DWORD dwMallID, INT nPrice, INT16 n16BuyNum, BYTE byIndex);
	DWORD	MallPackExchange(DWORD dwMallID, INT nPrice, BYTE byIndex);

public:
	//-----------------------------------------------------------------------------
	// 挂机相关相关
	//-----------------------------------------------------------------------------
	VOID	IncreaseHangNum() { m_n16HangNum++; }
	VOID	ZeroHangNum()	{ m_n16HangNum = 0; }
	INT16	GetHangNum()	{ return m_n16HangNum; }
	DWORD   StartHang();
	DWORD	CancelHang();
	VOID	SetUseLeaveExp(BOOL bExp) { m_bExp = bExp; }
	VOID	SetUseLeaveBrother(BOOL bBrother) { m_bBrotherhood = bBrother; }
	INT		GetLeaveExp() { return m_nLeaveExp; }
	INT     GetLeaveBrother() { return m_nLeaveBrotherHood; }
	BOOL	IsCanHang();

private:
	DWORD	RoleSetVocation(DWORD dwNPCID, EClassType eVocation);
	DWORD	RoleSetHeroVocation(DWORD dwNPCID, EClassTypeEx eHVocation);
	DWORD	ChangeVocation(DWORD dwNPCID, EClassType eVocation);

private:

	//-----------------------------------------------------------------------------
	// 商城元宝交易相关
	//-----------------------------------------------------------------------------
	DWORD	SaveYB2Account(DWORD dwID, INT n_num);
	DWORD 	SaveSilver2Account(DWORD dwID, INT64 n_num);
	DWORD 	DepositYBAccout(DWORD dwID, INT n_num);
	DWORD 	DepositSilverAccount(DWORD dwID, INT64 n_num);
	DWORD 	GetYBTradeInfo();
	DWORD 	SubmitSellOrder(DWORD dw_role_id, INT n_num, INT nPrice);
	DWORD 	SubmitBuyOrder(DWORD dwRole, INT n_num, INT nPrice);
	DWORD 	DeleteOrder(DWORD dw_role_id, DWORD dwOrderID, EYBOTYPE eYBOType);
	DWORD 	GetYBOrder(DWORD dw_role_id);

public:

	PracticeMgr& GetPractice() { return m_PracticeMgr; }
	// 结束修炼
	DWORD	PracticeEnd();
	//------------------------------------------------------------------------
	// 摆摊相关
	//------------------------------------------------------------------------
	DWORD	StartStall()
	{
		DWORD dwRtv = m_pStall->init( );
		if (E_Success == dwRtv)
		{
			StopMount();
		}
		return dwRtv;//?? 城镇税率
	}

	BYTE GetStallMaxIdx( )
	{
		return m_pStall->get_max_index( );
	}

	DWORD	SetStallGoods(INT64 n64_serial, INT64 n64UnitPrice, BYTE byIndex)
	{
		return m_pStall->set_goods(n64_serial, n64UnitPrice, byIndex);
	}

	DWORD	UnsetStallGoods(const BYTE byIndex)
	{
		return m_pStall->unset_goods(byIndex);
	}

	DWORD	SetStallTitle(LPCTSTR strTitle)
	{
		return m_pStall->set_title(strTitle);
	}

	DWORD SetStallAd( LPCTSTR strAd )
	{
		return m_pStall->set_ad( strAd );
	}

	DWORD	SetStallFinish()
	{
		return m_pStall->set_finish();
	}

	DWORD	CloseStall()
	{
		return m_pStall->destroy();
	}

	DWORD	GetStallTitle(OUT LPTSTR pSzTitle)
	{
		return m_pStall->get_title(pSzTitle);
	}

	DWORD GetStallAd( OUT LPTSTR pSzAd )
	{
		return m_pStall->get_ad( pSzAd );
	}

	VOID GetStallHistoryChat( Role* pDest )
	{
		if( VALID_POINT( m_pStall ) )
			m_pStall->get_history_chat( pDest );
	}

	VOID SaveStallChat( DWORD dwSender, LPVOID pData, DWORD dw_size )
	{
		if( VALID_POINT(m_pStall) ) 
			m_pStall->save_chat( dwSender, pData, dw_size );
	}
	DWORD	GetStallSpecGoods(BYTE byIndex, OUT LPVOID pData, OUT INT &nGoodsSz)
	{
		return m_pStall->get_goods(byIndex, pData, nGoodsSz);
	}
	
	DWORD	GetStallGoods(OUT LPVOID pData, OUT BYTE &byGoodsNum, OUT INT &nGoodsSz)
	{
		return m_pStall->get_goods(pData, byGoodsNum, nGoodsSz);
	}

	DWORD	BuyStallGoods(Role *pBuyer, INT64 n64_serial, INT64 n64UnitPrice, 
							INT16 n16Num, BYTE byIndex, OUT INT16 &n16RemainNum)
	{
		return m_pStall->sell_goods(pBuyer, n64UnitPrice, n64_serial, byIndex, n16Num, n16RemainNum);
	}

	BYTE	GetStallModeLevel() const
	{
		return m_pStall->get_modelevel();
	}

	BOOL	IsNoGoodsInStall() const
	{
		return m_pStall->empty();
	}

	BOOL	IsSetGoodsFinish() const
	{
		return IsInRoleState(ERS_Stall);
	}

	INT32	CalStallGoodsMemUsed() const	// 所有摊位上商品存入结构tagMsgStallGoods时的大小
	{
		return m_pStall->goods_memory_size();
	}

	DWORD	SendCloseStall();

 	DWORD	GainStallExp(INT32 nExp)
 	{
 		return m_pStall->gain_exp(nExp);
 	}

	VOID logKick(DWORD dwCmdID, INT nParam);

	DWORD	GMConsolidateEquip(INT16 n16ItemIndex, INT level);

	void	setInstancePass(int nIndex);
	bool	isInstancePass(int nIndex);
	bool	isSaodangIng();
	void	sendSaoDangData();
private:
	//---------------------------------------------------------------------------------
	// 部分职能NPC功能
	//---------------------------------------------------------------------------------
	DWORD	ProcDak(DWORD dwNPCID, INT32 nIndex, DWORD dwMapID, DWORD dwMisc, DWORD dwPosID);
	DWORD	InstanceSaodang(int nIndex);
	DWORD	InstanceSaodangOver();
	//---------------------------------------------------------------------------------
	// 使用磨石
	//---------------------------------------------------------------------------------
	DWORD	AbraseWeapon(INT64 n64AbraserSerial);

	//---------------------------------------------------------------------------------
	// 地图触发器
	//---------------------------------------------------------------------------------
	BOOL	MapTrigger(DWORD dwTriggerID, DWORD dwMisc);

	//---------------------------------------------------------------------------------
	// 通知队友进入副本
	//---------------------------------------------------------------------------------
	DWORD	InstanceNotify(BOOL bNotify);

	//---------------------------------------------------------------------------------
	// 玩家是否同意其它玩家进入副本的邀请
	//---------------------------------------------------------------------------------
	DWORD	InstanceAgree(BOOL bAgree);

	//---------------------------------------------------------------------------------
	// 玩家请求离开副本
	//---------------------------------------------------------------------------------
	DWORD	LeaveInstance();

	//---------------------------------------------------------------------------------
	// 玩家请求进入乱战副本
	//---------------------------------------------------------------------------------
	DWORD	EnterPVP();
	INT32	GetPVPMoney(INT nLevel);


	// 进入战场
	DWORD EnterBattle();
	//---------------------------------------------------------------------------------
	// 强化装备
	//---------------------------------------------------------------------------------
	// 铭纹
	//DWORD	PosyEquip(DWORD dwNPCID, DWORD dwFormulaID, INT64 n64ItemID, INT64 n64IMID, INT64 n64StuffID[], INT32 nArraySz, DWORD dw_cmd_id);	
	//DWORD	GMPosyEquip(DWORD dwFormulaID, INT16 n16ItemIndex);	
	// 镌刻
	//DWORD	EngraveEquip(DWORD dwNPCID, DWORD dwFormulaID, INT64 n64ItemID, INT64 n64IMID, INT64 n64StuffID[], INT32 nArraySz, DWORD dw_cmd_id);
	//DWORD	GMEngraveEquip(DWORD dwFormulaID, INT16 n16ItemIndex);
	//强化升星
	DWORD	ConsolidateEquip(DWORD dwNPCID, INT64 n64ItemID, INT64 n64StuffID, INT64 n64BaohuID, DWORD n64IMID, INT n_num, BOOL bBind);
	
	//升星失败
	VOID	OnConsolidateFail(tagEquip* pEquip, BOOL bBind);
	// 镶嵌
	DWORD	InlayEquip(INT64 n64DstItemID, INT64 (&n64SrcItemID)[MAX_EQUIPHOLE_NUM]);
	//拆嵌
	DWORD	Unbeset(INT64 n64SerialEquip, DWORD dwNPCID, BYTE byUnBesetPos);
	//原石打磨
	DWORD	damo(DWORD dwItemID, DWORD dwNumber, DWORD dwNPCID);
	//武器融合
	DWORD	FusionEquip(INT64 n64Equip1, INT64 n64Equip2, INT64 n64ItemID);
	//武器净化
	DWORD	PurificationEquip(DWORD dwNPCID, INT64 n64ItemID, INT64 n64StuffID);
	
	// 打孔
	DWORD	ChiselEquip(INT64 n64SrcItemID, DWORD dwNPCID, INT64 n64SuffID);
	DWORD	GMChiselEquip(INT16 pos, INT nHoldNum);
	// 开始修炼
	DWORD	PracticeBegin(INT64 n64Equip, DWORD* dwItem, DWORD* dwItemNum);
	
	DWORD	GMPractice(INT16 pos, INT level);
	// 武器学技能
	DWORD	WeaponLearnSkill(INT64 n64Equip, DWORD dwSkillID);
	// 武器升级技能
	DWORD	WeaponLevelUpSkill(INT64 n64Equip, DWORD dwSkillID);
	// 武器洗技能点	
	DWORD	WeaponClearTalent(INT64 n64ItemID);
	// 开启宠物寄养栏
	DWORD	PetXiulianSizeChange(DWORD dwNPCID);
	//---------------------------------------------------------------------------------
	// 生产合成物品
	//---------------------------------------------------------------------------------
	DWORD	ProduceItem(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, BOOL bBind, DWORD dw_cmd_id, DWORD byYuanBao, BOOL bFahion = FALSE);
	DWORD	ProduceEquip(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, BOOL bBind, BYTE byQualityType, BYTE byQualityNum);
	DWORD	CheckProduceLimit(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, const s_produce_proto_ser* &pProduceProto, BOOL bBind, BYTE byQualityType, BYTE byQualityNum);
	DWORD	CheckProduceAlchemy(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, const s_produce_proto_ser* &pProduceProto, BOOL bBind);
	DWORD	DeComposeItem(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, INT64 n64IMID, INT64 n64Item, DWORD dw_cmd_id, INT nOutStuffIndex[], INT& nStuffNum1, BOOL bPetDec);
	DWORD	CheckDeComposeLimit(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, const s_decompose_proto_ser* &pDeComposeProto, INT64 n64Item, tagEquip *pEquip);
	EProduceType Skill2ProduceType(ESkillTypeEx2 eSkillType);	
	tagItem*	 CreateItem(EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, INT16 n16Num, DWORD dwCreator, BOOL bBind = FALSE);
	VOID	CalIMEffect(E_consolidate_type_ser eConType, s_ime_effect &IMEffect, INT64 n64IMID, const LPVOID pProto, INT n_num);
	VOID	CalEquipFlare(tagEquip* pEquip);

	//---------------------------------------------------------------------------------
	// 查询某个天资所在的位置（如果没有，返回GT_INVALD）
	//---------------------------------------------------------------------------------
	INT FindTalentIndex(ETalentType eType)
	{
		INT nBegin = X_MAIN_TALENT_START, nEnd = X_ASS_TALENT_START;	// 设置起始点和终点

		// 如果是辅助天资，则取后半部分
		//if( ETT_Action == eType || ETT_Jugglery == eType )
		//{
		//	nBegin	=	X_ASS_TALENT_START;
		//	nEnd	=	X_MAX_TALENT_PER_ROLE;
		//}

		// 如果该天资是主天资，则
		for(INT n = nBegin; n < nEnd; n++)
		{
			if( m_Talent[n].eType == eType )
				return n;
		}

		return INVALID_VALUE;
	}

	//---------------------------------------------------------------------------------
	// 找到某个可插入的天资位置（如果没有，返回INVALID_VALUE）
	//---------------------------------------------------------------------------------
	INT FindBlankTalentIndex(ETalentType eType)
	{
		INT nBegin = X_MAIN_TALENT_START, nEnd = X_ASS_TALENT_START;	// 设置起始点和终点

		// 如果是辅助天资，则取后半部分
		//if( ETT_Action == eType || ETT_Jugglery == eType )
		//{
		//	nBegin	=	X_ASS_TALENT_START;
		//	nEnd	=	X_MAX_TALENT_PER_ROLE;
		//}

		for(INT n = nBegin; n < nEnd; n++)
		{
			if( ETT_Null == m_Talent[n].eType )
				return n;
		}

		return INVALID_VALUE;
	}

	//---------------------------------------------------------------------------------
	// 增加一个天资
	//---------------------------------------------------------------------------------
	VOID AddTalent(INT nIndex, ETalentType eType, INT nPoint=1)
	{
		ASSERT( nIndex >= 0 && nIndex < X_MAX_TALENT_PER_ROLE );
		ASSERT( ETT_Null == m_Talent[nIndex].eType );
		ASSERT( nPoint > 0 );

		m_Talent[nIndex].eType = eType;
		m_Talent[nIndex].nPoint = nPoint;

		// 同步
		NET_SIS_add_talent send;
		send.eType = eType;
		send.nPoint = nPoint;
		SendMessage(&send, send.dw_size);
	}

	//---------------------------------------------------------------------------------
	// 删除一个天资
	//---------------------------------------------------------------------------------
	VOID RemoveTalent(INT nIndex)
	{
		ASSERT( nIndex >= 0 && nIndex < X_MAX_TALENT_PER_ROLE );
		ASSERT( ETT_Null != m_Talent[nIndex].eType );
		
		ETalentType eType = m_Talent[nIndex].eType;

		m_Talent[nIndex].eType = ETT_Null;
		m_Talent[nIndex].nPoint = 0;

		// 同步
		NET_SIS_remove_talent send;
		send.eType = eType;
		SendMessage(&send, send.dw_size);
	}


	//---------------------------------------------------------------------------------
	// 得到某个索引所对应的天资当前投点数
	//---------------------------------------------------------------------------------
	INT GetTalentPoint(INT nIndex)
	{
		ASSERT( nIndex >= 0 && nIndex < X_MAX_TALENT_PER_ROLE );
		ASSERT( ETT_Null != m_Talent[nIndex].eType );

		return m_Talent[nIndex].nPoint;
	}

	//---------------------------------------------------------------------------------
	// 给天资加点
	//---------------------------------------------------------------------------------
	VOID AddTalentPoint(INT nIndex, INT nPoint=1)
	{
		ASSERT( nIndex >= 0 && nIndex < X_MAX_TALENT_PER_ROLE );
		ASSERT( ETT_Null != m_Talent[nIndex].eType );
		ASSERT( nPoint > 0 );

		m_Talent[nIndex].nPoint += nPoint;

		// 同步
		NET_SIS_update_talent send;
		send.eType = m_Talent[nIndex].eType;
		send.nPoint = m_Talent[nIndex].nPoint;
		SendMessage(&send, send.dw_size);
	}

	//---------------------------------------------------------------------------------
	// 被攻击
	//---------------------------------------------------------------------------------
	virtual VOID OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited);

	//--------------------------------------------------------------------
	// 被攻击时计算PK值
	//--------------------------------------------------------------------
	//VOID OnBeAttackedCalPK( Role* pAttacker );


	//---------------------------------------------------------------------------------
	// 死亡
	//---------------------------------------------------------------------------------
	virtual VOID OnDead(Unit* pSrc, Skill* pSkill=NULL, const tagBuffProto* pBuff=NULL, DWORD dwSerial=INVALID_VALUE, DWORD dwMisc=0, BOOL bCrit = FALSE);

	//---------------------------------------------------------------------------------
	// 击杀
	//---------------------------------------------------------------------------------
	virtual VOID OnKill(Unit* pSrc);

	//--------------------------------------------------------------------
	// 击杀后计算PK值 !返回值表示PK变化与否
	//--------------------------------------------------------------------
	BOOL OnKillCalPKVal( Role* pTarget );
	VOID OnKillCalJustice( Role* pTarget );

	//--------------------------------------------------------------------
	// PK惩罚
	//--------------------------------------------------------------------
	VOID PKPenalty( BOOL bIgnoreLoot = FALSE  );

	//---------------------------------------------------------------------------------
	// 是否进行死亡惩罚
	//---------------------------------------------------------------------------------
	BOOL IsDeadPenalty();

	//---------------------------------------------------------------------------------
	// 死亡惩罚
	//---------------------------------------------------------------------------------
	VOID DeadPenalty(Unit* pSrc);

	//---------------------------------------------------------------------------------
	// 增加杀人数
	//---------------------------------------------------------------------------------
	inline VOID IncKillNum(){ m_nKillNum++; }

	//---------------------------------------------------------------------------------
	// 帮会战分数计算
	//---------------------------------------------------------------------------------
	VOID GuildWarAchievement(Unit* pKill);
	VOID SetWarAchievement(EGuildMemberPos	eGuildPos, guild* pGuild);

	//---------------------------------------------------------------------------------
	// 目标的类型标志
	//---------------------------------------------------------------------------------
	virtual DWORD TargetTypeFlag(Unit* pTarget);

	//---------------------------------------------------------------------------------
	// 状态标志
	//---------------------------------------------------------------------------------
	virtual DWORD GetStateFlag()
	{
		DWORD dwStateFlag = Unit::GetStateFlag();

		dwStateFlag |= ( IsInRoleState(ERS_Mount)		?	ESF_Mount	:	ESF_NoMount );
		dwStateFlag |= ( IsInRoleState(ERS_PrisonArea)	?	ESF_Prison	:	ESF_NoPrison );
		dwStateFlag |= ( IsInRoleState(ERS_Commerce)	?	ESF_Commerce:	ESF_NoCommerce );
		dwStateFlag |= ( IsInRoleState(ERS_Hang)		?   ESF_Hang    :   ESF_NoHang);
		//dwStateFlag |= ( IsInRoleState(ERS_Prictice)	?	ESF_Prictice:	ESF_NoPrictice );

		return dwStateFlag;
	}

	//---------------------------------------------------------------------------------
	// 与目标的敌友中立判断
	//---------------------------------------------------------------------------------
	virtual DWORD FriendEnemy(Unit* pTarget);

	//----------------------------------------------------------------------------------
	// 触发器类型对应被动技能和装备触发器类型
	//----------------------------------------------------------------------------------
	EPassiveSkillAndEquipTrigger TriggerTypeToPassiveSkillAndEquipTriggerType(ETriggerEventType eType)
	{   
		switch(eType)
		{
		case ETEE_BeAttacked:
			return EPSAET_BeAttacked;
			break;

		case ETEE_Hit:
			return EPSAET_Hit;
			break;

		case ETEE_Hited:
			return EPSAET_Hited;
			break;

		case ETEE_Dodged:
			return EPSAET_Dodged;
			break;

		case ETEE_Dodge:
			return EPSAET_Dodge;
			break;

		case ETEE_Blocked:
			return EPSAET_Blocked;
			break;

		case ETEE_Block:
			return EPSAET_Block;

		case ETEE_Crit:
			return EPSAET_Crit;
			break;

		case ETEE_Crited:
			return EPSAET_Crited;
			break;

		case ETEE_Die:
			return EPSAET_Die;
			break;

		case ETEE_Random:
			return EPSAET_Random;
			break;

		case ETEE_Attack:
			return EPSAET_Attack;
			break;

		case ETEE_Kill:
			return EPSAET_Kill;
		
		case ETEE_Be_Dizzy:
			return EPSAET_Be_Dizzy;
		
		case ETEE_Kill_Player:
			return EPSAET_Kill_Player;

		default:
			return EPSAET_Null;
			break;
		}
	}

private:
	DWORD TargetTypeFlag(Role* pTarget);
	DWORD TargetTypeFlag(Creature* pTarget);
	DWORD FriendEnemy(Role* pTarget);
	DWORD FriendEnemy(Creature* pCreature);

private:
	//---------------------------------------------------------------------------------
	// 将角色数据保存到数据库相关
	//---------------------------------------------------------------------------------
	class SaveRole
	{
	public:
		SaveRole();
		~SaveRole();

		VOID Init();
		operator NET_DB2C_save_role*() { return m_pSaveRole; }

	private:
		NET_DB2C_save_role*	m_pSaveRole;
	};

	static SaveRole	m_SaveRole;
	static Mutex	m_SaveRoleLock;
	
	//---------------------------------------------------------------------------------
	// 复活相关
	//---------------------------------------------------------------------------------
	struct tagRevive
	{
		INT		nHP;
		INT		nMP;

		DWORD	dwMapID;
		FLOAT	fX;
		FLOAT	fY;
		FLOAT	fZ;

		tagRevive() { ZeroMemory(this, sizeof(*this)); }
	};

	tagRevive m_Revive;

	INT mCoolDownReviveCD;
	INT mCoolDownReviveTick;
	INT m_CoolDownReviveTick2;
	INT GetCoolDownReviveCD( );

	VOID UpdateCoolDownRevive( );
	VOID StartCoolDownRevive( );
public:
	VOID ResetCooldownCD(){ mCoolDownReviveCD = COOLDOWN_REVIVE_CD; }

private:
	//---------------------------------------------------------------------------------
	// 对应的session
	//---------------------------------------------------------------------------------
	PlayerSession*				m_pSession;

protected:
	//-------------------------------------------------------------------------------------
	// 时间相关
	//-------------------------------------------------------------------------------------
	tagDWORDTime		m_CreatTime;						// 创建时间
	tagDWORDTime		m_LoginTempTime;					// 临时存储登陆时间
	tagDWORDTime		m_LoginTime;						// 登陆时间
	tagDWORDTime		m_LogoutTime;						// 登出时间
	INT					m_nOnlineTime;						// 累计在线时间（秒）
	INT					m_nCurOnlineTime;					// 本次在线时间（秒）

	INT					m_nUnSetPVPTickCountDown;			// 关闭玩家PVP状态的Tick倒计时
	INT					m_nNeedSave2DBCountDown;			// 可以保存数据库倒计时

	tagDWORDTime		m_LastGetMallFreeTime;				// 上一次从商城获取免费物品时间
	tagDWORDTime		m_LeaveGuildTime;					// 离开帮会时间
	tagDWORDTime		m_dwMasterPrenticeForbidTime;		// (被)解除师徒关系时间

	//-------------------------------------------------------------------------------------
	// 挂机相关
	//-------------------------------------------------------------------------------------
	DWORD				m_dwHangTime;						// 挂机计时
	tagDWORDTime		m_dwHangBeginTime;					// 挂机开始时间
	INT16				m_n16HangNum;							// 挂机次数
	BOOL				m_bExp;								// 是否使用经验丹
	BOOL				m_bBrotherhood;						// 是否使用义气丹
	INT					m_nLeaveExp;						// 离线经验
	INT					m_nLeaveBrotherHood;				// 离线义气

	//-------------------------------------------------------------------------------------
	// 玩家当前状态
	//-------------------------------------------------------------------------------------
	RoleState			m_RoleState;						// 玩家状态	-- 变化后，需通知周围玩家
	RoleStateEx			m_RoleStateEx;						// 玩家状态 -- 变化后，只需要通知自己
	ERolePKState		m_ePKState;							// 当前PK状态
	INT					m_iPKValue;							// 当前PK值
	//BOOL				mIsPurpureDec;						
	package_map<DWORD,DWORD>	m_mapLastAttacker;					// 攻击者列表
	tagDWORDTime		m_dwLeaveCombatTime;				// 脱离战斗时间
	tagDWORDTime		m_dwPKValueDecTime;					// PK值扣减时间
	EBattleMode			m_eBattleMode;						// 战斗模式 
	DWORD				m_dwUseKillbadgeCD;					// 追杀令计时


	//-------------------------------------------------------------------------------------
	// 人物属性
	//-------------------------------------------------------------------------------------
	tagAvatarAtt		m_Avatar;							// 外观属性
	tagAvatarEquip		m_AvatarEquipEquip;					// 装备外观
	tagAvatarEquip		m_AvatarEquipFashion;				// 时装外观
	tagDisplaySet		m_DisplaySet;						// 装备显示设置
	EClassType			m_eClass;							// 职业
	EClassTypeEx		m_eClassEx;							// 扩展职业
	INT					m_nCredit;							// 信用度
	INT					m_nIdentity;						// 身份
	INT					m_nVIPPoint;						// 会员积分
	DWORD				m_dwGuildID;						// 所属帮派ID
	DWORD				m_dwSignRoleID;						// 签名对象
	DWORD				m_dwMasterID;
	INT						m_nGraduates;							// 是否出师
	DWORD				m_dwDestoryEquipCount;				// 摧毁装备次数
	TCHAR				m_szSignature_name[X_SHORT_NAME];	// 签名 
	//-------------------------------------------------------------------------------------
	// 物品冷却
	//-------------------------------------------------------------------------------------
	BOOL				m_bObjectCoolOff;					// 行囊内的物品是否冷却

	//-------------------------------------------------------------------------------------
	// 禁言
	//-------------------------------------------------------------------------------------
	DWORD		m_dwForbidTalkStart;
	DWORD		m_dwForbidTalkEnd;

	//-------------------------------------------------------------------------------------
	// 属性点和技能点
	//-------------------------------------------------------------------------------------
	INT					m_nAttPointAdd[X_ERA_ATTA_NUM];		// 投入各种一级属性的属性点
	tagRoleTalent		m_Talent[X_MAX_TALENT_PER_ROLE];	// 天资及各自天资点

	//-------------------------------------------------------------------------------------
	// 复活相关
	//-------------------------------------------------------------------------------------
	DWORD				m_dwRebornMapID;					// 复活地图ID
	
						
	//-------------------------------------------------------------------------------------
	// 被动技能的触发器索引表
	//-------------------------------------------------------------------------------------
	std::set<DWORD>		m_setPassiveSkillTrigger[EPSAET_End];

	//-------------------------------------------------------------------------------------
	// 装备的触发器索引表
	//-------------------------------------------------------------------------------------
	typedef std::bitset<X_EQUIP_BAR_SIZE>	BitSetEquipPos;		/*n16EquipPos*/
	typedef std::set<DWORD>					SetSuitTrigger;		/*dwSuitID*/

	BitSetEquipPos		m_bitsetEquipTrigger[EPSAET_End];
	SetSuitTrigger		m_setSuitTrigger[EPSAET_End];

	//-------------------------------------------------------------------------------------
	// 金钱
	//-------------------------------------------------------------------------------------
	CurrencyMgr			m_CurMgr;

	//-------------------------------------------------------------------------------------
	// 物品管理器 -- 管理行囊、任务页、装备栏、角色仓库和百宝袋等
	//-------------------------------------------------------------------------------------
	ItemMgr				m_ItemMgr;							// 物品管理器

	//-------------------------------------------------------------------------------------
	// 套装管理
	//-------------------------------------------------------------------------------------
	Suit				m_Suit;

	//-------------------------------------------------------------------------------------
	// 玩家间交易相关
	//-------------------------------------------------------------------------------------
	ExchangeMgr			m_ExchMgr;

	//-------------------------------------------------------------------------------------
	// 摆摊
	//-------------------------------------------------------------------------------------
	stall				*m_pStall;

	//-------------------------------------------------------------------------------------
	// 好友相关
	//-------------------------------------------------------------------------------------
	tagFriend						m_Friend[MAX_FRIENDNUM];				// 好友列表	
	package_map<DWORD, tagFriend*>			m_mapFriend;
	DWORD							m_dwBlackList[MAX_BLACKLIST];			// 黑名单
	DWORD							m_dwEnemyList[MAX_ENEMYNUM];			// 仇人

	//-------------------------------------------------------------------------------------
	// 团队相关
	//-------------------------------------------------------------------------------------
	DWORD				m_dwTeamID;											// 小队ID
	DWORD				m_dwGroupID;										// 团队ID
	DWORD				m_dwTeamInvite;										// 邀请人ID
	BOOL				m_bTeamSyn;											// 小队玩家状态同步标志
	BOOL				m_bLeader;

	//-------------------------------------------------------------------------------------
	// 副本相关
	//-------------------------------------------------------------------------------------
	DWORD				m_dwOwnInstanceMapID;								// 玩家所创建的副本的地图ID
	DWORD				m_dwOwnInstanceID;									// 玩家创建的副本ID
	tagDWORDTime		m_dwInstanceCreateTime;								// 副本创建时间
	DWORD				m_dwExportMapID;									// 传出副本时的地图ID
	Vector3				m_vExport;											// 传出副本时地图的坐标		

	//-------------------------------------------------------------------------------------
	// 对远端玩家公开信息设置
	//-------------------------------------------------------------------------------------
	tagRemoteOpenSet	m_sRemoteOpenSet;

	//-------------------------------------------------------------------------------------
	// 玩家脚本
	//-------------------------------------------------------------------------------------
	const RoleScript*	m_pScript;											// 玩家脚本

	//-------------------------------------------------------------------------------------
	// 角色开启宝箱计数
	//-------------------------------------------------------------------------------------
	INT					m_nTreasureSum;										// 玩家已开启的宝箱数
	
	//-------------------------------------------------------------------------------------
	// 开宝箱状态
	//-------------------------------------------------------------------------------------
	tagChestState		m_TreasureState;

	//-------------------------------------------------------------------------------------
	// 武器修炼
	//-------------------------------------------------------------------------------------
	PracticeMgr			m_PracticeMgr;

	//-------------------------------------------------------------------------------------
	// 进入地图限制
	//-------------------------------------------------------------------------------------
	MapLimitMap			m_mapMapLimit;

	//-------------------------------------------------------------------------------------
	// 当前杀人数
	//-------------------------------------------------------------------------------------
	INT			m_nKillNum;

	//-------------------------------------------------------------------------------------
	// 当前兑换次数
	//-------------------------------------------------------------------------------------
	INT			m_nYuanbaoExchangeNum;
	//-------------------------------------------------------------------------------------
	// 成就点数
	//-------------------------------------------------------------------------------------
	INT			m_nAchievementPoint;
public:
	// 获得元宝兑换次数
	VOID	ClearYuanBaoExchangeNum() { m_nYuanbaoExchangeNum = 0; }
	INT&	GetYuanbaoExchangeNum() { return m_nYuanbaoExchangeNum; }
	// 获取成就点数
	INT		GetAchievementPoint() { return m_nAchievementPoint; }
	VOID	ModAchievementPoint(INT nAdd, BOOL bSend = TRUE);

	//-------------------------------------------------------------------------------------
	// 得到好友个数
	//-------------------------------------------------------------------------------------
	INT  GetFriendCount() { return m_mapFriend.size(); }
	//-------------------------------------------------------------------------------------
	// 得到黑名个数
	//-------------------------------------------------------------------------------------
	INT GetBlackCount()	
	{
		INT nCount = 0;
		for(INT i = 0; i < MAX_BLACKLIST; i++)
		{
			if(m_dwBlackList[i] == INVALID_VALUE)
				continue;
			nCount++;
		}
		return nCount;
	}

	//-------------------------------------------------------------------------------------
	// 得到仇人个数
	//-------------------------------------------------------------------------------------
	INT GetEnemyCount();

public:
	// 初始化当前任务
	VOID init_current_quest(const BYTE* &p_data, INT32 n_num);

	// 初始化所有已完成任务
	VOID init_complete_quest(const BYTE* &p_data, INT32 n_num);

	// 通过NPC处接取任务
	INT accept_quest_from_npc(UINT16 quest_id, DWORD dw_npc);
	
	// 通过触发器接取任务
	INT accept_quest_from_trigger(UINT16 quest_id, DWORD trigger_id);

	// 是否可以接取任务
	INT accept_quest_check(UINT16 quest_id, INT& n_index, Creature* p_npc=NULL, BOOL IDSpcial = FALSE);

	// 增加任务
	BOOL add_quest(const tagQuestProto* p_proto, INT n_index, BOOL IDSpcial = FALSE);

	// 添加或删除一个已完成任务
	VOID add_del_complete_quest(UINT16 quest_id, BOOL b_done)
	{
		// 将指定任务加入已完成任务
		if( b_done )
		{
			if( done_quest_.is_exist(quest_id) ) return;

			tagQuestDoneSave* p_done = new tagQuestDoneSave;
			p_done->u16QuestID = QuestIDHelper::RestoreID(quest_id);
			p_done->nTimes = 1;
			p_done->dwStartTime = g_world.GetWorldTime();

			done_quest_.add(quest_id, p_done);

			// 向客户端发送消息
			NET_SIS_gm_command_state send;
			send.u16QuestID = quest_id;
			send.bDone = TRUE;
			SendMessage(&send, send.dw_size);
		}
		else // 令指定任务为未完成任务
		{		
			tagQuestDoneSave* p_done = done_quest_.find(quest_id);
			if( VALID_POINT(p_done) )
			{
				done_quest_.erase(quest_id);
				SAFE_DELETE(p_done);
			}

			// 向客户端发送消息
			NET_SIS_gm_command_state send;
			send.u16QuestID = quest_id;
			send.bDone = FALSE;
			SendMessage(&send, send.dw_size);
		}
	}



	// 完成任务
	INT complete_quest(UINT16 quest_id, DWORD dw_npc, INT choice_index, UINT16& next_quest);
	// WARNING !!!!! 悬赏任务专用 (!!!! 任何系统都不能调用此函数，会有多线程问题)
	INT complete_quest_ex(UINT16 quest_id, DWORD dw_npc, INT choice_index);

	// 完成检查
	INT complete_quest_check(quest* p_quest, DWORD dw_npc, INT choice_index);

	// 任务奖励
	VOID reward_quest(quest* p_quest, INT32 choice_index);

	// 删除任务
	INT delete_quest(UINT16 quest_id);

	//  no_del_complete_item 悬赏任务系统，会造成一个玩家接同一任务两次
	// 在这种情况下，交付悬赏任务时不能删物品
	VOID remove_quest(UINT16 quest_id, BOOL b_complete, BOOL no_del_complete_item = FALSE);

	// 更新NPC对话任务
	VOID update_quest_npc_talk(UINT16 quest_id, DWORD dw_npc);

	// 更新Trigger任务
	VOID update_quest_trigger(UINT16 quest_id, DWORD trigger_id);

	// 触发任务事件
	VOID on_quest_event(EQuestEvent quest_event, DWORD param1=0, DWORD param2=0, DWORD param3=0);

	// 服务器可控对话框缺省事件
	VOID on_default_dialog_event(EMsgUnitType unit_type, DWORD target_id, EDlgOption option);

	// 清空所有已完成任务
	VOID clear_done_quest()
	{
		tagQuestDoneSave* p_done = NULL;
		QuestDoneMap::map_iter it = done_quest_.begin();
		while( done_quest_.find_next(it, p_done) )
		{
			// 向客户端发送消息
			NET_SIS_gm_command_state send;
			send.u16QuestID = p_done->u16QuestID;
			send.bDone = FALSE;
			SendMessage(&send, send.dw_size);

			SAFE_DELETE(p_done);
		}
		done_quest_.clear();
	}



	// 检测是否做过某任务
	BOOL is_done_quest(UINT16 quest_id) { return done_quest_.is_exist(quest_id); }


	// 得到指定任务是否存在
	BOOL is_have_quest(UINT16 quest_id) { return current_quest_.is_exist(quest_id); }


	// 得到完成任务的个数
	INT GetCompleteQuestCount() { return done_quest_.size(); }


	// 得到任务数
	INT get_current_quest_count() { return current_quest_.size(); }


	// 得到任务完成次数
	INT get_done_quest_times(UINT16 quest_id)
	{
		tagQuestDoneSave* p_done = done_quest_.find(quest_id);
		return VALID_POINT(p_done) ? p_done->nTimes : INVALID_VALUE;
	}
	//周期任务零点完成次数清零 gx add 2013.7.15
	VOID day_clear_circle_quest_done_times();

	// 已任务接完成的接取时间
	DWORD get_done_quest_accept_time(UINT16 quest_id)
	{
		tagQuestDoneSave* p_done = done_quest_.find(quest_id);
		return VALID_POINT(p_done)? p_done->dwStartTime : INVALID_VALUE;
	}

	// 得到指定任务
	quest* get_quest(UINT16 quest_id) { return current_quest_.find(quest_id); }
	quest* get_quest_by_index(INT n_index){ return &quests_[n_index]; }

	INT set_track_number_mod(INT delta){ 
		current_track_number_ += delta;
		ASSERT( current_track_number_>=0 && current_track_number_<= QUEST_TRACK_MAX);
		if(current_track_number_<0) current_track_number_ = 0;
		if(current_track_number_>QUEST_TRACK_MAX) current_track_number_ = QUEST_TRACK_MAX;
		return current_track_number_;
	}
	BOOL can_track_quest() const { return current_track_number_ < QUEST_TRACK_MAX;}
	INT quest_track_number() const{ return current_track_number_;}

	INT set_track_quest(UINT16 quest_id, BYTE by_track);

	//  完成循环随机任务后继续随机
	VOID rand_circle_quest();
	VOID auto_rand_team_share_quest_after_complete_quest(UINT16 u16QuestID);
	VOID auto_rand_circle_quest_after_complete_quest(UINT16 u16QuestID); 
	VOID auto_rand_circle_quest_to_index(UINT16 u16Old);
	const CircleQuestMan& get_circle_quest_man() const { return circle_quest_man_; }
	VOID SendCirleQuest( );
	DWORD BuyRefreshCircleQuest( );
	VOID BuyCircleQuestPerdayNumber( );
// ========================>
//mwh 2011-09-06 
private:
	INT rand_circle_quest_one();
	VOID do_rand_circle_quest_from_person();
private:
	BOOL is_leader_set_share() const;
// <========================
private:
	CircleQuestMan  circle_quest_man_;
	INT				current_track_number_;
	quest			quests_[QUEST_MAX_COUNT];		// 玩家当前的任务
public:
	BOOL			quest_valid(INT n_index){return quests_[n_index].valid();}
private:
	QuestMap		current_quest_;			// 玩家当前的任务
	QuestDoneMap	done_quest_;			// 已经完成的任务集合
public:

	// 成就
	AchievementMgr& GetAchievementMgr() { return m_achievementMgr; }
	// 称号相关
	TitleMgr*		GetTitleMgr() { return m_pTitleMgr; }
	//角色名贴相关
	tagRoleVCard&	GetVCard() { return m_VCard; }
	//氏族信息
	ClanData&		GetClanData(){	return m_ClanData;}
	// 宠物带
	PetPocket*		GetPetPocket() { return m_pPetPocket; }
	// 坐骑
	RaidMgr&		GetRaidMgr() { return m_RaidMgr; }
private:
	AchievementMgr	m_achievementMgr;
	TitleMgr*		m_pTitleMgr;
	tagRoleVCard	m_VCard;
	ClanData		m_ClanData;
	PetPocket*		m_pPetPocket;
	RaidMgr			m_RaidMgr;
public:	
	VOID OnLeaveMap();
	void BreakMount();
	VOID OnEnterMap(Map* pScrMap);
public:
	// 世界喊话，并重置时间
	BOOL TalkToWorld();
	BOOL talk_to_map();
private:
	INT m_nWorldTalkCounter;
	VOID UpdateTalkToWorld();

	INT map_talk_counter_;
	void update_talk_to_map();

	INT m_nAddFriendValueCounter;
	VOID UpdateAddFreindValue();
public:

	FLOAT CalFriendExpPer(const Team* pTeam);
		
public:
	// 拾取物品
	DWORD PickUpItem(INT64 n64GroundID);
	// 放下物品
	DWORD putdown_item(INT64 n64_serial, BYTE by_type);
public:
	typedef MoveData::EMoveRet (MoveData::*PFMoveAction2P)(const Vector3& v1, const Vector3& v2);
	typedef MoveData::EMoveRet (MoveData::*PFMoveAction1P)(const Vector3& v);

	MoveData::EMoveRet	MoveAction(PFMoveAction2P pAction, Vector3& v1, Vector3& v2);
	MoveData::EMoveRet	MoveAction(PFMoveAction1P pAction, Vector3& v);

	//------------------------------------------------------------------------------
	// 玩家间宠物交易相关
	//------------------------------------------------------------------------------
	PetExchangeMgr& GetPetExchMgr() { return m_PetExchMgr; }

	BOOL	IsPetExchanging()					{ return IsInRoleState(ERS_PetExchange); }
	BOOL	CanPetExchange()
	{
		return !IsInRoleStateAny(ERS_Exchange | ERS_Shop | ERS_Stall | ERS_PetExchange) 
			&& (GetPetExchMgr().GetTgtRoleID() == INVALID_VALUE);
	}

	VOID	BeginPetExchange(DWORD dwTgtRoleID)
	{
		SetRoleState(ERS_PetExchange);
		GetPetExchMgr().CreateData();
		GetPetExchMgr().SetTgtRoleID(dwTgtRoleID);
	}

	VOID	EndPetExchange()
	{
		UnsetRoleState(ERS_PetExchange);
		GetPetExchMgr().DeleteData();
		GetPetExchMgr().SetTgtRoleID(INVALID_VALUE);
	}

	DWORD	ProcPetExchangeReq(OUT Role* &pTarget, DWORD dwTgtRoleID);
	DWORD	ProcPetExchangeReqRes(OUT Role* &pApplicant, DWORD dwTgtRoleID, DWORD dw_error_code);
	DWORD ProcPetExchangeAdd(OUT Role* &pTarget, DWORD dwPetID);
	DWORD ProcPetExchangeDec(OUT Role* &pTarget, DWORD dwPetID);
	DWORD	ProcPetExchangeMoney(OUT Role* &pTarget, INT64 n64Silver);
	DWORD	ProcPetExchangeLock(OUT Role* &pTarget);
	DWORD	ProcPetExchangeCancel(OUT Role* &pTarget);
	DWORD	ProcPetExchangeVerify(OUT Role* &pTarget, OUT DWORD &dwFailedRoleID);

	DWORD VerifyPetExchangeData();
	DWORD	ProcPetExchange();

	DWORD	ProcPreparePetExchange(OUT Role* &pTarget);

private:
	PetExchangeMgr m_PetExchMgr;
	//	角色开启宝箱计数相关
public:

	//	获取角色开启宝箱计数
	INT GetTreasureSum()		{ return m_nTreasureSum; }
	//	增加宝箱计数
	VOID IncTreasureSum();
	//	初始化宝箱相关
	VOID InitChestState();
	//	设置宝箱开启数
	VOID SetTreasureSum(INT nSum);

	VOID StopMount();

public:
	// 金牌网吧经验加成
	FLOAT	GetVNBExpRate();
	// 金牌网吧掉率加成
	FLOAT	GetVNBLootRate();
	
public:
	// GM命令调用的几个函数
	DWORD AddSuit(DWORD dwSuitID, INT nQlty);
	DWORD AddEquip(INT nType, INT nLevel, INT nQlty);
public:
	//重置是否有留言消息
	VOID	ResetHasLeftMsg()
	{
		m_bHasLeftMsg = FALSE;
	}
private:
	BOOL	m_bHasLeftMsg;

public:
	// 是否可以采集
	INT CanGather(Creature* pRes);

public:
	//是否可以播放个性动作
	DWORD	CanCastMotion(Unit* pDest, DWORD dwActionID);
	// 播放个性动作
	DWORD	CastMotion(Unit* pDest, DWORD dwActionID);

	// 判断dw_role_id有没有权利进行交互
	BOOL	GetMotionInviteStatus(DWORD dw_role_id);
	// 玩家是否可以接受或者邀请标记
	VOID	SetMotionInviteStatus(BOOL bSet, DWORD dw_role_id);
	// 更新状态
	VOID	UpdateMotionInviteState();
private:
	INT		m_nMotionCounter;
	DWORD	m_dwPartnerID;

	INT	m_nSendMailNum;			//  发送邮件的数量
public:
	//保存经验到db
	VOID	SaveExp2DB();
	//保存属性点到db
	VOID SaveAttPoint2DB();
	//保存天资点到db
	VOID	SaveTalentPoint2DB(INT nIndex);
	//保存等级到db
	VOID	SaveLevel2DB();
	// 保存义气到db
	VOID	SaveBrotherhood2DB();
	// 保存爱心值
	VOID	SaveLove2DB();
	// 保存武魂
	VOID	SaveWuhuen2DB();
//--------------------------------------------------------------------
//	坐骑相关 
//--------------------------------------------------------------------
public:
	//强化坐骑
	DWORD upgrade_ride( DWORD dw_npc, INT64 ride_serial, BYTE number, BOOL useBind = FALSE);
	//上装备
	DWORD ride_inlay(INT64 ride_serial, INT64* ride_equip, INT equip_number);
	//移除镶嵌
	DWORD remove_ride_inlay(DWORD dw_npc, INT64 ride_serial, INT64 crush_stone, BYTE index );
	//骑乘
	VOID StartRideSpell( );
	INT begin_ride_op( );
	void ride_buff_op(tagEquip* p_ride, bool add = true);
	INT GetMountSpellTime( );
	//取消骑乘
	DWORD cancel_ride( );
	//VOID cancel_ride_ex( );
	VOID InterruptRide();
	// 设置骑乘状态
	VOID set_mount(BOOL b_set, INT n_speed, const tagPetProto* p_proto);
	VOID set_mount_ex(BOOL b_set, INT n_speed, const tagPetProto* p_proto);
	// 设置/获取骑乘数据
	VOID set_ride_data(INT64 n64_serial, DWORD dw_data_id, BYTE level, INT speed);
	DWORD get_ride_type_id( ) const{ return ride_data_.dwRideTypeID; }
	INT64 get_ride_serial() { return ride_data_.n64_serial; }
	BYTE  get_ride_level( ) const { return ride_data_.bySolidateLevel; }
	INT	  get_ride_speed() const { return ride_data_.nSpeed; }
	VOID update_ride( );
	VOID cal_ride_speed( tagEquip* p_equip );
	BOOL ride_limit(tagEquip* pRide);
	// 装备坐骑
	DWORD EquipRide(INT64 n64Ride);
	DWORD UnEquipRide( );
	VOID DealMountSkill(BOOL bSet);

	VOID ModAttacStopMountProb(INT delta)
	{
		m_AttackStopMountProb += delta;
		if(m_AttackStopMountProb < 0) m_AttackStopMountProb = 0;
	}


	VOID SetAttacStopMountProb(INT val) {m_AttackStopMountProb = val; }
	BOOL GetAttacStopMountProb( ) const { return m_AttackStopMountProb;}

	BOOL AttackProbStopMount( );
	BOOL GetAttackStopMountProtect() const { return m_AttackStopMountProtect; }
	void SetAttackStopMountProtect(BOOL val) { m_AttackStopMountProtect = val; }
private:
	struct{
		INT64 n64_serial;
		DWORD dwRideTypeID;
		BYTE  bySolidateLevel;
		INT	 nSpeed;
	}ride_data_;
	INT m_AttackStopMountProb;
	BOOL m_AttackStopMountProtect;
public:
	VOID set_master_prentice_forbid_time( );
	DWORD get_master_prentice_forbid_time( ){ return (DWORD)m_dwMasterPrenticeForbidTime;};
	DWORD get_master_id( VOID ) const {return m_dwMasterID;}
	VOID set_master_id( DWORD dwID );
	BOOL is_graduates_from_master( ) const { return m_nGraduates > 0; }
	VOID inc_graduates_from_master( ) { ++m_nGraduates;}
public:
	const RoleScript* GetScript( ) const { return m_pScript; }
private:
	DWORD mCarryTypeID;
public:
	VOID SetCarrryID(DWORD dwID);
	DWORD GetCarryID( ) const ;
public:
	// 人物帮助数据
	BYTE	m_byRoleHelp[ROLE_HELP_NUM];
	VOID	SendRoleHelp();
	VOID	SetRoleHelp(BYTE byIndex);

	// 人物对话数据
	BYTE	m_byTalkData[TALK_DATA_NUM];
	VOID	SendRoleTalk();
	VOID	SetRoleTalk(BYTE byIndex, BYTE byState);
	
	// 每天清零数据
	BYTE	m_byDayClear[ROLE_DAY_CLEAR_NUM];
	VOID	SendRoleDayClear();
	VOID	SetRoleDayClearData(BYTE byIndex, BYTE byData);
	VOID	ModRoleDayClearDate(BYTE byIndex, BYTE byTime = 1, BOOL bSnd = TRUE);
	BYTE	GetDayClearData(BYTE byIndex) { return m_byDayClear[byIndex]; }
	VOID	ResetDayClearData() { memset(m_byDayClear, 0, sizeof(m_byDayClear)); SendRoleDayClear( ); }

	// 快捷键数据
	//tagKeyInfo m_stKeyInfo[MAX_KEY_NUM];
	roleOnlineState m_stKeyInfo;
	VOID	SendKeyInfo();
	VOID	SetKeyInfo(roleOnlineState* pKeyInfo);

	VOID	SendReceiveType();

	VOID	Send1v1ScoreInfo();

	INT32	m_n32_active_num;							// 活跃度
	INT32	m_n32_active_data[MAX_ACTIVE_DATA];		// 活跃度数据
	BOOL	m_b_active_receive[MAX_ACTIVE_RECEIVE];	// 活跃度领奖数据
	
	//帮会活跃度
	INT32	m_n32_guild_active_num;							// 活跃度
	INT32	m_n32_guild_active_data[MAX_ACTIVE_DATA];		// 活跃度数据
	BOOL	m_b_guild_active_receive[MAX_GUILD_ACTIVE_RECEIVE];	// 活跃度领奖数据


	// 副本扫荡相关
	INT32	m_nInstanceData;		// 副本是否打过了
	tagDWORDTime m_nInstanceShaodangTime; // 副本扫荡开始时间
	int		m_nShaodangIndex;		// 当前扫荡副本

	VOID	SendActiveInfo();
	DWORD   ActiveReceive(INT nIndex);
	VOID	ResetActive();
	DWORD	ActiveDone(INT nIndex,INT nBeishu = 1);	
	VOID	DailyActTransmit(INT nIndex);//每日活动一键传送 gx add 2013.12.18

	VOID	SendGuildActiveInfo();
	DWORD   GuildActiveReceive(INT nIndex);
	VOID	ResetGuildActive();
	
	VOID	SendConsumeReward();
	// 新手礼包
public:
	s_new_role_gift m_stNewRoleGift;

	VOID	ResetNewRoleGift();
	VOID	initRoleGift(INT nStep, DWORD dwLeftTime);
	VOID	CreateNewRoleGift();
	VOID	SendGiftInfo();
	VOID	UpdateNewRoleGift();
	DWORD	GetNewRoleGift();

private:
	INT		m_nTotalMasterMoral;
public:
	VOID SetMasterMoralMod( INT nMod ){ m_nTotalMasterMoral += nMod;}
	INT	 GetMasterMoralMod( ) const { return m_nTotalMasterMoral; }

private:
	ECamp		e_role_camp;			// 角色阵营
	ECamp		e_temp_role_camp;		// 角色临时阵营

public:
	ECamp		get_role_camp()		{ return e_role_camp; };
	ECamp		get_temp_role_camp()	{ return e_temp_role_camp; }

	VOID		set_role_camp(ECamp e_camp_)		{ e_role_camp = e_camp_; }
	VOID		set_temp_role_camp(ECamp e_camp_)	{ e_temp_role_camp = e_camp_; }

public:
	// 发送角色的拍卖信息
	VOID	send_paimai_info();

	// 发送角色竞拍信息
	VOID	send_jingpai_info();

	package_list<DWORD>&		get_query_paimai() { return list_query_paimai; }

	INT		get_paimai_limit() { return n_paimai_limit; }
	VOID	inc_paimai_limit()	{ n_paimai_limit++; }
	VOID	zero_paimai_limit() { n_paimai_limit = 0; }

private:
	package_list<DWORD>		list_query_paimai;
	INT						n_paimai_limit;
public:
	// 发送角色钱庄拍卖信息
	VOID	send_bank_paimai_info();

	package_list<DWORD>&	get_query_bank() { return list_query_bank; }
	package_list<DWORD>&	get_query_bank_ex() { return list_query_bank_yuanbao; }

	INT		get_bank_limit() { return n_bank_limit; }
	VOID	inc_bank_limit() { n_bank_limit++; }
	VOID	zero_bank_limit() { n_bank_limit = 0; }

private:
	package_list<DWORD>	list_query_bank;
	package_list<DWORD> list_query_bank_yuanbao;
	INT					n_bank_limit;
public:
	VOID send_vigour_reward(BOOL bFirstLogIn = FALSE);
	VOID reset_vigour();

	VOID update_vigour();
	
	VOID  inc_today_online_tick(){++dw_today_online_tick_;}
	VOID  set_today_online_tick(DWORD tick){dw_today_online_tick_=tick;}
	DWORD get_today_online_tick() const{return dw_today_online_tick_;}
	
	VOID  set_history_vigour_cost_ex(INT delta){ dw_history_vigour_cost_ += delta; }
	VOID  set_history_vigour_cost(INT val){ dw_history_vigour_cost_ = val; }
	DWORD get_history_vigour_cost() const {return dw_history_vigour_cost_;}
private:
	DWORD dw_today_online_tick_;
	DWORD dw_history_vigour_cost_;
	DWORD m_PerdayGetVigourTotal;
public:

	VOID	inc_exbag_stet(DWORD dw_type) 
	{
		if(dw_type == 0)
			n16_exbag_step++; 
		if(dw_type == 1)
			n16_exware_step++;
	}
	DWORD	exbag_check(DWORD dw_type, INT n_level);
	INT16	get_ware_step() { return n16_exware_step; }
//===> 切磋系统
public:
	INT  AskForDuel(DWORD dwTarget);
	BOOL WhenDeadDuelDeal(Unit* pKiller);
	BOOL PVPDead(Unit* pKiller);
	BOOL PVPBiWuDead(Unit* pKiller);
	BOOL Dead1v1(Unit* pKiller);
	VOID DuelResponse(DWORD dwTarget, BYTE byAck);
private:
	VOID UpdateDuel();
	VOID UpdatePrepare();
	VOID UpdateOperate();
	VOID EndDuel();
	INT CheckDuelLimitSelf(Role* pTarget, BOOL bSecond = FALSE);
	INT CheckDuelLimitTarget(Role* pTarget, BOOL bSecond = FALSE);
	BOOL IsInDuelDistance();
	VOID InitDuel();
	VOID DealDuelBanner(BOOL bShow);
public:
		BOOL IsDuel() const { return mIsDuel; }
private:
	VOID WhenDeadDealDuelHPMP();
	VOID DuelBroadcast(DWORD dwWinner, DWORD dwLoser);
	DWORD GetDuelTarget() const { return mDuelTarget; }
	VOID SetDuelTarget(DWORD val) { mDuelTarget = val; }
	VOID SetDuel(BOOL val) { mIsDuel = val; }
	BOOL IsDuelPrepare() const { return mIsDuelPrepare; }
	VOID SetDuelPrepare(BOOL val) { mIsDuelPrepare = val; }
	BOOL IsDuelOperate() const { return mIsDuelOperate; }
	VOID SetDuelOperate(BOOL val) { mIsDuelOperate = val; }
	DWORD GetDuelTickDown() const { return mDuelTickDown; }
	VOID SetDuelTickDown(DWORD val) { mDuelTickDown = val; }
	DWORD GetDueBannerID() const { return mDueBannerID; }
	VOID SetDueBannerID(DWORD val) { mDueBannerID = val; }
	Vector3 GetBannerPos() const { return mBannerPos; }
	VOID SetBannerPos(Vector3 val) { mBannerPos = val; }
private:
	BOOL  mIsDuel;
	BOOL  mIsDuelPrepare;
	BOOL  mIsDuelOperate;
	DWORD mDuelTickDown;
	DWORD mDuelTarget;
	DWORD mDueBannerID;
	Vector3 mBannerPos;
	// 切磋系统<===

//===========> 钓鱼
public:
	INT StartFishing(DWORD dwNpc, DWORD dwSkill, INT64 n64FishRod, INT64 n64Bait, BOOL bAutoFish);
	INT StopFishing();
	VOID UpdateFishing();
private:
	VOID InitFishing();
	VOID InitFishing(DWORD dwNpc, INT64 n64FishRod, INT64 n64Bait, BOOL bAutoFish);
	INT FishingCheckBasic(INT64 n64FishRod, INT64 n64Bait, DWORD dwNpc, DWORD dwSkill);
	VOID SetFishingCD();
	BOOL ProbGetFish();
	BOOL ProbBaitGet();
	VOID SetDecVigourCD();
private:
	INT64 mBait;					//	 鱼饵
	INT64 mFishingRod;		//  鱼竿
	INT mFishingTick;
	BOOL mAutoFish;
	DWORD mFishNpc;
	INT mFishDecVigourTick;
// 钓鱼 <===========
private:
	INT16	n16_exbag_step;
	INT16 n16_exware_step;
// 自动打怪
public:
	VOID UpdateAutoKill();
	VOID SetAutoKill(BOOL b){ mIsAutoKill = b; }
	BOOL GetAutoKill( ) const { return mIsAutoKill; }
	DWORD CanAutoKill( );
private:
	BOOL mIsAutoKill;
	INT32 mAutoKillTickDecVigour;
public:
	void SetCirleRefresh(INT n)
	{
		mnCirleRefresh = n; 
 		if(mnCirleRefresh>(CIRCLEQUESTFRESHNUMBER) )
 			mnCirleRefresh = CIRCLEQUESTFRESHNUMBER; 
	}

	void IncCirleRefresh(INT Delta)
	{
		mnCirleRefresh += Delta;
 		if(mnCirleRefresh>CIRCLEQUESTFRESHNUMBER) 
 			mnCirleRefresh = CIRCLEQUESTFRESHNUMBER; 
	}

	void DecCirleRefresh(INT Delta = 1)
	{
		mnCirleRefresh -= Delta;
		if(mnCirleRefresh < 0) 
			mnCirleRefresh = 0; 
	}

	INT GetCircleRefrshNumber( ) { return mnCirleRefresh; }


	void SetCirleRefreshMax(INT n)
	{
		mnCirleRefreshMax = n; 
		if(mnCirleRefreshMax>CIRCLEQUEST_FRESH_NUMBERMAXDAY) 
			mnCirleRefreshMax = CIRCLEQUEST_FRESH_NUMBERMAXDAY; 
	}

	void IncCirleRefreshMax(INT Delta)
	{
		mnCirleRefreshMax += Delta;
		if(mnCirleRefreshMax>CIRCLEQUEST_FRESH_NUMBERMAXDAY) 
			mnCirleRefreshMax = CIRCLEQUEST_FRESH_NUMBERMAXDAY; 
	}

	void DecCirleRefreshMax(INT Delta = 1)
	{
		mnCirleRefreshMax -= Delta;
		if(mnCirleRefreshMax < 0) 
			mnCirleRefreshMax = 0; 
	}

	INT GetCircleRefrshNumberMax( ) { return mnCirleRefreshMax; }

	void SetCirlePerdayNumber(INT n)
	{
		m_CirclePerDayNum = n; 
		if(m_CirclePerDayNum>CIRCLEQUESTPERDAYNUMBER) 
			m_CirclePerDayNum = CIRCLEQUESTPERDAYNUMBER; 
	}

	void IncCirlePerdayNumber(INT Delta)
	{
		m_CirclePerDayNum += Delta;
		if(m_CirclePerDayNum>CIRCLEQUESTPERDAYNUMBER) 
			m_CirclePerDayNum = CIRCLEQUESTPERDAYNUMBER; 
	}

	void DecCirlePerdayNumber(INT Delta = 1)
	{
		m_CirclePerDayNum -= Delta;
		if(m_CirclePerDayNum < 0) 
			m_CirclePerDayNum = 0; 
	}

	INT GetCirclePerdayNumber( ) { return m_CirclePerDayNum; }
private:
	INT32 mnCirleRefresh;
	INT32 mnCirleRefreshMax;
	INT32 m_CirclePerDayNum;
public:
	package_list<DWORD>& get_list_guild_recruit() { return list_guild_recruit; }
public:
	DWORD		GetTotalRecharge () const { return VALID_POINT(m_pSession) ? m_pSession->GetTotalRecharge() : 0; }
private:
	INT mGetExpTickUpdate;
	INT mPerDayHangGetExpTimeMS;
	INT m_HangDecVigourTick;
	VOID UpdateHangGetExp( );
	VOID UpdateHangVigour( );
public:
	DWORD StartHangGetExp(INT type);
	DWORD StopHangGetExp( );
	FLOAT GetDancingRate( );
	VOID SendDacingLeftTime( );
	VOID ResetPerDayHangGetExpTime( ) { mPerDayHangGetExpTimeMS = DAY_HANG_GETEXP_TIME_MS; }
	VOID AddPerDayHangGetExpTime( INT millisecond )  { mPerDayHangGetExpTimeMS += millisecond; }
	VOID SetPerDayHangGetExpTime( INT millisecond )  { mPerDayHangGetExpTimeMS = millisecond; }
	INT  GetPerDayHangGetExpTime( ) const { return mPerDayHangGetExpTimeMS; }
private:
	package_list<DWORD> list_guild_recruit;

	// 武宠合体
	Role* m_pTargetPetMonster; //目标宠物主人
	DWORD m_dwTargetPetID;	 //目标宠物ID

	DWORD m_dwCurSkillTarget;		//当前释放技能的目标
public:
	VOID SetTargetPet(Role* pMonster, DWORD dwID) { m_pTargetPetMonster = pMonster; m_dwTargetPetID = dwID; }
	Role* GetTargetPet(DWORD& dwID) { dwID = m_dwTargetPetID; return m_pTargetPetMonster; }

	VOID SetCurSkillTarget(DWORD dwID) { m_dwCurSkillTarget = dwID;}
	DWORD GetCurSkillTarget() { return m_dwCurSkillTarget; }

	BOOL HasCallPet();

	// 宠物sns
	//派遣宠物
	DWORD PaiqianPet(DWORD dw_pet_id, DWORD dw_friend_id, DWORD dw_time);
	BOOL IsFriendHasPaiqianPet(DWORD dw_frined_id);

	//回收宠物
	DWORD ReturnPet(DWORD dw_pet_id, BYTE type, INT& itemNum);
	
	//获得称号
	VOID SetTitle(DWORD dwID);
	BOOL HasTitle(DWORD dwID);

	//购买爱心值
	DWORD BuyLoveValue(INT32 nYuanbao);

	BOOL IsInNormalMap();
public:
	tagEquipTeamInfo& GetEquipTeamInfo() { return st_EquipTeamInfo; }

	VOID	SetEquipTeamInfo(tagEquip* pEquip, BOOL bAdd);
	VOID	SendTeamEquipInfo();
	VOID	SetRating(INT32 nRating);
private:
	tagEquipTeamInfo st_EquipTeamInfo;
	INT32			m_nBaseRating;
public:
	VOID	set_check_safe_code() { b_check_safe_code = TRUE; }
	BOOL	get_check_safe_code() { return b_check_safe_code; }	
private:
	BOOL	b_check_safe_code;

public:
	tagRolePvP& get_role_pvp() { return st_Role_PvP; }
	BOOL	IsReservationPvP() { return bReservationPvP; }
	VOID	SetReservationPvP(BOOL b) { bReservationPvP = b; dw_reservation_tick = 20000; }
	VOID	SetReservationID(DWORD dw_role_id) { dw_reservation_id = dw_role_id; }
	DWORD	GetReservationID() { return dw_reservation_id; }
	VOID	update_Reservation();
	DWORD	reservation_apply(DWORD dw_role_id);
	DWORD	reservation_result(DWORD	dw_role_id, BOOL b_ok);
	INT		get_reservation_yuanbao_num(INT nLevel);
private:
	tagRolePvP st_Role_PvP;
	BOOL	   bReservationPvP;
	DWORD	   dw_reservation_tick;
	DWORD	   dw_reservation_id;

public:
	s_1v1_score& get_1v1_score() { return st_1v1_scroe; }
private:
	s_1v1_score st_1v1_scroe;
public:
	VOID	   set_leave_prictice(BOOL b_prictice) { b_leave_prictice = b_prictice; }
	BOOL	   is_leave_pricitice() { return b_leave_prictice; }
private:
	BOOL	   b_leave_prictice;
public:
		LIST_INST_PRO&	get_list_inst_process() { return list_inst_process; }
private:
	LIST_INST_PRO	list_inst_process;
public:
	VOID set_gm_instance_id(DWORD dw_inst_id) { dw_gm_instance_id = dw_inst_id; }
	DWORD get_gm_instance_id() { return dw_gm_instance_id; }
private:
	DWORD dw_gm_instance_id;

	// 当前洗礼装备
	INT64				nCurEquip;
	// 洗礼属性
	INT32				EquipAttitional[MAX_RAND_ATT];
public:
	INT		get_shop_exploits() { return n_shop_exploits_limit; }
	VOID	inc_shop_exploits() { n_shop_exploits_limit++; }
	VOID	zero_shop_exploits() { n_shop_exploits_limit = 0; }
private:
	INT n_shop_exploits_limit;
	BOOL b_will_kick;
public:
	VOID updata_delay();
	VOID inc_delay_num() { n_delay_num++; }
	VOID inc_strength_delay_num() { n_strength_delay_num++; }
	VOID inc_ce_delay_num() { n_ce_check_num++; }
	VOID reset_check_delay_time()
	{
		dw_check_delay_time = g_world.GetWorldTime();
		dw_strength_delay_time = g_world.GetWorldTime();
		n_delay_num = 0;
		n_strength_delay_num = 0;
		n_ce_check_num = 0;
	}
private:
	INT		n_delay_num;	
	DWORD	dw_check_delay_time;

	INT		n_strength_delay_num;
	DWORD   dw_strength_delay_time;
	INT		n_delay_record_num;

	INT		n_ce_check_num;

public:
	INT32	get_justice_value() { return n32_justice; }
	VOID	add_justict_value(INT32 n32_add, BOOL Snd = TRUE) 
	{
		n32_justice += n32_add; 
		if(Snd){
			NET_SIS_justice_newvalue send;
			send.dwRoleID = GetID( );
			send.nJustice = n32_justice;
			SendMessage(&send, send.dw_size);
		}
	}
public:
	VOID ShowConsumUI( ){ if(VALID_POINT(m_pScript)) m_pScript->ShowConsumeUI(this);}
	VOID ConsumeReward(INT index){ if(VALID_POINT(m_pScript)) m_pScript->OnConsumeReward(this, index);}
private:

	INT32	n32_justice;		//正义值

public:
	VOID		set_in_guild_war(bool bwar){ b_in_guild_war = bwar;}
	BOOL		is_in_guild_war() { return b_in_guild_war;}

	VOID		set_defender(bool bdef){ b_defender = bdef;}
	BOOL		is_defender() { return b_defender;}
private:
	BOOL		b_in_guild_war;		// 帮会战中
	BOOL		b_defender;			// 是否防守方
public:
	VOID	init_sign_data(const BYTE* &pData);
	DWORD	get_sign_data();
	DWORD	role_sign(BOOL b_buqian, DWORD dwDate);
	DWORD	role_sign_reward(INT n_index);
	DWORD	role_get_sign_reward();
	//gx add 2013.6.3 玩家每日分时段免费抽奖计数
	DWORD   role_get_free_gamble();
	//end
	//gx add 2013.6.5 玩家当日付费抽奖计数
	DWORD   role_get_normal_gamble();
	//end
	//gx add 2013.8.17玩家当日每日任务计数
	DWORD   role_get_daily_quest_mowu();//魔物狩猎
	DWORD   role_get_daily_quest_xlsl();//修罗试炼

	VOID	role_set_daily_quest_mowu();//魔物
	VOID	role_set_daily_quest_xlsl();//修罗
	//end
	VOID	reset_sign_data() { ZeroMemory(&st_role_sign_data, sizeof(tagRoleSignData)); }

	VOID	init_HuenJing_data(const BYTE* &pData);
	VOID	init_reward_data(const BYTE* &pData);
	VOID	init_mounts_data(const BYTE* &pData);
private:
	tagRoleSignData st_role_sign_data;
public:
	DWORD get_shihun() { return dw_shihun; }
	VOID  add_shihun(DWORD dwAdd) { dw_shihun += dwAdd; }
	VOID  reset_shihun() { dw_shihun = 0; }

	INT16 get_pet_xiulian_size() { return n16_pet_xiulian_size; }
	VOID add_pet_xiulian_size(INT16 nSize) {n16_pet_xiulian_size += nSize; }

	VOID resetMianqianTime() { st_role_sign_data.n16_mianqian_time = 0; }

	VOID ChangeLinqi(INT nValue);

	DWORD buyLingqi();

	DWORD Huenlian(BYTE byType);

	DWORD send_huenjing_data();

	DWORD send_update_huenJing_data(BYTE byType, BYTE byIndex);	
	
	DWORD send_reward_data();

	INT get_huenjing_temp_bag_size();
	INT get_huenjing_temp_bag_num();
	INT get_huenjing_bag_size();

	VOID add_huenjing_to_temp_bag(const tagHuenJing& sHuenjing);
	VOID add_huenjing_to_bag(const tagHuenJing& sHuenjing);

	BYTE getCurArtisan(){ return st_role_huenjing_data.byCurArtisan; }
	
	DWORD HuenJingOpt(INT nIndex, BYTE byType, BYTE byConType);
	DWORD HuenJingLevelUp(INT nIndex);
	DWORD HuenJingInlay(INT nSrcIndex, INT nDesIndex, BYTE byType, BYTE byOptType);

	VOID  InitHuenJingAtt();
	
	// 是否已经镶嵌某属性魂精
	BOOL  IsHuenJingAttHave(BYTE byType, ERoleAttribute eat);

	//VOID  setGodLevel(INT nLevel) { m_nSignLevel = nLevel;}
	INT	  getGodLevel() {return m_nSignLevel;}
	VOID  setGodLevel(INT nLevel) {m_nSignLevel = nLevel;}//gx add 2013.10.28
	
	VOID  updateGodLevel() //零点更新在线玩家的签到奖励 gx add 2013.10.28
	{
		m_nSignLevel++;
		m_nSignLevel = min(6,m_nSignLevel);
		//向客户端发消息
		NET_SIS_update_sign_level send;
		send.nSignLevel = m_nSignLevel;
		SendMessage(&send,send.dw_size);
	}

	DWORD GodLevelUp();

	INT	  getMaxHunJingLevel();

	BOOL	addRewardItem(DWORD dwDataID, DWORD dwNubmer, E_REWARDFROM nType);

	BOOL	receiveRewardItem(E_REWARDFROM nType);

private:
	DWORD dw_shihun;
	INT16 n16_pet_xiulian_size;

	tagRoleHuenJingData		st_role_huenjing_data;

	INT	m_nSignLevel;							// 签到等级
	
	tagRewardData			st_role_reward_data[RF_NUM][MAX_REWARD_NUMBER];
	//双修相关定义 gx add 2013.6.27
	DWORD m_PracticeTick;//从双修开始已经经历的tick
	DWORD m_PracticePartner;//记录与我进行双修的玩家ID
	INT   m_PracticingLevel;//记录玩家进入双修状态时的等级，用于计算双修经验
	//结婚相关定义 gx add 2013.7.3
	DWORD m_SpouseID;//记录配偶ID
	//VIP 相关gx add 2013.08.14
	INT m_VIP_Level;
	tagDWORDTime m_VIP_DeadLine;//当前等级对应的VIP的截止日期
	DWORD m_VIP_RefreshTick;//记录VIp刷新经历的tick
public:
	inline VOID  SetComPracticePartner(DWORD dwRoleID){m_PracticePartner = dwRoleID;}
	inline DWORD GetComPracticePartner(){return m_PracticePartner;}
	inline VOID  SetComPracticeLevel(INT level) {m_PracticingLevel = level;}
	inline INT   GetComPracticeLevel() {return m_PracticingLevel;}
	VOID UpdateComPracticeExp();//处于双修状态下的经验增长
	inline VOID SetSpouseID(DWORD dwSpouseID) {m_SpouseID = dwSpouseID;}
	inline DWORD GetSpouseID()	{return m_SpouseID;}
	//VIP相关
	inline VOID SetVIPLevel(INT level) {m_VIP_Level = level;}
	inline INT GetVIPLevel() {return m_VIP_Level;}
	inline tagDWORDTime GetVIPDeatLine() {return m_VIP_DeadLine;}
	VOID UpdateVIPTime();//更新VIP时间
	VOID CheckVip_OffLine();//玩家下线时，检查一次VIP
	VOID SetVIPDeatLine(int vip_level)
	{
		int add_time = GET_VIP_EXTVAL(GetVIPLevel(),VIP_TIME,int);//获取当前VIP等级的有效期，单位为天
		if (add_time < 0)
			return;
		tagDWORDTime curTime = GetCurrentDWORDTime();
		INT increaseSecondes = add_time*24*60*60;
		m_VIP_DeadLine = IncreaseTime(curTime,increaseSecondes);
		//向DB发消息
		NET_DB2C_change_role_VIP_Info send;
		send.dw_role_id = GetID();
		send.n_VIP_Level = GetVIPLevel();
		send.dw_VIP_deadline = m_VIP_DeadLine;
		g_dbSession.Send(&send, send.dw_size);

		//向客户端发消息
		NET_SIS_get_vip_level cmd;
		cmd.dw_role_id = GetID();
		cmd.vip_level = GetVIPLevel();
		SendMessage(&cmd, cmd.dw_size);
	}
	//end
	//gx add 2013.10.29 玩家某个脚本数据发生变化，通知客户端
	VOID update_scriptdata2client(INT nindex,DWORD dwvalue)
	{
		if(nindex < 0 || nindex >= ESD_Role)
			return;//防止异常
		SetScriptData(nindex,dwvalue);
		//向客户端发消息
		NET_SIS_update_role_script_data cmd;
		cmd.dw_role_id = GetID();
		cmd.nindex = nindex;
		cmd.dwvalue = dwvalue;
		SendMessage(&cmd, cmd.dw_size);
	}
	//拜师相关，gx add 2013.12.18
private:
	DWORD m_dwShituAskID;//记录向该玩家发起拜师或收徒请求的角色ID，用于防止同一角色多次向该玩家发起请求
public:
	inline VOID SetShituAskID(DWORD dwRoleID) {m_dwShituAskID = dwRoleID;}
	inline DWORD GetShituAskID() {return m_dwShituAskID;}
};
