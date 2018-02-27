/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//称号系统

#pragma once

// project
#include "../../common/WorldDefine/title_define.h"
#include "../../common/WorldDefine/role_title_protocol.h"
#include "att_res.h"

// stl
#include <bitset>
#include <vector>
#include <list>
#include <algorithm>

using std::bitset;
using std::vector;
using std::list;
using std::find;

class TitleMgr
{
	typedef list<UINT16>					ListUINT16;
	typedef vector<UINT16>					VecUINT16;
	typedef bitset<MAX_TITLE_NUM>			BitSet;

public:
	TitleMgr();
	~TitleMgr(){	Destroy();	}

	// 1初始化选项
	void InitOpts(Role* pRole, DWORD u16ActTitleID, DWORD u16ActTitleID2, DWORD u16ActTitleID3, BOOL bVisibility);

	// 2初始化称号数据
	void InitTitles(const BYTE* &pData, const INT32 n_num);

	// 销毁数据
	void Destroy();

public:
	// 存储到数据库
	void SaveTitlesToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	// 获得称号
	VOID SetTitle(DWORD dwID);
	BOOL HasTitle(DWORD dwID);

	// 删除称号
	VOID DeleteTitle(DWORD dwID);

	// 设置称号可见性
	DWORD SetVisibility(bool bVisibility)	{	m_bVisibility = bVisibility;	return E_Title_Success;	}

	// 返回可见性
	bool Visibility() const { return TRUE == m_bVisibility; }

	// 激活称号
	DWORD setCurTitle(const DWORD u16TitleID, int nIndex);

	// 设置显示称号 gx add 2013.10.31
	VOID setCurShowTitle(INT nIndex,INT nvalue)
	{
		if (nIndex < 0 || nIndex >= 3)
			return ;
		m_u16ShowActive[nIndex] = nvalue;
	}//end
	
	void ActiviteTitle(const tagTitleProto* pProto, bool bActivite);

	// 当前使用称号
	DWORD GetActiviteTitle(int nIndex) const {	return m_u16ActiveID[nIndex];	}
	// 对应当前称号是否显示给其余玩家 gx add 2013.10.31
	INT GetShowActiviteTitle(int nIndex) const
	{
		if (nIndex < 0 || nIndex >= 3)
			return 0;
		return m_u16ShowActive[nIndex];
	}//end
	// 取得所有已获得称号数据
	DWORD GetObtainedTitleIDs(tagTitleData* &pData, DWORD &u16Num);

	// 已获得称号数目
	DWORD GetObtainedTitlesNum() {	return m_bitsetObtainedMark.count();}

	// 是否有某等级的称号
	BOOL HasLevelTitle(BYTE byLevel);

	// 是否有某颜色
	BOOL HasColorTitle(DWORD dwColor);
	// 购买称号
	DWORD BuyTitle(DWORD dwID);
	// 归还
	DWORD ReturnTitle(DWORD dwID);
	// 交还称号数据
	//DWORD ReturnTitlesData(LPVOID pData) {	SAFE_DELETE(pData);	return E_Title_Success;	}

	// 称号特殊处理
	//VOID TitleSpecEffect(DWORD u16TitleID);

	//VOID AddSpecEffect(Role* pRole, const tagTitleSpecEffect& st_TileSpecEffect);
	VOID Updata();
private:
	// 称号插入到数据库
	VOID InsertTitle2DB( DWORD u16TitleID);
	// 称号从数据库删除
	VOID DeleteTitle2DB( DWORD dwTitleID);

	// 通知客户端新获得的称号
	//void NotifyClient(VecUINT16& vecNewObtTitles);
	
private:
	DWORD								m_u16ActiveID[3];					// 当前称号
	INT									m_u16ShowActive[3];					//激活的称号是否显示 gx add
	BOOL								m_bVisibility;						// 可见性

	//ListUINT16							m_EventMap[ete_max_event_num];		// 事件映射
	//condition*							m_ArrPtrCondition[MAX_TITLE_NUM];	// 条件指针数组

	BitSet								m_bitsetObtainedMark;				// 称号是否已获得
	BitSet								m_bitsetDBInserted;					// 已插入数据库
	BitSet								m_bitsetNeedSaveDB;					// 需要保存到数据库
	
	std::map<DWORD,tagDWORDTime>		m_mapTitleDeleteTimes;				// 称号删除倒计时表


	Role*								m_pRole;							// 角色指针

	INT32								m_nTitleDeleteTick;					// 称号删除更新计时
};

