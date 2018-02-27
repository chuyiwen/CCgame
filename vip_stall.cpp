/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//VIP̯λ

#include "StdAfx.h"

#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/vipstall_server_define.h"

#include "../../common/WorldDefine/stall_protocol.h"

#include "world.h"
#include "db_session.h"
#include "role_mgr.h"
#include "role.h"
#include "vip_stall.h"

//-------------------------------------------------------------------------------------------------------
// ����
//-------------------------------------------------------------------------------------------------------
const DWORD MAX_PERIOD_OF_VALIDITY = 60 * 60 * 24;

//-------------------------------------------------------------------------------------------------------
// ������������
//-------------------------------------------------------------------------------------------------------
VIPStall::VIPStall()
{
	m_bInit = FALSE;
	m_vecVIPStall.clear();
	m_mapVIPStall.clear();

	m_dwSecTimer	= g_world.GetWorldTime();
	m_nMinuteTimer	= 0;
}

VIPStall::~VIPStall()
{
	Destroy();
}

//-------------------------------------------------------------------------------------------------------
// ��ʼ�������£�����
//-------------------------------------------------------------------------------------------------------
BOOL VIPStall::Init()
{
	m_vecVIPStall.resize(VIP_STALL_MAX_NUM);

	// ����̯λ�����Ϣ
	for (int n=0; n<VIP_STALL_MAX_NUM; n++)
	{
		m_vecVIPStall[n].nRent = AttRes::GetInstance()->GetVIPStallRent(n);
	}

	RegisterStallEventFunc();

	return TRUE;
}

VOID VIPStall::Update()
{
	// ��ʼ���ж�
	if (!m_bInit)
		return;

	// �����¼�
	EventMgr<VIPStall>::Update();

	// ̯λ���ڴ���
	if (g_world.GetWorldTime() != m_dwSecTimer)
	{
		m_dwSecTimer = g_world.GetWorldTime();

		for (int n=0; n<VIP_STALL_MAX_NUM; n++)
		{
			if (!VALID_VALUE(m_vecVIPStall[n].dwOwnerID))
				continue;

			m_vecVIPStall[n].nRemainTime--;
			if (m_vecVIPStall[n].nRemainTime <= 0)
			{
				// ����̯λ��Ϣ
				m_mapVIPStall.erase(m_vecVIPStall[n].dwOwnerID);

				m_vecVIPStall[n].Reset();
				m_vecVIPStall[n].dwLastUpdateTime = m_dwSecTimer;
				m_dwVIPStallChange = m_dwSecTimer;

				// ֪ͨ���ݿ�
				NET_DB2C_apply_vip_stall send;
				send.vip_stall_info.by_stall_index	= n;
				send.vip_stall_info.dw_owner_id		= INVALID_VALUE;
				send.vip_stall_info.n_remain_time	= INVALID_VALUE;
				g_dbSession.Send(&send, send.dw_size);
			}
		}

		// ÿ���ӷ���ʱ�������Ϣ
		if (++m_nMinuteTimer > 60)
		{
			NET_DB2C_update_vip_stall send;
			g_dbSession.Send(&send, send.dw_size);
			m_nMinuteTimer = 0;
		}
	}
}

VOID VIPStall::Destroy()
{
	m_vecVIPStall.clear();
	m_mapVIPStall.clear();
	m_bInit = FALSE;
}

//-------------------------------------------------------------------------------------------------------
// �¼�ע��ʹ���
//-------------------------------------------------------------------------------------------------------
VOID VIPStall::RegisterStallEventFunc()
{
	RegisterEventFunc(EVT_SetStallTitle,			&VIPStall::OnSetStallTitleFunc);
	RegisterEventFunc(EVT_SetStallStatus,			&VIPStall::OnSetStallStatusFunc);
	RegisterEventFunc(EVT_ChangeStallGoods,			&VIPStall::OnChangeStallGoodsFunc);
}

VOID VIPStall::OnSetStallTitleFunc( DWORD dwSenderID, VOID* pEventMessage )
{
	// pEventMessage <==> TCHAR*

	tagVIPStall* pInfo = GetVIPStallInfo(dwSenderID);

	if (!VALID_POINT(pInfo))
		return;

	// �ϲ㱣֤���ַ����Ľ�����
	TCHAR* pTitle = (TCHAR*)pEventMessage;

	// ��ȡ����޶ȵ��ַ���
	_tcsncpy(pInfo->szStallTitle, pTitle, STALL_MAX_TITLE_NUM-1);

	// ���ý�����
	pInfo->szStallTitle[STALL_MAX_TITLE_NUM-1] = '\0';

	pInfo->dwLastUpdateTime = m_dwSecTimer;
	m_dwVIPStallChange		= m_dwSecTimer;
}

VOID VIPStall::OnSetStallStatusFunc( DWORD dwSenderID, VOID* pEventMessage )
{
	// pEventMessage <==> BYTE*

	tagVIPStall* pInfo = GetVIPStallInfo(dwSenderID);

	if (!VALID_POINT(pInfo))
		return;

	pInfo->eStallStatus		= (EVIPStallStatus)(*((BYTE*)pEventMessage));
	pInfo->dwLastUpdateTime	= m_dwSecTimer;
	m_dwVIPStallChange		= m_dwSecTimer;
}

VOID VIPStall::OnChangeStallGoodsFunc( DWORD dwSenderID, VOID* pEventMessage )
{
	// pEventMessage <==> NULL

	tagVIPStall* pInfo = GetVIPStallInfo(dwSenderID);

	if (!VALID_POINT(pInfo))
		return;

	pInfo->dwLastGoodsTime = m_dwSecTimer;
}

//-------------------------------------------------------------------------------------------------------
// �����ݿ��ȡVIP̯λ��Ϣ
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::LoadAllVIPStallInfo( s_vip_stall_to_db* pVIPStallInfo )
{
	ASSERT(pVIPStallInfo);

	if (!VALID_POINT(pVIPStallInfo))
		return INVALID_VALUE;

	for (int n=0; n<VIP_STALL_MAX_NUM; n++)
	{
		m_vecVIPStall[n].byStallIndex		= pVIPStallInfo[n].by_stall_index;
		m_vecVIPStall[n].dwOwnerID			= pVIPStallInfo[n].dw_owner_id;
		m_vecVIPStall[n].nRemainTime		= pVIPStallInfo[n].n_remain_time;
		m_vecVIPStall[n].dwLastUpdateTime	= m_dwSecTimer;
		m_dwVIPStallChange					= m_dwSecTimer;

		if (VALID_VALUE(pVIPStallInfo[n].dw_owner_id))
		{
			m_mapVIPStall.add(pVIPStallInfo[n].dw_owner_id, n);

			Role* pRole = g_roleMgr.get_role(pVIPStallInfo[n].dw_owner_id);
			if (VALID_POINT(pRole) && pRole->IsInRoleState(ERS_Stall))
			{
				m_vecVIPStall[n].eStallStatus = EVSS_Open;
				m_vecVIPStall[n].dwLastGoodsTime = m_dwSecTimer;
				pRole->GetStallTitle(m_vecVIPStall[n].szStallTitle);
			}
			else
			{
				m_vecVIPStall[n].eStallStatus = EVSS_Close;
			}
		}
		else
		{
			m_vecVIPStall[n].eStallStatus = EVSS_ForHire;
		}
	}
	m_bInit = TRUE;

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// ȡ��VIP̯λ��Ҫ��Ϣ
//-------------------------------------------------------------------------------------------------------
tagVIPStall* VIPStall::GetVIPStallInfo( BYTE index )
{
	//ASSERT( index >=0 && index < VIP_STALL_MAX_NUM);

	if (index < 0 || index >= VIP_STALL_MAX_NUM)
		return NULL;

	if (!m_bInit)	return NULL;
	
	return &(m_vecVIPStall[index]);
}

tagVIPStall* VIPStall::GetVIPStallInfo( DWORD dw_role_id )
{
	if (!VALID_VALUE(dw_role_id))
		return NULL;

	if(!m_bInit)	
		return NULL;

	BYTE byIndex = m_mapVIPStall.find(dw_role_id);

	if (!VALID_VALUE(byIndex))
		return NULL;
	
	return GetVIPStallInfo(byIndex);
}

Role* VIPStall::GetVIPStallRole( BYTE index )
{
	tagVIPStall* pInfo = GetVIPStallInfo(index);

	// ̯λ������
	if (!VALID_POINT(pInfo))
		return NULL;

	// ̯λ����
	if (!VALID_VALUE(pInfo->dwOwnerID))
		return NULL;

	return g_roleMgr.get_role(pInfo->dwOwnerID);
}

BOOL VIPStall::IsInStatus( BYTE index, EVIPStallStatus eStatus )
{
	tagVIPStall* pInfo = GetVIPStallInfo(index);
	if (!VALID_POINT(pInfo))
		return FALSE;

	if (pInfo->eStallStatus == eStatus)
		return TRUE;

	return FALSE;
}

DWORD VIPStall::GetSpecVIPStallGoodsTime( BYTE index )
{
	tagVIPStall* pInfo = GetVIPStallInfo(index);
	if (!VALID_POINT(pInfo))
		return INVALID_VALUE;

	return pInfo->dwLastGoodsTime;
}

DWORD VIPStall::SetVIPStallStatus( BYTE index, EVIPStallStatus eStatus )
{
	tagVIPStall* pInfo = GetVIPStallInfo(index);
	if (!VALID_POINT(pInfo))
	{
		return INVALID_VALUE;
	}

	pInfo->eStallStatus = eStatus;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��ͼ�ϲ���Ϣ����
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
// ����VIP̯λ
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::ApplyVIPStall( Role* pRole, BYTE byIndex, INT32 nRent )
{
	if (!VALID_POINT(pRole))
		return INVALID_VALUE;

	if (!m_bInit)
		return E_Stall_VIP_Not_Init;

	// ��������Ƿ��Ѿ����޹�VIP̯λ
	if (m_mapVIPStall.is_exist(pRole->GetID()))
	{
		return E_Stall_VIP_Own_Another;
	}

	// ȡ��̯λ��Ϣ
	tagVIPStall* pInfo = GetVIPStallInfo(byIndex);
	if (!VALID_POINT(pInfo))
		return E_Stall_VIP_Info_NotExist;

	// ����̯λ�Ƿ��Ѿ�������
	if (VALID_VALUE(pInfo->dwOwnerID))
	{
		return E_Stall_VIP_Already_Rent;
	}

	if (pInfo->nRent != nRent)
		return E_Stall_VIP_Rent_Error;

	// ������Ԫ���Ƿ��㹻
	if(nRent > pRole->GetCurMgr().GetBaiBaoYuanBao() || nRent <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	// �۳����Ԫ��
	pRole->GetCurMgr().DecBaiBaoYuanBao(nRent, elcid_vip_stall_rent);

	// ����ɹ�
	pInfo->dwOwnerID		= pRole->GetID();
	pInfo->nRemainTime		= MAX_PERIOD_OF_VALIDITY;
	pInfo->dwLastUpdateTime	= m_dwSecTimer;
	m_dwVIPStallChange		= m_dwSecTimer;

	m_mapVIPStall.add(pInfo->dwOwnerID, pInfo->byStallIndex);


	if (pRole->IsInRoleState(ERS_Stall))
	{
		pInfo->eStallStatus = EVSS_Open;
		pInfo->dwLastGoodsTime = m_dwSecTimer;
		pRole->GetStallTitle(pInfo->szStallTitle);
	}
	else
	{
		pInfo->eStallStatus = EVSS_Close;
	}

	// ֪ͨ���ݿ�
	NET_DB2C_apply_vip_stall send;
	send.vip_stall_info.by_stall_index	= pInfo->byStallIndex;
	send.vip_stall_info.dw_owner_id		= pInfo->dwOwnerID;
	send.vip_stall_info.n_remain_time	= pInfo->nRemainTime;
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// �ͻ��˻�ȡ����VIP̯λ��Ҫ��Ϣ
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::GetAllVIPStallInfo( OUT tagVIPStall* pStallInfo, OUT DWORD& dw_time )
{
	if (!VALID_POINT(pStallInfo))
		return INVALID_VALUE;
	
	if (!m_bInit)
		return E_Stall_VIP_Not_Init;

	memcpy(pStallInfo, &m_vecVIPStall[0], sizeof(tagVIPStall)*VIP_STALL_MAX_NUM);
	dw_time = m_dwVIPStallChange;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// �ͻ��˻�ȡĳʱ������µ�����VIP̯λ��Ҫ��Ϣ
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::GetUpdatedStallInfo( OUT tagVIPStall* pStallInfo, INT32& n_num, DWORD& dw_time )
{
	if (!VALID_POINT(pStallInfo))
		return INVALID_VALUE;

	if (!m_bInit)
		return E_Stall_VIP_Not_Init;

	n_num = 0;

	if (dw_time >= m_dwVIPStallChange)
		return E_Success;

	for (int n=0; n<VIP_STALL_MAX_NUM; n++)
	{
		if (dw_time < m_vecVIPStall[n].dwLastUpdateTime)
		{
			memcpy(&(pStallInfo[n_num++]), &(m_vecVIPStall[n]), sizeof(tagVIPStall));
		}
	}
	dw_time = m_dwVIPStallChange;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// �ͻ��˻�ȡĳVIP̯λ��Ʒ��Ϣ
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::GetVIPStallGoods( OUT LPVOID pData, OUT BYTE &byGoodsNum, OUT INT &nGoodsSz, DWORD& dwRequestTime, BYTE byIndex )
{
	byGoodsNum = 0;
	nGoodsSz = 0;

	// ȡ��̯λ��Ҫ��Ϣ
	tagVIPStall* pInfo = GetVIPStallInfo(byIndex);
	if (!VALID_POINT(pInfo))
	{
		return E_Stall_VIP_Info_NotExist;
	}

	// �ж�̯λ�Ƿ���Ӫҵ��
	switch (pInfo->eStallStatus)
	{
	case EVSS_ForHire:
		return E_Stall_VIP_For_Hire;

	case EVSS_Close:
		return E_Stall_Role_Pull;
	}
	
	// �жϸ�̯λ��Ʒ�Ƿ���¹�
	if (dwRequestTime == pInfo->dwLastGoodsTime)
	{
		return E_Stall_VIP_GetGoods_Needless;
	}

	Role* pStallRole = g_roleMgr.get_role(pInfo->dwOwnerID);
	if (!VALID_POINT(pStallRole))
	{
		// �ϲ��Ѿ��жϹ��ˣ�����Ϊ�˱���
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pStallRole->GetStallGoods(pData, byGoodsNum, nGoodsSz);

	if (dw_error_code == E_Success)
	{
		dwRequestTime = pInfo->dwLastGoodsTime;
	}
	
	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
// ɾ����ɫʱ��������
//-------------------------------------------------------------------------------------------------------
VOID VIPStall::RemoveRoleVIPStall( DWORD dw_role_id )
{
	tagVIPStall* pInfo = GetVIPStallInfo(dw_role_id);
	if (!VALID_POINT(pInfo))
	{
		return;
	}

	// ����̯λ��Ϣ
	m_mapVIPStall.erase(pInfo->dwOwnerID);

	pInfo->Reset();
	pInfo->dwLastUpdateTime = m_dwSecTimer;
	m_dwVIPStallChange		= m_dwSecTimer;

	// ֪ͨ���ݿ�
	NET_DB2C_apply_vip_stall send;
	send.vip_stall_info.by_stall_index	= pInfo->byStallIndex;
	send.vip_stall_info.dw_owner_id		= INVALID_VALUE;
	send.vip_stall_info.n_remain_time	= INVALID_VALUE;
	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// ȫ�ֱ���
//-------------------------------------------------------------------------------------------------------
VIPStall g_VIPStall;
