
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ͻ����������
#pragma once

class PlayerSession;

typedef DWORD (PlayerSession::*NETMSGHANDLER)(tag_net_message*);

//-----------------------------------------------------------------------------
// �ͻ������������
//-----------------------------------------------------------------------------
class PlayerNetCmdMgr
{
public:
	PlayerNetCmdMgr();
	~PlayerNetCmdMgr();

	VOID Init();
	VOID Destroy();

	VOID LogAllMsg();

	BOOL RegisterRecvProc(LPCSTR szCmd, NETMSGHANDLER fp, LPCTSTR szDesc, DWORD dw_size);
	BOOL RegisterSendProc(LPCSTR szCmd);
	VOID UnRegisterAll();

	NETMSGHANDLER GetHandler(tag_net_message* p_message, UINT32 nMsgSize);
	VOID CountServerMsg(DWORD dwMsgID);

	BOOL HandleCmd(tag_net_message* p_message, DWORD nMsgSize, PlayerSession* pSession);

	// ȡ����Ϣִ�д���
	UINT32 GetRecvCmdRunTimes(DWORD dwMsgID);


protected:
	typedef struct tagPlayerCmd
	{
		std::string				strCmd;			// ������
		tstring					strDesc;		// ����
		DWORD					dw_size;			// ��Ϣ��С
		NETMSGHANDLER			handler;		// ����ָ��
		volatile UINT32			nTimes;			// ������Ĵ���
	} tagPlayerCmd;

	

	package_map<DWORD, tagPlayerCmd*>	m_mapRecvProc;	// ������Ϣ�Ĵ���ͳ��
	package_map<DWORD,	tagPlayerCmd*>	m_mapSendProc;	// ������Ϣ�Ĵ���ͳ��
};