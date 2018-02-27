
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//vipÍø°É
#pragma once
#include <vector>

//-------------------------------------------------------------------------
// ip¶Î
//-------------------------------------------------------------------------
class IpRange
{
public:
	IpRange(DWORD dwIpMin, DWORD dwIpMax, DWORD dwVNBId)
		:m_IPMin(dwIpMin), m_IPMax(dwIpMax), m_dwVNBId(dwVNBId){}
	BOOL	Fit(DWORD dw_ip)			const	{	return dw_ip >= m_IPMin && dw_ip <= m_IPMax;	}
	BOOL	OnLeftOf(DWORD dw_ip)	const	{	return dw_ip > m_IPMax;	}
	BOOL	OnRightOf(DWORD dw_ip)	const	{	return dw_ip < m_IPMin;	}
	DWORD	GetVNBId()				const	{	return m_dwVNBId;		}
	DWORD	GetIpMin()				const	{	return m_IPMin;			}
	DWORD	GetIpMax()				const	{	return m_IPMax;			}
private:
	DWORD	m_IPMin;
	DWORD	m_IPMax;
	DWORD	m_dwVNBId;
};


struct s_db_vnb_players;
struct s_vnb;

//-------------------------------------------------------------------------
// vipÍø°É¹ÜÀíÆ÷
//-------------------------------------------------------------------------
class VipNerBarMgr
{
	typedef package_map<DWORD, s_vnb*>	VipNetBarMap;
	typedef std::vector<IpRange>		IpRangeVector;
	typedef std::set<DWORD>				AccountIDSet;
	typedef package_map<DWORD, DWORD>			IP2VNBIdMap;
	typedef std::set<DWORD>				NotifySet;

public:
	BOOL	Init();
	VOID	Destroy();
	VOID	InitData(s_db_vnb_players* pInitData);
	VOID	PlayerLogin(DWORD dw_account_id, DWORD dw_ip);
	VOID	PlayerLogout(DWORD dw_ip);
	VOID	PlayerNotify(DWORD dw_account_id);
	INT		GetRate(DWORD dw_ip, INT nType);
	LPCTSTR	GetVNBName(DWORD dw_ip);
	

private:
	DWORD	TransTSIp2DWORD(LPCTSTR szIP);
	s_vnb* GetVipNetBar(DWORD dw_ip);
	DWORD	GetVNBId(DWORD dwIp);
	DWORD	FitNetBar(DWORD dwIp);
	VOID	UpdateDbPlayerLogin(DWORD dw_account_id, DWORD dw_time);
	VOID	GeneralzeIP(DWORD &dw_ip);

private:
	IP2VNBIdMap			m_mapIp2VNBId;
	VipNetBarMap		m_mapVipNetBars;
	IpRangeVector		m_vecIpRanges;				// ÓÐÐò
	AccountIDSet		m_setHistoryAccountID;
	AccountIDSet		m_setTodayAccountID;
	NotifySet			m_setNotify;	
	few_connect_client  trans_ip;
public:
	BOOL DynamicTest(DWORD dwTestNo, DWORD dwArg1, LPCTSTR szArg2);
};

extern VipNerBarMgr g_VipNetBarMgr;