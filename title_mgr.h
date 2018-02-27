/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ƺ�ϵͳ

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

	// 1��ʼ��ѡ��
	void InitOpts(Role* pRole, DWORD u16ActTitleID, DWORD u16ActTitleID2, DWORD u16ActTitleID3, BOOL bVisibility);

	// 2��ʼ���ƺ�����
	void InitTitles(const BYTE* &pData, const INT32 n_num);

	// ��������
	void Destroy();

public:
	// �洢�����ݿ�
	void SaveTitlesToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

	// ��óƺ�
	VOID SetTitle(DWORD dwID);
	BOOL HasTitle(DWORD dwID);

	// ɾ���ƺ�
	VOID DeleteTitle(DWORD dwID);

	// ���óƺſɼ���
	DWORD SetVisibility(bool bVisibility)	{	m_bVisibility = bVisibility;	return E_Title_Success;	}

	// ���ؿɼ���
	bool Visibility() const { return TRUE == m_bVisibility; }

	// ����ƺ�
	DWORD setCurTitle(const DWORD u16TitleID, int nIndex);

	// ������ʾ�ƺ� gx add 2013.10.31
	VOID setCurShowTitle(INT nIndex,INT nvalue)
	{
		if (nIndex < 0 || nIndex >= 3)
			return ;
		m_u16ShowActive[nIndex] = nvalue;
	}//end
	
	void ActiviteTitle(const tagTitleProto* pProto, bool bActivite);

	// ��ǰʹ�óƺ�
	DWORD GetActiviteTitle(int nIndex) const {	return m_u16ActiveID[nIndex];	}
	// ��Ӧ��ǰ�ƺ��Ƿ���ʾ��������� gx add 2013.10.31
	INT GetShowActiviteTitle(int nIndex) const
	{
		if (nIndex < 0 || nIndex >= 3)
			return 0;
		return m_u16ShowActive[nIndex];
	}//end
	// ȡ�������ѻ�óƺ�����
	DWORD GetObtainedTitleIDs(tagTitleData* &pData, DWORD &u16Num);

	// �ѻ�óƺ���Ŀ
	DWORD GetObtainedTitlesNum() {	return m_bitsetObtainedMark.count();}

	// �Ƿ���ĳ�ȼ��ĳƺ�
	BOOL HasLevelTitle(BYTE byLevel);

	// �Ƿ���ĳ��ɫ
	BOOL HasColorTitle(DWORD dwColor);
	// ����ƺ�
	DWORD BuyTitle(DWORD dwID);
	// �黹
	DWORD ReturnTitle(DWORD dwID);
	// �����ƺ�����
	//DWORD ReturnTitlesData(LPVOID pData) {	SAFE_DELETE(pData);	return E_Title_Success;	}

	// �ƺ����⴦��
	//VOID TitleSpecEffect(DWORD u16TitleID);

	//VOID AddSpecEffect(Role* pRole, const tagTitleSpecEffect& st_TileSpecEffect);
	VOID Updata();
private:
	// �ƺŲ��뵽���ݿ�
	VOID InsertTitle2DB( DWORD u16TitleID);
	// �ƺŴ����ݿ�ɾ��
	VOID DeleteTitle2DB( DWORD dwTitleID);

	// ֪ͨ�ͻ����»�õĳƺ�
	//void NotifyClient(VecUINT16& vecNewObtTitles);
	
private:
	DWORD								m_u16ActiveID[3];					// ��ǰ�ƺ�
	INT									m_u16ShowActive[3];					//����ĳƺ��Ƿ���ʾ gx add
	BOOL								m_bVisibility;						// �ɼ���

	//ListUINT16							m_EventMap[ete_max_event_num];		// �¼�ӳ��
	//condition*							m_ArrPtrCondition[MAX_TITLE_NUM];	// ����ָ������

	BitSet								m_bitsetObtainedMark;				// �ƺ��Ƿ��ѻ��
	BitSet								m_bitsetDBInserted;					// �Ѳ������ݿ�
	BitSet								m_bitsetNeedSaveDB;					// ��Ҫ���浽���ݿ�
	
	std::map<DWORD,tagDWORDTime>		m_mapTitleDeleteTimes;				// �ƺ�ɾ������ʱ��


	Role*								m_pRole;							// ��ɫָ��

	INT32								m_nTitleDeleteTick;					// �ƺ�ɾ�����¼�ʱ
};

