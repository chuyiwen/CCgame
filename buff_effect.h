/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//״̬Ч��

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
	// ע��Buff����Ч��������
	//------------------------------------------------------------------------------
	static VOID RegisterBuffEffectRutine();

	//-------------------------------------------------------------------------------
	// ����BuffЧ��
	//-------------------------------------------------------------------------------
	static VOID CalBuffEffect(Buff* pBuff, Unit* pTarget, Unit* pSrc, EBuffEffectType eEffect, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL);
	static VOID	CalBuffInstantEffect(Unit* pOwner, Unit* pSrc, EBuffEffectMode eMode, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes=1, Unit* pTarget=NULL);
	static VOID CalBuffPersistEffect(Buff* pBuff, Unit* pOwner, Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes=1, BOOL bSet=TRUE);
private:
	//-----------------------------------------------------------------------------------------------------------------
	// ��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectNull(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ˲��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectTeleport(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �������
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInterruptSpell(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ѣ��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectDizzy(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectRepel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ���
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectAssault(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ��ײ
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectCollid(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �⼼
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectNoSkill(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ��˯
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectSpor(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectTie(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ��ɢ
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectDispel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ȥ��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectCancel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// �޵�
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInvincible(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInstantDie(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ��ק
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectPull(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ת��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectHPTransfer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ת��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMPTransfer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectRevive(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ˲����ȴ
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectInstantCD(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectLurk(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectFly(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �ı���
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectChangeEnmity(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectTransmit(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �ɼ�
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectGather(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ��е
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectDisArm(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����λ��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectExchangePos(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectExplode(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �־�
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectFunk(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ׷��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectPursue(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ������ʱ��
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectNoPrepare(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// ˮ������
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectOnWater(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �ƶ���Ѫ
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMoveHPDmg(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// ���ӻ���
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectIgnoreArmor(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectSneer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// ���
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMount(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// �������
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectMountInvite(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// �⻷
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffEffectRing(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

	//-----------------------------------------------------------------------------------------------------------------
	// HP����
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectHPExchange(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// �����˺�
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectAbsorbDmg(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	// buff��ȡ
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectFilchBuff(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	//-----------------------------------------------------------------------------------------------------------------
	//������
	//-----------------------------------------------------------------------------------------------------------------
	static VOID BuffectRonghePet(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID BuffectCancelBeHeti(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID BUffectCancelHeTi(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	// ���pkֵ
	static VOID	BuffectClaerPkValue(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID	BuffectCantAttackUnMount(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID BuffectCallAttackPet(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
private:
	static VOID GatherEffect(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);
	static VOID InvesEffect(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet=TRUE, const tagBuffProto* pProto=NULL, Buff* pBuff = NULL);

private:
	static BUFFEFFECTRUTINE	m_Effect[EBET_End];
};