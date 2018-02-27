
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端和服务器端间消息处理 -- 开启宝箱相关

#include "StdAfx.h"

#include "role.h"
#include "player_session.h"
#include "db_session.h"
#include "world_session.h"
#include "item_creator.h"
#include "../../common/WorldDefine/grubbao_protocol.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "TreasureChest_mgr.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/TreasureChest_define.h"
#include "script_mgr.h"


//--------------------------------------------------------------------------
// 开启宝箱
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleOpenTreasureChest(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_treasure_chest);
	NET_SIC_treasure_chest * p_receive = ( NET_SIC_treasure_chest * ) pCmd ;  
	//	获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获取角色背包中的宝箱id和钥匙id
	tagItem* pItemChest = pRole->GetItemMgr().GetBagItem(p_receive->n64ChestID);
	tagItem* pItemKey	= pRole->GetItemMgr().GetBagItem(p_receive->n64KeyID);
	if ( !VALID_POINT(pItemChest) || !VALID_POINT(pItemKey)) 
	{
		NET_SIS_treasure_chest send;
		send.dw_error_code = INVALID_VALUE;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}

	DWORD dwChestID =  pItemChest->dw_data_id;
	DWORD dwKeyID = pItemKey->dw_data_id;

	// 记录玩家的宝箱和钥匙id
	pRole->SetChestSerial(p_receive->n64ChestID);
	pRole->SetKeySerial(p_receive->n64KeyID);
	pRole->SetChestTypeID(dwChestID);
	pRole->SetKeyTypeID(dwKeyID);

	// 调用脚本函数
	g_ScriptMgr.GetRoleScript()->OnOpenChest(pRole, dwChestID, dwKeyID);
	return E_Success;
}

//--------------------------------------------------------------------------
// 停止随机选中物品，并且确定最后选中的物品
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleStopTreasureChest(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stop_treasure_chest);
	NET_SIC_stop_treasure_chest * p_receive = ( NET_SIC_stop_treasure_chest * ) pCmd ;  
	//	获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dwChestID =  pRole->GetChestTypeID();
	DWORD dwKeyID = pRole->GetKeyTypeID();

	// 调用脚本函数
	g_ScriptMgr.GetRoleScript()->OnStopChest(pRole, dwChestID, dwKeyID);
	return E_Success;
}

//--------------------------------------------------------------------------
// 再试一次
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleAgainTreasureChest(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_repeat_treasure_chest);
	NET_SIC_repeat_treasure_chest * p_receive = ( NET_SIC_repeat_treasure_chest * ) pCmd ;  
	//	获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 调用脚本函数
	g_ScriptMgr.GetRoleScript()->OnAgainChest(pRole);
	return E_Success;
}

//--------------------------------------------------------------------------
// 获得道具
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleChestGetItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_treasure_item);
	NET_SIC_get_treasure_item * p_receive = ( NET_SIC_get_treasure_item * ) pCmd ;  
	//	获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dwItemID = pRole->GetChestItem().dw_data_id;
	INT n_num = pRole->GetChestItem().n_num;

	// 开宝箱的物品不对
	if (dwItemID != p_receive->dw_data_id)
	{
		NET_SIS_get_treasure_item send;
		send.dw_error_code = INVALID_VALUE;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}

	// 调用脚本函数
	g_ScriptMgr.GetRoleScript()->OnGetItem(pRole, dwItemID, n_num);
	return E_Success;
}
	