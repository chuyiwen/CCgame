/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/
//����Ź�


#include "StdAfx.h"

#include "../WorldDefine/mall_define.h"
#include "../WorldDefine/msg_mall.h"
#include "../WorldDefine/ItemDefine.h"

#include "../ServerDefine/log_server_define.h"
#include "../ServerDefine/mall_server_define.h"
#include "../ServerDefine/log_server_define.h"

#include "guild_purchase.h"
#include "guild.h"
#include "att_res.h"
#include "item_creator.h"
#include "mall.h"
#include "role_mgr.h"
#include "currency.h"
#include "role.h"
#include "guild_manager.h"

//-------------------------------------------------------------------------------------------------------
// ����������
//-------------------------------------------------------------------------------------------------------
GuildPurchase::GuildPurchase()
{
	m_pGuild = NULL;
	m_mapGPInfo.clear();
}

GuildPurchase::~GuildPurchase()
{
	Destory();
}

//-------------------------------------------------------------------------------------------------------
// ��ʼ�������¡�����
//-------------------------------------------------------------------------------------------------------
BOOL GuildPurchase::Init( DWORD dwGuildID )
{
	m_mapGPInfo.clear();

	// �ϲ��豣֤pGuild�ĺϷ���
	ASSERT(VALID_VALUE(dwGuildID));
	m_pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(m_pGuild))
	{
		return FALSE;
	}

	return TRUE;
}

VOID GuildPurchase::Update(DWORD dw_time)
{
	INT64 nMapKey = INVALID_VALUE;
	tagGroupPurchase* pGPInfo = NULL;

	MapGPInfo::map_iter iter = m_mapGPInfo.begin();
	while (m_mapGPInfo.find_next(iter, nMapKey, pGPInfo))
	{
		// ����24Сʱ���Ź�ʧ��
		pGPInfo->dwRemainTime -= dw_time;
		if ((INT32)(pGPInfo->dwRemainTime) <= 0)
		{
			RemoveGroupPurchaseInfo(pGPInfo, FALSE);
		}
	}

	// �ϲ��ж�map�Ƿ�Ϊ��
}

VOID GuildPurchase::Destory()
{
	tagGroupPurchase* pGPInfo = NULL;
	MapGPInfo::map_iter iter = m_mapGPInfo.begin();
	while (m_mapGPInfo.find_next(iter, pGPInfo))
	{
		SAFE_DELETE(pGPInfo);
	}
	m_mapGPInfo.clear();
}

//-------------------------------------------------------------------------------------------------------
// �Ź���Ϣ����
//-------------------------------------------------------------------------------------------------------
BOOL GuildPurchase::Add( tagGroupPurchase* pGPInfo, BOOL bNotify2DB /*= TRUE*/ )
{
	ASSERT(pGPInfo);
	if (!VALID_POINT(pGPInfo))
	{
		return FALSE;
	}

	// ��֤����
	if (pGPInfo->dwGuildID != m_pGuild->get_guild_att().dwID)
	{
		return FALSE;
	}

	// ����
	INT64 nKey = GetKey(pGPInfo->dw_role_id, pGPInfo->dwMallID);
	if (m_mapGPInfo.is_exist(nKey))
	{
		return FALSE;
	}

	m_mapGPInfo.add(nKey, pGPInfo);

	// ֪ͨ���ݿ�
	if (bNotify2DB)
	{
		AddGPInfo2DB(pGPInfo);
	}

	return TRUE;
}

BOOL GuildPurchase::Remove( tagGroupPurchase* pGPInfo, BOOL bNotify2DB /*= TRUE*/ )
{
	ASSERT(pGPInfo);
	if (!VALID_POINT(pGPInfo))
	{
		return FALSE;
	}

	// ��֤����
	if (pGPInfo->dwGuildID != m_pGuild->get_guild_att().dwID)
	{
		return FALSE;
	}

	// ����
	BOOL bRet = m_mapGPInfo.erase(GetKey(pGPInfo->dw_role_id, pGPInfo->dwMallID));

	// ֪ͨ���ݿ�
	if (bRet && bNotify2DB)
	{
		RemoveGPInfo2DB(pGPInfo);
	}

	return bRet;
}

//-------------------------------------------------------------------------------------------------------
// ��ȡ�Ź���Ϣ
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::GetAllSimGPInfo( INT &nGPInfoNum, tagSimGPInfo* pData )
{
	if (m_mapGPInfo.empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	tagGroupPurchase* pGPInfo = NULL;
	nGPInfoNum = 0;

	MapGPInfo::map_iter iter = m_mapGPInfo.begin();
	while (m_mapGPInfo.find_next(iter, pGPInfo))
	{
		if (!VALID_POINT(pGPInfo))
		{
			// ������
			ASSERT(pGPInfo);
			print_message(_T("\nthere is a error GP Info in map.\n"));
			continue;
		}
		pData[nGPInfoNum].dwGuildID			= pGPInfo->dwGuildID;
		pData[nGPInfoNum].dw_role_id			= pGPInfo->dw_role_id;
		pData[nGPInfoNum].dwMallID			= pGPInfo->dwMallID;
		pData[nGPInfoNum].nPrice			= pGPInfo->nPrice;
		pData[nGPInfoNum].nParticipatorNum	= pGPInfo->nParticipatorNum;
		pData[nGPInfoNum].nRequireNum		= pGPInfo->nRequireNum;
		pData[nGPInfoNum].dwRemainTime		= pGPInfo->dwRemainTime;

		nGPInfoNum++;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��ȡָ���Ź�����Ӧ���б�
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::GetParticipators( DWORD dwID, DWORD dw_role_id, DWORD *pData )
{
	if (m_mapGPInfo.empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	tagGroupPurchase* pGPInfo = NULL;
	INT64 nMapKey = GetKey(dw_role_id, dwID);

	pGPInfo = m_mapGPInfo.find(nMapKey);

	if (!VALID_POINT(pGPInfo) || !VALID_POINT(pGPInfo->listParticipators))
	{
		return E_GroupPurchase_NoInfo;
	}

	// û����Ӧ�ߵ��Ź����ݣ���Ӧ�ô���
	if (pGPInfo->listParticipators->empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	INT i = 0;
	pGPInfo->listParticipators->reset_iterator();
	while (pGPInfo->listParticipators->find_next(pData[i++]));

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��ȡ�Ź���Ϣ����
//-------------------------------------------------------------------------------------------------------
INT GuildPurchase::GetGroupPurchaseInfoNum()
{
	return m_mapGPInfo.size();
}

//-------------------------------------------------------------------------------------------------------
// ��ȡ��Ӧ������
//-------------------------------------------------------------------------------------------------------
INT GuildPurchase::GetParticipatorNum( DWORD dwID, DWORD dw_role_id )
{
	if (m_mapGPInfo.empty())
	{
		return 0;
	}

	tagGroupPurchase* pGPInfo = NULL;
	INT64 nMapKey = GetKey(dw_role_id, dwID);

	pGPInfo = m_mapGPInfo.find(nMapKey);

	if (!VALID_POINT(pGPInfo) || !VALID_POINT(pGPInfo->listParticipators))
	{
		return 0;
	}

	return pGPInfo->listParticipators->size();
}

//-------------------------------------------------------------------------------------------------------
// �����Ź�
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::LaunchGroupPurchase( Role *pRole, DWORD dwID, BYTE byScope, BYTE byIndex, INT nUnitPrice, OUT tagGroupPurchase* &pGPInfo, OUT DWORD& dwItemTypeID )
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(g_mall.is_init_ok());
	ASSERT(nUnitPrice > 0);

	DWORD dw_error_code = E_Success;

	// ��������Ϸ���
	const tagMallItemProto *pProto = g_mall.GetMallItem(byIndex, EMIT_Item)->pMallItem;
	if (!VALID_POINT(pProto))
	{
		return INVALID_VALUE;
	}

	// ������Ʒ�Ƿ�����Ź�
	switch(byScope)
	{
	case EGPS_SMALL:
		if (pProto->bySmallGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}
		break;

	case EGPS_MEDIUM:
		if (pProto->byMediumGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}
		break;

	case EGPS_BIG:
		if (pProto->byBigGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}
		break;

	default:
		return INVALID_VALUE;
		break;
	}

	// �ϲ㸺�����������
	INT32 nGroupPurchase = m_pGuild->get_guild_att().nGroupPurchase;

	// id
	if(pProto->dwID != dwID)
	{
		return E_Mall_ID_Error;
	}

	// ����Ƿ��Ѿ�����������Ʒ���Ź�
	INT64 nMapKey = GetKey(pRole->GetID(), dwID);
	if (m_mapGPInfo.is_exist(nMapKey))
		return E_GroupPurchase_AlreadyInitiate;

	INT nPrice = 0;

	// ����
	if(pProto->dwTimeSaleEnd != INVALID_VALUE && g_world.GetWorldTime() < pProto->dwTimeSaleEnd)
	{
		// ����
		nPrice = pProto->nSalePrice;
	}
	else
	{
		// ����
		nPrice = pProto->nPrice;
	}

	// ��Ǯ
	if(nPrice != nUnitPrice)
	{
		return E_Mall_YuanBao_Error;
	}

	// �����Ź��۸�
	nPrice *= pProto->byGroupPurchaseAmount;

	switch(byScope)
	{
	case EGPS_SMALL:
		nPrice = (INT)(nPrice * (pProto->bySmallGroupDiscount/100.0f - 0.2f*nGroupPurchase/100000));
		break;

	case EGPS_MEDIUM:
		nPrice = (INT)(nPrice * (pProto->byMediumGroupDiscount/100.0f - 0.2f*nGroupPurchase/100000));
		break;

	case EGPS_BIG:
		nPrice = (INT)(nPrice * (pProto->byBigGroupDiscount/100.0f - 0.2f*nGroupPurchase/100000));
		break;
	}

	// ������Ԫ���Ƿ��㹻
	if(nPrice > pRole->GetCurMgr().GetBagYuanBao() || nPrice <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	// ���ô�������
	pGPInfo = new tagGroupPurchase;
	if (!VALID_POINT(pGPInfo))
	{
		ASSERT(pGPInfo);
		return INVALID_VALUE;
	}

	pGPInfo->dwGuildID		= pRole->GetGuildID();
	pGPInfo->dw_role_id		= pRole->GetID();
	pGPInfo->dwMallID		= dwID;
	pGPInfo->nPrice			= nPrice;
	pGPInfo->dwRemainTime	= pProto->dwPersistTime * 60 * 60; // ����Ϊ��λ

	switch (byScope)
	{
	case EGPS_SMALL:
		pGPInfo->nRequireNum = pProto->bySmallGroupHeadcount;
		break;

	case EGPS_MEDIUM:
		pGPInfo->nRequireNum = pProto->byMediumGroupHeadcount;
		break;

	case EGPS_BIG:
		pGPInfo->nRequireNum = pProto->byBigGroupHeadcount;
		break;
	}
	pGPInfo->nParticipatorNum = 1;
	pGPInfo->listParticipators = new package_list<DWORD>;
	if (!VALID_POINT(pGPInfo->listParticipators))
	{
		ASSERT(pGPInfo->listParticipators);
		SAFE_DELETE(pGPInfo);
		return INVALID_VALUE;
	}
	pGPInfo->listParticipators->push_back(pGPInfo->dw_role_id);

	// �����Ź���Ϣ��������
	Add(pGPInfo);

	// ������ƷTypeID
	dwItemTypeID = pProto->dw_data_id;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��Ӧ�Ź�
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::RespondGroupPurchase( Role *pRole, DWORD dwID, DWORD dw_role_id, INT nPrice, OUT tagGroupPurchase* &pGPInfo )
{
	// ���������Ϣ
	if (!VALID_VALUE(pRole->GetGuildID()))
		return E_GroupPurchase_NotInGuild;

	if (!VALID_POINT(m_pGuild->get_member(pRole->GetID())))
		return E_GroupPurchase_NotMember;

	// �����Ƿ��Ƿ�����
	if (dw_role_id == pRole->GetID())
	{
		return E_GroupPurchase_IsInitiate;
	}

	// ����ָ�����Ź��Ƿ��Ѿ�����
	INT64 nMapKey = GetKey(dw_role_id, dwID);
	pGPInfo = m_mapGPInfo.find(nMapKey);

	if (!VALID_POINT(pGPInfo) || (INT32)(pGPInfo->dwRemainTime) <= 0)
		return E_GroupPurchase_AlreadyEnd;

	// �Ƿ���ϢӦ��ɾ��
	if (!VALID_POINT(pGPInfo->listParticipators))
		return E_GroupPurchase_AlreadyEnd;

	// �����Ƿ��Ѿ���Ӧ������Ź�
	DWORD dwTmpRoleID = pRole->GetID();
	if (pGPInfo->listParticipators->is_exist(dwTmpRoleID))
	{
		return E_GroupPurchase_AlreadyInitiate;
	}

	// ��Ǯ
	if(nPrice != pGPInfo->nPrice)
	{
		return E_Mall_YuanBao_Error;
	}

	// ������Ԫ���Ƿ��㹻
	if(nPrice > pRole->GetCurMgr().GetBagYuanBao() || nPrice <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// �����Ź���Ʒ
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::CreateGPItems( DWORD dwID, OUT tagMallItemSell &itemSell )
{
	ASSERT(VALID_VALUE(dwID));

	//��֤��Ʒ�ĺϷ���
	const tagMallItemProto *pProto = g_attRes.GetMallItemProto(dwID);

	if (!VALID_POINT(pProto))
	{
		return E_Mall_ID_Error;
	}

	// ������Ʒ
	tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, pProto->dw_data_id, pProto->byGroupPurchaseAmount, EIQ_Quality0);
	if(!VALID_POINT(pItemNew))
	{
		ASSERT(VALID_POINT(pItemNew));
		return E_Mall_CreateItem_Failed;
	}

	// �������Ʒ����������Ʒ
	tagItem *pPresentNew = NULL;
	if(pProto->dwPresentID != INVALID_VALUE)
	{
		pPresentNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
			pProto->dwPresentID, (INT16)pProto->byPresentNum * pProto->byGroupPurchaseAmount, EIQ_Quality0);
		if(!VALID_POINT(pPresentNew))
		{
			::Destroy(pItemNew);
			ASSERT(VALID_POINT(pPresentNew));
			return E_Mall_CreatePres_Failed;
		}
	}

	// ��������
	if (pProto->byExAssign >= 0)
	{
		itemSell.nExVolumeAssign = pProto->byExAssign;
	}

	// ���ô�������
	itemSell.pItem			= pItemNew;
	itemSell.pPresent		= pPresentNew;
	itemSell.nYuanBaoNeed	= 0;				// �Ѿ�Ԥ����
	itemSell.byRemainNum	= INVALID_VALUE;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ɾ��ָ���Ź���Ϣ
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::RemoveGroupPurchaseInfo( tagGroupPurchase* &pGPInfo, BOOL bSuccess /*= TRUE*/ )
{
	ASSERT(pGPInfo);

	if (!VALID_POINT(pGPInfo))
		return;

	// ɾ���������е���Ϣ
	INT64 nMapKey = GetKey(pGPInfo->dw_role_id, pGPInfo->dwMallID);
	m_mapGPInfo.erase(nMapKey);

	// �����Ź�ʧ���򷵻����Ԫ��
	if (!bSuccess)
	{
		ReturnCost2Participator(pGPInfo);

		m_pGuild->modify_group_exponent(FALSE);

		// �㲥�Ź�ʧ����Ϣ
		const tagMallItemProto *pProto = g_attRes.GetMallItemProto(pGPInfo->dwMallID);

		if (!VALID_POINT(pProto))
		{
			// ���ﲻ�ᷢ���ɣ��Է���һ
			ASSERT(pProto);
			return;
		}

		tagNS_RespondBroadCast send;
		send.eType = ERespondBroadCast_Lose;
		send.dw_role_id = pGPInfo->dw_role_id;
		send.dw_data_id = pProto->dw_data_id;
		m_pGuild->send_guild_message(&send, send.dw_size);
	}

	// ɾ�����ݿ��е���Ϣ
	RemoveGPInfo2DB(pGPInfo);

	// ɾ����Ϣ����
	SAFE_DELETE(pGPInfo);

	// �ϲ����Ƿ�ɾ��this����
}

//-------------------------------------------------------------------------------------------------------
// ɾ���ð��������Ź���Ϣ(���ɽ�ɢ)
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::RemoveGroupPurchaseInfo()
{
	// �ð��������Ź�ʧ��
	tagGroupPurchase *pGPInfo = NULL;
	if (!m_mapGPInfo.empty())
	{
		MapGPInfo::map_iter iter = m_mapGPInfo.begin();
		while (m_mapGPInfo.find_next(iter, pGPInfo))
		{
			ReturnCost2Participator(pGPInfo);
			SAFE_DELETE(pGPInfo);
		}
	}
	m_mapGPInfo.clear();

	// ɾ�����ݿ��иð��ɵ��Ź���Ϣ
	RemoveGuildGPInfo2DB();

	// �ϲ����Ƿ���Ҫɾ��this����
}

//-------------------------------------------------------------------------------------------------------
// �������Ԫ��(�Ź�ʧ��)
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::ReturnCost2Participator( tagGroupPurchase* pGPInfo )
{
	if (!VALID_POINT(pGPInfo) || !VALID_POINT(pGPInfo->listParticipators))
		return;

	DWORD tmpRoleID = INVALID_VALUE;

	pGPInfo->listParticipators->reset_iterator();
	while(pGPInfo->listParticipators->find_next(tmpRoleID))
	{
		// �������ǹ�����
		Role* pRole = g_roleMgr.GetRolePtrByID(tmpRoleID);
		if (!VALID_POINT(pRole))
		{
			// ��������ҷ���Ԫ��
			CurrencyMgr::ModifyBaiBaoYuanBao(tmpRoleID, pGPInfo->nPrice, elcid_group_purchase_faild);
		}
		else
		{
			// ��������ҷ���Ԫ��
			pRole->GetCurMgr().IncBaiBaoYuanBao(pGPInfo->nPrice, elcid_group_purchase_faild);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// ���ݿ����
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::AddGPInfo2DB( tagGroupPurchase* pGPInfo )
{
	ASSERT(VALID_POINT(pGPInfo));
	if (!VALID_POINT(pGPInfo))
		return;

	tagNDBC_AddGPInfo send;

	send.gp_info.dw_guild_id			= pGPInfo->dwGuildID;
	send.gp_info.dw_role_id			= pGPInfo->dw_role_id;
	send.gp_info.dw_mall_id			= pGPInfo->dwMallID;
	send.gp_info.n_price				= pGPInfo->nPrice;
	send.gp_info.n_participator_num	= pGPInfo->nParticipatorNum;
	send.gp_info.n_require_num			= pGPInfo->nRequireNum;
	send.gp_info.n_remain_time			= pGPInfo->dwRemainTime;
	send.gp_info.dw_participators[0]	= pGPInfo->dw_role_id;

	g_dbSession.Send(&send, send.dw_size);
}

VOID GuildPurchase::UpdateGPInfo2DB( tagGroupPurchase* pGPInfo )
{
	ASSERT(VALID_POINT(pGPInfo));
	if (!VALID_POINT(pGPInfo))
		return;

	tagNDBC_UpdateGPInfo send;

	send.gp_info_key_.dw_guild_id	= pGPInfo->dwGuildID;
	send.gp_info_key_.dw_role_id		= pGPInfo->dw_role_id;
	send.gp_info_key_.dw_mall_id		= pGPInfo->dwMallID;
	send.dw_new_participator_		= pGPInfo->listParticipators->get_list().back();

	g_dbSession.Send(&send, send.dw_size);
}

VOID GuildPurchase::RemoveGPInfo2DB( tagGroupPurchase* pGPInfo )
{
	ASSERT(VALID_POINT(pGPInfo));
	if (!VALID_POINT(pGPInfo))
		return;

	tagNDBC_RemoveGPInfo send;

	send.gp_info_key_.dw_guild_id	= pGPInfo->dwGuildID;
	send.gp_info_key_.dw_role_id		= pGPInfo->dw_role_id;
	send.gp_info_key_.dw_mall_id		= pGPInfo->dwMallID;

	g_dbSession.Send(&send, send.dw_size);
}

VOID GuildPurchase::RemoveGuildGPInfo2DB()
{
	ASSERT(VALID_POINT(m_pGuild));
	if (!VALID_POINT(m_pGuild))
	{
		return;
	}

	tagNDBC_RemoveGuildGPInfo send;

	send.dw_guild_id = m_pGuild->get_guild_att().dwID;

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// ���ɼ�ֵ
//-------------------------------------------------------------------------------------------------------
INT64 GuildPurchase::GetKey( DWORD dw_role_id, DWORD dwID )
{
	ASSERT(VALID_VALUE(dw_role_id) && VALID_VALUE(dwID));
	if (!VALID_VALUE(dw_role_id) || !VALID_VALUE(dwID))
		return INVALID_VALUE;

	INT64 n64Key = dw_role_id;

	n64Key = (n64Key << 32) | dwID;

	return n64Key;
}

//-------------------------------------------------------------------------------------------------------
// �Ź��ɹ�����
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::DoSuccessStuff( tagGroupPurchase* pGPInfo )
{
	// ��һ�¼���֤
	if (m_pGuild->get_guild_att().dwID != pGPInfo->dwGuildID)
	{
		return INVALID_VALUE;
	}

	DWORD dwItemTypeID		= INVALID_VALUE;
	DWORD dwLaunchRoleID	= pGPInfo->dw_role_id;

	// �������б��е���ҷ�����Ʒ
	DWORD tmpRoleID = INVALID_VALUE;
	package_list<DWORD>::list_iter iter = pGPInfo->listParticipators->begin();	// �ϲ��Ѿ���֤���б�ĺϷ���
	
	while(pGPInfo->listParticipators->find_next(iter, tmpRoleID))
	{
		// ������Ʒ
		tagMallItemSell	sItemSell;
		DWORD dw_error_code = CreateGPItems(pGPInfo->dwMallID, sItemSell);
		if (dw_error_code != E_Success)
		{
			return dw_error_code;
		}
		else if (VALID_POINT(sItemSell.pItem))
		{
			dwItemTypeID = sItemSell.pItem->dw_data_id;
		}
		else
		{
			return INVALID_VALUE;
		}

		// ������
		if(VALID_POINT(sItemSell.pItem))
		{		
			INT64 n64_serial = sItemSell.pItem->n64_serial;
			DWORD dwFstGainTime = g_world.GetWorldTime();
			INT16 n16BuyNum = sItemSell.pItem->n16Num;

			// �ٱ�����ʷ��¼
			ItemMgr::ProcBaiBaoRecord(sItemSell.pItem->dw_data_id, tmpRoleID, INVALID_VALUE, EBBRT_GroupPurchase, dwFstGainTime);

			// log
			g_mall.LogMallSell(tmpRoleID, INVALID_VALUE, *sItemSell.pItem, n64_serial, n16BuyNum, 
				dwFstGainTime, sItemSell.nYuanBaoNeed, 0, elcid_group_purchase_buy_item);

			// ����Ʒ�ŵ��ٱ�����
			Role *pTmpRole = g_roleMgr.GetRolePtrByID(tmpRoleID);
			if(VALID_POINT(pTmpRole))
			{
				pTmpRole->GetItemMgr().Add2BaiBao(sItemSell.pItem, elcid_group_purchase_buy_item);
			}
			else
			{
				// �洢��item_baibao����
				ItemMgr::InsertBaiBao2DB(sItemSell.pItem, tmpRoleID, elcid_group_purchase_buy_item);

				// ɾ����Ʒ
				::Destroy(sItemSell.pItem);
			}

			// �������Ʒ����ŵ��ٱ�����
			if(VALID_POINT(sItemSell.pPresent))
			{
				// �ٱ�����ʷ��¼
				ItemMgr::ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, tmpRoleID, INVALID_VALUE, EBBRT_Mall, dwFstGainTime);

				if(VALID_POINT(pTmpRole))
				{
					pTmpRole->GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_group_purchase_buy_item_add);
				}
				else
				{
					// �洢��item_baibao����
					ItemMgr::InsertBaiBao2DB(sItemSell.pPresent, tmpRoleID, elcid_group_purchase_buy_item_add);

					// ɾ����Ʒ
					::Destroy(sItemSell.pItem);
				}
			}

			// ��������
			if (VALID_POINT(pTmpRole) && sItemSell.nExVolumeAssign > 0)
			{
				pTmpRole->GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_group_purchase_buy_item);
			}
		}
	}

	// �����Ź�ָ��+1
	m_pGuild->modify_group_exponent(TRUE);

	// ɾ�������Ź���Ϣ��¼
	RemoveGroupPurchaseInfo(pGPInfo);

	// �����ڹ㲥�ɹ���Ϣ
	tagNS_RespondBroadCast send;
	send.eType = ERespondBroadCast_Success;
	send.dw_role_id = dwLaunchRoleID;
	send.dw_data_id = dwItemTypeID;
	m_pGuild->send_guild_message(&send, send.dw_size);

	return E_Success;
}