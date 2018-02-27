/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//战斗管理器

#pragma once

class Unit;

// 持续地物型技能结构
struct tagPilotUnit
{
	DWORD dwMapID;		//地物mapid
	DWORD dwInstanceID;	//地物副本id
	DWORD dwCretureID;	//对应地物id
	FLOAT fOPRadius;	//作用范围
	DWORD dwOverTime;	//剩余时间
	DWORD dwSkillID;	//对应技能id
};

class CombatHandler
{
	friend class Role;
public:
	enum ETargetEffectFlag
	{
		ETEF_Hited		=	0x0001,		// 命中
		ETEF_Block		=	0x0002,		// 格挡
		ETEF_Crited		=	0x0004,		// 暴击
	};

	enum EInterruptType
	{
		EIT_Null		=	-1,			// 无
		EIT_Move		=	0,			// 移动打断
		EIT_Skill		=	1,			// 技能
	};
	
	//技能使用的各阶段
	enum EUseSkillState
	{
		EUSS_NULL		=	-1,
		EUSS_Preparing	=	0,			// 吟唱
		EUSS_Operating	=	1,			// 释放中
		EUSS_Piloting	=	2,			// 持续中
	};
public:
	//----------------------------------------------------------------------------
	// Constructor
	//----------------------------------------------------------------------------
	CombatHandler();

	//----------------------------------------------------------------------------
	// 初始化、开始，更新和结束
	//----------------------------------------------------------------------------
	VOID	Init(Unit* pOwner);
	INT		UseSkill(DWORD dwSkillID, DWORD dwTargetUnitID, DWORD dwSerial ,Vector3 vDesPos = Vector3(0,0,0));
	INT		UseItem(INT64 n64ItemID, DWORD dwTargetUnitID, DWORD dwSerial, DWORD &dw_data_id, bool& bImmediate);
	DWORD	UseRide();//Ares
	VOID	Update();
	VOID	End();

	DWORD CheckUseRide(tagEquip* pRide);

	VOID	SetCombatStateCoolDown() { m_dwCombatStateCoolDown = 6000; }

	//----------------------------------------------------------------------------
	// 取消技能使用
	//----------------------------------------------------------------------------
	VOID	CancelSkillUse(DWORD dwSkillID);

	//----------------------------------------------------------------------------
	// 取消物品使用
	//----------------------------------------------------------------------------
	VOID	CancelItemUse(INT64 n64ItemSerial);

	//----------------------------------------------------------------------------
	// 取消骑乘
	//----------------------------------------------------------------------------
	VOID InterruptRide();

	//----------------------------------------------------------------------------
	// 打断起手
	//----------------------------------------------------------------------------
	BOOL	InterruptPrepare(EInterruptType eType, BOOL bOrdinary, BOOL bForce=FALSE);

	//----------------------------------------------------------------------------
	// 打断释放
	//----------------------------------------------------------------------------
	BOOL	InterruptOperate(EInterruptType eType, DWORD dwMisc, BOOL bForce=FALSE);

	//----------------------------------------------------------------------------
	// 各种mod
	//----------------------------------------------------------------------------
	VOID	ModSkillPrepareModPct(INT nModPct);
	VOID	ModTargetArmorLeftPct(INT nModPct);

	//----------------------------------------------------------------------------
	// 各种Get
	//----------------------------------------------------------------------------
	BOOL	IsValid()					{ return IsUseSkill() || IsUseItem() || IsRide(); }
	BOOL	IsUseSkill()				{ return VALID_POINT(m_dwSkillID); }
	BOOL	IsUseItem()					{ return VALID_POINT(m_n64ItemID); }
	BOOL	IsRide()					{ return m_bRide;}
	BOOL	IsPreparing()				{ return IsSkillPreparing() || IsItemPreparing() || IsRidePreparing(); }
	BOOL	IsOperating()				{ return IsSkillOperating() || IsItemOperating() || IsRideOperating(); }
	BOOL	IsPiloting()				{return	 IsSkillPiloting();}
	INT		GetSkillPrepareCountDown()	{ return m_nSkillPrepareCountDown; }
	INT		GetItemPrepareCountDown()	{ return m_nItemPrepareCountDown; }
	INT		GetRidePrepareCountDown()	{ return m_nRidePrepareCountDown; }
	DWORD	GetTargetUnitID()			{ return m_dwTargetUnitID; }
	DWORD	GetSkillID()				{ return m_dwSkillID; }
	FLOAT	GetTargetArmor(Unit* target, DWORD dwArmorType);

	INT		GetPilotTimeCD()			{return m_nPersistSkillTimeCD;}
	//----------------------------------------------------------------------------
	// 技能使用判断
	//----------------------------------------------------------------------------
	INT		CanCastSkill(Skill* pSkill, DWORD dwTargetUnitID ,const Vector3&);

	INT		CheckSkillAbility(Skill* pSkill);							// 测试技能能力
	INT		CheckOwnerLimitSkill();											// 测试技能所有者限制
	INT		CheckSkillLimit(Skill* pSkill);								// 测试技能本身限制
	INT		CheckTargetLimit(Skill* pSkill, DWORD dwTargetUnitID,const Vector3&);		// 测试目标限制
	INT		CheckCostLimit(Skill* pSkill);								// 测试消耗限制
	INT		CheckVocationLimit(Skill* pSkill);							// 测试职业限制
	INT		CheckTargetLogicLimit(Skill* pSkill, Unit* pTarget);		// 检测技能和目标的
	INT		CheckMapLimit(Skill* pSkill);								// 检测地图限制
	VOID	CheckInCombat(Unit* pTarget);								// 战斗状态判定

	BOOL	CheckSkillConflict(Skill* pSkill);							// 检查技能使用的冲突

	//----------------------------------------------------------------------------
	// 计算技能效果
	//----------------------------------------------------------------------------
	DWORD	CalculateSkillEffect(Skill* pSkill, Unit* pTarget);
	VOID	CalSkillTargetList(Vector3 vPosDes = Vector3(0,0,0), DWORD dwMaxNumber = INVALID_VALUE);
	BOOL	CalculateHit(Skill* pSkill, Unit* pTarget);
	BOOL	CalculateBlock(Skill* pSkill, Unit* pTarget);
	BOOL	CalculateCritRate(Skill* pSkill, Unit* pTarget);
	FLOAT	CalculateCritAmount(Skill* pSkill, Unit* pTarget);	
	VOID	CalculateDmg(Skill* pSkill, Unit* pTarget);
	VOID	CalculateCost(Skill* pSkill);
	
	VOID	CalSkillTargetList(package_list<DWORD>& targetList, const tagPilotUnit* pPilotUnit, Vector3 vPosDes = Vector3(0,0,0));
	VOID	CalculateDmgNoSpe(Skill* pSkill, Unit* pTarget);
	VOID	ClearPilotList();
	//----------------------------------------------------------------------------
	// 物品使用判断
	//----------------------------------------------------------------------------
	INT		can_use_item(tagItem* pItem);
	INT		CheckItemAbility(tagItem* pItem);							// 检测物品本身
	INT		CheckOwnerLimitItem();										// 检测自身
	INT		CheckRoleProtoLimit(tagItem* pItem);						// 检测人物属性限制
	INT		CheckRoleStateLimit(tagItem* pItem);						// 检测人物状态限制
	INT		CheckRoleVocationLimit(tagItem* pItem);						// 检测人物职业限制
	INT		CheckMapLimit(tagItem* pItem);								// 检测地图限制
	BOOL	CheckItemConflict(tagItem* pItem);

	//-----------------------------------------------------------------------------
	// 计算物品效果
	//-----------------------------------------------------------------------------
	VOID	CalUseItemTargetList();
	
	//-----------------------------------------------------------------------------
	// 计算能量效果
	//-----------------------------------------------------------------------------
	//VOID	OnHit(Skill* pSkill, INT32 nlevelSub, BOOL bCrit = FALSE);//命中时
	//VOID	OnDmg(Unit* pRole, INT dmg, INT32 nlevelSub, BOOL bCrit = FALSE);//受到伤害时

	//-----------------------------------------------------------------------------
	// 游侠的能量变化逻辑相关函数
	//-----------------------------------------------------------------------------
	//得到蓄力类型
	//DWORD	GetChargeType();
private:
	//-----------------------------------------------------------------------------
	// 各种Get
	//-----------------------------------------------------------------------------
	bool	IsSkillPreparing()			{ return m_eUseSkillState == EUSS_Preparing; }
	bool	IsSkillOperating()			{ return m_eUseSkillState == EUSS_Operating; }
	bool	IsSkillPiloting()			{ return m_eUseSkillState == EUSS_Piloting;}	//add by guohui
	bool	IsItemPreparing()			{ return m_bItemPreparing; }
	bool	IsItemOperating()			{ return m_bItemOperating; }
	bool	IsRidePreparing()			{ return m_bRidePreparing; }
	bool	IsRideOperating()			{ return m_bRideOperating; }

	//-----------------------------------------------------------------------------
	// Mod底层调用函数
	//-----------------------------------------------------------------------------
	VOID	ModPct(IN OUT FLOAT &fDstPct, IN INT nModPct);

	//-----------------------------------------------------------------------------
	// 更新操作
	//-----------------------------------------------------------------------------
	VOID	UpdateSkillPrepare();
	VOID	UpdateSkillOperate();
	VOID	UpdateItemPrepare();
	VOID	UpdateItemOperate();
	VOID	UpdateRidePrepare();
	VOID	UpdateRideOperate();
	VOID	UpdateSkillPiloting();
	VOID	UpdatePrepareUnit();
	//-----------------------------------------------------------------------------
	// 结束操作
	//-----------------------------------------------------------------------------
	VOID	EndUseSkill();
	VOID	EndUseItem();
	VOID	EndRide();

	//----------------------------------------------------------------------------
	// 伤害公式的系数计算
	//----------------------------------------------------------------------------
	FLOAT	CalBaseDmg(Skill* pSkill, Unit* pTarget);
	FLOAT	CalAttackDefenceCoef(Skill* pSkill, Unit* pTarget);
	//FLOAT	CalMoraleCoef(Unit* pTarget);
	FLOAT	CalDerateCoef(Skill* pSkill, Unit* pTarget);
	//FLOAT	CalInjuryCoef();
	FLOAT	CalLevelCoef(Skill* pSkill, Unit* pTarget);

private:
	Unit*			m_pOwner;					// 发起者

	DWORD			m_dwSkillID;				// 发动的技能ID
	INT64			m_n64ItemID;				// 使用物品64位ID
	DWORD			m_dwTargetUnitID;			// 技能发动的主目标ID
	DWORD			m_dwTargetEffectFlag;		// 主目标的作用效果
	DWORD			m_dwSkillSerial;			// 技能攻击序列号
	DWORD			m_dwItemSerial;				// 物品使用序列号
	DWORD			m_dwTargetUnitIDItem;		// 使用物品的目标ID
	BOOL			m_bRide;					// 使用坐骑
	
	EUseSkillState	m_eUseSkillState;			// 当前技能阶段
	//bool			m_bSkillPreparing;			// 技能是否在起手
	//bool			m_bSkillOperating;			// 技能是否在发动
	//bool			m_bSkillPiloting;			// 技能正在引导中

	bool			m_bItemOperating;			// 物品是否在发动
	bool			m_bItemPreparing;			// 物品正在起手

	bool			m_bRidePreparing;			// 坐骑起手
	bool			m_bRideOperating;			// 坐骑发动

	bool			m_bTrigger;					// 是否计算触发相关

	INT				m_nRidePrepareCountDown;
	INT				m_nSkillPrepareCountDown;	// 技能起手倒计时（毫秒）
	INT				m_nItemPrepareCountDown;	// 物品起手倒计时（毫秒）

	FLOAT			m_fSkillPrepareModPct;		// 技能起手时间影响百分比
	FLOAT			m_fTargetArmorLeftPct;		// 目标护甲削弱后剩余百分比（1.0f - 削弱百分比）

	INT				m_nSkillOperateTime;		// 技能操作的时间，用于间隔计算伤害（毫秒）
	INT				m_nSkillCurDmgIndex;		// 当前要计算第几次伤害

	INT				m_nPersistSkillTimeCD;		//持续技能时间倒计时
	INT				m_nPersistSkillTime;		//持续技能时间
	INT				m_nPersistSkillCnt;			//持续技能当前次数
	
	//bool			m_bDropMP;					// 持续技能是否扣篮
	Vector3			m_vPersistSkillPos;			//持续技能目标点 （不是由目标id决定的）

	INT				m_dwPublicCoolDown;				// 技能公共冷却时间

	INT				m_dwCombatStateCoolDown;		// 战斗状态冷却时间
	
	INT				m_dwChixuCoolDown;				// 火墙类技能间隔

	BOOL			m_bCD;

	package_list<DWORD>	m_listTargetID;				// 技能目标列表
	package_list<DWORD>	m_listHitedTarget;			// 技能命中的目标
	package_list<DWORD>	m_listDodgedTarget;			// 技能闪避的目标
	package_list<DWORD>	m_listBlockedTarget;		// 技能格挡的目标
	package_list<DWORD>	m_listCritedTarget;			// 技能暴击的目标

	
	std::map<DWORD, tagPilotUnit*>	m_listPilotUnit;	// 持续型地物技能
};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline CombatHandler::CombatHandler()
: m_pOwner(NULL), m_dwSkillID(INVALID_VALUE), m_n64ItemID(INVALID_VALUE), m_dwTargetUnitID(INVALID_VALUE),
m_dwTargetEffectFlag(0), m_dwSkillSerial(0), m_dwItemSerial(0), m_eUseSkillState(EUSS_NULL),/*m_bSkillPreparing(false), */m_bItemPreparing(false),
/*m_bSkillOperating(false),*/ m_bItemOperating(false), m_nSkillPrepareCountDown(0), m_nItemPrepareCountDown(0),
m_fSkillPrepareModPct(1.0f), m_fTargetArmorLeftPct(1.0f), m_nSkillOperateTime(0), m_nSkillCurDmgIndex(0)/*,m_bSkillPiloting(false)*/
,m_nPersistSkillTime(0),m_nPersistSkillCnt(0),m_bTrigger(FALSE),m_dwPublicCoolDown(0),m_dwCombatStateCoolDown(0)//,m_bDropMP(true)
,m_bRide(FALSE),m_bRidePreparing(FALSE),m_nRidePrepareCountDown(0),m_bCD(FALSE),m_dwChixuCoolDown(0)
{
}

//-----------------------------------------------------------------------------
// 当前技能初始化
//-----------------------------------------------------------------------------
inline VOID CombatHandler::Init(Unit* pOwner)
{
	m_pOwner	=	pOwner;
}


//-------------------------------------------------------------------------------------------
// 结束使用物品
//-------------------------------------------------------------------------------------------
inline VOID CombatHandler::EndUseItem()
{
	m_n64ItemID					=	INVALID_VALUE;
	m_dwItemSerial				=	0;
	m_bItemPreparing			=	false;
	m_bItemOperating			=	false;
	m_nItemPrepareCountDown		=	0;
}
//-------------------------------------------------------------------------------------------
// 结束使用坐骑
//-------------------------------------------------------------------------------------------
inline VOID CombatHandler::EndRide()
{
	m_bRide = FALSE;
	m_bRidePreparing = false;
	m_bRideOperating = false;
	m_nRidePrepareCountDown = 0;
}

//--------------------------------------------------------------------------------------------
// 结束战斗系统所有动作
//--------------------------------------------------------------------------------------------
inline VOID CombatHandler::End()
{
	EndUseSkill();
	EndUseItem();
	EndRide();
}

//-----------------------------------------------------------------------------
// Mod底层调用函数
//-----------------------------------------------------------------------------
inline VOID CombatHandler::ModPct(IN OUT FLOAT &fDstPct, IN INT nModPct)
{
	fDstPct += (FLOAT)nModPct / 10000.0f;

	if(fDstPct < 0.0f)
	{
		// 当百分比变为负值时，无法进行反向操作
		ASSERT(fDstPct >= 0.0f);
		fDstPct = 0.0f;
	}
}

//----------------------------------------------------------------------------
// 设置起手时间影响百分比
//----------------------------------------------------------------------------
inline VOID CombatHandler::ModSkillPrepareModPct(INT nModPct)
{
	ModPct(m_fSkillPrepareModPct, nModPct);
}

//----------------------------------------------------------------------------
// 设置对目标护甲影响百分比
//----------------------------------------------------------------------------
inline VOID CombatHandler::ModTargetArmorLeftPct(INT nModPct)
{
	ModPct(m_fTargetArmorLeftPct, nModPct);
}
