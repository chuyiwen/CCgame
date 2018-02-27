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

	// ��Ϣ����
	BOOL	Add(tagGroupPurchase* pGPInfo, BOOL bNotify2DB = TRUE);
	BOOL	Remove(tagGroupPurchase* pGPInfo, BOOL bNotify2DB = TRUE);
	BOOL	IsEmpty()	{ return m_mapGPInfo.empty(); }

	// ��ȡ�Ź���Ϣ
	DWORD	GetAllSimGPInfo(INT &nGPInfoNum, tagSimGPInfo* pData);
	DWORD	GetParticipators(DWORD dwID, DWORD dw_role_id, DWORD *pData);

	INT		GetGroupPurchaseInfoNum();
	INT		GetParticipatorNum(DWORD dwID, DWORD dw_role_id);

	// ����/��Ӧ�Ź�
	DWORD	LaunchGroupPurchase(Role *pRole, DWORD dwID, BYTE byScope,
		BYTE byIndex, INT nUnitPrice, OUT tagGroupPurchase* &pGPInfo, OUT DWORD& dwItemTypeID);
	DWORD	RespondGroupPurchase(Role *pRole, DWORD dwID, DWORD dw_role_id,
		INT nPrice, OUT tagGroupPurchase* &pGPInfo);

	// �����Ź���Ʒ
	DWORD	CreateGPItems(DWORD dwID, OUT tagMallItemSell &itemSell);

	// ɾ���Ź���Ϣ
	VOID	RemoveGroupPurchaseInfo(tagGroupPurchase* &pGPInfo, BOOL bSuccess = TRUE);
	VOID	RemoveGroupPurchaseInfo();

	// �������Ԫ��
	VOID	ReturnCost2Participator(tagGroupPurchase* pGPInfo);

	// ������Ӧ���б�
	VOID	UpdateGPInfo2DB(tagGroupPurchase* pGPInfo);

	// �Ź��ɹ�����
	DWORD	DoSuccessStuff(tagGroupPurchase* pGPInfo);

private:
	// ���ݿ����
	VOID AddGPInfo2DB(tagGroupPurchase* pGPInfo);
	VOID RemoveGPInfo2DB(tagGroupPurchase* pGPInfo);
	VOID RemoveGuildGPInfo2DB();

private:
	// ���ɼ�ֵ
	INT64	GetKey(DWORD dw_role_id, DWORD dwID);
	
private:
	typedef package_map<INT64, tagGroupPurchase*> MapGPInfo;
	MapGPInfo				m_mapGPInfo;

	guild*					m_pGuild;
};