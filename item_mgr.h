/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ʒ��װ��������

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
	
	// ��Ʒ�Ƿ�����ȴʱ��
	BOOL IsItemCDTime(DWORD dw_data_id);

	// �����������Ʒ����ȴʱ��
	VOID Add2CDTimeMap(DWORD dw_data_id, DWORD dwCDTime = INVALID_VALUE);
	VOID SaveCDTime2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID GetSameCDItemList(OUT package_map<DWORD, DWORD> &mapSameCD, IN DWORD dw_data_id);
	
	// ����ո�¶ȴ���
	VOID ProcEquipNewness();
	// ����ո�¶ȴ���
	VOID ProcArmorNewness();
	
	BOOL GetLostNewnessPos(INT16& pos);

	static BOOL CalSpaceUsed(DWORD dw_data_id, INT32 n_num, 
		OUT INT16 &n16UseBagSpace, OUT INT16 &n16UseQuestSpace, OUT INT16 &n16MaxLap);

	//-----------------------------------------------------------------------------
	// �������Ƿ����ظ�ID
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
	// ��ɫ��ʼ����Ʒ&װ��
	//-----------------------------------------------------------------------------
	VOID	SendInitStateItem();
	VOID	SendInitStateItemCDTime();

	DWORD	Put2Container(tagItem *pItem);

	VOID	UpdateEquipSpec(tagEquip &equip);

public:
	//-----------------------------------------------------------------------------
	// �������л�������Ϣ
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
	// ��һ�װ
	//-----------------------------------------------------------------------------
	DWORD Equip(INT64 n64SerialSrc, EEquipPos ePosDst);
	DWORD Unequip(INT64 n64SerialSrc, INT16 n16IndexDst);
	//DWORD SwapWeapon();
	DWORD MoveRing(INT64 n64SerialSrc, INT16 n16PosDst);

public:
	//-----------------------------------------------------------------------------
	// ��һ��&ʧȥ��Ʒ&װ�� -- ��ͨ��Ʒ���뱳����������Ʒ����������
	//-----------------------------------------------------------------------------
	BOOL Add2Role(EItemCreateMode eCreateMode, DWORD dwCreateID, 
				DWORD dw_data_id, INT32 n_num, EItemQuality eQlty, DWORD dw_cmd_id, BOOL bBind = FALSE);

	DWORD RemoveFromRole(DWORD dw_data_id, INT32 n_num, DWORD dw_cmd_id, INT nBind = INVALID_VALUE); // n_num == -1 ʱ��ʾȫ��ɾ��

	//-----------------------------------------------------------------------------
	// ɾ�����������Ʒ -- ��鱳����������Ʒ��
	//-----------------------------------------------------------------------------
	VOID RemoveFromRole(UINT16 u16QuestID, DWORD dw_cmd_id);

	//-----------------------------------------------------------------------------
	// ��һ����Ʒ&װ��
	//-----------------------------------------------------------------------------
	DWORD Add2Bag(tagItem *&pItem, DWORD dw_cmd_id, BOOL bInsert2DB = FALSE, BOOL bCheckAdd = TRUE);
	DWORD Add2BagByIndex(tagItem *&pItem, DWORD dw_cmd_id, INT16 n16Index);
	DWORD Add2BagByIndexAndInsertDB(tagItem *&pItem, DWORD dw_cmd_id, INT16 n16Index);
	DWORD Add2QuestBag(tagItem *&pItem, DWORD dw_cmd_id);
	DWORD Add2RoleWare(tagItem *&pItem, DWORD dw_cmd_id, BOOL bInsert2DB = FALSE, BOOL bCheckAdd = TRUE);

	//-----------------------------------------------------------------------------
	// �ٱ������
	//-----------------------------------------------------------------------------
	DWORD Add2BaiBao(tagItem *&pItem, DWORD dw_cmd_id, BOOL bReadFromDB = FALSE, DWORD dwRoleIDRel = INVALID_VALUE);
	static VOID InsertBaiBao2DB(tagItem *pItem, DWORD dw_role_id, DWORD dw_cmd_id);
	static VOID InsertBaiBao2DBEx(tagItem *pItem, DWORD dwAccountId, DWORD dw_cmd_id);

	static DWORD ProcBaiBaoRecord(DWORD dw_data_id, DWORD dwDstRoleID, DWORD dwSrcRoleID, 
		INT16 n16Type = EBBRT_System, DWORD dw_time = INVALID_VALUE, LPCTSTR szLeaveWords = _T(""));

public:
	//-----------------------------------------------------------------------------
	// ɾ����Ʒ��װϸ��
	//-----------------------------------------------------------------------------
	// �ӱ�����ȡ������������������ -- �ڴ�û���ͷ�
	DWORD TakeOutFromBag(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelFromDB);
	
	// ������Ʒ���� -- n16NumȡĬ��ֵʱ��ʾȫ��ɾ��������db����������ͷ��ڴ�
	DWORD DelFromBag(INT64 n64_serial, DWORD dw_cmd_id, INT16 n16Num = INVALID_VALUE, BOOL bCheckDel = FALSE);

	// ��ָ���б���ɾ��ָ����������Ʒ
	DWORD DelBagSameItem(package_list<tagItem*> &list, INT32 n_num, DWORD dw_cmd_id);
	
	DWORD ItemUsedSameItem(package_list<tagItem*> &list, INT32 n_num, DWORD dw_cmd_id);

	// ������ʹ�õ����ĸ�������� -- �ɶѵ���Ʒ���ĸ��������ɶѵ���Ʒ��Ϣ����
	DWORD ItemUsedFromBag(INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id);

	// �ӱ�����ȡ���������� -- ��db�����������Ʒ�ڴ��ͷţ�������Ʒ���ɶ���
	DWORD DiscardFromBag(INT64 n64_serial, DWORD dw_cmd_id, OUT tagItem *&pOut, BYTE by_type);

	// �ӱ����е��� -- ��db��������󶨻�������Ʒ�ڴ��ͷ�
	DWORD LootFromBag(INT64 n64_serial, DWORD dw_cmd_id, OUT tagItem *&pOut);

	// �ӱ����е��� -- ����ɵ�����Ʒ������������Ʒ��Ϣ
	DWORD LootFromBag(package_list<tagItem*>& listItems, DWORD dw_cmd_id);

	// ��װ�����е��� -- ��db��������󶨻�������Ʒ�ڴ��ͷ�
	DWORD LootFromEquipBar(package_list<tagItem*>& listItems, package_list<DWORD>& listGemID, DWORD dw_cmd_id);

	// ��װ�����е��� -- ��db��������󶨻�������Ʒ�ڴ��ͷ�<����������>
	DWORD LootFromEquipBar( INT nLootNum, DWORD dw_cmd_id );

public:
	//-----------------------------------------------------------------------------
	// ������ɾ������Ӷ����Ʒ(��Ҽ佻��,�ʼ�,����)
	//-----------------------------------------------------------------------------
	VOID Add2Bag(tagItem* pItem[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);
	VOID RemoveFromBag(INT64 n64_serial[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);
	BOOL CheckExistInBag(OUT tagItem* pItem[], INT64 n64_serial[], INT16 n16Num[], INT32 nSize);

	//-----------------------------------------------------------------------------
	// �ƶ���Ʒ
	//-----------------------------------------------------------------------------
	DWORD Move(EItemConType eConType, INT64 n64_serial, INT16 n16Num, INT16 n16PosDst, DWORD dw_cmd_id);
	DWORD move_to_other(EItemConType eConTypeSrc, INT64 n64Serial1, 
					EItemConType eConTypeDst, INT16 n16PosDst, DWORD dw_cmd_id);
	
	VOID Stack(EItemConType eConType);
	//-----------------------------------------------------------------------------
	// ��װ������ֱ��ɾ��һ��װ��
	//-----------------------------------------------------------------------------
	tagItem* RemoveFromEquipBar(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelMem = FALSE);

public:
	//-----------------------------------------------------------------------------
	// ����&��ɫ�ֿ�����
	//-----------------------------------------------------------------------------
	DWORD ExtendBag(INT64 n64ItemID, Role* pRole, INT32 n32_type);
	//DWORD ExtendRoleWare(INT64 n64ItemID);

	//-----------------------------------------------------------------------------
	// ����&��ɫ�ֿ�����
	//-----------------------------------------------------------------------------
	DWORD ReorderBag(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	DWORD ReorderRoleWare(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);

	DWORD ReorderBagEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);
	DWORD ReorderRoleWareEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);

	DWORD ReorderQuest(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	DWORD ReorderRoleQuestEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);

public:
	//-----------------------------------------------------------------------------
	// ��ӵ������������Ʒ����
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
	// ��Ʒ -- bDelFromDB		�Ƿ����Ϸ���ݿ���ɾ��,
	//		   bCheckRemove		�Ƿ��жϸ���Ʒ�ɴ�������ɾ��
	//		   bDelMem			�Ƿ������ڴ�
	//		   bIncUseTimes		�Ƿ����ĵ���ʹ�ô���
	//-----------------------------------------------------------------------------
	DWORD add_item(ItemContainer& container, tagItem *&pItem, DWORD dw_cmd_id, 
					BOOL bInsert2DB = FALSE, BOOL bCheckAdd = TRUE, DWORD dwRoleIDRel = INVALID_VALUE, BOOL bChangeOwner = TRUE);

	DWORD RemoveItem(ItemContainer& container, INT64 n64_serial, DWORD dw_cmd_id, 
					BOOL bDelFromDB = FALSE, BOOL bDelMem = FALSE, 
					BOOL bCheckRemove = TRUE, DWORD dwRoleIDRel = INVALID_VALUE);

	DWORD RemoveItem(ItemContainer& container, INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id, 
					BOOL bCheckRemove = TRUE, BOOL bDelete = FALSE);

	//-----------------------------------------------------------------------------
	// ����Ϸ��ɾ��ָ�������Ʒ,�Ҳ�����Ʒ�Ƿ�ɴ�������ɾ���ļ��
	//-----------------------------------------------------------------------------
	DWORD RemoveItems(ItemContainer& container, DWORD dw_data_id, DWORD dw_cmd_id, INT nBind = INVALID_VALUE);
	DWORD RemoveItems(ItemContainer& container, DWORD dw_data_id, INT32 n_num, DWORD dw_cmd_id, INT nBind = INVALID_VALUE);

	//-----------------------------------------------------------------------------
	// ����Ϸ��ɾ����ָ��������ص���Ʒ
	//-----------------------------------------------------------------------------
	VOID RemoveItems(ItemContainer& container, UINT16 u16QuestID, DWORD dw_cmd_id);
	VOID RemoveItems(EquipContainer& container, UINT16 u16QuestID, DWORD dw_cmd_id);
	
	//-----------------------------------------------------------------------------
	// ������ɾ������Ӷ����Ʒ(��Ҽ佻��,�ʼ�,����)
	//-----------------------------------------------------------------------------
	VOID AddItems(ItemContainer& container, tagItem* pItem[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);
	VOID RemoveItems(ItemContainer& container, INT64 n64_serial[], INT32 nSize, DWORD dw_cmd_id, DWORD dwRoleIDRel);

	BOOL CheckItemsExist(OUT tagItem* pItem[], ItemContainer& container, 
						INT64 n64_serial[], INT16 n16Num[], INT32 nSize);

	BOOL IsQuestItem(DWORD dw_data_id);

private:
	//-----------------------------------------------------------------------------
	// ��ͻ��˷�����Ϣ
	//-----------------------------------------------------------------------------
	VOID SendMessage(LPVOID p_message, DWORD dw_size);
	
	//-----------------------------------------------------------------------------
	// �����������&ɾ����Ʒ�����͵��ͻ�����Ϣ��װ
	//-----------------------------------------------------------------------------
	VOID SendAddItem2Client(EItemConType eConType, INT16 n16Index, INT64 n64_serial, INT16 n16Num, BOOL bOverlap,EItemCreateMode eCreateMode/*gx add*/);
	VOID SendDelItem2Client(EItemConType eConType, INT16 n16Index, INT64 n64_serial, INT16 n16Num, DWORD dw_cmd_id);
	VOID SendAddNew2Client(const tagItem *pItem, BOOL bPickUp);

	VOID insert_item_to_db(tagItem &item);
	VOID delete_item_from_db(INT64 n64_serial, INT32 dw_data_id);

	//-----------------------------------------------------------------------------
	// װ�����Ըı�󣬼�ʱ�������ݿ⼰��ͻ��˷�����Ϣ
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

	ItemContainer		m_Bag;				// ����
	ItemContainer		m_QuestItemBag;		// ������Ʒ��
	ItemContainer		m_BaiBaoBag;		// �����ݿ������Ʒ, ע��Addʱ�ĵ��ýӿ�
	ItemContainer		m_RoleWare;			// �ŵ���ɫ�ֿ��е���Ʒ,���ı�����(ע��Addʱ�ĵ��ýӿ�)
	EquipContainer		m_EquipBar;			// װ����

	MapCDTime			m_mapCDTime;		// ��Ʒ&װ����ȴʱ��<dw_data_id, dwRemainTime>
	MapMaxHold			m_mapMaxHold;		// ���ƿ�ӵ��������Ʒ����
};


//-------------------------------------------------------------------------------------------------------
// �������л�ȡ���е���Ʒ����,��������С
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
			if(MIsEquipment(pTemp->dw_data_id))	// װ��
			{
				get_fast_code()->memory_copy(byData, pTemp, SIZE_EQUIP);
				//((tagEquip*)byData)->equipSpec.n16QltyModPctEx = 0;	// �Կͻ������ض���������
				byData += SIZE_EQUIP;
			}
			else	// ��Ʒ
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
// ��DBServerͬ����Ʒװ����Ϣ(ֻͬ��λ�ü�ʹ�������Ϣ)
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


//****************************** ��������ʵ�� **********************************


//-----------------------------------------------------------------------------
// ��ʽ���������ݿ�����
//-----------------------------------------------------------------------------
inline VOID ItemMgr::SaveCDTime2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	FormatCDTime(pData, pOutPointer, n_num);
}

//-----------------------------------------------------------------------------
// �Ƿ���Գ���
//-----------------------------------------------------------------------------
inline BOOL ItemMgr::CanSell(const tagItem& item) const
{
	return (item.pProtoType->bCanSell && !item.IsLock() && !MIsQuestItem(item.pProtoType));
}

//-----------------------------------------------------------------------------
// ����װ����̬��Ϣ�������Ϣ��װ
//-----------------------------------------------------------------------------
inline VOID ItemMgr::UpdateEquipSpec(tagEquip &equip)
{
	//if (equip.pEquipProto->eEquipPos == EEP_RightHand)
	//{
		equip.equipSpec.nRating = equip.GetRating();//��������
	//}
	SendEquipSpec2DB(equip);
	SendEquipSpec2Client(equip);
}

//-----------------------------------------------------------------------------
// �������и�����
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBagFreeSize()
{
	return GetBag().GetFreeSpaceSize();
}

//-----------------------------------------------------------------------------
// ������ǰ�ܸ�����
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBagCurSize()
{
	return GetBag().GetCurSpaceSize();
}

//-----------------------------------------------------------------------------
// ��ȡ������һ�����и����±�
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBagOneFreeSpace()
{
	return GetBag().GetOneFreeSpace();
}

//-----------------------------------------------------------------------------
// ��ȡ��������ͬ��Ʒ���ܸ���
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
// �����������Ʒ����Ϊdw_data_id��lis, ����ʵ�ʻ�ø���t -- ָ��n_numʱ�����ҵ�n_num����Ʒ����
//----------------------------------------------------------------------
inline INT32 ItemMgr::GetBagSameItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, IN INT32 n_num)
{
	return GetBag().GetSameItemList(list, dw_data_id, n_num);
}

//-----------------------------------------------------------------------------
// ��ȡ��������ͬ��Ʒ���ܸ���
//-----------------------------------------------------------------------------
inline INT32 ItemMgr::GetQuestBagSameItemCount(DWORD dw_data_id)
{
	return GetQuestItemBag().GetSameItemCount(dw_data_id);
}

//-----------------------------------------------------------------------------
// ��ȡ��Ʒ
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
// ��ȡ��Ʒ
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
// ������ָ��λ�ø����Ƿ����
//-----------------------------------------------------------------------------
inline BOOL	ItemMgr::IsBagOneSpaceFree(INT16 n16Index)
{
	return GetBag().IsOnePlaceFree(n16Index);
}

//-----------------------------------------------------------------------------
// ���װ��������Ʒ
//-----------------------------------------------------------------------------
inline tagEquip* ItemMgr::GetEquipBarEquip(INT64 n64_serial)
{
	return GetEquipBar().GetItem(n64_serial);
}

//-----------------------------------------------------------------------------
// ���װ��������Ʒ
//-----------------------------------------------------------------------------
inline tagEquip* ItemMgr::GetEquipBarEquip(INT16 n16Index)
{
	return GetEquipBar().GetItem(n16Index);
}

//-----------------------------------------------------------------------------
// ��ȡ�ٱ������и�����
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetBaiBaoFreeSize()
{
	return GetBaiBaoBag().GetFreeSpaceSize();
}

//-----------------------------------------------------------------------------
// ��ɫ�ֿ⵱ǰ�ܸ�����
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetWareCurSize()
{
	return GetRoleWare().GetCurSpaceSize();
}

//-----------------------------------------------------------------------------
// �õ����������ÿռ�
//-----------------------------------------------------------------------------
inline INT16 ItemMgr::GetQuestItemBagFreeSize()
{
	return GetQuestItemBag().GetFreeSpaceSize();
}

//-----------------------------------------------------------------------------
// �ӱ��������ȡ��һ����Ʒ��64λid
//-----------------------------------------------------------------------------
inline BOOL	ItemMgr::GetBagRandom(INT64 &n64_serial)
{
	return GetBag().GetRandom(n64_serial);
}

//-----------------------------------------------------------------------------
// �������������ȡ��һ����Ʒ��64λid
//-----------------------------------------------------------------------------
inline BOOL	ItemMgr::GetQuestItemBagRandom(INT64 &n64_serial)
{
	return GetQuestItemBag().GetRandom(n64_serial);
}

//-----------------------------------------------------------------------------
// ��ȡչʾ��Ʒ����
//-----------------------------------------------------------------------------
inline tagItem* ItemMgr::GetDisplayItem(EItemConType eConType, INT64 n64_serial) 
{ 
	//�Ƿ�װ����
	if(eConType == EICT_Equip)
	{
		//��װ����
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
// ��װ��
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::Equip(INT64 n64SerialSrc, EEquipPos ePosDst)
{
	return GetEquipBar().Equip(GetBag(), n64SerialSrc, ePosDst);
}

//-----------------------------------------------------------------------------
// ��װ��
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::Unequip(INT64 n64SerialSrc, INT16 n16IndexDst)
{
	return GetEquipBar().Unequip(n64SerialSrc, GetBag(), n16IndexDst);
}

//-----------------------------------------------------------------------------
// ��������Ʒ�Ի�
//-----------------------------------------------------------------------------
//inline DWORD ItemMgr::SwapWeapon()
//{
//	return GetEquipBar().SwapWeapon();
//}

//-----------------------------------------------------------------------------
// ������ָ����
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::MoveRing(INT64 n64SerialSrc, INT16 n16PosDst)
{
	return GetEquipBar().MoveTo(n64SerialSrc, (EEquipPos)n16PosDst);
}

//-----------------------------------------------------------------------------
// ɾ�����������Ʒ -- ��鱳����������Ʒ��
//-----------------------------------------------------------------------------
inline VOID ItemMgr::RemoveFromRole(UINT16 u16QuestID, DWORD dw_cmd_id)
{
	RemoveItems(GetBag(), u16QuestID, dw_cmd_id);
	RemoveItems(GetQuestItemBag(), u16QuestID, dw_cmd_id);
	RemoveItems(GetEquipBar(), u16QuestID, dw_cmd_id);
}

//-----------------------------------------------------------------------------
// ��һ����Ʒ&װ��
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
// �ӱ�����ȡ������������������ -- �ڴ�û���ͷ�
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::TakeOutFromBag(INT64 n64_serial, DWORD dw_cmd_id, BOOL bDelFromDB)
{
	return RemoveItem(GetBag(), n64_serial, dw_cmd_id, bDelFromDB, FALSE, TRUE);
}

//-----------------------------------------------------------------------------
// ������Ʒ���� -- n16NumȡĬ��ֵʱ��ʾȫ��ɾ��������db����������ͷ��ڴ�
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
// ������Ʒ(ʹ�ô��������) -- ʣ��ʹ�ô���Ϊ0ʱ����db��ɾ�������ͷ��ڴ�
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
// ������ɾ������Ӷ����Ʒ(��Ҽ佻��,�ʼ�,����)
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
// ��������
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
// ��ɫ�ֿ�����
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
// ��ɫ������Ʒ����
//-----------------------------------------------------------------------------
inline DWORD ItemMgr::ReorderQuest(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	return GetQuestItemBag().Reorder(pData, pOutData, n16Num) ? E_Success : INVALID_VALUE;
}

inline DWORD ItemMgr::ReorderRoleQuestEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num)
{
	return GetQuestItemBag().ReorderEx(pData, pOutData, n16OutNum, n16Num) ? E_Success : INVALID_VALUE;
}
