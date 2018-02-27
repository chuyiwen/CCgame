/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//状态效果

#pragma once


class Unit;
class Buff;
class BuffEffect;

typedef VOID (*BUFFEFFECTRUTINE)(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff);

class BuffEffect
{
public:
	static VOID Init();

	//------------------------------------------------------------------------------
	// 注册Buff特殊效果处理函数
	//------------------------------------------------------------------------------
	static VOID RegisterBuffEffectRutine();

	//-------------------------------------------------------------------------------
	// 计算Buff效果
	//-------------------------------------------------------------------------------
	static VOID CalBuffEffect(Buff* pBuff, Unit* pTarget, Unit* pSrc, EBuffEffectType eEffect, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL);
	static VOID	CalBuffInstantEffect(Unit* pOwner, Unit* pSrc, EBuffEffectMode eMode, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes=1, Unit* pTarget=NULL);
	static VOID CalBuffPersistEffect(Buff* pBuff, Unit* pOwner, Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes=1, BOOL bSet=TRUE);
private:
	//-----------------------------------------------------------------------------------------------------------------
	// 无
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectNull(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 瞬移
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectTeleport(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 打断起手
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInterruptSpell(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 眩晕
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectDizzy(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 击退
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectRepel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 冲锋
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectAssault(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 冲撞
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectCollid(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 封技
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectNoSkill(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 昏睡
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectSpor(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 定身
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectTie(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 驱散
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectDispel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 去除
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectCancel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// 无敌
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInvincible(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 即死
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInstantDie(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 拖拽
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectPull(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 转魂
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectHPTransfer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 转精
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMPTransfer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 复活
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectRevive(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 瞬间冷却
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInstantCD(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 隐身
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectLurk(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 飞行
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectFly(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 改变仇恨
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectChangeEnmity(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 传送
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectTransmit(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 采集
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectGather(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 缴械
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectDisArm(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 交换位置
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectExchangePos(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 引爆
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectExplode(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 恐惧
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectFunk(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 追踪
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectPursue(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 无起手时间
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectNoPrepare(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 水上行走
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectOnWater(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 移动掉血
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMoveHPDmg(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// 无视护甲
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectIgnoreArmor(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// 嘲讽
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectSneer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// 骑乘
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMount(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// 邀请骑乘
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMountInvite(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// 光环
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectRing(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// HP交换
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectHPExchange(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// 吸收伤害
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectAbsorbDmg(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// buff窃取
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectFilchBuff(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	//武宠合体
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectRonghePet(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID BuffectCancelBeHeti(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID BUffectCancelHeTi(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	// 清除pk值
	static VOID	BuffectClaerPkValue(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID	BuffectCantAttackUnMount(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID BuffectCallAttackPet(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
private:
	static VOID GatherEffect(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID InvesEffect(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

private:
	static BUFFEFFECTRUTINE	m_Effect[EBET_End];
};