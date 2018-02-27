/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//VIP̯λ

#pragma once
#include "event_mgr.h"
#include "../../common/WorldDefine/stall_define.h"

struct tagVIPStall;
struct s_vip_stall_to_db;

class VIPStall : public EventMgr<VIPStall>
{
public:
	VIPStall();
	~VIPStall();

	BOOL	Init();
	VOID	Update();
	VOID	Destroy();

	// �¼�������
	VOID	OnSetStallTitleFunc(DWORD dwSenderID, VOID* pEventMessage);
	VOID	OnSetStallStatusFunc(DWORD dwSenderID, VOID* pEventMessage);
	VOID	OnChangeStallGoodsFunc(DWORD dwSenderID, VOID* pEventMessage);

	// ��ͼ�ϲ���Ϣ����
	DWORD	ApplyVIPStall(Role* pRole, BYTE byIndex, INT32 nRent);
	DWORD	GetAllVIPStallInfo(OUT tagVIPStall* pStallInfo, DWORD& dw_time);
	DWORD	GetUpdatedStallInfo(OUT tagVIPStall* pStallInfo, INT32& n_num, DWORD& dw_time);
	DWORD	GetVIPStallGoods(OUT LPVOID pData, OUT BYTE &byGoodsNum, OUT INT &nGoodsSz, DWORD& dwRequestTime, BYTE byIndex);

public:
	// ̯λ��Ϣ���
	tagVIPStall*	GetVIPStallInfo(BYTE index);
	tagVIPStall*	GetVIPStallInfo(DWORD dw_role_id);
	Role*			GetVIPStallRole(BYTE index);
	BOOL			IsInStatus(BYTE index, EVIPStallStatus eStatus);
	DWORD			GetSpecVIPStallGoodsTime(BYTE index);

	// ����̯λ״̬
	DWORD			SetVIPStallStatus(BYTE index, EVIPStallStatus eStatus);

	// ��ɫɾ��ʱ���������
	VOID			RemoveRoleVIPStall(DWORD dw_role_id);

	// ��ȡ����VIP̯λ��Ϣ
	DWORD	LoadAllVIPStallInfo(s_vip_stall_to_db* pVIPStallInfo);

private:
	static	VOID RegisterStallEventFunc();

private:
	vector<tagVIPStall>		m_vecVIPStall;		// VIP̯λ��Ϣ
	package_map<DWORD, BYTE>		m_mapVIPStall;		// ��ɫID��VIP̯λ��Ŷ��ձ�
	BOOL					m_bInit;

	DWORD					m_dwVIPStallChange;	// VIP̯λ���ʱ��
	tagDWORDTime			m_dwSecTimer;		// ���¼�ʱ
	INT						m_nMinuteTimer;		// ���Ӽ�ʱ
};

extern VIPStall g_VIPStall;