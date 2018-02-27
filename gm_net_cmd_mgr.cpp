
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
#include "StdAfx.h"
#include "gm_net_cmd_mgr.h"
#include "role.h"
#include "clandata.h"
#include "chat_mgr.h"
#include "role_mgr.h"
#include "../common/ServerDefine/log_server_define.h"

//-----------------------------------------------------------------------------
// 构造&析构
//-----------------------------------------------------------------------------
GMCommandMgr::GMCommandMgr()
{
	//m_dwGMCmdID = INVALID_VALUE;
}

GMCommandMgr::~GMCommandMgr(){}

//-----------------------------------------------------------------------------
// 注册一条GM命令(注: 第一个参数必须为小写)
//-----------------------------------------------------------------------------
VOID GMCommandMgr::Register(LPCTSTR szName, GMCMDHANDLE pFun, BYTE byPrivilege, 
							LPCTSTR szDesc/*=NULL*/, BYTE byParamNum/*=1*/)
{
	DWORD dwID = get_tool()->crc32(szName);

	tagGMCommand* pCmd = m_mapGMCommand.find(dwID);
	if(VALID_POINT(pCmd))
	{
		ASSERT(0);	// 重复注册
		return;
	}

	pCmd = new tagGMCommand;
	pCmd->strCmd		= szName;		// 该字符串必须为小写
	pCmd->strDesc		= szDesc;
	pCmd->handler		= pFun;
	pCmd->byParamNum	= byParamNum;
	pCmd->byPrivilege	= byPrivilege;
	pCmd->n16ExeTimes	= 0;

	m_mapGMCommand.add(dwID, pCmd);

	return;
}

//-----------------------------------------------------------------------------
// 注册所以GM命令(注意：注册GM命令时，命令名称必须为小写。如："item")
//-----------------------------------------------------------------------------
VOID GMCommandMgr::RegisterAll()
{
	Register(_T("addskill"),		&GMCommandMgr::HandleAddSkill,			4, _T("addskill dw_data_id"),					1);
	Register(_T("addbuff"),			&GMCommandMgr::HandleAddBuff,			2, _T("gm addbuff dwBuffID"),				1);
	Register(_T("addquest"),		&GMCommandMgr::HandleAddQuest,			4, _T("addquest nQuestId"),					1);
	Register(_T("addsuit"),			&GMCommandMgr::HandleAddSuit,			4, _T("gm addsuit dwSuitID nQlty"),			2);
	Register(_T("addequip"),		&GMCommandMgr::HandleAddEquip,			4, _T("gm addequip nType nLevel nQlty"),	3);
	Register(_T("affair"),			&GMCommandMgr::HandleResetAffairTimes,	4, _T("affair"),							0);
	Register(_T("att"),				&GMCommandMgr::HandleChangeRoleAtt,		4, _T("att eroleatt nval"),					2);
	//Register(_T("addweaponatt"),	&GMCommandMgr::HandleAddAtt2Weapon,		4, _T("addweaponatt n16ItemIndex bAttType AttID"),		3);
	Register(_T("speed"),			&GMCommandMgr::HandleChangSpeed,		1, _T("Change Speed"),					1);

	Register(_T("clandata"),		&GMCommandMgr::HandleClanData,			3, _T("clandata testNo clanType nVal"),		3);
	Register(_T("changescriptdata"),&GMCommandMgr::HandleChangeScriptData,	5, _T("changescriptdata nIndex dwValue"),	2);
	Register(_T("class"),			&GMCommandMgr::HandleSetVocation,		2, _T("class nType eVocation"),				2);
	Register(_T("clearbag"),		&GMCommandMgr::HandleClearBag,			1, _T("clearbag"),							0);
	Register(_T("cooloff"),			&GMCommandMgr::HandleCoolOff,			5, _T("gm cooloff"),						0);
	Register(_T("coolon"),			&GMCommandMgr::HandleCoolOn,			5, _T("gm coolon"),							0);
	Register(_T("contribute"),		&GMCommandMgr::HandleGetContribute,		4, _T("contribute nContribute"),			1);

	Register(_T("double"),			&GMCommandMgr::HandleDouble,			1, _T("double rate"),						3);
	
	Register(_T("exvolume"),		&GMCommandMgr::HandleGetExVolume,		2, _T("exvolume nExVolume"),				1);
	Register(_T("exp"),				&GMCommandMgr::HandleFillExp,			4, _T("exp nExp"),							1);
	//Register(_T("equippot"),		&GMCommandMgr::HandleEquipPotInc,		4, _T("equippot nIndex nValue"),			2);
	//Register(_T("engrave"),			&GMCommandMgr::HandleEngrave,			4, _T("engrave n16ItemIndex dwFormula"),	2);

	Register(_T("fill"),			&GMCommandMgr::HandleFillLevel,			4, _T("fill nLevel"),						1);
	Register(_T("fund"),			&GMCommandMgr::HandleGetFund,			4, _T("fund nFund"),						1);

	Register(_T("goto"),			&GMCommandMgr::HandleGoto,				1, _T("goto szMapName x z"),				3);
	Register(_T("gotorole"),		&GMCommandMgr::HandleGotoRole,			1, _T("gotorole szRoleName"),				1);
	Register(_T("gotoinst"),		&GMCommandMgr::HandleGotoRoleInst,		1, _T("gotorole inst"),						2);
	Register(_T("guild"),			&GMCommandMgr::HandleGuildStatus,		4, _T("guild"),								2);
	Register(_T("guildfacility"),	&GMCommandMgr::HandleGuildFacility,		4, _T("guildfacility"),						2);
	
	Register(_T("item"),			&GMCommandMgr::HandleCreateItem,		4, _T("item dw_data_id nItemNum nQuality"),	3);
	//added by mmz at 2010.9.19 
	Register(_T("allitem"),			&GMCommandMgr::HandleAllRoleEquip,		4, _T("allitem dw_data_id nQuality"),			2);
	Register(_T("invincible"),		&GMCommandMgr::HandleInvincible,		2, _T("invincible"),						0);
	
	Register(_T("kickid"),			&GMCommandMgr::HandleKickRoleID,			2, _T("kick roleid"),					1);
	Register(_T("kick"),			&GMCommandMgr::HandleKickRole,			2, _T("kick szRoleName"),					1);
	Register(_T("killmonster"),		&GMCommandMgr::HandleKillMonster,		3, _T("killmonster dwMonsterID"),			1);
	Register(_T("killall"),			&GMCommandMgr::HandleKillAll,			2, _T("kill all monster"),					0);

	Register(_T("leftmsg"),			&GMCommandMgr::HandleLeftMsg,			0, _T("gm leftmsg n_num"),					1);
	Register(_T("launchgp"),		&GMCommandMgr::HandleLaunchGP,			1, _T("launch group purchase"),				4);
	Register(_T("lurk"),			&GMCommandMgr::HandleLurk,				1, _T("Lurk nLurkLevel"),					1);

	Register(_T("moverole"),		&GMCommandMgr::HandleMoveRole,			2, _T("moverole szRoleName"),				1);
	Register(_T("material"),		&GMCommandMgr::HandleGetMaterial,		4, _T("material nMaterial"),				1);
	Register(_T("monster"),			&GMCommandMgr::HandleCreateMonster,		3, _T("create monster"),					2);
	Register(_T("maxonline"),		&GMCommandMgr::HandleResizeOnlineNum,	5, _T("maxonline n_num"),					3);

	Register(_T("noquestdone"),		&GMCommandMgr::HandleClearRoleQuest,	4, _T("noquestdone"),						0);
	Register(_T("nospeak"),			&GMCommandMgr::HandleNoSpeak,			2, _T("nospeak"),							0);

	Register(_T("pet"),				&GMCommandMgr::HandlePet,				1, _T("pet testNo petIndex petProtoID"),	3);
	//Register(_T("posy"),			&GMCommandMgr::HandlePosy,				4, _T("posy n16ItemIndex dwFormula"),		2);
	Register(_T("proficiency"),		&GMCommandMgr::HandleProficiency,		4, _T("gm proficiency dwSkillID nValue"),	2);

	Register(_T("quest"),			&GMCommandMgr::HandleRoleQuest,			4, _T("quest dwQuestId bDone"),				2);
	Register(_T("completequest"),	&GMCommandMgr::HandleCompleteQuest,			4, _T("quest dwQuestId bDone"),				2);
	Register(_T("putquest"),		&GMCommandMgr::HandlePutQuest,			4, _T("quest dwQuestId bDone"),				1);
	Register(_T("getquest"),		&GMCommandMgr::HandleGetQuest,			4, _T("quest dwQuestId bDone"),				1);

	//Register(_T("reputation"),	&GMCommandMgr::HandleSetReputation,		2, _T("reputation eClan nReputation"),		2);
	Register(_T("reloadscript"),	&GMCommandMgr::HandleReloadScripts,		5, _T("reload server scripts"),				0);
	Register(_T("roleguild"),		&GMCommandMgr::HandleRoleGuild,			4, _T("roleguild"),							2);
	Register(_T("respondgp"),		&GMCommandMgr::HandleRespondGP,			1, _T("respond group purchase"),			3);
	Register(_T("roleposition"),	&GMCommandMgr::HandleRolePosition,		2, _T("roleposition"),						1);

	Register(_T("skillcd"),			&GMCommandMgr::HandleClearSkillCD,		4, _T("skillcd talenttype exceptid	2"),	1);
	Register(_T("silver"),			&GMCommandMgr::HandleGetSilver,			6, _T("silver nGold nSilver nCopper"),				3);
	Register(_T("bindsilver"),		&GMCommandMgr::HandGetBindSilver,		4,	_T("bindsilver nGold nSilver nCopper"),			3);

	Register(_T("triggeroff"),		&GMCommandMgr::HandleTriggerOff,		4, _T("triggeroff"),						0);
	Register(_T("triggeron"),		&GMCommandMgr::HandleTriggerOn,			4, _T("triggeron"),							0);
	Register(_T("tael"),			&GMCommandMgr::HandleGetTael,			4, _T("tael nTael"),						1);
	Register(_T("title"),			&GMCommandMgr::HandleTitle,				3, _T("title event para1 para2"),			3);
	Register(_T("settitle"),		&GMCommandMgr::HandleSetTitle,			2, _T("set title"),			1);

	Register(_T("vnb"),				&GMCommandMgr::HandleVipNetBar,			1, _T("vnb testNo accountID ip"),			3);
	Register(_T("vcard"),			&GMCommandMgr::HandleVCard,				1, _T("vcard roleid"),						1);

	Register(_T("yuanbao"),			&GMCommandMgr::HandleGetYuanBao,		6, _T("yuanbao nYuanBao"),					1);

	// 测试宝箱节点
	Register(_T("changerolechest"),	&GMCommandMgr::HandleChangeRoleChestSum,1,	_T("change role chest sum"),			1);
	Register(_T("changeserverchest"),&GMCommandMgr::HandleChangeServerChestSum, 1,	_T("change server chest sum"),		1);

	Register(_T("speakoff"),		&GMCommandMgr::HandleNoSpeak,			3, _T("speakoff dw_role_id"),					1);
	Register(_T("speakon"),			&GMCommandMgr::HandleCancelNoSpeak,		3, _T("speakon dw_role_id"),					1);

	Register(_T("stallexp"),		&GMCommandMgr::HandleStallExp,			3, _T("stallexp nExp"),						1);

	Register(_T("pkstate"),			&GMCommandMgr::HandleSetPKState,		4, _T("pkstate state"),						1);

	Register(_T("createguild"),		&GMCommandMgr::HandleGMCreateGuild,		4, _T("create guild"),						0);
	Register(_T("guildsign"),		&GMCommandMgr::HandleGMGuildSign,			4, _T("guild sign"),					0);
	Register(_T("guildwar"),		&GMCommandMgr::HandleDeclareWar,			4, _T("guild declare war"),				1);
	Register(_T("guilddeclarewarres"), &GMCommandMgr::HandelDeclareWarRes,		4, _T("guild declare war res"),			2);
	Register(_T("guildgradeup"),		&GMCommandMgr::HandleGuildGradeUp,		4, _T("guild grade up"),				1);
	Register(_T("dismissguild"),	&GMCommandMgr::HandleMissGuild,			4, _T("guild dismiss"), 0);
	Register(_T("incguildpro"),		&GMCommandMgr::handleincguildpro,		4, _T("guild incpro"), 1);

	Register(_T("rolerevive"),		&GMCommandMgr::HandleRoleRevive,		4,	_T("role revive"),					0);
	Register(_T("changesymbol"),	&GMCommandMgr::HandleChangeSymbol,		4, _T("change symbol"),					0);

	Register(_T("beginride"),		&GMCommandMgr::HandlebeginRide,				4, _T("begin ride"),			1);
	Register(_T("cancelride"),		&GMCommandMgr::HandleCancelRide,			4, _T("cancel ride"),				1);
	Register(_T("upgride"),			&GMCommandMgr::HandleUpgradeRide,		4, _T("upgrade ride"),			5);
	Register(_T("remrideinlay"),	&GMCommandMgr::HandleRemoveRideInlay,		4, _T("remove ride inlay"),		4);


	Register(_T("makemaster"),		&GMCommandMgr::HandleMakeMaster,		4, _T("remove ride inlay"),		1);
	Register(_T("makemasterex"),	&GMCommandMgr::HandleMakeMasterEx,		4, _T("remove ride inlay"),		2);
	Register(_T("makeprentice"),	&GMCommandMgr::HandleMakePrentice,		4, _T("remove ride inlay"),		1);
	Register(_T("makeprenticeex"),	&GMCommandMgr::HandleMakePrenticeEx,		4, _T("remove ride inlay"),		2);
	Register(_T("masterprenticebreak"),	&GMCommandMgr::HandleMasterPrenticeBreak,		4, _T("remove ride inlay"),		1);
	Register(_T("getmasters"),		&GMCommandMgr::HandleGetMasters,		4, _T("remove ride inlay"),		0);
	Register(_T("getmasterplacard"),&GMCommandMgr::HandleGetMasterPlacard,		4, _T("remove ride inlay"),		0);

	Register(_T("addweaponskill"),	&GMCommandMgr::HandleAddWeaponSkill,		4, _T("add weapon skill"),		1);

	Register(_T("starthang"),		&GMCommandMgr::HandStartHang,					4, _T("starthang"),			0);
	Register(_T("sethang"),			&GMCommandMgr::HandSetHang,					4, _T("sethang use1 use2"), 2);
	Register(_T("pickupexp"),		&GMCommandMgr::HandlePickUpExp,				4,	_T("pickupexp type"),	1);
	Register(_T("cancelhang"),		&GMCommandMgr::HandleCancelHang,		4, _T("cancel hang"),	0);
	Register(_T("usekillbadge"),	&GMCommandMgr::HandleUseKillBadge,		4, _T("cancel hang"),	2);

	Register(_T("startgift"),		&GMCommandMgr::HandStartGift,			4,	_T("start gift"),	0);
	Register(_T("getgift"),			&GMCommandMgr::HandleGetGift,			4,	_T("get gift"),		0);

	Register(_T("qianghua"),		&GMCommandMgr::HandleConsolidateEquip,		4, _T("pos level"),		2);
	Register(_T("xiulian"),			&GMCommandMgr::HandlexiulianEquip,		4, _T("pos level"),		2);
	Register(_T("dakong"),			&GMCommandMgr::HandleDaKongEquip,		4, _T("pos holdnum"),		2);
	Register(_T("kaiguang"),		&GMCommandMgr::HandlekaiguangEquip,		4, _T("pos holdnum"),		2);
	Register(_T("xili"),			&GMCommandMgr::Handlexili,				4, _T("xili"), 2);
	Register(_T("xilitihuan"),		&GMCommandMgr::HandlexiliTihuan,		4, _T("xilitihuan"), 1);

	Register(_T("beginpaimai"),		&GMCommandMgr::handle_begin_paimai,		4,	_T("begin paimai"),	4);
	Register(_T("cancelpaimai"),	&GMCommandMgr::handle_cancel_paimai,    4,	_T("cancel paimai"), 1);
	Register(_T("beginjing"),		&GMCommandMgr::handle_begin_jing,		4,  _T("begin jing"),	1);
	Register(_T("paimaichaw"),		&GMCommandMgr::handle_paimai_chaw,		4,	_T("paimai chaw"),	1);
	Register(_T("getallpaimai"),	&GMCommandMgr::handle_get_all_paimai,	4,	_T("get all paimai"),	1);

	Register(_T("beginbank"),		&GMCommandMgr::handle_begin_bank,		4,	_T("begin bank"),	5);
	Register(_T("bankjing"),		&GMCommandMgr::handle_bank_jing,		4,	_T("bank jing"),	1);

	Register(_T("resetinst"),		&GMCommandMgr::handle_reset_instance,	4,	_T("reset instance"), 0);

	Register(_T("joinrecruit"),		&GMCommandMgr::handle_join_recruit,		4,	_T("join recruit"),		0);
	Register(_T("leaverecruit"),	&GMCommandMgr::handle_leave_recruit,	4,	_T("leave recruit"),	0);
	Register(_T("queryrecruit"),	&GMCommandMgr::handle_query_recruit,	4,	_T("query recruit"),	0);

	Register(_T("stackitem"),		&GMCommandMgr::handle_stack_item,		4,	_T("stack item"), 0);
	
	Register(_T("whosyourdaddy"),	&GMCommandMgr::handle_whosyourdaddy,		4,	_T("whos your daddy"), 0);

	Register(_T("chengjiupoint"),	&GMCommandMgr::handle_chengjiupoint,		4,	_T("cheng jiu point"), 0);

	Register(_T("createinst"),		&GMCommandMgr::handle_create_instance,		4,	_T("create instance"),	1);

	Register(_T("naijiu"),			&GMCommandMgr::handle_change_newness,		4,	_T("chenge newness"),	2);

	Register(_T("enterpvp"),		&GMCommandMgr::handle_enter_pvp,			4,	_T("enter pvp"),	0);

	Register(_T("enter1v1"),		&GMCommandMgr::handle_enter_1v1,			4,	_T("enter 1v1"),	0);

	Register(_T("apply1v1"),		&GMCommandMgr::handle_apply_1v1,			4,	_T("apply 1v1"),	1);
	Register(_T("result1v1"),		&GMCommandMgr::handle_result_1v1,			4,	_T("result 1v1"),	1);

	Register(_T("leave"),			&GMCommandMgr::handle_leave_prictice,		4,	_T("leave prictice"),	2);

	Register(_T("clearsession"),	&GMCommandMgr::handle_clearsession,			10, _T("clear session"), 1);
	
	Register(_T("openver"),			&GMCommandMgr::handle_openver,				6, _T("open verification"), 1);

	Register(_T("kickfaster"),		&GMCommandMgr::handle_KickFaster,			6, _T("kick faster"), 1);
	Register(_T("strengthcheck"),	&GMCommandMgr::handle_strengthcheck,		6, _T("strength check"), 2);
	Register(_T("exploit"),			&GMCommandMgr::handle_GetExploits,				6, _T("get exploits"), 1);
	Register(_T("serial"),			&GMCommandMgr::handle_serial_reward,		6, _T("serial reward"), 1);
	Register(_T("addbanggong"),		&GMCommandMgr::handle_add_banggong,		6,	_T("add banggong"), 1);
	Register(_T("clearpk"),			&GMCommandMgr::HandleClearPKValue,		6,	_T("Clear PK"), 2);

	Register(_T("movelimit"),		&GMCommandMgr::handle_move_limit,				6,	_T("move limit"), 3);
	
	Register(_T("settime"),			&GMCommandMgr::hanle_set_time,				10, _T("set time"), 3);
	Register(_T("cleardaydata"),	&GMCommandMgr::hanle_cleardaydata,				6, _T("clear day data"), 0);
	Register(_T("ernie"),			&GMCommandMgr::hanle_set_pet_ernie_level,	10, _T("set pet ernie"), 1);
	Register(_T("sign"),			&GMCommandMgr::handle_sign,				6,		_T("sign"),  1);
	Register(_T("signreward"),		&GMCommandMgr::handle_sign_reward,		6,		_T("sign reward"), 1);
	Register(_T("getsign"),			&GMCommandMgr::handle_get_sign,			6,		_T("get sign"),	0);
	Register(_T("huenlian"),		&GMCommandMgr::handle_huenlian,			6,		_T("huenlian"),	0);
	Register(_T("godlevel"),		&GMCommandMgr::handle_godLevel,			6,		_T("god level"),	0);
	Register(_T("reward"),			&GMCommandMgr::handle_reward_Item,		4,		_T("reward Item"),	3);
	Register(_T("opensbk"),			&GMCommandMgr::handle_open_SBK,			4,		_T("open SBK"), 0);
	Register(_T("expr"),			&GMCommandMgr::handle_exp_raid,			4,		_T("exp raid"), 1);	
	Register(_T("eb"),				&GMCommandMgr::handle_enter_battle,		4,		_T("enter battle"), 0);	
	Register(_T("testopenserver"),	&GMCommandMgr::handle_open_server,		4,		_T("open server"), 1);	
}

//------------------------------------------------------------------------------
// 取消注册
//------------------------------------------------------------------------------
VOID GMCommandMgr::UnRegisterAll()
{
	m_mapGMCommand.reset_iterator();
	tagGMCommand* pCmd = NULL;
	while(m_mapGMCommand.find_next(pCmd))
	{
		SAFE_DELETE(pCmd);
	}

	m_mapGMCommand.clear();
}

//-----------------------------------------------------------------------------
// 执行 command
//-----------------------------------------------------------------------------
DWORD GMCommandMgr::Excute(LPCTSTR szCommand, Role *pRole)
{
	ASSERT(VALID_POINT(pRole));

	std::vector<tstring> command;
	get_tool()->string_to_vector(command, szCommand);
	command.resize(10);	// 不可能超过10个参数
	std::vector<DWORD> vp;

	if(command.size() == 0)
		return INVALID_VALUE;

	// 得到实际的参数
	for(INT n=1; n<(INT)command.size(); ++n)
	{
		vp.push_back((DWORD)command[n].c_str());
	}

	transform(command[0].begin(), command[0].end(), command[0].begin(), tolower);	
	DWORD dwID = get_tool()->crc32(command[0].c_str());	// 转为小写再运算

	tagGMCommand *pCmd = m_mapGMCommand.find(dwID);
	if(!VALID_POINT(pCmd))
	{
		print_message(_T("Unknow GM command recved[%s]\r\n"), command[0].c_str());
		return INVALID_VALUE;
	}

	if(!VALID_POINT(pRole->GetSession()))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	// 命令权限判断
	if(!VALID_POINT(pRole->GetSession()) || !pRole->GetSession()->IsPrivilegeEnough(pCmd->byPrivilege))
	{
		print_message(_T("No enough privilege<RoleID:%u> to execute<gm %s>!!!!!!!!!!!!!\r\n"), 
			pRole->GetID(), pCmd->strCmd.c_str());
		return INVALID_VALUE;
	}

	if((BYTE)vp.size() < pCmd->byParamNum)
	{
		return INVALID_VALUE;
	}

	++pCmd->n16ExeTimes;

	DWORD dwRet = (this->*pCmd->handler)(vp, pRole);

	TCHAR szCmd[LONG_STRING] = {0};
	_stprintf(szCmd, _T("gm %s"), szCommand);

	LogGMCmd(pRole->GetID(), szCmd, dwRet);

	return dwRet;
}

DWORD GMCommandMgr::HandleLeftMsg( const std::vector<DWORD>& vectParam, Role* pGM )
{
	INT nType = _ttoi((LPCTSTR)vectParam[0]);

	if ( TRUE == g_msgMgr.load_offline_msg(pGM->GetID()) )
		return E_Success;
	else
		return E_SystemError;
}

VOID GMCommandMgr::LogGMCmd(DWORD dw_role_id, LPCTSTR szCmd, DWORD dw_error_code)
{
	NET_DB2C_log_gm_cmd send;

	s_role_info* pRoleInfo = g_roleMgr.get_role_info(dw_role_id);
	if (!VALID_POINT(pRoleInfo))
	{
		return;
	}
	
	send.s_log_gm_cmd_.dw_role_id	= dw_role_id;
	send.s_log_gm_cmd_.dw_error_code	= dw_error_code;
	_tcscpy(send.s_log_gm_cmd_.sz_gm_cmd, szCmd);

	g_dbSession.Send(&send, send.dw_size);
}
