
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//·´Ë½·þip

#pragma once

class PSIpDict
{
	typedef std::vector<DWORD>	VecIp;

public:
	BOOL	Init();

	BOOL	LookUp(LPCSTR pIP);
	BOOL	LookUp(DWORD dw_ip);

public:
	BOOL	Add(LPCSTR pIP);
	BOOL	Add(DWORD dw_ip);
	
	static BOOL Test();

private:
	VecIp				m_vecIpDict;

	few_connect_client	trans_ip;	
};

extern PSIpDict	g_ipDict;