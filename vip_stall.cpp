/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//VIP摊位

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
// 常量
//-------------------------------------------------------------------------------------------------------
const DWORD MAX_PERIOD_OF_VALIDITY = 60 * 60 * 24;

//-------------------------------------------------------------------------------------------------------
// 构造析构函数
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
// 初始化，更新，销毁
//-------------------------------------------------------------------------------------------------------
BOOL VIPStall::Init()
{
	m_vecVIPStall.resize(VIP_STALL_MAX_NUM);

	// 载入摊位租金信息
	for (int n=0; n<VIP_STALL_MAX_NUM; n++)
	{
		m_vecVIPStall[n].nRent = AttRes::GetInstance()->GetVIPStallRent(n);
	}

	RegisterStallEventFunc();

	return TRUE;
}

VOID VIPStall::Update()
{
	// 初始化判断
	if (!m_bInit)
		return;

	// 处理事件
	EventMgr<VIPStall>::Update();

	// 摊位到期处理
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
				// 重置摊位信息
				m_mapVIPStall.erase(m_vecVIPStall[n].dwOwnerID);

				m_vecVIPStall[n].Reset();
				m_vecVIPStall[n].dwLastUpdateTime = m_dwSecTimer;
				m_dwVIPStallChange = m_dwSecTimer;

				// 通知数据库
				NET_DB2C_apply_vip_stall send;
				send.vip_stall_info.by_stall_index	= n;
				send.vip_stall_info.dw_owner_id		= INVALID_VALUE;
				send.vip_stall_info.n_remain_time	= INVALID_VALUE;
				g_dbSession.Send(&send, send.dw_size);
			}
		}

		// 每分钟发送时间更新信息
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
// 事件注册和处理
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

	// 上层保证了字符串的结束符
	TCHAR* pTitle = (TCHAR*)pEventMessage;

	// 截取最大限度的字符串
	_tcsncpy(pInfo->szStallTitle, pTitle, STALL_MAX_TITLE_NUM-1);

	// 重置结束符
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
// 从数据库获取VIP摊位信息
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
// 取得VIP摊位概要信息
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

	// 摊位不存在
	if (!VALID_POINT(pInfo))
		return NULL;

	// 摊位无主
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
// 地图上层消息处理
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
// 申请VIP摊位
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::ApplyVIPStall( Role* pRole, BYTE byIndex, INT32 nRent )
{
	if (!VALID_POINT(pRole))
		return INVALID_VALUE;

	if (!m_bInit)
		return E_Stall_VIP_Not_Init;

	// 检测该玩家是否已经租赁过VIP摊位
	if (m_mapVIPStall.is_exist(pRole->GetID()))
	{
		return E_Stall_VIP_Own_Another;
	}

	// 取出摊位信息
	tagVIPStall* pInfo = GetVIPStallInfo(byIndex);
	if (!VALID_POINT(pInfo))
		return E_Stall_VIP_Info_NotExist;

	// 检测该摊位是否已经被租赁
	if (VALID_VALUE(pInfo->dwOwnerID))
	{
		return E_Stall_VIP_Already_Rent;
	}

	if (pInfo->nRent != nRent)
		return E_Stall_VIP_Rent_Error;

	// 检查玩家元宝是否足够
	if(nRent > pRole->GetCurMgr().GetBaiBaoYuanBao() || nRent <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	// 扣除玩家元宝
	pRole->GetCurMgr().DecBaiBaoYuanBao(nRent, elcid_vip_stall_rent);

	// 申请成功
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

	// 通知数据库
	NET_DB2C_apply_vip_stall send;
	send.vip_stall_info.by_stall_index	= pInfo->byStallIndex;
	send.vip_stall_info.dw_owner_id		= pInfo->dwOwnerID;
	send.vip_stall_info.n_remain_time	= pInfo->nRemainTime;
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 客户端获取所有VIP摊位概要信息
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
// 客户端获取某时间点后更新的所有VIP摊位概要信息
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
// 客户端获取某VIP摊位商品信息
//-------------------------------------------------------------------------------------------------------
DWORD VIPStall::GetVIPStallGoods( OUT LPVOID pData, OUT BYTE &byGoodsNum, OUT INT &nGoodsSz, DWORD& dwRequestTime, BYTE byIndex )
{
	byGoodsNum = 0;
	nGoodsSz = 0;

	// 取得摊位概要信息
	tagVIPStall* pInfo = GetVIPStallInfo(byIndex);
	if (!VALID_POINT(pInfo))
	{
		return E_Stall_VIP_Info_NotExist;
	}

	// 判断摊位是否在营业中
	switch (pInfo->eStallStatus)
	{
	case EVSS_ForHire:
		return E_Stall_VIP_For_Hire;

	case EVSS_Close:
		return E_Stall_Role_Pull;
	}
	
	// 判断该摊位物品是否更新过
	if (dwRequestTime == pInfo->dwLastGoodsTime)
	{
		return E_Stall_VIP_GetGoods_Needless;
	}

	Role* pStallRole = g_roleMgr.get_role(pInfo->dwOwnerID);
	if (!VALID_POINT(pStallRole))
	{
		// 上层已经判断过了，这里为了保险
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
// 删除角色时的清理动作
//-------------------------------------------------------------------------------------------------------
VOID VIPStall::RemoveRoleVIPStall( DWORD dw_role_id )
{
	tagVIPStall* pInfo = GetVIPStallInfo(dw_role_id);
	if (!VALID_POINT(pInfo))
	{
		return;
	}

	// 重置摊位信息
	m_mapVIPStall.erase(pInfo->dwOwnerID);

	pInfo->Reset();
	pInfo->dwLastUpdateTime = m_dwSecTimer;
	m_dwVIPStallChange		= m_dwSecTimer;

	// 通知数据库
	NET_DB2C_apply_vip_stall send;
	send.vip_stall_info.by_stall_index	= pInfo->byStallIndex;
	send.vip_stall_info.dw_owner_id		= INVALID_VALUE;
	send.vip_stall_info.n_remain_time	= INVALID_VALUE;
	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 全局变量
//-------------------------------------------------------------------------------------------------------
VIPStall g_VIPStall;
