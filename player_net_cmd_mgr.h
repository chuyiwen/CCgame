
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端命令管理
#pragma once

class PlayerSession;

typedef DWORD (PlayerSession::*NETMSGHANDLER)(tag_net_message*);

//-----------------------------------------------------------------------------
// 客户端命令管理器
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

	// 取得消息执行次数
	UINT32 GetRecvCmdRunTimes(DWORD dwMsgID);


protected:
	typedef struct tagPlayerCmd
	{
		std::string				strCmd;			// 命令名
		tstring					strDesc;		// 描述
		DWORD					dw_size;			// 消息大小
		NETMSGHANDLER			handler;		// 函数指针
		volatile UINT32			nTimes;			// 此命令的次数
	} tagPlayerCmd;

	

	package_map<DWORD, tagPlayerCmd*>	m_mapRecvProc;	// 接收消息的处理及统计
	package_map<DWORD,	tagPlayerCmd*>	m_mapSendProc;	// 发送消息的处理及统计
};