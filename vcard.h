//��Name				:   vcard.h
//��Compiler			:	Microsoft Visual C++ 9.0
//��Version				:	1.00
//��Create Date			:	05/31/2009
//��LastModified		:	05/31/2009
//��Copyright (C)		:	
//��Writen  by			:   
//��Mode  by			:   
//��Brief				:	��ɫ���� STworld�ýṹ
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../common/WorldDefine/vcard_define.h"



class Role;
//-----------------------------------------------------------------------------
// �û��Զ�����Ϣ
//-----------------------------------------------------------------------------
struct tagRoleCustomVCard
{
	//BOOL	bVisibility;		// �ɼ���
	//BYTE	bySex;				// �Ա�Ĭ��Ϊ��ɫ�Ա���3���ѡ���С�Ů�����ܡ�
	BYTE	byConstellation;	// �����������˵�ѡ����13����ѡ���ֱ�Ϊ12�����ͱ��ܣ�Ĭ��Ϊ���ܡ�
	//BYTE	byChineseZodiac;	// ��Ф�������˵�ѡ����13����ѡ	
	//BYTE	byArea;				// ����
	//BYTE	byProvince;			// ʡ��

	TCHAR	tchCity[LEN_CITY_NAME];			// ����

	//tstring	strHeadUrl;		// ͷ��Ĭ��Ϊ��ɫ�Ա�ͷ��
	tstring	strSignature;		// ����ǩ�������100�����֡�200���ַ���

	tagRoleCustomVCard();
	~tagRoleCustomVCard();
	//-----------------------------------------------------------------------------
	// ��ʼ���Զ�������
	//-----------------------------------------------------------------------------
	void Init(IN const tagCustomVCardData* pVCardData, OUT const BYTE*& pData);

	//-----------------------------------------------------------------------------
	// ����ͷ��URL
	//-----------------------------------------------------------------------------
	void HeadUrl(const BYTE* szUrl, BYTE bySize);

	//-----------------------------------------------------------------------------
	// ����ǩ��
	//-----------------------------------------------------------------------------
	void Signature(const BYTE* szSignature, BYTE bySize);

	//-----------------------------------------------------------------------------
	// ȡͷ��
	//-----------------------------------------------------------------------------
	//const TCHAR* HeadUrl()	const {	return strHeadUrl.c_str();		}

	//-----------------------------------------------------------------------------
	// ȡǩ��
	//-----------------------------------------------------------------------------
	const TCHAR* Signature()const {	return strSignature.c_str();	}

	//-----------------------------------------------------------------------------
	// ���л� ΪtagCustomVCardData
	//-----------------------------------------------------------------------------
	void Serialize(IN tagCustomVCardData* pCustomVCard, OUT BYTE*& pData);

	DWORD GetCustomVCardDataSize() const;

};

//-----------------------------------------------------------------------------
// ��ɫ������Ϣ
//-----------------------------------------------------------------------------
struct tagRoleVCardBase
{
	DWORD	dw_role_id;			//��ɫID����ʾ��ҵĽ�ɫ���ơ�
	DWORD	dwLevel;			//��ɫ�ȼ�����ʾ��ҵĽ�ɫ�ȼ���
	DWORD	dwJob;				//��ɫְҵ����ʾ��ҵĽ�ɫְҵ��δתְ��ɫ��ʾΪ�����ˡ�
	DWORD	dwMateRoleID;		//��ɫ��ż����ʾ��ҵĽ�ɫ��ż��δ���ɫ��ʾΪ��
	DWORD	dwFactionID;		//�������ɣ���ʾ��ҵ������������ƣ��ް��������ʾΪ�ա�
	DWORD	dwPosition;			//����ְλ����ʾ��ҵİ���ְλ���ް��������ʾΪ�ա�

	tagRoleVCardBase(){m_pRole = NULL;}

	//-----------------------------------------------------------------------------
	// ���л�������Ϣ
	//-----------------------------------------------------------------------------
	void Serialize(tagVCardData* pVCardData);
	
protected:
	//-----------------------------------------------------------------------------
	// ��ʼ����ɫ������Ϣ
	//-----------------------------------------------------------------------------
	BOOL InitUpdate(Role *pRole, const tagVCardData* pVCardData)
	{
		BOOL bNeedSave = FALSE;
		Init(pVCardData);
		if (VALID_POINT(pRole))
		{
			ASSERT( VALID_POINT(pRole) );
			m_pRole = pRole;
			bNeedSave = Refresh();
		}
		return bNeedSave;
	}
	void Init(const tagVCardData* pVCardData )
	{
		dw_role_id	= pVCardData->dw_role_id;	
		dwLevel		= pVCardData->dwLevel;
		dwJob		= pVCardData->dwJob;
		dwMateRoleID= pVCardData->dwMateRoleID;	
		dwFactionID	= pVCardData->dwFactionID;
		dwPosition	= pVCardData->dwPosition;
	}
public:
	//-----------------------------------------------------------------------------
	// ��������������Ϣ	
	//-----------------------------------------------------------------------------
	BOOL Refresh();

protected:
	Role*	m_pRole;
};

//-----------------------------------------------------------------------------
// ��ɫ����
//-----------------------------------------------------------------------------
struct tagRoleVCard:public tagRoleVCardBase
{
	//-----------------------------------------------------------------------------
	// �û��Զ��忨Ƭ
	//-----------------------------------------------------------------------------
	tagRoleCustomVCard	customVCard;

	//-----------------------------------------------------------------------------
	// ��ʼ��
	//-----------------------------------------------------------------------------
//	void Init(const BYTE*& pData, Role *pRole);

	//-----------------------------------------------------------------------------
	// ���л� ΪtagVCardData
	//-----------------------------------------------------------------------------
	void Serialize(IN tagVCardData* pVCardData, OUT BYTE*& pData);

	//-----------------------------------------------------------------------------
	// �õ���Ӧ��tagVCardData�ߴ�
	//-----------------------------------------------------------------------------
	DWORD GetVCardDataSize() const;
	DWORD GetCustomDataSize() const;

	//-----------------------------------------------------------------------------
	// ������
	//-----------------------------------------------------------------------------
	static tagCustomVCardData* GetInitData();
	static VOID Test();

public:
	void Init(const BYTE*& pData, Role *pRole);
	void SetCustomData(tagCustomVCardData* pData);
	void SendVCard2Client(DWORD dwDstRoleID);
	void SendSaveDB();
	void SendLoadOffLineVCard(DWORD dwSrcRoleID, DWORD dwDstRoleID);
	//void SendHeadUrlTo(DWORD dwDstRoleID);
	void SendNullUrlToMe(DWORD dwWhosHead);

	VOID NotifyClientGetVCard(DWORD dw_role_id, DWORD dwErrCode);
	VOID NotifyClientSetVCard(DWORD dw_role_id, DWORD dwErrCode);

};