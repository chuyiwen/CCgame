/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ű���Ϣ������

#include "StdAfx.h"
#include "MsgInfoMgr.h"
#include "role.h"
#include "role_mgr.h"
#include "team.h"

#include "../common/ServerDefine/base_server_define.h"

MsgInfoMgr g_MsgInfoMgr;

MsgInfoMgr::~MsgInfoMgr()
{
	MsgInfo* pMsgInfo = (MsgInfo*)INVALID_VALUE;
	std::list<DWORD>		listMsgInfo;
	m_mapMsgInfo.copy_key_to_list(listMsgInfo);

	std::list<DWORD>::iterator it = listMsgInfo.begin();

	for (; it != listMsgInfo.end(); ++it )
	{
		pMsgInfo = m_mapMsgInfo.find(*it);
		SAFE_DELETE(pMsgInfo);
	}
	
	/*package_map<DWORD, MsgInfo*>::map_iter it = m_mapMsgInfo.Begin();
	while(m_mapMsgInfo.find_next(it, pMsgInfo))
	{
		SAFE_DELETE(pMsgInfo);
	}*/

	m_mapMsgInfo.clear();	
}

//-------------------------------------------------------------------------------------------------------
// �����ű�ͨ����Ϣ
//-------------------------------------------------------------------------------------------------------
DWORD MsgInfoMgr::BeginMsgEvent()
{
	m_Lock.Acquire();
	MsgInfo* pMsgInfo = new MsgInfo(m_dMsgInfoID);
	DWORD	dwMsgInfoID	= m_dMsgInfoID;

	m_mapMsgInfo.add(m_dMsgInfoID, pMsgInfo);

	++m_dMsgInfoID;
	m_Lock.Release();

	return dwMsgInfoID;
}

//-------------------------------------------------------------------------------------------------------
// ����Ϣ���������¼�����
//-------------------------------------------------------------------------------------------------------
VOID MsgInfoMgr::AddMsgEvent(DWORD dwMsgInfoID, EMsgUnitType eMsgUnitType, LPVOID pData)
{
	MsgInfo *pMsgInfo = m_mapMsgInfo.find(dwMsgInfoID);
	if(!VALID_POINT(pMsgInfo))
		return;

	pMsgInfo->AddMsgUnit(eMsgUnitType, pData);
}

//-------------------------------------------------------------------------------------------------------
// ���ͽű�ͨ����Ϣ�����
//-------------------------------------------------------------------------------------------------------
VOID MsgInfoMgr::DispatchRoleMsgEvent(DWORD dwMsgInfoID, Role *pRole)
{
	if(!VALID_POINT(pRole))
		return;

	MsgInfo *pMsgInfo = m_mapMsgInfo.find(dwMsgInfoID);
	if(!VALID_POINT(pMsgInfo))
		return;

	LPVOID pSend = pMsgInfo->CreateMsgByMsgInfo(g_mem_pool_safe);
	DWORD dw_size = pMsgInfo->GetMsgSize();

	pRole->SendMessage(pSend, dw_size);
	pMsgInfo->DeleteMsg(pSend, g_mem_pool_safe);

	RemoveMsgInfo(pMsgInfo);
}

//-------------------------------------------------------------------------------------------------------
// �����������е�ͼ�ڵ���ҷ��ͽű�ͨ����Ϣ
//-------------------------------------------------------------------------------------------------------
VOID MsgInfoMgr::DispatchWorldMsgEvent(DWORD dwMsgInfoID)
{
	MsgInfo *pMsgInfo = m_mapMsgInfo.find(dwMsgInfoID);
	if(!VALID_POINT(pMsgInfo))
		return;

	LPVOID pSend = pMsgInfo->CreateMsgByMsgInfo(g_mem_pool_safe);
	DWORD dw_size = pMsgInfo->GetMsgSize();

	g_roleMgr.send_world_msg(pSend, dw_size);
	pMsgInfo->DeleteMsg(pSend, g_mem_pool_safe);

	RemoveMsgInfo(pMsgInfo);
}

//-------------------------------------------------------------------------------------------------------
// ��ͬһ��ͼ�ڵ���ҷ��ͽű�ͨ����Ϣ
//-------------------------------------------------------------------------------------------------------
VOID MsgInfoMgr::DispatchMapMsgEvent(DWORD dwMsgInfoID, Map* pMap)
{
	if(!VALID_POINT(pMap))	return;

	MsgInfo *pMsgInfo = m_mapMsgInfo.find(dwMsgInfoID);
	if(!VALID_POINT(pMsgInfo))
		return;

	LPVOID pSend = pMsgInfo->CreateMsgByMsgInfo(g_mem_pool_safe);
	DWORD dw_size = pMsgInfo->GetMsgSize();

	pMap->send_map_message(pSend, dw_size);
	pMsgInfo->DeleteMsg(pSend, g_mem_pool_safe);

	RemoveMsgInfo(pMsgInfo);
}

//-------------------------------------------------------------------------------------------------------
// ��ͬһ��ͼ�ڵ�С�Ӷ��ѷ��ͽű�ͨ����Ϣ
//-------------------------------------------------------------------------------------------------------
VOID MsgInfoMgr::DispatchTeamMapMsgEvent(DWORD dwMsgInfoID, Map *pMap, Team *pTeam)
{
	if( !VALID_POINT(pTeam) ) return;
	if( !VALID_POINT(pMap) ) return;

	MsgInfo *pMsgInfo = m_mapMsgInfo.find(dwMsgInfoID);
	if(!VALID_POINT(pMsgInfo))
		return;

	LPVOID pSend = pMsgInfo->CreateMsgByMsgInfo(g_mem_pool_safe);
	DWORD dw_size = pMsgInfo->GetMsgSize();

	pTeam->send_team_msg_in_same_map(pMap->get_map_id(), pSend, dw_size);
	pMsgInfo->DeleteMsg(pSend, g_mem_pool_safe);

	RemoveMsgInfo(pMsgInfo);
}













