
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�������

#pragma once

#include "StdAfx.h"
#include "player_net_cmd_mgr.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "Mutex.h"
#include "gm_net_cmd_mgr.h"
#include "fatigue_guarder.h"
#include "verification.h"
#include "static_array.h"

class Creature;

const INT CON_LOST	=	-1000000;		// ���ӶϿ���־
const INT RET_TRANS	=	-2000000;		// ���ϲ㴦��

struct PlayerSessionCommonData{
	DWORD dwRoleID;
	tagDWORDTime dwDelGuardTime;
	tagDWORDTime dwChangeNameTime;
public:
	PlayerSessionCommonData(){
		memset(this, -1, sizeof(*this));
	}
};



struct PSCSearchPred{
public:
	PSCSearchPred(DWORD id):dwRoleID(id){};
	bool operator()(const PlayerSessionCommonData* p){
		return p->dwRoleID == dwRoleID;
	}
public:
	DWORD dwRoleID;
};

class PlayerSession
{
public:
	friend class DBSession;
	friend class center_session;
	friend class WorldNetCmdMgr;

	PlayerSession(DWORD dwSessionID, DWORD dwInternalIndex, DWORD dw_ip, BYTE byPrivilege,
				BOOL bGuard, DWORD dwAccOLSec, LPCSTR tszAccount,tagDWORDTime dwPreLoginTime,
				DWORD dwPreLoginIP);
	~PlayerSession();

	//-----------------------------------------------------------------------------
	// ����Get
	//-----------------------------------------------------------------------------

	tagDWORDTime GetPreLoginTime()	const	{ return m_dwPreLoginTime;}
	DWORD		 GetPreLoginIP()	const	{ return m_dwPreLoginIP;}
	DWORD		 GetCurLoginIP()	const	{ return m_dwIP;}

	LPCSTR		GetAccount()		const	{ return m_szAccount;	}
	DWORD		GetGetIP()			const	{ return m_dwIP; }
	DWORD		GetSessionID()		const	{ return m_dwAccountID; }
	INT			GetMsgWaitNum()		const	{ return m_nMsgNum; }
	DWORD		GetInternalIndex()	const	{ return m_dwInternalIndex; }
	Role*		GetRole()			const	{ return m_pRole; }
	Role*		GetOtherInMap( DWORD dw_role_id ) const ;
	const FatigueGuarder&	GetFatigueGuarder()	const { return m_FatigueGarder; }
	Verification& GetVerification() { return m_Verification;}
	INT			GetVNBExpRate()		const;
	INT			GetVNBLootRate()		const;
	LPCTSTR		GetVNBName()		const;
	BOOL		IsKicked() const			{ return m_bKicked; }

	// �ʺ��½�ɫͨ��������ز���
	const INT	GetBaiBaoYB()		const	{ return m_sAccountCommon.n_baibao_yuanbao_; }
	const INT32	GetScore()			const	{ return m_sAccountCommon.n32_score; }
	const INT64	GetWareSilver()		const	{ return m_sAccountCommon.n64_ware_silver_; }
	const INT16	GetWareSize()		const	{ return m_sAccountCommon.n16_ware_size_; }
	const INT16	GetWareStep()	const { return m_sAccountCommon.n16_ware_step; }
	
	DWORD		GetTotalRecharge () const { return m_sAccountCommon.dw_total_recharge; }

	const BOOL  GetReceive()		const	{ return m_sAccountCommon.b_receive; }
	const INT16 GetReceiveType()	const	{ return m_sAccountCommon.n16_receive_type; }
	const DWORD GetReceiveTypeEx()	const	{ return m_sAccountCommon.dw_receive_type; }
	//----------------------------------------------------------------------------
	// ����Set
	//----------------------------------------------------------------------------
	VOID		SetRole(Role* pRole)		{ m_pRole = pRole; }
	VOID		SetConnectionLost()			{ Interlocked_Exchange((LPLONG)(&m_bConnectionLost), TRUE); }
	VOID		SetKicked()					{ Interlocked_Exchange((LPLONG)(&m_bKicked), TRUE); }

	VOID		SetBaiBaoYB(INT nYuanBao)		{ m_sAccountCommon.n_baibao_yuanbao_	+= nYuanBao; }
	VOID		SetExchange(INT nVolume)		{ m_sAccountCommon.n32_score += nVolume; }
	VOID		SetReceiveType(DWORD dw_receive_type) { m_sAccountCommon.dw_receive_type |= dw_receive_type; }
	VOID		SetTotalRecharge(INT nYuanBao)	{ m_sAccountCommon.dw_total_recharge += nYuanBao; }
	VOID		SetWareSilver(INT64 n64Silver)	{ m_sAccountCommon.n64_ware_silver_	= n64Silver; }
	VOID		SetAccOLMin(DWORD dwState, DWORD dwAccOLMin)	{ m_FatigueGarder.SetAccOLTimeMin(dwState, dwAccOLMin);				}

	VOID		SessionLogout();

	//----------------------------------------------------------------------------
	// ��Ϣ�������
	//----------------------------------------------------------------------------
	INT			HandleMessage();
	VOID		SendMessage(LPVOID p_message, DWORD dw_size);
	VOID		BroadcastCurrChannel(LPCTSTR szMsg);

	//-----------------------------------------------------------------------------
	// ѡ�˽�����ж�
	//-----------------------------------------------------------------------------
	bool		IsRoleLoading()		const	{ return m_bRoleLoading; }
	bool		IsRoleEnuming()		const	{ return m_bRoleEnuming; }
	bool		IsRoleCreating()	const	{ return m_bRoleCreating; }
	bool		IsRoleDeleting()	const	{ return m_bRoleDeleting; }
	bool		IsRoleChangeNaming()	const	{ return m_bRoleChangeNameing; }
	bool		IsRoleDelGuardCanceling()const	{ return m_bRoleDelGuardCanceling; }
	bool		IsInWorld()			const	{ return m_bRoleInWorld; }

	//-----------------------------------------------------------------------------
	// ��ɫͨ���������
	//-----------------------------------------------------------------------------
	bool		IsHaveBagPsd()		const	{ return GetBagPsd() != INVALID_VALUE; }

	//-----------------------------------------------------------------------------
	// ��ɫ���
	//-----------------------------------------------------------------------------
	BOOL		FullLogin(Role* pRole, BOOL bFirst);
	VOID		LogoutPlayer();
	VOID		Refresh();

	//-----------------------------------------------------------------------------
	// ����������غ�GM����
	//----------------------------------------------------------------------------
	static VOID RegisterAllPlayerCmd();
	static VOID RegisterALLSendCmd();
	static VOID UnRegisterALL();

	
	//-----------------------------------------------------------------------------
	// GM�������
	//-----------------------------------------------------------------------------
	BOOL		IsPrivilegeEnough(BYTE byPrivilege) const { return byPrivilege <= m_byPrivilege; }
	BOOL		IsGM( ) const { return m_byPrivilege > 0; }
	BYTE		GetPrivilege() const { return m_byPrivilege; }
	//-----------------------------------------------------------------------
	// ������ϢMsg(�����������ã��������ͷ�)
	//-----------------------------------------------------------------------
	VOID		RecycleMsg(LPBYTE p_message);

	//-----------------------------------------------------------------------------
	// ����
	//----------------------------------------------------------------------------
	INT			Update();

	VOID			Log_Free_Message();
	
	//����֤���������Ҫ��֤��ͼƬ
	VOID		SendVerifyCodeMessage();

	BOOL		IsRobort() { return m_sAccountCommon.b_Robort; }
private:
	//-----------------------------------------------------------------------
	// ��Ҫ�����е�ͼ�߳��ϲ㴦�����Ϣע��
	//-----------------------------------------------------------------------
	static VOID	RegisterWorldMsg(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size);

	//-----------------------------------------------------------------------
	// ��Ϣ�������
	//-----------------------------------------------------------------------
	VOID			SendSmallMessage(LPVOID p_message, DWORD dw_size);
	VOID			SendLargeMessage(LPVOID p_message, DWORD dw_size);

	//-----------------------------------------------------------------------
	// �ײ�����
	//-----------------------------------------------------------------------
	LPBYTE			RecvMsg(DWORD& dw_size);
	VOID			ReturnMsg(LPBYTE p_message);
	VOID			SendMsg(LPBYTE p_message, DWORD dw_size);

	//-----------------------------------------------------------------------
	// ѡ�˽������
	//-----------------------------------------------------------------------
	BOOL			IsRoleExist(const DWORD dw_role_id) const;
	BOOL			AddRole(const DWORD dw_role_id);
	BOOL			RemoveRole(const DWORD dw_role_id);
	BOOL			CanSetSafeCode();
	BOOL			CanResetSafeCode() const;
	BOOL			CanCancelSafeCodeReset() const;
	DWORD			GetRoleDelGuardTime(const DWORD dw_role_id) const;
	DWORD			GetRoleChangeNameTime(const DWORD dw_role_id) const;
	VOID			SetRoleDelGuardTime(DWORD dw_role_id, DWORD dw_time);
	VOID			SetChangeNameTime(DWORD dw_role_id, DWORD dw_time);
	BOOL			IsRoleInDelGuard(const DWORD dw_role_id) const;
	BOOL			IsCanChangeName(const DWORD dw_role_id) const;

	//-----------------------------------------------------------------------
	// �ʺ��½�ɫͨ��������ز���
	//-----------------------------------------------------------------------
	const DWORD		GetBagPsd()		const { return m_sAccountCommon.dw_bag_password_crc_; }
	const DWORD		GetSafeCode()	const { return m_sAccountCommon.s_safe_code_.dw_safe_code_crc; }

	VOID			SetBagPsd(DWORD dwNewPsdCrc);

	/************************************************************************
	** Handlers -- map thread
	*************************************************************************/

	//-----------------------------------------------------------------------
	// ������Ϸ
	//-----------------------------------------------------------------------
	DWORD	HandleJoinGame(tag_net_message* pCmd);

	//-----------------------------------------------------------------------
	// ��ɫ������ɾ������ȡ��ѡ��
	//-----------------------------------------------------------------------
	DWORD	HandleRoleCreate(tag_net_message* pCmd);
	DWORD	HandleRoleEnum(tag_net_message* pCmd);
	DWORD	HandleRoleDelete(tag_net_message* pCmd);
	DWORD	HandleRoleGuardCancel(tag_net_message* pCmd);
	DWORD	HandleRoleSelect(tag_net_message* pCmd);
	DWORD	HandleRoleChangeName(tag_net_message* pCmd);
	DWORD	HandleRoleCheckName(tag_net_message* pCmd);

	//-----------------------------------------------------------------------
	// ��ȡ�˺Ž���
	//-----------------------------------------------------------------------
	DWORD HandleReceiveAccountReward(tag_net_message* pCmd);
	DWORD HandleReceiveAccountRewardEx(tag_net_message* pCmd);
	DWORD HandleReceiveSerialReward(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��ȫ�봦��
	//------------------------------------------------------------------------
	DWORD	HandleRoleSetSafeCode(tag_net_message* pCmd);
	DWORD	HandleRoleResetSafeCode(tag_net_message* pCmd);
	DWORD	HandleRoleCancelSafeCodeReset(tag_net_message* pCmd);
	
	DWORD	HandleResetVerCode(tag_net_message* pCmd);
	DWORD	HandleNeedVerCode(tag_net_message* pCmd);
	DWORD	HandleRoleCodeCheck(tag_net_message* pCmd);
	DWORD	HandleGotoVer(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// �������Ի�ȡ
	//------------------------------------------------------------------------
	DWORD	HandleGetRoleInitAtt(tag_net_message* pCmd);
	DWORD	HandleGetRemoteUnitAtt(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ����
	//------------------------------------------------------------------------
	DWORD	HandleRoleWalk(tag_net_message* pCmd);
	DWORD	HandleRoleStopWalk(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// װ�����
	//------------------------------------------------------------------------
	DWORD	HandleRoleEquip(tag_net_message* pCmd);
	DWORD	HandleRoleUnequip(tag_net_message* pCmd);
	DWORD	HandleRoleSwapWeapon(tag_net_message* pCmd);
	DWORD	HandleRoleIdentifyEquip(tag_net_message* pCmd);
	DWORD	HandleRoleEquipRepair(tag_net_message* pCmd);
	DWORD	HandleRoleEquipBind(tag_net_message* pCmd);
	DWORD	HandleRoleEquipUnBind(tag_net_message* pCmd);

	DWORD	HandleEquipDestroy(tag_net_message* pCmd);

	DWORD	HandleRoleEquipReatt(tag_net_message* pCmd);
	DWORD	HandleRoleEquipKaiguang(tag_net_message* pCmd);
	
	DWORD	HandleRoleEquipXili(tag_net_message* pCmd);
	DWORD	HandleRoleEquipXiliChange(tag_net_message* pCmd);
	DWORD	HandleRoleEquipGetWuhuen(tag_net_message* pCmd);
	DWORD	HandleRoleEquipRongLian(tag_net_message* pCmd);
	DWORD	HandleShiPinFumo(tag_net_message* pCmd);
	DWORD	HandleEquipChange(tag_net_message* pCmd);
	DWORD	HandleEquipLuckYou(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// ��ɫ�����ʾ����
	//------------------------------------------------------------------------
	DWORD	HandleRoleSetFashion(tag_net_message* pCmd);
	DWORD	HandleRoleSetDisplay(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��Ʒ�������
	//------------------------------------------------------------------------
	DWORD	HandleRoleChangeItemPos(tag_net_message* pCmd);
	DWORD	HandleRoleChangeItemPosEx(tag_net_message* pCmd);
	DWORD	HandleRoleReorderItem(tag_net_message* pCmd);
	DWORD	HandleRoleReorderItemEx(tag_net_message* pCmd);
	DWORD	HandleRoleStackItem(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// ��Ҽ佻�����
	//------------------------------------------------------------------------
	DWORD	HandleRoleExchangeReq(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeReqRes(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeAdd(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeDec(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeMoney(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeLock(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeCancel(tag_net_message* pCmd);
	DWORD	HandleRoleExchangeVerify(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �̵����
	//------------------------------------------------------------------------
	DWORD	HandleRoleGetShopItems(tag_net_message* pCmd);
	DWORD	HandleRoleGetShopEquips(tag_net_message* pCmd);
	DWORD	HandleRoleBuyShopItem(tag_net_message* pCmd);
	DWORD	HandleRoleBuyShopEquip(tag_net_message* pCmd);
	DWORD	HandleRoleSellToShop(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��̯���
	//------------------------------------------------------------------------
	DWORD	HandleRoleStallStart(tag_net_message* pCmd);
	DWORD	HandleRoleStallSetGoods(tag_net_message* pCmd);
	DWORD	HandleRoleStallUnsetGoods(tag_net_message* pCmd);
	DWORD	HandleRoleStallSetTitle(tag_net_message* pCmd);
	DWORD	HandleRoleStallSetAdText(tag_net_message* pCmd);
	DWORD	HandleRoleStallSetFinish(tag_net_message* pCmd);
	DWORD	HandleRoleStallClose(tag_net_message* pCmd);
	DWORD	HandleRoleStallGet(tag_net_message* pCmd);
	DWORD	HandleRoleStallGetTitle(tag_net_message* pCmd);
	DWORD	HandleRoleStallGetAd(tag_net_message* pCmd);
	DWORD	HandleRoleStallBuy(tag_net_message* pCmd);
	DWORD	HandleRoleStallGetSpec(tag_net_message* pCmd);
	DWORD	HandleStallChat( tag_net_message* pCmd );
	DWORD	HandleStallHistoryChat( tag_net_message* pCmd );

	//------------------------------------------------------------------------
	// ��վ&Ǭ��ʯ
	//------------------------------------------------------------------------
	DWORD	HandleRoleDak(tag_net_message* pCmd);
	DWORD	HandleRoleInstanceSaodang(tag_net_message* pCmd);
	DWORD	HandleRoleSaodangOver(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// ʹ��ĥʯ
	//------------------------------------------------------------------------
	DWORD	HandleRoleAbrase(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��ɫ�ֿ�
	//------------------------------------------------------------------------
	//DWORD	HandleRoleWareOpen(tag_net_message* pCmd);
	//DWORD	HandleRoleWareExtend(tag_net_message* pCmd);
	DWORD	HandleRoleBagExtand(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��ɫ�ֿ��д�ȡ��Ǯ&Ԫ��
	//------------------------------------------------------------------------
	DWORD	HandleRoleSaveSilver(tag_net_message* pCmd);
	DWORD	HandleRoleGetSilver(tag_net_message* pCmd);
	//DWORD	HandleRoleSaveYuanBao(tag_net_message* pCmd);
	DWORD	HandleRoleGetYuanBao(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ���Ҽ������
	//------------------------------------------------------------------------
	DWORD	HandleRoleSetBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleUnsetBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleCheckBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleResetBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleOpenBagPsd(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ����
	//------------------------------------------------------------------------
	DWORD	HandleRoleEnterWorld(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ����
	//------------------------------------------------------------------------
	DWORD   HandleRoleChat(tag_net_message* pCmd);
	DWORD   HandleRoleGetID(tag_net_message* pCmd);
	DWORD	HandleRoleGetNameByNameID(tag_net_message* pCmd);
	DWORD	HandleRoleGetSomeName(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// װ��չʾ
	//------------------------------------------------------------------------
	DWORD   HandleRoleShowEquip(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// ��Ʒչʾ
	//------------------------------------------------------------------------
	DWORD   HandleRoleShowItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��ȡ����
	//------------------------------------------------------------------------
	DWORD   HandleRoleLoadLeftMsg(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��ͼ�¼�
	//------------------------------------------------------------------------
	DWORD	HandleRoleMapTrigger(tag_net_message* pCmd);
	DWORD	HandleRoleInstanceNotify(tag_net_message* pCmd);
	DWORD	HandleRoleInstanceAgree(tag_net_message* pCmd);
	DWORD	HandleRoleLeaveInstance(tag_net_message* pCmd);
	DWORD	HandleEnterPVPInstance(tag_net_message* pCmd);
	DWORD	HandleLeavePVPInstance(tag_net_message* pCmd);
	DWORD   HandleLeave1v1(tag_net_message* pCmd);
	DWORD	HandleEnterBattle(tag_net_message* pCmd);
	//-------------------------------------------------------------------------
	// ���Ե����
	//-------------------------------------------------------------------------
	DWORD	HandleRoleBidAttPoint(tag_net_message* pCmd);
	DWORD	HandleRoleClearAttPoint(tag_net_message* pCmd);

	//-------------------------------------------------------------------------
	// ���ʼ������
	//------------------------------------------------------------------------
	DWORD	HandleRoleLearnSkill(tag_net_message* pCmd);
	DWORD	HandleRoleLevelUpSkill(tag_net_message* pCmd);
	DWORD	HandleRoleForgetSkill(tag_net_message* pCmd);
	DWORD	HandleRoleClearTalent(tag_net_message* pCmd);
	
	DWORD	HandleRoleEquipLearnSkill(tag_net_message* pCmd);
	DWORD	HandleRoleEquipLevelUpSkill(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// ս��ϵͳ
	//------------------------------------------------------------------------
	DWORD	HandleRoleEnterCombat(tag_net_message* pCmd);
	DWORD	HandleRoleLeaveCombat(tag_net_message* pCmd);
	DWORD	HandleRoleSkill(tag_net_message* pCmd);
	DWORD	HandleRoleInterruptSkill(tag_net_message* pCmd);

	DWORD	HandleRoleCancelBuff(tag_net_message* pCmd);
	DWORD	HandleTargetChange(tag_net_message* pCmd);

	DWORD	HandleDuelAskFor(tag_net_message* pCmd);
	DWORD	HandleDuelResponse(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// PKϵͳ
	//------------------------------------------------------------------------
	DWORD   HandleRoleChangePKState(tag_net_message* pCmd);
	DWORD	HandleRoleUseKillBadage( tag_net_message* pCmd );

	//------------------------------------------------------------------------
	// ����
	//------------------------------------------------------------------------
	DWORD	HandleRoleBindRebornMap(tag_net_message* pCmd);
	DWORD	HandleRoleRevive(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ͨ�ú��� -- ���ڵ�ͼ�߳��ϲ㴦�����Ϣ
	//------------------------------------------------------------------------
	DWORD   HandleRoleMsg2World(tag_net_message* pCmd) { return RET_TRANS; }

	//------------------------------------------------------------------------
	// ͨ������ -- GM
	//------------------------------------------------------------------------
	DWORD   HandleGMCommand(tag_net_message* pCmd);
	
	//------------------------------------------------------------------------
	// �������
	//------------------------------------------------------------------------
	DWORD	HandleRoleNPCAcceptQuest(tag_net_message* p_cmd);
	DWORD	HandleRoleTriggerAcceptQuest(tag_net_message* p_cmd);
	DWORD	HandleRoleCompleteQuest(tag_net_message* p_cmd);
	DWORD   HandleRoleDeleteQuest(tag_net_message* p_cmd);
	DWORD	HandleRoleUpdateQuestNPCTalk(tag_net_message* p_cmd);
	DWORD	HandleTrackQuest(tag_net_message* p_cmd);
	DWORD	HandleCircleQuestList(tag_net_message* pCmd);
	DWORD	HandleRefreshCircleQuest(tag_net_message* pCmd);
	DWORD   HandlePutQuest(tag_net_message* p_cmd);
	DWORD   HandleGetQuest(tag_net_message* p_cmd);
	//DWORD   HandleGiveUpQuest(tag_net_message* p_cmd);
	DWORD   HandleGetOnePageGDQuest(tag_net_message* p_cmd);
	DWORD	HandleUpdateMyPutGDQuest(tag_net_message* p_cmd);
	DWORD	HandleUpdateMyGetGDQuest(tag_net_message* p_cmd);
	DWORD	HandleCancelPutGDQuest(tag_net_message* p_cmd);
	DWORD	HandleTeamNextQuest(tag_net_message* p_cmd);
	DWORD HandleBuyRefreshCircleQuest(tag_net_message *pCmd);
	DWORD HandleBuyCircleQuestPerdayNumber(tag_net_message *pCmd);

	//------------------------------------------------------------------------
	// ���а�
	//------------------------------------------------------------------------
	DWORD	HandleGetLevelRank(tag_net_message* pCmd );
	DWORD   HandleGetEquipRank(tag_net_message* pCmd);
	DWORD	HandleGetGuildRank(tag_net_message* pCmd);
	DWORD	HandleGetKillRank(tag_net_message* pCmd);
	DWORD	HandleGetJusticeRank(tag_net_message* pCmd);
	DWORD	HandGet1v1Rank(tag_net_message* pCmd);
	DWORD   HandleGetShihunRank(tag_net_message* pCmd);
	DWORD	HandleGetAchPointRank(tag_net_message* pCmd);
	DWORD	HandleGetAchNumberRank(tag_net_message* pCmd);
	DWORD	HandleGetMasterRank(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// ��Ʒǿ��
	//------------------------------------------------------------------------
	//DWORD	HandleRolePosyEquip(tag_net_message* pCmd);
	//DWORD	HandleRoleEngraveEquip(tag_net_message* pCmd);
	DWORD	HandleRoleInlayEquip(tag_net_message* pCmd);
	DWORD	HandleRoleChisel(tag_net_message* pCmd);
	DWORD	HandleRoleEquipShengXing(tag_net_message* pCmd);
	DWORD   HandleRoleUnbeset(tag_net_message* pCmd);
	DWORD	HandleRoleDamo(tag_net_message* pCmd);
	DWORD	HandleRolePracticeBegin(tag_net_message* pCmd);
	DWORD	HandleRolePracticeEnd(tag_net_message* pCmd);
	DWORD	HandleRoleFusion(tag_net_message* pCmd);
	DWORD	HandleRoleWeaponClearTalent(tag_net_message* pCmd);
	DWORD	HandleRoleLeavePracitice(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// �����ϳ���Ʒ
	//------------------------------------------------------------------------
	DWORD	HandleRoleProduceItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �㻯, װ���ֽ�
	//------------------------------------------------------------------------
	DWORD	HandleRoleDeCompose(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ʹ����Ʒ
	//------------------------------------------------------------------------
	DWORD	HandleRoleUseItem(tag_net_message* pCmd);
	DWORD	HandleRoleInterruptUseItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ʰȡ��Ʒ
	//------------------------------------------------------------------------
	DWORD	HandleRolePickUpItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �ӵ���Ʒ
	//------------------------------------------------------------------------
	DWORD	HandleRolePutDownItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �������
	//------------------------------------------------------------------------
	DWORD   HandleRoleMakeFriend(tag_net_message* pCmd);
	DWORD	HandleRoleCancelFriend(tag_net_message* pCmd);
	DWORD	HandleUpdateFriendGroup(tag_net_message* pCmd);
	DWORD	HandleMoveBlackList(tag_net_message* pCmd);
	DWORD	HandleAddEnemy(tag_net_message* pCmd);
	DWORD	HandleDeleteBlackList(tag_net_message* pCmd);
	DWORD   HandleDeleteEnemyList(tag_net_message* pCmd);
	DWORD   HandleGetEnemyPos(tag_net_message* pCmd);
	DWORD	HandleRoleSendGift(tag_net_message* pCmd);
	DWORD	HandleRoleSendGiftReply(tag_net_message* pCmd);
	DWORD   HandleUpdateFriOnline(tag_net_message* pCmd);
	DWORD	HandleGetRoleSignature(tag_net_message* pCmd);
	DWORD	HandleChangeSignature(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// С�����
	//------------------------------------------------------------------------
	DWORD   HandleOwnCreateTeam(tag_net_message* pCmd);
	DWORD   HandleDismissTeam(tag_net_message* pCmd);
	DWORD	HandleApplyJoinTeam(tag_net_message* pCmd);
	DWORD   HandleApplyJoinTeamReply(tag_net_message* pCmd);
	DWORD   HandleCleanApply(tag_net_message* pCmd);
	DWORD	HandleMemInviteJoinTeam(tag_net_message* pCmd);
	DWORD	HandleMemInviteJoinTeamReply(tag_net_message* pCmd);
	DWORD	HandleChangeTeamPlacard(tag_net_message* pCmd);
	DWORD	HandleGetMapTeamInfo(tag_net_message* pCmd);
	DWORD   HandleRoleJoinTeam(tag_net_message* pCmd);
	DWORD	HandleRoleJoinTeamReply(tag_net_message* pCmd);
	DWORD	HandleRoleKickMember(tag_net_message* pCmd);
	DWORD	HandleRoleLeaveTeam(tag_net_message* pCmd);
	DWORD	HandleRoleSetPickMol(tag_net_message* pCmd);
	DWORD	HandleRoleChangeLeader(tag_net_message* pCmd);
	DWORD	HandleRoleSetAssignQuality( tag_net_message* pCmd );
	DWORD	HandleRoleLeaderAssign( tag_net_message* pCmd );
	DWORD	HandleRoleTeamAssignSice( tag_net_message* pCmd );
	DWORD	HandleRoleTeamSetShareLeaderCircleQuest( tag_net_message* pCmd );
	
	//------------------------------------------------------------------------
	// ˫����� gx add 2013.6.27
	//------------------------------------------------------------------------
	DWORD	HandleInviteRoleComPractice(tag_net_message* pCmd);//������ҽ���˫��
	DWORD	HandleInviteRoleCompracticeReply(tag_net_message* pCmd);
	DWORD   HandleRoleCancelCompractice(tag_net_message* pCmd);//���ȡ��˫��
	//------------------------------------------------------------------------
	// ������ gx add 2013.7.3
	//------------------------------------------------------------------------
	DWORD	HandleMalePropose(tag_net_message* pCmd);//���������Ů��������
	DWORD	HandleProposeFemaleReply(tag_net_message* pCmd);
	DWORD   HandleRoleDivorce(tag_net_message* pCmd);//���
	DWORD	HandleGetQbjjReward(tag_net_message* pCmd);//���˫����ȡ��Ƚ�ά
	//------------------------------------------------------------------------
	// �ƺ����
	//------------------------------------------------------------------------
	DWORD HandleRoleActiveTitle(tag_net_message* pCmd);
	DWORD HandleRoleGetTitles(tag_net_message* pCmd);
	DWORD HandleRoleTitleBuy(tag_net_message* pCmd);
	DWORD HandleRoleTitleReturn(tag_net_message* pCmd);
	DWORD HandleRoleShowActiveTitle(tag_net_message* pCmd);//gx add 2013.10.31
	//------------------------------------------------------------------------
	// �������
	//------------------------------------------------------------------------
	DWORD HandleRoleGetVCard(tag_net_message* pCmd);
	DWORD HandleRoleSetVCard(tag_net_message* pCmd);
	DWORD HandleGetHeadPicUrl(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ���������
	//------------------------------------------------------------------------
	DWORD HandleGetFatigueInfo(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ���������
	//------------------------------------------------------------------------
	DWORD HandleGetFameHallRoles(tag_net_message* pCmd);
	DWORD HandleGetReputeTop(tag_net_message* pCmd);
	DWORD HandleGetActClanTreasure(tag_net_message* pCmd);
	DWORD HandleActiveTreasure(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �������
	//------------------------------------------------------------------------
	DWORD HandleGetRoleClanData(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ��������
	//------------------------------------------------------------------------
	DWORD HandleGameGuarderMsg(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �������
	//------------------------------------------------------------------------
	DWORD HandleGetPetAttr(tag_net_message* pCmd);
	DWORD HandlePetSkill(tag_net_message* pCmd);
	DWORD HandleUsePetEgg(tag_net_message* pCmd);
	DWORD HandleDeletePet(tag_net_message* pCmd);
	DWORD HandleGetPetDispInfo(tag_net_message* pCmd);
	DWORD HandleSetPetState(tag_net_message* pCmd);
	DWORD HandlePetReName(tag_net_message* pCmd);
	DWORD HandleUseSpePetItem(tag_net_message* pCmd);
	DWORD HandlePocketSizeChange(tag_net_message* pCmd);

	DWORD HandlePetEquip(tag_net_message* pCmd);
	DWORD HandlePetUnEquip(tag_net_message* pCmd);
	DWORD HandlePetSwapEquipPos(tag_net_message* pCmd);
	DWORD HandleGetPetPourExpMoneyNeed(tag_net_message* pCmd);
	DWORD HandlePetPourExp(tag_net_message* pCmd);
	DWORD HandlePetUpStep(tag_net_message* pCmd);
	DWORD HandlePetEnhance(tag_net_message* pCmd);
	DWORD HandlePetLearnSkill(tag_net_message* pCmd);
	DWORD HandlePetLevelUpSkill(tag_net_message* pCmd);
	DWORD HandlePetForgetSkill(tag_net_message* pCmd);
	DWORD HandlePetInvite(tag_net_message* pCmd);
	DWORD HandlePetOnInvite(tag_net_message* pCmd);
	DWORD HandlePetFood(tag_net_message* pCmd);
	DWORD HandlePetSetLock(tag_net_message* pCmd);
	DWORD HandleRebornPet(tag_net_message* pCmd);
	DWORD HandlePetPaiqian(tag_net_message* pCmd);
	DWORD HandlePetReturn(tag_net_message* pCmd);
	DWORD HandleBuyLoveValue(tag_net_message* pCmd);
	DWORD HandlePetColor(tag_net_message* pCmd);
	DWORD HandlePetChange(tag_net_message* pCmd);
	DWORD HandlePetaddPoint(tag_net_message* pCmd);
	DWORD HandlePetFusion(tag_net_message* pCmd);
	DWORD HandlePetXiulianSizeChange(tag_net_message* pCmd);
	DWORD HandlePetXiulian(tag_net_message* pCmd);
	DWORD HandlePetXiulianReturn(tag_net_message* pCmd);

	DWORD	HandleRolePetExchangeReq(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeReqRes(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeAdd(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeDec(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeMoney(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeLock(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeCancel(tag_net_message* pCmd);
	DWORD	HandleRolePetExchangeVerify(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 1v1������
	//------------------------------------------------------------------------
	DWORD	Handle1v1Apply(tag_net_message* pCmd);
	DWORD	Handle1v1LeaveQueue(tag_net_message* pCmd);
	DWORD	HandleGet1v1ScoreAward(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// Լս������
	//------------------------------------------------------------------------
	DWORD	HandleReservationApply(tag_net_message* pCmd);
	DWORD	HandleGetReservationInfo(tag_net_message* pCmd);
	DWORD	HandleReservationResult(tag_net_message* pCmd);


	//------------------------------------------------------------------------
	// �ٱ������
	//------------------------------------------------------------------------
	DWORD HandleInitBaiBaoRecord(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// VIP̯λ���
	//------------------------------------------------------------------------
	DWORD HandleGetAllVIPStallInfo(tag_net_message* pCmd);
	DWORD HandleUpdateVIPStallInfo(tag_net_message* pCmd);
	DWORD HandleApplyVIPStall(tag_net_message* pCmd);
	DWORD HandleSpecVIPStallGet(tag_net_message* pCmd);
	DWORD HandleBuyVIPStallGoods(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// NPC���
	//------------------------------------------------------------------------
	DWORD HandleTalkToNPC(tag_net_message* pCmd);


	//------------------------------------------------------------------------
	// ���Զ���
	//------------------------------------------------------------------------
	DWORD	HandleRoleStyleAction(tag_net_message* pCmd);
	DWORD	HandleRoleDuetMotionInvite(tag_net_message* pCmd);
	DWORD	HandleRoleDuetMotionOnInvite(tag_net_message* pCmd);
	DWORD	HandleRoleDuetMotionStart(tag_net_message* pCmd);

	/************************************************************************
	** Handlers -- upper all map thread
	*************************************************************************/

	//------------------------------------------------------------------------
	// �̳����
	//------------------------------------------------------------------------
	DWORD HandleRoleMallGet(tag_net_message* pCmd);
	DWORD HandleRoleMallUpdate(tag_net_message* pCmd);
	DWORD HandleRoleMallBuyItem(tag_net_message* pCmd);
	DWORD HandleRoleMallBuyPack(tag_net_message* pCmd);
	DWORD HandleRoleMallPresentItem(tag_net_message* pCmd);
	DWORD HandleRoleMallPresentPack(tag_net_message* pCmd);
	DWORD HandleRoleMallFreeGetItem(tag_net_message* pCmd);
	DWORD HandleRoleMallLaunchGroupPurchase(tag_net_message* pCmd);
	DWORD HandleRoleMallRespondGroupPurchase(tag_net_message* pCmd);
	DWORD HandleRoleMallGetGroupPurchaseInfo(tag_net_message* pCmd);
	DWORD HandleRoleMallGetParticipators(tag_net_message* pCmd);
	DWORD HandleRoleMallItemExchange(tag_net_message* pCmd);
	DWORD HandleRoleMallPackExchange(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// Ԫ���������
	//------------------------------------------------------------------------
	DWORD HandleRoleSaveYB2Account(tag_net_message* pCmd);
	DWORD HandleRoleSaveSilver2Account(tag_net_message* pCmd);
	DWORD HandleRoleDepositYBAccount(tag_net_message* pCmd);
	DWORD HandleRoleDepositSilver(tag_net_message* pCmd);
	DWORD HandleRoleGetYBTradeInfo(tag_net_message* pCmd);
	DWORD HandleRoleSubmitSellOrder(tag_net_message* pCmd);
	DWORD HandleRoleSubmitBuyOrder(tag_net_message* pCmd);
	DWORD HandleRoleDeleteOrder(tag_net_message* pCmd);
	DWORD HandleRoleGetYBOrder(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �������
	//------------------------------------------------------------------------
	DWORD HandleCreateGuild(tag_net_message* pCmd);
	DWORD HandleDismissGuild(tag_net_message* pCmd);
	DWORD HandleJoinGuildReq(tag_net_message* pCmd);
	DWORD HandleJoinGuildReqByName(tag_net_message* pCmd);
	DWORD HandleJoinGuildReqRes(tag_net_message* pCmd);
	DWORD HandleLeaveGuild(tag_net_message* pCmd);
	DWORD HandleKickFromGuild(tag_net_message* pCmd);
	DWORD HandleTurnoverGuild(tag_net_message* pCmd);
	DWORD HandleDemissFromGuild(tag_net_message* pCmd);
	DWORD HandleAppointForGuild(tag_net_message* pCmd);
	DWORD HandleChangeGuildTenet(tag_net_message* pCmd);
	DWORD HandleChangeGuildSymbol(tag_net_message* pCmd);
	DWORD HandleGuildPosNameChange(tag_net_message* pCmd);
	DWORD HandleGuildPosPowerChange(tag_net_message* pCmd);
	DWORD HandleEnterGuildMap(tag_net_message* pCmd);
	DWORD HandleSetEnemyGuild(tag_net_message* pCmd);
	DWORD HandleDelEnemyGuild(tag_net_message* pCmd);
	DWORD HandleGetEnemyData(tag_net_message* pCmd);
	DWORD HandleGuildDeclareWar(tag_net_message* pCmd);
	DWORD HandleGuildDeclareWarRes(tag_net_message* pCmd);
	DWORD HandleGuildQualifyWar(tag_net_message* pCmd);
	DWORD HandleGuildDisqualifyWar(tag_net_message* pCmd);
	DWORD HandleGuildWarNum(tag_net_message* pCmd);
	DWORD HandleGuildWarMoneyParam(tag_net_message* pCmd);

	DWORD HandleStartGuildPractice(tag_net_message* pCmd);
	DWORD HandleGuildTripodInfo(tag_net_message* pCmd);
	DWORD HandleGuildGetFund(tag_net_message* pCmd);
	DWORD HandleGuildWarHistory(tag_net_message* pCmd);
	DWORD HandleGuildWarRelay(tag_net_message* pCmd);
	DWORD HandleGuildIncreaseFund(tag_net_message* pCmd);
	DWORD HandleGuildMaterialReceive(tag_net_message* pCmd);
	DWORD HandleGuilddonateFund(tag_net_message* pCmd);
	DWORD HandleGetAllGuildInfo(tag_net_message* pCmd);
	DWORD HandleGuildMianzhan(tag_net_message* pCmd);
	DWORD HandleGuildModifyApplyLevel(tag_net_message* pCmd);
	DWORD HandleSignUpAttack(tag_net_message* pCmd);
	DWORD HandleGetSBKData(tag_net_message* pCmd);
	DWORD HandleGetSBKReward(tag_net_message* pCmd);

	DWORD HandleJoinGuildRecruit(tag_net_message* pCmd);
	DWORD HandleLeaveGuildRecruit(tag_net_message* pCmd);
	DWORD HandleQueryGuildRecruit(tag_net_message* pCmd);
	DWORD HandleQueryPageGuildRecruit(tag_net_message* pCmd);
	DWORD HandleAgreeJoin(tag_net_message* pCmd);
	DWORD HandlenoAgreeJoin(tag_net_message* pCmd);

	DWORD HandleGetDKP(tag_net_message* pCmd);
	DWORD HandldSetDKP(tag_net_message* pCmd);
	DWORD HandleDKPAffirmance(tag_net_message* pCmd);

	DWORD HandleInviteLeague(tag_net_message* pCmd);
	DWORD HandleInviteLeagueRes(tag_net_message* pCmd);
	DWORD HandelRelieveLeague(tag_net_message* pCmd);

	DWORD HandleInviteSign(tag_net_message* pCmd);
	DWORD HandleCancelSign(tag_net_message* pCmd);
	DWORD HandleAffirmanceSign(tag_net_message* pCmd);
	DWORD HandleReferSign(tag_net_message* pCmd);

	DWORD HandleGetGuildDelateData(tag_net_message* pCmd);
	DWORD HandleGetGuildDelateContent(tag_net_message* pCmd);
	DWORD HandleDelateLeader(tag_net_message* pCmd);
	DWORD HandleDelateBallot(tag_net_message* pCmd);

	DWORD HandleSyncGuildInfo(tag_net_message* pCmd);
	DWORD HandleSyncGuildWarInfo(tag_net_message* pCmd);

	DWORD HandleGetGuildMembers(tag_net_message* pCmd);
	DWORD HandleGetGuildMemberEx(tag_net_message* pCmd);
	DWORD HandleRefreshGuildMember(tag_net_message* pCmd);
	DWORD HandleGetGuildName(tag_net_message* pCmd);
	DWORD HandleGetGuildTenet(tag_net_message* pCmd);
	DWORD HandleGetGuildSymbol(tag_net_message* pCmd);

	DWORD HandleGetGuildWareItems(tag_net_message* pCmd);
	DWORD HandleGetGuildWarePriList(tag_net_message* pCmd);
	DWORD HandleGuildWarePrivilege(tag_net_message* pCmd);

	DWORD HandleGetGuildFacilitiesInfo(tag_net_message* pCmd);
	DWORD HandleHandInItems(tag_net_message* pCmd);

	DWORD HandleSpreadGuildAffair(tag_net_message* pCmd);

	DWORD HandleGetGuildSkillInfo(tag_net_message* pCmd);
	DWORD HandleUpgradeGuildSkill(tag_net_message* pCmd);
	DWORD HandleLearnGuildSkill(tag_net_message* pCmd);
	DWORD HandleSetResearchSkill(tag_net_message* pCmd);

	DWORD HandleGetCofCInfo(tag_net_message* pCmd);
	DWORD HandleCloseCofC(tag_net_message* pCmd);
	DWORD HandleBuyCofCGoods(tag_net_message* pCmd);
	DWORD HandleSellCofCGoods(tag_net_message* pCmd);
	DWORD HandleGetCommodityInfo(tag_net_message* pCmd);
	DWORD HandleGetTaelInfo(tag_net_message* pCmd);
	DWORD HandleGetCommerceRank(tag_net_message* pCmd);

	DWORD HandleAcceptCommerce(tag_net_message* pCmd);
	DWORD HandleCompleteCommerce(tag_net_message* pCmd);
	DWORD HandleAbandonCommerce(tag_net_message* pCmd);
	DWORD HandleSwitchCommendation(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �����������
	//------------------------------------------------------------------------
	DWORD HandleOpenTreasureChest(tag_net_message* pCmd);
	DWORD HandleStopTreasureChest(tag_net_message* pCmd);
	DWORD HandleAgainTreasureChest(tag_net_message* pCmd);
	DWORD HandleChestGetItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// ���ؽ�ɫѡ��
	//------------------------------------------------------------------------
	DWORD HandleReturnRoleSelect(tag_net_message* pCmd);

	// �ͻ��˶Ի��򷢸������ȱʡ��Ϣ
	DWORD HandleDlgDefaultMsg(tag_net_message* pCmd);
	// �ͻ��˴����������ű���ȱʡ��Ϣ
	DWORD HandleDefaultRequest(tag_net_message* pCmd);

	// ���������ҵ�װ����Ϣ
	DWORD HandleGetSomeoneEquip(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// �ʼ�������
	//------------------------------------------------------------------------
	DWORD HandleSendMail(tag_net_message* pCmd);
	DWORD HandleGetOwnMail(tag_net_message* pCmd);
	DWORD HandleGetOneMail(tag_net_message* pCmd);
	DWORD HandleGetMailContent(tag_net_message* pCmd);
	DWORD HandleGetMailItem(tag_net_message* pCmd);
	DWORD HandleAcceptAccessoryMailItem(tag_net_message* pCmd);
	DWORD HandleReadMail(tag_net_message* pCmd);
	DWORD HandleDeleteMail(tag_net_message* pCmd);
	DWORD HandleReturnMail(tag_net_message* pCmd);
	DWORD HandleGetMailNum(tag_net_message* pCmd);
	
	//--------------------------------------------------------------------
	// ���� 
	//--------------------------------------------------------------------
	DWORD HandleUpgRide( tag_net_message* pCmd );
	DWORD HandleRideInlay( tag_net_message* pCmd );
	DWORD HandleRemoveRideInlay( tag_net_message* pCmd );
	DWORD HandleBeginRide( tag_net_message* pCmd );
	DWORD HandleInterruptRide( tag_net_message* pCmd );
	DWORD HandleCancelRide( tag_net_message* pCmd );
	DWORD HandleEquipRide(tag_net_message* pCmd);
	DWORD HandleUnEquipRide(tag_net_message* pCmd);
	DWORD HandletogRide(tag_net_message* pCmd);
	DWORD HandleGetRideAtt(tag_net_message* pCmd);
	//--------------------------------------------------------------------
	// ʦͽ
	//--------------------------------------------------------------------
	DWORD HandleMakeMaster( tag_net_message* pCmd );
	DWORD HandleMakeMasterEx( tag_net_message* pCmd );
	DWORD HandleMakePrentice( tag_net_message* pCmd );
	DWORD HandleMakePrenticeEx( tag_net_message* pCmd );
	DWORD HandleMasterPrenticeBreak( tag_net_message* pCmd );
	DWORD HandleGetMasterPlacard( tag_net_message* pCmd );
	DWORD HandleShowInMasterPlacard( tag_net_message* pCmd );
	DWORD HandleCallInMaster( tag_net_message* pCmd );
	DWORD HandlePrenticeCallIn( tag_net_message* pCmd );
	DWORD HandleJoinMasterRecruit(tag_net_message* pCmd);
	DWORD HandleLeaveMasterRecruit(tag_net_message* pCmd);
	DWORD HandleQueryPageMasterRecruit(tag_net_message* pCmd);
	DWORD HandleSayGoodByteToMaster(tag_net_message* pCmd);
	DWORD HandleMasterTeachPrentice(tag_net_message* pCmd);
	//--------------------------------------------------------------------
	// ��ʼ�һ�
	//--------------------------------------------------------------------
	DWORD HandleStartHang(tag_net_message* pCmd);
	DWORD HandleCancelHang(tag_net_message* pCmd);
	DWORD HandleSetLeaveLineHang(tag_net_message* pCmd);
	DWORD HandleGetLeaveExp(tag_net_message* pCmd);
	DWORD HandlePickLeaveExp(tag_net_message* pCmd);
	DWORD HandleStartHangGetExp(tag_net_message *pCmd);
	DWORD HandleStopHangGetExp(tag_net_message *pCmd);


	DWORD HandleLoadComplete(tag_net_message* pCmd);
	DWORD handle_reset_instance(tag_net_message* pCmd);
	DWORD handle_reset_inst_limit(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// ��������
	//--------------------------------------------------------------------
	DWORD HandleModRoleHelp(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// �Ի�����
	//--------------------------------------------------------------------
	DWORD HandleModRoleTalk(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// ��ݼ�����
	//--------------------------------------------------------------------
	DWORD HandlekeyInfo(tag_net_message* pCmd);
	DWORD HandleModKeyInfo(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// ��Ծ������
	//--------------------------------------------------------------------
	DWORD HandleGetActiveInfo(tag_net_message* pCmd);
	DWORD HandleRoleActiveReceive(tag_net_message* pCmd);
	DWORD HandleRoleActiveDone(tag_net_message* pCmd);
	DWORD HandleGetGuildActiveInfo(tag_net_message* pCmd);
	DWORD HandleGuildActiveReceive(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// ÿ�ջһ������ gx add 2013.12.18
	//--------------------------------------------------------------------
	DWORD HandleDailyActTransmit(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// �������߽���
	//--------------------------------------------------------------------
	DWORD HandleStartNewRoleGift(tag_net_message* pCmd);
	DWORD HandleGetNewRoleGift(tag_net_message* pCmd);
	DWORD HandleBeginRoleGiftTime(tag_net_message* pCmd);
	
	// �ӳ�
	DWORD HandleGetDelay(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	//��ʼ����
	//--------------------------------------------------------------------
	DWORD handle_begin_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//ȡ������
	//--------------------------------------------------------------------
	DWORD handle_cancel_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//��ʼ����
	//--------------------------------------------------------------------
	DWORD handle_jingpai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//һ�ڼ۹���
	//--------------------------------------------------------------------
	DWORD handle_chaw_buy(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//������ѯ
	//--------------------------------------------------------------------
	DWORD handle_paimai_query(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//������ҳ
	//--------------------------------------------------------------------
	DWORD handle_paimai_change_page(tag_net_message* p_cmd);

	//--------------------------------------------------------------------
	//��ʼǮׯ����
	//--------------------------------------------------------------------
	DWORD handle_begin_bank_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//ȡ��Ǯׯ����
	//--------------------------------------------------------------------
	DWORD handle_cancel_role_bank_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//Ǯׯ����
	//--------------------------------------------------------------------
	DWORD handle_begin_bank_jing(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//Ǯׯһ�ڼ۹���
	//--------------------------------------------------------------------
	DWORD handle_bank_chaw_buy(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//��ѯǮׯ��Ϣ
	//--------------------------------------------------------------------
	DWORD handle_query_bank(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//��ҳ
	//--------------------------------------------------------------------
	DWORD handle_bank_change_page(tag_net_message* p_cmd);

	//--------------------------------------------------------------------
	//Ԫ���һ�
	//--------------------------------------------------------------------
	DWORD handle_yuanbao_exchange(tag_net_message* p_cmd);
	DWORD handle_get_yuanbao_exchange_num(tag_net_message* p_cmd);
	DWORD handle_get_bank_radio(tag_net_message* p_cmd);

	//--------------------------------------------------------------------
	// ����
	//--------------------------------------------------------------------
	DWORD HandleStartFishing(tag_net_message* pCmd);
	DWORD HandleStopFishing(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// ����
	//--------------------------------------------------------------------
	DWORD HandleStopCarrySomething(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// �Զ����
	//--------------------------------------------------------------------
	DWORD HandleAutoKillStart(tag_net_message* p_cmd);
	DWORD HandleAutoKillEnd(tag_net_message* p_cmd);

	DWORD HandleConsumeReward(tag_net_message* p_cmd);


	// �齱
	DWORD HandleGetLottery(tag_net_message* pCmd);


	//--------------------------------------------------------------------
	// ǩ��
	//--------------------------------------------------------------------
	DWORD HandleGetSignData(tag_net_message* p_cmd);
	DWORD HandleSign(tag_net_message* p_cmd);
	DWORD HandleSignReward(tag_net_message* p_cmd);
	DWORD HandleGetSignReward(tag_net_message* p_cmd);

	DWORD HandleBuyLingqi(tag_net_message* p_cmd);
	DWORD HandleGetHuenJingData(tag_net_message* p_cmd);
	DWORD HandleHuenLian(tag_net_message* p_cmd);
	DWORD HandleHuenjingOpteron(tag_net_message* p_cmd);
	DWORD HandleHuenjingLevelUp(tag_net_message* p_cmd);
	DWORD HandleHuenjingInlay(tag_net_message* p_cmd);

	DWORD HandleGodLevelUp(tag_net_message* p_cmd);

	DWORD HandleGetRewardData(tag_net_message* p_cmd);
	DWORD HandleReceiveReward(tag_net_message* p_cmd);

	DWORD HandleGetOpenActiveData(tag_net_message* p_cmd);
	DWORD HandleGetOpenActiveReceive(tag_net_message* p_cmd);
private:
	static PlayerNetCmdMgr	m_PlayerNetMgr;						// ��Ӧ�Ŀͻ�����Ϣ������
	static GMCommandMgr		m_GMCommandMgr;						// GM���������

	DWORD					m_dwAccountID;						// session id����Ӧ�����ʺ�ID
	DWORD					m_dwInternalIndex;					// �ײ������ID
	INT						m_nMsgNum;							// ����ײ�δ�������Ϣ����

	bool					m_bRoleLoading;						// ѡ������ʱ���ڵȴ����ݿⷵ��
	bool					m_bRoleEnuming;						// ������Ϸʱ�ȴ�������ѡ�˽���ɫ��Ϣ��ȡ����
	bool					m_bRoleEnumDone;					// ��ȡ��ɫ��Ϣ���
	bool					m_bRoleEnumSuccess;					// ��ȡ��ɫ��Ϣ�Ƿ�ɹ�
	bool					m_bRoleCreating;					// �ȴ�������ɫ
	bool					m_bRoleDeleting;					// ɾ������ʱ�ȴ����ݿⷵ��
	bool					m_bRoleChangeNameing;				// �������ʱ�ȴ����ݿⷵ��
	bool					m_bRoleDelGuardCanceling;			// �ָ�ɾ����ɫ�ı���ʱ��
	bool					m_bRoleVerifying;					// �ȴ���֤������
	BYTE					m_byPrivilege;						// gmȨ��
	INT8					m_n8RoleNum;						// �Ѿ������Ľ�ɫ����

	bool					m_bRoleInWorld;						// ����Ϸ������

	//DWORD					m_dwRoleID[MAX_ROLENUM_ONEACCOUNT];	// �ʺ������н�ɫID
	StaticArraySP<MAX_ROLENUM_ONEACCOUNT, PlayerSessionCommonData> m_SessionCommonData;
	

	char					m_szAccount[X_SHORT_NAME];			// ����˺�

	volatile BOOL			m_bConnectionLost;					// �����Ƿ��Ѿ��Ͽ�
	volatile BOOL			m_bKicked;							// �Ƿ��Ѿ����ߵ�

	s_account_common		m_sAccountCommon;					// �˺�ͨ����Ϣ

	Role*					m_pRole;							// ��Ӧ�Ľ�ɫ����

	DWORD					m_dwIP;								// �ͻ���IP

	FatigueGuarder			m_FatigueGarder;					// ������

	Verification			m_Verification;						// ������֤��

	tagDWORDTime				m_dwPreLoginTime;		//�ϴε�¼ʱ��
	DWORD						m_dwPreLoginIP;			//�ϴε�¼ip

	BYTE					by_delay_send;				// ��������
};


