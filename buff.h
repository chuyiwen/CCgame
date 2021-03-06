/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//状态

#pragma once

class Unit;

//--------------------------------------------------------------------------------
// 添加到身上的buff
//--------------------------------------------------------------------------------
class Buff
{
public:
	enum EBuffState				// Buff状态
	{
		EBS_Idle		=	0,	// 空闲
		EBS_Initing		=	1,	// 初始化
		EBS_Updating	=	2,	// 更新
		EBS_Destroying	=	3,	// 删除
	};

public:
	Buff();
	~Buff();

	//----------------------------------------------------------------------------
	// 初始化，更新和销毁和保存
	//----------------------------------------------------------------------------
	VOID			Init(const tagBuffProto* pBuffProto, Unit* pTarget, Unit* pSrc, DWORD dwSrcSkillID, 
						const tagItem* pItem, INT nIndex, package_list<DWORD>* listModifier=NULL);
	VOID			Init(Unit* pTarget, const s_buff_save* pBuffSave, INT nIndex);
	VOID			InitBuffSave(OUT s_buff_save *pBuffSave, OUT INT32 &nSize);
	BOOL			Update();
	VOID			Destroy(BOOL bSelf);

	//----------------------------------------------------------------------------
	// 叠加和打断和触发
	//----------------------------------------------------------------------------
	VOID			Wrap();
	BOOL			Interrupt(EBuffInterruptFlag eFlag, INT nMisc=INVALID_VALUE);
	BOOL			OnTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);

	//----------------------------------------------------------------------------
	// 各种Get
	//----------------------------------------------------------------------------
	BOOL			IsValid()				{ return VALID_POINT(m_dwID); }
	DWORD			GetID()					{ return m_dwID; }
	DWORD			GetTypeID()				{ return m_pProto->dwID; }
	INT				get_level()				{ return m_nLevel; }
	INT				GetGroupLevel()			{ return m_pProto->nLevel; }
	INT				GetIndex()				{ return m_nIndex; }
	DWORD			GetSrcUnitID()			{ return m_dwSrcUnitID; }
	DWORD			GetSrcSkillID()			{ return m_dwSrcSkillID; }
	DWORD			GetGroupFlag()			{ return m_pProto->dwGroupFlag; }
	DWORD			GetBuffInterruptID()	{ return m_pProto->dwBuffInterruptID; }
	INT				GetWrapTimes()			{ return m_nWrapTimes; }
	EBuffResistType	GetResistType()			{ return m_pProto->eResistType; }
	EBuffEffectType	GetEffectType(EBuffEffectMode eMod)			{ return m_pProto->eEffect[eMod]; }
	EBuffState		GetState()				{ return m_eState; }

	INT				GetPersistTimeLeft()	{ return ( !IsPermanent() ? ((GetMaxPersistTick() - m_nPersistTick) * TICK_TIME) : INVALID_VALUE); }
	INT				GetPersistTime()		{ return ( !IsPermanent() ? GetMaxPersistTick() * TICK_TIME : INVALID_VALUE); }
	
	BOOL			IsBenifit()				
	{ 
		if (!VALID_POINT(m_pProto))
			return FALSE;

		return m_pProto->bBenifit;
	}

	const tagBuffProto* GetProto() { return m_pProto;}
	//伤害吸收相关
	DWORD GetAbsorbDmg() const { return m_dwAbsorbDmg; }
	void SetAbsorbDmg(DWORD val) { m_dwAbsorbDmg = val; }
	//-----------------------------------------------------------------------------
	// 一些方便的静态函数
	//-----------------------------------------------------------------------------
	static DWORD	GetIDFromTypeID(DWORD dw_data_id)						{ return dw_data_id / 100; }
	static INT		GetLevelFromTypeID(DWORD dw_data_id)					{ return dw_data_id % 100; }
	static DWORD	GetTypeIDFromIDAndLevel(DWORD dwID, INT nLevel)		{ return dwID * 100 + nLevel; }

private:
	//-----------------------------------------------------------------------------
	// 状态切换
	//-----------------------------------------------------------------------------
	VOID			BeginInit()			{ m_eState = EBS_Initing; }
	VOID			EndInit()			{ m_eState = EBS_Idle; }
	VOID			BeginUpdate()		{ m_eState = EBS_Updating; }
	VOID			EndUpdate()			{ m_eState = EBS_Idle; }
	VOID			BeginDestroy()		{ m_eState = EBS_Destroying; }
	VOID			EndDestroy()		{ m_eState = EBS_Idle; }

	//----------------------------------------------------------------------------
	// 一些Get函数
	//----------------------------------------------------------------------------
	INT				GetMaxPersistTick();
	INT				GetMaxWrapTimes();
	INT				GetAttackInterrupt();

	//----------------------------------------------------------------------------
	// 永久性Buff和间隔作用性buff判断
	//----------------------------------------------------------------------------
	INT				IsPermanent();
	INT				IsInterOP();

	//----------------------------------------------------------------------------
	// 生成属性ID
	//----------------------------------------------------------------------------
	DWORD			CreateTypeID(DWORD dwID, INT nLevel)	{ return dwID * 100 + nLevel; }
	


private:
	EBuffState						m_eState;						// 当前状态
	DWORD							m_dwID;							// ID
	INT								m_nLevel;						// 等级
	INT								m_nIndex;						// 对应的Buff中的索引
	DWORD							m_dwSrcUnitID;					// 源Unit的ID
	DWORD							m_dwSrcSkillID;					// 由哪个技能产生（大ID）
	INT64							m_n64ItemID;					// 由哪个物品或装备产生
	DWORD							m_dwItemTypeID;					// 物品或装备的属性ID
	DWORD							m_dwAbsorbDmg;					// 伤害吸收量

	INT								m_nPersistTick;					// 当前的持续时间
	INT								m_nCurTick;						// 当前作用时间 
	INT								m_nWrapTimes;					// 当前叠加次数

	Unit*							m_pOwner;						// 在谁身上
	const tagBuffProto*				m_pProto;						// 静态属性
	const tagTriggerProto*			m_pTrigger;						// 效果触发器
	tagBuffMod*						m_pMod;							// 属性加成

};

//-------------------------------------------------------------------------------------
// 得到Buff的最大持续时间
//-------------------------------------------------------------------------------------
inline INT Buff::GetMaxPersistTick()
{
	INT nPersistTick = m_pProto->nPersistTick;
	if( VALID_POINT(nPersistTick) && VALID_POINT(m_pMod) )
	{
		nPersistTick = nPersistTick + m_pMod->nPersistTickMod;
	}
	return nPersistTick;
}

//-------------------------------------------------------------------------------------
// 得到Buff的最大叠加次数
//-------------------------------------------------------------------------------------
inline INT Buff::GetMaxWrapTimes()
{
	return m_pProto->nWarpTimes + ( VALID_POINT(m_pMod) ? m_pMod->nWarpTimesMod : 0 );
}

//--------------------------------------------------------------------------------------
// 得到Buff的攻击被打断几率
//--------------------------------------------------------------------------------------
inline INT Buff::GetAttackInterrupt()
{
	return m_pProto->nAttackInterruptRate + (VALID_POINT(m_pMod) ? m_pMod->nAttackInterruptRateMod : 0);
}

//--------------------------------------------------------------------------------------
// Buff是否为永久性Buff
//--------------------------------------------------------------------------------------
inline INT Buff::IsPermanent()
{
	return !VALID_POINT(m_pProto->nPersistTick);
}

//--------------------------------------------------------------------------------------
// Buff是否为间隔作用性Buff
//--------------------------------------------------------------------------------------
inline INT Buff::IsInterOP()
{
	return m_pProto->nInterOPTick > 0;
}
