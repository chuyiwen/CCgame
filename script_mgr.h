
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//Lua脚本管理器
#pragma once

extern "C"
{
#include "../common/lua/src/lua.h"
#include "../common/lua/src/lauxlib.h"
#include "../common/lua/src/lualib.h"
};
#include "mutex.h"
#include "event_mgr.h"

class Unit;
class Creature;
class Role;
class activity_fix;
class ScriptMgr;
class Skill;

struct tag_map_trigger_info;
struct tag_map_area_info;

enum ERoleReviveType;

//-------------------------------------------------------------------------------
// 任务脚本事件枚举
//-------------------------------------------------------------------------------
enum EScriptQuestEvent
{
	ESQE_On_Accept			=	0,			// 任务接取
	ESQE_On_Complete		=	1,			// 任务完成
	ESQE_On_Cancel			=	2,			// 任务取消
	ESQE_On_Creature_Kill	=	3,			// 杀死一只怪物
	ESQE_On_CheckAccept		=	4,			// 检测接取
	ESQE_On_CheckComplete	=	5,			// 检测交
	ESQE_On_NPC_Talk		=	6,			// 和NPC对话
	ESQE_On_Init			=	7,			// 任务初始化
	ESQE_On_Dlg_Default		=	8,			// 服务器可控对话框缺省事件
	ESQE_On_Invest			=	9,			// 调查地物		
	ESQE_End				=	10,
};

//---------------------------------------------------------------------------------
// 怪物脚本事件枚举
//---------------------------------------------------------------------------------
enum EScriptCreatureEvent
{
	// 日常事件
	ESCAE_On_Load			=	0,			// 载入
	ESCAE_On_Respawn		=	1,			// 重生
	ESCAE_On_Enter_Combat	=	2,			// 进入战斗
	ESCAE_On_Leave_Combat	=	3,			// 离开战斗
	ESCAE_On_Die			=	4,			// 死亡
	ESCAE_On_Timer			=	5,			// 定时触发
	ESCAE_On_Invest			=	6,			// 被调查
	ESCAE_On_Talk			=	7,			// 被说话
	// AI事件
	ESCAE_On_UpdateAI		=	8,			// 更新AI状态机
	ESCAE_On_UpdateCurAI	=	9,			// 更新当前AI状态
	ESCAE_On_EnterCurAI		=	10,			// 进入当前AI状态
	ESCAE_On_LeaveCurAI		=	11,			// 离开当前AI状态
	ESCAE_On_EventCurAI		=	12,			// 当前AI状态事件触发

	// 新增怪物脚本事件
	ESCAE_On_ReachPoint		=   13,			// 怪物达到某个导航点
	ESCAE_On_ReachEndPath	=   14,			// 怪物路径结束 //zhaopeng
	ESCAE_On_RangeEvent		=	15,			// 范围事件
	ESCAE_On_BeHelp			=   16,			// 被呼救
	ESCAE_On_BeAttack		=	17,			// 被攻击

	ESCAE_End
};

//---------------------------------------------------------------------------------
// 玩家脚本
//---------------------------------------------------------------------------------
enum EScriptRoleEvent
{
	ESRE_On_Online			=	0,			// 上线
	ESRE_On_FirstOnline		=	1,			// 第一次上线
	ESRE_On_IntoWorld		=	2,			// 进入游戏世界
	ESRE_On_FirstIntoWorld	=	3,			// 第一次进入游戏世界
	ESRE_On_EnterMap		=	4,			// 进入地图
	ESRE_IsDeadPenalty		=   5,			// 是否对玩家进行死亡惩罚
	ESRE_On_LevelChange		=	6,			// 等级提升
	ESRE_On_OpenChest		=	7,			// 开启宝箱
	ESRE_On_StopChest		=	8,			// 停止开启，产生随机物品
	ESRE_On_AgainChest		=	9,			// 再开一次
	ESRE_On_GetItem			=	10,			// 得到宝箱物品
	ESRE_On_Equip			=	11,			// 穿装备时
	ESRE_On_UnEquip			=	12,			// 拖装备时
	ESRE_On_RangeEvent		=	13,			// 执行范围事件
	ESRE_On_AddFriend		=   14,			// 加好友
	ESRE_On_CalFriendExp	=   15,			// 计算好友经验加成
	ESRE_On_WeaponLevelUp	=	16,			// 武器熔炼
	ESRE_On_WeapFusion		=	17,			// 武器提取
	ESRE_On_EquipConsolidate =	18,			// 武器强化
	ESRE_On_MakeMaster		=	19,			// 拜师收徒
	ESRE_On_OpenRank		=	20,			// 打开排行榜
	ESRE_On_PutOutXSQuest	=	21,			// 发布悬赏任务
	ESRE_On_JoinGuild		=	22,			// 加入帮会
	ESRE_On_CreateGuild		=	23,			// 创建帮会
	ESRE_On_LearnSkill		=	24,			// 学习技能
	ESRE_On_RideConsolidate =	25,			// 坐骑强化
	ESRE_On_ProduceEquip	=	26,			// 装备打造
	ESRE_On_VigourReward	=	27,			// 元气值奖励
	ESRE_On_DeCompose		=	28,			// 分解
	ESRE_On_JoinRecruit		=	29,			// 登入帮会招募榜
	ESRE_On_JoinMasterRecruit		=	30,			// 登入拜师榜
	ESRE_On_ProdueceItem	=	31,			// 炼丹
	ESRE_On_Guild_TrunOver	=	32,			// 移交帮主
	ESRE_On_InlayEquip			=	33,		// 装备镶嵌
	ESRE_On_RonghePet			= 34,		// 武宠合体
	ESRE_On_BindEquip			=	35,		// 绑定装备
	ESRE_On_SetRebornMap	= 36,		// 设置复活点
	ESRE_On_ReceiveAccountReward	=	37,		// 领取账号奖励
	ESRE_On_ReceiveAccountRewardEx	=	38,		// 领取账号奖励扩充
	ESRE_On_EquipDestroy	= 39,			// 摧毁装备
	ESRE_On_SayGoodbyeToMaster = 40, // 出师
	ESRE_On_MasterPrenticeBreak = 41,  //解除师徒关系
	ESRE_On_Get1v1Award		=	42, // 1v1领奖

	ESRE_On_LeavePractice = 43,		// 离线修炼
	ESME_ChangeMap			=   44,			// 玩家切换地图
	ESME_LeaveTimeReward	=   45,		// 离线时间奖励
	ESME_OnLineCompensate	=	46,		// 上线赔偿
	ESME_ActiveReceive		=	47,		// 领取活跃度奖励
	ESME_EquipXiLi			=	48,		// 装备洗礼
	ESME_JoinTeam			=	49,		// 加入队伍
	ESME_PetFeed			=	50,		// 宠物喂食
	ESME_ChatWorld			=	51,		// 世界喊话
	ESME_Fishing			=	52,		// 钓鱼
	ESME_On_ReceiveSerialReward	= 53,	// 领取序列号礼包
	ESRE_On_getwuhuen		=	54,		// 领悟武魂
	ESRE_On_Hang		=	55,		// 挂机
	ESME_On_FastCheck		=	56,		// 反加速
	ESME_On_TouPlant		=	57,		// 偷菜
	ESME_On_JuanMate		=	58,		// 捐材料
	ESME_On_JuanMonery		=	59,		// 捐资金
	ESME_GuildActiveReceive	=	60,		// 领取帮会活跃度奖励
	ESME_RoleConsumeYuanbao	=	61,		// 预报消耗
	ESME_RoleConsumeReward	=	62,		// 领取消费奖励
	ESME_RoleShowConsumeUI	=	63,
	ESME_Pet_Skill_Change	=	64,		// 宠物技能激活/取消
	ESME_sign				=	65,		// 签到
	ESRE_On_LearnGodSkill	=	66,		// 学习神级技能
	ESRE_On_lianhuen		=	67,		// 炼魂
	ESRE_On_shuangxiu		=	68,		// 双修
	ESRE_On_songhua			=	69,		// 送花
	ESRE_On_shangxiang		=	70,		// 帮会上香
	ESRE_On_Lottery			=	71,		// 抽奖
	ESME_ActiveDone			=	72,		// 一键完成活跃度
	ESME_On_Instance_SaoDang=	73,		// 副本扫荡完成
	ESRE_On_SBK_Reward		=	74,		// 领取沙巴克奖励
	ESRE_On_Recharge		=	75,		// 充值
	ESRE_On_DailyAct_Transmit = 76,		//每日活动一键传送
	ESRE_On_Get_Open_Receive =  77,		// 领取开服活动奖励
	ESRE_On_Get_Battle_Gift	 =  78,		// 领取战场奖励
	ESRE_End,
};

//---------------------------------------------------------------------------------
// 地图脚本
//---------------------------------------------------------------------------------
enum EScriptMapEvent
{
	ESME_OnInit					=	0,			// 初始化时
	ESME_OnTimer				=	1,			// 定时更新
	ESME_OnPlayerEnter			=	2,			// 玩家进入
	ESME_OnPlayerLeave			=	3,			// 玩家离开
	ESME_OnCreatureDie			=	4,			// 生物死亡
	ESME_OnRoleDie				=	5,			// 玩家死亡
	ESME_OnEnterTrigger			=	6,			// 进入触发器
	ESME_OnEnterArea			=	7,			// 进入区域
	ESME_CanInviteJoinTeam		=	8,			// 是否允许邀请组队
	ESME_CanLeaveTeam			=	9,			// 是否允许离开队伍
	ESME_CanChangeLeader		=	10,			// 是否能移交队长
	ESME_On_Revive				=	11,			// 玩家复活	
	ESME_CanEnterWhenOnline		=   12,			// 玩家上线时是否能计入地图
	ESME_GetExportMapAndCoord	=	13,			// 得到玩家离开当前地图后的地图ＩＤ和坐标
	ESME_GetOnePerfectMap		=   14,			// 找到最佳的副本实例
	ESME_CanEnter				=	15,			// 玩家是否能进入该地图
	ESME_FriendEnemy			=	16,			// 两个对象间的敌我关系
	ESME_CanKickMember			=	17,			// 是否允许踢掉队友
	ESME_OnCreatureDisappear	=	18,			// 生物消失
	ESME_Safeguard				=	19,			// 是否允许玩家开启保护模式
	ESME_CanUseItem				=	20,			// 是否允许使用物品
	ESME_CanUseSkill			=	21,			// 是否允许使用技能
	ESME_Clock					=	22,			// 刷新战功商店
	ESME_GuildWarRelay			=	23,			// 帮会战准备开始
	ESME_GuildWarStart			=	24,			// 帮会战开始
	ESME_GuildWarEnd			=	25,			// 帮会战结束
	ESME_End					
};

//---------------------------------------------------------------------------------
// 固定活动脚本
//---------------------------------------------------------------------------------
enum EScriptActEvent
{
	ESAE_OnInit				=	0,			// 初始化时
	ESAE_OnTimer			=	1,			// 定时更新
	ESAE_OnStart			=	2,			// 活动开始
	ESAE_OnEnd				=	3,			// 活动结束
	ESAE_OnTimerMin			=	4,			// 活动每分钟更新
	ESAE_OnDefaultRequest	=	5,			// 客户端触发服务器脚本的缺省消息 
	ESAE_Broad				=	6,			// 活动广播
	ESAE_End				=	7,
};

//---------------------------------------------------------------------------------
// 游戏世界事件脚本
//---------------------------------------------------------------------------------
enum EScriptWorldEvent
{
	ESWE_Adventure			=	0,			// 奇遇产生
	ESWE_Guild_Init_Ok		=	1,			// 帮会信息初始化完成
	ESWE_End				=	2,
};

//---------------------------------------------------------------------------------
// 物品事件脚本
//---------------------------------------------------------------------------------
enum EScriptItemEvent
{
	ESIE_CanUse				=	0,			// 物品是否可用
	ESIE_Use				=	1,			// 物品使用
	ESIE_Del				=   2,			// 删除时限物品
	ESIE_End
};

//---------------------------------------------------------------------------------
// 技能事件脚本
//---------------------------------------------------------------------------------
enum EScriptSkillEvent
{
	ESSE_CanCast			=	0,			// 技能是否可用
	ESSE_Cast				=	1,			// 技能使用	
	ESSE_CalDmg				=	2,			// 技能伤害
	ESSE_KillUnit			=	3,			// 击杀目标
	ESSE_Preparing			=	4,			// 开始吟唱
	ESSE_End
};
//---------------------------------------------------------------------------------
// buff事件脚本
//---------------------------------------------------------------------------------
enum EScriptBuffEvent
{
	ESBE_CalDmg				=	0,			// buff伤害计算
	ESBE_Init				=	1,			// buff初始化时
	ESBE_Trigger			=	2,			// buff效果触发时
	ESBE_Destory			=	3,			// buff结束时
	ESBE_End
};

//排行榜事件脚本
enum EScriptRankEvent
{
	ESRKE_InitRoleLevel		=	0,			// 角色等级榜初始化
	ESRKE_Shihun			=	1,			// 噬魂发奖
	ESRKE_End							
};

//---------------------------------------------------------------------------------
// 通用脚本类
//---------------------------------------------------------------------------------
template<INT nSize>
class Script
{
public:
	VOID RegisterFunc(INT nIndex, const CHAR* szFunc);
	VOID Destroy();

protected:
	Script();
	~Script();

protected:
	CHAR*	m_szFunc[nSize];		// 脚本函数字符串
};

template<INT nSize>
inline Script<nSize>::Script()
{
	ZeroMemory(m_szFunc, sizeof(m_szFunc));
}

template<INT nSize>
inline Script<nSize>::~Script()
{
	Destroy();
}

template<INT nSize>
inline VOID Script<nSize>::RegisterFunc(INT nIndex, const CHAR* szFunc)
{
	if( nIndex < 0  || nIndex >= nSize ) return;
	if( !VALID_POINT(szFunc) ) return;

	if( VALID_POINT(m_szFunc[nIndex]) )
	{
		free(m_szFunc[nIndex]);
	}

	m_szFunc[nIndex] = _strdup(szFunc);
}

template<INT nSize>
inline VOID Script<nSize>::Destroy()
{
	for(INT n = 0; n < nSize; ++n)
	{
		if( VALID_POINT(m_szFunc[n]) )
		{
			free(m_szFunc[n]);
			m_szFunc[n] = NULL;
		}
	}
}

//---------------------------------------------------------------------------------
// 任务脚本
//---------------------------------------------------------------------------------
class quest_script : public Script<ESQE_End>
{
	friend class ScriptMgr;
private:
	~quest_script() {}
public:
	INT  check_accept(UINT16 quest_id, Role* p_role, Creature* p_npc, INT& tErrorCode) const;
	INT  check_complete(UINT16 quest_id, Role* p_role, Creature* p_npc, INT& tErrorCode) const;

	VOID on_init(UINT16 quest_id, Role* p_role) const;
	VOID on_accept(UINT16 quest_id, Role* p_role, Creature* p_npc) const;
	VOID on_complete(UINT16 quest_id, Role* p_role, Creature* p_npc) const;
	VOID on_cancel(UINT16 quest_id, Role* p_role) const;
	VOID on_creature_kill(UINT16 quest_id, Role* p_role, DWORD dw_creature_type, BOOL bAddExp) const;
	VOID on_npc_talk(UINT16 quest_id, Role* p_role, DWORD dw_npc, DWORD dw_npc_type) const;
	VOID on_default_dialog(UINT16 quest_id, Role* p_role, DWORD option) const;
	VOID on_invest(UINT16 quest_id, Role* p_role, DWORD dw_creature_type) const;
};

//----------------------------------------------------------------------------------
// 怪物AI脚本
//----------------------------------------------------------------------------------
class CreatureScript : public Script<ESCAE_End>
{
	friend class ScriptMgr;
private:
	~CreatureScript() {}
public:
	VOID OnLoad(Creature* pCreature) const;
	VOID OnTalk(Creature* pCreature, Role* pRole, INT nIndex=-1) const;
	VOID OnRespawn(Creature* pCreature) const;
	VOID OnEnterCombat(Creature* pCreature) const;
	VOID OnLeaveCombat(Creature* pCreature) const;
	VOID OnDie(Creature* pCreature, Unit* pKiller, DWORD dwTaggedOwner) const;
	VOID OnInvest(Creature* pCreature, Role* pScr) const;
	VOID OnRangeEvent(Creature* pCreature, DWORD dwEventType) const;
	VOID OnBeHelp(Creature* pSrc, Creature* pHelp) const;

	VOID OnUpdateAI(Creature* pCreature) const;
	VOID OnUpdateCurAI(Creature* pCreature) const;
	VOID OnEnterCurAI(Creature* pCreature) const;
	VOID OnLeaveCurAI(Creature* pCreature) const;
	BOOL OnEventCurAI(Creature* pCreature) const;
	BOOL OnArrivalPoint(Creature* pCreature, int nNode ) const;
	VOID OnReachEndPath(Creature* pCreature) const;
	INT OnBeAttack(Creature* pCreature, DWORD dwSkillID, DWORD dwSkillLevel, BOOL bBlock, BOOL bCrited, INT nDmg) const;
};

//-------------------------------------------------------------------------------------
// 玩家脚本
//-------------------------------------------------------------------------------------
class RoleScript : public Script<ESRE_End>
{
	friend class ScriptMgr;
private:
	~RoleScript() {}
public:
	VOID OnRoleOnline(Role* pRole) const;
	VOID OnRoleFirstOnline(Role* pRole) const;
	VOID OnRoleIntoWorld(Role* pRole) const;
	VOID OnRoleFirstIntoWorld(Role* pRole) const;
	VOID OnRoleEnterMap(Role* pRole) const;
	INT  IsDeadPenalty(Role* pRole) const;
	VOID OnRoleLevelChange(Role* pRole) const;
	VOID OnOpenChest(Role* pRole, DWORD dwChestID, DWORD dwKeyID) const;
	VOID OnStopChest(Role* pRole, DWORD dwChestID, DWORD dwKeyID) const;
	VOID OnAgainChest(Role* pRole) const;
	VOID OnGetItem(Role* pRole, DWORD dwItemID, INT n_num) const;
	VOID OnEquip(Role* pRole, INT nPos, DWORD dw_data_id) const;
	VOID OnUnEquip(Role* pRole, INT nPos, DWORD dw_data_id) const;
	VOID OnRangeEvent(Role* pRole, DWORD dwEventType) const;
	VOID OnAddFriend( Role* pRole, DWORD dwFriendID ) const;
	FLOAT OnCalFriendExp(Role* pRole, DWORD dwFriendVal) const;
	VOID OnWeaponLevelUp( Role* pRole, DWORD dwLevel ) const;
	VOID OnWeaponFusion( Role* pRole ) const;
	VOID OnEquipConsolidate( Role* pRole, DWORD dwLevel ) const;
	VOID OnEquipDestroy( Role* pRole) const;
	VOID OnMakeMaster( Role* pRole, DWORD dwMaster, DWORD dwPrentice ) const;
	VOID OnOpenRank( Role* pRole ) const ;
	VOID OnPutOutXSQuest( Role* pRole, DWORD dwQuestID ) const ;
	VOID OnJoinGuild( Role* pRole, DWORD dwGuildID ) const;
	VOID OnCreateGuild( Role* pRole, DWORD dwGuildID ) const;
	VOID OnLearnSkill( Role* pRole, DWORD dwSkillID, INT nLevel ) const;
	VOID OnRideConsolidate( Role* pRole, INT nLevel ) const;
	VOID OnProduceEquip(Role* pRole, DWORD dwTypeID, DWORD dwQuality, BOOL bBox) const;
	VOID OnProduceItem(Role* pRole, DWORD dwTypeID, BOOL bSecc, BOOL bBox) const;
	VOID OnDeComposeItem(Role* pRole, DWORD dwTypeID) const;
	BOOL OnVigourReward(Role* pRole, DWORD dwVigourCost, DWORD dwFirstLogin) const;
	VOID OnJoinRecruit(Role* pRole) const;
	VOID OnJoinMasterRecruit(Role* pRole) const;
	VOID OnGuildTrunOver(Role* pRole, Role* pNewRole) const;
	VOID OnInlayEquip(Role* pRole) const;
	VOID OnBindEquip(Role* pRole) const;
	VOID OnRonghePet(Role* pRole, DWORD dwQuality, BOOL bAdd) const;
	VOID OnSetRebornMap(Role* pRole, DWORD dwMapID, DWORD dwInstanceID) const;
	VOID OnReceiveAccountReward(Role* pRole, DWORD dwMapID, DWORD dwInstanceID, INT16 n16ReceiveType) const;
	VOID OnReceiveAccountRewardEx(Role* pRole, DWORD dwMapID, DWORD dwInstanceID, DWORD dwReceiveType) const;
	VOID OnSayGoodbyeToMaster(Role* pRole, DWORD dwMasterID, BOOL bGo) const;
	VOID OnMasterPrenticeBreak(Role* pRole, DWORD dwMasterID, DWORD dwPrenticeID) const;
	VOID OnGet1v1Award(Role* pRole) const;
	VOID OnLeavePractice(Role* pRole, DWORD dwTime, DWORD dwLove, DWORD dwType) const;
	VOID OnChangeMap(Role* pRole, DWORD	dw_src_map_id,	DWORD dw_des_map_id, DWORD dw_instance_id) const;
	VOID OnLeaveTimeReward(Role* pRole) const;
	VOID OnLineCompensate(Role* pRole, DWORD nYear,	DWORD nMonth, DWORD nDay) const;
	VOID OnReceiveSerialReward(Role* pRole, INT n_type, DWORD& dw_result) const;
	DWORD OnActiveReceive(Role* pRole, INT nIndex) const;
	DWORD OnActiveDone(Role* pRole, INT nIndex,INT nbeishu) const;
	VOID OnDailyActTransmit(Role* pRole,INT nIndex) const;
	DWORD OnGuildActiveReceive(Role* pRole, INT nIndex) const;
	VOID OnEquipXiLi(Role* pRole) const;
	VOID OnJoinTeam(Role* pRole) const;
	VOID OnPetFeed(Role* pRole) const;
	VOID OnChatWorld(Role* pRole) const;
	VOID OnFishing(Role* pRole) const;
	VOID OnGetWuhuen(Role* pRole) const;
	VOID OnConsumeYuanBao(Role* pRole, INT sum, DWORD dwCmdid) const;
	VOID OnConsumeReward(Role *pRole, INT index) const;
	VOID ShowConsumeUI(Role *pRole) const;
	/**/
	VOID OnHang(Role* pRole, int type) const;
	VOID OnFastCheck(Role* pRole, INT nType) const;
	VOID OnTouPlant(Role* pRole) const;
	VOID OnJuanMate(Role* pRole, INT nNum) const;
	VOID OnJuanMonery(Role* pRole, INT nNum) const;
	VOID OnPetSkillChange(Role* pRole, DWORD dwSkillID, BOOL bActive) const;
	VOID OnSign(Role* pRole) const;
	VOID OnLearnGodSkill( Role* pRole, DWORD dwSkillID) const;
	VOID OnLianhuen(Role* pRole) const;
	VOID OnSonghua(Role* pRole,INT16 nSendFlowers) const;
	VOID OnShuangxiu(Role* pRole) const;
	VOID OnShangxiang(Role* pRole) const;
	VOID OnLottery(Role* pRole) const;
	VOID OnInstanceSaodang(Role* pRole, int nIndex) const;
	VOID OnSBKLottery(Role* pRole, int nPos) const;
	VOID OnRecharge(Role* pRole, int Recharge, int nTotleRecharge) const;
	VOID OnGetOpenReceive(Role* pRole, int nType, int bFirst) const;
	VOID OnGetBattleGift(Role* pRole, BOOL bWin, int nRank) const;
};

//-------------------------------------------------------------------------------------
// 地图脚本
//-------------------------------------------------------------------------------------
class MapScript : public Script<ESME_End>
{
	friend class ScriptMgr;
private:
	~MapScript() {}
public:
	VOID	OnInit(Map* pMap) const;
	VOID	OnTimer(Map* pMap, INT nMilliSec) const;
	VOID	OnPlayerEnter(Role* pRole, Map* pMap) const;
	VOID	OnPlayerLeave(Role* pRole, Map* pMap) const;
	VOID	OnCreatureDie(Creature* pCreature, Unit* pKiller, Map* pMap) const;
	VOID	OnRoleDie(Role* pRole, Unit* pKiller, Map* pMap) const;
	VOID	Revive(Role* pRole, ERoleReviveType eType, INT &nReviveHP, INT &nReviveMP, FLOAT &fx, FLOAT &fy, FLOAT &fz, DWORD &dwRebornMapID) const;
	VOID	OnEnterTrigger(Role* pRole, tag_map_trigger_info* pTrigger, Map* pMap) const;
	VOID	OnEnterArea(Role* pRole, tag_map_area_info* pArea, Map* pMap) const;
	INT		can_invite_join_team(Map* pMap) const;
	INT		can_leave_team(Map* pMap) const;
	INT		can_change_leader(Map* pMap) const;
	INT		can_kick_member(Map* pMap) const;
	DWORD	FriendEnemy(Map* pMap, Unit* pSrc, Unit* pTarget, BOOL& bIgnore) const;
	VOID	OnCreatureDisappear(Creature* pCreature, Map* pMap) const;
	INT		can_set_safe_guard(Map* pMap) const;
	BOOL	can_use_skill(Map* pMap, DWORD dw_data_id) const;
	BOOL	can_use_item(Map* pMap, DWORD dw_data_id, INT64 n64_Item_Serial) const;
	VOID	onclock(Map* pMap, INT nClock) const;

	// 脚本创建的副本独有接口
	INT		GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut) const;
	VOID	CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut) const;
	VOID	GetOnePerfectMap(Role* pRole, DWORD &dwInstanceID) const;
	INT		CanEnter(Role* pRole) const;

	// 帮会战
	VOID	GuildWarRelay(Map* pMap) const;
	VOID	GuildWarStart(Map* pMap, DWORD dwdefentNumber, DWORD dwAttackNum, DWORD dwDefentID, DWORD dwAttackID) const;
	VOID	GuildWarEnd(Map* pMap) const;
};

//-------------------------------------------------------------------------------------
// 活动脚本
//-------------------------------------------------------------------------------------
class ActScript : public Script<ESAE_End>
{
	friend class ScriptMgr;
private:
	~ActScript() {}
public:
	VOID OnInit(DWORD dwActID) const;
	VOID OnTimer(DWORD dwActID, INT nSec) const;
	VOID OnTimerMin(DWORD dwActID) const;
	VOID OnActStart(DWORD dwActID) const;
	VOID on_act_end(DWORD dwActID) const;
	VOID OnDefaultRequest(DWORD dwActID, Role* pRole, DWORD	dwEventType) const;
	VOID BroadActivityState(DWORD dwActID, INT nState) const;
};

//-------------------------------------------------------------------------------------
// 物品脚本
//-------------------------------------------------------------------------------------
class ItemScript : public Script<ESIE_End>
{
	friend class ScriptMgr;
private:
	~ItemScript() {}
public:
	BOOL can_use_item(Map* pMap, DWORD dw_data_id, DWORD dwSrcID, DWORD dwTargetID, BOOL &bIgnore, INT64 n64_Item_Serial) const;
	BOOL UseItem(Map* pMap, DWORD dw_data_id, DWORD dwSrcID, DWORD dwTargetID, INT64 n64_Item_Serial) const;
	VOID DelItem(Map* pMap, DWORD dw_role_id, DWORD dw_data_id) const;
};

//-------------------------------------------------------------------------------------
// 技能脚本
//-------------------------------------------------------------------------------------
class SkillScript : public Script<ESSE_End>
{
	friend class ScriptMgr;
private:
	~SkillScript () {}
public:
	DWORD	CanCastSkill(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDstUnitID) const;
	VOID	CastSkill(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, BOOL &bIgnore) const;
	FLOAT	CalculateDmg(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDestUnitID, DWORD bblock, DWORD bcrit, FLOAT& fDmg) const;
	VOID	KillUnit(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDestUnitID) const;
	VOID	PreparingSkill(Map* pMap, DWORD dwSkillID, DWORD dwSkillLevel, DWORD dwOwnerID, DWORD dwDstUnitID) const;
};
//-------------------------------------------------------------------------------------
// buff脚本
//-------------------------------------------------------------------------------------
class BuffScript : public Script<ESBE_End>
{
	friend class ScriptMgr;
private:
	~BuffScript() {}
public:
	INT	CalculateDmg(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwDestUnitID, INT& fDmg) const;
	INT	OnInit(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwSrcID) const;
	INT	OnTrigger(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwSrcID) const;
	INT OnDestory(Map* pMap, DWORD dwBuffID, DWORD dwLevel, DWORD dwOwnerID, DWORD dwSrcID) const;
};
//-------------------------------------------------------------------------------------
// 排行榜事件脚本
//-------------------------------------------------------------------------------------
class RankScript : public Script<ESBE_End>
{
	friend class ScriptMgr;
private:
	~RankScript() {}
public:
	VOID OnInitRoleLevel() const;
	VOID OnShihunGiveReward() const;
};

//-------------------------------------------------------------------------------------
// 游戏世界事件脚本
//-------------------------------------------------------------------------------------
class WorldScript : public Script<ESWE_End>
{
	friend class ScriptMgr;
private:
	~WorldScript() {}
public:
	VOID OnAdventure(Role* pRole) const;
	VOID OnGuildInitOk() const;
};

//-------------------------------------------------------------------------------------
// 脚本管理器
//-------------------------------------------------------------------------------------
class ScriptMgr : public EventMgr<ScriptMgr>
{
public:
	//---------------------------------------------------------------------------------
	// 初始化和销毁
	//---------------------------------------------------------------------------------
	BOOL Init();
	VOID Update();
	VOID Destroy();

	//---------------------------------------------------------------------------------
	// 注册脚本函数
	//---------------------------------------------------------------------------------
	VOID RegisterCreatureEvent(DWORD dwID, EScriptCreatureEvent eEvent, const CHAR* szFunction);
	VOID RegisterQuestEvent(UINT16 u16QuestID, EScriptQuestEvent eEvent, const CHAR* szFunction);
	VOID RegisterRoleEvent(EScriptRoleEvent eEvent, const CHAR* szFunction);
	VOID RegisterMapEvent(const CHAR* szMapName, EScriptMapEvent eEvent, const CHAR* szFunction);
	VOID RegisterActEvent(DWORD dwActID, EScriptActEvent eEvent, const CHAR* szFunction);
	VOID RegisterWorldEvent(EScriptWorldEvent eEvent, const CHAR* szFunction);
	VOID RegisterItemEvent(DWORD dw_data_id, EScriptItemEvent eEvent, const CHAR* szFunction);
	VOID RegisterSkillEvent(DWORD dw_data_id, EScriptSkillEvent eEvent, const CHAR* szFunction);	
	VOID RegisterBuffEvent(DWORD dw_data_id, EScriptBuffEvent eEvent, const CHAR* szFunction);
	VOID RegisterRankEvent(EScriptRankEvent eEvent, const CHAR* szFunction);
	//---------------------------------------------------------------------------------
	// 生成脚本对象
	//---------------------------------------------------------------------------------
	const CreatureScript*	GetCreatureScript(DWORD dwCreatureID)	{ return m_mapCreatureScript.find(dwCreatureID); }
	const quest_script*		GetQuestScript(UINT16 u16QuestID)		{ return m_mapQusetScript.find(u16QuestID); }
	const MapScript*		GetMapScript(DWORD dwMapID)				{ return m_mapMapScript.find(dwMapID); }
	const RoleScript*		GetRoleScript()							{ return m_pRoleScript; }
	const ActScript*		GetActScript(DWORD dwActID)				{ return m_mapActScript.find(dwActID); }
	const WorldScript*		GetWorldScript()						{ return m_pWorldScript; }
	const ItemScript*		GetItemScript(DWORD dw_data_id)			{ return m_mapItemScript.find(dw_data_id); }
	const SkillScript*		GetSkillScript(DWORD dw_data_id)			{ return m_mapSkillScript.find(dw_data_id); }
	const BuffScript*		GetBuffScript(DWORD dw_data_id)			{ return m_mapBuffScript.find(dw_data_id); }
	const RankScript*		GetRankScript()							{ return m_pRankScript; }
	//---------------------------------------------------------------------------------
	// 调用脚本
	//---------------------------------------------------------------------------------
	VOID	CallScriptFunction(CHAR* szFunction, CHAR* szFormat, ...);

	//---------------------------------------------------------------------------------
	// 生成脚本锁
	//---------------------------------------------------------------------------------
	DWORD	CreateScriptMutex();

	//---------------------------------------------------------------------------------
	// 锁住解除某个脚本锁
	//---------------------------------------------------------------------------------
	VOID	LockScriptMutex(DWORD dwMutexID);
	VOID	UnLockScriptMutex(DWORD dwMutexID);

	//---------------------------------------------------------------------------------
	// 获取全局变量
	//---------------------------------------------------------------------------------
	template<typename T>
	VOID	GetGlobal(LPCSTR strVarName, OUT T &res);
	
	// 获取lua使用内存
	DWORD	GetMemcory();
private:
	//---------------------------------------------------------------------------------
	// 注册异步事件函数
	//---------------------------------------------------------------------------------
	static VOID	RegisterScriptEventFunc();

	//---------------------------------------------------------------------------------
	// 异步事件处理函数
	//---------------------------------------------------------------------------------
	VOID	OnReload(DWORD dwSenderID, LPVOID pEventMessage);

	//---------------------------------------------------------------------------------
	// 初始化
	//---------------------------------------------------------------------------------
	VOID	LoadScripts();
	VOID	LoadScriptsFromDir(LPCTSTR szDir, std::vector<tstring>& luaFiles, BOOL bFirst=FALSE);
	VOID	RegisterCoreFunctions();

	//-----------------------------------------------------------------------------------
	// 销毁
	//-----------------------------------------------------------------------------------
	VOID	DestroyScripts();
	VOID	DestroyAllStates();
	VOID	UnloadScripts();

	//-----------------------------------------------------------------------------------
	// 重新加载脚本
	//-----------------------------------------------------------------------------------
	VOID	Reload();

	//-----------------------------------------------------------------------------------
	// 得到一个线程状态
	//-----------------------------------------------------------------------------------
	lua_State* GetThreadState();
	lua_State* CreateThreadState(DWORD dwThreadID);

	//-----------------------------------------------------------------------------------
	// 错误显示
	//-----------------------------------------------------------------------------------
	VOID ErrMsg(lua_State* pState);

private:
	typedef package_map<UINT16, quest_script*>			QuestScriptMap;
	typedef package_map<DWORD, CreatureScript*>		CreatureScriptMap;
	typedef package_map<DWORD, MapScript*>				MapScriptMap;
	typedef package_map<DWORD, ActScript*>				ActScriptMap;
	typedef package_map<DWORD, ItemScript*>			ItemScriptMap;
	typedef package_map<DWORD, SkillScript*>			SkillScriptMap;
	typedef package_map<DWORD, BuffScript*>			BuffScriptMap;
private:
	
	Mutex							m_Lock;					// 用于生成线程状态的锁

	lua_State*						m_pMasterState;			// 主状态
	package_safe_map<DWORD, lua_State*>		m_mapThreadState;		// 各个线程状态

	package_map<DWORD, Mutex*>				m_mapScriptMutex;		// 脚本锁，用于需要锁定运行的脚本函数
	DWORD							m_dwScriptMutexIDGen;	// 脚本锁ID生成器

	QuestScriptMap					m_mapQusetScript;		// 任务脚本表（任务ID---脚本）
	CreatureScriptMap				m_mapCreatureScript;	// 怪物脚本表（怪物ID——脚本）
	MapScriptMap					m_mapMapScript;			// 地图脚本表（地图属性ID——脚本）
	ActScriptMap					m_mapActScript;			// 活动脚本表（活动ID－－脚本）
	ItemScriptMap					m_mapItemScript;		// 物品脚本表（物品TypeID－－脚本)
	SkillScriptMap					m_mapSkillScript;		// 技能脚本
	BuffScriptMap					m_mapBuffScript;		// buff脚本

	WorldScript*					m_pWorldScript;			// 游戏世界脚本
	RoleScript*						m_pRoleScript;			// 玩家脚本
	RankScript*						m_pRankScript;			// 排行榜脚本
	DWORD							m_dwMaxPcallTime;		// 脚本调用的最长时间
};


//-----------------------------------------------------------------------------------
// 生成脚本锁
//-----------------------------------------------------------------------------------
inline DWORD ScriptMgr::CreateScriptMutex()
{
	Mutex* pMutex = new Mutex;

	++m_dwScriptMutexIDGen;
	m_mapScriptMutex.add(m_dwScriptMutexIDGen, pMutex);

	return m_dwScriptMutexIDGen;
}

//------------------------------------------------------------------------------------
// 锁住某个脚本锁
//------------------------------------------------------------------------------------
inline VOID ScriptMgr::LockScriptMutex(DWORD dwMutexID)
{
	Mutex* pMutex = m_mapScriptMutex.find(dwMutexID);

	if( VALID_POINT(pMutex) )
	{
		pMutex->Acquire();
	}
}

//-------------------------------------------------------------------------------------
// 解除某个脚本锁
//-------------------------------------------------------------------------------------
inline VOID ScriptMgr::UnLockScriptMutex(DWORD dwMutexID)
{
	Mutex* pMutex = m_mapScriptMutex.find(dwMutexID);

	if( VALID_POINT(pMutex) )
	{
		pMutex->Release();
	}
}

//---------------------------------------------------------------------------------
// 获取全局变量
//---------------------------------------------------------------------------------
template<typename INT>
VOID ScriptMgr::GetGlobal(LPCSTR strVarName, OUT INT &res)
{
	lua_State* pThreadState = GetThreadState();
	if( !VALID_POINT(pThreadState) ) return;

	m_Lock.Acquire();

	lua_getglobal(pThreadState, strVarName);
	res = luaL_checkint(pThreadState, 1);

	lua_pop(pThreadState, 1);

	m_Lock.Release();
}

extern ScriptMgr g_ScriptMgr;

