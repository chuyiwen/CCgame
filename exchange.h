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

#include "../../common/WorldDefine/exchange_define.h"
//-----------------------------------------------------------------------------
// ��Ҽ佻�׷����������ݽṹ
//-----------------------------------------------------------------------------
struct tagExchangeData
{
	BOOL	bLock;			// ����Ƿ�ȷ�Ͻ���
	BOOL	bVerify;		// ����Ƿ��ٴ�ȷ�Ͻ���
	BYTE	byTypeNum;		// ������Ʒ�������
	BOOL	bDummy;			// ������	
	INT64	n64Money;		// ���׵Ľ�Ǯ
	INT64	n64_serial[MAX_EXCHANGE_ITEM];	// ���׵���Ʒ��0��ʾû�н�����Ʒ
	INT16	n16ItemNum[MAX_EXCHANGE_ITEM];	// ��������Ʒ�ĸ���
};

//-----------------------------------------------------------------------------
// ��Ҽ佻�׹�����
//-----------------------------------------------------------------------------
class ExchangeMgr
{
public:
	ExchangeMgr();
	~ExchangeMgr();

public:
	VOID CreateData();
	VOID DeleteData();

	INT32	add_item(INT64 n64_serial, INT16 n16Num);	// ���� -- �ɹ�:����λ���±ꣻʧ��:INVALID_VALUE
	DWORD	DecItem(INT64 n64_serial);	// ���ش�����
	VOID	ResetMoney(INT64 n64Money);

	VOID	Lock();
	VOID	Unlock();
	VOID	Verify();

	VOID	SetTgtRoleID(const DWORD dwTgtRoleID);

public:
	DWORD	GetTgtRoleID() const;
	INT64	GetMoney() const;
	BYTE	GetItemTypeNum() const;
	INT64*	GetSerialArray() const;
	INT16*	GetNumArray() const;
	BOOL	IsLock() const;
	BOOL	IsVerify() const;

private:
	tagExchangeData*	m_pExData;
	DWORD				m_dwTgtRoleID;	// Ŀ����ң�INVALID_VALUE��ʾ�޽���Ŀ��
};



/*********************** ��Ҽ佻�׹���������������ʵ�� *******************************/

inline VOID ExchangeMgr::DeleteData()	{ SAFE_DELETE(m_pExData); }

//-----------------------------------------------------------------------------
// ���ý���Ŀ��
//-----------------------------------------------------------------------------
inline VOID	ExchangeMgr::SetTgtRoleID(const DWORD dwTgtRoleID)
{
	m_dwTgtRoleID = dwTgtRoleID;
}

//-----------------------------------------------------------------------------
// ��ȡ������������
//-----------------------------------------------------------------------------
inline DWORD ExchangeMgr::GetTgtRoleID() const
{
	return m_dwTgtRoleID;
}

inline INT64 ExchangeMgr::GetMoney() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->n64Money;
}

inline BYTE  ExchangeMgr::GetItemTypeNum() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->byTypeNum;
}

inline INT64* ExchangeMgr::GetSerialArray() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->n64_serial;
}

inline INT16* ExchangeMgr::GetNumArray() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->n16ItemNum;
}

inline BOOL ExchangeMgr::IsLock() const
{ 
	ASSERT(m_pExData != NULL);
	return m_pExData->bLock;
}

inline BOOL ExchangeMgr::IsVerify() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->bVerify;
}

//-----------------------------------------------------------------------------
// �޸Ľ�����������
//-----------------------------------------------------------------------------
inline VOID ExchangeMgr::ResetMoney(INT64 n64Money)	
{ 
	ASSERT(m_pExData != NULL); 
	m_pExData->n64Money = n64Money; 
}


inline VOID ExchangeMgr::Lock()
{ 
	ASSERT(m_pExData != NULL); 
	m_pExData->bLock = TRUE; 
}

inline VOID ExchangeMgr::Unlock()
{ 
	ASSERT(m_pExData != NULL); 
	m_pExData->bLock = FALSE; 
}

inline VOID	ExchangeMgr::Verify()
{
	ASSERT(m_pExData != NULL); 
	m_pExData->bVerify = TRUE; 
}