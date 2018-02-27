
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//玩家连接

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

const INT CON_LOST	=	-1000000;		// 连接断开标志
const INT RET_TRANS	=	-2000000;		// 需上层处理

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
	// 各种Get
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

	// 帐号下角色通用属性相关操作
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
	// 各种Set
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
	// 消息处理相关
	//----------------------------------------------------------------------------
	INT			HandleMessage();
	VOID		SendMessage(LPVOID p_message, DWORD dw_size);
	VOID		BroadcastCurrChannel(LPCTSTR szMsg);

	//-----------------------------------------------------------------------------
	// 选人界面的判断
	//-----------------------------------------------------------------------------
	bool		IsRoleLoading()		const	{ return m_bRoleLoading; }
	bool		IsRoleEnuming()		const	{ return m_bRoleEnuming; }
	bool		IsRoleCreating()	const	{ return m_bRoleCreating; }
	bool		IsRoleDeleting()	const	{ return m_bRoleDeleting; }
	bool		IsRoleChangeNaming()	const	{ return m_bRoleChangeNameing; }
	bool		IsRoleDelGuardCanceling()const	{ return m_bRoleDelGuardCanceling; }
	bool		IsInWorld()			const	{ return m_bRoleInWorld; }

	//-----------------------------------------------------------------------------
	// 角色通用属性相关
	//-----------------------------------------------------------------------------
	bool		IsHaveBagPsd()		const	{ return GetBagPsd() != INVALID_VALUE; }

	//-----------------------------------------------------------------------------
	// 角色相关
	//-----------------------------------------------------------------------------
	BOOL		FullLogin(Role* pRole, BOOL bFirst);
	VOID		LogoutPlayer();
	VOID		Refresh();

	//-----------------------------------------------------------------------------
	// 网络命令相关和GM命令
	//----------------------------------------------------------------------------
	static VOID RegisterAllPlayerCmd();
	static VOID RegisterALLSendCmd();
	static VOID UnRegisterALL();

	
	//-----------------------------------------------------------------------------
	// GM命令相关
	//-----------------------------------------------------------------------------
	BOOL		IsPrivilegeEnough(BYTE byPrivilege) const { return byPrivilege <= m_byPrivilege; }
	BOOL		IsGM( ) const { return m_byPrivilege > 0; }
	BYTE		GetPrivilege() const { return m_byPrivilege; }
	//-----------------------------------------------------------------------
	// 回收消息Msg(有用则再利用，无用则释放)
	//-----------------------------------------------------------------------
	VOID		RecycleMsg(LPBYTE p_message);

	//-----------------------------------------------------------------------------
	// 更新
	//----------------------------------------------------------------------------
	INT			Update();

	VOID			Log_Free_Message();
	
	//向验证码服务器索要验证码图片
	VOID		SendVerifyCodeMessage();

	BOOL		IsRobort() { return m_sAccountCommon.b_Robort; }
private:
	//-----------------------------------------------------------------------
	// 需要在所有地图线程上层处理的消息注册
	//-----------------------------------------------------------------------
	static VOID	RegisterWorldMsg(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size);

	//-----------------------------------------------------------------------
	// 消息处理相关
	//-----------------------------------------------------------------------
	VOID			SendSmallMessage(LPVOID p_message, DWORD dw_size);
	VOID			SendLargeMessage(LPVOID p_message, DWORD dw_size);

	//-----------------------------------------------------------------------
	// 底层包相关
	//-----------------------------------------------------------------------
	LPBYTE			RecvMsg(DWORD& dw_size);
	VOID			ReturnMsg(LPBYTE p_message);
	VOID			SendMsg(LPBYTE p_message, DWORD dw_size);

	//-----------------------------------------------------------------------
	// 选人界面相关
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
	// 帐号下角色通用属性相关操作
	//-----------------------------------------------------------------------
	const DWORD		GetBagPsd()		const { return m_sAccountCommon.dw_bag_password_crc_; }
	const DWORD		GetSafeCode()	const { return m_sAccountCommon.s_safe_code_.dw_safe_code_crc; }

	VOID			SetBagPsd(DWORD dwNewPsdCrc);

	/************************************************************************
	** Handlers -- map thread
	*************************************************************************/

	//-----------------------------------------------------------------------
	// 进入游戏
	//-----------------------------------------------------------------------
	DWORD	HandleJoinGame(tag_net_message* pCmd);

	//-----------------------------------------------------------------------
	// 角色创建、删除、获取及选择
	//-----------------------------------------------------------------------
	DWORD	HandleRoleCreate(tag_net_message* pCmd);
	DWORD	HandleRoleEnum(tag_net_message* pCmd);
	DWORD	HandleRoleDelete(tag_net_message* pCmd);
	DWORD	HandleRoleGuardCancel(tag_net_message* pCmd);
	DWORD	HandleRoleSelect(tag_net_message* pCmd);
	DWORD	HandleRoleChangeName(tag_net_message* pCmd);
	DWORD	HandleRoleCheckName(tag_net_message* pCmd);

	//-----------------------------------------------------------------------
	// 领取账号奖励
	//-----------------------------------------------------------------------
	DWORD HandleReceiveAccountReward(tag_net_message* pCmd);
	DWORD HandleReceiveAccountRewardEx(tag_net_message* pCmd);
	DWORD HandleReceiveSerialReward(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 安全码处理
	//------------------------------------------------------------------------
	DWORD	HandleRoleSetSafeCode(tag_net_message* pCmd);
	DWORD	HandleRoleResetSafeCode(tag_net_message* pCmd);
	DWORD	HandleRoleCancelSafeCodeReset(tag_net_message* pCmd);
	
	DWORD	HandleResetVerCode(tag_net_message* pCmd);
	DWORD	HandleNeedVerCode(tag_net_message* pCmd);
	DWORD	HandleRoleCodeCheck(tag_net_message* pCmd);
	DWORD	HandleGotoVer(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// 人物属性获取
	//------------------------------------------------------------------------
	DWORD	HandleGetRoleInitAtt(tag_net_message* pCmd);
	DWORD	HandleGetRemoteUnitAtt(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 行走
	//------------------------------------------------------------------------
	DWORD	HandleRoleWalk(tag_net_message* pCmd);
	DWORD	HandleRoleStopWalk(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 装备相关
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
	// 角色外观显示设置
	//------------------------------------------------------------------------
	DWORD	HandleRoleSetFashion(tag_net_message* pCmd);
	DWORD	HandleRoleSetDisplay(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 物品操作相关
	//------------------------------------------------------------------------
	DWORD	HandleRoleChangeItemPos(tag_net_message* pCmd);
	DWORD	HandleRoleChangeItemPosEx(tag_net_message* pCmd);
	DWORD	HandleRoleReorderItem(tag_net_message* pCmd);
	DWORD	HandleRoleReorderItemEx(tag_net_message* pCmd);
	DWORD	HandleRoleStackItem(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// 玩家间交易相关
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
	// 商店相关
	//------------------------------------------------------------------------
	DWORD	HandleRoleGetShopItems(tag_net_message* pCmd);
	DWORD	HandleRoleGetShopEquips(tag_net_message* pCmd);
	DWORD	HandleRoleBuyShopItem(tag_net_message* pCmd);
	DWORD	HandleRoleBuyShopEquip(tag_net_message* pCmd);
	DWORD	HandleRoleSellToShop(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 摆摊相关
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
	// 驿站&乾坤石
	//------------------------------------------------------------------------
	DWORD	HandleRoleDak(tag_net_message* pCmd);
	DWORD	HandleRoleInstanceSaodang(tag_net_message* pCmd);
	DWORD	HandleRoleSaodangOver(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// 使用磨石
	//------------------------------------------------------------------------
	DWORD	HandleRoleAbrase(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 角色仓库
	//------------------------------------------------------------------------
	//DWORD	HandleRoleWareOpen(tag_net_message* pCmd);
	//DWORD	HandleRoleWareExtend(tag_net_message* pCmd);
	DWORD	HandleRoleBagExtand(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 角色仓库中存取金钱&元宝
	//------------------------------------------------------------------------
	DWORD	HandleRoleSaveSilver(tag_net_message* pCmd);
	DWORD	HandleRoleGetSilver(tag_net_message* pCmd);
	//DWORD	HandleRoleSaveYuanBao(tag_net_message* pCmd);
	DWORD	HandleRoleGetYuanBao(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 行囊加密相关
	//------------------------------------------------------------------------
	DWORD	HandleRoleSetBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleUnsetBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleCheckBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleResetBagPsd(tag_net_message* pCmd);
	DWORD	HandleRoleOpenBagPsd(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 测试
	//------------------------------------------------------------------------
	DWORD	HandleRoleEnterWorld(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 聊天
	//------------------------------------------------------------------------
	DWORD   HandleRoleChat(tag_net_message* pCmd);
	DWORD   HandleRoleGetID(tag_net_message* pCmd);
	DWORD	HandleRoleGetNameByNameID(tag_net_message* pCmd);
	DWORD	HandleRoleGetSomeName(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 装备展示
	//------------------------------------------------------------------------
	DWORD   HandleRoleShowEquip(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// 物品展示
	//------------------------------------------------------------------------
	DWORD   HandleRoleShowItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 读取留言
	//------------------------------------------------------------------------
	DWORD   HandleRoleLoadLeftMsg(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 地图事件
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
	// 属性点相关
	//-------------------------------------------------------------------------
	DWORD	HandleRoleBidAttPoint(tag_net_message* pCmd);
	DWORD	HandleRoleClearAttPoint(tag_net_message* pCmd);

	//-------------------------------------------------------------------------
	// 天资技能相关
	//------------------------------------------------------------------------
	DWORD	HandleRoleLearnSkill(tag_net_message* pCmd);
	DWORD	HandleRoleLevelUpSkill(tag_net_message* pCmd);
	DWORD	HandleRoleForgetSkill(tag_net_message* pCmd);
	DWORD	HandleRoleClearTalent(tag_net_message* pCmd);
	
	DWORD	HandleRoleEquipLearnSkill(tag_net_message* pCmd);
	DWORD	HandleRoleEquipLevelUpSkill(tag_net_message* pCmd);
	//------------------------------------------------------------------------
	// 战斗系统
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
	// PK系统
	//------------------------------------------------------------------------
	DWORD   HandleRoleChangePKState(tag_net_message* pCmd);
	DWORD	HandleRoleUseKillBadage( tag_net_message* pCmd );

	//------------------------------------------------------------------------
	// 复活
	//------------------------------------------------------------------------
	DWORD	HandleRoleBindRebornMap(tag_net_message* pCmd);
	DWORD	HandleRoleRevive(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 通用函数 -- 需在地图线程上层处理的消息
	//------------------------------------------------------------------------
	DWORD   HandleRoleMsg2World(tag_net_message* pCmd) { return RET_TRANS; }

	//------------------------------------------------------------------------
	// 通用命令 -- GM
	//------------------------------------------------------------------------
	DWORD   HandleGMCommand(tag_net_message* pCmd);
	
	//------------------------------------------------------------------------
	// 任务相关
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
	// 排行榜
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
	// 物品强化
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
	// 生产合成物品
	//------------------------------------------------------------------------
	DWORD	HandleRoleProduceItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 点化, 装备分解
	//------------------------------------------------------------------------
	DWORD	HandleRoleDeCompose(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 使用物品
	//------------------------------------------------------------------------
	DWORD	HandleRoleUseItem(tag_net_message* pCmd);
	DWORD	HandleRoleInterruptUseItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 拾取物品
	//------------------------------------------------------------------------
	DWORD	HandleRolePickUpItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 扔掉物品
	//------------------------------------------------------------------------
	DWORD	HandleRolePutDownItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 好友相关
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
	// 小队相关
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
	// 双修相关 gx add 2013.6.27
	//------------------------------------------------------------------------
	DWORD	HandleInviteRoleComPractice(tag_net_message* pCmd);//邀请玩家进行双修
	DWORD	HandleInviteRoleCompracticeReply(tag_net_message* pCmd);
	DWORD   HandleRoleCancelCompractice(tag_net_message* pCmd);//玩家取消双修
	//------------------------------------------------------------------------
	// 结婚相关 gx add 2013.7.3
	//------------------------------------------------------------------------
	DWORD	HandleMalePropose(tag_net_message* pCmd);//男性玩家向女性玩家求婚
	DWORD	HandleProposeFemaleReply(tag_net_message* pCmd);
	DWORD   HandleRoleDivorce(tag_net_message* pCmd);//离婚
	DWORD	HandleGetQbjjReward(tag_net_message* pCmd);//结婚双方领取情比金坚奖
	//------------------------------------------------------------------------
	// 称号相关
	//------------------------------------------------------------------------
	DWORD HandleRoleActiveTitle(tag_net_message* pCmd);
	DWORD HandleRoleGetTitles(tag_net_message* pCmd);
	DWORD HandleRoleTitleBuy(tag_net_message* pCmd);
	DWORD HandleRoleTitleReturn(tag_net_message* pCmd);
	DWORD HandleRoleShowActiveTitle(tag_net_message* pCmd);//gx add 2013.10.31
	//------------------------------------------------------------------------
	// 名帖相关
	//------------------------------------------------------------------------
	DWORD HandleRoleGetVCard(tag_net_message* pCmd);
	DWORD HandleRoleSetVCard(tag_net_message* pCmd);
	DWORD HandleGetHeadPicUrl(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 防沉迷相关
	//------------------------------------------------------------------------
	DWORD HandleGetFatigueInfo(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 名人堂相关
	//------------------------------------------------------------------------
	DWORD HandleGetFameHallRoles(tag_net_message* pCmd);
	DWORD HandleGetReputeTop(tag_net_message* pCmd);
	DWORD HandleGetActClanTreasure(tag_net_message* pCmd);
	DWORD HandleActiveTreasure(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 声望相关
	//------------------------------------------------------------------------
	DWORD HandleGetRoleClanData(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 反外挂相关
	//------------------------------------------------------------------------
	DWORD HandleGameGuarderMsg(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 宠物相关
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
	// 1v1竞技场
	//------------------------------------------------------------------------
	DWORD	Handle1v1Apply(tag_net_message* pCmd);
	DWORD	Handle1v1LeaveQueue(tag_net_message* pCmd);
	DWORD	HandleGet1v1ScoreAward(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 约战竞技场
	//------------------------------------------------------------------------
	DWORD	HandleReservationApply(tag_net_message* pCmd);
	DWORD	HandleGetReservationInfo(tag_net_message* pCmd);
	DWORD	HandleReservationResult(tag_net_message* pCmd);


	//------------------------------------------------------------------------
	// 百宝袋相关
	//------------------------------------------------------------------------
	DWORD HandleInitBaiBaoRecord(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// VIP摊位相关
	//------------------------------------------------------------------------
	DWORD HandleGetAllVIPStallInfo(tag_net_message* pCmd);
	DWORD HandleUpdateVIPStallInfo(tag_net_message* pCmd);
	DWORD HandleApplyVIPStall(tag_net_message* pCmd);
	DWORD HandleSpecVIPStallGet(tag_net_message* pCmd);
	DWORD HandleBuyVIPStallGoods(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// NPC相关
	//------------------------------------------------------------------------
	DWORD HandleTalkToNPC(tag_net_message* pCmd);


	//------------------------------------------------------------------------
	// 个性动作
	//------------------------------------------------------------------------
	DWORD	HandleRoleStyleAction(tag_net_message* pCmd);
	DWORD	HandleRoleDuetMotionInvite(tag_net_message* pCmd);
	DWORD	HandleRoleDuetMotionOnInvite(tag_net_message* pCmd);
	DWORD	HandleRoleDuetMotionStart(tag_net_message* pCmd);

	/************************************************************************
	** Handlers -- upper all map thread
	*************************************************************************/

	//------------------------------------------------------------------------
	// 商城相关
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
	// 元宝交易相关
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
	// 帮派相关
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
	// 开启宝箱相关
	//------------------------------------------------------------------------
	DWORD HandleOpenTreasureChest(tag_net_message* pCmd);
	DWORD HandleStopTreasureChest(tag_net_message* pCmd);
	DWORD HandleAgainTreasureChest(tag_net_message* pCmd);
	DWORD HandleChestGetItem(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 返回角色选择
	//------------------------------------------------------------------------
	DWORD HandleReturnRoleSelect(tag_net_message* pCmd);

	// 客户端对话框发给服务的缺省消息
	DWORD HandleDlgDefaultMsg(tag_net_message* pCmd);
	// 客户端触发服务器脚本的缺省消息
	DWORD HandleDefaultRequest(tag_net_message* pCmd);

	// 获得其他玩家的装备信息
	DWORD HandleGetSomeoneEquip(tag_net_message* pCmd);

	//------------------------------------------------------------------------
	// 邮件相关相关
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
	// 坐骑 
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
	// 师徒
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
	// 开始挂机
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
	// 帮助数据
	//--------------------------------------------------------------------
	DWORD HandleModRoleHelp(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 对话数据
	//--------------------------------------------------------------------
	DWORD HandleModRoleTalk(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 快捷键数据
	//--------------------------------------------------------------------
	DWORD HandlekeyInfo(tag_net_message* pCmd);
	DWORD HandleModKeyInfo(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 活跃度数据
	//--------------------------------------------------------------------
	DWORD HandleGetActiveInfo(tag_net_message* pCmd);
	DWORD HandleRoleActiveReceive(tag_net_message* pCmd);
	DWORD HandleRoleActiveDone(tag_net_message* pCmd);
	DWORD HandleGetGuildActiveInfo(tag_net_message* pCmd);
	DWORD HandleGuildActiveReceive(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 每日活动一键传送 gx add 2013.12.18
	//--------------------------------------------------------------------
	DWORD HandleDailyActTransmit(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 新手在线奖励
	//--------------------------------------------------------------------
	DWORD HandleStartNewRoleGift(tag_net_message* pCmd);
	DWORD HandleGetNewRoleGift(tag_net_message* pCmd);
	DWORD HandleBeginRoleGiftTime(tag_net_message* pCmd);
	
	// 延迟
	DWORD HandleGetDelay(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	//开始拍卖
	//--------------------------------------------------------------------
	DWORD handle_begin_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//取消拍卖
	//--------------------------------------------------------------------
	DWORD handle_cancel_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//开始竞拍
	//--------------------------------------------------------------------
	DWORD handle_jingpai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//一口价购买
	//--------------------------------------------------------------------
	DWORD handle_chaw_buy(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//拍卖查询
	//--------------------------------------------------------------------
	DWORD handle_paimai_query(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//拍卖翻页
	//--------------------------------------------------------------------
	DWORD handle_paimai_change_page(tag_net_message* p_cmd);

	//--------------------------------------------------------------------
	//开始钱庄拍卖
	//--------------------------------------------------------------------
	DWORD handle_begin_bank_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//取消钱庄拍卖
	//--------------------------------------------------------------------
	DWORD handle_cancel_role_bank_paimai(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//钱庄竞拍
	//--------------------------------------------------------------------
	DWORD handle_begin_bank_jing(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//钱庄一口价购买
	//--------------------------------------------------------------------
	DWORD handle_bank_chaw_buy(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//查询钱庄信息
	//--------------------------------------------------------------------
	DWORD handle_query_bank(tag_net_message* p_cmd);
	//--------------------------------------------------------------------
	//翻页
	//--------------------------------------------------------------------
	DWORD handle_bank_change_page(tag_net_message* p_cmd);

	//--------------------------------------------------------------------
	//元宝兑换
	//--------------------------------------------------------------------
	DWORD handle_yuanbao_exchange(tag_net_message* p_cmd);
	DWORD handle_get_yuanbao_exchange_num(tag_net_message* p_cmd);
	DWORD handle_get_bank_radio(tag_net_message* p_cmd);

	//--------------------------------------------------------------------
	// 钓鱼
	//--------------------------------------------------------------------
	DWORD HandleStartFishing(tag_net_message* pCmd);
	DWORD HandleStopFishing(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 搬运
	//--------------------------------------------------------------------
	DWORD HandleStopCarrySomething(tag_net_message* pCmd);

	//--------------------------------------------------------------------
	// 自动打怪
	//--------------------------------------------------------------------
	DWORD HandleAutoKillStart(tag_net_message* p_cmd);
	DWORD HandleAutoKillEnd(tag_net_message* p_cmd);

	DWORD HandleConsumeReward(tag_net_message* p_cmd);


	// 抽奖
	DWORD HandleGetLottery(tag_net_message* pCmd);


	//--------------------------------------------------------------------
	// 签到
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
	static PlayerNetCmdMgr	m_PlayerNetMgr;						// 对应的客户端消息管理器
	static GMCommandMgr		m_GMCommandMgr;						// GM命令管理器

	DWORD					m_dwAccountID;						// session id，对应的是帐号ID
	DWORD					m_dwInternalIndex;					// 底层的网络ID
	INT						m_nMsgNum;							// 网络底层未处理的消息数量

	bool					m_bRoleLoading;						// 选择人物时正在等待数据库返回
	bool					m_bRoleEnuming;						// 进入游戏时等待服务器选人将角色信息读取出来
	bool					m_bRoleEnumDone;					// 读取角色信息完毕
	bool					m_bRoleEnumSuccess;					// 读取角色信息是否成功
	bool					m_bRoleCreating;					// 等待创建角色
	bool					m_bRoleDeleting;					// 删除人物时等待数据库返回
	bool					m_bRoleChangeNameing;				// 人物改名时等待数据库返回
	bool					m_bRoleDelGuardCanceling;			// 恢复删除角色的保护时间
	bool					m_bRoleVerifying;					// 等待验证码数据
	BYTE					m_byPrivilege;						// gm权限
	INT8					m_n8RoleNum;						// 已经创建的角色个数

	bool					m_bRoleInWorld;						// 在游戏世界中

	//DWORD					m_dwRoleID[MAX_ROLENUM_ONEACCOUNT];	// 帐号下所有角色ID
	StaticArraySP<MAX_ROLENUM_ONEACCOUNT, PlayerSessionCommonData> m_SessionCommonData;
	

	char					m_szAccount[X_SHORT_NAME];			// 玩家账号

	volatile BOOL			m_bConnectionLost;					// 连接是否已经断开
	volatile BOOL			m_bKicked;							// 是否已经被踢掉

	s_account_common		m_sAccountCommon;					// 账号通用信息

	Role*					m_pRole;							// 对应的角色对象

	DWORD					m_dwIP;								// 客户端IP

	FatigueGuarder			m_FatigueGarder;					// 防沉迷

	Verification			m_Verification;						// 在线验证码

	tagDWORDTime				m_dwPreLoginTime;		//上次登录时间
	DWORD						m_dwPreLoginIP;			//上次登录ip

	BYTE					by_delay_send;				// 减缓发包
};


