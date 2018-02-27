/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ｻ��

#pragma once

#include "../../common/WorldDefine/exchange_define.h"
#pragma pack(push, 1)
//-----------------------------------------------------------------------------
// ��Ҽ佻�׷����������ݽṹ
//-----------------------------------------------------------------------------
struct tagPetExchangeData
{
	BOOL	bLock;			// ����Ƿ�ȷ�Ͻ���
	BOOL	bVerify;		// ����Ƿ��ٴ�ȷ�Ͻ���
	BYTE	byPetNum;		// ������Ʒ�������

	INT64	n64Money;		// ���׵Ľ�Ǯ
	DWORD	dwPetIDs[MAX_EXCHANGE_ITEM];	// ���׵���Ʒ��0��ʾû�н�����Ʒ
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
// ��Ҽ佻�׹�����
//-----------------------------------------------------------------------------
class PetExchangeMgr
{
public:
	PetExchangeMgr();
	~PetExchangeMgr();

public:
	VOID CreateData();
	VOID DeleteData();

	INT32	AddPet(DWORD dwPetID);	// ���� -- �ɹ�:����λ���±ꣻʧ��:INVALID_VALUE
	DWORD	DecPet(DWORD dwPetID);	// ���ش�����
	VOID	ResetMoney(INT64 n64Money);

	VOID	Lock();
	VOID	Unlock();
	VOID	Verify();

	VOID	SetTgtRoleID(const DWORD dwTgtRoleID);

public:
	DWORD	GetTgtRoleID() const;
	INT64	GetMoney() const;
	BYTE	GetPetNum() const;
	DWORD*	GetPetIDs() const;
	BOOL	IsLock() const;
	BOOL	IsVerify() const;

private:
	tagPetExchangeData*	m_pExData;
	DWORD				m_dwTgtRoleID;	// Ŀ����ң�INVALID_VALUE��ʾ�޽���Ŀ��
};



/*********************** ��Ҽ佻�׹���������������ʵ�� *******************************/

inline VOID PetExchangeMgr::DeleteData()	{ SAFE_DELETE(m_pExData); }

//-----------------------------------------------------------------------------
// ���ý���Ŀ��
//-----------------------------------------------------------------------------
inline VOID	PetExchangeMgr::SetTgtRoleID(const DWORD dwTgtRoleID)
{
	m_dwTgtRoleID = dwTgtRoleID;
}

//-----------------------------------------------------------------------------
// ��ȡ������������
//-----------------------------------------------------------------------------
inline DWORD PetExchangeMgr::GetTgtRoleID() const
{
	return m_dwTgtRoleID;
}

inline INT64 PetExchangeMgr::GetMoney() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->n64Money;
}

inline BYTE  PetExchangeMgr::GetPetNum() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->byPetNum;
}

inline DWORD* PetExchangeMgr::GetPetIDs() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->dwPetIDs;
}

inline BOOL PetExchangeMgr::IsLock() const
{ 
	ASSERT(m_pExData != NULL);
	return m_pExData->bLock;
}

inline BOOL PetExchangeMgr::IsVerify() const
{
	ASSERT(m_pExData != NULL);
	return m_pExData->bVerify;
}

//-----------------------------------------------------------------------------
// �޸Ľ�����������
//-----------------------------------------------------------------------------
inline VOID PetExchangeMgr::ResetMoney(INT64 n64Money)	
{ 
	ASSERT(m_pExData != NULL); 
	m_pExData->n64Money = n64Money; 
}


inline VOID PetExchangeMgr::Lock()
{ 
	ASSERT(m_pExData != NULL); 
	m_pExData->bLock = TRUE; 
}

inline VOID PetExchangeMgr::Unlock()
{ 
	ASSERT(m_pExData != NULL); 
	m_pExData->bLock = FALSE; 
}

inline VOID	PetExchangeMgr::Verify()
{
	ASSERT(m_pExData != NULL); 
	m_pExData->bVerify = TRUE; 
}