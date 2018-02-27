
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//GM 命令
#pragma once

class Role;
//-----------------------------------------------------------------------------
class GMCommandMgr
{
	typedef DWORD (GMCommandMgr::*GMCMDHANDLE)(const std::vector<DWORD>&, Role*);

public:
	GMCommandMgr();
	~GMCommandMgr();

public:
	DWORD Excute(LPCTSTR szCommand, Role *pRole);
	VOID RegisterAll();
	VOID UnRegisterAll();

public:
	VOID LogGMCmd(DWORD dw_role_id, LPCTSTR szCmd, DWORD dw_error_code);
//public:
//	VOID SetGMCmdID(DWORD dw_cmd_id) { if(INVALID_VALUE == m_dwGMCmdID) { m_dwGMCmdID = dw_cmd_id;} }

private:
	VOID Register(LPCTSTR szName, GMCMDHANDLE pFun, BYTE byPrivilege, LPCTSTR szDesc=NULL, BYTE byParamNum=1);

private: // GM 命令处理函数
	// GM自身
	DWORD HandleCreateItem(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetSilver(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandGetBindSilver(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetYuanBao(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetExVolume(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleClearBag(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGoto(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGotoRole(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGotoRoleInst(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleFillExp(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleFillLevel(const std::vector<DWORD>& vectParam, Role* pGM);
	//DWORD HandleEquipPotInc(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleAddQuest(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleClearSkillCD(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleCoolOff(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleCoolOn(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleChangeRoleAtt(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleChangSpeed(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleSetReputation(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleRoleQuest(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleClearRoleQuest(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleAddBuff(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleSetVocation(const std::vector<DWORD>& vectParam, Role* pGM);

	// 对玩家
	DWORD HandleMoveRole(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleKickRole(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleKickRoleID(const std::vector<DWORD>& vectParam, Role* pGM);

	// 脚本
	DWORD HandleReloadScripts(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleChangeScriptData(const std::vector<DWORD>& vectParam, Role* pGM);

	// 游戏世界
	DWORD HandleDouble(const std::vector<DWORD>& vectParam, Role* pGM);

	// 调整可登陆玩家人数
	DWORD HandleResizeOnlineNum(const std::vector<DWORD>& vectParam, Role* pGM);

	// 测试氏族数据用
	DWORD HandleClanData(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleVipNetBar(const std::vector<DWORD>& vectParam, Role* pGM);

	// 测试宠物
	DWORD HandlePet(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleAddSkill(const std::vector<DWORD>& vectParam, Role* pGM);

	// 帮派测试
	DWORD HandleGetFund(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetMaterial(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetContribute(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleResetAffairTimes(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetTael(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGuildStatus(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGuildFacility(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleRoleGuild(const std::vector<DWORD>& vectParam, Role* pGM);

	// 帮派团购测试用
	DWORD HandleLaunchGP(const std::vector<DWORD>& vectParam, Role* pGM);	
	DWORD HandleRespondGP(const std::vector<DWORD>& vectParam, Role* pGM);	

	// 测试名帖
	DWORD HandleVCard(const std::vector<DWORD>& vectParam, Role* pGM);

	// 测试称号
	DWORD HandleTitle( const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleSetTitle( const std::vector<DWORD>& vectParam, Role* pGM);

	// 在玩家当前坐标刷出怪物　
	DWORD HandleCreateMonster( const std::vector<DWORD>& vectParam, Role* pGM);

	// 是否计算技能的触发率
	DWORD HandleTriggerOff(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleTriggerOn(const std::vector<DWORD>& vectParam, Role* pGM);

	// 装备強化
	//DWORD HandleEngrave(const std::vector<DWORD>& vectParam, Role* pGM);
	//DWORD HandlePosy(const std::vector<DWORD>& vectParam, Role* pGM);
	//DWORD HandleAddAtt2Weapon(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleConsolidateEquip(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandlexiulianEquip(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleDaKongEquip(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandlekaiguangEquip(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD Handlexili(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandlexiliTihuan(const std::vector<DWORD>& vectParam, Role* pGM);

	// 调整生产技能熟练度
	DWORD HandleProficiency(const std::vector<DWORD>& vectParam, Role* pGM);

	// 增加装备套装
	DWORD HandleAddSuit(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleAddEquip(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleLeftMsg(const std::vector<DWORD>& vectParam, Role* pGM);

	// 测试宝箱节点
	DWORD HandleChangeRoleChestSum(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleChangeServerChestSum(const std::vector<DWORD>& vectParam, Role* pGM);
	
	DWORD HandleLurk(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleInvincible(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleRolePosition(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleNoSpeak(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleCancelNoSpeak(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleKillMonster(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleKillAll(const std::vector<DWORD>& vectParam, Role* pGM);

	// 摊位经验
	DWORD HandleStallExp(const std::vector<DWORD>& vectParam, Role* pGM);

	// add by wuingran at 2010.9.19 给服务器所有人添加装备
	DWORD HandleAllRoleEquip(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleSetPKState(const std::vector<DWORD>& vectParam,	Role* pGM);

	DWORD HandleGMCreateGuild(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleGMGuildSign(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleDeclareWar(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandelDeclareWarRes(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleGuildGradeUp(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleMissGuild(const std::vector<DWORD>& vectParam, Role* pGM);
	
	DWORD handleincguildpro(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleRoleRevive(const std::vector<DWORD>& vectParam,	Role* pGM);

	DWORD HandleChangeSymbol(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleCompleteQuest(const std::vector<DWORD>& vectParam, Role* pGM );

	DWORD HandlePutQuest(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetQuest(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandlebeginRide(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleCancelRide(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleUpgradeRide(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleRemoveRideInlay(const std::vector<DWORD>& vectParam, Role* pGM );

	DWORD HandleMakeMaster(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleMakeMasterEx(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleMakePrentice(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleMakePrenticeEx(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleMasterPrenticeBreak(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleGetMasters(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleGetMasterPlacard(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleAddWeaponSkill(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandStartHang(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandSetHang(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandlePickUpExp(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleCancelHang(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandleUseKillBadge(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD HandStartGift(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD HandleGetGift(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD handle_begin_paimai(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_cancel_paimai(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_begin_jing(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_paimai_chaw(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_get_all_paimai(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD handle_begin_bank(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_bank_jing(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD handle_reset_instance(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_join_recruit(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_leave_recruit(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_query_recruit(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD handle_stack_item(const std::vector<DWORD>& vectParam, Role* pGm);
	DWORD handle_whosyourdaddy(const std::vector<DWORD>& vectParam, Role* pGm);
	DWORD handle_chengjiupoint(const std::vector<DWORD>& vectParam, Role* pGm);

	DWORD handle_create_instance(const std::vector<DWORD>& vectParam, Role* pGm);
	DWORD handle_change_newness(const std::vector<DWORD>& vectParam, Role* pGm);
	DWORD handle_enter_pvp(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_enter_1v1(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_apply_1v1(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_result_1v1(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_leave_prictice(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD handle_clearsession(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_openver(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_GetExploits(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_serial_reward(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_add_banggong(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD HandleClearPKValue(const std::vector<DWORD>& vectParam, Role* pGM );
	DWORD hanle_cleardaydata(const std::vector<DWORD>& vectParam, Role* pGM );	
	DWORD hanle_set_pet_ernie_level(const std::vector<DWORD>& vectParam, Role* pGM );	

	DWORD handle_KickFaster(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_strengthcheck(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_move_limit(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD hanle_set_time(const std::vector<DWORD>& vectParam, Role* pGM);

	DWORD handle_sign(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_sign_reward(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_get_sign(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_huenlian(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_godLevel(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_reward_Item(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_open_SBK(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_exp_raid(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_open_server(const std::vector<DWORD>& vectParam, Role* pGM);
	DWORD handle_enter_battle(const std::vector<DWORD>& vectParam, Role* pGM);
private:
	struct tagGMCommand
	{
		tstring		strCmd;		// 命令名
		tstring		strDesc;	// 描述
		GMCMDHANDLE	handler;	// 函数指针
		BYTE		byParamNum;	// 参数个数
		BYTE		byPrivilege;// GM命令权限
		INT16		n16ExeTimes;// 执行次数
	};

	package_map<DWORD, tagGMCommand*>	m_mapGMCommand;
};
