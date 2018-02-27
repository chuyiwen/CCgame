#include "stdafx.h"
#include "pet_sns_mgr.h"
#include "../common/ServerDefine/pet_server_define.h"
#include "db_session.h"
#include "role_mgr.h"

Pet_Sns_mgr g_petSnsMgr;


VOID Pet_Sns_mgr::initList(tagPetSNSInfo* pInfo, int nNum)
{
	if (!VALID_POINT(pInfo))
		return;

	pet_sns_info.clear();
	for (int i = 0; i < nNum; i++)
	{
		tagPetSNSInfo* p_new = new tagPetSNSInfo;
		*p_new = pInfo[i];

		pet_sns_info.add(p_new->dw_pet_id, p_new);
	}
}
VOID Pet_Sns_mgr::destory()
{
	// 锁住
	//mutex_.Acquire();

	tagPetSNSInfo* info = NULL;
	pet_sns_info.reset_iterator();
	while(pet_sns_info.find_next(info))
	{
		pet_sns_info.erase(info->dw_pet_id);
		SAFE_DELETE(info);
	}

	// 解锁
	//mutex_.Release();
}

BOOL Pet_Sns_mgr::insertSNSinfo(tagPetSNSInfo* pInfo)
{
	
	if (!pet_sns_info.is_exist(pInfo->dw_pet_id))
	{
		tagPetSNSInfo* p_new = new tagPetSNSInfo;
		if( !VALID_POINT(p_new) ) return FALSE;

		get_fast_code()->memory_copy(p_new, pInfo, sizeof(tagPetSNSInfo));
		
		//mutex_.Acquire();
		pet_sns_info.add(p_new->dw_pet_id, p_new);
		//mutex_.Release();

		// 发送给db
		NET_DB2C_insert_pet_sns send;

		send.pet_sns_info = *pInfo;
		g_dbSession.Send(&send, send.dw_size);
	}

	
	return TRUE;
}

BOOL Pet_Sns_mgr::deleteSNSinfo(const DWORD dw_pet_id)
{

	//mutex_.Acquire();
	tagPetSNSInfo* p_info = pet_sns_info.find(dw_pet_id);
	if( VALID_POINT(p_info) )
	{
		
		pet_sns_info.erase(dw_pet_id);
	
		SAFE_DELETE(p_info);

		// 发送给db
		NET_DB2C_delete_pet_sns send;

		send.dw_pet_id = dw_pet_id;
		g_dbSession.Send(&send, send.dw_size);

	}
	//mutex_.Release();
	return TRUE;
}
BOOL Pet_Sns_mgr::IsFriendHasPaiqianPet(DWORD dw_master_id, DWORD dw_friend_id)
{
	tagPetSNSInfo* info = NULL;
	pet_sns_info.reset_iterator();
	while(pet_sns_info.find_next(info))
	{
		if (!VALID_POINT(info) && dw_master_id == info->dw_master_id && dw_friend_id == info->dw_friend_id)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//是否有派遣
BOOL Pet_Sns_mgr::IsPetPaiQian(DWORD dw_pet_id)
{
	return pet_sns_info.is_exist(dw_pet_id);
}

tagPetSNSInfo* Pet_Sns_mgr::getPetSNSinfo(DWORD dw_pet_id)
{
	tagPetSNSInfo* pInfo = pet_sns_info.find(dw_pet_id);
	return pInfo;
}

VOID	Pet_Sns_mgr::getPetSNSinfoByMasterID(std::vector<tagPetSNSInfo*>& vec, DWORD masterID)
{
	mutex_.Acquire();
	tagPetSNSInfo* info = NULL;
	pet_sns_info.reset_iterator();
	while(pet_sns_info.find_next(info))
	{
		s_role_info* pRoleInfo = g_roleMgr.get_role_info(masterID);
		if (VALID_POINT(info) && VALID_POINT(pRoleInfo) && masterID == info->dw_master_id)
		{
			vec.push_back(info);
		}

		// 角色已经不存在的,删掉sns信息
		if (!VALID_POINT(pRoleInfo))
		{
			NET_DB2C_delete_pet_sns send;

			send.dw_pet_id = info->dw_pet_id;
			g_dbSession.Send(&send, send.dw_size);
		}
	}
	mutex_.Release();
}
VOID	Pet_Sns_mgr::getPetSNSinfoByFriendID(std::vector<tagPetSNSInfo*>& vec, DWORD friendID)
{
	mutex_.Acquire();
	tagPetSNSInfo* info = NULL;
	pet_sns_info.reset_iterator();
	while(pet_sns_info.find_next(info))
	{
		s_role_info* pRoleInfo = g_roleMgr.get_role_info(friendID);
		if (VALID_POINT(info) && VALID_POINT(pRoleInfo) && friendID == info->dw_friend_id)
		{
			vec.push_back(info);
		}
		// 角色已经不存在的,删掉sns信息
		if (!VALID_POINT(pRoleInfo))
		{
			NET_DB2C_delete_pet_sns send;

			send.dw_pet_id = info->dw_pet_id;
			g_dbSession.Send(&send, send.dw_size);
		}
	}
	mutex_.Release();
}