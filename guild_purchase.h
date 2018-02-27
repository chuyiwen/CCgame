/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/
//帮会团购

#pragma once

struct tagGroupPurchase;
struct tagSimGPInfo;
struct tagMallItemSell;
struct tagItem;
class guild;

class GuildPurchase
{
public:
	GuildPurchase();
	~GuildPurchase();

	BOOL Init(DWORD dwGuildID);
	VOID Update(DWORD dw_time);
	VOID Destory();

	// 信息管理
	BOOL	Add(tagGroupPurchase* pGPInfo, BOOL bNotify2DB = TRUE);
	BOOL	Remove(tagGroupPurchase* pGPInfo, BOOL bNotify2DB = TRUE);
	BOOL	IsEmpty()	{ return m_mapGPInfo.empty(); }

	// 获取团购信息
	DWORD	GetAllSimGPInfo(INT &nGPInfoNum, tagSimGPInfo* pData);
	DWORD	GetParticipators(DWORD dwID, DWORD dw_role_id, DWORD *pData);

	INT		GetGroupPurchaseInfoNum();
	INT		GetParticipatorNum(DWORD dwID, DWORD dw_role_id);

	// 发起/响应团购
	DWORD	LaunchGroupPurchase(Role *pRole, DWORD dwID, BYTE byScope,
		BYTE byIndex, INT nUnitPrice, OUT tagGroupPurchase* &pGPInfo, OUT DWORD& dwItemTypeID);
	DWORD	RespondGroupPurchase(Role *pRole, DWORD dwID, DWORD dw_role_id,
		INT nPrice, OUT tagGroupPurchase* &pGPInfo);

	// 生成团购物品
	DWORD	CreateGPItems(DWORD dwID, OUT tagMallItemSell &itemSell);

	// 删除团购信息
	VOID	RemoveGroupPurchaseInfo(tagGroupPurchase* &pGPInfo, BOOL bSuccess = TRUE);
	VOID	RemoveGroupPurchaseInfo();

	// 返还玩家元宝
	VOID	ReturnCost2Participator(tagGroupPurchase* pGPInfo);

	// 更新响应者列表
	VOID	UpdateGPInfo2DB(tagGroupPurchase* pGPInfo);

	// 团购成功处理
	DWORD	DoSuccessStuff(tagGroupPurchase* pGPInfo);

private:
	// 数据库操作
	VOID AddGPInfo2DB(tagGroupPurchase* pGPInfo);
	VOID RemoveGPInfo2DB(tagGroupPurchase* pGPInfo);
	VOID RemoveGuildGPInfo2DB();

private:
	// 生成键值
	INT64	GetKey(DWORD dw_role_id, DWORD dwID);
	
private:
	typedef package_map<INT64, tagGroupPurchase*> MapGPInfo;
	MapGPInfo				m_mapGPInfo;

	guild*					m_pGuild;
};