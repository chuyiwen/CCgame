/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#pragma once

#include <map>
#include <list>
#include <vector>
#include <set>
#include "mutex.h"
#include "../common/ServerDefine/famehall_server_define.h"

// ������������ʱ��
const INT32		REP_ORDER_UPDATE_INTERVAL_TICKS		= 10 * 60 * TICK_PER_SECOND;	

// ������������������
const INT32		ENTER_FAMEHALL_REP_LIM				= 328800;

// ���������ú󼤻��������
const INT8		ENTER_ACTCOUNT_REWARD				= 1;				


//----------------------------------------------------------------------------
// �����䱦ԭ��
//----------------------------------------------------------------------------
struct tagClanTreasureProto
{
	DWORD		dw_data_id;
	DWORD		dwNpcID;
	DWORD		dwMapID;
	ECLanType	eClanType;
	INT32		nActClanConNeed;

};

//----------------------------------------------------------------------------
// �����䱦
//----------------------------------------------------------------------------
struct tagClanTreasure
{
	e_reasure_state	eState;						// �䱦״̬
	union {
		const tagClanTreasureProto*		pProto;			// �����������幱��
		DWORD							dwNamePrefixID;	// �䱦����ǰ׺
	};
}; 
//----------------------------------------------------------------------------
// �����䱦�����б�
//----------------------------------------------------------------------------
class ClanTreasureActList
{
// 	typedef		std::list<tagClanTreasure>			ListClanTreasure;
// 	typedef		std::vector<DWORD>					VecTreasureID;
	typedef		std::map<UINT16, tagClanTreasure*>	MapU16ClanTreasure;
	typedef		std::pair<UINT16, tagClanTreasure*>	PairU16ClanTreasure;
public:// ��ͼ�̵߳���
	// �䱦���ݻ��
	VOID	GetActivatedTreasure(PVOID pData);

	// �䱦�������
	INT16	GetActivatedTreasureNum();


public:// ���̵߳���
	~ClanTreasureActList(){Destroy();}

	// ��ʼ����ȡ���������䱦�ṹ
	BOOL	Init(ECLanType eClantype);

	// ����
	VOID	Destroy();

	// ���Լ����䱦�����������̣߳�
	DWORD	TryActiveTreasure(Role* pRole, UINT16 u16TreasureID, ECLanType eclantype);

	// ��ȡ�䱦
	VOID	HandleLoadDBTreasure(tagTreasureData* pTreasureData, const INT32 n_num);
private:
	// �ɷ񼤻�
	DWORD	CanActiveTreasure(Role* pRole, UINT16 u16TreasureID, ECLanType eclantype);

	// ����
	DWORD	ActiveTreasure(Role* pRole, UINT16 u16TreasureID, ECLanType eclantype);

	// ������Ҽ������Ʒ
	VOID	GiveRoleItem( DWORD dwNameID, Role* pRole, DWORD dw_data_id );

	// ���ü�����ڳ�ʼ����
	VOID	SetAct(UINT16 u16TreasureID, DWORD dwNamePrefixID);
	
	// ��������
	VOID	ResetAll();
private:
	MapU16ClanTreasure		m_mapAllTreasure;
	INT						m_nActNum;
};


//----------------------------------------------------------------------------
// �����ý�ɫ����
//----------------------------------------------------------------------------
class FameRoleTracker
{
	typedef std::vector<tagFameHallEnterSnap>	VecFameMember;
	typedef std::set<DWORD>						SetMemberID;
public:
	// ��¼��ɫ
	VOID	TrackRole(DWORD dw_role_id, ECLanType eClanType);

	// ��ɫ�Ƿ񱻼�¼����
	BOOL	IsSnapAdded(DWORD dw_role_id, DWORD dwNameID);

	// ��ÿ���ǰn_num��
	INT32	GetTopSnap(BYTE* pData, INT32 n_num = 50);// dwNameID

	// ��ó�Ա����
	INT32	GetTopSnapNum(INT32 n_num = 50) const;

	// ��ʼ�����س�Ա�������
	VOID	LoadInitFameHallEnterSnap(tagFameHallEnterSnap* pFameMember, const INT32 n_num);

	// �Ƿ�������
	BOOL IsFameRole(DWORD dw_role_id, ECLanType eClanType);

private:
	// ��ӽ�ɫ����
	VOID	AddRoleSnap(DWORD dw_role_id, ECLanType eClanType);

	// ��������
	BOOL	IsSnapFull();

	// ��ӳ�Ա
	VOID MarkFameRole(DWORD dw_role_id, ECLanType eClanType);
private:
	VecFameMember	m_vecAllMembers;
	SetMemberID		m_setAllMemberIDs;
};


//----------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------
class ClanTrunk
{
	typedef		std::vector<tagRepRankData>	VecRepRank;

public:// ��ͼ�̵߳���
	// ���Խ���������
	BOOL TryEnterFameHall(Role* pRole);

	// �õ�����������ǰ50
	VOID GetFameHallTop50(BYTE* pData);

	// �õ������õ�ǰ����
	INT32 GetFameHallTop50Num();

	// �õ�������ո���ʱ��
	tagDWORDTime GetEnterSnapUpdateTime() const {	return m_dwtRepResetTime;	}

	// �õ�������������
	VOID GetRepRank(PVOID pData);

	// �õ�����������������
	INT32 GetRepRankNum();

	// �õ�������������ʱ��
	tagDWORDTime GetRepRankUpdateTime() const {	return m_dwtRepRankUpdateTime;	}

public:// ���̵߳���
	// ����
	VOID Update()
	{
		UpdateEnter();
		UpdateRepRank();
	}

	// ������������
	VOID HandleUpdateRepRank(tagRepRankData* pRepOrderData, const INT32 n_num);

	// ��ʼ������������
	VOID HandleInitFameHallTop50(tagFameHallEnterSnap* pMember, const INT32 n_num);

	// ��ʼ�������䱦
	VOID HandleInitActTreasureList(tagTreasureData* pTreasure, const INT32 n_num);

	// ��ʼ����������ʱ��
	VOID HandleInitRepRstTimeStamp(tagDWORDTime dwtRstTimeStamp) { m_dwtRepResetTime = dwtRstTimeStamp;}

	BOOL Init(ECLanType eClanType, INT32 nEnterLim, INT8 n8ActCount);
	VOID Destroy(){}

	// ���������䱦
	DWORD ActiveClanTreasure(Role* pRole, UINT16 u16TreasureID);

	// ��������䱦�б�
	VOID GetActTreasureList(PVOID pData);

	// ��������䱦����
	INT32 GetActTreasureNum();
	
	ClanTrunk()	:m_pRoleEnter( NULL ){}
	~ClanTrunk(){Destroy();}

	// tbc������nameid
	static DWORD GetNameID(DWORD dw_role_id);

private:// ���߳�
	// ���½���������
	BOOL UpdateEnter();

	// ������������
	VOID UpdateRepRank();

	// ����������
	BOOL Enter();

	// �ܷ����������
	BOOL CanEnter();
private:
	ClanTreasureActList		m_ClanTreasure;					// �䱦
	
	VecRepRank				m_vecRepRank;					// ��������				DBSession�г�ʼ��
	tagDWORDTime			m_dwtRepRankUpdateTime;			// �ϴθ���ʱ��
	INT32					m_nRepRankUpdateTickCounter;	// �����������µ�����ʱ	
	

	FameRoleTracker			m_FameRoleTracker;				// ����������				DBSession�г�ʼ��
	Role*					m_pRoleEnter;					// �����ɫ					���캯����ʼ��
	Mutex					m_EnterLock;					// ����������ͼ�̣߳�
	INT32					m_nEnterFameHallRepLim;			// ������������������		Init������ʼ��
	INT8					m_n8ActCountReward;				// ���������䱦����			Init������ʼ��
	tagDWORDTime			m_dwtRepResetTime;				// �ϴ���������ʱ��		DBSession�г�ʼ��

	ECLanType				m_eClanType;					// ��������					Init������ʼ��
};