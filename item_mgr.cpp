/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//物品、装备管理器

#pragma once

#include "StdAfx.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../../common/WorldDefine/function_npc_protocol.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/common_server_define.h"
#include "container_restrict.h"
#include "item_creator.h"
#include "role.h"
#include "role_mgr.h"
#include "db_session.h"
#include "att_res.h"
#include "map.h"
#include "item_mgr.h"
#include "pet_soul.h"
#include "pet_pocket.h"

BOOL tagEquip::ExpChange(INT nValue)
{
	//if (nValue == 0) return FALSE;

	//const tagEquipLevelUpEffect* pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(equipSpec.nLevel);
	//ASSERT(VALID_POINT(pEffect));
	//INT nExpLevelUp = pEffect->nExpLevelUp;
	//if (pEquipProto->eEquipPos == EEP_Shipin1 )
	//{
	//	nExpLevelUp = pEffect->nExpLevelUpShipin;
	//}

	//INT nLevelUpExpRemain = nExpLevelUp - equipSpec.nCurLevelExp;
	//ASSERT( nLevelUpExpRemain >= 0 );

	//// 不会升级
	//if( nValue < nLevelUpExpRemain )
	//{
	//	// 累加上新的经验
	//	equipSpec.nCurLevelExp += nValue;
	//	return FALSE;
	//}
	//// 需要升级
	//else
	//{
	//	// 根据是否强制升级来决定技能的等级上限
	//	equipSpec.nCurLevelExp += nLevelUpExpRemain;

	//	// 可以升级
	//	if (equipSpec.nLevel != pEquipProto->byMaxLevel)
	//	{
	//		INT nExpRealAdd = nLevelUpExpRemain;
	//		nValue -= nLevelUpExpRemain;
	//		INT nNextLevel = equipSpec.nLevel + 1;
	//		INT nTalentPointAdd = 0;

	//		// 看能升几级
	//		for(; nNextLevel <= pEquipProto->byMaxLevel; nNextLevel++)
	//		{
	//			pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(nNextLevel);

	//			// 还可以再继续升级
	//			if( nExpLevelUp <= nValue )
	//			{
	//				nValue -= nExpLevelUp;
	//				nExpRealAdd = nExpLevelUp;
	//				nTalentPointAdd += pEffect->n16TalentAvail;
	//			}
	//			// 不可以再升级了
	//			else
	//			{
	//				nTalentPointAdd += pEffect->n16TalentAvail;
	//				break;
	//			}
	//		}

	//		// 改变等级
	//		if( nNextLevel > pEquipProto->byMaxLevel )
	//		{
	//			nNextLevel = pEquipProto->byMaxLevel;
	//			// 经验修正一下
	//			if( nValue > nExpLevelUp )
	//			{
	//				nValue = nExpLevelUp;
	//			}
	//		}
	//		LevelChange(nNextLevel);
	//		equipSpec.byMaxTalentPoint += nTalentPointAdd;
	//	}	
	//	return TRUE;
	//}
	return FALSE;
}

INT16 tagEquip::GetAddTalent(INT32 beginLevel, INT32 endLevel)
{
	if (beginLevel >= endLevel)
		return 0;
	
	INT16 n16AddTalent = 0;
	for (INT32 i = beginLevel+1;i <= endLevel; i++)
	{
		const tagEquipLevelUpEffect* pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(i);
		if (!VALID_POINT(pEffect)) return 0;
		
		n16AddTalent += pEffect->n16TalentAvail;
	}
	
	return n16AddTalent;
}

INT tagEquip::GetRating() const
{
	int classParam = pEquipProto->dw_data_id/100000%10 - 1;
	classParam = max(0, classParam);
	classParam = min(2, classParam);
	ASSERT(classParam>=0 && classParam<=2);
	if (classParam > 2)
	{
		classParam = 0;
	}
	double fRating = 0.0f;
	//fRating += (GetMinDmg() + GetMaxDmg() )/2 * 20.0 ;

	//fRating += GetArmor() * 3.0;
	// 固定属性
	//for (int i = 0; i < MAX_ROLEATT_BASE_EFFECT; i++)
	//{
	//	EquipAddAtt eaa = ItemCreator::ERA2EAA(pEquipProto->BaseEffect[i].eRoleAtt);
	//	if (eaa == EAA_NULL) continue;
	//	fRating += nRatingParam[classParam][eaa]*pEquipProto->BaseEffect[i].nValue * 1.0f;
	//}
	// 附加属性
	for (int i = 0; i < MAX_ADDITIONAL_EFFECT; i++)
	{
		EquipAddAtt eaa = ItemCreator::ERA2EAA(equipSpec.EquipAttitionalAtt[i].eRoleAtt);
		if (eaa == EAA_NULL) continue;

		int nAttValue = equipSpec.EquipAttitionalAtt[i].nValue;
		if (i < MAX_BASE_ATT)
		{
			nAttValue += ItemHelper::getConsolidateAtt(nAttValue, equipSpec.byConsolidateLevel);
		}
		fRating += G_RATING_ATT[classParam][eaa]*nAttValue * 1.0;
	
	}
	// 宝石属性
	for (int i = 0; i < MAX_EQUIPHOLE_NUM; i++)
	{
		const tagConsolidateItem *pGemProto = AttRes::GetInstance()->GetConsolidateProto(equipSpec.dwHoleGemID[i]);
		if(VALID_POINT(pGemProto))
		{
			for (int j = 0; j < MAX_CONSOLIDATE_ROLEATT; j++)
			{
				EquipAddAtt eaa = ItemCreator::ERA2EAA(pGemProto->tagRoleAtt[j].eRoleAtt);
				if (eaa == EAA_NULL) continue;
				fRating += G_RATING_ATT[classParam][eaa]*pGemProto->tagRoleAtt[j].nAttVal * 1.0;
			}
		}
	}
	//FLOAT fQualityParam[] = { 1, 1.25, 1.5, 2, 3};
	//fRating = fRating * fQualityParam[equipSpec.byQuality] * (10+equipSpec.nLevel)/11.0f;
	fRating = fRating / 1000.0;
	return (INT)fRating;
}
//-------------------------------------------------------------------------------------------------------
// 构造/析构函数
//-------------------------------------------------------------------------------------------------------
ItemMgr::ItemMgr(Role* p_role, DWORD dwAcctID, DWORD dw_role_id, INT16 n16BagSize, INT16 n16WareSize)
			: m_Bag(EICT_Bag, n16BagSize, SPACE_ALL_BAG, dw_role_id, dwAcctID), 
			  m_QuestItemBag(EICT_Quest, SPACE_QUEST_BAG, SPACE_QUEST_BAG, dw_role_id, dwAcctID, new QusetItemBarRestrict),
			  m_BaiBaoBag(EICT_Baibao, SPACE_BAIBAO_BAG, SPACE_BAIBAO_BAG, dw_role_id, dwAcctID, new BaiBaoBagRestrict),
			  m_RoleWare(EICT_RoleWare, n16WareSize, SPACE_ALL_WARE, dw_role_id, dwAcctID, new WareRestrict),
			  m_EquipBar(EICT_Equip, X_EQUIP_BAR_SIZE, X_EQUIP_BAR_SIZE) 
{
	m_pRole = p_role;
	m_mapCDTime.clear();
	m_mapMaxHold.clear();
}

ItemMgr::~ItemMgr() {}



//-------------------------------------------------------------------------------------------------------
// 获得容器指针。注只能得到ItemContainer对象。
//-------------------------------------------------------------------------------------------------------
ItemContainer* ItemMgr::GetContainer(EItemConType eConType)
{
	switch(eConType)
	{
		case EICT_Bag:
			return &m_Bag;
			break;
		case EICT_Quest:
			return &m_QuestItemBag;
			break;
		case EICT_Baibao:
			return &m_BaiBaoBag;
			break;
		case EICT_RoleWare:
			return &m_RoleWare;
			break;
		case EICT_Equip:
		case EICT_Shop:
		case EICT_Ground:
			return NULL;
			break;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------------
// 检查物品是否需要记录log。若需要，则向DBServer发送消息
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::log_item(const tagItem &item1, const tagItem *pItem2, 
					  INT16 n16OptNum, DWORD dw_cmd_id, DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{
	if(!(is_item_need_log(item1) || (VALID_POINT(pItem2) && is_item_need_log(*pItem2))))
	{
		return;
	}
	
	NET_DB2C_log_item send;
	send.s_log_item_.dw_role_id		= m_pRole->GetID();
	send.s_log_item_.dw_data_id		= item1.pProtoType->dw_data_id;

	send.s_log_item_.n64_serial1	= item1.n64_serial;
	send.s_log_item_.n8_con_type1	= item1.eConType;
	send.s_log_item_.n16_res_num1	= item1.n16Num;

	send.s_log_item_.n16_opt_num		= n16OptNum;
	send.s_log_item_.dw_cmd_id		= dw_cmd_id;

	send.s_log_item_.dw_role_id_rel	= dwRoleIDRel;

	if(VALID_POINT(pItem2))
	{
		send.s_log_item_.n64_serial2	= pItem2->n64_serial;
		send.s_log_item_.n8_con_type2	= pItem2->eConType;
		send.s_log_item_.n16_res_num2	= pItem2->n16Num;
	}
	else
	{
		send.s_log_item_.n64_serial2	= 0;
		send.s_log_item_.n8_con_type2	= EICT_Null;
		send.s_log_item_.n16_res_num2	= 0;
	}

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 检查物品是否需要记录log。若需要，则向DBServer发送消息
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::LogItemTimes(const tagItem &item, DWORD dw_cmd_id)
{
	if(!is_item_need_log(item))
	{
		return;
	}
	
	NET_DB2C_log_item_times send;
	send.s_log_item_times_.dw_role_id		= m_pRole->GetID();
	send.s_log_item_times_.dw_cmd_id		= dw_cmd_id;
	send.s_log_item_times_.n64_serial	= item.n64_serial;
	send.s_log_item_times_.dw_data_id		= item.dw_data_id;
	send.s_log_item_times_.n_used_times	= item.nUseTimes;
	send.s_log_item_times_.n_max_use_times	= item.pProtoType->nMaxUseTimes;
	send.s_log_item_times_.n8_con_type	= item.eConType;

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 装备鉴定
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::IdentifyEquip(INT64 n64SerialReel, INT64 n64SerialEquip, DWORD dw_cmd_id)
{
	ItemContainer &Bag = GetBag();

	// 检查鉴定卷
	tagItem *pReel = Bag.GetItem(n64SerialReel);
	if(!VALID_POINT(pReel))
	{
		return E_Item_NotFound;
	}

	if(pReel->pProtoType->eSpecFunc != EISF_IdetifyEquip)
	{
		return INVALID_VALUE;
	}

	// 检查装备
	tagEquip *pEquip = (tagEquip*)Bag.GetItem(n64SerialEquip);
	if(!VALID_POINT(pEquip))
	{
		return E_Item_NotFound;
	}

	if(!MIsEquipment(pEquip->dw_data_id))
	{
		return E_Item_NotEquipment;
	}

	if(M_is_identified(pEquip))
	{
		return E_Equip_Idendtifed;
	}

	// 检查鉴定卷等级是否满足要求(鉴定卷等级≥装备等级/10)
	if(pReel->pProtoType->byLevel < (pEquip->pProtoType->byLevel / 10))
	{
		return E_Item_LevelLimit;
	}
	
	// 生成鉴定后属性
	ItemCreator::IdentifyEquip(pEquip);

	// 向DBServer和客户端发消息
	UpdateEquipSpec(*pEquip);

	// 使用一次
	ItemUsedFromBag(n64SerialReel, 1, dw_cmd_id);

	return E_Success;
}

//----------------------------------------------------------------------------------------
// 初始化物品装备 -- 发给客户端
//----------------------------------------------------------------------------------------
VOID ItemMgr::SendInitStateItem()
{
	ItemContainer	&Bag		= GetBag();
	ItemContainer	&QuestBag	= GetQuestItemBag();
	ItemContainer	&BaiBao		= GetBaiBaoBag();
	ItemContainer	&RoleWare	= GetRoleWare();
	EquipContainer	&EquipBar	= GetEquipBar();

	INT16 n16NumBag			= Bag.GetCurSpaceSize() - Bag.GetFreeSpaceSize();
	INT16 n16NumQuestBag	= QuestBag.GetCurSpaceSize() - QuestBag.GetFreeSpaceSize();
	INT16 n16NumBaiBao		= BaiBao.GetCurSpaceSize() - BaiBao.GetFreeSpaceSize();
	INT16 n16NumRoleWare	= RoleWare.GetCurSpaceSize() - RoleWare.GetFreeSpaceSize();
	INT16 n16NumEquipBar	= EquipBar.GetCurSpaceSize() - EquipBar.GetFreeSpaceSize();

	INT32 nItemNum = n16NumBag + n16NumQuestBag + n16NumBaiBao + n16NumRoleWare + n16NumEquipBar;

	// 计算可能的最大空间
	INT32 nMaxMsgSize = SIZE_EQUIP * nItemNum + sizeof(NET_SIS_get_role_init_state_item);

	// 申请消息
	CREATE_MSG(pSend, nMaxMsgSize, NET_SIS_get_role_init_state_item);

	pSend->n16SzBag			= Bag.GetCurSpaceSize();
	pSend->n16SzRoleWare	= RoleWare.GetCurSpaceSize();
	pSend->n_num				= nItemNum;

	INT32 nSizeTotal	= 0;
	INT32 nSizeOne		= 0;

	GetAllItem(Bag, n16NumBag, pSend->byData + nSizeTotal, nSizeOne);
	nSizeTotal += nSizeOne;

	GetAllItem(QuestBag, n16NumQuestBag, pSend->byData + nSizeTotal, nSizeOne);
	nSizeTotal += nSizeOne;

	GetAllItem(BaiBao, n16NumBaiBao, pSend->byData + nSizeTotal, nSizeOne);
	nSizeTotal += nSizeOne;

	GetAllItem(RoleWare, n16NumRoleWare, pSend->byData + nSizeTotal, nSizeOne);
	nSizeTotal += nSizeOne;

	GetAllItem(EquipBar, n16NumEquipBar, pSend->byData + nSizeTotal, nSizeOne);
	nSizeTotal += nSizeOne;

	pSend->dw_size = sizeof(NET_SIS_get_role_init_state_item) - 1 + nSizeTotal;

	SendMessage(pSend, pSend->dw_size);

	// 释放消息内存
	MDEL_MSG(pSend);
}

//-------------------------------------------------------------------------------------------------------
// 角色初始化时，将物品&装备放入指定容器位置
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::Put2Container(tagItem *pItem)
{	
	if(!VALID_POINT(m_pRole))
	{
		ASSERT(VALID_POINT(m_pRole));
		return INVALID_VALUE;
	}

	DWORD dw_error_code = INVALID_VALUE;

	//加入物品上限管理
	if(!CanAddMaxHoldItem(*pItem)) return E_Item_MaxHold;

	switch(pItem->eConType)
	{
	case EICT_Bag:
		dw_error_code = GetBag().Add(pItem, pItem->n16Index, FALSE, FALSE);
		break;
	case EICT_Quest:
		dw_error_code = GetQuestItemBag().Add(pItem, pItem->n16Index, FALSE, FALSE);
		break;
	case EICT_Baibao:
		dw_error_code = GetBaiBaoBag().Add(pItem, pItem->n16Index, FALSE, FALSE);
		break;
	case EICT_RoleWare:
		dw_error_code = GetRoleWare().Add(pItem, pItem->n16Index, FALSE, FALSE);
		break;
	case EICT_Equip:
		dw_error_code = GetEquipBar().Add((tagEquip*)pItem, (EEquipPos)pItem->n16Index);
		break;
	default:
		//ASSERT(0);
		break;
	}

	// 物品上限管理
	if(dw_error_code == E_Success)
		AddMaxHoldItem(*pItem);

	// 修改保存标志位
	pItem->SetUpdate(EUDBS_Null);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 是否可以交易
//-----------------------------------------------------------------------------
BOOL ItemMgr::CanExchange(const tagItem& item) const
{
	//gx modify 2013.8.2 仅是绑定的物品不能交易与摆摊
	return !(/*item.IsLock() || */item.IsBind()) /*&& m_pRole->GetRoleStateEx().IsInState(ERSE_BagPsdPass)*/;
}

//-----------------------------------------------------------------------------
// 玩家获得物品&装备 -- 普通物品放入背包，任务物品放入任务栏
//-----------------------------------------------------------------------------
BOOL ItemMgr::Add2Role(EItemCreateMode eCreateMode, DWORD dwCreateID, 
					   DWORD dw_data_id, INT32 n_num, EItemQuality eQlty, DWORD dw_cmd_id, BOOL bBind)
{
	INT16 n16BagSpace = 0;
	INT16 n16QuestSpace = 0;
	INT16 n16MaxLap;

	if(!CalSpaceUsed(dw_data_id, n_num, n16BagSpace, n16QuestSpace, n16MaxLap))
	{
		return FALSE;
	}

	if(n16BagSpace > 0 && GetBagFreeSize() < n16BagSpace
		|| n16QuestSpace > 0 && GetQuestItemBagFreeSize() < n16QuestSpace)
	{
		return FALSE;
	}
	
	tagItem *pNew;
	ItemContainer &con = n16BagSpace > 0 ? GetBag() : GetQuestItemBag();
	INT16 n16Space = max(n16BagSpace, n16QuestSpace);

	INT16 n16NewLeft = n_num;
	for(INT16 i=0; i<n16Space-1; ++i)
	{
		pNew = ItemCreator::CreateEx(eCreateMode, dwCreateID, dw_data_id, n16MaxLap, eQlty, bBind);
		if(!VALID_POINT(pNew))
		{
			return FALSE;
		}

		if(add_item(con, pNew, dw_cmd_id, TRUE, TRUE) != E_Success)
		{
			Destroy(pNew);
			return FALSE;
		}
		
		n16NewLeft -= n16MaxLap;
	}

	pNew = ItemCreator::CreateEx(eCreateMode, dwCreateID, dw_data_id, n16NewLeft, eQlty, bBind);
	if(!VALID_POINT(pNew))
	{
		return FALSE;
	}
	//if (bBind)
	//{
	//	pNew->byBind = EBS_SYSTEM_Bind;
	//}

	if(add_item(con, pNew, dw_cmd_id, TRUE, TRUE) != E_Success)
	{
		Destroy(pNew);
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// 从背包或任务栏中清除物品
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::RemoveFromRole(DWORD dw_data_id, INT32 n_num, DWORD dw_cmd_id, INT nBind)
{
	if(IsQuestItem(dw_data_id))
	{
		if(INVALID_VALUE == n_num)
		{
			return RemoveItems(GetQuestItemBag(), dw_data_id, dw_cmd_id, nBind);
		}
		else
		{
			return RemoveItems(GetQuestItemBag(), dw_data_id, n_num, dw_cmd_id, nBind);
		}
	}
	else
	{
		if(INVALID_VALUE == n_num)
		{
			return RemoveItems(GetBag(), dw_data_id, dw_cmd_id, nBind);
		}
		else
		{
			return RemoveItems(GetBag(), dw_data_id, n_num, dw_cmd_id, nBind);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// 向容器中添加物品
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::add_item(ItemContainer& container, tagItem *&pItem, DWORD dw_cmd_id, 
					   BOOL bInsert2DB/* = FALSE*/, BOOL bCheckAdd/* = TRUE*/, DWORD dwRoleIDRel/* = INVALID_VALUE*/, BOOL bChangeOwner/* = TRUE*/)
{
	ASSERT(VALID_POINT(pItem));
	ASSERT(VALID_POINT(pItem->n64_serial));

	INT16		n16Index;
	DWORD		dw_error_code;
	s_item_move	itemMove;
	INT16		n16AddNum = pItem->n16Num;

	// 检查是否有获取时间
	if(0 == pItem->dw1stGainTime)
	{
		pItem->dw1stGainTime = g_world.GetWorldTime();
	}
	
	// 物品上限
	if ( CanAddMaxHoldItem(*pItem) )
	{
		// 加入容器
		dw_error_code = container.Add(pItem, n16Index, itemMove, bCheckAdd, bChangeOwner);
	}
	else
	{
		dw_error_code = E_Item_MaxHold;
	}

	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
		print_message(_T("\nAdd item to container failed! \nroleid: %u, item serial: %lld, item typeid: %d, container type: %d!!!!\n\n"),
			container.GetOwnerID(), pItem->n64_serial, pItem->pEquipProto->dw_data_id, container.GetConType());

		//?? 考虑将该物品保存到log中

		return dw_error_code;
	}

	// 管理物品上限列表
	AddMaxHoldItem(*pItem);

	INT16 n16Num = itemMove.n_num_res1 != 0 ? itemMove.n_num_res1: itemMove.n_num_res2;
	
	// 如果是新物品,且没有和其他物品堆叠,则向DBServer发消息
	if(bInsert2DB && !itemMove.b_overlap)
	{
		// 绑定处理
		//if((EBS_Unknown == pItem->byBind) && bChangeOwner)
		//{
		//	pItem->Bind();
		//}
		
		insert_item_to_db(*pItem);
		pItem->SetUpdate(EUDBS_Null);

		SendAddNew2Client(pItem, dw_cmd_id==elcid_pickup_item?TRUE:FALSE);
	}
	else
	{
		SendAddItem2Client(container.GetConType(), n16Index, pItem->n64_serial, n16Num, itemMove.b_overlap,pItem->eCreateMode);
	}
	
	// 检查是否需要记录log
	log_item(*pItem, itemMove.p_item2, n16AddNum, dw_cmd_id, dwRoleIDRel);

	// 任务触发
	if(container.GetConType() == EICT_Bag || container.GetConType() == EICT_Quest)
	{
		m_pRole->on_quest_event(EQE_Item, pItem->dw_data_id, n16AddNum, TRUE);
		m_pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_add_item, pItem->dw_data_id, n16AddNum);
	}

	// 检查是否需释放该物品内存
	if(itemMove.b_overlap)
	{
		NET_DB2C_delete_item send;
		send.n64_serial = pItem->n64_serial;
		g_dbSession.Send(&send, send.dw_size);

		Destroy(pItem);
		pItem = itemMove.p_item2;
	}

	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
// 向容器中指定位置添加物品
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::Add2BagByIndex(tagItem *&pItem, DWORD dw_cmd_id, INT16 n16Index)
{
	ASSERT(VALID_POINT(pItem));
	ASSERT(VALID_POINT(pItem->n64_serial));

	DWORD		dw_error_code;

	// 加入容器
	dw_error_code =  GetBag().Add(pItem, n16Index, FALSE, FALSE);
	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
		print_message(_T("\nAdd item to container failed! \nroleid: %u, item serial: %lld, item typeid: %d, container type: %d!!!!\n\n"),
			GetBag().GetOwnerID(), pItem->n64_serial, pItem->pEquipProto->dw_data_id, GetBag().GetConType());

		//?? 考虑将该物品保存到log中

		return dw_error_code;
	}

	// 检查是否需要记录log
	log_item(*pItem, NULL, pItem->n16Num, dw_cmd_id);

	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
// 向容器中指定位置添加物品并存入数据库
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::Add2BagByIndexAndInsertDB( tagItem *&pItem, DWORD dw_cmd_id, INT16 n16Index )
{
	ASSERT(VALID_POINT(pItem));
	ASSERT(VALID_POINT(pItem->n64_serial));

	DWORD		dw_error_code;

	// 加入容器
	dw_error_code =  GetBag().Add(pItem, n16Index, TRUE, FALSE);
	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
		print_message(_T("\nAdd item to container failed! \nroleid: %u, item serial: %lld, item typeid: %d, container type: %d!!!!\n\n"),
			GetBag().GetOwnerID(), pItem->n64_serial, pItem->pEquipProto->dw_data_id, GetBag().GetConType());

		//?? 考虑将该物品保存到log中

		return dw_error_code;
	}

	//变态，绑定物品在ItemContainer::Add里面设置不了所有者，所以得在这里再弄一下
	pItem->SetOwner(m_pRole->GetID(), m_pRole->GetSession()->GetSessionID());

	// 记录数据库
	insert_item_to_db(*pItem);
	pItem->SetUpdate(EUDBS_Null);
	
	// 通知客户端
	SendAddNew2Client(pItem, FALSE);
//	SendAddItem2Client(EICT_Bag, pItem->n16Index, pItem->GetKey(), pItem->n16Num, pItem->pProtoType->n16MaxLapNum > 1);

	// 检查是否需要记录log
	log_item(*pItem, NULL, pItem->n16Num, dw_cmd_id);

	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除物品
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::RemoveItem(ItemContainer& container, INT64 n64_serial,  
						  DWORD dw_cmd_id, BOOL bDelFromDB, BOOL bDelMem, BOOL bCheckRemove, DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{
	tagItem *pItem = container.GetItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		ASSERT(VALID_POINT(pItem));
		print_message(_T("(RemoveItem1)Can not find item<serial: %lld> in container<Type: %d, RoleID: %u>!!!\r\n"),
					n64_serial, container.GetConType(), container.GetOwnerID());
		return INVALID_VALUE;
	}

	INT16 n16OldIndex	= pItem->n16Index;
	INT16 n16Num		= pItem->n16Num;

	DWORD dw_error_code = container.Remove(n64_serial, TRUE, bCheckRemove);
	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
		print_message(_T("\nRemove item<roleid:%u, container type: %d, serial:%lld, error:%u> from container failed!\n\n"),
			container.GetOwnerID(), container.GetConType(), n64_serial, dw_error_code);

		return dw_error_code;
	}

	// 物品上限管理
	RemoveMaxHoldItem(pItem->dw_data_id, pItem->n16Num);

	// 向客户端发消息
	SendDelItem2Client(container.GetConType(), n16OldIndex, n64_serial, 0, dw_cmd_id);
	
	// 检查是否需要记录log -- 即使此时物品要从游戏世界中删除,此时内存也没有释放
	log_item(*pItem, NULL, -n16Num, dw_cmd_id, dwRoleIDRel);

	// 任务触发
	if(container.GetConType() == EICT_Bag || container.GetConType() == EICT_Quest)
	{
		m_pRole->on_quest_event(EQE_Item, pItem->dw_data_id, n16Num, FALSE);
	}

	// 释放内存
	if(bDelMem)
	{
		if(VALID_POINT(pItem->pScript) && VALID_POINT(m_pRole) && VALID_POINT(m_pRole->get_map()))
		{
			pItem->pScript->DelItem(m_pRole->get_map(), m_pRole->GetID(), pItem->dw_data_id);
		}
		delete_item_from_db(n64_serial, pItem->dw_data_id);
		Destroy(pItem);
	}
	else if(bDelFromDB)
	{
		delete_item_from_db(n64_serial, pItem->dw_data_id);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除指定物品的个数(或次数)
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::RemoveItem(ItemContainer& container, INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id,
						  BOOL bCheckRemove/* = TRUE*/, BOOL bDelete/* = FALSE*/)
{
	if(n16Num <= 0)
	{
		ASSERT(0);
		return INVALID_VALUE;
	}
	
	tagItem *pItem = container.GetItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		//ASSERT(VALID_POINT(pItem));
		print_message(_T("(RemoveItem2)Can not find item<serial: %lld> in container<Type: %d, RoleID: %u>!!!\r\n"),
			n64_serial, container.GetConType(), container.GetOwnerID());
		return INVALID_VALUE;
	}
	
	/******************** 消耗使用次数 *******************/
	if(!bDelete && !pItem->CanOverlap() && pItem->pProtoType->nMaxUseTimes != 1)
	{
		ASSERT(!pItem->CanOverlap());

		// 不可堆叠物品减少的是使用次数
		pItem->IncUseTimes(n16Num);
		ASSERT( pItem->pProtoType->nMaxUseTimes == INVALID_VALUE || pItem->nUseTimes <= pItem->pProtoType->nMaxUseTimes );

		// mwh 2011-08-03 取消记录,db数据量太大
		// 检查是否需要记录log
		// LogItemTimes(*pItem, dw_cmd_id);

		//ASSERT(FALSE == bCheckRemove);
		if(pItem->pProtoType->nMaxUseTimes != INVALID_VALUE)
		{
			if(pItem->nUseTimes >= pItem->pProtoType->nMaxUseTimes)
			{
				return RemoveItem(container, n64_serial, dw_cmd_id, TRUE, TRUE, FALSE);
			}
			else
			{
				SendDelItem2Client(container.GetConType(), pItem->n16Index, n64_serial, pItem->nUseTimes, dw_cmd_id);
			}
		}

		return E_Success;
	}

	/********************* 消耗物品 ******************/
	ASSERT(n16Num <= pItem->n16Num);
	
	// 检查是否完全删除
	if(n16Num == pItem->n16Num)
	{
		return RemoveItem(container, n64_serial, dw_cmd_id, TRUE, TRUE, bCheckRemove);
	}
	
	INT16 n16OldIndex = pItem->n16Index;

	DWORD dw_error_code = container.Remove(n64_serial, n16Num, bCheckRemove);
	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
		print_message(_T("\nRemove item<roleid:%u, container type: %d, serial:%lld, error:%u> from container failed!\n\n"),
			container.GetOwnerID(), container.GetConType(), n64_serial, dw_error_code);

		return dw_error_code;
	}

	// 物品上限管理
	RemoveMaxHoldItem(pItem->dw_data_id, n16Num);

	// 向客户端发消息
	SendDelItem2Client(container.GetConType(), n16OldIndex, n64_serial, pItem->n16Num, dw_cmd_id);

	// 检查是否需要记录log
	log_item(*pItem, NULL, -n16Num, dw_cmd_id);

	// 任务触发
	if(container.GetConType() == EICT_Bag || container.GetConType() == EICT_Quest)
	{
		m_pRole->on_quest_event(EQE_Item, pItem->dw_data_id, n16Num, FALSE);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 同一容器内移动物品(n16Num = 0 表示全部移动)
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::Move(EItemConType eConType, INT64 n64_serial,	INT16 n16Num, INT16 n16PosDst, DWORD dw_cmd_id)
{
	ASSERT(n16Num >= 0);

	ItemContainer *pItemCon = GetContainer(eConType);
	if(!VALID_POINT(pItemCon))
	{
		return INVALID_VALUE;
	}

	tagItem *pItem1 = pItemCon->GetItem(n64_serial);
	if(!VALID_POINT(pItem1))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code	= INVALID_VALUE;
	INT16 n16SrcPos		= pItem1->n16Index;	// n64_serial原位置

	s_item_move	itemMove;

	if(0 == n16Num)
	{
		n16Num = pItem1->n16Num;
		dw_error_code = pItemCon->MoveTo(n64_serial, n16PosDst, itemMove);
	}
	else if(n16Num > 0)
	{
		dw_error_code = pItemCon->MoveTo(n64_serial, n16Num, n16PosDst, itemMove);
	}

	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
#ifdef _DEBUG
		print_message(_T("\nMove item<roleid:%u, container type: %d, serial:%lld, error:%u> in container failed!\n\n"),
			pItemCon->GetOwnerID(), pItemCon->GetConType(), n64_serial, dw_error_code);
#endif // _DEBUG

		return dw_error_code;
	}

	// 向客户端发消息
	NET_SIS_item_position_change send;
	send.eConType	= eConType;
	send.n64Serial1	= n64_serial;
	send.n64Serial2	= itemMove.p_item2 != NULL ? itemMove.p_item2->n64_serial : INVALID_VALUE;
	
	send.n16Num1	= itemMove.n_num_res1;
	send.n16Num2	= itemMove.n_num_res2;
	send.bCreateItem = itemMove.b_create_item;

	if(itemMove.b_change_pos)
	{
		send.n16PosDst1	= n16PosDst;
		send.n16PosDst2	= n16SrcPos;
	}
	else
	{
		send.n16PosDst1	= n16SrcPos;
		send.n16PosDst2	= n16PosDst;
	}

	if(itemMove.b_overlap)
	{
		send.n16PosDst1	= n16SrcPos;
		send.n16PosDst2	= n16PosDst;
	}

	SendMessage(&send, send.dw_size);

	// 检查是否需要记录log
	if(itemMove.b_create_item || itemMove.b_overlap)
	{
		log_item(*pItem1, itemMove.p_item2, -n16Num, dw_cmd_id);
	}

	// 检查移动的物品堆是否存在，如不存在,需与STDB同步和回收内存
	if(0 == itemMove.n_num_res1)
	{
		delete_item_from_db(n64_serial, pItem1->dw_data_id);

		// 检查是否需释放该物品内存
		Destroy(pItem1);
	}

	// 检查是否创建了新的物品堆
	if(itemMove.b_create_item)
	{
		NET_DB2C_new_item send;
		get_fast_code()->memory_copy(&send.item, itemMove.p_item2, SIZE_ITEM);

		g_dbSession.Send(&send, send.dw_size);

	}

	return dw_error_code;
}

VOID ItemMgr::Stack(EItemConType eConType)
{
	ItemContainer *pItemCon = GetContainer(eConType);

	for (INT16 i = 0; i < pItemCon->GetCurSpaceSize(); i++)
	{
		tagItem* pItem = pItemCon->GetItem(i);
		// 不能堆叠
		if (!VALID_POINT(pItem) ||
			!pItem->CanOverlap() || 
			pItem->n16Num >= pItem->pProtoType->n16MaxLapNum)
			continue;

		for (INT16 j = i+1; j < pItemCon->GetCurSpaceSize(); j++)
		{
			tagItem* pItem2 = pItemCon->GetItem(j);
			if (!VALID_POINT(pItem2))
				continue;

			if (pItem->dw_data_id != pItem2->dw_data_id || pItem->IsBind() != pItem2->IsBind() || pItem2->n16Num >= pItem2->pProtoType->n16MaxLapNum)
				continue;

			Move(eConType, pItem2->GetKey(), 0, i, elcid_item_move);

			if (pItem->n16Num >= pItem->pProtoType->n16MaxLapNum)
				break;

		}
	}

	//tagItem* pItem = NULL;
	//package_map<INT64, INT16>::map_iter it = pItemCon->Begin();
	//while (pItemCon->GetNextItem(it, pItem))
	//{
	//	// 不能堆叠
	//	if (!VALID_POINT(pItem) ||
	//		!pItem->CanOverlap() || 
	//		pItem->n16Num >= pItem->pProtoType->n16MaxLapNum)
	//		continue;

	//	tagItem* pItem2 = NULL;
	//	package_map<INT64, INT16>::map_iter it2 = it;
	//	while (pItemCon->GetNextItem(it2, pItem2))
	//	{
	//		if (pItem->dw_data_id != pItem2->dw_data_id || pItem->IsBind() != pItem2->IsBind() || pItem2->n16Num >= pItem2->pProtoType->n16MaxLapNum)
	//			continue;

	//		Move(eConType, pItem2->GetKey(), 0, pItemCon->GetIndex(pItem), elcid_item_move);

	//		if (pItem->n16Num >= pItem->pProtoType->n16MaxLapNum)
	//			break;
	//	}
	//}

}
//-------------------------------------------------------------------------------------------------------
// 容器间物品移动
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::move_to_other(EItemConType eConTypeSrc, INT64 n64Serial1, 
							 EItemConType eConTypeDst, INT16 n16PosDst, DWORD dw_cmd_id)
{
	ItemContainer *pConSrc = GetContainer(eConTypeSrc);
	if(!VALID_POINT(pConSrc))
	{
		return INVALID_VALUE;
	}

	ItemContainer *pConDst = GetContainer(eConTypeDst);
	if(!VALID_POINT(pConDst))
	{
		return INVALID_VALUE;
	}

	tagItem *pItem1 = pConSrc->GetItem(n64Serial1);
	if(!VALID_POINT(pItem1))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code	= INVALID_VALUE;
	INT16 n16PosSrc1	= pItem1->n16Index;	// n64_serial原位置
	INT16 n16Num		= pItem1->n16Num;
	DWORD dw_data_id		= pItem1->dw_data_id;

	INT16 n16destNum = 0;
	DWORD dw_dest_data_id = 0;
	if(!pConDst->IsOnePlaceFree(n16PosDst)){
		tagItem *pDstItem = pConDst->GetItem(n16PosDst);
		if(VALID_POINT(pDstItem)){
			n16destNum = pDstItem->n16Num;
			dw_dest_data_id = pDstItem->dw_data_id;
		}
	}

	s_item_move	itemMove;

	if(INVALID_VALUE == n16PosDst)	// 右击，没有指定目标位置
	{
		dw_error_code = pConSrc->MoveTo(n64Serial1, *pConDst, itemMove, n16PosDst);
	}
	else
	{
		dw_error_code = pConSrc->MoveTo(n64Serial1, *pConDst, n16PosDst, itemMove);
	}

	if(dw_error_code != E_Success)
	{
		ASSERT(E_Success == dw_error_code);
#ifdef _DEBUG
		print_message(_T("\nMove item<roleid:%u, container type: %d, serial:%lld, error:%u> in container failed!\n\n"),
			pConSrc->GetOwnerID(), pConSrc->GetConType(), n64Serial1, dw_error_code);
#endif // _DEBUG

		return dw_error_code;
	}

	// 向客户端发消息
	NET_SIS_item_position_change_extend send;

	send.eConTypeSrc1	= eConTypeSrc;
	send.eConTypeSrc2	= eConTypeDst;
	send.n64Serial1		= n64Serial1;
	send.n64Serial2		= itemMove.p_item2 != NULL ? itemMove.p_item2->n64_serial : INVALID_VALUE;
	send.n16Num1		= itemMove.n_num_res1;
	send.n16Num2		= itemMove.n_num_res2;

	if(itemMove.b_overlap)
	{
		send.eConTypeDst1	= eConTypeSrc;
		send.eConTypeDst2	= eConTypeDst;
		send.n16PosDst1		= n16PosSrc1;
		send.n16PosDst2		= n16PosDst;
	}
	else
	{
		send.eConTypeDst1	= eConTypeDst;
		send.eConTypeDst2	= eConTypeSrc;
		send.n16PosDst1		= n16PosDst;
		send.n16PosDst2		= n16PosSrc1;
	}

	SendMessage(&send, send.dw_size);

	// 记录百宝袋领取历史记录
	if (eConTypeSrc == EICT_Baibao && eConTypeDst == EICT_Bag)
	{
		//if ((itemMove.n_num_res1 != 0) && (EBS_Unknown == pItem1->byBind))
		//{
		//	pItem1->Bind();
		//}
		ProcBaiBaoRecord(dw_data_id, m_pRole->GetID(), m_pRole->GetID(), EBBRT_Myself);
	}

	// 任务触发
	if( eConTypeSrc == EICT_RoleWare && eConTypeDst == EICT_Bag)
	{
		if(itemMove.b_overlap) n16Num -= send.n16Num1;
		if(itemMove.b_overlap) n16destNum -= send.n16Num2;
		if(n16Num) m_pRole->on_quest_event(EQE_Item, dw_data_id, n16Num, TRUE);
		if(dw_dest_data_id && n16destNum ){
			m_pRole->on_quest_event(EQE_Item, dw_dest_data_id, n16destNum, FALSE);
		}
	}
	else if(eConTypeSrc == EICT_Bag && eConTypeDst == EICT_RoleWare)
	{
		if(itemMove.b_overlap) n16Num -= send.n16Num1;
		if(itemMove.b_overlap) n16destNum -= send.n16Num2;
		if(n16Num)m_pRole->on_quest_event(EQE_Item, dw_data_id, n16Num, FALSE);
		if(dw_dest_data_id && n16destNum){
			m_pRole->on_quest_event(EQE_Item, dw_dest_data_id, n16destNum, TRUE);
		}
	}


	// 检查是否需要记录log
	log_item(*pItem1, itemMove.p_item2, pItem1->n16Num - n16Num, dw_cmd_id);

	// 检查移动的物品堆是否存在，如不存在,需与DBServer同步和回收内存
	if(0 == itemMove.n_num_res1)
	{
		delete_item_from_db(n64Serial1, pItem1->dw_data_id);

		// 释放该物品内存
		Destroy(pItem1);
	}

	return dw_error_code;
}

//-------------------------------------------------------------------------------------------------------
// 添加背包中多个物品(玩家间交易,邮件) -- private
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::AddItems(ItemContainer& container, tagItem* pItem[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel)
{
	DWORD dw_error_code;
	for(INT32 i=0; i<nSize; ++i)
	{
		if(NULL == pItem[i])
		{
			return;
		}

		if(MIsQuestItem(pItem[i]->pProtoType))
		{
			dw_error_code = add_item(GetQuestItemBag(), pItem[i], dw_cmd_id, TRUE, TRUE, dwRoleIDRel);
		}
		else
		{
			dw_error_code = add_item(GetBag(), pItem[i], dw_cmd_id, TRUE, TRUE, dwRoleIDRel);
		}

		/**/
		ASSERT(E_Success == dw_error_code);
	}
}

//-------------------------------------------------------------------------------------------------------
// 删除背包中多个物品(玩家间交易,邮件)
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::RemoveItems(ItemContainer& container, INT64 n64_serial[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel)
{
	for(INT32 i=0,j=0; i<nSize; ++i)
	{
		if(n64_serial[i] != 0)
		{
			DWORD dw_error_code = RemoveItem(container, n64_serial[i], dw_cmd_id, TRUE, FALSE, FALSE, dwRoleIDRel);
			ASSERT(E_Success == dw_error_code);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除某类物品(该类物品被销毁)，返回成功个数
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::RemoveItems(ItemContainer& container, DWORD dw_data_id, DWORD dw_cmd_id, INT nBind)
{
	DWORD dw_error_code;

	package_list<tagItem*> itemList;
	if (nBind == INVALID_VALUE)
	{
		container.GetSameItemList(itemList, dw_data_id);
	}
	else
	{
		container.GetSameBindItemList(itemList, dw_data_id, nBind);
	}
	tagItem *pItem;
	itemList.reset_iterator();
	while(itemList.find_next(pItem))
	{		
		dw_error_code = RemoveItem(container, pItem->n64_serial, dw_cmd_id, TRUE, TRUE, FALSE);
		ASSERT(E_Success == dw_error_code);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除指定个数的某类物品(该物品被销毁)，返回成功个数
//-------------------------------------------------------------------------------------------------------
DWORD ItemMgr::RemoveItems(ItemContainer& container, DWORD dw_data_id, INT32 n_num, DWORD dw_cmd_id, INT nBind)
{
	if(n_num <= 0)
	{
		ASSERT(n_num > 0);
		return INVALID_VALUE;
	}
	
	package_list<tagItem*> itemList;
	INT32 nNumTotal = 0;
	if (nBind == INVALID_VALUE)
	{
		nNumTotal = container.GetSameItemList(itemList, dw_data_id);

	}
	else
	{
		nNumTotal = container.GetSameBindItemList(itemList, dw_data_id, nBind);

	}
	
	// 不够
	if(nNumTotal < n_num)
	{
		// 删除失败,发送监控信息 //??

		//ASSERT(nNumTotal >= n_num);
		return INVALID_VALUE;
	}

	// 刚好, 或富余
	DWORD dw_error_code;
	INT16 n16NumDel = 0;
	INT32 nNumNeedDel = n_num;
	tagItem *pItem = NULL;
	itemList.reset_iterator();
	while(itemList.find_next(pItem) && nNumNeedDel != 0)
	{
		if(pItem->n16Num <= nNumNeedDel)
		{
			n16NumDel = pItem->n16Num;
			dw_error_code = RemoveItem(container, pItem->n64_serial, dw_cmd_id, TRUE, TRUE, FALSE);
			if(dw_error_code != E_Success)
			{
				ASSERT(E_Success == dw_error_code);
				n16NumDel = 0;
			}
			
			nNumNeedDel -= n16NumDel;
		}
		else
		{
			n16NumDel = nNumNeedDel;
			dw_error_code = RemoveItem(container, pItem->n64_serial, nNumNeedDel, dw_cmd_id, FALSE);
			if(dw_error_code != E_Success)
			{
				ASSERT(E_Success == dw_error_code);
				n16NumDel = 0;
			}

			nNumNeedDel -= n16NumDel;
		}
	}

	ASSERT(0 == nNumNeedDel);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 从游戏中删除和指定任务相关的物品
//-----------------------------------------------------------------------------
VOID ItemMgr::RemoveItems(ItemContainer& container, UINT16 u16QuestID, DWORD dw_cmd_id)
{
	const tagItem *pItem;
	for(INT16 i=0; i<container.GetCurSpaceSize(); ++i)
	{
		pItem = container.GetItem(i);
		if(!VALID_POINT(pItem))
		{
			continue;
		}

		// 和任务相关则删除该物品
		if(u16QuestID == pItem->pProtoType->dwQuestID)
		{
			RemoveItem(container, pItem->n64_serial, dw_cmd_id, TRUE, TRUE, FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
// 从装备栏中删除和指定任务相关的物品
//-----------------------------------------------------------------------------
VOID ItemMgr::RemoveItems(EquipContainer& container, UINT16 u16QuestID, DWORD dw_cmd_id)
{
	const tagItem *pItem;
	for(INT16 i=0; i<container.GetCurSpaceSize(); ++i)
	{
		pItem = container.GetItem(i);
		if(!VALID_POINT(pItem))
		{
			continue;
		}

		// 和任务相关则删除该物品
		if(u16QuestID == pItem->pProtoType->dwQuestID)
		{
			RemoveFromEquipBar(pItem->n64_serial, dw_cmd_id, TRUE);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// 检查背包中多个物品是否存在(玩家间交易,邮件)
//-------------------------------------------------------------------------------------------------------
BOOL ItemMgr::CheckItemsExist(OUT tagItem* pItem[], ItemContainer& container, 
							  INT64 n64_serial[], INT16 n16Num[], INT32 nSize)
{
	ASSERT(pItem != NULL);
	ASSERT(n64_serial != NULL);
	ASSERT(n16Num != NULL);
	
	for(INT32 i=0,j=0; i<nSize; ++i)
	{
		if(n64_serial[i] != 0)
		{
			pItem[j] = container.GetItem(n64_serial[i]);
			if(!VALID_POINT(pItem[j]) || pItem[j]->n16Num != n16Num[i])
			{
				return FALSE;
			}

			++j;
		}
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// 向客户端发送消息
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::SendMessage(LPVOID p_message, DWORD dw_size)
{
	ASSERT(VALID_POINT(m_pRole));

	PlayerSession *pSession = m_pRole->GetSession();
	if(VALID_POINT(pSession))
	{
		pSession->SendMessage(p_message, dw_size);
	}

	ASSERT(VALID_POINT(pSession));
}

//-------------------------------------------------------------------------------------------------------
// 向容器中添加物品，发送到客户端消息封装
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::SendAddNew2Client(const tagItem *pItem, BOOL bPickUp)
{
	if(MIsEquipment(pItem->dw_data_id))
	{
		NET_SIS_new_equip_add Send;
		get_fast_code()->memory_copy(&Send.Equip, pItem, SIZE_EQUIP);
		//Send.Equip.equipSpec.n16QltyModPctEx = 0;	// 对客户端隐藏二级修正率
		Send.bPickUp = bPickUp;
		SendMessage(&Send, Send.dw_size);
	}
	else
	{
		NET_SIS_new_item_add Send;
		get_fast_code()->memory_copy(&Send.Item, pItem, SIZE_ITEM);

		SendMessage(&Send, Send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 向容器中添加物品，发送到客户端消息封装
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::SendAddItem2Client(EItemConType eConType, INT16 n16Index, INT64 n64_serial, INT16 n16Num, BOOL bOverlap,EItemCreateMode eCreateMode)
{
	NET_SIS_item_add	send;
	send.n64_serial	= n64_serial;
	send.n16Index	= n16Index;
	send.n16Num		= n16Num;
	send.eConType	= eConType;
	send.bOverlap	= bOverlap ? true : false;
	send.eCreateMode = eCreateMode;//gx add
	SendMessage(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除物品，发送到客户端消息封装
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::SendDelItem2Client(EItemConType eConType, INT16 n16Index, INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id)
{
	NET_SIS_item_remove send;
	//send.dw_error_code= dw_error_code;
	send.n64_serial	= n64_serial;
	send.n16Index	= n16Index;
	send.n16Num		= n16Num;
	send.eConType	= eConType;
	send.dwCmdID	= dw_cmd_id;
	SendMessage(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 向DBServer发消息
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::insert_item_to_db(tagItem &item)
{
	if(MIsEquipment(item.dw_data_id))
	{
		NET_DB2C_new_equip send;
		get_fast_code()->memory_copy(&send.equip, &item, SIZE_EQUIP);
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_new_item send;
		get_fast_code()->memory_copy(&send.item, &item, SIZE_ITEM);
		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 向DBServer发消息
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::delete_item_from_db(INT64 n64_serial, INT32 dw_data_id)
{
	if(!MIsEquipment(dw_data_id))
	{
		NET_DB2C_delete_item send;
		send.n64_serial = n64_serial;

		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_delete_equip send;
		send.n64_serial = n64_serial;

		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 更新物品信息 -- 保存角色数据时调用
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::SaveItem2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	INT32 nTmpNum;
	n_num = 0;

	Save2DB(GetBag(), pData, pData, nTmpNum);
	n_num += nTmpNum;

	Save2DB(GetQuestItemBag(), pData, pData, nTmpNum);
	n_num += nTmpNum;

	Save2DB(GetBaiBaoBag(), pData, pData, nTmpNum);
	n_num += nTmpNum;

	Save2DB(GetRoleWare(), pData, pData, nTmpNum);
	n_num += nTmpNum;

	Save2DB(GetEquipBar(), pData, pData, nTmpNum);
	n_num += nTmpNum;

	pOutPointer = pData;
}

//-----------------------------------------------------------------------------
// 装备属性改变后，即时保存数据库
//-----------------------------------------------------------------------------
VOID ItemMgr::SendEquipSpec2DB(const tagEquip &equip)
{
	NET_DB2C_update_equip_att send;
	send.equip_spec_update.n64_serial = equip.n64_serial;
	get_fast_code()->memory_copy(&send.equip_spec_update.equip_spec, &equip.equipSpec, sizeof(tagEquipSpec));

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// 装备属性改变后，向客户端发消息
//-----------------------------------------------------------------------------
VOID ItemMgr::SendEquipSpec2Client(const tagEquip &equip)
{
	NET_SIS_equip_change send;
	send.n64_serial = equip.n64_serial;
	get_fast_code()->memory_copy(&send.equipSpec, &equip.equipSpec, sizeof(tagEquipSpec));
	//send.equipSpec.n16QltyModPctEx = 0;	// 对客户端隐藏二级修正率
	SendMessage(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 每个tick调用一次
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::Update()
{
	UpdateCDTime();

	// 容器update
	UpdateContainer(m_Bag);
	UpdateContainer(m_QuestItemBag);
	UpdateContainer(m_BaiBaoBag);
	UpdateContainer(m_RoleWare);
	UpdateContainer(m_EquipBar);
}

//-------------------------------------------------------------------------------------------------------
// 每个tick调用一次
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::UpdateContainer(ItemContainer& sItemCon)
{
	sItemCon.Update();

	package_list<INT64>& lstNeedDel = sItemCon.GetNeedDelList();
	if(lstNeedDel.size() > 0)
	{
		INT64 n64_serial;
		package_list<INT64>::list_iter iter = lstNeedDel.begin();
		while(lstNeedDel.find_next(iter, n64_serial))
		{
			if(this->RemoveItem(sItemCon, n64_serial, (DWORD)elcid_item_at_term, TRUE, TRUE, FALSE) != E_Success)
			{
				ASSERT(0);
				print_message(_T("Item<%ld> time at term, but delete failed! Please check!\n"), n64_serial);
			}
		}

		sItemCon.ClearNeedDelList();
	}

	/*package_list<INT64>& lstNeedChangeBind = sItemCon.GetChangeBindList();
	if (lstNeedChangeBind.size() > 0)
	{
		INT64 n64_serial;
		package_list<INT64>::list_iter iter = lstNeedChangeBind.begin();
		while(lstNeedChangeBind.find_next(iter, n64_serial))
		{
			tagItem* pItem = sItemCon.GetItem(n64_serial);
			if(VALID_POINT(pItem))
			{
			
				EBindStatus eBindType = EBS_Unbind;
				if (pItem->bCreateBind && !MEquipIsRide(pItem->pEquipProto))
				{
					eBindType = EBS_SYSTEM_Bind;
				}
				//else if(sItemCon.GetConType() == EICT_Equip)
				//{
				//	eBindType = EBS_Equip_Bind;
				//}

				if (MIsEquipment(pItem->dw_data_id))
				{
					tagEquip* pEquip = (tagEquip*)pItem;

					//if (pEquip->eConType == EICT_Equip)
					//{
					//	m_pRole->ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, -1);
					//	m_pRole->RecalAtt(TRUE);
					//}

					// 清楚绑定属性
					ItemCreator::RemoveEquipBindAtt(pEquip);
					UpdateEquipSpec(*pEquip);

					// 修炼过的武器绑定
					//if (pEquip->pEquipProto->eEquipPos == EEP_RightHand && pEquip->equipSpec.nLevel > 1)
					//{
					//	eBindType = EBS_Equip_Bind;
					//}

				}

				pItem->SetBind(eBindType);
				pItem->SetBindTime(0);

				NET_SIS_bind_change send;
				send.n64EquipSerial = pItem->n64_serial;
				send.byBind = pItem->byBind;
				send.time = 0;
				send.dwConType = pItem->eConType;
				SendMessage(&send, send.dw_size);
			}
		}

		sItemCon.ClearChangeBindList();
	}*/
}

//-------------------------------------------------------------------------------------------------------
// 每个tick调用一次
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::UpdateContainer(EquipContainer& sEquipCon)
{
	sEquipCon.Update();

	package_list<INT64>& lstNeedDel = sEquipCon.GetNeedDelList();
	if(lstNeedDel.size() > 0)
	{
		INT64 n64_serial;
		package_list<INT64>::list_iter iter = lstNeedDel.begin();
		while(lstNeedDel.find_next(iter, n64_serial))
		{
			this->RemoveFromEquipBar(n64_serial, (DWORD)elcid_item_at_term, TRUE);
		}

		sEquipCon.ClearNeedDelList();
	}


	//package_list<INT64>& lstNeedChangeBind = sEquipCon.GetChangeBindList();
	//if (lstNeedChangeBind.size() > 0)
	//{
	//	INT64 n64_serial;
	//	package_list<INT64>::list_iter iter = lstNeedChangeBind.begin();
	//	while(lstNeedChangeBind.find_next(iter, n64_serial))
	//	{
	//		tagItem* pItem = sEquipCon.GetItem(n64_serial);
	//		if(VALID_POINT(pItem))
	//		{
	//			EBindStatus eBindType = EBS_Unbind;
	//			if (pItem->bCreateBind && !MEquipIsRide(pItem->pEquipProto))
	//			{
	//				eBindType = EBS_Bind;
	//			}
	//			
	//			if (MIsEquipment(pItem->dw_data_id))
	//			{
	//				tagEquip* pEquip = (tagEquip*)pItem;
	//				
	//				// 清楚绑定属性
	//				m_pRole->ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, -1);
	//				m_pRole->RecalAtt(TRUE);
	//
	//				ItemCreator::RemoveEquipBindAtt(pEquip);
	//				UpdateEquipSpec(*pEquip);
	//			
	//				// 修炼过的武器绑定
	//				//if (pEquip->pEquipProto->eEquipPos == EEP_RightHand && pEquip->equipSpec.nLevel > 1)
	//				//{
	//				//	eBindType = EBS_Equip_Bind;
	//				//}

	//			}

	//			pItem->SetBind(eBindType);
	//			pItem->SetBindTime(0);

	//			NET_SIS_bind_change send;
	//			send.n64EquipSerial = pItem->n64_serial;
	//			send.byBind = pItem->byBind;
	//			send.time = pItem->dwBindTime;
	//			send.dwConType = pItem->eConType;
	//			SendMessage(&send, send.dw_size);
	//		}
	//		
	//	}

	//	sEquipCon.ClearChangeBindList();
	//}
}

//-------------------------------------------------------------------------------------------------------
// 每个tick更新一次冷却时间
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::UpdateCDTime()
{
	if(m_mapCDTime.size() == 0)
	{
		return;
	}

	DWORD	dw_data_id;
	DWORD	dw_time;
	MapCDTime::map_iter iter = m_mapCDTime.begin();
	while(m_mapCDTime.find_next(iter, dw_data_id, dw_time))
	{
		if(dw_time <= TICK_TIME)
		{
			m_mapCDTime.erase(dw_data_id);
			continue;
		}

		m_mapCDTime.change_value(dw_data_id, dw_time - TICK_TIME);
	}
}

//-------------------------------------------------------------------------------------------------------
// 添加新类型物品的冷却时间
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::Add2CDTimeMap(DWORD dw_data_id, DWORD dwCDTime/* = INVALID_VALUE*/)
{
	if ( m_pRole->ObjectCoolOff() == TRUE)
	{
		m_mapCDTime.add(dw_data_id, 0);
		return;
	}

	if(INVALID_VALUE == dwCDTime)
	{
		MapCDTime mapCDTime;
		GetSameCDItemList(mapCDTime, dw_data_id);
		
		DWORD dwTempCDTime	= INVALID_VALUE;
		DWORD dwTempTypeID	= INVALID_VALUE;
		MapCDTime::map_iter iter = mapCDTime.begin();
		while (mapCDTime.find_next(iter, dwTempTypeID, dwTempCDTime))
		{
			if (!VALID_VALUE(dwTempTypeID) || !VALID_VALUE(dwTempCDTime) || (dwTempCDTime == 0))
			{
				continue;
			}

			// 已经检查过原表中是否存在
			m_mapCDTime.add(dwTempTypeID, dwTempCDTime);

			NET_SIS_item_cd_update	send;
			send.dw_data_id = dwTempTypeID;
			send.dwCDTime = dwTempCDTime;
			m_pRole->SendMessage(&send, send.dw_size);
		}
	}
	else
	{
		m_mapCDTime.add(dw_data_id, dwCDTime);

		NET_SIS_item_cd_update	send;
		send.dw_data_id = dw_data_id;
		send.dwCDTime = dwCDTime;
		m_pRole->SendMessage(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 取得背包中同组CD物品类型列表
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::GetSameCDItemList(package_map<DWORD, DWORD> &mapSameCD, DWORD dw_data_id)
{
	mapSameCD.clear();

	// 上层逻辑错误
	if (m_mapCDTime.is_exist(dw_data_id))
	{
		ASSERT(!m_mapCDTime.is_exist(dw_data_id));
		return;
	}

	tagItemProto *pProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
	if(!VALID_POINT(pProto))
	{
		ASSERT(VALID_POINT(pProto));
		return;
	}

	// 没有同组物品
	if (pProto->eTypeReserved == EITR_Null)
	{
		// 将自身类型加入
		mapSameCD.add(dw_data_id, pProto->dwCooldownTime);
		return;
	}

	// 遍历背包
	tagItem* pItem = NULL;
	package_map<INT64, INT16>::map_iter iter = GetBag().Begin();

	while(GetBag().GetNextItem(iter, pItem))
	{
		if (!VALID_POINT(pItem))
		{
			continue;
		}
		if (mapSameCD.is_exist(pItem->dw_data_id) || m_mapCDTime.is_exist(pItem->dw_data_id))
		{
			continue;
		}
		if (pItem->pProtoType->eTypeReserved == pProto->eTypeReserved)
		{
			if (pItem->pProtoType->dwCooldownTime != 0)
			{
				mapSameCD.add(pItem->dw_data_id, pItem->pProtoType->dwCooldownTime);
			}
			else
			{
				mapSameCD.add(pItem->dw_data_id, 1000);
			}
		}
	}

	// 遍历仓库
	pItem = NULL;
	iter = GetRoleWare().Begin();

	while(GetRoleWare().GetNextItem(iter, pItem))
	{
		if (!VALID_POINT(pItem))
		{
			continue;
		}
		if (mapSameCD.is_exist(pItem->dw_data_id) || m_mapCDTime.is_exist(pItem->dw_data_id))
		{
			continue;
		}
		if (pItem->pProtoType->eTypeReserved == pProto->eTypeReserved)
		{
			if (pItem->pProtoType->dwCooldownTime != 0)
			{
				mapSameCD.add(pItem->dw_data_id, pItem->pProtoType->dwCooldownTime);
			}
			else
			{
				mapSameCD.add(pItem->dw_data_id, 1000);
			}
		}
	}

}

//-------------------------------------------------------------------------------------------------------
// 物品是否处于冷却时间
//-------------------------------------------------------------------------------------------------------
BOOL ItemMgr::IsItemCDTime(DWORD dw_data_id)
{
	if(m_mapCDTime.is_exist(dw_data_id))
		return TRUE;
	else
		return FALSE;
}

//-------------------------------------------------------------------------------------------------------
// 将冷却时间保存到数据库中
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::FormatCDTime(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	if(!VALID_POINT(m_pRole))
	{
		ASSERT(VALID_POINT(m_pRole));
		return;
	}
	
	n_num = m_mapCDTime.size();
	pOutPointer = pData;

	if(n_num != 0)
	{
		DWORD	dw_data_id;
		DWORD	dw_time;
		INT32	nCounter = 0;

		M_trans_pointer(pCDTime, pData, tagCDTime);
		MapCDTime::map_iter iter = m_mapCDTime.begin();
		while(m_mapCDTime.find_next(iter, dw_data_id, dw_time))
		{
			pCDTime[nCounter].dw_data_id	= dw_data_id;
			pCDTime[nCounter].dw_time	= dw_time;

			++nCounter;
		}

		pOutPointer = &pCDTime[nCounter];
	}
}

//-------------------------------------------------------------------------------------------------------
// 将冷却时间发送到客户端
//-------------------------------------------------------------------------------------------------------
VOID ItemMgr::SendInitStateItemCDTime()
{
	INT32 n_num = m_mapCDTime.size();
	if(0 == n_num)
	{
		return;
	}

	INT32 nSize = sizeof(NET_SIS_get_role_init_state_itemcdtime) - 1 + sizeof(tagCDTime) * n_num;

	CREATE_MSG(pSend, nSize, NET_SIS_get_role_init_state_itemcdtime);
	pSend->n_num = n_num;

	LPVOID pDummy;
	FormatCDTime(pSend->byData, pDummy, n_num);

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

//-----------------------------------------------------------------------------
// 数组中是否有重复ID
//-----------------------------------------------------------------------------
BOOL ItemMgr::IsRepeatID(INT64 n64ItemID[], INT32 nArraySz)
{
	//定以一个数组用来承接需要测试的数组
	set<INT64> idSet;

	for(INT n = 0; n < nArraySz; ++n)
	{
		idSet.insert(n64ItemID[n]);
	}

	if(idSet.size() != nArraySz)
		return TRUE;
	else
		return FALSE;
}

// 获取装备中特定宝石等级宝石id
DWORD ItemMgr::getGemIDbyLevel(tagEquip* pEquip, int nLevel)
{
	package_map<int, DWORD> map_gem;
	for (int i = 0; i < MAX_EQUIPHOLE_NUM; i++)
	{
		// 防止原石防暴.策划把原石配成了13
		DWORD dwGemID = pEquip->equipSpec.dwHoleGemID[i];
		if (dwGemID != 0 && dwGemID != -1 && dwGemID % 100 >= nLevel && dwGemID % 100 < 13)
		{
			map_gem.add(i, dwGemID);
		}
		
	}
	
	if (map_gem.empty())
		return 0;

	int index = 0;
	DWORD dwGem = 0;
	map_gem.rand_find(index, dwGem);

	m_pRole->EquipEffect(pEquip, FALSE);
	pEquip->equipSpec.dwHoleGemID[index] = 0;
	UpdateEquipSpec(*pEquip);
	m_pRole->EquipEffect(pEquip, TRUE);
	
	return dwGem;

}

//-----------------------------------------------------------------------------
// 计算占用格子数
//-----------------------------------------------------------------------------
BOOL ItemMgr::CalSpaceUsed(DWORD dw_data_id, INT32 n_num, OUT INT16 &n16UseBagSpace, 
						   OUT INT16 &n16UseQuestSpace, OUT INT16 &n16MaxLap)
{
	n16UseBagSpace = n16UseQuestSpace = 0;
	
	const tagItemProto *pProto;
	if(!MIsEquipment(dw_data_id))
	{
		pProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
	}
	else
	{
		pProto = AttRes::GetInstance()->GetEquipProto(dw_data_id);
	}

	if(!VALID_POINT(pProto))
	{
		ASSERT(0);
		print_message(_T("Cannot find item/equip proto<id: %ld>!"), dw_data_id);
		return FALSE;
	}

	// 计算占用的格子
	INT16 nUseSpace = 0;
	if(n_num <= pProto->n16MaxLapNum)
	{
		nUseSpace = 1;
	}
	else
	{
		nUseSpace = (1 == pProto->n16MaxLapNum) ? n_num : ((n_num - 1) / pProto->n16MaxLapNum + 1);
	}
	
	// 看是背包还是任务栏物品
	if(MIsQuestItem(pProto))
	{
		n16UseQuestSpace = nUseSpace;
	}
	else
	{
		n16UseBagSpace = nUseSpace;
	}

	n16MaxLap = pProto->n16MaxLapNum;

	return TRUE;
}

//-----------------------------------------------------------------------------
// 是否是任务物品
//-----------------------------------------------------------------------------
BOOL ItemMgr::IsQuestItem(DWORD dw_data_id)
{
	const tagItemProto *pProto;
	if(!MIsEquipment(dw_data_id))
	{
		pProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
	}
	else
	{
		pProto = AttRes::GetInstance()->GetEquipProto(dw_data_id);
	}

	if(!VALID_POINT(pProto))
	{
		ASSERT(0);
		print_message(_T("Cannot find item/equip proto<id: %ld>!"), dw_data_id);
		return FALSE;
	}

	return MIsQuestItem(pProto);
}

//-----------------------------------------------------------------------------
// 从装备栏上直接删除一件装备
//-----------------------------------------------------------------------------
tagItem* ItemMgr::RemoveFromEquipBar(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelMem)
{
	tagItem *pItem = GetEquipBar().GetItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		ASSERT(VALID_POINT(pItem));
		print_message(_T("(RemoveFromEquipBar)Can not find item<serial: %lld> in container<Type: %d, RoleID: %u>!!!\r\n"),
			n64_serial, GetEquipBar().GetConType(), m_pRole->GetID());
		return NULL;
	}

	INT16 n16OldIndex = pItem->n16Index;
	
	// 脱下装备
	tagEquip *pEquipSrc = GetEquipBar().Remove(n64_serial);
	if(NULL == pEquipSrc)
	{
		ASSERT(pEquipSrc != NULL);
		return NULL;
	}

	// 向客户端发消息
	SendDelItem2Client(GetEquipBar().GetConType(), n16OldIndex, n64_serial, 0, dw_cmd_id);

	// 计算影响
	m_pRole->ProcEquipEffect(NULL, (tagEquip*)pItem, n16OldIndex, TRUE);

	// 从数据库中删除
	delete_item_from_db(n64_serial, pItem->dw_data_id);

	// 检查是否需要记录log -- 即使此时物品要从游戏世界中删除,此时内存也没有释放
	log_item(*pItem, NULL, 1, dw_cmd_id);

	// 检查是否需释放该物品内存
	if(bDelMem)
	{
		Destroy(pItem);
		return NULL;
	}

	return pItem;
}

//-----------------------------------------------------------------------------
// 背包&角色仓库扩容
//-----------------------------------------------------------------------------
DWORD ItemMgr::ExtendBag(INT64 n64ItemID, Role* pRole, INT32 n32_type)
{
		
	ItemContainer* pCon = NULL;

	if (n32_type == 0)
	{
		pCon = GetContainer(EICT_Bag);
	}
	else if (n32_type == 1)
	{
		pCon = GetContainer(EICT_RoleWare);
	}


	if (!VALID_POINT(pCon))
	{
		return E_Con_Cannot_Extend;
	}
	
	// 是否可扩
	if(pCon->GetMaxSpaceSize() <= pCon->GetCurSpaceSize())
	{
		return E_Con_Cannot_Extend;
	}

	// 扩充道具阶段判定
	//DWORD dw_error = pRole->exbag_check(pItem->pProtoType->nSpecFuncVal1, pItem->pProtoType->byLevel);
	//if(dw_error != E_Success)
	//{
	//	return dw_error;
	//}
	
	INT nCurNum = pCon->GetCurSpaceSize() - BAG_EXTERN_BEGIN_NUM + 1;

	// 看物品够不够
	tagItem* pItem = GetBag().GetItem(n64ItemID);
	if (VALID_POINT(pItem))
	{
		if (pItem->pProtoType->eSpecFunc != EIST_BagExtand)
		{
			return E_Con_Cannot_itemExtend;
		}
		
		if (pItem->n16Num < nCurNum)
		{
			return E_Con_Cannot_itemExtend;
		}

		// 判断通过，删除物品
		ItemUsedFromBag(n64ItemID, nCurNum, (DWORD)elcid_bag_extend);

	}//再看绑定元宝
	//else if( pRole->GetCurMgr().GetBagYuanBao() >= nCurNum)
	//{
	//	pRole->GetCurMgr().DecBagYuanBao(nCurNum, elcid_bag_extend);
	//}//最后看元宝
	if(pRole->GetCurMgr().GetBaiBaoYuanBao() >= nCurNum)
	{
		pRole->GetCurMgr().DecBaiBaoYuanBao(nCurNum, elcid_bag_extend);
	}
	else
	{
		return INVALID_VALUE;
	}


	// 计算扩充大小
	//INT16 n16ExtendSz = min(pItem->pProtoType->nSpecFuncVal2, pCon->GetMaxSpaceSize() - pCon->GetCurSpaceSize());

	// 扩充背包
	pCon->IncreaseSize(1);

	//pRole->inc_exbag_stet(pItem->pProtoType->nSpecFuncVal1);

	//n32_type = pItem->pProtoType->nSpecFuncVal1;

	/*if(n32_type == 1)
	{
		NET_DB2C_ware_size_update send;
		send.dw_account_id = pRole->GetSession()->GetSessionID();
		send.n16_ware_size = pCon->GetCurSpaceSize();
		send.n16_ware_step = pRole->get_ware_step();
		g_dbSession.Send(&send, send.dw_size);
	}*/


	//pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_extendbag, nConType, pCon->GetCurSpaceSize());

	return E_Success;
}

//DWORD ItemMgr::ExtendRoleWare(INT64 n64ItemID)
//{
//	ItemContainer &ware = GetRoleWare();
//
//	// 是否可扩
//	if(ware.GetMaxSpaceSize() <= ware.GetCurSpaceSize())
//	{
//		return E_Con_Cannot_Extend;
//	}
//
//	// 计算已扩充次数
//	INT32 nExTimes = MCalWareExTimes(ware.GetCurSpaceSize());
//
//	// 获取货币管理器
//	CurrencyMgr &curMgr = m_pRole->GetCurMgr();
//
//	// 根据扩充次数判断并扣除对应消耗
//	if(bUseSilver)
//	{
//		// 消耗金钱
//		INT32 nCost = MCalWareExSilver(nExTimes);
//		if(nCost > curMgr.GetBagSilver())
//		{
//			return E_Silver_NotEnough;
//		}
//
//		// 扣除金钱
//		curMgr.DecBagSilver(nCost, ELCLD_RoleWare_Extend);
//	}
//	else
//	{
//		// 消耗元宝
//		INT32 nCost = MCalWareExYuanBao(nExTimes);
//		if(nCost > curMgr.GetBagYuanBao())
//		{
//			return E_YuanBao_NotEnough;
//		}
//
//		// 扣除金钱
//		curMgr.DecBagYuanBao(nCost, ELCLD_RoleWare_Extend);
//	}
//
//	// 计算扩充大小
//	INT16 n16ExtendSz = min(SPACE_WARE_PER_EXTEND, ware.GetMaxSpaceSize() - ware.GetCurSpaceSize());
//
//	// 扩充背包
//	ware.IncreaseSize(n16ExtendSz);
//
//	return E_Success;
//}

//-----------------------------------------------------------------------------
// 从背包中取出，并丢弃 -- 从db中清除，绑定物品内释放
//-----------------------------------------------------------------------------
DWORD ItemMgr::DiscardFromBag(INT64 n64_serial, DWORD dw_cmd_id, OUT tagItem *&pOut, BYTE by_type)
{
	pOut = NULL;

	// 行囊是否解锁
	/*if(!m_pRole->GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}*/

	tagItem *pItem = NULL;
	if(by_type == 1)
	{
		pItem = GetQuestItemBag().GetItem(n64_serial);
	}
	else
	{
		pItem = GetBag().GetItem(n64_serial);
	}
	
	if(!VALID_POINT(pItem))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	if(pItem->IsLock())
	{
		return E_Equip_Lock;
	}
	
	if(VALID_POINT(pItem->pProtoType) && pItem->pProtoType->bCantDrop)
		return INVALID_VALUE;

	/*if(pItem->IsBind())
	{
		return RemoveItem(GetBag(), n64_serial, dw_cmd_id, TRUE, TRUE, TRUE);
	}

	pOut = pItem;*/
	DWORD dw_error = E_Success;
	if(by_type == 1)
	{
		RemoveItem(GetQuestItemBag(), n64_serial, dw_cmd_id, TRUE, TRUE, TRUE);
	}
	else
	{
		RemoveItem(GetBag(), n64_serial, dw_cmd_id, TRUE, TRUE, TRUE);
	}
	
	//if (VALID_POINT(m_pRole))
	//{
	//	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_delete_item , 1);
	//}
	
	return dw_error;
}

//-----------------------------------------------------------------------------
// 从背包中掉落 -- 从db中清除，绑定或锁定物品内存释放
//-----------------------------------------------------------------------------
DWORD ItemMgr::LootFromBag(INT64 n64_serial, DWORD dw_cmd_id, OUT tagItem *&pOut)
{
	pOut = NULL;
	
	tagItem *pItem = GetBag().GetItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	// 装备有死亡守护特殊属性
	//if(MIsEquipment(pItem->dw_data_id) 
	//	&& EESA_Guard_DeadPunish == ((tagEquip*)pItem)->equipSpec.bySpecAtt)
	//{
	//	return E_Equip_Guard_DeadPunish;
	//}

	if(pItem->IsBind() || pItem->IsLock())
	{
		return RemoveItem(GetBag(), n64_serial, dw_cmd_id, TRUE, TRUE, FALSE);
	}

	pOut = pItem;
	return RemoveItem(GetBag(), n64_serial, dw_cmd_id, TRUE, FALSE, FALSE);
}

//-----------------------------------------------------------------------------
// 从背包中掉落 -- 清除可掉落物品并传出掉落物品信息
//-----------------------------------------------------------------------------
DWORD ItemMgr::LootFromBag(package_list<tagItem*>& listItems, DWORD dw_cmd_id)
{
	listItems.clear();
	//if( nLootNum <= 0 )
	//{
	//	return E_Success;
	//}

	// 读取所有可掉落物品
	package_list<tagItem*> listCanLoot;

	// 轮询背包
	tagItem* pItem = NULL;
	package_map<INT64, INT16>::map_iter iter = GetBag().Begin();

	while( GetBag().GetNextItem(iter, pItem) )
	{
 		// 不可掉落的物品
 		if( !pItem->pProtoType->bDeadLoot )
 		{
 			continue;
		}

		if( pItem->IsBind() || pItem->IsLock() )
		{
			continue;
		}

		if(get_tool()->tool_rand() % 100 > 3) 
			continue;

		if (pItem->n16Num > 1)
			continue;

// 		// 装备有死亡守护特殊属性
// 		if( MIsEquipment(pItem->dw_data_id) 
// 			&& EESA_Guard_DeadPunish == ((tagEquip*)pItem)->equip_spec.bySpecAtt)
// 		{
// 			continue;
// 		}

		// 加入列表
		listCanLoot.push_back(pItem);
	}

	// 没有可掉落物品
	if( listCanLoot.empty() )
	{
		return E_Success;
	}
	
	// 可掉落物品不足
	//if( listCanLoot.size() <= nLootNum )
	//{
		// 一个一个的拿出检测
		tagItem* pItem2 = listCanLoot.pop_front();

		while( VALID_POINT(pItem2) )
		{
			
			RemoveItem(GetBag(), pItem2->n64_serial, dw_cmd_id, TRUE, FALSE, FALSE);
			listItems.push_back(pItem2);
			
			pItem2 = listCanLoot.pop_front();
		}
	//}
	// 大于要掉落的物品个数，随机抽取
	//else
	//{
	//	// 最多取掉落的最大数量
	//	for(INT n = 0; n < nLootNum; ++n)
	//	{
	//		if( !listCanLoot.rand_find(pItem, TRUE) )
	//		{
	//			continue;
	//		}

	//		RemoveItem(GetBag(), pItem->n64_serial, dw_cmd_id, TRUE, FALSE, FALSE);
	//		listItems.push_back(pItem);
	//		
	//	}
	//}
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// 从装备栏中掉落 -- 从db中清除，绑定或锁定物品内存释放
//-----------------------------------------------------------------------------
DWORD ItemMgr::LootFromEquipBar(package_list<tagItem*>& listItems, package_list<DWORD>& listGemID, DWORD dw_cmd_id)
{
	tagEquip* pEquip = NULL;
	package_map<INT64, INT16>::map_iter iter = GetEquipBar().Begin();

	while( GetEquipBar().GetNextItem(iter, pEquip) )
	{
		//tagEquip* pEquip = (tagEquip*)pItem;
		if (pEquip->pEquipProto->eEquipPos == EEP_Fashion || 
			pEquip->pEquipProto->eEquipPos == EEP_Body1 ||
			pEquip->pEquipProto->eEquipPos == EEP_Ride)
		{
			continue;
		}

		int nPro = 99;
		if (m_pRole->GetPKValue() > 0)
		{
			nPro = 66;
		}

		if (get_tool()->tool_rand() % nPro != 1)
			continue;
		
		DWORD dwGemID = getGemIDbyLevel(pEquip, 8);
		if (dwGemID == 0)
		{
			RemoveFromEquipBar(pEquip->n64_serial, dw_cmd_id ,false);
			listItems.push_back(pEquip);
		}
		else
		{
			listGemID.push_back(dwGemID);
		}
		
	}
	
	// 检查是否需要清除内存中数据
	//if(pOut->IsBind() || pOut->IsLock())
	//{
	//	Destroy(pOut);
	//}

	return E_Success;
}
//-----------------------------------------------------------------------------
// 从装备栏中掉落 -- 从db中清除，绑定或锁定物品内存释放
//-----------------------------------------------------------------------------
DWORD ItemMgr::LootFromEquipBar( INT nLootNum,  DWORD dw_cmd_id )
{
	if( nLootNum <= 0 )	return E_Success;

	// 读取所有可掉落物品
	package_list<tagEquip*> listCanLoot;

	// 轮询背包
	tagEquip* pEquip = NULL;
	package_map<INT64, INT16>::map_iter iter = GetEquipBar().Begin();

	while( GetEquipBar( ).GetNextItem(iter, pEquip) )
	{
// 		// 不可掉落的物品
// 		if( !pEquip->pProtoType->bDeadLoot )continue;
// 
// 		// 装备有死亡守护特殊属性
// 		if( EESA_Guard_DeadPunish == pEquip->equip_spec.bySpecAtt) continue;

		//武器不掉落
		if( M_is_weapon(pEquip) ) continue;
		
		// 加入列表
		listCanLoot.push_back(pEquip);
	}

	// 可掉落物品不足
	if( listCanLoot.size() <= nLootNum )
	{
		// 一个一个的拿出检测
		tagEquip* pEquip = listCanLoot.pop_front();

		while( VALID_POINT(pEquip) )
		{
			RemoveFromEquipBar( pEquip->n64_serial, dw_cmd_id, TRUE) ;
			pEquip = listCanLoot.pop_front();
		}
	}
	// 大于要掉落的物品个数，随机抽取
	else
	{
		// 最多取掉落的最大数量
		for(INT n = 0; n < nLootNum; ++n)
		{
			if( !listCanLoot.rand_find(pEquip, TRUE) ) continue;
			RemoveFromEquipBar( pEquip->n64_serial, dw_cmd_id, TRUE) ;
		}
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 从指定列表中删除指定个数的物品
//-----------------------------------------------------------------------------
DWORD ItemMgr::DelBagSameItem(package_list<tagItem*> &list, INT32 n_num, DWORD dw_cmd_id)
 {
	ASSERT(list.size() > 0 && n_num > 0);

	INT32 nNumLeft	= n_num;
	INT32 n16NumDel	= 0;
	tagItem *pItem;
	package_list<tagItem*>::list_iter iter = list.begin();
	while(list.find_next(iter, pItem))
	{
		if(nNumLeft <= 0)
		{
			break;
		}

		n16NumDel = min(pItem->n16Num, nNumLeft);
		DelFromBag(pItem->n64_serial, dw_cmd_id, n16NumDel);
		nNumLeft -= n16NumDel;
	}

	return E_Success;
}

DWORD ItemMgr::ItemUsedSameItem(package_list<tagItem*> &list, INT32 n_num, DWORD dw_cmd_id)
{
	ASSERT(list.size() > 0 && n_num > 0);

	INT32 nNumLeft	= n_num;
	INT32 n16NumDel	= 0;
	tagItem *pItem;
	package_list<tagItem*>::list_iter iter = list.begin();
	while(list.find_next(iter, pItem))
	{
		if(nNumLeft <= 0)
		{
			break;
		}

		n16NumDel = min(pItem->n16Num, nNumLeft);
		ItemUsedFromBag(pItem->n64_serial, n16NumDel, dw_cmd_id);
		nNumLeft -= n16NumDel;
	}

	return E_Success;
}
//-----------------------------------------------------------------------------
// 百宝袋相关
//-----------------------------------------------------------------------------
DWORD ItemMgr::Add2BaiBao(tagItem *&pItem, DWORD dw_cmd_id, 
						  BOOL bReadFromDB/* = FALSE*/, DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{
	// 检查是否有获取时间
	if(0 == pItem->dw1stGainTime)
	{
		pItem->dw1stGainTime = g_world.GetWorldTime();
	}
	
	if(bReadFromDB || GetBaiBaoFreeSize() > 0)
	{
		// 存储到item表中
		return add_item(GetBaiBaoBag(), pItem, dw_cmd_id, TRUE, FALSE, dwRoleIDRel, FALSE);
	}
	else
	{
		// 存储到item_baibao表中
		ItemMgr::InsertBaiBao2DB(pItem, m_pRole->GetID(), dw_cmd_id);

		Destroy(pItem);
	}

	return E_Success;
}

VOID ItemMgr::InsertBaiBao2DB(tagItem *pItem, DWORD dw_role_id, DWORD dw_cmd_id)
{
	s_role_info* RoleInfo = g_roleMgr.get_role_info(dw_role_id);
	if(!VALID_POINT(RoleInfo))	return;

	ASSERT(pItem != NULL);

	pItem->dw_account_id = RoleInfo->dw_account_id;
	pItem->dwOwnerID = dw_role_id;
	// 临时设置物品属性以保存到item_baibao表
	pItem->n16Index = 0;
	pItem->eConType = EICT_Baibao;

	if(MIsEquipment(pItem->dw_data_id))
	{
		NET_DB2C_new_baibao_equip send;
		memcpy(&send.equip, pItem, SIZE_EQUIP);
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_new_baibao_item send;
		memcpy(&send.item, pItem, SIZE_ITEM);
		g_dbSession.Send(&send, send.dw_size);
	}

	// log处理//??
}

DWORD ItemMgr::ProcBaiBaoRecord(DWORD dw_data_id, DWORD dwDstRoleID, DWORD dwSrcRoleID, INT16 n16Type, DWORD dw_time, LPCTSTR szLeaveWords)
{
	// 上层需保证物品及角色的安全性,这里再做下简单判断
	s_role_info* RoleInfo = g_roleMgr.get_role_info(dwDstRoleID);

	if(!VALID_POINT(RoleInfo))
		return INVALID_VALUE;

	INT nLen = _tcsnlen(szLeaveWords, Max_LeaveWord_Length);
	INT nSize = sizeof(tagBaiBaoRecord) + (nLen * sizeof(TCHAR));

	//向DB发送保存请求
	CREATE_MSG(pSendDB, sizeof(NET_DB2C_add_baibao_log)+nSize-sizeof(tagBaiBaoRecord), NET_DB2C_add_baibao_log);
	pSendDB->dw_account_id = RoleInfo->dw_account_id;

	tagBaiBaoRecord* pRecord = &(pSendDB->log_info_);
	
	pRecord->n16Size = nSize;
	pRecord->dw_role_id = dwSrcRoleID;
	pRecord->dw_data_id = dw_data_id;
	pRecord->n16Type = n16Type;

	if (dw_time == INVALID_VALUE)
	{
		pRecord->dw_time = g_world.GetWorldTime();
	}
	else
	{
		pRecord->dw_time = dw_time;
	}

	_tcsncpy(pRecord->szWords, szLeaveWords, nLen);

	g_dbSession.Send(pSendDB, pSendDB->dw_size);
	

	// 判断目的玩家是否在线
	Role* p_role = g_roleMgr.get_role(dwDstRoleID);
	if (VALID_POINT(p_role))
	{
		// 向客户端发送记录同步消息
		CREATE_MSG(pSend, sizeof(NET_SIS_single_baobao_record)-sizeof(tagBaiBaoRecord)+nSize, NET_SIS_single_baobao_record);
		get_fast_code()->memory_copy(&(pSend->sRecord), pRecord, nSize);
		p_role->SendMessage(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}

	MDEL_MSG(pSendDB);	

	return E_Success;
}

//-----------------------------------------------------------------------------
// 物品可拥有上限相关
//-----------------------------------------------------------------------------
BOOL ItemMgr::IsMaxHoldLimitItem( DWORD dw_data_id )
{
	if (MIsEquipment(dw_data_id))
	{
		tagEquipProto* pTempEquipProto = AttRes::GetInstance()->GetEquipProto(dw_data_id);
		if (VALID_POINT(pTempEquipProto))
		{
			return pTempEquipProto->n16MaxHoldNum != INVALID_VALUE;
		}
	}
	else
	{
		tagItemProto* pTempItemProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
		if (VALID_POINT(pTempItemProto))
		{
			return pTempItemProto->n16MaxHoldNum != INVALID_VALUE;
		}
	}

	return FALSE;
}

DWORD ItemMgr::AddMaxHoldItem( DWORD dw_data_id, INT n_num )
{
	//if (!IsMaxHoldLimitItem(dw_data_id)) return E_Success;
	//上层保证该物品已通过验证
	m_mapMaxHold.modify_value(dw_data_id, n_num);

	return E_Success;
}

DWORD ItemMgr::AddMaxHoldItem( const tagItem& item )
{
	if (item.IsBind() && item.dwOwnerID != m_pRole->GetID()) return E_Success;

	m_mapMaxHold.modify_value(item.dw_data_id, item.n16Num);

	return E_Success;
}

VOID ItemMgr::RemoveMaxHoldItem( DWORD dw_data_id, INT n_num )
{
	if (!IsMaxHoldLimitItem(dw_data_id)) return;

	INT HoldNum = m_mapMaxHold.find(dw_data_id);

	if (HoldNum == (INT)INVALID_VALUE || HoldNum < n_num)
	{
		print_message(_T("\nDec Holdnum <item:%d, hold num: %d, dec num: %d> failed!\n\n"),
			dw_data_id, HoldNum, n_num);
		ASSERT(0);
		return;
	}

	m_mapMaxHold.modify_value(dw_data_id, -n_num);
}

BOOL ItemMgr::CanAddMaxHoldItem( DWORD dw_data_id, INT n_num )
{
	if (!IsMaxHoldLimitItem(dw_data_id)) return TRUE;

	INT HoldNum = m_mapMaxHold.find(dw_data_id);
	if ( HoldNum == (INT)INVALID_VALUE)
	{
		HoldNum = 0;
	}

	return n_num + HoldNum <= AttRes::GetInstance()->GetItemProto(dw_data_id)->n16MaxHoldNum;
}

BOOL ItemMgr::CanAddMaxHoldItem( const tagItem& item )
{
	if (!IsMaxHoldLimitItem(item.dw_data_id)) return TRUE;

	if (item.IsBind() && item.dwOwnerID != m_pRole->GetID()) return TRUE;

	INT HoldNum = m_mapMaxHold.find(item.dw_data_id);
	if ( HoldNum == (INT)INVALID_VALUE)
	{
		HoldNum = 0;
	}

	return item.n16Num + HoldNum <= item.pProtoType->n16MaxHoldNum;
}

//-----------------------------------------------------------------------------
// 武器崭新度处理
//-----------------------------------------------------------------------------
VOID ItemMgr::ProcEquipNewness()    
{
	tagEquip *pEquip = GetEquipBarEquip((INT16)EEP_RightHand);
	if(!VALID_POINT(pEquip))
	{
		return;
	}

	////有1/3的概率掉耐久
	//INT16 nRand = IUTIL->tool_rand() % 3;
	//if (nRand != 0)
	//	return;

	INT32 nCurNewness = (INT32)(pEquip->pEquipProto->n16Newness 
		- pEquip->nUseTimes / ABRASION2USETIMES);

	if(nCurNewness)
	{
		INT nAttackTimes = 0;
		//计算攻击次数
		pEquip->IncAttackTimes();
		//检查是否要更新崭新度
		if(pEquip->IsNewessChange(nAttackTimes))
		{
			NET_SIS_newess_change send;
			send.n64EquipSerial = pEquip->n64_serial;
			send.nAttackTimes = nAttackTimes;
			SendMessage(&send, send.dw_size);

			nCurNewness = pEquip->pEquipProto->n16Newness
				- pEquip->nUseTimes / ABRASION2USETIMES;
			if(!nCurNewness)
			{
				m_pRole->EquipEffect(pEquip, FALSE);
				//if(pEquip->IsBind())
				//{
				//	// 武器伤害影响
				//	//m_pRole->ResetWeaponDmg(*pEquip);
				//	m_pRole->EquipEffect(pEquip, FALSE);
				//}
				//else
				//{
				//	//m_pRole->ResetWeaponDmg(*pEquip);
				//	m_pRole->EquipEffect(pEquip, FALSE);
				//	RemoveFromEquipBar(pEquip->n64_serial, elcid_equip_newness_lost, TRUE);
				//}
				//m_pRole->SetRating();
				//m_pRole->SendTeamEquipInfo();
			}
		}
	}
}
// 防具崭新度处理
VOID ItemMgr::ProcArmorNewness()
{
	//EEquipPos ArmorPos[EEP_ARMOR_END];
	//memset(ArmorPos, EEP_Start, sizeof(ArmorPos));
	//INT16 nCount = 0;
	//for(INT16 i = EEP_Equip_Start; i <= EEP_ARMOR_END; i++)
	//{
	//	tagEquip *pEquip = GetEquipBarEquip(i);
	//	if(!VALID_POINT(pEquip))
	//	{
	//		continue;
	//	}
	//	ArmorPos[nCount] = (EEquipPos)i;
	//	nCount++;
	//}

	//if(nCount <= 0)
	//{
	//	return;
	//}

	//INT16 nRand = IUTIL->tool_rand() % (nCount);

	//从防具装备部位+2的数量中随机出一个数,随到对应的部位则减少耐久
	//INT16 nRand = IUTIL->tool_rand() % (X_ARMOR_NUM+2);
	//if (nRand > EEP_ARMOR_END || nRand < EEP_ARMOR_Start)
	//	return;

	INT16 nPos = 0;
	if (!GetLostNewnessPos(nPos)) return;

	tagEquip *pEquip = GetEquipBarEquip(nPos);
	if(!VALID_POINT(pEquip))
	{
		return;
	}

	INT32 nCurNewness = (INT32)(pEquip->pEquipProto->n16Newness 
		- pEquip->nUseTimes / ARMORABRASION2USETIMES);
	if(nCurNewness > 0)
	{
		INT nAttackTimes = 0;
		//计算攻击次数
		pEquip->IncAttackTimes();
		//检查是否要更新崭新度
		if(pEquip->IsArmorNewessChange(nAttackTimes))
		{
			NET_SIS_newess_change send;
			send.n64EquipSerial = pEquip->n64_serial;
			send.nAttackTimes = nAttackTimes;
			SendMessage(&send, send.dw_size);

			nCurNewness = pEquip->pEquipProto->n16Newness
				- pEquip->nUseTimes / ARMORABRASION2USETIMES;
			if(!nCurNewness)
			{
				m_pRole->EquipEffect(pEquip, FALSE);
				//// 计算防具影响
				//if(pEquip->IsBind())
				//{
				//	//m_pRole->ResetWeaponDmg(*pEquip);
				//	m_pRole->EquipEffect(pEquip, FALSE);
				//}
				//else
				//{
				//	//m_pRole->ResetWeaponDmg(*pEquip);
				//	m_pRole->EquipEffect(pEquip, FALSE);
				//	RemoveFromEquipBar(pEquip->n64_serial, elcid_equip_newness_lost, TRUE);
				//}
				//m_pRole->SetRating();
				//m_pRole->SendTeamEquipInfo();
			}

		}
	}

}

//-----------------------------------------------------------------------------
// 获取掉耐久的装备部位
//-----------------------------------------------------------------------------
BOOL ItemMgr::GetLostNewnessPos(INT16& pos)
{
	INT rand = get_tool()->tool_rand() % 10000;
	INT nPct = 0;
	for (int i = EEP_Head; i <= EEP_Equip_End; i++)
	{
		nPct += (10000/12);
		if (rand <= nPct)
		{
			pos = i;
			return true;
		}
	}
	
	return false;

	//if (rand < 88)
	//	pos = EEP_Head;
	//else if (rand < 184)
	//	pos = EEP_Body;
	////else if (rand < 264)
	////	pos = EEP_Back;
	//else if (rand < 328)
	//	pos = EEP_Wrist;
	//else if (rand < 400)
	//	pos = EEP_Legs;
	//else if (rand < 464)
	//	pos = EEP_Legs;
	//else if (rand < 528)
	//	pos = EEP_Feet;
	//else if (rand < 584)
	//	pos = EEP_Neck;
	//else if (rand < 640)
	//	pos = EEP_Neck;
	//else if (rand < 688)
	//	pos = EEP_Finger1;
	//else if (rand < 736)
	//	pos = EEP_Finger2;
	//else if (rand < 800)
	//	pos = EEP_Waist;
	//else
	//	return false;

	//return true;
}

VOID ItemMgr::InsertBaiBao2DBEx( tagItem *pItem, DWORD dwAccountId, DWORD dw_cmd_id )
{
	ASSERT(pItem != NULL);

	pItem->dw_account_id = dwAccountId;
	pItem->dwOwnerID = INVALID_VALUE;
	// 临时设置物品属性以保存到item_baibao表
	pItem->n16Index = 0;
	pItem->eConType = EICT_Baibao;

	if(MIsEquipment(pItem->dw_data_id))
	{
		NET_DB2C_new_baibao_equip send;
		memcpy(&send.equip, pItem, SIZE_EQUIP);
		g_dbSession.Send(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_new_baibao_item send;
		memcpy(&send.item, pItem, SIZE_ITEM);
		g_dbSession.Send(&send, send.dw_size);
	}
}
