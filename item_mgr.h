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

#include "container.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../common/ServerDefine/base_server_define.h"
#include "world.h"

struct	tagRoleData;
//-----------------------------------------------------------------------------

class ItemMgr
{
	typedef package_map<DWORD, DWORD> MapCDTime;
	typedef package_map<DWORD, INT> MapMaxHold;

public:
	ItemMgr(Role* pRole, DWORD dwAcctID, DWORD dw_role_id, INT16 n16BagSize, INT16 n16WareSize);
	~ItemMgr();

public:
	VOID SaveItem2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID Update();
	
	// 物品是否处于冷却时间
	BOOL IsItemCDTime(DWORD dw_data_id);

	// 添加新类型物品的冷却时间
	VOID Add2CDTimeMap(DWORD dw_data_id, DWORD dwCDTime = INVALID_VALUE);
	VOID SaveCDTime2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID GetSameCDItemList(OUT package_map<DWORD, DWORD> &mapSameCD, IN DWORD dw_data_id);
	
	// 武器崭新度处理
	VOID ProcEquipNewness();
	// 防具崭新度处理
	VOID ProcArmorNewness();
	
	BOOL GetLostNewnessPos(INT16& pos);

	static BOOL CalSpaceUsed(DWORD dw_data_id, INT32 n_num, 
		OUT INT16 &n16UseBagSpace, OUT INT16 &n16UseQuestSpace, OUT INT16 &n16MaxLap);

	//-----------------------------------------------------------------------------
	// 数组中是否有重复ID
	//-----------------------------------------------------------------------------
	BOOL IsRepeatID(INT64 n64ItemID[], INT32 nArraySz);

	DWORD getGemIDbyLevel(tagEquip* pEquip, int nLevel);
public:
	BOOL CanExchange(const tagItem& item) const;
	BOOL CanSell(const tagItem& item) const;

public:
	DWORD IdentifyEquip(INT64 n64SerialReel, INT64 n64SerialEquip, DWORD dw_cmd_id);

public:
	//-----------------------------------------------------------------------------
	// 角色初始化物品&装备
	//-----------------------------------------------------------------------------
	VOID	SendInitStateItem();
	VOID	SendInitStateItemCDTime();

	DWORD	Put2Container(tagItem *pItem);

	VOID	UpdateEquipSpec(tagEquip &equip);

public:
	//-----------------------------------------------------------------------------
	// 从容器中获得相关信息
	//-----------------------------------------------------------------------------
	INT16	GetBagFreeSize();
	INT16	GetBagCurSize();
	INT16	GetBagOneFreeSpace();
	INT32	GetBagSameItemCount(DWORD dw_data_id);
	INT32	GetBagSameBindItemCount(DWORD dw_data_id, BOOL bBind);
	INT32	GetBagSameItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, IN INT32 n_num = INT_MAX);

	INT16	GetQuestItemBagFreeSize();
	INT32	GetQuestBagSameItemCount(DWORD dw_data_id);

	INT16	GetBaiBaoFreeSize();
	INT16	GetWareCurSize();

	BOOL	IsBagOneSpaceFree(INT16 n16Index);
	BOOL	GetBagRandom(INT64 &n64_serial);
	BOOL	GetQuestItemBagRandom(INT64 &n64_serial);

	tagItem*	GetBagItem(INT64 n64_serial);
	tagItem*	GetBagItem(INT16 n16Index);
	tagEquip*	GetEquipBarEquip(INT64 n64_serial);
	tagEquip*	GetEquipBarEquip(INT16 n16Index);
	tagItem*	GetDisplayItem(EItemConType eConType, INT64 n64_serial);

public:	
	//-----------------------------------------------------------------------------
	// 玩家换装
	//-----------------------------------------------------------------------------
	DWORD Equip(INT64 n64SerialSrc, EEquipPos ePosDst);
	DWORD Unequip(INT64 n64SerialSrc, INT16 n16IndexDst);
	//DWORD SwapWeapon();
	DWORD MoveRing(INT64 n64SerialSrc, INT16 n16PosDst);

public:
	//-----------------------------------------------------------------------------
	// 玩家获得&失去物品&装备 -- 普通物品放入背包，任务物品放入任务栏
	//-----------------------------------------------------------------------------
	BOOL Add2Role(EItemCreateMode eCreateMode, DWORD dwCreateID, 
				DWORD dw_data_id, INT32 n_num, EItemQuality eQlty, DWORD dw_cmd_id, BOOL bBind = FALSE);

	DWORD RemoveFromRole(DWORD dw_data_id, INT32 n_num, DWORD dw_cmd_id, INT nBind = INVALID_VALUE); // n_num == -1 时表示全部删除

	//-----------------------------------------------------------------------------
	// 删除任务相关物品 -- 需查背包，任务物品栏
	//-----------------------------------------------------------------------------
	VOID RemoveFromRole(UINT16 u16QuestID, DWORD dw_cmd_id);

	//-----------------------------------------------------------------------------
	// 玩家获得物品&装备
	//-----------------------------------------------------------------------------
	DWORD Add2Bag(tagItem *&pItem, DWORD dw_cmd_id, BOOL bInsert2DB = FALSE, BOOL bCheckAdd = TRUE);
	DWORD Add2BagByIndex(tagItem *&pItem, DWORD dw_cmd_id, INT16 n16Index);
	DWORD Add2BagByIndexAndInsertDB(tagItem *&pItem, DWORD dw_cmd_id, INT16 n16Index);
	DWORD Add2QuestBag(tagItem *&pItem, DWORD dw_cmd_id);
	DWORD Add2RoleWare(tagItem *&pItem, DWORD dw_cmd_id, BOOL bInsert2DB = FALSE, BOOL bCheckAdd = TRUE);

	//-----------------------------------------------------------------------------
	// 百宝袋相关
	//-----------------------------------------------------------------------------
	DWORD Add2BaiBao(tagItem *&pItem, DWORD dw_cmd_id, BOOL bReadFromDB = FALSE, DWORD dwRoleIDRel = INVALID_VALUE);
	static VOID InsertBaiBao2DB(tagItem *pItem, DWORD dw_role_id, DWORD dw_cmd_id);
	static VOID InsertBaiBao2DBEx(tagItem *pItem, DWORD dwAccountId, DWORD dw_cmd_id);

	static DWORD ProcBaiBaoRecord(DWORD dw_data_id, DWORD dwDstRoleID, DWORD dwSrcRoleID, 
		INT16 n16Type = EBBRT_System, DWORD dw_time = INVALID_VALUE, LPCTSTR szLeaveWords = _T(""));

public:
	//-----------------------------------------------------------------------------
	// 删除物品封装细化
	//-----------------------------------------------------------------------------
	// 从背包中取出，待放入其他背包 -- 内存没有释放
	DWORD TakeOutFromBag(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelFromDB);
	
	// 消耗物品个数 -- n16Num取默认值时表示全部删除，并从db中清除，且释放内存
	DWORD DelFromBag(INT64 n64_serial, DWORD dw_cmd_id, INT16 n16Num = INVALID_VALUE, BOOL bCheckDel = FALSE);

	// 从指定列表中删除指定个数的物品
	DWORD DelBagSameItem(package_list<tagItem*> &list, INT32 n_num, DWORD dw_cmd_id);
	
	DWORD ItemUsedSameItem(package_list<tagItem*> &list, INT32 n_num, DWORD dw_cmd_id);

	// 生产、使用等消耗个数或次数 -- 可堆叠物品消耗个数，不可堆叠物品消息次数
	DWORD ItemUsedFromBag(INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id);

	// 从背包中取出，并丢弃 -- 从db中清除，绑定物品内存释放，锁定物品不可丢弃
	DWORD DiscardFromBag(INT64 n64_serial, DWORD dw_cmd_id, OUT tagItem *&pOut, BYTE by_type);

	// 从背包中掉落 -- 从db中清除，绑定或锁定物品内存释放
	DWORD LootFromBag(INT64 n64_serial, DWORD dw_cmd_id, OUT tagItem *&pOut);

	// 从背包中掉落 -- 清除可掉落物品并传出掉落物品信息
	DWORD LootFromBag(package_list<tagItem*>& listItems, DWORD dw_cmd_id);

	// 从装备栏中掉落 -- 从db中清除，绑定或锁定物品内存释放
	DWORD LootFromEquipBar(package_list<tagItem*>& listItems, package_list<DWORD>& listGemID, DWORD dw_cmd_id);

	// 从装备栏中掉落 -- 从db中清除，绑定或锁定物品内存释放<武器不掉落>
	DWORD LootFromEquipBar( INT nLootNum, DWORD dw_cmd_id );

public:
	//-----------------------------------------------------------------------------
	// 背包中删除或添加多个物品(玩家间交易,邮件,任务)
	//-----------------------------------------------------------------------------
	VOID Add2Bag(tagItem* pItem[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);
	VOID RemoveFromBag(INT64 n64_serial[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);
	BOOL CheckExistInBag(OUT tagItem* pItem[], INT64 n64_serial[], INT16 n16Num[], INT32 nSize);

	//-----------------------------------------------------------------------------
	// 移动物品
	//-----------------------------------------------------------------------------
	DWORD Move(EItemConType eConType, INT64 n64_serial, INT16 n16Num, INT16 n16PosDst, DWORD dw_cmd_id);
	DWORD move_to_other(EItemConType eConTypeSrc, INT64 n64Serial1, 
					EItemConType eConTypeDst, INT16 n16PosDst, DWORD dw_cmd_id);
	
	VOID Stack(EItemConType eConType);
	//-----------------------------------------------------------------------------
	// 从装备栏上直接删除一件装备
	//-----------------------------------------------------------------------------
	tagItem* RemoveFromEquipBar(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelMem = FALSE);

public:
	//-----------------------------------------------------------------------------
	// 背包&角色仓库扩容
	//-----------------------------------------------------------------------------
	DWORD ExtendBag(INT64 n64ItemID, Role* pRole, INT32 n32_type);
	//DWORD ExtendRoleWare(INT64 n64ItemID);

	//-----------------------------------------------------------------------------
	// 背包&角色仓库整理
	//-----------------------------------------------------------------------------
	DWORD ReorderBag(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	DWORD ReorderRoleWare(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);

	DWORD ReorderBagEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);
	DWORD ReorderRoleWareEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);

	DWORD ReorderQuest(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	DWORD ReorderRoleQuestEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);

public:
	//-----------------------------------------------------------------------------
	// 可拥有数量限制物品管理
	//-----------------------------------------------------------------------------
	BOOL IsMaxHoldLimitItem( DWORD dw_data_id );
	BOOL CanAddMaxHoldItem( DWORD dw_data_id, INT n_num );
	BOOL CanAddMaxHoldItem( const tagItem& item );
	DWORD AddMaxHoldItem( DWORD dw_data_id, INT n_num );
	DWORD AddMaxHoldItem( const tagItem& item );
	VOID RemoveMaxHoldItem( DWORD dw_data_id, INT n_num );

private:
	template<class T> VOID Save2DB(IN Container<T> &con, OUT LPVOID pData, 
								OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	template<class T> VOID GetAllItem(IN Container<T> &con, const INT16 n16ReadNum, 
								OUT LPVOID pData, OUT INT32 &nSize);

	VOID FormatCDTime(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID UpdateCDTime();

	VOID UpdateContainer(ItemContainer& sItemCon);
	VOID UpdateContainer(EquipContainer& sEquipCon);

public:
	ItemContainer*	GetContainer(EItemConType eConType);

	ItemContainer&	GetBag()			{ return m_Bag; }
	ItemContainer&	GetQuestItemBag()	{ return m_QuestItemBag; }
	ItemContainer&	GetBaiBaoBag()		{ return m_BaiBaoBag; }
	ItemContainer&	GetRoleWare()		{ return m_RoleWare; }
	EquipContainer& GetEquipBar()		{ return m_EquipBar; }

private:
	//-----------------------------------------------------------------------------
	// 物品 -- bDelFromDB		是否从游戏数据库中删除,
	//		   bCheckRemove		是否判断该物品可从容器中删除
	//		   bDelMem			是否销毁内存
	//		   bIncUseTimes		是否消耗的是使用次数
	//-----------------------------------------------------------------------------
	DWORD add_item(ItemContainer& container, tagItem *&pItem, DWORD dw_cmd_id, 
					BOOL bInsert2DB = FALSE, BOOL bCheckAdd = TRUE, DWORD dwRoleIDRel = INVALID_VALUE, BOOL bChangeOwner = TRUE);

	DWORD RemoveItem(ItemContainer& container, INT64 n64_serial, DWORD dw_cmd_id, 
					BOOL bDelFromDB = FALSE, BOOL bDelMem = FALSE, 
					BOOL bCheckRemove = TRUE, DWORD dwRoleIDRel = INVALID_VALUE);

	DWORD RemoveItem(ItemContainer& container, INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id, 
					BOOL bCheckRemove = TRUE, BOOL bDelete = FALSE);

	//-----------------------------------------------------------------------------
	// 从游戏中删除指定类别物品,且不做物品是否可从容器中删除的检查
	//-----------------------------------------------------------------------------
	DWORD RemoveItems(ItemContainer& container, DWORD dw_data_id, DWORD dw_cmd_id, INT nBind = INVALID_VALUE);
	DWORD RemoveItems(ItemContainer& container, DWORD dw_data_id, INT32 n_num, DWORD dw_cmd_id, INT nBind = INVALID_VALUE);

	//-----------------------------------------------------------------------------
	// 从游戏中删除和指定任务相关的物品
	//-----------------------------------------------------------------------------
	VOID RemoveItems(ItemContainer& container, UINT16 u16QuestID, DWORD dw_cmd_id);
	VOID RemoveItems(EquipContainer& container, UINT16 u16QuestID, DWORD dw_cmd_id);
	
	//-----------------------------------------------------------------------------
	// 背包中删除或添加多个物品(玩家间交易,邮件,任务)
	//-----------------------------------------------------------------------------
	VOID AddItems(ItemContainer& container, tagItem* pItem[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);
	VOID RemoveItems(ItemContainer& container, INT64 n64_serial[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);

	BOOL CheckItemsExist(OUT tagItem* pItem[], ItemContainer& container, 
						INT64 n64_serial[], INT16 n16Num[], INT32 nSize);

	BOOL IsQuestItem(DWORD dw_data_id);

private:
	//-----------------------------------------------------------------------------
	// 向客户端发送消息
	//-----------------------------------------------------------------------------
	VOID SendMessage(LPVOID p_message, DWORD dw_size);
	
	//-----------------------------------------------------------------------------
	// 向容器中添加&删除物品，发送到客户端消息封装
	//-----------------------------------------------------------------------------
	VOID SendAddItem2Client(EItemConType eConType, INT16 n16Index, INT64 n64_serial, INT16 n16Num, BOOL bOverlap,EItemCreateMode eCreateMode/*gx add*/);
	VOID SendDelItem2Client(EItemConType eConType, INT16 n16Index, INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id);
	VOID SendAddNew2Client(const tagItem *pItem, BOOL bPickUp);

	VOID insert_item_to_db(tagItem &item);
	VOID delete_item_from_db(INT64 n64_serial, INT32 dw_data_id);

	//-----------------------------------------------------------------------------
	// 装备属性改变后，即时保存数据库及向客户端发送消息
	//-----------------------------------------------------------------------------
	VOID SendEquipSpec2DB(const tagEquip &equip);
	VOID SendEquipSpec2Client(const tagEquip &equip);

private:
	__forceinline VOID log_item(const tagItem &item1, const tagItem *pItem2, 
							  INT16 n16OptNum, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE);
	__forceinline VOID LogItemTimes(const tagItem &item, DWORD dw_cmd_id);

	BOOL is_item_need_log(const tagItem &item) const { return item.pProtoType->bNeedLog && item.pProtoType->byLogMinQlty <= ItemHelper::GetQuality(&item); }


private:
	Role*				m_pRole;

	ItemContainer		m_Bag;				// 背包
	ItemContainer		m_QuestItemBag;		// 任务物品包
	ItemContainer		m_BaiBaoBag;		// 从数据库读入物品, 注意Add时的调用接口
	ItemContainer		m_RoleWare;			// 放到角色仓库中的物品,不改变所属(注意Add时的调用接口)
	EquipContainer		m_EquipBar;			// 装备栏

	MapCDTime			m_mapCDTime;		// 物品&装备冷却时间<dw_data_id, dwRemainTime>
	MapMaxHold			m_mapMaxHold;		// 限制可拥有数量物品管理
};


//-------------------------------------------------------------------------------------------------------
// 从容器中获取所有的物品数据,个数及大小
//-------------------------------------------------------------------------------------------------------
template<class T> 
VOID ItemMgr::GetAllItem(IN Container<T> &con, const INT16 n16ReadNum, OUT LPVOID pData, OUT INT32 &nSize)
{
	nSize	= 0;

	INT16	n16Num	= 0;
	T		*pTemp	= NULL;
	BYTE	*byData = (BYTE*)pData;

	for(INT16 i=0; i<con.GetCurSpaceSize(); ++i)
	{
		pTemp = con.GetItem(i);
		if(VALID_POINT(pTemp))
		{
			if(MIsEquipment(pTemp->dw_data_id))	// 装备
			{
				get_fast_code()->memory_copy(byData, pTemp, SIZE_EQUIP);
				//((tagEquip*)byData)->equipSpec.n16QltyModPctEx = 0;	// 对客户端隐藏二级修正率
				byData += SIZE_EQUIP;
			}
			else	// 物品
			{
				get_fast_code()->memory_copy(byData, pTemp, SIZE_ITEM);
				byData += SIZE_ITEM;
			}

			++n16Num;
			if(n16ReadNum == n16Num)
			{
				break;
			}
		}
	}

	nSize = byData - (BYTE*)pData;
}

//-------------------------------------------------------------------------------------------------------
// 和DBServer同步物品装备信息(只同步位置及使用相关信息)
//-------------------------------------------------------------------------------------------------------
template<class T> 
VOID ItemMgr::Save2DB(IN Container<T> &con, OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	n_num = 0;

	M_trans_pointer(pCurPointer, pData, s_item_update);

	T	*pTemp	= NULL;
	for(INT16 i=0; i<con.GetCurSpaceSize(); ++i)
	{
		pTemp = con.GetItem(i);
		if(VALID_POINT(pTemp) && pTemp->eStatus != EUDBS_Null)
		{
			pCurPointer[n_num].n64_serial		= pTemp->n64_serial;
			pCurPointer[n_num].dw_owner_id		= pTemp->dwOwnerID;
			pCurPointer[n_num].dw_account_id	= pTemp->dw_account_id;
			pCurPointer[n_num].n_use_times		= pTemp->nUseTimes;
			pCurPointer[n_num].n16_num		= pTemp->n16Num;
			pCurPointer[n_num].n16_index		= pTemp->n16Index;
			pCurPointer[n_num].by_conType		= pTemp->eConType;
			pCurPointer[n_num].by_bind		= pTemp->byBind;
			pCurPointer[n_num].dw_bind_time	= pTemp->dwBindTime;
			memcpy(pCurPointer[n_num].dw_script_data, pTemp->dw_script_data, sizeof(DWORD)*2);
			pTemp->SetUpdate(EUDBS_Null);

			++n_num;
		}
	}

	pOutPointer = &pCurPointer[n_num];
}


//****************************** 内联函数实现 **********************************


//-----------------------------------------------------------------------------
// 格式化保存数据库数据
//-----------------------------------------------------------------------------
inline VOID ItemMgr::SaveCDTime2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	FormatCDTime(pData, pOutPointer, n_num);
}

//-----------------------------------------------------------------------------
// 是否可以出售
//-----------------------------------------------------------------------------
inline BOOL ItemMgr::CanSell(const tagItem& item) const
{
	return (item.pProtoType->bCanSell && !item.IsLock() && !MIsQuestItem(item.pProtoType));
}

//-----------------------------------------------------------------------------
// 更新装备动态信息后，相关消息封装
//-----------------------------------------------------------------------------
inline VOID ItemMgr::UpdateEquipSpec(tagEquip &equip)
{
	//if (equip.pEquipProto->eEquipPos == EEP_RightHand)
	//{
		equip.equipSpec.nRating = equip.GetRating();//算下评分
	//}
	SendEquipSpec2DB(equip);
	SendEquipSpec2Client(equip);
}

//-----------------------------------------------------------------------------
// 背包空闲格子数
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBagFreeSize()
{
	return GetBag().GetFreeSpaceSize();
}

//-----------------------------------------------------------------------------
// 背包当前总格子数
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBagCurSize()
{
	return GetBag().GetCurSpaceSize();
}

//-----------------------------------------------------------------------------
// 获取背包中一个空闲格子下标
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBagOneFreeSpace()
{
	return GetBag().GetOneFreeSpace();
}

//-----------------------------------------------------------------------------
// 获取背包中相同物品的总个数
//-----------------------------------------------------------------------------
inline INT32 ItemMgr::GetBagSameItemCount(DWORD dw_data_id)
{
	return GetBag().GetSameItemCount(dw_data_id);
}


inline INT32 ItemMgr::GetBagSameBindItemCount(DWORD dw_data_id, BOOL bBind)
{
	return GetBag().GetSameBindItemCount(dw_data_id, bBind);
}
//----------------------------------------------------------------------
// 获得容器中物品类型为dw_data_id的lis, 返回实际获得个数t -- 指定n_num时，仅找到n_num个物品即可
//----------------------------------------------------------------------
inline INT32 ItemMgr::GetBagSameItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, IN INT32 n_num)
{
	return GetBag().GetSameItemList(list, dw_data_id, n_num);
}

//-----------------------------------------------------------------------------
// 获取任务栏相同物品的总个数
//-----------------------------------------------------------------------------
inline INT32 ItemMgr::GetQuestBagSameItemCount(DWORD dw_data_id)
{
	return GetQuestItemBag().GetSameItemCount(dw_data_id);
}

//-----------------------------------------------------------------------------
// 获取物品
//-----------------------------------------------------------------------------
inline tagItem *ItemMgr::GetBagItem(INT64 n64_serial)
{
	tagItem* pItem = GetBag().GetItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		pItem = GetQuestItemBag().GetItem(n64_serial);
	}
	return pItem;
	//return GetBag().GetItem(n64_serial);
}

//-----------------------------------------------------------------------------
// 获取物品
//-----------------------------------------------------------------------------
inline tagItem *ItemMgr::GetBagItem(INT16 n16Index)
{
	tagItem* pItem = GetBag().GetItem(n16Index);
	if(!VALID_POINT(pItem))
	{
		pItem = GetQuestItemBag().GetItem(n16Index);
	}
	return pItem;
	//return GetBag().GetItem(n16Index);
}

//-----------------------------------------------------------------------------
// 背包中指定位置格子是否空闲
//-----------------------------------------------------------------------------
inline BOOL	ItemMgr::IsBagOneSpaceFree(INT16 n16Index)
{
	return GetBag().IsOnePlaceFree(n16Index);
}

//-----------------------------------------------------------------------------
// 获得装备栏上物品
//-----------------------------------------------------------------------------
inline tagEquip* ItemMgr::GetEquipBarEquip(INT64 n64_serial)
{
	return GetEquipBar().GetItem(n64_serial);
}

//-----------------------------------------------------------------------------
// 获得装备栏上物品
//-----------------------------------------------------------------------------
inline tagEquip* ItemMgr::GetEquipBarEquip(INT16 n16Index)
{
	return GetEquipBar().GetItem(n16Index);
}

//-----------------------------------------------------------------------------
// 获取百宝袋空闲格子数
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBaiBaoFreeSize()
{
	return GetBaiBaoBag().GetFreeSpaceSize();
}

//-----------------------------------------------------------------------------
// 角色仓库当前总格子数
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetWareCurSize()
{
	return GetRoleWare().GetCurSpaceSize();
}

//-----------------------------------------------------------------------------
// 得到任务栏可用空间
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetQuestItemBagFreeSize()
{
	return GetQuestItemBag().GetFreeSpaceSize();
}

//-----------------------------------------------------------------------------
// 从背包中随机取个一个物品的64位id
//-----------------------------------------------------------------------------
inline BOOL	ItemMgr::GetBagRandom(INT64 &n64_serial)
{
	return GetBag().GetRandom(n64_serial);
}

//-----------------------------------------------------------------------------
// 从任务栏中随机取个一个物品的64位id
//-----------------------------------------------------------------------------
inline BOOL	ItemMgr::GetQuestItemBagRandom(INT64 &n64_serial)
{
	return GetQuestItemBag().GetRandom(n64_serial);
}

//-----------------------------------------------------------------------------
// 获取展示物品数据
//-----------------------------------------------------------------------------
inline tagItem* ItemMgr::GetDisplayItem(EItemConType eConType, INT64 n64_serial) 
{ 
	//是否装备栏
	if(eConType == EICT_Equip)
	{
		//是装备栏
		return GetEquipBar().GetItem((INT16)n64_serial);
	}
	else
	{
		ItemContainer* pContainer = GetContainer(eConType); 
		if (!VALID_POINT(pContainer))
			return NULL;
		return pContainer->GetItem(n64_serial);
	}
}

//-----------------------------------------------------------------------------
// 穿装备
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::Equip(INT64 n64SerialSrc, EEquipPos ePosDst)
{
	return GetEquipBar().Equip(GetBag(), n64SerialSrc, ePosDst);
}

//-----------------------------------------------------------------------------
// 脱装备
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::Unequip(INT64 n64SerialSrc, INT16 n16IndexDst)
{
	return GetEquipBar().Unequip(n64SerialSrc, GetBag(), n16IndexDst);
}

//-----------------------------------------------------------------------------
// 主副手物品对换
//-----------------------------------------------------------------------------
//inline DWORD ItemMgr::SwapWeapon()
//{
//	return GetEquipBar().SwapWeapon();
//}

//-----------------------------------------------------------------------------
// 两个戒指互换
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::MoveRing(INT64 n64SerialSrc, INT16 n16PosDst)
{
	return GetEquipBar().MoveTo(n64SerialSrc, (EEquipPos)n16PosDst);
}

//-----------------------------------------------------------------------------
// 删除任务相关物品 -- 需查背包，任务物品栏
//-----------------------------------------------------------------------------
inline VOID ItemMgr::RemoveFromRole(UINT16 u16QuestID, DWORD dw_cmd_id)
{
	RemoveItems(GetBag(), u16QuestID, dw_cmd_id);
	RemoveItems(GetQuestItemBag(), u16QuestID, dw_cmd_id);
	RemoveItems(GetEquipBar(), u16QuestID, dw_cmd_id);
}

//-----------------------------------------------------------------------------
// 玩家获得物品&装备
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::Add2Bag(tagItem *&pItem, DWORD dw_cmd_id, BOOL bInsert2DB, BOOL bCheckAdd)
{
	if(MIsQuestItem(pItem->pProtoType))
	{
		return add_item(GetQuestItemBag(), pItem, dw_cmd_id, bInsert2DB, bCheckAdd);
	}
	else
	{
		return add_item(GetBag(), pItem, dw_cmd_id, bInsert2DB, bCheckAdd);
	}
	return FALSE;
}

inline DWORD ItemMgr::Add2QuestBag(tagItem *&pItem, DWORD dw_cmd_id)
{
	return add_item(GetQuestItemBag(), pItem, dw_cmd_id, TRUE, TRUE);
}

inline DWORD ItemMgr::Add2RoleWare(tagItem *&pItem, DWORD dw_cmd_id, BOOL bInsert2DB, BOOL bCheckAdd)
{
	return add_item(GetRoleWare(), pItem, dw_cmd_id, bInsert2DB, bCheckAdd);
}

//-----------------------------------------------------------------------------
// 从背包中取出，待放入其他背包 -- 内存没有释放
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::TakeOutFromBag(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelFromDB)
{
	return RemoveItem(GetBag(), n64_serial, dw_cmd_id, bDelFromDB, FALSE, TRUE);
}

//-----------------------------------------------------------------------------
// 消耗物品个数 -- n16Num取默认值时表示全部删除，并从db中清除，且释放内存
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::DelFromBag(INT64 n64_serial, DWORD dw_cmd_id, INT16 n16Num, BOOL bCheckDel)
{
	if(INVALID_VALUE == n16Num)
	{
		return RemoveItem(GetBag(), n64_serial, dw_cmd_id, TRUE, TRUE, bCheckDel);
	}

	return RemoveItem(GetBag(), n64_serial, n16Num, dw_cmd_id, bCheckDel, TRUE);
}

//-----------------------------------------------------------------------------
// 消耗物品(使用次数或个数) -- 剩余使用次数为0时，从db中删除，并释放内存
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::ItemUsedFromBag(INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id)
{
	if(E_Success != RemoveItem(GetBag(), n64_serial, n16Num, dw_cmd_id, TRUE, FALSE))
	{
		return RemoveItem(GetQuestItemBag(), n64_serial, n16Num, dw_cmd_id, TRUE, FALSE);
	}
	return E_Success;
	//return RemoveItem(GetBag(), n64_serial, n16Num, dw_cmd_id, TRUE, FALSE);
}

//-----------------------------------------------------------------------------
// 背包中删除或添加多个物品(玩家间交易,邮件,任务)
//-----------------------------------------------------------------------------
inline VOID ItemMgr::Add2Bag(tagItem* pItem[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel)
{
	AddItems(GetBag(), pItem, nSize, dw_cmd_id, dwRoleIDRel);
}

inline VOID ItemMgr::RemoveFromBag(INT64 n64_serial[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel)
{
	RemoveItems(GetBag(), n64_serial, nSize, dw_cmd_id, dwRoleIDRel);
}

inline BOOL ItemMgr::CheckExistInBag(OUT tagItem* pItem[], INT64 n64_serial[], INT16 n16Num[], INT32 nSize)
{
	return CheckItemsExist(pItem, GetBag(), n64_serial, n16Num, nSize);
}

//-----------------------------------------------------------------------------
// 背包整理
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::ReorderBag(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	return GetBag().Reorder(pData, pOutData, n16Num) ? E_Success : INVALID_VALUE;
}

inline DWORD ItemMgr::ReorderBagEx(IN LPVOID pData, OUT LPVOID pOutData, 
							OUT INT16 &n16OutNum, const INT16 n16Num)
{
	return GetBag().ReorderEx(pData, pOutData, n16OutNum, n16Num) ? E_Success : INVALID_VALUE;
}

//-----------------------------------------------------------------------------
// 角色仓库整理
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::ReorderRoleWare(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	return GetRoleWare().Reorder(pData, pOutData, n16Num) ? E_Success : INVALID_VALUE;
}

inline DWORD ItemMgr::ReorderRoleWareEx(IN LPVOID pData, OUT LPVOID pOutData, 
							   OUT INT16 &n16OutNum, const INT16 n16Num)
{
	return GetRoleWare().ReorderEx(pData, pOutData, n16OutNum, n16Num) ? E_Success : INVALID_VALUE;
}

//-----------------------------------------------------------------------------
// 角色任务物品整理
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::ReorderQuest(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	return GetQuestItemBag().Reorder(pData, pOutData, n16Num) ? E_Success : INVALID_VALUE;
}

inline DWORD ItemMgr::ReorderRoleQuestEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num)
{
	return GetQuestItemBag().ReorderEx(pData, pOutData, n16OutNum, n16Num) ? E_Success : INVALID_VALUE;
}
