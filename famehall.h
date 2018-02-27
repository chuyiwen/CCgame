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

#include "famehall_part.h"
//----------------------------------------------------------------------------
//数据库保存办法
//	进入名人堂：	数据库插入消息
//	激活氏族珍宝：	数据库插入消息
//	声望重置时间：	数据库更新消息
//	角色排名：		STWorld定期保存声望信息
//数据库读取方法：	数据库消息

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// 名人堂
//----------------------------------------------------------------------------
class FameHall
{
public:// 主线程调用
	FameHall()	{}
	~FameHall()	{	Destroy();	}
	BOOL Init();

	VOID Destroy(){}

	// 周期更新各个氏族数据
	VOID Update();

	// 发送数据库消息
	VOID SendLoadDBData();

	// 处理声望排名消息
	VOID HandleUpdateRepRank(tag_net_message* pCmd);

	// 处理初始化氏族珍宝列表消息
	VOID HandleInitActTreasureList(tag_net_message* pCmd);

	// 初始化名人堂进入快照
	VOID HandleInitFameHallTop50(tag_net_message* pCmd);

	// 初始化声望重置时间
	VOID HandleInitRepRstTimeStamp(tag_net_message* pCmd);

	// 激活氏族珍宝（保留到主线程）
	DWORD ActClanTreasure(Role* pRole, UINT16 u16TreasureID);

	// 角色读取后更新声望值
	VOID RoleRepUpdate(Role* pRole, ECLanType eClanType);

public:// 地图线程调用
	// 尝试进入名人堂
	BOOL TryEnterFameHall(Role* pRole, ECLanType eClanType);

	// 获取名人堂最多前50（nameid）
	VOID GetMemberTop50(BYTE* pData, ECLanType eClanType);

	// 获取名人堂最多前50具体数目
	INT32 GetMemberTop50Num(ECLanType byClanType);

	// 获得声望排名（tagRepRankData）
	VOID GetRepRankTop(PVOID pData, ECLanType eClanType);

	// 获得声望排名数目
	INT32 GetRepRankNum(ECLanType byClanType);

	// 获得已激活氏族珍宝列表
	VOID GetActClanTreasure(PVOID pData, ECLanType eClanType);

	// 获得已激活氏族珍宝数目
	INT32 GetActClanTreasureNum(ECLanType byClanType);

	// 获得声望排名更新时间
	tagDWORDTime GetRepRankTimeStamp(ECLanType byClanType) const	{	return m_ArrClanTrunk[byClanType].GetRepRankUpdateTime();}

	// 获得名人堂列表更新时间
	tagDWORDTime GetFameSnapTimeStamp(ECLanType byClanType) const	{	return m_ArrClanTrunk[byClanType].GetEnterSnapUpdateTime();}

private:
	ClanTrunk			m_ArrClanTrunk[ECLT_NUM];			// 氏族数据数组
	BOOL				m_bInitOK;							// 初始化成功

private:// 禁用拷贝
	FameHall(const FameHall& rhs);
	FameHall& operator = (const FameHall& rhs);
};


extern FameHall g_fameHall;
