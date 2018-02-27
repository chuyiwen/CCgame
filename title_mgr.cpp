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

#include "stdafx.h"
#include "title_mgr.h"
#include "../common/ServerDefine/title_server_define.h"
#include "../../common/WorldDefine/role_title_protocol.h"
#include "role.h"


TitleMgr::TitleMgr()
{
	memset(m_u16ActiveID, -1, sizeof(m_u16ActiveID));
	memset(m_u16ShowActive,1,sizeof(m_u16ShowActive));//Ĭ�϶���ʾ gx add
	m_bVisibility = false;
	m_pRole = NULL;
	m_nTitleDeleteTick = 0;
}
// 1��ʼ��ѡ��
void TitleMgr::InitOpts(Role* pRole, DWORD u16ActTitleID, DWORD u16ActTitleID2, DWORD u16ActTitleID3, BOOL bVisibility)
{
	
	ASSERT( VALID_POINT(pRole) );
	m_pRole = pRole;
	setCurTitle(u16ActTitleID, 0);
	setCurTitle(u16ActTitleID2, 1);
	setCurTitle(u16ActTitleID3, 2);
	//m_u16ActiveID[0] = u16ActTitleID;
	//m_u16ActiveID[1] = u16ActTitleID2;
	//m_u16ActiveID[2] = u16ActTitleID3;

	m_bVisibility = bVisibility;

	//m_nTitleDeleteTick = TITLE_UPDATA_TIME;
}

// 2��ʼ���ƺ�����
void TitleMgr::InitTitles(const BYTE* &pData, const INT32 n_num)
{
	m_bitsetObtainedMark.reset();
	//m_bitsetDBInserted.reset();
	m_bitsetNeedSaveDB.reset();


	// ��ʼ����ȡ�ĳƺ�����
	const s_title_save* pTitleLoad = reinterpret_cast<const s_title_save*>( pData );
	for( INT nLoadIndex = 0; nLoadIndex < n_num; ++nLoadIndex )
	{
		DWORD u16TitleID	= pTitleLoad[nLoadIndex].n_title_id_;
		//DWORD dwStateMark	= pTitleLoad[nLoadIndex].dw_state_mark_;
		
		const tagTitleProto* pProto = AttRes::GetInstance()->GetTitleProto(u16TitleID);
		
		// ��ʱ�����ƵĻ�
		if (VALID_POINT(pProto) && pProto->m_dwTimeLimit != 0)
		{
			tagDWORDTime dwTimeLimit = pTitleLoad[nLoadIndex].dw_time;
			m_mapTitleDeleteTimes[u16TitleID] = dwTimeLimit;
		}
		// �ѻ�óƺ�
		m_bitsetObtainedMark.set(u16TitleID);
		
	}


	//// ���¼���ָ��λ��
	pData = reinterpret_cast<const BYTE *>( pTitleLoad + n_num );

}
// ��������
void TitleMgr::Destroy()
{
	
}

// ��óƺ�
VOID TitleMgr::SetTitle(DWORD dwID)
{

	if (m_bitsetObtainedMark.test(dwID))
	{
		InsertTitle2DB( dwID );

		const tagTitleProto* pProto = AttRes::GetInstance()->GetTitleProto(dwID);

		// ��ʱ�����ƵĻ�
		if (VALID_POINT(pProto) && pProto->m_dwTimeLimit != 0)
		{
			m_mapTitleDeleteTimes[dwID] = GetCurrentDWORDTime();
		}

		return;
	}

	// �»�óƺ�
	m_bitsetObtainedMark.set( dwID );

	// ����Ҫ��������ݿ�
	InsertTitle2DB( dwID );
	m_bitsetDBInserted.set(dwID);
	
	
	const tagTitleProto* pProto = AttRes::GetInstance()->GetTitleProto(dwID);

	// ��ʱ�����ƵĻ�
	if (VALID_POINT(pProto) && pProto->m_dwTimeLimit != 0)
	{
		m_mapTitleDeleteTimes[dwID] = GetCurrentDWORDTime();
	}

	// �ƺ����⴦��
	//TitleSpecEffect(dwID);
	
	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_title_num, m_bitsetObtainedMark.count());
	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_title_get, dwID, 1);

	// ���͸��ͻ���		
	NET_SIS_net_titles send;
	send.dw_role_id = m_pRole->GetID();
	send.dw_title_id = dwID;
	send.dw_time = GetCurrentDWORDTime();
	m_pRole->SendMessage(&send, send.dw_size);


	for (int i = 0; i < 3; i++)
	{
		if (m_u16ActiveID[i] == INVALID_VALUE || m_u16ActiveID[i] == 0)
		{
			setCurTitle(i, dwID);
			break;
		}
	}
	
	//NotifyClient( vecNewObtTitleIDs );

}
BOOL TitleMgr::HasTitle(DWORD dwID)
{
	return m_bitsetObtainedMark.test(dwID);
}

// ɾ���ƺ�
VOID TitleMgr::DeleteTitle(DWORD dwID)
{
	if (!m_bitsetObtainedMark.test(dwID))
		return;

	m_bitsetObtainedMark.reset( dwID );

	DeleteTitle2DB( dwID );

	// ���͸��ͻ���		
	NET_SIS_delete_title send;
	send.dw_role_id = m_pRole->GetID();
	send.dw_title_id = dwID;
	m_pRole->SendMessage(&send, send.dw_size);

}


// �洢�����ݿ�
void TitleMgr::SaveTitlesToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num)
{
	//s_title_save* pCondData = static_cast<s_title_save *>( pData );
	//INT nTitle2SaveNum = 0;
	//for (INT nTitleIndex = 0; nTitleIndex < MAX_TITLE_NUM; ++nTitleIndex)
	//{
	//	if (!m_bitsetNeedSaveDB.test(nTitleIndex))
	//		continue;

	//	if ( !m_bitsetObtainedMark.test(nTitleIndex) && m_bitsetDBInserted.test(nTitleIndex) )
	//	{
	//		pCondData[nTitle2SaveNum].n_title_id_ = nTitleIndex;
	//		++nTitle2SaveNum;
	//	}
	//	else if( m_bitsetObtainedMark.test(nTitleIndex) && m_bitsetDBInserted.test(nTitleIndex) )
	//	{
	//		pCondData[nTitle2SaveNum].n_title_id_ = nTitleIndex;
	//		++nTitle2SaveNum;
	//	}

	//	m_bitsetNeedSaveDB.reset(nTitleIndex);
	//}
	//pOutPointer = static_cast<BYTE *>(pData) + sizeof(s_title_save) * nTitle2SaveNum;
	//n_num += nTitle2SaveNum;
}

// ֪ͨ�ͻ���
//void TitleMgr::NotifyClient(VecUINT16& vecNewTitleIDs)
//{	
//	INT nNewTitleNum = vecNewTitleIDs.size();
//
//	ASSERT( nNewTitleNum > 0);
//	if (nNewTitleNum <= 0)
//	{
//		return;
//	}
//
//	DWORD dw_size = sizeof(NET_SIS_net_titles) - 1 + sizeof(UINT16) * nNewTitleNum;
//	CREATE_MSG(pSend, dw_size, NET_SIS_net_titles);
//
//	pSend->dw_role_id = m_pRole->GetID();
//	pSend->u16TitleNum = nNewTitleNum;
//
//	UINT16* pu16TitleID = reinterpret_cast<UINT16 *>( pSend->byData );
//	INT	nTitleSendNum = 0;
//	VecUINT16::iterator itrNewTitle = vecNewTitleIDs.begin();
//	while( itrNewTitle != vecNewTitleIDs.end() )
//	{
//		pu16TitleID[nTitleSendNum++] = *itrNewTitle;
//		++itrNewTitle;
//	}
//
//	ASSERT( nNewTitleNum == nTitleSendNum );
//	if (nNewTitleNum != nTitleSendNum)
//	{
//		MDEL_MSG(pSend);
//		return;
//	}
//
//	m_pRole->SendMessage(pSend, pSend->dw_size);
//	MDEL_MSG(pSend);
//	return ;
//}


// ����ƺ�
DWORD TitleMgr::setCurTitle(const DWORD u16TitleID, int nIndex)
{
	if (u16TitleID < 0 || u16TitleID >= MAX_TITLE_NUM)
		return E_Title_TitleActivateFailed;
	if (!m_bitsetObtainedMark.test(u16TitleID))
		return E_Title_TitleActivateFailed;

	for (int i = 0; i < 3; i++)
	{
		if( u16TitleID == m_u16ActiveID[i] )
			return E_Title_Success;
	}



	if( TITLEID_VALID(m_u16ActiveID[nIndex]) )
	{// ����
		//DWORD dwBuffID = AttRes::GetInstance()->GetTitleProto(m_u16ActiveID[nIndex])->m_dwBuffID;
		//m_pRole->RemoveBuff(Buff::GetIDFromTypeID(dwBuffID), TRUE);
		ActiviteTitle(AttRes::GetInstance()->GetTitleProto(m_u16ActiveID[nIndex]), false);
		m_u16ActiveID[nIndex] = u16TitleID;
		if( TITLEID_VALID(m_u16ActiveID[nIndex]) )
		{
			if (!m_bitsetObtainedMark.test(m_u16ActiveID[nIndex]))
			{
				m_u16ActiveID[nIndex] = INVALID_VALUE;
				return E_Title_TitleActivateFailed;
			}

			//DWORD dwBuffID = AttRes::GetInstance()->GetTitleProto(m_u16ActiveID[nIndex])->m_dwBuffID;
			//m_pRole->TryAddBuff(m_pRole, AttRes::GetInstance()->GetBuffProto(dwBuffID), NULL, NULL, NULL);
			ActiviteTitle(AttRes::GetInstance()->GetTitleProto(m_u16ActiveID[nIndex]), true);
		}
	}
	else if( TITLEID_VALID(u16TitleID) )
	{// ����
		ASSERT( !TITLEID_VALID(m_u16ActiveID[nIndex]) );
		m_u16ActiveID[nIndex] = u16TitleID;
		if (!TITLEID_VALID(m_u16ActiveID[nIndex]) || !m_bitsetObtainedMark.test(m_u16ActiveID[nIndex]))
		{
			m_u16ActiveID[nIndex] = INVALID_VALUE;
			return E_Title_TitleActivateFailed;
		}
		//DWORD dwBuffID = AttRes::GetInstance()->GetTitleProto(m_u16ActiveID[nIndex])->m_dwBuffID;
		//m_pRole->TryAddBuff(m_pRole, AttRes::GetInstance()->GetBuffProto(dwBuffID), NULL, NULL, NULL);
		ActiviteTitle(AttRes::GetInstance()->GetTitleProto(m_u16ActiveID[nIndex]), true);
	}

	m_pRole->RecalAtt(TRUE);
	// ֪ͨ�ͻ���
	NET_SIS_role_title_change_broad send;
	send.dw_role_id	= m_pRole->GetID();
	send.nIndex = nIndex;
	send.dwTitleID	= m_u16ActiveID[nIndex];
	m_pRole->get_map()->send_big_visible_tile_message(m_pRole, &send, send.dw_size);

	return E_Title_Success;

}

// ȡ�������ѻ�óƺ�����
DWORD TitleMgr::GetObtainedTitleIDs(tagTitleData* &pData, DWORD &u16Num)
{
	u16Num = 0;
	for (UINT i = 0; i < MAX_TITLE_NUM; ++i)
	{
		if ( m_bitsetObtainedMark.test(i) )
		{
			pData[u16Num].dwTitleID = i;

			if (m_mapTitleDeleteTimes.find(i) != m_mapTitleDeleteTimes.end())
			{
				pData[u16Num].dwTime = m_mapTitleDeleteTimes[i];
			}
			else
			{
				pData[u16Num].dwTime = 0;
			}
			
			u16Num++;
		}
	}

	ASSERT( m_bitsetObtainedMark.count() == u16Num );
	if (m_bitsetObtainedMark.count() != u16Num)
	{
		return E_Title_UnknownError;
	}
	pData += u16Num;
	return E_Title_Success;			
}

BOOL TitleMgr::HasLevelTitle(BYTE byLevel)
{
	if (byLevel < 1)
		return true;

	for (UINT i = 0; i < MAX_TITLE_NUM; ++i)
	{
		if ( m_bitsetObtainedMark.test(i) )
		{
			const tagTitleProto* pProto = AttRes::GetInstance()->GetTitleProto(i);
			// vip�ƺŲ���ȼ�
			if (VALID_POINT(pProto) && pProto->m_byLevel >= byLevel && pProto->m_nNeedYuanbaoLevel <= 0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
BOOL TitleMgr::HasColorTitle(DWORD dwColor)
{
	for (UINT i = 0; i < MAX_TITLE_NUM; ++i)
	{
		if ( m_bitsetObtainedMark.test(i) )
		{
			const tagTitleProto* pProto = AttRes::GetInstance()->GetTitleProto(i);
			
			if (VALID_POINT(pProto) && pProto->m_dwColor == dwColor && pProto->m_dwType == TT_SANHUI)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
// �ƺŲ��뵽���ݿ�
VOID TitleMgr::InsertTitle2DB( DWORD u16TitleID)
{
	NET_DB2C_title_insert send;
	send.dw_role_id = m_pRole->GetID();
	send.s_title_save_.n_title_id_ = u16TitleID;
	send.s_title_save_.dw_time = GetCurrentDWORDTime();
	g_dbSession.Send( &send, send.dw_size );
}

// �ƺŴ����ݿ�ɾ��
VOID TitleMgr::DeleteTitle2DB( DWORD dwTitleID)
{
	NET_DB2C_title_delete send;
	send.dw_role_id = m_pRole->GetID();
	send.dw_title_id = dwTitleID;
	g_dbSession.Send( &send, send.dw_size );
}

// ����ƺ�
DWORD TitleMgr::BuyTitle(DWORD dwID)
{
	const tagTitleProto* pTitle = AttRes::GetInstance()->GetTitleProto(dwID);
	if (!VALID_POINT(pTitle))
	{
		return E_Title_Buy_Not_Find;
	}
	
	if (!pTitle->m_bSpecial)
	{
		return E_Title_Buy_Cannot_buy;
	}

	PlayerSession* pSession = m_pRole->GetSession();
	if( !VALID_POINT(pSession) || pSession->GetTotalRecharge() < pTitle->m_nNeedYuanbaoLevel)
	{
		return E_Title_Buy_Account_level_not;
	}

	// �ɾ͵�������
	if (m_pRole->GetAchievementPoint() < pTitle->m_dwAchievementPoint)
	{
		return E_Title_Buy_Not_point;
	}
	// �ȼ�����
	if (m_pRole->get_level() < pTitle->m_byNeedRoleLevel)
	{
		return E_Title_Buy_Not_level;
	}
	
	// �Ƿ���ǰ�õȼ��ƺ�
	if (!HasLevelTitle(pTitle->m_byLevel-1))
	{
		return E_Titel_Buy_Not_Before;
	}
	
	if (pTitle->m_dwType == TT_SANHUI && HasColorTitle(pTitle->m_dwColor))
	{
		return  E_Title_Buy_Has_Color;
	}
	// �óƺ��Ѿ�����
	if (m_bitsetObtainedMark.test(dwID))
	{
		return E_Title_Buy_Realy_Have;
	}

	// ��Ҫ�ĳɾ͵ȼ�����
	//if (AchievementHelper::GetAchievementLevel(m_pRole->GetAchievementMgr().GetComplateNumber()) < pTitle->m_nNeedAchievementLevel)
	//{
	//	return E_Title_Buy_Achievement_level_not;
	//}

	// �۳��ɾ͵���
	//m_pRole->ModAchievementPoint(-pTitle->m_dwAchievementPoint);

	SetTitle(dwID);

	return E_Title_Success;
}

// �黹
DWORD TitleMgr::ReturnTitle(DWORD dwID)
{
	const tagTitleProto* pTitle = AttRes::GetInstance()->GetTitleProto(dwID);
	if (!VALID_POINT(pTitle))
	{
		return INVALID_VALUE;
	}
	
	if (!pTitle->m_bSpecial)
	{
		return E_Title_Return_Cannot;
	}

	DeleteTitle(dwID);

	return E_Title_Success;
}
VOID TitleMgr::Updata()
{
	if(--m_nTitleDeleteTick > 0)
		return;
	
	m_nTitleDeleteTick = TITLE_UPDATA_TIME;

	std::map<DWORD,tagDWORDTime>::iterator iter = m_mapTitleDeleteTimes.begin();
	for (; iter != m_mapTitleDeleteTimes.end(); )
	{
		DWORD dwTitleID = iter->first;
		const tagTitleProto* pTitle = AttRes::GetInstance()->GetTitleProto(dwTitleID);
		if (VALID_POINT(pTitle))
		{
			// ʱ�䵽��,ɾ���óƺ�
			if(CalcTimeDiff(g_world.GetWorldTime(), iter->second) >= pTitle->m_dwTimeLimit)
			{
				for (int i = 0; i < 3; i++)
				{
					if (m_u16ActiveID[i] == dwTitleID)
					{
						setCurTitle(INVALID_VALUE, i);
					}
				}
	

				DeleteTitle(dwTitleID);

				m_mapTitleDeleteTimes.erase(iter++);
			}
			else
			{
				++iter;
			}
		}
		else
		{
			++iter;
		}
	}
}

void TitleMgr::ActiviteTitle( const tagTitleProto* pProto, bool bActivite )
{
	if (!VALID_POINT(pProto))
		return;
	
	int nSet = bActivite ? 1 : -1;

	for (int i = 0; i < MAX_TITLE_SPECNUM; i++)
	{
		if (pProto->m_roleAtt[i].eRoleAtt != ERA_Null &&
			pProto->m_roleAtt[i].nValue != 0)
		{
			m_pRole->ModAttModValue(pProto->m_roleAtt[i].eRoleAtt, pProto->m_roleAtt[i].nValue * nSet);
		}
		
	}
}

// �ƺ����⴦��
//VOID TitleMgr::TitleSpecEffect(DWORD u16TitleID)
//{
//	const tagTitleProto* pProto = AttRes::GetInstance()->GetTitleProto(u16TitleID);
//	if(!VALID_POINT(pProto))
//		return;
//
//	for (int i = 0; i < MAX_TITLE_SPECNUM; i++)
//	{
//		AddSpecEffect(m_pRole, pProto->st_TitleSpecEffect[i]);
//	}
//	
//
//}

// ��ӳƺ�Ч��
//VOID TitleMgr::AddSpecEffect(Role* pRole, const tagTitleSpecEffect& st_TileSpecEffect)
//{
	//if(!VALID_POINT(pRole))
	//	return;

	//switch(st_TileSpecEffect.eSpecTarget)
	//{
	//case ett_own:
	//	{
	//		/*if(st_TileSpecEffect.eType == ETS_Exp)
	//		{
	//			if(st_TileSpecEffect.eSpecTarget == ETT_Own)
	//			{
	//				pRole->ExpChange(st_TileSpecEffect.nValue);
	//			}
	//		}*/
	//		break;
	//	}
	//case ett_master:
	//	{
	//		break;
	//	}
	//}
//}