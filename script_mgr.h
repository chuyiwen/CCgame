
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//Lua�ű�������
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
// ����ű��¼�ö��
//-------------------------------------------------------------------------------
enum EScriptQuestEvent
{
	ESQE_On_Accept			=	0,			// �����ȡ
	ESQE_On_Complete		=	1,			// �������
	ESQE_On_Cancel			=	2,			// ����ȡ��
	ESQE_On_Creature_Kill	=	3,			// ɱ��һֻ����
	ESQE_On_CheckAccept		=	4,			// ����ȡ
	ESQE_On_CheckComplete	=	5,			// ��⽻
	ESQE_On_NPC_Talk		=	6,			// ��NPC�Ի�
	ESQE_On_Init			=	7,			// �����ʼ��
	ESQE_On_Dlg_Default		=	8,			// �������ɿضԻ���ȱʡ�¼�
	ESQE_On_Invest			=	9,			// �������		
	ESQE_End				=	10,
};

//---------------------------------------------------------------------------------
// ����ű��¼�ö��
//---------------------------------------------------------------------------------
enum EScriptCreatureEvent
{
	// �ճ��¼�
	ESCAE_On_Load			=	0,			// ����
	ESCAE_On_Respawn		=	1,			// ����
	ESCAE_On_Enter_Combat	=	2,			// ����ս��
	ESCAE_On_Leave_Combat	=	3,			// �뿪ս��
	ESCAE_On_Die			=	4,			// ����
	ESCAE_On_Timer			=	5,			// ��ʱ����
	ESCAE_On_Invest			=	6,			// ������
	ESCAE_On_Talk			=	7,			// ��˵��
	// AI�¼�
	ESCAE_On_UpdateAI		=	8,			// ����AI״̬��
	ESCAE_On_UpdateCurAI	=	9,			// ���µ�ǰAI״̬
	ESCAE_On_EnterCurAI		=	10,			// ���뵱ǰAI״̬
	ESCAE_On_LeaveCurAI		=	11,			// �뿪��ǰAI״̬
	ESCAE_On_EventCurAI		=	12,			// ��ǰAI״̬�¼�����

	// ��������ű��¼�
	ESCAE_On_ReachPoint		=   13,			// ����ﵽĳ��������
	ESCAE_On_ReachEndPath	=   14,			// ����·������ //zhaopeng
	ESCAE_On_RangeEvent		=	15,			// ��Χ�¼�
	ESCAE_On_BeHelp			=   16,			// ������
	ESCAE_On_BeAttack		=	17,			// ������

	ESCAE_End
};

//---------------------------------------------------------------------------------
// ��ҽű�
//---------------------------------------------------------------------------------
enum EScriptRoleEvent
{
	ESRE_On_Online			=	0,			// ����
	ESRE_On_FirstOnline		=	1,			// ��һ������
	ESRE_On_IntoWorld		=	2,			// ������Ϸ����
	ESRE_On_FirstIntoWorld	=	3,			// ��һ�ν�����Ϸ����
	ESRE_On_EnterMap		=	4,			// �����ͼ
	ESRE_IsDeadPenalty		=   5,			// �Ƿ����ҽ��������ͷ�
	ESRE_On_LevelChange		=	6,			// �ȼ�����
	ESRE_On_OpenChest		=	7,			// ��������
	ESRE_On_StopChest		=	8,			// ֹͣ���������������Ʒ
	ESRE_On_AgainChest		=	9,			// �ٿ�һ��
	ESRE_On_GetItem			=	10,			// �õ�������Ʒ
	ESRE_On_Equip			=	11,			// ��װ��ʱ
	ESRE_On_UnEquip			=	12,			// ��װ��ʱ
	ESRE_On_RangeEvent		=	13,			// ִ�з�Χ�¼�
	ESRE_On_AddFriend		=   14,			// �Ӻ���
	ESRE_On_CalFriendExp	=   15,			// ������Ѿ���ӳ�
	ESRE_On_WeaponLevelUp	=	16,			// ��������
	ESRE_On_WeapFusion		=	17,			// ������ȡ
	ESRE_On_EquipConsolidate =	18,			// ����ǿ��
	ESRE_On_MakeMaster		=	19,			// ��ʦ��ͽ
	ESRE_On_OpenRank		=	20,			// �����а�
	ESRE_On_PutOutXSQuest	=	21,			// ������������
	ESRE_On_JoinGuild		=	22,			// ������
	ESRE_On_CreateGuild		=	23,			// �������
	ESRE_On_LearnSkill		=	24,			// ѧϰ����
	ESRE_On_RideConsolidate =	25,			// ����ǿ��
	ESRE_On_ProduceEquip	=	26,			// װ������
	ESRE_On_VigourReward	=	27,			// Ԫ��ֵ����
	ESRE_On_DeCompose		=	28,			// �ֽ�
	ESRE_On_JoinRecruit		=	29,			// ��������ļ��
	ESRE_On_JoinMasterRecruit		=	30,			// �����ʦ��
	ESRE_On_ProdueceItem	=	31,			// ����
	ESRE_On_Guild_TrunOver	=	32,			// �ƽ�����
	ESRE_On_InlayEquip			=	33,		// װ����Ƕ
	ESRE_On_RonghePet			= 34,		// ������
	ESRE_On_BindEquip			=	35,		// ��װ��
	ESRE_On_SetRebornMap	= 36,		// ���ø����
	ESRE_On_ReceiveAccountReward	=	37,		// ��ȡ�˺Ž���
	ESRE_On_ReceiveAccountRewardEx	=	38,		// ��ȡ�˺Ž�������
	ESRE_On_EquipDestroy	= 39,			// �ݻ�װ��
	ESRE_On_SayGoodbyeToMaster = 40, // ��ʦ
	ESRE_On_MasterPrenticeBreak = 41,  //���ʦͽ��ϵ
	ESRE_On_Get1v1Award		=	42, // 1v1�콱

	ESRE_On_LeavePractice = 43,		// ��������
	ESME_ChangeMap			=   44,			// ����л���ͼ
	ESME_LeaveTimeReward	=   45,		// ����ʱ�佱��
	ESME_OnLineCompensate	=	46,		// �����⳥
	ESME_ActiveReceive		=	47,		// ��ȡ��Ծ�Ƚ���
	ESME_EquipXiLi			=	48,		// װ��ϴ��
	ESME_JoinTeam			=	49,		// �������
	ESME_PetFeed			=	50,		// ����ιʳ
	ESME_ChatWorld			=	51,		// ���纰��
	ESME_Fishing			=	52,		// ����
	ESME_On_ReceiveSerialReward	= 53,	// ��ȡ���к����
	ESRE_On_getwuhuen		=	54,		// �������
	ESRE_On_Hang		=	55,		// �һ�
	ESME_On_FastCheck		=	56,		// ������
	ESME_On_TouPlant		=	57,		// ͵��
	ESME_On_JuanMate		=	58,		// �����
	ESME_On_JuanMonery		=	59,		// ���ʽ�
	ESME_GuildActiveReceive	=	60,		// ��ȡ����Ծ�Ƚ���
	ESME_RoleConsumeYuanbao	=	61,		// Ԥ������
	ESME_RoleConsumeReward	=	62,		// ��ȡ���ѽ���
	ESME_RoleShowConsumeUI	=	63,
	ESME_Pet_Skill_Change	=	64,		// ���＼�ܼ���/ȡ��
	ESME_sign				=	65,		// ǩ��
	ESRE_On_LearnGodSkill	=	66,		// ѧϰ�񼶼���
	ESRE_On_lianhuen		=	67,		// ����
	ESRE_On_shuangxiu		=	68,		// ˫��
	ESRE_On_songhua			=	69,		// �ͻ�
	ESRE_On_shangxiang		=	70,		// �������
	ESRE_On_Lottery			=	71,		// �齱
	ESME_ActiveDone			=	72,		// һ����ɻ�Ծ��
	ESME_On_Instance_SaoDang=	73,		// ����ɨ�����
	ESRE_On_SBK_Reward		=	74,		// ��ȡɳ�Ϳ˽���
	ESRE_On_Recharge		=	75,		// ��ֵ
	ESRE_On_DailyAct_Transmit = 76,		//ÿ�ջһ������
	ESRE_On_Get_Open_Receive =  77,		// ��ȡ���������
	ESRE_On_Get_Battle_Gift	 =  78,		// ��ȡս������
	ESRE_End,
};

//---------------------------------------------------------------------------------
// ��ͼ�ű�
//---------------------------------------------------------------------------------
enum EScriptMapEvent
{
	ESME_OnInit					=	0,			// ��ʼ��ʱ
	ESME_OnTimer				=	1,			// ��ʱ����
	ESME_OnPlayerEnter			=	2,			// ��ҽ���
	ESME_OnPlayerLeave			=	3,			// ����뿪
	ESME_OnCreatureDie			=	4,			// ��������
	ESME_OnRoleDie				=	5,			// �������
	ESME_OnEnterTrigger			=	6,			// ���봥����
	ESME_OnEnterArea			=	7,			// ��������
	ESME_CanInviteJoinTeam		=	8,			// �Ƿ������������
	ESME_CanLeaveTeam			=	9,			// �Ƿ������뿪����
	ESME_CanChangeLeader		=	10,			// �Ƿ����ƽ��ӳ�
	ESME_On_Revive				=	11,			// ��Ҹ���	
	ESME_CanEnterWhenOnline		=   12,			// �������ʱ�Ƿ��ܼ����ͼ
	ESME_GetExportMapAndCoord	=	13,			// �õ�����뿪��ǰ��ͼ��ĵ�ͼ�ɣĺ�����
	ESME_GetOnePerfectMap		=   14,			// �ҵ���ѵĸ���ʵ��
	ESME_CanEnter				=	15,			// ����Ƿ��ܽ���õ�ͼ
	ESME_FriendEnemy			=	16,			// ���������ĵ��ҹ�ϵ
	ESME_CanKickMember			=	17,			// �Ƿ������ߵ�����
	ESME_OnCreatureDisappear	=	18,			// ������ʧ
	ESME_Safeguard				=	19,			// �Ƿ�������ҿ�������ģʽ
	ESME_CanUseItem				=	20,			// �Ƿ�����ʹ����Ʒ
	ESME_CanUseSkill			=	21,			// �Ƿ�����ʹ�ü���
	ESME_Clock					=	22,			// ˢ��ս���̵�
	ESME_GuildWarRelay			=	23,			// ���ս׼����ʼ
	ESME_GuildWarStart			=	24,			// ���ս��ʼ
	ESME_GuildWarEnd			=	25,			// ���ս����
	ESME_End					
};

//---------------------------------------------------------------------------------
// �̶���ű�
//---------------------------------------------------------------------------------
enum EScriptActEvent
{
	ESAE_OnInit				=	0,			// ��ʼ��ʱ
	ESAE_OnTimer			=	1,			// ��ʱ����
	ESAE_OnStart			=	2,			// ���ʼ
	ESAE_OnEnd				=	3,			// �����
	ESAE_OnTimerMin			=	4,			// �ÿ���Ӹ���
	ESAE_OnDefaultRequest	=	5,			// �ͻ��˴����������ű���ȱʡ��Ϣ 
	ESAE_Broad				=	6,			// ��㲥
	ESAE_End				=	7,
};

//---------------------------------------------------------------------------------
// ��Ϸ�����¼��ű�
//---------------------------------------------------------------------------------
enum EScriptWorldEvent
{
	ESWE_Adventure			=	0,			// ��������
	ESWE_Guild_Init_Ok		=	1,			// �����Ϣ��ʼ�����
	ESWE_End				=	2,
};

//---------------------------------------------------------------------------------
// ��Ʒ�¼��ű�
//---------------------------------------------------------------------------------
enum EScriptItemEvent
{
	ESIE_CanUse				=	0,			// ��Ʒ�Ƿ����
	ESIE_Use				=	1,			// ��Ʒʹ��
	ESIE_Del				=   2,			// ɾ��ʱ����Ʒ
	ESIE_End
};

//---------------------------------------------------------------------------------
// �����¼��ű�
//---------------------------------------------------------------------------------
enum EScriptSkillEvent
{
	ESSE_CanCast			=	0,			// �����Ƿ����
	ESSE_Cast				=	1,			// ����ʹ��	
	ESSE_CalDmg				=	2,			// �����˺�
	ESSE_KillUnit			=	3,			// ��ɱĿ��
	ESSE_Preparing			=	4,			// ��ʼ����
	ESSE_End
};
//---------------------------------------------------------------------------------
// buff�¼��ű�
//---------------------------------------------------------------------------------
enum EScriptBuffEvent
{
	ESBE_CalDmg				=	0,			// buff�˺�����
	ESBE_Init				=	1,			// buff��ʼ��ʱ
	ESBE_Trigger			=	2,			// buffЧ������ʱ
	ESBE_Destory			=	3,			// buff����ʱ
	ESBE_End
};

//���а��¼��ű�
enum EScriptRankEvent
{
	ESRKE_InitRoleLevel		=	0,			// ��ɫ�ȼ����ʼ��
	ESRKE_Shihun			=	1,			// �ɻ귢��
	ESRKE_End							
};

//---------------------------------------------------------------------------------
// ͨ�ýű���
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
	CHAR*	m_szFunc[nSize];		// �ű������ַ���
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
// ����ű�
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
// ����AI�ű�
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
// ��ҽű�
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
// ��ͼ�ű�
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

	// �ű������ĸ������нӿ�
	INT		GetExportMapAndCoord(Role* pRole, DWORD& dwOutMapID, Vector3& vOut) const;
	VOID	CanTakeOverWhenOnline(Role* pRole, DWORD& dwOutMapID, Vector3& vOut) const;
	VOID	GetOnePerfectMap(Role* pRole, DWORD &dwInstanceID) const;
	INT		CanEnter(Role* pRole) const;

	// ���ս
	VOID	GuildWarRelay(Map* pMap) const;
	VOID	GuildWarStart(Map* pMap, DWORD dwdefentNumber, DWORD dwAttackNum, DWORD dwDefentID, DWORD dwAttackID) const;
	VOID	GuildWarEnd(Map* pMap) const;
};

//-------------------------------------------------------------------------------------
// ��ű�
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
// ��Ʒ�ű�
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
// ���ܽű�
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
// buff�ű�
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
// ���а��¼��ű�
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
// ��Ϸ�����¼��ű�
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
// �ű�������
//-------------------------------------------------------------------------------------
class ScriptMgr : public EventMgr<ScriptMgr>
{
public:
	//---------------------------------------------------------------------------------
	// ��ʼ��������
	//---------------------------------------------------------------------------------
	BOOL Init();
	VOID Update();
	VOID Destroy();

	//---------------------------------------------------------------------------------
	// ע��ű�����
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
	// ���ɽű�����
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
	// ���ýű�
	//---------------------------------------------------------------------------------
	VOID	CallScriptFunction(CHAR* szFunction, CHAR* szFormat, ...);

	//---------------------------------------------------------------------------------
	// ���ɽű���
	//---------------------------------------------------------------------------------
	DWORD	CreateScriptMutex();

	//---------------------------------------------------------------------------------
	// ��ס���ĳ���ű���
	//---------------------------------------------------------------------------------
	VOID	LockScriptMutex(DWORD dwMutexID);
	VOID	UnLockScriptMutex(DWORD dwMutexID);

	//---------------------------------------------------------------------------------
	// ��ȡȫ�ֱ���
	//---------------------------------------------------------------------------------
	template<typename T>
	VOID	GetGlobal(LPCSTR strVarName, OUT T &res);
	
	// ��ȡluaʹ���ڴ�
	DWORD	GetMemcory();
private:
	//---------------------------------------------------------------------------------
	// ע���첽�¼�����
	//---------------------------------------------------------------------------------
	static VOID	RegisterScriptEventFunc();

	//---------------------------------------------------------------------------------
	// �첽�¼�������
	//---------------------------------------------------------------------------------
	VOID	OnReload(DWORD dwSenderID, LPVOID pEventMessage);

	//---------------------------------------------------------------------------------
	// ��ʼ��
	//---------------------------------------------------------------------------------
	VOID	LoadScripts();
	VOID	LoadScriptsFromDir(LPCTSTR szDir, std::vector<tstring>& luaFiles, BOOL bFirst=FALSE);
	VOID	RegisterCoreFunctions();

	//-----------------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------------
	VOID	DestroyScripts();
	VOID	DestroyAllStates();
	VOID	UnloadScripts();

	//-----------------------------------------------------------------------------------
	// ���¼��ؽű�
	//-----------------------------------------------------------------------------------
	VOID	Reload();

	//-----------------------------------------------------------------------------------
	// �õ�һ���߳�״̬
	//-----------------------------------------------------------------------------------
	lua_State* GetThreadState();
	lua_State* CreateThreadState(DWORD dwThreadID);

	//-----------------------------------------------------------------------------------
	// ������ʾ
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
	
	Mutex							m_Lock;					// ���������߳�״̬����

	lua_State*						m_pMasterState;			// ��״̬
	package_safe_map<DWORD, lua_State*>		m_mapThreadState;		// �����߳�״̬

	package_map<DWORD, Mutex*>				m_mapScriptMutex;		// �ű�����������Ҫ�������еĽű�����
	DWORD							m_dwScriptMutexIDGen;	// �ű���ID������

	QuestScriptMap					m_mapQusetScript;		// ����ű�������ID---�ű���
	CreatureScriptMap				m_mapCreatureScript;	// ����ű�������ID�����ű���
	MapScriptMap					m_mapMapScript;			// ��ͼ�ű�����ͼ����ID�����ű���
	ActScriptMap					m_mapActScript;			// ��ű����ID�����ű���
	ItemScriptMap					m_mapItemScript;		// ��Ʒ�ű�����ƷTypeID�����ű�)
	SkillScriptMap					m_mapSkillScript;		// ���ܽű�
	BuffScriptMap					m_mapBuffScript;		// buff�ű�

	WorldScript*					m_pWorldScript;			// ��Ϸ����ű�
	RoleScript*						m_pRoleScript;			// ��ҽű�
	RankScript*						m_pRankScript;			// ���а�ű�
	DWORD							m_dwMaxPcallTime;		// �ű����õ��ʱ��
};


//-----------------------------------------------------------------------------------
// ���ɽű���
//-----------------------------------------------------------------------------------
inline DWORD ScriptMgr::CreateScriptMutex()
{
	Mutex* pMutex = new Mutex;

	++m_dwScriptMutexIDGen;
	m_mapScriptMutex.add(m_dwScriptMutexIDGen, pMutex);

	return m_dwScriptMutexIDGen;
}

//------------------------------------------------------------------------------------
// ��סĳ���ű���
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
// ���ĳ���ű���
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
// ��ȡȫ�ֱ���
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

