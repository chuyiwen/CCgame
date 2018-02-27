
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

#include "StdAfx.h"
#include "ps_ipchecker.h"
#include "world.h"

PSIpDict g_ipDict;

LPCTSTR	tszInfPath = _T("server_data/information_position.ins");

BOOL PSIpDict::Init()
{
	file_container* p_var = new file_container;

	p_var->set_save(TRUE);

	if (!p_var->load(g_world.get_virtual_filesys(), tszInfPath))
	{
		return FALSE;
	}

	std::list<tstring>& rNameList = p_var->get_element_name();
	for (std::list<tstring>::iterator itr = rNameList.begin(); itr != rNameList.end(); ++itr)
	{
		LPSTR pIP = (LPSTR)get_tool()->unicode_to_ansi(p_var->get_string((*itr).c_str()));
		DWORD dw_ip = trans_ip.stringip_to_ip(pIP);
		Add(dw_ip);
	}

	SAFE_DELETE(p_var);
	

	return TRUE;
}

BOOL PSIpDict::LookUp( LPCSTR pIP )
{
	DWORD dw_ip = trans_ip.stringip_to_ip((LPSTR)pIP);

	return LookUp(dw_ip);
}

BOOL PSIpDict::LookUp( DWORD dw_ip )
{
	return m_vecIpDict.end() != std::find(m_vecIpDict.begin(), m_vecIpDict.end(), get_tool()->crc32(LPBYTE(&dw_ip), sizeof(DWORD)));
}

BOOL PSIpDict::Add( LPCSTR pIP )
{
	DWORD dw_ip = trans_ip.stringip_to_ip((LPSTR)pIP);

	return Add(dw_ip);
}

BOOL PSIpDict::Add( DWORD dw_ip )
{
	if (LookUp(dw_ip))
	{
		return FALSE;
	}
	m_vecIpDict.push_back(get_tool()->crc32(LPBYTE(&dw_ip), sizeof(DWORD)));
	return TRUE;
}

BOOL PSIpDict::Test()
{
	PSIpDict ipDict;

	ASSERT( ipDict.Init() );

	ASSERT( ipDict.LookUp("127.0.0.1") );


	return TRUE;

}