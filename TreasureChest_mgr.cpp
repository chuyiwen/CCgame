/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宝箱管理

#include <time.h>
#include "StdAfx.h"
#include "att_res.h"
#include "TreasureChest_mgr.h"
//#include "../serverbase/debug/debug.h"
#include "../../common/WorldDefine/grubbao_protocol.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "world_session.h"

//-------------------------------------------------------------------------------
// 初始化  载入xml文件
//-------------------------------------------------------------------------------
BOOL TreasureChestMgr::Init()
{
	// 读取TreasureItem.xml
	list<tstring> listProto;
	tstring szTreasureItemProto = _T("server_data//TreasureItem.xml");
	file_container* varTreasureItemProto = new file_container;
	if (!varTreasureItemProto->load(g_world.get_virtual_filesys(), szTreasureItemProto.c_str(), "TableID", &listProto))
	{
		print_message(_T("Load TreasureItem proto file failed"));
		return FALSE;
	}

	INT nChestNum = varTreasureItemProto->get_int(_T("TableID"), listProto.back().c_str()) / 100;
	map< int, vector<tagChestItem> >	mapChest;

	list<tstring>::iterator end = listProto.end();
	for (list<tstring>::iterator begin = listProto.begin(); begin != end; )
	{
		
		for (INT n = 0; begin != end && n != MAX_CHEST_NUM; ++begin, ++n)
		{
			tagChestItem item;
			item.dw_data_id = varTreasureItemProto->get_dword(_T("TypeID"), begin->c_str());
			item.nTableID = varTreasureItemProto->get_int(_T("TableID"), begin->c_str());
			item.n_num = varTreasureItemProto->get_int(_T("Num"), begin->c_str());
			item.fBeginRate = varTreasureItemProto->get_float(_T("BeginRate"), begin->c_str());
			item.fRoleRate = varTreasureItemProto->get_float(_T("RoleRate"), begin->c_str());
			item.fServerRate = varTreasureItemProto->get_float(_T("ServerRate"), begin->c_str());
			item.fNormalRate = varTreasureItemProto->get_float(_T("NormalRate"), begin->c_str());

			map< int, vector<tagChestItem> >::iterator it = mapChest.find(item.nTableID/100 - 1);
			if (it == mapChest.end())
			{
				vector<tagChestItem> vectmp;
				vectmp.push_back(item);
				mapChest.insert(make_pair(item.nTableID/100 - 1, vectmp));
			}
			else
			{
				it->second.push_back(item);
			}
		}
	}

	for (INT i = 0; i != nChestNum; ++i)
	{
		m_vecItems.push_back(mapChest[i]);
	}

	SAFE_DELETE(varTreasureItemProto);
	return TRUE;
}

//-------------------------------------------------------------------------------
// 返回宝箱内物品
//-------------------------------------------------------------------------------
vector<tagChestItem>& TreasureChestMgr::GetChest(INT nIndex)
{
	--nIndex;								//索引需减1
	ASSERT(nIndex < m_vecItems.size());
	return m_vecItems[nIndex];
}

//-------------------------------------------------------------------------------
// 开出宝箱中物品
//-------------------------------------------------------------------------------
tagChestItem* TreasureChestMgr::GetRandomItem(DWORD dwChestID, ERateType eRate, FLOAT fRand)
{
	tagItemProto* pProto = AttRes::GetInstance()->GetItemProto(dwChestID);
	ASSERT(VALID_POINT(pProto));
	INT32 nIndex = pProto->nSpecFuncVal1;
	INT n = 0;

	switch (eRate)
	{
	case ERTT_BeginRate:
		for (; n != MAX_CHEST_NUM && fRand >= 0.0; ++n)
		{
			fRand -= GetChest(nIndex)[n].fBeginRate;
		}
		break;

	case ERTT_RoleRate:
		for (; n != MAX_CHEST_NUM && fRand >= 0.0; ++n)
		{
			fRand -= GetChest(nIndex)[n].fRoleRate;
		}
		break;

	case ERTT_ServerRate:
		for (; n != MAX_CHEST_NUM && fRand >= 0.0; ++n)
		{
			fRand -= GetChest(nIndex)[n].fServerRate;
		}
		break;

	case ERTT_NormalRate:
		for (; n != MAX_CHEST_NUM && fRand >= 0.0; ++n)
		{
			fRand -= GetChest(nIndex)[n].fNormalRate;
		}
		break;

	default:
		print_message(_T("Should not get here"));
		break;
	}

	return &m_vecItems[nIndex-1][n-1];		// 返回索引均需减一
}

//-------------------------------------------------------------------------------
// 向客户端发送消息
//-------------------------------------------------------------------------------
BOOL TreasureChestMgr::SendMsg2Client(Role *pRole, DWORD dwChestID, const std::string strMsgName, 
									  BOOL bOpened, BOOL bDestroy, DWORD dwItemID, INT nItemNum, DWORD dw_error_code)
{
	if (strMsgName == "OpenChest")
	{
		NET_SIS_treasure_chest send;
		send.dwChestTypeID = dwChestID;
		if (bOpened)
		{
			tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dwChestID);
			if (!VALID_POINT(pItemProto))
			{
				return 0;
			}

			//得到宝箱物品属性id
			for (INT i = 0; i != MAX_CHEST_NUM; ++i)
			{
				send.dw_data_id[i] = g_TreasureChestMgr.GetChest(pItemProto->nSpecFuncVal1)[i].dw_data_id;
				send.n_num[i] = g_TreasureChestMgr.GetChest(pItemProto->nSpecFuncVal1)[i].n_num;
			}
		}

		send.dw_error_code = dw_error_code;
		// 发送消息给客户端
		pRole->SendMessage(&send, send.dw_size);
		return TRUE;
	}
	else if (strMsgName == "StopChest")
	{
		NET_SIS_stop_treasure_chest send;
		send.dw_data_id = dwItemID;
		send.n_num = nItemNum;
		send.dw_error_code = dw_error_code;

		// 记录开出的物品
		tagChestItem item;
		item.dw_data_id = dwItemID;
		item.n_num = nItemNum;
		item.nTableID = 0;
		item.fBeginRate = 0;
		item.fNormalRate = 0;
		item.fRoleRate = 0;
		item.fServerRate = 0;

		pRole->SetChestItem(item);

		// 发送消息给客户端
		pRole->SendMessage(&send, send.dw_size);
		return TRUE;
	}
	else if (strMsgName == "AgainChest")
	{
		NET_SIS_repeat_treasure_chest send;
		send.byDestroy = bDestroy ? 1 : 0;
		send.dw_error_code = dw_error_code;
		// 清空角色最后开出物品的记录
		tagChestItem item;
		item.dw_data_id = 0;
		item.n_num = 0;
		item.nTableID = 0;
		item.fBeginRate = 0;
		item.fNormalRate = 0;
		item.fRoleRate = 0;
		item.fServerRate = 0;
		pRole->SetChestItem(item);
		// 发送消息给客户端
		pRole->SendMessage(&send, send.dw_size);
		return TRUE;
	}

	else if (strMsgName == "GetItem")					// "GetItem"
	{
		NET_SIS_get_treasure_item send;
		send.dw_error_code = dw_error_code;
		// 清空角色最后开出物品的记录
		tagChestItem item;
		item.dw_data_id = 0;
		item.n_num = 0;
		item.nTableID = 0;
		item.fBeginRate = 0;
		item.fNormalRate = 0;
		item.fRoleRate = 0;
		item.fServerRate = 0;
		pRole->SetChestItem(item);

		// 清空角色宝箱和钥匙ID记录
		pRole->SetChestSerial(0);
		pRole->SetKeySerial(0);
		// 发送消息给客户端
		pRole->SendMessage(&send, send.dw_size);
		return TRUE;
	}

	else
	{
		return FALSE;
	}
	
}

//-------------------------------------------------------------------------------
// 销毁
//-------------------------------------------------------------------------------
VOID TreasureChestMgr::Destroy()
{
	m_vecItems.clear();
}

TreasureChestMgr g_TreasureChestMgr;