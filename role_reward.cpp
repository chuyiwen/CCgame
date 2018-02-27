#include "StdAfx.h"

#include "player_session.h"

#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/reward_define.h"

#include "../common/ServerDefine/log_server_define.h"
#include "map.h"
#include "att_res.h"
#include "role.h"
#include "role_mgr.h"
#include "hearSay_helper.h"
#include "item_mgr.h"
#include "item_creator.h"



DWORD Role::send_reward_data()
{
	NET_SIS_get_reward_data send;
	memcpy(&send.st_data, &st_role_reward_data, sizeof(st_role_reward_data));
	SendMessage(&send, send.dw_size);

	return E_Success;
}


BOOL Role::addRewardItem(DWORD dwDataID, DWORD dwNubmer, E_REWARDFROM nType)
{
	if (nType < RF_CHONGZHI || nType >=  RF_NUM)
		return false;

	
	for (int i = 0; i < MAX_REWARD_NUMBER; i++)
	{
		if (st_role_reward_data[nType][i].dwItemDataID == 0)
		{
			st_role_reward_data[nType][i].dwItemDataID = dwDataID;
			st_role_reward_data[nType][i].dwNumber = dwNubmer;
			st_role_reward_data[nType][i].nType = nType;

			// 发消息
			NET_SIS_update_reward_data send;
			send.byIndex = i;
			send.byType = nType;
			memcpy(&send.st_data, &st_role_reward_data[nType][i], sizeof(tagRewardData));
			SendMessage(&send, send.dw_size);

			//gx add 2013.9.17 也向数据库发消息
			UpdateRoleRewardData();
			return true;
		}
	} 

	return false;
}

BOOL Role::receiveRewardItem(E_REWARDFROM nType)
{
	if (nType < 0 || nType >= RF_NUM)
		return false;

	//if (byIndex < 0 || byIndex > MAX_REWARD_NUMBER )
	//	return false;


	for (int i = 0; i < MAX_REWARD_NUMBER; i++)
	{
		const tagItemProto* pProto = AttRes::GetInstance()->GetItemProto(st_role_reward_data[nType][i].dwItemDataID);
		if (!VALID_POINT(pProto))
			continue;

		// 预检查背包中是否有空位
		if(GetItemMgr().GetBagFreeSize() < 1)
		{
			return false;
		}

		// 生成物品
		tagItem* pItem = ItemCreator::CreateEx(EICM_Activity, INVALID_VALUE, pProto->dw_data_id, st_role_reward_data[nType][i].dwNumber, EIQ_Quality0);
		if( !VALID_POINT(pItem) ) return false;

		DWORD dwRtv = GetItemMgr().Add2Bag(pItem, elcid_wu_se_shi, TRUE);

		if(E_Success != dwRtv)
		{
			SAFE_DELETE(pItem);
			return false;
		}

		memset(&st_role_reward_data[nType][i], 0, sizeof(tagRewardData));

		// 发消息
		NET_SIS_update_reward_data send;
		send.byType = nType;
		send.byIndex = i;
		memcpy(&send.st_data, &st_role_reward_data[nType][i], sizeof(tagRewardData));
		SendMessage(&send, send.dw_size);
	}


	return true;
}
