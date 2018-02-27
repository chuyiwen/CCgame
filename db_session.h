
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ϸ�����������ݿ��������ͨ��
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

	// ����յ���������Ϣ
	VOID FreeRecved(LPVOID p_message) { m_pTran->free_recv_msg((LPBYTE)p_message); }

	INT	GetUnsendPackageNum() { return m_pTran->get_recv_package_num(); }
	INT	GetReceivedPackageNum() { return m_pTran->get_recv_package_num(); }

private:
	// ��ȡ�ļ�, ��ʼ����Ա
	BOOL InitConfig();

	// ע�����е���������
	VOID RegisterAllDBCommand();

	VOID UnRegisterAllDBCommand();

private:
	// �������ݿ������
	UINT thread_connect();
	static UINT WINAPI static_thread_connect(LPVOID p_data);

private:
	// ��֤������
	DWORD HandleCertification(tag_net_message* p_message, DWORD);
	DWORD HandleHeartBeat(tag_net_message* p_message, DWORD);

	// ��ʼ����Ϣ
	DWORD HandleSTWorldInitOK(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAllRoleInfo(tag_net_message* p_message, DWORD);
	DWORD HandleItemInfo(tag_net_message* p_message, DWORD);
	DWORD HandleItemNeedLog(tag_net_message* p_message, DWORD);

	// ���ﴴ����ɾ���Ͷ�ȡ
	DWORD HandleRoleEnum(tag_net_message* p_message, DWORD);
	DWORD HandleRoleCreate(tag_net_message* p_message, DWORD);
	DWORD HandleRoleDelete(tag_net_message* p_message, DWORD);
	DWORD HandleRoleDeleteGuardSet(tag_net_message* p_message, DWORD);
	DWORD HandleCancelRoleDelGuardTime(tag_net_message* p_message, DWORD);
	DWORD HandleRoleChangeName(tag_net_message* p_message, DWORD);
	DWORD HandleRoleResume(tag_net_message* p_message, DWORD) { return 0; }
	DWORD HandleRoleLoad(tag_net_message* p_message, DWORD);
	DWORD HandleLoadSerialReward(tag_net_message* p_message, DWORD);

	// ��ٱ��������µ���Ʒ
	DWORD HandleBaiBaoLoad(tag_net_message* p_message, DWORD);
	// ���°ٱ����е�Ԫ��
	DWORD HandleBaiBaoYuanBaoLoad(tag_net_message* p_message, DWORD);

	// �����콱��־
	DWORD HandleLoadWebReceive(tag_net_message* p_message, DWORD);

	// Ԫ���������
	DWORD HandleLoadAllYBAccount(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAllYBOrder(tag_net_message* p_message, DWORD);
	DWORD HandleRoleGetYBOrder(tag_net_message* p_message, DWORD);

	// �ٱ�����ʷ��¼���
	DWORD HandleBaiBaoLoadLog(tag_net_message* p_message, DWORD);

	// ������
	DWORD HandleRepRankLoad(tag_net_message* p_message, DWORD);
	DWORD HandleGetActTreasureList(tag_net_message* p_message, DWORD);
	DWORD HandleRepRstTimeStamp(tag_net_message* p_message, DWORD);
	DWORD HandleGetFameHallEnterSnap(tag_net_message* p_message, DWORD);

	// �������
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
	// ����
	DWORD HandleCreatePetSoul(tag_net_message* p_message, DWORD);
	DWORD HandleLoadPetSns(tag_net_message* p_message, DWORD);
	// �̳����
	DWORD HandleLoadAllGPInfo(tag_net_message* p_message, DWORD);

	// ��ɫ����
	DWORD HandleLoadRoleVCard(tag_net_message* p_message, DWORD);

	// VIP̯λ
	DWORD HandleLoadVIPStallInfo(tag_net_message* p_message, DWORD);

	// �̶��
	DWORD HandleLoadActivityData(tag_net_message* p_message, DWORD);

	// VIP����
	DWORD HandleLoadVNBData(tag_net_message* p_message, DWORD);

	// ��ȡ����
	DWORD HandleLoadLeftMsg(tag_net_message* p_message, DWORD);

	// ��ȡ�ʼ�
	DWORD HandleLoadAllMail(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAllMailEnd(tag_net_message* p_message, DWORD);

	// ��ȡ�ʼ�����
	DWORD HandleLoadMailContent(tag_net_message* p_message, DWORD);
	DWORD HandleLoadMailContentEnd(tag_net_message* p_message, DWORD);

	// ��ȡ�ʼ���Ʒ
	DWORD HandleLoadMailItem(tag_net_message* p_message, DWORD);
	DWORD HandleLoadMailItemEnd(tag_net_message* p_message, DWORD);

	// ��ȡ�ʼ������
	DWORD HandleGetMailMaxSerial(tag_net_message* p_message, DWORD);

	DWORD HandleLoadAllMasterPrentices( tag_net_message* p_message, DWORD );
	DWORD HandleLoadAllMasterrecruit(tag_net_message* p_message, DWORD);

	// ���а�
	DWORD HandleLoadLevelRank(tag_net_message* p_message, DWORD);
	DWORD HandleEquipRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadGuildRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadKillRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadJusticeRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoad1v1ScoreRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadShihunRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAchPointRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadAchNumberRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadMasterRank(tag_net_message* p_message,DWORD);//ʦͽ�� gx add 2013.12.06
	DWORD HandleLoadMountsRank(tag_net_message* p_message, DWORD);
	DWORD HandleLoadReachRank(tag_net_message* p_message, DWORD);
	// ����
	DWORD HandleLoadPaiMaiMaxID(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_paimai(tag_net_message* p_message, DWORD);
	DWORD handle_load_paimai_item(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_paimai_end(tag_net_message* p_message, DWORD);
	DWORD handle_load_paimai_item_end(tag_net_message* p_message, DWORD);

	// �Զ�����
	DWORD handle_load_auto_paimai(tag_net_message* p_message, DWORD);
	DWORD handle_load_auto_paimai_end(tag_net_message* p_message, DWORD);
	DWORD handle_check_is_paimai(tag_net_message* p_message, DWORD);
	DWORD handle_auto_paimai_init_ok(tag_net_message* p_message, DWORD);

	// Ǯׯ���
	DWORD handle_get_max_bank_id(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_bank(tag_net_message* p_message, DWORD);

	DWORD handle_load_all_guerdonquest(tag_net_message* p_message, DWORD);
	DWORD handle_load_all_guerdonquest_reward(tag_net_message* p_message, DWORD);
	
private:
	
	few_connect_client*			m_pTran;
	

	// ���Ӳ���
	CHAR						m_szDBIP[X_IP_LEN];		// ���ݿ������ip
	DWORD						m_dwDBPort;				// ���ݿ������port

	// �������Ӻ��跢�͵�����
	DWORD						m_dwGoldenCode;					// ���ݿ��������ɫ����
	//TCHAR						m_szServerName[LONG_STRING];	// ������Ϸ����Ļ�������

	volatile BOOL				m_bTermConnect;					// ��¼��ǰ����״̬
	volatile BOOL				m_bInitOK;						// �Ƿ��ʼ�����
};

extern DBSession g_dbSession;		// DB sessionȫ�ֶ���
