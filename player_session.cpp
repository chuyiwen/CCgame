
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//玩家连线

#include "StdAfx.h"
#include "player_session.h"
#include "world_session.h"
#include "player_net_cmd_mgr.h"
#include "role.h"
#include "mutex.h"
#include "map.h"
#include "world_net_cmd_mgr.h"
#include "map_creator.h"
#include "vip_netbar.h"
#include "paimai_manager.h"
#include "bank_manager.h"
#include "lottery.h"

#include "../../common/WorldDefine/chat_protocol.h"
#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/all_msg_cmd.h"
#include "../../common/WorldDefine/mail_protocol.h"
#include "../../common/WorldDefine/ride_protocol.h"
#include "../../common/WorldDefine/master_prentice_protocol.h"
#include "../../common/WorldDefine/rank_protocol.h"
#include "../../common/WorldDefine/paimai_protocol.h"
#include "../../common/WorldDefine/bank_protocol.h"
#include "../../common/WorldDefine/guerdon_quest_protocol.h"
#include "../../common/WorldDefine/duel_protocol.h"
#include "../../common/WorldDefine/pet_sns_protocol.h"
#include "../../common/WorldDefine/fishing_protocol.h"
#include "../../common/WorldDefine/auto_kill_protocol.h"
#include "../../common/WorldDefine/verification_protocol.h"
#include "../../common/WorldDefine/TeamRandShareProtocol.h"
#include "../../common/WorldDefine/RoleCarryDefine.h"
#include "../../common/WorldDefine/gp_mall_protocol.h"
#include "../../common/WorldDefine/LianHun_define.h"
#include "../../common/WorldDefine/role_god_level.h"
#include "../../common/WorldDefine/battle_ground_protocol.h"

//需要广播字符串长度最大值
#define MAX_BROADCAST_MSG_LEN 200

// 定义两个宏，分别对应普通消息注册和世界消息注册
#define M_REGISTER_PLAYER_RECV_CMD(name, handler, desc)	m_PlayerNetMgr.RegisterRecvProc(#name, handler, desc, sizeof(tag##name))
#define M_REGISTER_WORLD_RECV_CMD(name, handler, desc)	RegisterWorldMsg(#name, handler, desc, sizeof(tag##name))

#define REGISTER_ROLE_RECV_COMMAND(name, handler, desc) m_PlayerNetMgr.RegisterRecvProc(#name, handler, desc, sizeof(name))
#define REGISTER_WORLD_RECV_COMMAND(name, handler, desc) RegisterWorldMsg(#name, handler, desc, sizeof(name))

PlayerNetCmdMgr PlayerSession::m_PlayerNetMgr;
GMCommandMgr	PlayerSession::m_GMCommandMgr;

#include "../common/ServerDefine/role_data_server_define.h"
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
PlayerSession::PlayerSession(DWORD dwSessionID, DWORD dwInternalIndex, DWORD dw_ip, BYTE byPrivilege, BOOL bGuard, DWORD dwAccOLSec, LPCSTR sz_account,tagDWORDTime dwPreLoginTime, DWORD dwPreLoginIP)
: m_dwAccountID(dwSessionID), m_dwInternalIndex(dwInternalIndex), m_dwIP(dw_ip), m_byPrivilege(byPrivilege), m_nMsgNum(0), m_FatigueGarder(this, bGuard, dwAccOLSec),m_Verification(this),
m_dwPreLoginTime(dwPreLoginTime),m_dwPreLoginIP(dwPreLoginIP)
{
	m_bRoleEnuming = false;
	m_bRoleEnumDone = false;
	m_bRoleEnumSuccess = false;

	m_bRoleLoading = false;
	m_bRoleDeleting = false;
	m_bRoleCreating = false;
	m_bRoleChangeNameing = false;
	m_bRoleDelGuardCanceling = false;
	m_bRoleVerifying = false;
	m_bRoleInWorld = false;
	m_bConnectionLost = false;
	m_bKicked = false;

	by_delay_send = 0;

	strncpy_s(m_szAccount, X_SHORT_NAME, sz_account, X_SHORT_NAME);

	m_pRole = NULL;
	g_VipNetBarMgr.PlayerLogin(m_dwAccountID, m_dwIP);
	m_SessionCommonData.Initialize( );
}


//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
PlayerSession::~PlayerSession()
{

}

//-----------------------------------------------------------------------
// 需要在所有地图线程上层处理的消息注册
//-----------------------------------------------------------------------
VOID PlayerSession::RegisterWorldMsg(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size)
{
	m_PlayerNetMgr.RegisterRecvProc(szCmd, &HandleRoleMsg2World, _T("need proc upper map thread"), dw_size);
	WorldNetCmdMgr::GetInstance()->RegisterRecvProc(szCmd, fp, szDesc, dw_size);
}

//------------------------------------------------------------------------------
// 注册所有客户端的网络命令
//------------------------------------------------------------------------------
VOID PlayerSession::RegisterAllPlayerCmd()
{
	// 进入游戏
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_join_game,					&HandleJoinGame,				_T("Join Game"));

	// 选人界面
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_enum_role,					&HandleRoleEnum,				_T("Enum Role"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_create_role,				&HandleRoleCreate,				_T("Create Role"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_delete_role,				&HandleRoleDelete,				_T("Delete Role"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_select_role,				&HandleRoleSelect,				_T("Select Role"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_check_name,				&HandleRoleCheckName,			_T("Check Name Role"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_change_role_name,		&HandleRoleChangeName,			_T("change Role name"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_delete_role_guard_Cancel,&HandleRoleGuardCancel,			_T("restore role"));

	// 账号相关
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_receive_account_reward,	&HandleReceiveAccountReward,	_T("receive account reward"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_receive_account_reward_ex, &HandleReceiveAccountRewardEx,	_T("receive account reward ex"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_receive_serial_reward, &HandleReceiveSerialReward,		_T("receive serial reward"));

	// 安全码
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_safe_code,				&HandleRoleSetSafeCode,			_T("set safe code"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_reset_safe_code,			&HandleRoleResetSafeCode,		_T("reset safe code"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_cancel_safe_code_reset,	&HandleRoleCancelSafeCodeReset,	_T("cancel safe code reset"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_code_check_ok,			&HandleRoleCodeCheck,			_T("Code Check"));
	
	// 验证码
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_reset_verification_code,	&HandleResetVerCode,				_T("reset verification code"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_need_verification_return,&HandleNeedVerCode,				_T("need verification code"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_goto_verification,		&HandleGotoVer,					_T("goto verification code"));
	// 人物属性
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_role_init_state,			&HandleGetRoleInitAtt,			_T("Get Role Init State"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_remote_role_state,		&HandleGetRemoteUnitAtt,		_T("Get Remote Role State"));

	// 行走
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_mouse_walk,				&HandleRoleWalk,				_T("Mouse Walk"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_keyboar_walk,			&HandleRoleWalk,				_T("Keyboard Walk"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stop_walk,				&HandleRoleStopWalk,			_T("Stop Walk"));

	// 装备相关
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip,					&HandleRoleEquip,				_T("Equip"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_unequip,					&HandleRoleUnequip,				_T("Unequip"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_swap_weapon,				&HandleRoleSwapWeapon,			_T("Swap Weapon"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_identify_equip,			&HandleRoleIdentifyEquip,		_T("Identify Weapon"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_repair,			&HandleRoleEquipRepair,			_T("Equip Repair"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_bind,				&HandleRoleEquipBind,			_T("Equip bind"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_unbind,			&HandleRoleEquipUnBind,			_T("Equip unbind"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_destroy,			&HandleEquipDestroy,		_T("equip destroy"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_reatt,				&HandleRoleEquipReatt,			_T("Equip reatt"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_kaiguang,			&HandleRoleEquipKaiguang,		_T("Equip kaiguang"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_xili,				&HandleRoleEquipXili,					_T("Equip xili"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_xili_change,		&HandleRoleEquipXiliChange,				_T("Equip xili change"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_get_wuhuen,		&HandleRoleEquipGetWuhuen,				_T("Equip get wuhuen"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_ronglian,			&HandleRoleEquipRongLian,				_T("Equip ronglian"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_fumo,				&HandleShiPinFumo,					_T("Equip fumo"));	
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_type_change,		&HandleEquipChange,					_T("Equip change"));	
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_luck_you,			&HandleEquipLuckYou,					_T("use luck you"));		
	
	// 角色外观显示设置
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_fashion,					&HandleRoleSetFashion,			_T("set fashion"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_set_display,		&HandleRoleSetDisplay,			_T("set display"));

	// 物品
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_item_position_change,	&HandleRoleChangeItemPos,		_T("Change Item Position"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_item_position_change_extend,	&HandleRoleChangeItemPosEx,		_T("Change Item Position"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_item_reorder,				&HandleRoleReorderItem,			_T("container item reorder"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_item_reorder_extend,			&HandleRoleReorderItemEx,		_T("container item reorder"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stack_item,					&HandleRoleStackItem,		_T("stack item"));

	// 玩家间交易
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_exchange_request,			&HandleRoleExchangeReq,			_T("ExchangeReq"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_exchange_request_result,		&HandleRoleExchangeReqRes,		_T("ExchangeReqRes"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_add_exchange,				&HandleRoleExchangeAdd,			_T("ExchangeAdd"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_cancel_exchange_item,		&HandleRoleExchangeDec,			_T("ExchangeDec"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_exchange_money,			&HandleRoleExchangeMoney,		_T("ExchangeMoney"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_exchange_item_lock,		&HandleRoleExchangeLock,		_T("ExchangeLock"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_exchange_cancel,			&HandleRoleExchangeCancel,		_T("ExchangeCancel"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_exchange_verify,			&HandleRoleExchangeVerify,		_T("ExchangeVerify"));

	// 商店
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_shop_item,			&HandleRoleGetShopItems,		_T("Get Goods(Item) List"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_shop_equip,			&HandleRoleGetShopEquips,		_T("Get Goods(Equip) List"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_buy_shop_item,			&HandleRoleBuyShopItem,			_T("Buy Item"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_buy_shop_equip,			&HandleRoleBuyShopEquip,		_T("Buy Equip"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_sell_to_shop,			&HandleRoleSellToShop,			_T("Sell To Shop"));

	// 摆摊
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_start,				&HandleRoleStallStart,			_T("start stall"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_set_goods,			&HandleRoleStallSetGoods,		_T("set stall goods"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_unset_goods,		&HandleRoleStallUnsetGoods,		_T("unset stall goods"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_set_title,			&HandleRoleStallSetTitle,		_T("set stall title"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_set_advertisement,	&HandleRoleStallSetAdText,		_T("set stall ad"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_get_advertisement,	&HandleRoleStallGetAd,			_T("get stall ad"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_set_finish,		&HandleRoleStallSetFinish,		_T("set stall finish"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_close,				&HandleRoleStallClose,			_T("close stall"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_get,				&HandleRoleStallGet,			_T("get all stall goods"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_get_title,			&HandleRoleStallGetTitle,		_T("get stall title"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_buy,				&HandleRoleStallBuy,			_T("buy stall goods"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_get_special,		&HandleRoleStallGetSpec,		_T("get stall spec goods"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_chat,				&HandleStallChat,				_T("stall chat"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_stall_history_chat,		&HandleStallHistoryChat,			_T("get stall history chat"));

	// 驿站&乾坤石
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_posthouse,				&HandleRoleDak,					_T("Move To Another Map"));
	// 副本扫荡
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_instance_saodang,		&HandleRoleInstanceSaodang,		_T("saodang instance"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_saodang_over,			&HandleRoleSaodangOver,			_T("saodang over"));
	
	// 磨石
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_abrase_stone,					&HandleRoleAbrase,				_T("abrase weapon"));

	// 角色仓库
	//m_PlayerNetMgr.Register("NC_WareOpen",				&HandleRoleWareOpen,			_T("open role ware"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_ware_extend,				&HandleRoleWareExtend,			_T("extend role ware space"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_bag_extend,				&HandleRoleBagExtand,			_T("extend role bag space"));

	// 角色仓库中存取金钱&元宝
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_save_silver,				&HandleRoleSaveSilver,			_T("save silver to ware"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_silver,				&HandleRoleGetSilver,			_T("get silver from ware to bag"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_yuan_bao,				&HandleRoleGetYuanBao,			_T("get yuanbao from ware to bag"));

	// 行囊加密相关
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_set_bag_password,				&HandleRoleSetBagPsd,			_T("set bag password"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_unset_bag_password,				&HandleRoleUnsetBagPsd,			_T("cancel bag psd"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_old_bag_password,				&HandleRoleCheckBagPsd,			_T("check old bag psd"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_reset_bag_password,				&HandleRoleResetBagPsd,			_T("modify bag psd"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_open_bag_password,				&HandleRoleOpenBagPsd,			_T("open bag need"));

	// 地图事件
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_map_trigger,			&HandleRoleMapTrigger,			_T("Map Trigger"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_enter_battle_instance,		&HandleEnterBattle,				_T("enter battlent"));
	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_instance_notify,			&HandleRoleInstanceNotify,		_T("Notify Teamate Enter Instance"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_instance_agree,			&HandleRoleInstanceAgree,		_T("Agree Enter Instance"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_leave_instance,			&HandleRoleLeaveInstance,		_T("Role Leave Instance"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_enter_pvp_instance,		&HandleEnterPVPInstance,		_T("Role Enter PVP Instance"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_leave_pvp_instance,		&HandleLeavePVPInstance,		_T("Role Leave PVP Instance"));

	// 属性点相关
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_att_point,			&HandleRoleBidAttPoint,			_T("Role Bid Att"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_clear_att_point,		&HandleRoleClearAttPoint,		_T("Role Clear Att"));

	// 天资技能相关
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_learn_skill,				&HandleRoleLearnSkill,			_T("Role Learn Skill"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_level_up_skill,				&HandleRoleLevelUpSkill,		_T("Role's Skill Level Up"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_forget_skill,				&HandleRoleForgetSkill,			_T("Role Forget Skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_clear_talent,				&HandleRoleClearTalent,			_T("Role Clear His/Her Skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_learn_skill,			&HandleRoleEquipLearnSkill,		_T("Equip Learn Skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_equip_level_up_skill,		&HandleRoleEquipLevelUpSkill,	_T("Equip LevelUp Skill"));

	// 战斗
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_enter_combat,				&HandleRoleEnterCombat,			_T("Role Enter Combat"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_leave_combat,				&HandleRoleLeaveCombat,			_T("Role Level Combat"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_skill,					&HandleRoleSkill,				_T("Role Skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_skill_interrupt,			&HandleRoleInterruptSkill,		_T("Role Interrupt Skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_target_change,			&HandleTargetChange,			_T("Role Target Change"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_AskForDuel,			&HandleDuelAskFor,			_T("NET_SIC_AskForDuel"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_AskForDuelResponse,	&HandleDuelResponse,		_T("NET_SIC_AskForDuelResponse"));

	// 在线挂机
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_start_hang,				&HandleStartHang,				_T("Role Start Hang"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_cancel_hang,				&HandleCancelHang,				_T("Role Cancel Hang"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_set_leave_line_hang,		&HandleSetLeaveLineHang,		_T("Role Set Leave Line Hang"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_leave_exp,				&HandleGetLeaveExp,				_T("Role Get Leave Exp"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_pickup_leave_exp,			&HandlePickLeaveExp,			_T("Role Pickup Leave Exp"));
	

	// PK
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_change_pk_value,			&HandleRoleChangePKState,		_T("Role Change PK State"));

	// Buff
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_cancel_buffer,				&HandleRoleCancelBuff,			_T("Role Cancel Buff"));

	// 个性动作
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_style_action,			&HandleRoleStyleAction,			_T("Role Style Action"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_double_motion_invite,			&HandleRoleDuetMotionInvite,	_T("Role Duet Action"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_double_motion_on_invite,		&HandleRoleDuetMotionOnInvite,	_T("Role Duet OnInvite"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_double_motion_start,			&HandleRoleDuetMotionStart,		_T("Role Duet Start"));

	// 复活
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_bind_reborn_map,			&HandleRoleBindRebornMap,       _T("Bind Reborn Map"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_revive,				&HandleRoleRevive,				_T("Revive"));

	// 聊天
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_char,					&HandleRoleChat,				_T("Chat"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_get_id,				&HandleRoleGetID,				_T("RoleGetID"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_name_by_nameid,			&HandleRoleGetNameByNameID,		_T("RoleGetNamebyNameID"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_set_some_name,			&HandleRoleGetSomeName,			_T("RoleGetSomeName"));

	// 装备展示
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_show_equip,			&HandleRoleShowEquip,			_T("NET_SIC_role_show_equip"));
	// 物品展示
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_show_item,				&HandleRoleShowItem,			_T("NET_SIC_role_show_item"));
	// 读取留言
	
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_leave_word,				&HandleRoleLoadLeftMsg,			_T("NET_SIC_leave_word"));

	// 任务
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_npc_accept_quest,			&HandleRoleNPCAcceptQuest,		_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_trigger_accept_quest,		&HandleRoleTriggerAcceptQuest,	_T("Quest"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_complete_quest,				&HandleRoleCompleteQuest,		_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_delete_quest,				&HandleRoleDeleteQuest,			_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_update_quest_npc_talk,		&HandleRoleUpdateQuestNPCTalk,	_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_set_quest_track,				&HandleTrackQuest,		_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_CircleQuestList,				&HandleCircleQuestList,		_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_RefreshCircleQuest,			&HandleRefreshCircleQuest,		_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_BuyRefreshCircleQuest,		&HandleBuyRefreshCircleQuest,		_T("Quest"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_BuyCircleQuestPerdayNumber,	&HandleBuyCircleQuestPerdayNumber,		_T("Quest"));

	//拾取物品
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_pickup_item,			&HandleRolePickUpItem,			_T("PickUp"));
	//扔掉物品
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_putdown_item,			&HandleRolePutDownItem,			_T("PutDown"));

	// 装备强化
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_consolidate_posy,			&HandleRolePosyEquip,			_T("Posy Equip"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_consolidate_engrave,		&HandleRoleEngraveEquip,		_T("Engrave Equip"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_inlay,					&HandleRoleInlayEquip,			_T("Inlay Equip"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_produce,					&HandleRoleProduceItem,			_T("Produce Item"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_decomposition,			&HandleRoleDeCompose,			_T("Decompose Item"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_chisel,					&HandleRoleChisel,				_T("Chisel Equip"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_unbeset,					&HandleRoleUnbeset,				_T("Equip Unbeset"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_damo,					&HandleRoleDamo,				_T("damo baoshi"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_shengxing,				&HandleRoleEquipShengXing,		_T("Equip ShengXing"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_practice_begin,			&HandleRolePracticeBegin,		_T("Practice Begin"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_parctice_end,				&HandleRolePracticeEnd,			_T("Practice End"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_fusion,					&HandleRoleFusion,				_T("Fusion"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_weapon_clear_talent,		&HandleRoleWeaponClearTalent,	_T("WeaponClearTalent"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_leave_parcitice,			&HandleRoleLeavePracitice,		_T("RoleLeavePracitice"));

	// 使用物品
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_item,					&HandleRoleUseItem,             _T("Use Item"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_item_interrupt,			&HandleRoleInterruptUseItem,	_T("Interrupt Use Item"));

	// 好友相关
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_make_friend,			&HandleRoleMakeFriend,			_T("Make Friend"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_cancel_friend,			&HandleRoleCancelFriend,		_T("Cancel Friend"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_update_friend_group,			&HandleUpdateFriendGroup,		_T("Friend Group"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_move_to_black_list,			&HandleMoveBlackList,			_T("Move BlackList"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_delete_black_list,			&HandleDeleteBlackList,			_T("Delete BlackList"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_add_enemy,					&HandleAddEnemy,				_T("Move EnemyList"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_delete_enemy_list,			&HandleDeleteEnemyList,			_T("Delete EnemyList"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_enemy_position,			&HandleGetEnemyPos,				_T("get enemy pos"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_send_gift,					&HandleRoleSendGift,			_T("Send Gift"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_send_gift_reply,				&HandleRoleSendGiftReply,		_T("Send Gift Repley"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_get_signature_name,		&HandleGetRoleSignature,		_T("Get Role Signature"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_set_signature_name,		&HandleChangeSignature,			_T("Set Role Signature"));
	//双修相关 gx add 2013.6.28
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_invite_practice,		&HandleInviteRoleComPractice,			_T("Invite Role ComPractice"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_invite_practice_reply,		&HandleInviteRoleCompracticeReply,			_T("Role ComPractice Reply"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_cancel_practice,				&HandleRoleCancelCompractice,_T("Role Cancel ComPractice"));
	//end
	//结婚相关 gx add 2013.7.3
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_male_propose,		&HandleMalePropose,			_T("Male Role Propose"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_propose_reply,		&HandleProposeFemaleReply,			_T("Female Propose Reply"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_divorce,		&HandleRoleDivorce,			_T("role divorce"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_qbjj_reward,		&HandleGetQbjjReward,		_T("get qbjj reward"));
	//end
	// 小队相关
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_own_create_team,			&HandleOwnCreateTeam,			_T("own create team"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_dismiss_team,				&HandleDismissTeam,				_T("dismiss team"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_apply_join_team,			&HandleApplyJoinTeam,			_T("Apply Join Team"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_apply_join_team_reply,		&HandleApplyJoinTeamReply,		_T("Apply Join Team Reply"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_clean_apply,				&HandleCleanApply,				_T("Clean Apply"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_member_invite_join_team,		&HandleMemInviteJoinTeam,		_T("Member Invite Join Team"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_member_invite_join_team_replay,	&HandleMemInviteJoinTeamReply,	_T("Member Invite Join Team Reply"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_change_team_placard,		&HandleChangeTeamPlacard,		_T("Change Team Placard"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_map_team_info,			&HandleGetMapTeamInfo,			_T("Get Map Team Info"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_invite_join_team,			&HandleRoleJoinTeam,			_T("Invite Join Team"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_invite_reply,				&HandleRoleJoinTeamReply,		_T("Invite Join Team Reply"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_kick_member,				&HandleRoleKickMember,			_T("Kick Member"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_leave_team,				&HandleRoleLeaveTeam,			_T("Leave Team"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_set_pick_mode,				&HandleRoleSetPickMol,			_T("Set Pick Mode"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_change_leader,				&HandleRoleChangeLeader,		_T("Change Leader"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_update_friend_state,			&HandleUpdateFriOnline,			_T("Update Friend Online"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_set_assign_quality,		&HandleRoleSetAssignQuality,	_T("Set Assign Quality"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_team_lead_assign,			&HandleRoleLeaderAssign,		_T("Leader Assign"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_team_assign_sice,			&HandleRoleTeamAssignSice,		_T("Team Assign Sice"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_team_leader_set_share_circle_quest,	&HandleRoleTeamSetShareLeaderCircleQuest,		_T("set rand from leader"));

	// 称号相关
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_role_title,			&HandleRoleActiveTitle,			_T("Active a title"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_role_titles,			&HandleRoleGetTitles,			_T("Get all obtained titles"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_show_active_title,		&HandleRoleShowActiveTitle,		_T("Set show active title"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_title_buy,			&HandleRoleTitleBuy,			_T("Buy titles"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_title_return,		&HandleRoleTitleReturn,			_T("return titles"));
	// 名帖相关
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_role_card,				&HandleRoleGetVCard,			_T("Get VCard"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_role_head_picture,		&HandleGetHeadPicUrl,			_T("Get HeadPicUrl"));

	// 反外挂
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_game_sentinel,				&HandleGameGuarderMsg,			_T("Game Guarder"));

	// 声望
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_reputation,			&HandleGetRoleClanData,			_T("get role clan data"));

	// 名人堂
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_activation_gens_treasure,		&HandleGetActClanTreasure,		_T("get active treasure"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_celeb_role,			&HandleGetFameHallRoles,		_T("get famehall role top 50"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_reputation_top,				&HandleGetReputeTop,			_T("get reputation top 50"));

	// 百宝袋
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_init_baibao_record,			&HandleInitBaiBaoRecord,		_T("get baibao records"));
	
	// 防沉迷
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_enthrallment_info,			&HandleGetFatigueInfo,			_T("fatigue info"));

	// 宠物
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_pet_att,				&HandleGetPetAttr,				_T("get pet att"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_pet_skill,				&HandlePetSkill,				_T("pet skill cmd"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_pet_egg,				&HandleUsePetEgg,				_T("use pet egg"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_delete_pet,				&HandleDeletePet,				_T("delete pet"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_pet_display_info,			&HandleGetPetDispInfo,			_T("get pet disp"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_set_pet_state,				&HandleSetPetState,				_T("set pet state"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_rename,				&HandlePetReName,				_T("pet rename"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_use_special_pet_item,			&HandleUseSpePetItem,		_T("Exp Pet Pocket Size"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_pocket_size_change,			&HandlePocketSizeChange,	_T("pocket size change"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_equip,					&HandlePetEquip,				_T("pet equip"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_unequip,				&HandlePetUnEquip,				_T("pet unequip"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_equip_position_swap,			&HandlePetSwapEquipPos,			_T("pet swap equip pos"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_pet_pour_exp_money_need,	&HandleGetPetPourExpMoneyNeed,	_T("pet pour exp money need"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_pour_exp,				&HandlePetPourExp,				_T("pet pour exp"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_up_step,				&HandlePetUpStep,				_T("pet up step"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_enhance,				&HandlePetEnhance,				_T("pet enhance"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_food,					&HandlePetFood,					_T("pet food"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_learn_skill,			&HandlePetLearnSkill,			_T("pet learn skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_levelup_skill,		&HandlePetLevelUpSkill,			_T("pet level skill"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_forget_skill,		&HandlePetForgetSkill,			_T("pet forget skill"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_mount_invite,				&HandlePetInvite,				_T("invite sb mount"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_mount_on_invite,			&HandlePetOnInvite,				_T("on invited"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_set_lock,				&HandlePetSetLock,				_T("set pet lock"));
	//
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_reborn_pet,				&HandleRebornPet,				_T("reborn pet"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_buy_love_vaule,			&HandleBuyLoveValue,			_T("buy loveValue"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_color,				&HandlePetColor,				_T("pet color"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_change,				&HandlePetChange,				_T("pet change"));	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_add_point,			&HandlePetaddPoint,				_T("pet addpoint"));	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_fusion,				&HandlePetFusion,				_T("pet fusion"));	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_xiulian_size_change,	&HandlePetXiulianSizeChange,	_T("pet xiulian change"));	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_xiulian,				&HandlePetXiulian,				_T("pet xiulian"));	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_xiulian_return,		&HandlePetXiulianReturn,		_T("pet xiulian return"));	

	//// 玩家间宠物交易
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_request,			&HandleRolePetExchangeReq,		_T("PetExchangeReq"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_request_result,		&HandleRolePetExchangeReqRes,	_T("PetExchangeReqRes"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_add,			&HandleRolePetExchangeAdd,		_T("PetExchangeAdd"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_subtract,			&HandleRolePetExchangeDec,		_T("PetExchangeDec"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_money,			&HandleRolePetExchangeMoney,	_T("PetExchangeMoney"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_lock,			&HandleRolePetExchangeLock,		_T("PetExchangeLock"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_cancel,		&HandleRolePetExchangeCancel,	_T("PetExchangeCancel"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_pet_exchange_verify,		&HandleRolePetExchangeVerify,	_T("PetExchangeVerify"));

	// npc
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_npc_talk,					&HandleTalkToNPC,				_T("talk to npc"));

	// 跑商相关
	/*REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_chamber_goods_info,				&HandleGetCofCInfo,				_T("get CofC goods info"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_colse_chamber,				&HandleCloseCofC,				_T("close CofC"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_buy_chamber_goods,				&HandleBuyCofCGoods,			_T("buy goods from CofC"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_sell_chamber_goods,			&HandleSellCofCGoods,			_T("sell goods to CofC"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_commodity_info,			&HandleGetCommodityInfo,		_T("get commodity info"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_tael_info,				&HandleGetTaelInfo,				_T("get commerce beginning info"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_commerce_rank,			&HandleGetCommerceRank,			_T("get commerce rank info"));*/

	//	宝箱
	/*REGISTER_ROLE_RECV_COMMAND(NET_SIC_treasure_chest,			&HandleOpenTreasureChest,		_T("open treasure chest"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_stop_treasure_chest,		&HandleStopTreasureChest,		_T("stop roll item"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_repeat_treasure_chest,		&HandleAgainTreasureChest,		_T("roll item again"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_treasure_item,				&HandleChestGetItem,			_T("get chest item"));*/

	// 返回角色选择
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_return_role_select,			&HandleReturnRoleSelect,		_T("return role select"));
	// todo: add new register to here.
	//M_REGISTER_PLAYER_RECV_CMD( "",		&HandleRole,       _T(""));


	//骑乘
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_upgrade_ride,			&HandleUpgRide,							_T("upgrade ride"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_ride_inlay,				&HandleRideInlay,						_T("Inlay ride"));
	////REGISTER_ROLE_RECV_COMMAND(NET_SIC_remove_ride_inlay,		&HandleRemoveRideInlay,					_T("remove inlay "));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_begin_ride,				&HandleBeginRide,						_T("begin ride"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_cancel_ride,				&HandleCancelRide,						_T("cancel ride"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_tog_ride,				&HandletogRide,							_T("tog ride"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_raid_att,			&HandleGetRideAtt,						_T("get ride att"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_interrupt_begin_ride,	&HandleInterruptRide,					_T("cancel ride"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_Equip_ride,				&HandleEquipRide,						_T("NET_SIC_Equip_ride"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_UnEquip_ride,			&HandleUnEquipRide,						_T("NET_SIC_UnEquip_ride"));

	//帮助数据
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_modify_role_help,				&HandleModRoleHelp,						_T("mod role help"));

	// 对话数据
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_modify_role_talk,				&HandleModRoleTalk,						_T("mod role talk"));

	// 快捷键数据
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_key_info,					&HandlekeyInfo,						_T("key info"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_modify_key_info,				&HandleModKeyInfo,						_T("mod key info"));

	// 活跃度
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_active_info,				&HandleGetActiveInfo,				_T("get active info"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_role_active_receive,			&HandleRoleActiveReceive,			_T("active receive"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_active_done,					&HandleRoleActiveDone,			_T("active done"));
	// 每日活动一键传送 gx add 2013.12.18
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_daily_act_transmit,			&HandleDailyActTransmit,	_T("daily act transmit"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_guild_active_info,		&HandleGetGuildActiveInfo,				_T("get active info"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_guild_active_receive,		&HandleGuildActiveReceive,			_T("active receive"));


	// 新手在线奖励
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_begin_role_gift_time,				&HandleBeginRoleGiftTime,				_T("begin role gift"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_start_new_role_gift,		&HandleStartNewRoleGift,			_T("start new role gift"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_new_role_gift,			&HandleGetNewRoleGift,				_T("get new role gift"));

	// 钓鱼
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_Start_Fishing,			&HandleStartFishing,				_T("NET_SIC_Start_Fishing"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_Stop_Fishing,			&HandleStopFishing,				_T("NET_SIC_Stop_Fishing"));

	// 自动打怪
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_Auto_Kill_Start,			&HandleAutoKillStart,			_T("NET_SIC_Auto_Kill_Start"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_Auto_Kill_End,			&HandleAutoKillEnd,				_T("NET_SIC_Auto_Kill_End"));
	
	// 延迟
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_delay,				&HandleGetDelay,					_T("get delay"));

	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_StopCarrySomething,		&HandleStopCarrySomething, _T("stop carry something"));
	
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_start_hang_get_exp,		&HandleStartHangGetExp, _T("NET_SIC_start_hang_get_exp"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_stop_hang_get_exp,		&HandleStopHangGetExp, _T("NET_SIC_stop_hang_get_exp"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_consume_reward,		&HandleConsumeReward, _T("NET_SIC_get_consume_reward"));

	// 签到
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_sign_data,			&HandleGetSignData,		_T("get sign data"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_sign,					&HandleSign,			_T("sign"));
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_sign_reward,				&HandleSignReward,		_T("sign reward"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_sign_reward,			&HandleGetSignReward,	_T("get sign reward"));
	// 成神之路
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_god_level_up,			&HandleGodLevelUp,		_T("god level up"));

	// 购买灵气
	//REGISTER_ROLE_RECV_COMMAND(NET_SIC_buy_lingqi_vaule,		&HandleBuyLingqi,			_T("buy lingqi"));
	
	// 混炼
	/*REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_HunJing_data,		&HandleGetHuenJingData,		_T("get huenjing data"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_LianHun,					&HandleHuenLian,			_T("huenlian"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_huenjing_opteron,		&HandleHuenjingOpteron,		_T("HuenJingOpteron"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_huenjing_UpLevel,		&HandleHuenjingLevelUp,		_T("HuenJingLevelUp"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_huenjing_Inlay,			&HandleHuenjingInlay,		_T("HuenJingInlay"));*/
	
	// 奖励
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_reward_data,			&HandleGetRewardData,		_T("get reward data"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_receive_reward,			&HandleReceiveReward,		_T("receive reward"));
	
	// 开服7天
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_server_acitvity,			&HandleGetOpenActiveData,		_T("get open active data"));
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_get_server_acitvity_receive,	&HandleGetOpenActiveReceive,	_T("get active receive"));

	//--------------------------------------------------------------------------
	// GM命令
	//--------------------------------------------------------------------------
	REGISTER_ROLE_RECV_COMMAND(NET_SIC_gm_command,				&HandleGMCommand,				_T("GM Command"));
	m_GMCommandMgr.RegisterAll();


	//--------------------------------------------------------------------------
	// 需在地图线程上层处理的消息
	//--------------------------------------------------------------------------
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_load_complete,				&PlayerSession::HandleLoadComplete,						_T("role load ok"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_reset_instance,				&PlayerSession::handle_reset_instance,					_T("reset instance"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_reset_instance_limit,		&PlayerSession::handle_reset_inst_limit,				_T("reset inst limit"));
	// 商城
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_get,					&PlayerSession::HandleRoleMallGet,						_T("mall get item"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_update,				&PlayerSession::HandleRoleMallUpdate,					_T("mall update"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_buy_item,				&PlayerSession::HandleRoleMallBuyItem,					_T("mall buy item"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_buy_pack,				&PlayerSession::HandleRoleMallBuyPack,					_T("mall buy pack"));
	/*REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_present_item,			&PlayerSession::HandleRoleMallPresentItem,				_T("mall buy item for friend"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_present_pack,			&PlayerSession::HandleRoleMallPresentPack,				_T("mall buy pack for friend"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_free_get_item,			&PlayerSession::HandleRoleMallFreeGetItem,				_T("mall get free item"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_launch_group_purchase,		&PlayerSession::HandleRoleMallLaunchGroupPurchase,		_T("launch group purchase"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_respond_group_purchase,		&PlayerSession::HandleRoleMallRespondGroupPurchase,		_T("respond group purchase"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_buy_info,				&PlayerSession::HandleRoleMallGetGroupPurchaseInfo,		_T("get guild group purchase info"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_participators,			&PlayerSession::HandleRoleMallGetParticipators,			_T("get guild group purchase participators"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_item_exchange,			&PlayerSession::HandleRoleMallItemExchange,				_T("mall item exchange"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_mall_pack_exchange,			&PlayerSession::HandleRoleMallPackExchange,				_T("mall pack exchange"));
*/

	// 元宝交易
	/*REGISTER_WORLD_RECV_COMMAND(NET_SIC_save_yuanbao_to_account,			&PlayerSession::HandleRoleSaveYB2Account,				_T("save yuan bao to account"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_save_silver_to_account,		&PlayerSession::HandleRoleSaveSilver2Account,			_T("save silver to account"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_deposit_account_yuanbao,			&PlayerSession::HandleRoleDepositYBAccount,				_T("deposit yuan bao from account"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_deposit_account_silver,		&PlayerSession::HandleRoleDepositSilver,				_T("deposit silver from account"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_synchronize_yuanbao_trade_info,			&PlayerSession::HandleRoleGetYBTradeInfo,				_T("get yuan bao trade information"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_submit_sell_order,			&PlayerSession::HandleRoleSubmitSellOrder,				_T("submit yuan bao sell order"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_submit_buy_order,			&PlayerSession::HandleRoleSubmitBuyOrder,				_T("submit yuan bao buy order"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_delete_order,				&PlayerSession::HandleRoleDeleteOrder,					_T("delete yuan bao trade order"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_yuanbao_order,			&PlayerSession::HandleRoleGetYBOrder,					_T("get yuan bao order"));*/

	// 名人堂
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_activation_treasure,			&PlayerSession::HandleActiveTreasure,					_T("active clan treasure"));

	// 帮派
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_create,				&PlayerSession::HandleCreateGuild,						_T("create guild"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_dismiss,				&PlayerSession::HandleDismissGuild,						_T("dismiss guild"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_join_request,				&PlayerSession::HandleJoinGuildReq,						_T("join guild req"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_join_request_by_name,		&PlayerSession::HandleJoinGuildReqByName,				_T("join guild req by RoleName"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_join_request_result,			&PlayerSession::HandleJoinGuildReqRes,					_T("join guild req res"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_leave,				&PlayerSession::HandleLeaveGuild,						_T("leave guild"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_kick,					&PlayerSession::HandleKickFromGuild,					_T("kick from guild"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_trunover,				&PlayerSession::HandleTurnoverGuild,					_T("turnover guild"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_demiss,				&PlayerSession::HandleDemissFromGuild,					_T("demiss from guild"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_appoint,				&PlayerSession::HandleAppointForGuild,					_T("appoint for guild"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_tenet_change,			&PlayerSession::HandleChangeGuildTenet,					_T("change guild tenet"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_position_name_change,		&PlayerSession::HandleGuildPosNameChange,			_T("change guild pos name"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_position_power_change,		&PlayerSession::HandleGuildPosPowerChange,		_T("change guild pos power"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_symbol,				&PlayerSession::HandleChangeGuildSymbol,				_T("change guild symbol"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_enter_guild_map,			&PlayerSession::HandleEnterGuildMap,					_T("enter guild map"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Start_Guild_Practice,		&PlayerSession::HandleStartGuildPractice,				_T("start guild practice"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Guild_Tripod_Info,			&PlayerSession::HandleGuildTripodInfo,					_T("guild tripod info"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Get_Guild_Fund,				&PlayerSession::HandleGuildGetFund,						_T("guild get fund"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Guild_War_History,			&PlayerSession::HandleGuildWarHistory,					_T("guild war history"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Guild_increase_fund,		&PlayerSession::HandleGuildIncreaseFund,				_T("guild increase fund"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Material_receive,			&PlayerSession::HandleGuildMaterialReceive,				_T("guild material receive"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_Guild_donate_fund,			&PlayerSession::HandleGuilddonateFund,					_T("guild donate fund"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_all_guild_info,			&PlayerSession::HandleGetAllGuildInfo,					_T("get guild info"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_mianzhan,				&PlayerSession::HandleGuildMianzhan,					_T("guild mianzhan"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_Modify_ApplyLevel,			&PlayerSession::HandleGuildModifyApplyLevel,			_T("guild mianzhan"));

	// 帮会招募榜
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_join_guild_recruit,			&PlayerSession::HandleJoinGuildRecruit,					_T("join guild recruit"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_leave_guild_recruit,		&PlayerSession::HandleLeaveGuildRecruit,				_T("leave guild recruit"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_query_guild_recruit,		&PlayerSession::HandleQueryGuildRecruit,				_T("query guild recruit"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_query_page_guild_recruit,	&PlayerSession::HandleQueryPageGuildRecruit,			_T("query page guild recruit"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_agree_join,					&PlayerSession::HandleAgreeJoin,						_T("agree join"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_noagree_join,				&PlayerSession::HandlenoAgreeJoin,						_T("noagree join"));


	//DKP
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_dkp,					&PlayerSession::HandleGetDKP,							_T("guild get dkp"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_set_dkp,					&PlayerSession::HandldSetDKP,							_T("guild set dkp"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_dkp_affirmance,			&PlayerSession::HandleDKPAffirmance,					_T("guild dkp affirmance"));

	// 帮会战
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_set_enemy_guild,			&PlayerSession::HandleSetEnemyGuild,					_T("set enemy guild"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_delete_enemy_guild,			&PlayerSession::HandleDelEnemyGuild,					_T("del enemy guild"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_enemy_data,				&PlayerSession::HandleGetEnemyData,						_T("get enemy data"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_declare_war,			&PlayerSession::HandleGuildDeclareWar,					_T("guild declare war"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_declare_war_result,	&PlayerSession::HandleGuildDeclareWarRes,				_T("guild declare war res"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_war_qualify,			&PlayerSession::HandleGuildQualifyWar,					_T("guild qualify war"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_war_dis_qualify,		&PlayerSession::HandleGuildDisqualifyWar,				_T("guild disqualify war"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_war_num,				&PlayerSession::HandleGuildWarNum,						_T("guild war number"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_war_money_param,		&PlayerSession::HandleGuildWarMoneyParam,				_T("guild war money param"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_Guild_War_Relay,			&PlayerSession::HandleGuildWarRelay,					_T("guild war relay"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_sign_up_attack,				&PlayerSession::HandleSignUpAttack,						_T("guild sign up attack"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_SBK_data,				&PlayerSession::HandleGetSBKData,						_T("get SBK data"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_SBK_reward,				&PlayerSession::HandleGetSBKReward,						_T("get SBK reward"));	
	// 帮会联盟
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_invite_league,				&PlayerSession::HandleInviteLeague,						_T("guild invite league"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_invite_league_result,			&PlayerSession::HandleInviteLeagueRes,					_T("guild invite league res"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_relieve_league,			&PlayerSession::HandelRelieveLeague,					_T("guild relieve league"));

	//帮派签名
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_invite_sign,				&PlayerSession::HandleInviteSign,						_T("guild sign"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_cancel_sign,				&PlayerSession::HandleCancelSign,						_T("guild Cancel Sign"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_affirmance_sign,			&PlayerSession::HandleAffirmanceSign,					_T("guild Affirmance Sign"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_refer_sign,				&PlayerSession::HandleReferSign,						_T("guild refer sign"));

	// 帮会弹劾
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_delate_data,		&PlayerSession::HandleGetGuildDelateData,				_T("get guild delate data"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_delate_content,	&PlayerSession::HandleGetGuildDelateContent,			_T("get guild delate content"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_delate_leader,				&PlayerSession::HandleDelateLeader,						_T("guild delate leader"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_delate_ballot,				&PlayerSession::HandleDelateBallot,						_T("guild delate ballot"));

	// 帮派信息获取消息
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_get_all_member,		&PlayerSession::HandleGetGuildMembers,					_T("get all guild members"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_get_member_extend,			&PlayerSession::HandleGetGuildMemberEx,					_T("get guild member ex"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_refresh_member,			&PlayerSession::HandleRefreshGuildMember,				_T("refresh guild member"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_name,				&PlayerSession::HandleGetGuildName,						_T("get guild name"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_tenet,				&PlayerSession::HandleGetGuildTenet,					_T("get guild tenet"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_symbol,			&PlayerSession::HandleGetGuildSymbol,					_T("get guild symbol"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_synchronize_guild_info,				&PlayerSession::HandleSyncGuildInfo,					_T("sync guild base info"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_synchronize_guild_war_info,				&PlayerSession::HandleSyncGuildWarInfo,					_T("sync guild war info"));

	// 帮派仓库
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_ware,				&PlayerSession::HandleGetGuildWareItems,				_T("get guild warehouse items"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_ware_power_list,		&PlayerSession::HandleGetGuildWarePriList,				_T("get member warehouse privilege list"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_change_guild_ware_power,				&PlayerSession::HandleGuildWarePrivilege,				_T("change member warehouse privilege"));

	// 帮派设施升级
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_upgreae_info,		&PlayerSession::HandleGetGuildFacilitiesInfo,			_T("get guild facilities info"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_upgrede_level,				&PlayerSession::HandleHandInItems,						_T("hand in items"));

	// 帮务发布
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_issuance_guild_affair,			&PlayerSession::HandleSpreadGuildAffair,				_T("spread guild affair"));

	//// 帮派技能
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_skill_info,			&PlayerSession::HandleGetGuildSkillInfo,				_T("get guild skill info"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_guild_skill_upgrade,			&PlayerSession::HandleUpgradeGuildSkill,				_T("handin guild skill book"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_learn_guild_skill,			&PlayerSession::HandleLearnGuildSkill,					_T("learn guild skill"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_research_skill,			&PlayerSession::HandleSetResearchSkill,					_T("set current research guild skill"));

	//// 帮派跑商相关
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_accept_commerce,			&PlayerSession::HandleAcceptCommerce,					_T("accept commerce"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_complete_commerce,			&PlayerSession::HandleCompleteCommerce,					_T("complete commerce"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_abandon_commerce,			&PlayerSession::HandleAbandonCommerce,					_T("abandon commerce"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_switch_commendation,		&PlayerSession::HandleSwitchCommendation,				_T("switch commend status"));

	// 角色名贴
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_set_role_card,				&PlayerSession::HandleRoleSetVCard,						_T("Set VCard"));

	// 客户端对话框发给服务的缺省消息
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_dialog_default_message,				&PlayerSession::HandleDlgDefaultMsg,					_T("dialog default message"));
	// 客户端触发服务器脚本的缺省消息
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_default_request,			&PlayerSession::HandleDefaultRequest,					_T("Default Request"));

	// VIP摊位相关
	/*REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_all_vip_stall_info,		&PlayerSession::HandleGetAllVIPStallInfo,				_T("get all vip stall info"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_update_vip_stall_info,		&PlayerSession::HandleUpdateVIPStallInfo,				_T("get updated vip stall info"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_apply_vip_stall,				&PlayerSession::HandleApplyVIPStall,					_T("apply vip stall"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_special_vip_stall_get,			&PlayerSession::HandleSpecVIPStallGet,					_T("get vip stall goods info"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_vip_stall_buy,				&PlayerSession::HandleBuyVIPStallGoods,					_T("buy vip stall goods"));*/


	// 获得其他玩家的装备信息
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_remote_role_equip_info,	&PlayerSession::HandleGetSomeoneEquip,					_T("get someone's equip info"));

	//邮件系统
	/*REGISTER_WORLD_RECV_COMMAND(NET_SIC_send_mail,						&PlayerSession::HandleSendMail,							_T("send mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_own_mail,					&PlayerSession::HandleGetOwnMail,						_T("get own mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_mail_content,				&PlayerSession::HandleGetMailContent,					_T("get mail content"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_one_mail,					&PlayerSession::HandleGetOneMail,						_T("get one mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_mail_item,					&PlayerSession::HandleGetMailItem,						_T("get mail item"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_accept_accessory_mail,			&PlayerSession::HandleAcceptAccessoryMailItem,			_T("accept mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_read_mail,						&PlayerSession::HandleReadMail,							_T("read mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_delete_mail,					&PlayerSession::HandleDeleteMail,						_T("delete mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_return_mail,					&PlayerSession::HandleReturnMail,						_T("return mail"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_mail_num,					&PlayerSession::HandleGetMailNum,						_T("get mail num"));*/

	//师徒
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_make_master,					&PlayerSession::HandleMakeMaster,				_T("make master"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_make_master_extend,					&PlayerSession::HandleMakeMasterEx,				_T("make master ex"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_make_prentice,					&PlayerSession::HandleMakePrentice,				_T("make prentice"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_make_prentice_extend,				&PlayerSession::HandleMakePrenticeEx,			_T("make prentice ex"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_master_prentice_break,			&PlayerSession::HandleMasterPrenticeBreak,		_T("master prentice break"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_master_placard,				&PlayerSession::HandleGetMasterPlacard,			_T("get master placards"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_show_in_master_placard,			&PlayerSession::HandleShowInMasterPlacard,		_T("hide or show in master placard"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_call_in_master,					&PlayerSession::HandleCallInMaster,				_T("call in master"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_prentice_call_in,				&PlayerSession::HandlePrenticeCallIn,			_T("prentice call me"));

	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_join_master_recruit,				&PlayerSession::HandleJoinMasterRecruit,			_T("join master recruit"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_leave_master_recruit,				&PlayerSession::HandleLeaveMasterRecruit,			_T("leave master recruit"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_query_page_master_recruit,	&PlayerSession::HandleQueryPageMasterRecruit,			_T("query master recruit"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_say_goodbye_to_master,	&PlayerSession::HandleSayGoodByteToMaster,			_T("say goodbye to master"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_Master_teach_Prentice, &PlayerSession::HandleMasterTeachPrentice,			_T("master teach prentice"));//gx add 2013.12.06
	// 排行榜
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_level_rank,				&PlayerSession::HandleGetLevelRank,				_T("level rank"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_equip_rank,				&PlayerSession::HandleGetEquipRank,				_T("equip rank"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_guild_rank,				&PlayerSession::HandleGetGuildRank,				_T("guild rank"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_skill_rank,				&PlayerSession::HandleGetKillRank,				_T("kill rank"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_justice_rank,			&PlayerSession::HandleGetJusticeRank,			_T(""));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_1v1_rank,				&PlayerSession::HandGet1v1Rank,					_T("1v1 rank"));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_meili_rank,			&PlayerSession::HandleGetShihunRank,			_T(""));
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_master_rank,		&PlayerSession::HandleGetMasterRank,	_T(""));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_ach_point_rank,			&PlayerSession::HandleGetAchPointRank,			_T("achievement point"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_ach_number_rank,		&PlayerSession::HandleGetAchNumberRank,			_T("achievement number"));
	//!!!!!使用追杀令道具
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_use_skill_badge_item,		&PlayerSession::HandleRoleUseKillBadage, _T("HandleRoleUseKillBadage"));

	//// !! 悬赏任务
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_PutGDQuest,				&PlayerSession::HandlePutQuest,		_T("NET_SIC_PutGDQuest"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_GetGDQuest,				&PlayerSession::HandleGetQuest,		_T("NET_SIC_GetGDQuest"));
	////REGISTER_WORLD_RECV_COMMAND(NET_SIC_GiveUpGDQuest,			&PlayerSession::HandleGiveUpQuest,		_T("NET_SIC_GiveUpGDQuest"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_GetOnePageGuerdonQuest,	&PlayerSession::HandleGetOnePageGDQuest,	_T("NET_SIC_GetOnePageGuerdonQuest"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_UpdateMyPutGuerdonQuest,	&PlayerSession::HandleUpdateMyPutGDQuest,	_T("NET_SIC_UpdateMyPutGuerdonQuest"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_UpdateMyGetGuerdonQuest,	&PlayerSession::HandleUpdateMyGetGDQuest,	_T("NET_SIC_UpdateMyGetGuerdonQuest"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_CancelPutGDQuest,	&PlayerSession::HandleCancelPutGDQuest,	_T("NET_SIC_CancelPutGDQuest"));
	////REGISTER_WORLD_RECV_COMMAND(NET_SIC_Get_Team_Share_Quest,	&PlayerSession::HandleTeamNextQuest,	_T("NET_SIC_Get_Team_Share_Quest"));


	//拍卖行
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_begin_paimai,			&PlayerSession::handle_begin_paimai,		_T("handle_begin_paimai"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_cancel_paimai,			&PlayerSession::handle_cancel_paimai,		_T("handle cancel paimai"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_jingpai,				&PlayerSession::handle_jingpai,				_T("handle jingpai"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_chaw_buy,				&PlayerSession::handle_chaw_buy,			_T("handle chaw buy"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_paimai_query,			&PlayerSession::handle_paimai_query,		_T("handle paimai query"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_paimai_change_page,		&PlayerSession::handle_paimai_change_page,	_T("handle paimai change page"));

	//// 钱庄
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_begin_bank_paimai,		&PlayerSession::handle_begin_bank_paimai,	_T("handle begin bank paimai"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_cancel_role_bank_paimai,	&PlayerSession::handle_cancel_role_bank_paimai, _T("handle cancel bank paimai"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_begin_bank_jing,		&PlayerSession::handle_begin_bank_jing,			_T("handle begin bank jing"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_bank_chaw_buy,			&PlayerSession::handle_bank_chaw_buy,			_T("handle bank chaw buy"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_query_bank,				&PlayerSession::handle_query_bank,				_T("handle query bank"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_bank_change_page,		&PlayerSession::handle_bank_change_page,		_T("handle bank change page"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_yuanban_exchange,		&PlayerSession::handle_yuanbao_exchange,		_T("handle yuanbao exchange"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_yuanbao_exchange_num, &PlayerSession::handle_get_yuanbao_exchange_num, _T("handle get yuanbao exchange num"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_bank_radio,			&PlayerSession::handle_get_bank_radio,			_T("handle get_bank radio"));
	//

	// 宠物sns
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_pet_paiqian,			&PlayerSession::HandlePetPaiqian,				_T("paiqian pet"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_pet_return,				&PlayerSession::HandlePetReturn,				_T("return pet"));

	////1V1竞技场
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_1v1_apply,				&PlayerSession::Handle1v1Apply,					_T("1v1 apply"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_1v1_leave_queue,		&PlayerSession::Handle1v1LeaveQueue,			_T("1v1 leave queue"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_1v1_leave,				&PlayerSession::HandleLeave1v1,					_T("leave 1v1"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_1v1_score_award,	&PlayerSession::HandleGet1v1ScoreAward,			_T("get 1v1 award"));

	//// 约战竞技场
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_reservation_apply,		&PlayerSession::HandleReservationApply,			_T("reservation apply"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_reservation_info,	&PlayerSession::HandleGetReservationInfo,		_T("get reservation info"));
	//REGISTER_WORLD_RECV_COMMAND(NET_SIC_reservation_result,		&PlayerSession::HandleReservationResult,		_T("reservation result"));

	//// gp 商城
	REGISTER_WORLD_RECV_COMMAND(NET_SIC_get_lottery,			&PlayerSession::HandleGetLottery,				_T("get lottery"));
	
}

//------------------------------------------------------------------------------------
// 注册所有的发送消息
//------------------------------------------------------------------------------------
VOID PlayerSession::RegisterALLSendCmd()
{
	// 进入游戏
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_join_game");

	// 选人界面
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_enum_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_create_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_select_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_safe_code");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_reset_safe_code");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_safe_code_reset");

	// 属性和状态
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_unset_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_role_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_unset_role_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_att");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_complete_quest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_incomplete_quest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_suit");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_itemcdtime");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_init_state_money");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_remote_role_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_remote_creature_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_single_role_att_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mutiple_role_att_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_single_remote_att_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mutiple_remote_att_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_role_exp");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_role_level");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_att_point");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_clear_att_point");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_change_role_att_point");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_friend_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_remove_remote");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_target_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_get_signature_name");

	// 名字和ID
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_get_id");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_name_by_nameid");
	
	// 移动
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_walk");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_jump");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_drop");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_apeak_drop");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_slide");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_stand");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_move_failed");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_hit_fly");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_move_speed_change");

	// 聊天
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_char");

	// 战斗
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_skill_interrupt");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_item_interrupt");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_hit_target");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_hp_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_dead");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_revive");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_revive_notify");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_role_buffer");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_remove_role_buffer");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_role_buffer");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stop_action");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_style_action");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_monster_enter_combat");

	// 生产
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_produce");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_decomposition");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_consolidate_posy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_consolidate_engrave");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_consolidate_quench");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_inlay");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_chisel");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_dye_fashion");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_consolidate_posy");
	
	// 金钱和元宝
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bag_silver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_ware_silver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bag_yuanbao");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_baibao_yuanbao");

	// 交易
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_request");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_request_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_exchange_to_src");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_exchange_to_dest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_exchange_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_money");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_item_lock");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_cancel");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_finish");

	// 职能NPC
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_posthouse");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_ware_extend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bag_extend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_save_silver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_silver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_save_yuan_bao");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_abrase_stone");

	// 组队
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_to_leader");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_join_team");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_reply");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_kick_member");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_leave_team");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_pick_mode");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_leader");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_state_to_team");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_position_to_team");

	// 物品和装备
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_unequip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_swap_weapon");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_avatar_equip_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_identify_equip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_suit_effect");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_suit_num");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_item_position_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_item_position_change_extend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_item_add");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_new_item_add");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_new_equip_add");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_item_cd_update");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_practice_begin");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_parctice_end");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_fusion");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_weapon_clear_talent");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_bind");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_unbind");

	// 掉落
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_ground_item_disappear");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_putdown_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_pickup_item");

	// 地图
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_enter_instance");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_goto_new_map");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_instance_nofity");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_instance_agree");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_instance_complete");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_instance_time");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bind_reborn_map");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_enter_battle_instance");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_battle_ground_end");

	// PK
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_pk_value");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_pk_state");

	// 商店
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_shop_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_shop_equip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_buy_shop_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_buy_shop_equip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_feedback_from_shop");

	// 展示物品和装备
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_show_equip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_show_item");

	// 社会关系
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_login_to_friend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_logout_to_friend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_make_friend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_cancel_friend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_friend_group");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_move_to_black_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_black_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_gift_to_friend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_gift_to_sender");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_gift_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_black_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_friend_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_make_friend_notice");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_friend_value");

	// 摆摊
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_start");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_set_goods");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_unset_goods");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_set_title");
	//m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_set_advertisement");
	//m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_set_advertisement_flag");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_set_finish");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_close");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_buy_refresh");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_set_refresh");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_unset_refresh");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_get");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_get_title");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_buy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_get_special");

	// 天资及技能
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_learn_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_level_up_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_skill_cool_down");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_forget_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_clear_talent");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_remove_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_talent");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_remove_talent");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_talent");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_learn_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_level_up_skill");

	// 任务
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_accept_quest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_quest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_complete_quest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_quest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_update_kill_creature");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_update_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_update_npc_talk");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_update_trigger");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_CircleQuestList");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_inves_faild");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_quest_track");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_var_value");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_faild_message");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_update_inveset");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_quest_update_npc_talk");



	// 声望
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_reputation");

	// 反外挂
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_game_sentinel");

	// 名人堂
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_celeb_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_reputation_top");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_activation_gens_treasure");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_activation_treasure");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_new_activation_treasure");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_treasure_act_count");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_becomeframe");

	// 角色名贴
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_card");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_role_card");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_head_picture");
	
	// 称号
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_role_title");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_role_titles");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_net_titles");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_title_change_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_title_buy");

	// 帮派
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_create_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_dismiss_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_join_request");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_join_request_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_join_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_leave_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_kick_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_trunover_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_demiss_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_appoint_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_tenet_change_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_failed_disposal");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_get_all_member");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_get_member_extend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_refresh_member");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_name");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_tenet");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Start_Guild_Practice");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_qualify");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_dis_qualify");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_num");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_money_param");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_member_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_maxnum_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_money_param_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_guild_war_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Get_Guild_Fund");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Guild_War_Relay");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Guild_increase_fund");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Guil_war_member_number_update");

	// 百宝袋	
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_init_baibao_record");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_single_baobao_record");


	// 宠物
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_pet_att");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_pet_display_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_display_info_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_pet_egg");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_att_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_rename");
	m_PlayerNetMgr.RegisterSendProc("NET_SIC_use_special_pet_item");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_equip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_unequip");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_equip_position_swap");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_pet_pour_exp_money_need");
	
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_pet_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_pet_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_remove_pet_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_forget_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_paiqian");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_be_paiqian");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_return");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_buy_love_vaule");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_color");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_add_point");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_fusion");

	// 宠物交易
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_request");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_request_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_add_to_src");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_exchange_add_to_dest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_subtract");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_money");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_lock");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_cancel");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_exchange_finish");
	

	// 跑商相关
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_chamber_goods_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_commodity_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_tael_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_commerce_rank");

	//	宝箱
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_treasure_chest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stop_treasure_chest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_repeat_treasure_chest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_treasure_item");

	// 返回角色选择
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_return_role_select");
	
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_acitvity_state");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_begin_bank_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_role_bank_paimai_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_role_bank_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_role_bank_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_role_bank_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_begin_bank_jing");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_bank_jing");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bank_chaw_buy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_query_bank");
	
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_auto_placard");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_right_placard");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_monster_type_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_monster_say");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_play_scene_music");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_monster_play_action");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_special_move");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_unbeset");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_shengxing");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_gens_contribute");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_present_point");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_enthrallment_info");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_gm_command_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_UpdateMyGetGuerdonQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_UpdateMyPutGuerdonQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_GetOnePageGuerdonQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_GDQuestTimeOut");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_GDQuestCompleteByReciver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_CompleteGDQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_GiveUpGDQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_GetGDQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_CancelPutGDQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_NewGuerdonQuest");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_PutGDQuest");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_query_guild_recruit");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_leave_guild_recruit");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_join_guild_recruit");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_inform_leave_guild_map");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_switch_commendation");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_abandon_commerce");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_complete_commerce");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_accept_commerce");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_research_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_learn_guild_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_skill_level_up");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_skill_upgrade");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_skill_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_issuance_guild_affair");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_upgrade");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_facilities_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_upgreade_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_unset_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_set_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_guild_ware_power");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_ware_power_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_ware");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_outcome_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_war_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_refuse_declare_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_declare_war_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_declare_war");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_enemy_data");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_unview_enemy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_enemy_guild");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_relieve_league");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_relieve_league");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_invite_league_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_league_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_invite_league");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_league");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_enemy_guild");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_view_enemy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_enemy_guild");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_dkp_affirmance");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_dkp");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_dkp");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_position_power_change_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_position_name_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_symbol");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_exploit");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_contribution");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_guild_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delate_ballot");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delate_leader");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_delate_content");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_delate_data");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_refer_sign");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_affirmance_sign");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_sign");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_sign_data");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_invite_sign");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_symbol_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bind_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_newess_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_bag_password");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_item_remove");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stack_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_repair");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_effect_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_mail_num");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_return_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_accept_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_read_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_mail_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_mail_content");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_own_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_one_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_inform_delete_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_inform_return_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_inform_noread_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_inform_new_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_mail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_order");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_yuanbao_order");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_submit_buy_order");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_submit_sell_order");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_buy_price_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_price_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_yuanbao_account");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_buy_price_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_sell_price_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_deposit_account_silver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_deposit_account_yuanbao");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_save_silver_to_account");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_save_yuanbao_to_account");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_account");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_account_silver");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_account_yuanbao");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_pack_exchange");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_item_exchange");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_respond_broad");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_participators");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_guild_buy_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_respond_group_purchase");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_launch_group_purchase");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_free_get_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_present_pack");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_present_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_buy_pack");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_buy_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_update_pack");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_update_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_update");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_pack");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_free_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mall_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_open_mall");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_reset_instance");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_call_in_master");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_prentice_call_in");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_prentice_delete_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_master_delete_role");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_show_in_master_placard");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_master_prentice_break");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_master_prentice_break_error");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_prentice_graduate");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_master_moral_and_gradates");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_master_placard");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_master");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_make_prentice");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_new_prentice");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_make_prentice_extend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_master_and_prentices");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_make_master");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_make_master_extend");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_double_motion_on_invite");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_double_motion_invite");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_paimai_query");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_chaw_buy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_jingpai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_delete_jingpai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_jingpai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_jingpai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_own_jingpai_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_own_paimai_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_add_paimai_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_cancel_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_begin_paimai");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_special_pet_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_rename");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_set_lock");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mount_invite");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mount_on_invite");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_mount2");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_food");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_enhance");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_up_step");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_pour_exp");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_pet_pour_exp_money_need");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_pet_state");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_learn_skill");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_use_skill_badge_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_to_prison");

	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_rank_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_guild_rank");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_view_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_equip_rank_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_equip_rank");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_skill_rank_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_skill_rank");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_level_rand_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_level_rank");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_UnEquip_ride");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Equip_ride");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_ride");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_new_mount");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_interrupt_begin_ride");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_begin_ride");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_ride_inlay");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_upgrade_ride");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_vigour_reward");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_delay");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_new_role_gift");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_new_role_gift");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_key_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_talk");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_help");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pet_pocket_size_change");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_hang_num");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_leave_exp_clueon");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_pickup_leave_exp");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_leave_exp");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_cancel_hang");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_start_hang");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_remote_role_equip_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_creuture_adscription");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_close_door");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_enemy_list");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_get_signature_name");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_vip_stall_buy");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_special_vip_stall_get");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_apply_vip_stall");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_update_vip_stall_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_all_vip_stall_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_chat");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_stall_history_chat");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_team_leader_set");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_team_member_obtain_ground_item");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_team_assign_error");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_team_assign_sice_result");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_team_assign_sicefor");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_team_leader_assign");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_set_assign_quality");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_send_map_team_info");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_team_placard_fail");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_change_team_placard");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_team_id");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_role_state_to_invitee");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_synchronize_time");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_yuanbao_exchange");
	
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Material_receive");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_Guild_donate_fund");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_get_wuhuen");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_ronglian");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_equip_fumo");

	// 验证码
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_reset_verification_code");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_need_verification");
	m_PlayerNetMgr.RegisterSendProc("NET_SIS_need_verification_return");


	m_PlayerNetMgr.RegisterSendProc("NET_SIS_get_lottery");

	//m_PlayerNetMgr.RegisterSendProc("");
}

//------------------------------------------------------------------------------------
// 撤销所有客户端网络命令
//------------------------------------------------------------------------------------
VOID PlayerSession::UnRegisterALL()
{
	// 将本次统计信息写入log
	m_PlayerNetMgr.LogAllMsg();

	m_PlayerNetMgr.UnRegisterAll();
	m_GMCommandMgr.UnRegisterAll();
}

//------------------------------------------------------------------------------------
// 消息处理主函数（当返回INVALID_VALUE, 则说明连接已经丢失，要删除该session）
//------------------------------------------------------------------------------------
INT PlayerSession::HandleMessage()
{
	// 检测客户端是否已经失去连接
	if( m_bConnectionLost )
	{
		LogoutPlayer();
		return CON_LOST;
	}

	// 处理客户端消息
	DWORD dw_size = 0;
	LPBYTE p_message = RecvMsg(dw_size);
	if( !VALID_POINT(p_message) ) return 0;

	tag_net_message* pCmd = (tag_net_message*)p_message;

	NETMSGHANDLER pHandler = m_PlayerNetMgr.GetHandler(pCmd, dw_size);

	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	DWORD dwRet = INVALID_VALUE;
	if( NULL != pHandler )
		dwRet = (this->*pHandler)(pCmd);

	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 10)
	{
		g_world.get_log()->write_log(_T("update_session time %d, %d\r\n"), dw_new_time - dw_time, pCmd->dw_message_id);
	}

	// 判断是否需上层处理
	if( RET_TRANS == dwRet )
	{
		WorldNetCmdMgr::GetInstance()->Add(GetSessionID(), p_message, dw_size);
	}
	else
	{
		RecycleMsg(p_message);
	}

	BYTE by_delay = 0;
	if(GetMsgWaitNum() < 5)
		by_delay = 0;
	else if(GetMsgWaitNum() >= 5 && GetMsgWaitNum() < 10)
		by_delay = 1;
	else if(GetMsgWaitNum() >= 10 && GetMsgWaitNum() < 30)
		by_delay = 2;
	else if(GetMsgWaitNum() >= 30 && GetMsgWaitNum() < 64)
		by_delay = 3;

	if(by_delay_send != by_delay)
	{
		by_delay_send = by_delay;
		NET_SIS_delay_send send;
		send.by_delay = by_delay_send;
		SendMessage(&send, send.dw_size);
	}


	// 如果发现session的m_bPushed被设置成TRUE，则说明客户端在此消息处理之后被移动出地图
	


	return 0;
}

//------------------------------------------------------------------------------------
// 发送消息接口
//------------------------------------------------------------------------------------
VOID PlayerSession::SendMessage(LPVOID p_message, DWORD dw_size)
{
	if( !VALID_POINT(p_message) || dw_size == 0 )
		return;

	// 记录发包数量
	m_PlayerNetMgr.CountServerMsg(*(DWORD*)p_message);

	// 连接已经中断，不再发送了
	if( m_bConnectionLost || m_bKicked ) return;

	if( dw_size > g_world.GetMaxMsgSize() )
	{
		g_world.SetMaxMsgSize(dw_size);

		tag_net_message* p_msg = (tag_net_message*)p_message;

		g_world.get_log()->write_log(_T("SendMessage id=%u, size=%d\r\n"), p_msg->dw_message_id, dw_size);
	}

	// 发送到网络底层
	SendMsg((LPBYTE)p_message, dw_size);
}

//------------------------------------------------------------------------------
// 完全登陆玩家（要将玩家从选人界面session列表移动到一个mapmgr的session列表中）
//------------------------------------------------------------------------------
BOOL PlayerSession::FullLogin(Role* pRole, BOOL bFirst)
{
	ASSERT( VALID_POINT(pRole) );

	SetRole(pRole);

	m_bRoleLoading = false;

	DWORD dwSessionID = m_dwAccountID;	// 保证预先取出sessionID，线程安全

	// 将该角色加入到地图中
	if( !pRole->AddToWorld(bFirst) )
	{
		SetRole(NULL);
		return FALSE;
	}

	// 清空dressid
	if( bFirst )
	{
		NET_DB2C_clear_role_dress_mdid send;
		send.dw_role_id	= pRole->GetID();
		send.w_new_val	= DressMdIDInvalid;
		g_dbSession.Send(&send, send.dw_size);
	}

	// 已经加入到地图中，从全局列表中删除自己
	g_worldSession.RemoveGlobalSession(dwSessionID);
	return TRUE;
}

//------------------------------------------------------------------------------
// 接收消息
//------------------------------------------------------------------------------
LPBYTE PlayerSession::RecvMsg(DWORD& dw_size)
{
	return g_worldSession.RecvMsg(dw_size, m_nMsgNum, m_dwInternalIndex);
}

VOID PlayerSession::Log_Free_Message()
{
	g_worldSession.Log_Free_Message(m_dwInternalIndex, m_dwAccountID);
}

//------------------------------------------------------------------------------
// 归还消息
//------------------------------------------------------------------------------
VOID PlayerSession::ReturnMsg(LPBYTE p_message)
{
	g_worldSession.ReturnMsg(p_message);
}

//------------------------------------------------------------------------------
// 发送消息
//------------------------------------------------------------------------------
VOID PlayerSession::SendMsg(LPBYTE p_message, DWORD dw_size)
{
	g_worldSession.SendMsg(m_dwInternalIndex, p_message, dw_size);
}

//------------------------------------------------------------------------------
// 登出玩家
//------------------------------------------------------------------------------
VOID PlayerSession::LogoutPlayer()
{
	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return;

	// 记录玩家要登出
	g_mapCreator.role_logout(pRole->GetID());
}

//------------------------------------------------------------------------------
// 重置session的内容，用于返回选人界面
//------------------------------------------------------------------------------
VOID PlayerSession::Refresh()
{
	m_bRoleEnuming		=	false;
	m_bRoleEnumDone		=	false;
	m_bRoleEnumSuccess	=	false;
	m_bRoleLoading		=	false;
	m_bRoleDeleting		=	false;
	m_bRoleCreating		=	false;
	m_bRoleInWorld		=	false;
	m_bRoleChangeNameing = false;
	m_bRoleDelGuardCanceling = false;
	m_bRoleVerifying	=	false;
	m_pRole				=	NULL;
}

//------------------------------------------------------------------------------
// 判断是否帐号下是否有该角色
//------------------------------------------------------------------------------
BOOL PlayerSession::IsRoleExist(const DWORD dw_role_id) const
{
/*
	if(INVALID_VALUE == dw_role_id)
	{
		return FALSE;
	}

	for(INT i=0; i<MAX_ROLENUM_ONEACCOUNT; ++i)
	{
		if(dw_role_id == m_dwRoleID[i])
		{
			return TRUE;
		}
	}

	return FALSE;
*/
	return m_SessionCommonData.IsExist(PSCSearchPred(dw_role_id));
}

//------------------------------------------------------------------------------
// 帐号添加新角色
//------------------------------------------------------------------------------
BOOL PlayerSession::AddRole(const DWORD dw_role_id)
{
/*
	for(INT i=0; i<MAX_ROLENUM_ONEACCOUNT; ++i)
	{
		if(INVALID_VALUE == m_dwRoleID[i])
		{
			m_dwRoleID[i] = dw_role_id;
			++m_n8RoleNum;
			return TRUE;
		}
	}

	return FALSE;
*/
	PlayerSessionCommonData data;
	data.dwRoleID = dw_role_id;
	return m_SessionCommonData.Add(data);
}

//------------------------------------------------------------------------------
// 删除帐号下角色
//------------------------------------------------------------------------------
BOOL PlayerSession::RemoveRole(const DWORD dw_role_id)
{
/*
	for(INT i=0; i<MAX_ROLENUM_ONEACCOUNT; ++i)
	{
		if(dw_role_id == m_dwRoleID[i])
		{
			m_dwRoleID[i] = INVALID_VALUE;
			--m_n8RoleNum;
			return TRUE;
		}
	}

	
	return FALSE;
*/
	int index = m_SessionCommonData.Find(PSCSearchPred(dw_role_id));
	return m_SessionCommonData.RemoveIndex(index);
}

DWORD PlayerSession::GetRoleDelGuardTime(const DWORD dw_role_id) const
{
	int index = m_SessionCommonData.Find(PSCSearchPred(dw_role_id));
	const PlayerSessionCommonData* pPSCD = m_SessionCommonData.GetPtr(index);
	return VALID_POINT(pPSCD) ? *((const DWORD*)&pPSCD->dwDelGuardTime) : INVALID_VALUE;
}

VOID PlayerSession::SetRoleDelGuardTime(DWORD dw_role_id, DWORD dw_time)
{
	int index = m_SessionCommonData.Find(PSCSearchPred(dw_role_id));
	if(index != -1) m_SessionCommonData.GetRef(index).dwDelGuardTime = dw_time; 
}

BOOL PlayerSession::IsRoleInDelGuard(const DWORD dw_role_id) const
{
	DWORD dwTime = GetRoleDelGuardTime(dw_role_id);
	return VALID_POINT(dwTime) ? TRUE : FALSE;
}

DWORD PlayerSession::GetRoleChangeNameTime(const DWORD dw_role_id) const
{
	int index = m_SessionCommonData.Find(PSCSearchPred(dw_role_id));
	const PlayerSessionCommonData* pPSCD = m_SessionCommonData.GetPtr(index);
	return VALID_POINT(pPSCD) ? *((const DWORD*)&pPSCD->dwChangeNameTime) : INVALID_VALUE;
}

VOID PlayerSession::SetChangeNameTime(DWORD dw_role_id, DWORD dw_time)
{
	int index = m_SessionCommonData.Find(PSCSearchPred(dw_role_id));
	if(index != -1) m_SessionCommonData.GetRef(index).dwChangeNameTime = dw_time; 
}

BOOL PlayerSession::IsCanChangeName(const DWORD dw_role_id) const
{
	tagDWORDTime dwTime; dwTime = GetRoleChangeNameTime(dw_role_id);
	if(!VALID_POINT(dwTime)) return TRUE;

	return CalcTimeDiff(dwTime, GetCurrentDWORDTime()) <= 0;
}

//------------------------------------------------------------------------------
// 判断是否可以设置安全码
//------------------------------------------------------------------------------
BOOL PlayerSession::CanSetSafeCode()
{
	// 没有设置过安全码
	if(INVALID_VALUE == m_sAccountCommon.s_safe_code_.dw_safe_code_crc)
	{
		return TRUE;
	}
		
	// 当前有安全码，而且没有重置过
	if(INVALID_VALUE == m_sAccountCommon.s_safe_code_.dw_reset_time)
	{
		return FALSE;
	}

	// 重置时间未够72小时
	if(CalcTimeDiff(g_world.GetWorldTime(), m_sAccountCommon.s_safe_code_.dw_reset_time) < MAX_SAFECODE_RESET_TIME)
	{
		return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// 判断是否可以重置安全码
//------------------------------------------------------------------------------
BOOL PlayerSession::CanResetSafeCode() const
{
	// 没有设置过安全码
	if(INVALID_VALUE == m_sAccountCommon.s_safe_code_.dw_safe_code_crc)
	{
		return FALSE;
	}

	// 当前有安全码，而且没有重置过
	if(INVALID_VALUE == m_sAccountCommon.s_safe_code_.dw_reset_time)
	{
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
// 判断是否可以取消安全码重置
//------------------------------------------------------------------------------
BOOL PlayerSession::CanCancelSafeCodeReset() const
{
	// 没有设置过安全码
	if(INVALID_VALUE == m_sAccountCommon.s_safe_code_.dw_safe_code_crc)
	{
		return FALSE;
	}

	// 当前有安全码，而且没有重置过
	if(INVALID_VALUE == m_sAccountCommon.s_safe_code_.dw_reset_time)
	{
		return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// 在当前频道向九宫格内所有玩家广播消息
//------------------------------------------------------------------------------
VOID PlayerSession::BroadcastCurrChannel(LPCTSTR szMsg)
{
	if (!VALID_POINT(szMsg))
	{
		return;
	}

	size_t len = _tcslen(szMsg);
	INT	nSzMsg = len * sizeof(TCHAR);
	if (len > MAX_BROADCAST_MSG_LEN)
	{
		return;
	}

	if (!VALID_POINT(m_pRole))
	{
		return;
	}

	Map* pMap = m_pRole->get_map();
	if (!VALID_POINT(pMap))
	{
		return;
	}

	TCHAR msg[1024] = {0};

	NET_SIS_role_char* pSend   = (NET_SIS_role_char*)msg;
	pSend->byChannel		= (BYTE)ESCC_Common;
	pSend->dw_message_id				= get_tool()->crc32("NET_SIS_role_char");
	pSend->dwDestRoleID		= m_pRole->GetID();
	pSend->dwSrcRoleID		= m_pRole->GetID(); 
	pSend->dw_error_code		= 0;
	pSend->dw_size			= sizeof(NET_SIS_role_char) + nSzMsg;
	get_fast_code()->memory_copy(pSend->szMsg, szMsg, nSzMsg);

	// 加字符串结束符
	pSend->szMsg[len] = _T('\0');

	pMap->send_big_visible_tile_message(m_pRole, (LPVOID)pSend, pSend->dw_size);
}

//-----------------------------------------------------------------------
// 回收消息Unit(有用则再利用，无用则释放)
//-----------------------------------------------------------------------
VOID PlayerSession::RecycleMsg(LPBYTE p_message)
{
	g_worldSession.ReturnMsg(p_message);
}

//-----------------------------------------------------------------------------
// 更新
//----------------------------------------------------------------------------
INT PlayerSession::Update()
{
	return HandleMessage();
}

//-----------------------------------------------------------------------------
// 查找当前地图中的玩家
//----------------------------------------------------------------------------
Role* PlayerSession::GetOtherInMap( DWORD dw_role_id ) const 
{
	M_trans_else_ret(pThisRole,	GetRole(),					Role,	NULL);
	M_trans_else_ret(pMap,		pThisRole->get_map(),		Map,	NULL);
	M_trans_else_ret(pRole,		pMap->find_role(dw_role_id),	Role,	NULL);
	return pRole;
}

INT PlayerSession::GetVNBExpRate() const
{
	return g_VipNetBarMgr.GetRate(m_dwIP, 0);
}

INT PlayerSession::GetVNBLootRate() const
{
	return g_VipNetBarMgr.GetRate(m_dwIP, 1);
}

VOID PlayerSession::SessionLogout()
{
	g_VipNetBarMgr.PlayerLogout(m_dwIP);
	SetConnectionLost();
}

LPCTSTR PlayerSession::GetVNBName() const
{
	return g_VipNetBarMgr.GetVNBName(m_dwIP);
}

// 延迟
DWORD PlayerSession::HandleGetDelay(tag_net_message* pCmd)
{
	NET_SIS_get_delay send;
	SendMessage(&send, send.dw_size);

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	pRole->inc_delay_num();
	pRole->inc_strength_delay_num();

	return E_Success;
}

// 开始拍卖
DWORD PlayerSession::handle_begin_paimai(tag_net_message* p_cmd)
{
	NET_SIC_begin_paimai* p_recv = (NET_SIC_begin_paimai*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	

	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}


	DWORD dw_error = g_paimai.create_paimai(p_recv, pRole);

	if(dw_error != E_Success)
	{
		NET_SIS_begin_paimai send;
		send.dw_Error = dw_error;

		pRole->SendMessage(&send, send.dw_size);

	}

	return E_Success;
}

// 取消拍卖
DWORD PlayerSession::handle_cancel_paimai(tag_net_message* p_cmd)
{
	NET_SIC_cancel_paimai* p_recv = (NET_SIC_cancel_paimai*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_error = g_paimai.cancel_paimai(p_recv, pRole);

	NET_SIS_cancel_paimai send;
	send.dw_paimai_id = p_recv->dw_paimai_id;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 开始竞拍
DWORD PlayerSession::handle_jingpai(tag_net_message* p_cmd)
{
	NET_SIC_jingpai* p_recv = (NET_SIC_jingpai*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD dw_error = g_paimai.jingpai(p_recv, pRole);

	NET_SIS_jingpai send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 一口价购买
DWORD PlayerSession::handle_chaw_buy(tag_net_message* p_cmd)
{
	NET_SIC_chaw_buy* p_recv = (NET_SIC_chaw_buy*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	

	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD dw_error = g_paimai.chaw_buy(p_recv, pRole);

	NET_SIS_chaw_buy send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 拍卖查询
DWORD PlayerSession::handle_paimai_query(tag_net_message* p_cmd)
{
	NET_SIC_paimai_query* p_recv = (NET_SIC_paimai_query*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	g_paimai.query_paimai(pRole, p_recv);

	return E_Success;
}

// 翻页查询
DWORD PlayerSession::handle_paimai_change_page(tag_net_message* p_cmd)
{
	NET_SIC_paimai_change_page* p_recv = (NET_SIC_paimai_change_page*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	//pRole->get_query_paimai().clear();
	g_paimai.send_query_result(pRole, p_recv->n_page, pRole->get_query_paimai());

	return E_Success;
}

// 开始钱庄拍卖
DWORD PlayerSession::handle_begin_bank_paimai(tag_net_message* p_cmd)
{
	NET_SIC_begin_bank_paimai* p_recv = (NET_SIC_begin_bank_paimai*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;

		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD dw_error = g_bankmgr.begin_bank_paimai(pRole, p_recv);

	NET_SIS_begin_bank_paimai send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 取消钱庄拍卖
DWORD PlayerSession::handle_cancel_role_bank_paimai(tag_net_message* p_cmd)
{
	NET_SIC_cancel_role_bank_paimai* p_recv = (NET_SIC_cancel_role_bank_paimai*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD	dw_error = g_bankmgr.cancel_bank_paimai(pRole, p_recv);

	NET_SIS_cancel_role_bank_paimai send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;	
}

// 钱庄竞拍
DWORD PlayerSession::handle_begin_bank_jing(tag_net_message* p_cmd)
{
	NET_SIC_begin_bank_jing* p_recv = (NET_SIC_begin_bank_jing*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;

		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD	dw_error = g_bankmgr.begin_bank_jing(pRole, p_recv);

	NET_SIS_begin_bank_jing send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 钱庄一口价购买
DWORD PlayerSession::handle_bank_chaw_buy(tag_net_message* p_cmd)
{
	NET_SIC_bank_chaw_buy* p_recv = (NET_SIC_bank_chaw_buy*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD	dw_error = g_bankmgr.chaw_buy(pRole, p_recv);

	NET_SIS_bank_chaw_buy send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 查询钱庄信息
DWORD PlayerSession::handle_query_bank(tag_net_message* p_cmd)
{
	NET_SIC_query_bank* p_recv = (NET_SIC_query_bank*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	g_bankmgr.query_bank(pRole, p_recv);

	return E_Success;
}

// 翻页
DWORD PlayerSession::handle_bank_change_page(tag_net_message* p_cmd)
{
	NET_SIC_bank_change_page* p_recv = (NET_SIC_bank_change_page*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	//pRole->get_query_bank().clear();
	if(!p_recv->by_type)
		g_bankmgr.send_query_result(pRole, p_recv->n_page, p_recv->by_type, pRole->get_query_bank());
	else
		g_bankmgr.send_query_result(pRole, p_recv->n_page, p_recv->by_type, pRole->get_query_bank_ex());

	return E_Success;
}

// 元宝兑换
DWORD PlayerSession::handle_yuanbao_exchange(tag_net_message* p_cmd)
{
	NET_SIC_yuanban_exchange* p_recv = (NET_SIC_yuanban_exchange*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	

	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD dw_error = g_bankmgr.yuanbao_exchange(pRole, p_recv->by_type);

	NET_SIS_yuanbao_exchange send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 获取元宝兑换次数
DWORD PlayerSession::handle_get_yuanbao_exchange_num(tag_net_message* p_cmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	NET_SIS_yuanbao_exchange_num send;
	send.n_num = pRole->GetYuanbaoExchangeNum();
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 获取汇率
DWORD PlayerSession::handle_get_bank_radio(tag_net_message* p_cmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	NET_SIS_get_bank_radio send;
	if(g_bankmgr.get_radio() > 0)
	{
		send.n_bank_radio = g_bankmgr.get_radio();
	}
	else
	{
		send.n_bank_radio = AttRes::GetInstance()->GetVariableLen().n_bank_radio;
	}
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 抽奖
DWORD PlayerSession::HandleGetLottery(tag_net_message* pCmd)
{

	NET_SIC_get_lottery* p_recv = (NET_SIC_get_lottery*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	NET_SIS_get_lottery send;
	
	
	send.dwErrorCode = g_lottery.getLottery(pRole, p_recv->byType, send.dwItemIndex,send.bItemNum);
	send.byType = p_recv->byType;

	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 神级提升
DWORD PlayerSession::HandleGodLevelUp(tag_net_message* p_cmd)
{
	NET_SIC_god_level_up* p_recv = (NET_SIC_god_level_up*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_god_level_up send;
	send.dw_error_code = pRole->GodLevelUp();
	send.nLevel = pRole->getGodLevel();
	SendMessage(&send, send.dw_size);

	return 0;
}
