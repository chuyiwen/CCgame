
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//游戏服务器与数据库服务器的通信
#pragma once

#include "StdAfx.h"

#include "../../common/WorldDefine/base_define.h"

//-----------------------------------------------------------------------------
class DBSession
{
public:
	DBSession();
	~DBSession();

public:
	BOOL Init();
	VOID Destroy();

	VOID Update();
	BOOL IsWell() { return (m_bInitOK && m_pTran->is_connect()); }

	VOID Send(LPVOID p_message, DWORD dwMsgSize)	{ m_pTran->send_msg(p_message, dwMsgSize); }

	LPBYTE Recv(DWORD& dwMsgSize) { return m_pTran->recv_msg( dwMsgSize ); }

	// 清除收到的网络消息
	VOID FreeRecved(LPVOID p_message) { m_pTran->free_recv_msg((LPBYTE)p_message); }

	INT	GetUnsendPackageNum() { return m_pTran->get_recv_package_num(); }
	INT	GetReceivedPackageNum() { return m_pTran->get_recv_package_num(); }

private:
	// 读取文件, 初始化成员
	BOOL InitConfig();

	// 注册所有的网络命令
	VOID RegisterAllDBCommand();

	VOID UnRegisterAllDBCommand();

private:
	// 连接数据库服务器
	UINT thread_connect();
	static UINT WINAPI static_thread_connect(LPVOID p_data);

private:
	// 认证及心跳
	DWORD HandleCertification(tag_net_message* p_message, DWORD);
	DWORD HandleHeartBeat(tag_net_message* p_message, DWORD);

	// 初始化消息
	DWORD HandleSTWorldInitOK(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAllRoleInfo(tag_net_message* p_message, DWORD);
	DWORD HandleItemInfo(tag_net_message* p_message, DWORD);
	DWORD HandleItemNeedLog(tag_net_message* p_message, DWORD);

	// 人物创建、删除和读取
	DWORD HandleRoleEnum(tag_net_message* p_message, DWORD);
	DWORD HandleRoleCreate(tag_net_message* p_message, DWORD);
	DWORD HandleRoleDelete(tag_net_message* p_message, DWORD);
	DWORD HandleRoleDeleteGuardSet(tag_net_message* p_message, DWORD);
	DWORD HandleCancelRoleDelGuardTime(tag_net_message* p_message, DWORD);
	DWORD HandleRoleChangeName(tag_net_message* p_message, DWORD);
	DWORD HandleRoleResume(tag_net_message* p_message, DWORD) { return 0; }
	DWORD HandleRoleLoad(tag_net_message* p_message, DWORD);
	DWORD HandleLoadSerialReward(tag_net_message* p_message, DWORD);

	// 向百宝袋放入新的物品
	DWORD HandleBaiBaoLoad(tag_net_message* p_message, DWORD);
	// 更新百宝袋中的元宝
	DWORD HandleBaiBaoYuanBaoLoad(tag_net_message* p_message, DWORD);

	// 更新领奖标志
	DWORD HandleLoadWebReceive(tag_net_message* p_message, DWORD);

	// 元宝交易相关
	DWORD HandleLoadAllYBAccount(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAllYBOrder(tag_net_message* p_message, DWORD);
	DWORD HandleRoleGetYBOrder(tag_net_message* p_message, DWORD);

	// 百宝袋历史记录相关
	DWORD HandleBaiBaoLoadLog(tag_net_message* p_message, DWORD);

	// 名人堂
	DWORD HandleRepRankLoad(tag_net_message* p_message, DWORD);
	DWORD HandleGetActTreasureList(tag_net_message* p_message, DWORD);
	DWORD HandleRepRstTimeStamp(tag_net_message* p_message, DWORD);
	DWORD HandleGetFameHallEnterSnap(tag_net_message* p_message, DWORD);

	// 帮派相关
	DWORD HandleLoadGuild(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildMember(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildWareItems(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildUpgradeInfo(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildSkillInfo(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildCommerceInfo(tag_net_message* p_message, DWORD);
	DWORD HandleLoadCommerceRankInfo(tag_net_message* p_message, DWORD);
	DWORD HandleGuildInitOK(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildDelate(tag_net_message* p_message, DWORD);
	DWORD HandleLoadPvPData(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildRecruit(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildSkillBoss(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildWarHistory(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildPlantData(tag_net_message* p_message, DWORD);
	DWORD HandleLoadSBKData(tag_net_message* p_message, DWORD);
	// 宠物
	DWORD HandleCreatePetSoul(tag_net_message* p_message, DWORD);
	DWORD HandleLoadPetSns(tag_net_message* p_message, DWORD);
	// 商城相关
	DWORD HandleLoadAllGPInfo(tag_net_message* p_message, DWORD);

	// 角色名贴
	DWORD HandleLoadRoleVCard(tag_net_message* p_message, DWORD);

	// VIP摊位
	DWORD HandleLoadVIPStallInfo(tag_net_message* p_message, DWORD);

	// 固定活动
	DWORD HandleLoadActivityData(tag_net_message* p_message, DWORD);

	// VIP网吧
	DWORD HandleLoadVNBData(tag_net_message* p_message, DWORD);

	// 读取留言
	DWORD HandleLoadLeftMsg(tag_net_message* p_message, DWORD);

	// 读取邮件
	DWORD HandleLoadAllMail(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAllMailEnd(tag_net_message* p_message, DWORD);

	// 读取邮件内容
	DWORD HandleLoadMailContent(tag_net_message* p_message, DWORD);
	DWORD HandleLoadMailContentEnd(tag_net_message* p_message, DWORD);

	// 读取邮件物品
	DWORD HandleLoadMailItem(tag_net_message* p_message, DWORD);
	DWORD HandleLoadMailItemEnd(tag_net_message* p_message, DWORD);

	// 获取邮件最大编号
	DWORD HandleGetMailMaxSerial(tag_net_message* p_message, DWORD);

	DWORD HandleLoadAllMasterPrentices( tag_net_message* p_message, DWORD );
	DWORD HandleLoadAllMasterrecruit(tag_net_message* p_message, DWORD);

	// 排行榜
	DWORD HandleLoadLevelRank(tag_net_message* p_message, DWORD);
	DWORD HandleEquipRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadKillRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadJusticeRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoad1v1ScoreRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadShihunRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAchPointRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAchNumberRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadMasterRank(tag_net_message* p_message,DWORD);//师徒榜 gx add 2013.12.06
	DWORD HandleLoadMountsRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadReachRank(tag_net_message* p_message, DWORD);
	// 拍卖
	DWORD HandleLoadPaiMaiMaxID(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_paimai(tag_net_message* p_message, DWORD);
	DWORD handle_load_paimai_item(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_paimai_end(tag_net_message* p_message, DWORD);
	DWORD handle_load_paimai_item_end(tag_net_message* p_message, DWORD);

	// 自动拍卖
	DWORD handle_load_auto_paimai(tag_net_message* p_message, DWORD);
	DWORD handle_load_auto_paimai_end(tag_net_message* p_message, DWORD);
	DWORD handle_check_is_paimai(tag_net_message* p_message, DWORD);
	DWORD handle_auto_paimai_init_ok(tag_net_message* p_message, DWORD);

	// 钱庄相关
	DWORD handle_get_max_bank_id(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_bank(tag_net_message* p_message, DWORD);

	DWORD handle_load_all_guerdonquest(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_guerdonquest_reward(tag_net_message* p_message, DWORD);
	
private:
	
	few_connect_client*			m_pTran;
	

	// 连接参数
	CHAR						m_szDBIP[X_IP_LEN];		// 数据库服务器ip
	DWORD						m_dwDBPort;				// 数据库服务器port

	// 初次连接后，需发送的数据
	DWORD						m_dwGoldenCode;					// 数据库服务器金色代码
	//TCHAR						m_szServerName[LONG_STRING];	// 运行游戏世界的机器名称

	volatile BOOL				m_bTermConnect;					// 记录当前连接状态
	volatile BOOL				m_bInitOK;						// 是否初始化完成
};

extern DBSession g_dbSession;		// DB session全局对象
