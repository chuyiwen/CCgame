/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//����sns

#include "StdAfx.h"

#include "role.h"
#include "pet_pocket.h"
#include "pet_soul.h"
#include "../../common/WorldDefine/pet_sns_protocol.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "pet_sns_mgr.h"
#include "social_mgr.h"
#include "mail_mgr.h"
#include "item_creator.h"
#include "../../common/WorldDefine/social_protocol.h"

BOOL Role::IsFriendHasPaiqianPet(DWORD dw_frined_id)
{
	DWORD dwPetID[MAX_PETSOUL_NUM];
	INT nNum;
	GetPetPocket()->GetAllPetID(dwPetID, nNum);

	for (int i = 0; i < nNum; i++)
	{
		PetSoul* pPetSoul = GetPetPocket()->GetPetSoul(dwPetID[i]);
		if (VALID_POINT(pPetSoul) /*&& pPetSoul->GetPaiqianFriendID() == dw_frined_id*/)
		{
			return TRUE;
		}
	}

	return FALSE;
}
DWORD Role::PaiqianPet(DWORD dw_pet_id, DWORD dw_friend_id, DWORD dw_time)
{
	if( !IS_PET(dw_pet_id))
	{
		//ASSERT(0);
		return E_Pets_SNS_UnkownErr;
	}

	PetSoul* pCalledSoul = GetPetPocket()->GetPetSoul(dw_pet_id);
	if (!VALID_POINT(pCalledSoul))
	{
		//ASSERT(0);
		return E_Pets_SNS_Soul_NotExist;

	}

	// �ܷ���ǲ
	if (!pCalledSoul->CanSetWroking())
	{
		return E_PetS_SNS_Cannot_paiqian;
	}

	// �Է��Ƿ�����
	Role* pFriend = g_roleMgr.get_role(dw_friend_id);
	if (!VALID_POINT(pFriend))
	{
		return E_PetS_SNS_Friend_NotExis;
	}


	// �Ƿ��Ǻ���
	tagFriend* ptagFriend = GetFriendPtr(dw_friend_id);
	if (!VALID_POINT(ptagFriend))
	{
		return E_PetS_SNS_Friend_NotExis;
	}

	// �Ƿ���˫�����
	ptagFriend = pFriend->GetFriendPtr(GetID());
	if (!VALID_POINT(ptagFriend))
	{
		return E_PetS_SNS_Friend_NotExis;
	}

	// �ȼ���̫��
	INT nLevelSub = get_level() - pFriend->get_level();
	if (nLevelSub < -5 || nLevelSub > 5)
	{
		return E_PetS_SNS_Friend_Level_NotExis;
	}


	if ( g_petSnsMgr.IsFriendHasPaiqianPet(GetID(), dw_friend_id))
	{
		return E_PetS_SNS_Friend_Has_pet;
	}

	tagPetSNSInfo snsInfo;
	snsInfo.dw_pet_id = pCalledSoul->GetID();
	snsInfo.dw_master_id = GetID();
	snsInfo.dw_friend_id = dw_friend_id;
	snsInfo.dw_begin_time = dw_time;
	if (!g_petSnsMgr.insertSNSinfo(&snsInfo))
	{
		print_message(_T("insertSNSinfo fail!!!!!!!"));
	}
	pCalledSoul->SetWorking(TRUE);


	// ��Ŀ�귢�ͱ���ǲ��Ϣ
	NET_SIS_pet_be_paiqian send;
	send.dw_pet_id =snsInfo.dw_pet_id;
	send.dw_friend_id = snsInfo.dw_master_id;
	send.dw_time = snsInfo.dw_begin_time;

	pFriend->SendMessage(&send, send.dw_size);
	
	GetAchievementMgr().UpdateAchievementCriteria(ete_pet_paiqian, 1);	

	return E_Pets_SNS_Success;
}

DWORD Role::ReturnPet(DWORD dw_pet_id, BYTE type, INT& itemNum)
{
	if (type == 0) // ���Լ��ĳ���
	{
		PetSoul* pPsoul = GetPetPocket()->GetPetSoul(dw_pet_id);
		if (!VALID_POINT(pPsoul))
		{
			return E_PetS_SNS_Not_Pet;
		}
		if (!pPsoul->IsWorking())
		{
			return E_PetS_SNS_Not_Work;
		}
			
		tagPetSNSInfo* pPetSNSinfo = g_petSnsMgr.getPetSNSinfo(dw_pet_id);
		if (!VALID_POINT(pPetSNSinfo))
		{
			return E_PetS_SNS_Not_Work;
		}
		tagFriend* pFriend = GetFriendPtr(pPetSNSinfo->dw_friend_id);
		if (!VALID_POINT(pFriend))
		{
			//�ջس���
			pPsoul->SetWorking(FALSE);
			g_petSnsMgr.deleteSNSinfo(dw_pet_id);

			return E_PetS_SNS_Friend_NotExis;
		}

		//�Ƿ����㽱������
		DWORD dwTimeSub = CalcTimeDiff(GetCurrentDWORDTime(), pPetSNSinfo->dw_begin_time);
		if ( dwTimeSub > AttRes::GetInstance()->GetVariableLen().n_pet_return_min_time && dwTimeSub < AttRes::GetInstance()->GetVariableLen().n_pet_return_max_time)
		{
			float fFriendValueParam = social_mgr::getFriendValueLevel(pFriend->dwFriVal) * 0.6f;
			INT nBaseNum = (INT)fFriendValueParam;
			INT nAddNum = get_tool()->probability((fFriendValueParam - nBaseNum) * 100)? 1:0;
			
			itemNum = nBaseNum + nAddNum;

			
			tagMailBase st_mail_base;
			ZeroMemory(&st_mail_base, sizeof(st_mail_base));
			st_mail_base.dwSendRoleID = INVALID_VALUE;
			st_mail_base.dwRecvRoleID = GetID();
				
			TCHAR szPetMailContent[X_SHORT_NAME] = _T("");
			TCHAR szName[X_SHORT_NAME] = _T("");
			s_role_info* pRoleInfo = g_roleMgr.get_role_info(pFriend->dwFriendID);
			if (VALID_POINT(pRoleInfo))
			{	
				_tcsncpy(szName, pRoleInfo->sz_role_name, X_SHORT_NAME);
			}
			_stprintf(szPetMailContent, _T("%d_%s_%d"), PET_JIEJING, szName, 40);

			if (itemNum > 0)
			{
				tagItem* pItem = ItemCreator::Create(EICM_Mail, INVALID_VALUE, PET_JIEJING, itemNum, TRUE);
				g_mailmgr.CreateMail(st_mail_base, _T("&petM&"), szPetMailContent, &pItem,1);
			}
			/*else
			{
				g_mailmgr.CreateMail(st_mail_base, _T("&petM&"), szPetMailContent, NULL,0);
			}*/
		

			
		}

		// ���Է������ջس���
		Role* pTarget = g_roleMgr.get_role(pPetSNSinfo->dw_friend_id);
		if (VALID_POINT(pTarget))
		{
			NET_SIS_pet_be_return send;
			send.dw_pet_id = dw_pet_id;
			send.by_type = 1;

			pTarget->SendMessage(&send, send.dw_size);

		}
		
		// ���Ӻ��Ѷ�
		if (dwTimeSub > AttRes::GetInstance()->GetVariableLen().n_pet_return_max_time)
		{
			pFriend->dwFriVal += 40;
			NET_SIS_update_friend_value send;
			send.dw_role_id = pFriend->dwFriendID;
			send.nFriVal = pFriend->dwFriVal;
			SendMessage(&send, send.dw_size);

			if (VALID_POINT(pTarget))
			{
				tagFriend* pFriendt = pTarget->GetFriendPtr(GetID());
				if (VALID_POINT(pFriendt))
				{
					pFriendt->dwFriVal += 40;

				}
				NET_SIS_update_friend_value send;
				send.dw_role_id = GetID();
				send.nFriVal = pFriend->dwFriVal;
				pTarget->SendMessage(&send, send.dw_size);
			}

			NET_DB2C_update_friend_value sendDB;
			sendDB.dw_role_id = GetID();
			sendDB.s_friendship_save_.dw_friend_id_ = pFriend->dwFriendID;
			sendDB.s_friendship_save_.n_frival_ = pFriend->dwFriVal;
			g_dbSession.Send(&sendDB, sendDB.dw_size);

			NET_DB2C_update_friend_value sendDB2;
			sendDB2.dw_role_id = pFriend->dwFriendID;
			sendDB2.s_friendship_save_.dw_friend_id_ = GetID();
			sendDB2.s_friendship_save_.n_frival_ = pFriend->dwFriVal;
			g_dbSession.Send(&sendDB2, sendDB2.dw_size);

		}


		//�ջس���
		pPsoul->SetWorking(FALSE);
		g_petSnsMgr.deleteSNSinfo(dw_pet_id);

	}
	else if (type == 1) // �պ��ѵĳ���
	{
		tagPetSNSInfo* pPetSNSinfo = g_petSnsMgr.getPetSNSinfo(dw_pet_id);
		if (!VALID_POINT(pPetSNSinfo))
		{
			return E_PetS_SNS_Not_Work;
		}
		if (pPetSNSinfo->dw_friend_id != GetID())
		{
			return E_PetS_SNS_Not_to_You;
		}
		tagFriend* pFriend = GetFriendPtr(pPetSNSinfo->dw_master_id);
		if (!VALID_POINT(pFriend))
		{
			return E_PetS_SNS_Friend_NotExis;
		}

		//�Ƿ����㽱������
		DWORD dwTimeSub = CalcTimeDiff(GetCurrentDWORDTime(), pPetSNSinfo->dw_begin_time);
		if (dwTimeSub < AttRes::GetInstance()->GetVariableLen().n_pet_return_max_time)
		{
			return E_PetS_SNS_Time_Not_Relay;
		}


		// ��ý���
		float fFriendValueParam = social_mgr::getFriendValueLevel(pFriend->dwFriVal) * 0.6f;
		INT nBaseNum = (INT)fFriendValueParam;
		INT nAddNum = get_tool()->probability((fFriendValueParam - nBaseNum) * 100)? 1:0;
		
		itemNum = nBaseNum + nAddNum;
		
		tagMailBase st_mail_base;
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = GetID();

		

		TCHAR szPetMailContent[X_SHORT_NAME] = _T("");
		_stprintf(szPetMailContent, _T("%d"), PET_JIEJING);
		TCHAR szName[X_SHORT_NAME] = _T("");
		s_role_info* pRoleInfo = g_roleMgr.get_role_info(pFriend->dwFriendID);
		if (VALID_POINT(pRoleInfo))
		{	
			_tcsncpy(szName, pRoleInfo->sz_role_name, X_SHORT_NAME);
		}
		_stprintf(szPetMailContent, _T("%d_%s_%d"), PET_JIEJING, szName, 40);

		if (itemNum > 0)
		{
			tagItem* pItem = ItemCreator::Create(EICM_Mail, INVALID_VALUE, PET_JIEJING, itemNum, TRUE);
			g_mailmgr.CreateMail(st_mail_base, _T("&petF&"), szPetMailContent, &pItem,1);
		}
		/*else
		{
			g_mailmgr.CreateMail(st_mail_base, _T("&petF&"), szPetMailContent, NULL,0);	
		}*/


		// ���Է������ջس���
		Role* pMaster = g_roleMgr.get_role(pPetSNSinfo->dw_master_id);
		if (VALID_POINT(pMaster))
		{
			PetSoul* pPetSoul = pMaster->GetPetPocket()->GetPetSoul(dw_pet_id);
			if (VALID_POINT(pPetSoul))
			{
				pPetSoul->SetWorking(FALSE);
			}

			NET_SIS_pet_be_return send;
			send.dw_pet_id = dw_pet_id;
			send.by_type = 0;

			pMaster->SendMessage(&send, send.dw_size);

		}
		
		// ���Ӻ��Ѷ�
		if (dwTimeSub > AttRes::GetInstance()->GetVariableLen().n_pet_return_max_time)
		{
			pFriend->dwFriVal += 40; //��ʱ��40���Ժ��й�ʽ
			NET_SIS_update_friend_value send;
			send.dw_role_id = pFriend->dwFriendID;
			send.nFriVal = pFriend->dwFriVal;
			SendMessage(&send, send.dw_size);

			if (VALID_POINT(pMaster))
			{
				tagFriend* pFriendt = pMaster->GetFriendPtr(GetID());
				if (VALID_POINT(pFriendt))
				{
					pFriendt->dwFriVal += 40;
				}
				NET_SIS_update_friend_value send;
				send.dw_role_id = GetID();
				send.nFriVal = pFriend->dwFriVal;
				pMaster->SendMessage(&send, send.dw_size);
			}

			NET_DB2C_update_friend_value sendDB;
			sendDB.dw_role_id = GetID();
			sendDB.s_friendship_save_.dw_friend_id_ = pFriend->dwFriendID;
			sendDB.s_friendship_save_.n_frival_ = pFriend->dwFriVal;
			g_dbSession.Send(&sendDB, sendDB.dw_size);

			NET_DB2C_update_friend_value sendDB2;
			sendDB2.dw_role_id = pFriend->dwFriendID;
			sendDB2.s_friendship_save_.dw_friend_id_ = GetID();
			sendDB2.s_friendship_save_.n_frival_ = pFriend->dwFriVal;
			g_dbSession.Send(&sendDB2, sendDB2.dw_size);
		}
		
		g_petSnsMgr.deleteSNSinfo(dw_pet_id);

	}
	return E_Pets_SNS_Success;
}

//������ֵ
DWORD Role::BuyLoveValue(INT32 nYuanbao)
{
	if (GetCurMgr().GetBaiBaoYuanBao() < nYuanbao)
		return E_Pets_Buy_LoveValue_not_yuanbao;

	GetCurMgr().DecBaiBaoYuanBao(nYuanbao, elcid_trade_buy_lovevalue);
	ModAttValue(ERA_Love, nYuanbao * YUANBAO_LOVE_VALUE);
	return E_Pets_Success;
}