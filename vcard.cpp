//★Name				:   vcard.cpp
//★Compiler			:	Microsoft Visual C++ 9.0
//★Version				:	1.00
//★Create Date			:	05/31/2009
//★LastModified		:	05/31/2009
//★Copyright (C)		:	
//★Writen  by			:   
//★Mode  by			:   
//★Brief				:	角色名贴 STworld用结构
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "vcard.h"
#include "role.h"
#include "role_mgr.h"
#include "../common/ServerDefine/vcard_server_define.h"

//-----------------------------------------------------------------------------
// 更新名帖基本信息	
//-----------------------------------------------------------------------------
#define CHGANDSAVE(a1, a2)	if((a1) != (a2)){a1 = (a2);bSave = TRUE;	}
BOOL tagRoleVCardBase::Refresh()
{
	if (!VALID_POINT(m_pRole))
	{
		return FALSE;
	}

	ASSERT( dw_role_id == m_pRole->GetID());
	
	BOOL bSave = FALSE;
	CHGANDSAVE(dwLevel, m_pRole->get_level());

	// 	dwJob			= pRole->GetJob();
	// 	dwMateRoleID	= pRole->GetMate();
	// 	dwFactionID		= pRole->GetFaction();
	// 	dwPosition		= pRole->GetPosition();

	return bSave;
}
#undef CHGANDSAVE

tagRoleCustomVCard::tagRoleCustomVCard()
{

}
tagRoleCustomVCard::~tagRoleCustomVCard()
{
}

//-----------------------------------------------------------------------------
// 初始化自定义数据
//-----------------------------------------------------------------------------
void tagRoleCustomVCard::Init(IN const tagCustomVCardData* pVCardData, OUT const BYTE* &pData)
{
 	ASSERT( VALID_POINT(pVCardData) );

	DWORD dw_size = sizeof(tagCustomVCardData) - 1 + /*pVCardData->byHeadUrlSize +*/ pVCardData->bySignatureSize;

	//bVisibility		= pVCardData->bVisibility;
	//bySex			= pVCardData->bySex;
	byConstellation	= pVCardData->byConstellation;
	//byChineseZodiac = pVCardData->byChineseZodiac;
	//byProvince		= pVCardData->byProvince;
	//byArea			= pVCardData->byArea;

	get_fast_code()->memory_copy(tchCity, pVCardData->chCity, sizeof(TCHAR) * LEN_CITY_NAME);


	//HeadUrl(pVCardData->byData, pVCardData->byHeadUrlSize);
	Signature(pVCardData->byData, pVCardData->bySignatureSize);

	pData = reinterpret_cast<const BYTE *>(pVCardData) + dw_size;
}

//-----------------------------------------------------------------------------
// 设置头像URL
//-----------------------------------------------------------------------------
//void tagRoleCustomVCard::HeadUrl(const BYTE* szUrl, BYTE bySize)
//{
//	// 客户端不可直接更改
//	if (bySize <= 2)
//	{
//		return;
//	}
//	strHeadUrl = (TCHAR*)szUrl;
//}


//-----------------------------------------------------------------------------
// 设置签名
//-----------------------------------------------------------------------------
void tagRoleCustomVCard::Signature(const BYTE* szSignature, BYTE bySize)
{
	strSignature = (TCHAR*)szSignature;
}


//-----------------------------------------------------------------------------
// 序列化 为tagCustomVCardData
//-----------------------------------------------------------------------------
void tagRoleCustomVCard::Serialize(IN tagCustomVCardData* pCustomVCard, OUT BYTE*& pData)
{
	DWORD dw_size = sizeof(tagCustomVCardData) - 1 + (/*strHeadUrl.size() + */strSignature.size() + 2) * sizeof(TCHAR);

	//pCustomVCard->bVisibility		= bVisibility;
	//pCustomVCard->byChineseZodiac	= byChineseZodiac;
	pCustomVCard->byConstellation	= byConstellation;
	//pCustomVCard->bySex				= bySex;
	//pCustomVCard->byProvince		= byProvince;
	//pCustomVCard->byArea			= byArea;
	get_fast_code()->memory_copy(pCustomVCard->chCity, tchCity, sizeof(TCHAR) * LEN_CITY_NAME);

	//pCustomVCard->byHeadUrlSize		= (strHeadUrl.size() + 1) * sizeof(TCHAR);
	pCustomVCard->bySignatureSize	= (strSignature.size() + 1) * sizeof(TCHAR);
	//get_fast_code()->memory_copy(pCustomVCard->byData, strHeadUrl.c_str(), pCustomVCard->byHeadUrlSize);
	get_fast_code()->memory_copy(pCustomVCard->byData /*+ pCustomVCard->byHeadUrlSize*/, strSignature.c_str(), pCustomVCard->bySignatureSize);

	pData = reinterpret_cast<BYTE *>(pCustomVCard) + dw_size;
}


DWORD tagRoleCustomVCard::GetCustomVCardDataSize() const
{
	return sizeof(tagCustomVCardData) - 1 + (/*strHeadUrl.size() + */strSignature.size() + 2) * sizeof(TCHAR);
}

//-----------------------------------------------------------------------------
// 序列化基本信息
//-----------------------------------------------------------------------------
void tagRoleVCardBase::Serialize(tagVCardData* pVCardData)
{
	pVCardData->dw_role_id	= dw_role_id;
	pVCardData->dwLevel		= dwLevel;
	pVCardData->dwJob		= dwJob;
	pVCardData->dwMateRoleID= dwMateRoleID;
	pVCardData->dwFactionID	= dwFactionID;
	pVCardData->dwPosition	= dwPosition;
}




tagCustomVCardData* tagRoleVCard::GetInitData()
{
	TCHAR* pTchHead = _T("");
	TCHAR* pTchSign = _T("");
	DWORD dwHeadSize = (_tcslen(pTchHead) + 1) * sizeof(TCHAR);
	DWORD dwSignSize = (_tcslen(pTchSign) + 1) * sizeof(TCHAR);

	DWORD dwCustomSize = sizeof(tagCustomVCardData) - 1 + dwHeadSize + dwSignSize;

	tagCustomVCardData* pCustom = reinterpret_cast<tagCustomVCardData * >(new BYTE[dwCustomSize]);


	//pCustom->bVisibility = FALSE;
	//pCustom->byArea = 0;
	//pCustom->byChineseZodiac = 0;
	pCustom->byConstellation = 0;
	//pCustom->byProvince = 0;
	//pCustom->bySex = 0;
	//pCustom->byHeadUrlSize = (BYTE)dwHeadSize;
	pCustom->bySignatureSize = (BYTE)dwSignSize;
	memcpy(pCustom->chCity, _T(""), LEN_CITY_NAME* sizeof(TCHAR));
	memcpy(pCustom->byData, pTchHead, dwHeadSize);		
	memcpy(pCustom->byData + dwHeadSize, pTchSign, dwSignSize);		
	return pCustom;
}

//-----------------------------------------------------------------------------
// 初始化
//-----------------------------------------------------------------------------
void tagRoleVCard::Init(const BYTE*& pData, Role *pRole)
{
	const tagVCardData* pVCData = (const tagVCardData*)pData;
	BOOL bSave = tagRoleVCardBase::InitUpdate(pRole, pVCData);

	// 初始化自定义信息
	const BYTE* pCustomData = (const BYTE*)&pVCData->customVCardData;
	customVCard.Init(&pVCData->customVCardData, pCustomData);
	pData = pCustomData;

	if (bSave)
	{
		// 更新数据库
		DWORD dwSaveDB = sizeof(NET_DB2C_vcard_change) - sizeof(tagVCardData) + GetVCardDataSize();

		CREATE_MSG(pSendDB, dwSaveDB, NET_DB2C_vcard_change);
		pSendDB->dw_role_id = dw_role_id;

		BYTE* pByte = NULL;
		Serialize(&(pSendDB->s_card_data_), pByte);

		ASSERT(pByte- (BYTE *)(&(pSendDB->s_card_data_)) == GetVCardDataSize());

		// 发送
		g_dbSession.Send(pSendDB, pSendDB->dw_size);
		MDEL_MSG(pSendDB);
	}
}

//-----------------------------------------------------------------------------
// 序列化 为tagVCardData
//-----------------------------------------------------------------------------
void tagRoleVCard::Serialize(IN tagVCardData* pVCardData, OUT BYTE*& pData)
{

	// 序列化基本信息
	tagRoleVCardBase::Serialize(pVCardData);

	// 序列化自定义信息
	customVCard.Serialize(&pVCardData->customVCardData, pData);
}

//-----------------------------------------------------------------------------
// 得到对应的tagVCardData尺寸
//-----------------------------------------------------------------------------
DWORD tagRoleVCard::GetVCardDataSize() const
{
	return sizeof(tagVCardData) - sizeof(tagCustomVCardData) + GetCustomDataSize();
}
DWORD tagRoleVCard::GetCustomDataSize() const
{
	return customVCard.GetCustomVCardDataSize();
}

#include "../common/ServerDefine/base_server_define.h"
#include "../../common/WorldDefine/role_card_protocol.h"
VOID tagRoleVCard::Test()
{
	tagRoleVCard testvcard;
// 	testvcard.Init(/*tagVCardData**/ pData);
// 	testvcard.SetCustomData(/*tagCustomVCardData**/ pData);
// 	testvcard.SendVCard2Client(/*DWORD */dwDstRoleID);
// 	testvcard.SendSaveDB();
// 	testvcard.SendLoadOffLineVCard(/*DWORD */dwDstRoleID);
	
// 	tagRoleVCard vcard;
// 
// 	tagCustomVCardData* pvcData = tagRoleVCard::GetInitData();
// 	const BYTE* pVCData = (BYTE *)pvcData;
// 	vcard.Init(pVCData, NULL);
// 	delete pvcData;
// 
// 	DWORD dw_size = sizeof(NET_SIS_get_role_card) - 1 + vcard.GetVCardDataSize();
// 
// 	MCREATE_MSG(pSend, dw_size, NET_SIS_get_role_card);
// 	pSend->dw_role_id = 1;
// 	pSend->dw_error_code = E_VCard_Success;
// //	vcard.Refresh();
// 	BYTE* pByte = NULL;
// 	vcard.Serialize(reinterpret_cast<tagVCardData *>( pSend->pData ), pByte);
// 
// 	ASSERT(pByte - pSend->pData == vcard.GetVCardDataSize());
// 
// 	tagVCardData* pcustomvcdata = reinterpret_cast<tagVCardData *>( pSend->pData );
// 	const BYTE* pDFADF = (BYTE*)(&(pcustomvcdata->customVCardData));
// 	vcard.Init(pDFADF, NULL);
// 
// 	MDEL_MSG(pSend);
// 
// 
// 	BYTE vcdata[1000];
// 	LPVOID pVdCData = vcdata;
// 	vcard.Save2DB(pVdCData, pVdCData);
}

void tagRoleVCard::SetCustomData( tagCustomVCardData* pData )
{
	const BYTE* pByte = NULL;
	customVCard.Init( pData, pByte);
}

void tagRoleVCard::SendVCard2Client( DWORD dwDstRoleID )
{
	Role* pDst = g_roleMgr.get_role(dwDstRoleID);
	if (!VALID_POINT(pDst))
		return;

	DWORD dw_size = sizeof(NET_SIS_get_role_card) - 1 + GetVCardDataSize();
	CREATE_MSG(pSend, dw_size, NET_SIS_get_role_card);

	pSend->dw_role_id = dw_role_id;
	pSend->dw_error_code = E_VCard_Success;
	Refresh();
	BYTE* pByte = NULL;
	Serialize(reinterpret_cast<tagVCardData *>( pSend->pData ), pByte);
	ASSERT(pByte - pSend->pData == GetVCardDataSize());
	pDst->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

void tagRoleVCard::SendSaveDB()
{
	DWORD dwSaveDB = sizeof(NET_DB2C_vcard_change) - sizeof(tagVCardData) + GetVCardDataSize();
	CREATE_MSG(pSendDB, dwSaveDB, NET_DB2C_vcard_change);

	pSendDB->dw_role_id = dw_role_id;
	BYTE* pByte = NULL;
	Serialize(&pSendDB->s_card_data_, pByte);
	ASSERT(pByte - (BYTE *)(&(pSendDB->s_card_data_)) == GetVCardDataSize());
	g_dbSession.Send(pSendDB, pSendDB->dw_size);

	MDEL_MSG(pSendDB);
}

void tagRoleVCard::SendLoadOffLineVCard( DWORD dwSrcRoleID, DWORD dwDstRoleID )
{
	NET_DB2C_get_off_line_vcard send;
	send.dw_role_id = dwSrcRoleID;
	send.dw_query_id = dwDstRoleID;
	g_dbSession.Send(&send, send.dw_size);
}

//void tagRoleVCard::SendHeadUrlTo( DWORD dwDstRoleID )
//{
//	Role* pDst = g_roleMgr.get_role(dwDstRoleID);
//	if (!VALID_POINT(pDst))
//		return;
//
//	const TCHAR* pUrl = customVCard.HeadUrl();
//	DWORD dwUrlSize = (_tcslen(pUrl) + 1) * sizeof(TCHAR);
//	DWORD dwMsgSize = sizeof(NET_SIS_get_role_head_picture) - 1 + dwUrlSize;
//
//	CREATE_MSG(pSend, dwMsgSize, NET_SIS_get_role_head_picture);
//	pSend->dw_role_id = dw_role_id;
//	pSend->byHeadUrlSize = BYTE(dwUrlSize);
//	get_fast_code()->memory_copy(pSend->byData, pUrl, dwUrlSize);
//	pDst->SendMessage(pSend, pSend->dw_size);
//	MDEL_MSG(pSend);
//}

void tagRoleVCard::SendNullUrlToMe( DWORD dwWhosHead )
{
	NET_SIS_get_role_head_picture send;
	send.dw_role_id = dwWhosHead;
	send.byHeadUrlSize = 0;
	ASSERT(VALID_POINT(m_pRole));
	m_pRole->SendMessage(&send, send.dw_size);
}

VOID tagRoleVCard::NotifyClientGetVCard( DWORD dw_role_id, DWORD dwErrCode )
{
	ASSERT(dwErrCode != E_VCard_Success);
	NET_SIS_get_role_card send;
	send.dw_role_id = dw_role_id;
	send.dw_error_code = dwErrCode;
	ASSERT(VALID_POINT(m_pRole));
	m_pRole->SendMessage(&send, send.dw_size);
}

VOID tagRoleVCard::NotifyClientSetVCard( DWORD dw_role_id, DWORD dwErrCode )
{
	NET_SIS_set_role_card send;
	send.dw_role_id = dw_role_id;
	send.dw_error_code = dwErrCode;
	ASSERT(VALID_POINT(m_pRole));
	m_pRole->SendMessage(&send, send.dw_size);
}