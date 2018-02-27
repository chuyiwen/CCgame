
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物消息处理器
#include "StdAfx.h"
#include "player_session.h"

#include "role.h"

#include "../../common/WorldDefine/pet_equip_protocol.h"
#include "../../common/WorldDefine/pet_skill_protocol.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "../../common/WorldDefine/pet_sns_protocol.h"

#include "pet_skill_server_define.h"

#include "role_mgr.h"
#include "pet_skill.h"
#include "pet_pocket.h"
#include "pet.h"
#include "pet_soul.h"

//-------------------------------------------------------------------------------------------------------
// 获取宠物属性
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetPetAttr(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_get_pet_att);
	M_trans_else_ret(pRole, GetOtherInMap(p_receive->dw_role_id), Role, INVALID_VALUE);

	DWORD dwPetIDs[MAX_PETSOUL_NUM] = {0};
	INT nPetNum = 0;
	if (VALID_VALUE(p_receive->dwPetID))
	{
		nPetNum		= 1;
		dwPetIDs[0] = p_receive->dwPetID;
	}
	else
	{
		pRole->GetPetPocket()->GetAllPetID(dwPetIDs, nPetNum);
	}

	BYTE buffer[1000]={0};
	for (INT i=0; i<nPetNum; ++i)
	{
		NET_SIS_get_pet_att* pSend = (NET_SIS_get_pet_att*)buffer;
		NET_SIS_get_pet_att tmp;
		pSend->dw_message_id = tmp.dw_message_id;
		pSend->dw_role_id = p_receive->dw_role_id;
		PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(dwPetIDs[i]);
		if (!VALID_POINT(pSoul))
			continue;

		pSoul->FillClientPetAtt(&pSend->petAttr);
		pSend->dw_size = sizeof(NET_SIS_get_pet_att) - 1 + 
			sizeof(tagPetSkillMsgInfo) * pSend->petAttr.nPetSkillNum + 
			sizeof(tagPetEquipMsgInfo) * pSend->petAttr.nPetEquipNum;

		SendMessage(pSend, pSend->dw_size);
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 使用宠物蛋
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandleUsePetEgg( tag_net_message* pCmd )
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_use_pet_egg);
	NET_SIC_use_pet_egg * p_receive = ( NET_SIC_use_pet_egg * ) pCmd ;  
	Role*		pRole	= GetRole();
	PetPocket*	pPocket = pRole->GetPetPocket();

	DWORD dwRtv = pPocket->HatchEgg(p_receive->n64ItemID, p_receive->szPetName);

	NET_SIS_use_pet_egg send;
	send.dwErrCode = dwRtv;
	send.n64ItemID = p_receive->n64ItemID;
	SendMessage(&send, send.dw_size);
		
	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 删除宠物
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandleDeletePet(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole)) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_delete_pet);
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	PetSoul* pSoul = GetRole()->GetPetPocket()->GetAway(p_receive->dwPetID, TRUE);
	if (VALID_POINT(pSoul))
	{
		PetSoul::DeleteSoul(pSoul, TRUE);
	}
	return 0;
}
//-------------------------------------------------------------------------------------------------------
// 宠物技能处理消息
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetSkill( tag_net_message* pCmd )
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_use_pet_skill);
	NET_SIC_use_pet_skill * p_receive = ( NET_SIC_use_pet_skill * ) pCmd ;  
	// 建立消息缓冲
	BYTE byBuffer[1024] = {0};
	NET_SIS_use_pet_skill send;
	NET_SIS_use_pet_skill* pSend = (NET_SIS_use_pet_skill*)byBuffer;
	*pSend = send;

	// 准备发送的消息
	pSend->dwPetID	 = p_receive->dwPetID;
	pSend->dwSkillID	 = p_receive->dwSkillID;

	// 准备消息处理参数
	DWORD dwDataSize = 0;
	tagPetSkillCmdParam pParam(p_receive->byData, pSend->byData, dwDataSize);

	Role*		pRole	= GetRole();
	PetPocket*	pPocket = pRole->GetPetPocket();
	PetSoul*	pSoul	= pPocket->GetPetSoul(p_receive->dwPetID);
	if (!VALID_POINT(pSoul))
	{
		pSend->dwErrCode	= E_Pets_Soul_NotExist;
	}
	else
	{
		// 处理消息
		pSend->dwErrCode	 = pSoul->HandleSkillCmd(p_receive->dwSkillID, &pParam);
	}
	
	// 处理消息大小
	if (0 != dwDataSize)
	{
		pSend->dw_size = sizeof(NET_SIS_use_pet_skill) - 1 + dwDataSize;
	}

	SendMessage(pSend, pSend->dw_size);

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 获取宠物显示信息
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetPetDispInfo(tag_net_message* pCmd)
{
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
	M_trans_else_ret(p_receive, pCmd, NET_SIC_get_pet_display_info, INVALID_VALUE);

	DWORD dwErr = E_Success;

	NET_SIS_get_pet_display_info send;
	send.dwPetID = p_receive->dwPetID;

	Role* pOther =  GetOtherInMap(p_receive->dw_role_id);
	if (!VALID_POINT(pOther))
	{
		dwErr = E_Pets_Soul_MasterNotFound;
	}
	else
	{
		PetSoul* pSoul = pOther->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
		if (!VALID_POINT(pSoul))
		{
			dwErr = E_Pets_Soul_NotExist;
		}
		else
		{
			pSoul->GetPetAtt().GetName(send.DispData.szName);
			send.DispData.dw_data_id = pSoul->GetPetAtt().GetProtoID();
			send.DispData.n_color = pSoul->GetPetAtt().GetAttVal(epa_color);
			send.DispData.byQuality = pSoul->GetPetAtt().GetAttVal(epa_quality);
		}
	}

	
	SendMessage(&send, send.dw_size);	

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 召唤宠物
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandleSetPetState(tag_net_message* pCmd)
{
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
	M_trans_else_ret(p_receive, pCmd, NET_SIC_set_pet_state, INVALID_VALUE);
	M_trans_else_ret(pPocket, pRole->GetPetPocket(), PetPocket, INVALID_VALUE);

	NET_SIS_set_pet_state send;
	send.dwPetID	= p_receive->dwPetID;
	send.ePetState	= p_receive->ePetState;
	switch(p_receive->ePetState)
	{
	case EPS_Called:
		send.dwErrCode = p_receive->bVal ? pPocket->CallPet(p_receive->dwPetID) : pPocket->RestPet(p_receive->dwPetID);
		break;
// 	case EPS_Preparing:
// 		send.dwErrCode = p_receive->bVal ? pPocket->PreparePet(p_receive->dwPetID) : pPocket->UnPreparePet(p_receive->dwPetID);
// 		break;
// 	case EPS_Mounting:
// 		break;
	default:
		send.dwErrCode = -1;
		break;
	}
	
	SendMessage(&send, send.dw_size);

	return 0;
}
//-------------------------------------------------------------------------------------------------------
// 宠物改名
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetReName(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_rename);
	NET_SIC_pet_rename * p_receive = ( NET_SIC_pet_rename * ) pCmd ;  
	Role*		pRole	= GetRole();
	PetPocket*	pPocket = pRole->GetPetPocket();

	DWORD dwRtv = pPocket->ReName(p_receive->dwPetID, p_receive->szPetName);

	NET_SIS_pet_rename send;
	send.dwErrCode = dwRtv;
	send.dwPetID = p_receive->dwPetID;
	_tcsncpy(send.szPetName,  p_receive->szPetName, X_SHORT_NAME);
	SendMessage(&send, send.dw_size);

	return 0;
}
// 宠物特殊道具
DWORD PlayerSession::HandleUseSpePetItem(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_use_special_pet_item);
	NET_SIC_use_special_pet_item * p_receive = ( NET_SIC_use_special_pet_item * ) pCmd ;  
	Role*		pRole	= GetRole();
	PetPocket*	pPocket = pRole->GetPetPocket();

	DWORD dwRtv = pPocket->SpePetItem(p_receive->n64ItemID);

	NET_SIS_use_special_pet_item send;
	send.dwErrCode = dwRtv;
	send.n64ItemID = p_receive->n64ItemID;
	SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandlePocketSizeChange(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	NET_SIC_pet_pocket_size_change * p_receive = ( NET_SIC_pet_pocket_size_change * ) pCmd ;  
	Role*		pRole	= GetRole();
	PetPocket*	pPocket = pRole->GetPetPocket();

	
	NET_SIS_pet_pocket_size_change sendNS;
	sendNS.dwErrorCode = pPocket->ChangeSize();
	sendNS.n16NewSize = pPocket->GetSize();
	SendMessage(&sendNS, sendNS.dw_size);

	return 0;
}
//-------------------------------------------------------------------------------------------------------
// 宠物换装
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetEquip(tag_net_message* pCmd)
{
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
	M_trans_else_ret(p_receive, pCmd, NET_SIC_pet_equip, INVALID_VALUE);
	
	NET_SIS_pet_equip send;
	send.dwErrCode = E_Success;
	send.dwPetID = p_receive->dwPetID;	

	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (VALID_POINT(pSoul))
	{
		send.dwErrCode = pSoul->Equip(p_receive->n64ItemID, p_receive->n8DstPos, FALSE);
		if (E_Success == send.dwErrCode)
		{
			pSoul->GetEquipInfo(p_receive->n64ItemID, &send.itemData);
		}
	}
	else
	{
		send.dwErrCode = E_Pets_PetEquip_Soul_NotExist;
	}
	
	SendMessage(&send, send.dw_size);
	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 宠物卸装
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetUnEquip(tag_net_message* pCmd)
{
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
	M_trans_else_ret(p_receive, pCmd, NET_SIC_pet_unequip, INVALID_VALUE);

	NET_SIS_pet_unequip send;
	send.dwErrCode = E_Success;
	send.n64ItemID = p_receive->n64ItemID;
	send.dwPetID = p_receive->dwPetID;
 
	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (VALID_POINT(pSoul))
	{
		send.dwErrCode = pSoul->UnEquip(p_receive->n64ItemID, p_receive->n16PosDst, FALSE);
	}

	SendMessage(&send, send.dw_size);
	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 宠物装备换位置
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetSwapEquipPos(tag_net_message* pCmd)
{
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
	M_trans_else_ret(p_receive, pCmd, NET_SIC_pet_equip_position_swap, INVALID_VALUE);

	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (VALID_POINT(pSoul))
	{
		pSoul->EquipSwapPos(p_receive->n64ItemID, p_receive->n8PosDst, TRUE);
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 宠物灌注经验需要money
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetPetPourExpMoneyNeed(tag_net_message* pCmd)
{
	M_trans_pointer(p_receive, pCmd, NET_SIC_get_pet_pour_exp_money_need);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	
	NET_SIS_get_pet_pour_exp_money_need send;
	send.dwPetID		= p_receive->dwPetID;
	send.n64MoneyNeed	= VALID_POINT(pSoul) ? pSoul->GetPetAtt().CalPourMoney() : 0;

	SendMessage(&send, send.dw_size);
	
	return 0;
}


//-------------------------------------------------------------------------------------------------------
// 宠物灌注经验
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetPourExp(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_pour_exp);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	NET_SIS_pet_pour_exp send;
	send.dwPetID	= p_receive->dwPetID;
	
	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (!VALID_POINT(pSoul))
	{
		send.dwErrCode	= E_Pets_Soul_NotExist;
	}
	else
	{
		send.dwErrCode	= pSoul->PourExp(p_receive->n64ItemID);
	}

	SendMessage(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 宠物灌注经验
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetUpStep(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_up_step);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	NET_SIS_pet_up_step send;
	send.n64ItemID	= p_receive->n64ItemID;
	send.dwSkillID	= p_receive->dwSkillID;
	send.nDstStep	= p_receive->nDstStep;
	
	send.dwErrCode	= pRole->GetPetPocket()->CalledSoulUpStep(send.n64ItemID, send.dwSkillID, send.nDstStep);

	if (VALID_VALUE(p_receive->n64ItemID))
	{
		SendMessage(&send, send.dw_size);
	}

	return 0;
}
//-------------------------------------------------------------------------------------------------------
// 宠物资质提升
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetEnhance(tag_net_message* pCmd)
{	
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_enhance);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	NET_SIS_pet_enhance send;
	send.n64ItemID	= p_receive->n64ItemID;
	send.dwErrCode	= pRole->GetPetPocket()->CalledSoulEnhance(p_receive->n64ItemID);
	SendMessage(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 宠物学习技能
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetLearnSkill(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_learn_skill);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	NET_SIS_pet_learn_skill send;
	send.dwPetID		= p_receive->dwPetID;
	send.n64ItemID		= p_receive->n64ItemID;
	
	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (VALID_POINT(pSoul))
	{
		send.dwErrCode	= pSoul->LearnBookSkill(p_receive->n64ItemID);
	}
	else
	{
		send.dwErrCode	= E_Pets_Soul_NotExist;
	}

	SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandlePetLevelUpSkill(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_levelup_skill);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	NET_SIS_pet_levelup_skill send;
	send.dwPetID		= p_receive->dwPetID;
	send.dwSkillID		= p_receive->dwSkillID;

	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (VALID_POINT(pSoul))
	{
		send.dwErrCode	= pSoul->SkillLevelUp(p_receive->dwSkillID);
	}
	else
	{
		send.dwErrCode	= E_Pets_Soul_NotExist;
	}

	SendMessage(&send, send.dw_size);

	return 0;
}
// 宠物遗忘技能
DWORD PlayerSession::HandlePetForgetSkill(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_forget_skill);
	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);

	NET_SIS_pet_forget_skill send;
	send.dwPetID		= p_receive->dwPetID;

	PetSoul* pSoul = pRole->GetPetPocket()->GetPetSoul(p_receive->dwPetID);
	if (VALID_POINT(pSoul))
	{
		send.dwErrCode	= pSoul->ForgetSkill(MTransPetSkillID(p_receive->dwSkillID), TRUE);
	}
	else
	{
		send.dwErrCode	= E_Pets_PetSkill_Forget_Fiel;
	}


	SendMessage(&send, send.dw_size);
	return 0;
}
//-------------------------------------------------------------------------------------------------------
// 邀请驾驭
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetInvite( tag_net_message* pCmd )
{
// 	if (!VALID_POINT(GetRole())) return INVALID_VALUE;
// 
// 	M_trans_pointer(p_receive, pCmd, NET_SIC_mount_invite);
// 	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
// 
// 	DWORD dwRtv = E_Pets_Success;
// 
// 	Role* pDstRole = GetOtherInMap(p_receive->dwDstRoleID);
// 	if (!VALID_POINT(pDstRole))
// 	{
// 		dwRtv = E_Pets_Mount_DstRoleNotFound;
// 	}
// 	else 
// 	{
// 		PetSoul* pSoul = GetRole()->GetPetPocket()->GetMountPetSoul();
// 		if (!VALID_POINT(pSoul))
// 		{
// 			dwRtv = E_Pets_Mount_MountSoulNotFound;
// 		}
// 		else
// 		{
// 			dwRtv = GetRole()->GetPetPocket()->CanAddPassenger(pDstRole);
// 			if(E_Pets_Success == dwRtv)
// 			{
// 				dwRtv = pDstRole->GetPetPocket()->CanSetRideAfter(GetRole(), TRUE);
// 				if (E_Pets_Success == dwRtv)
// 				{
// 					NET_SIS_mount_on_invite send;
// 					send.dwSrcRoleID = GetRole()->GetID();
// 					pDstRole->SendMessage(&send, send.dw_size);
// 				}			
// 			}
// 
// 		}
// 	}
// 	
// 	if (E_Pets_Success != dwRtv)
// 	{
// 		NET_SIS_mount_invite send;
// 		send.dwDstRoleID = p_receive->dwDstRoleID;
// 		send.dwErrCode = dwRtv;
// 		SendMessage(&send, send.dw_size);
// 	}

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 邀请驾驭应答
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetOnInvite(tag_net_message* pCmd)
{
// 	if (!VALID_POINT(GetRole())) return INVALID_VALUE;
// 
// 	M_trans_pointer(p_receive, pCmd, NET_SIC_mount_on_invite);
// 	M_trans_else_ret(pRole, GetRole(), Role, INVALID_VALUE);
// 	M_trans_else_ret(pSrcRole, GetOtherInMap(p_receive->dwSrcRoleID), Role, INVALID_VALUE);
// 
// 	DWORD dwRtv = E_Pets_Success;
// 
// 	dwRtv = p_receive->bReceive ? E_Pets_Success : E_Pets_Mount_DstRoleRefuse;
// 	if (E_Pets_Success != dwRtv)		goto FINISH;
// 
// 	dwRtv = pSrcRole->GetPetPocket()->CanAddPassenger(pRole);
// 	if (E_Pets_Success != dwRtv)		goto FINISH;
// 
// 	dwRtv = pRole->GetPetPocket()->CanSetRideAfter(pSrcRole, TRUE);
// 	if (E_Pets_Success != dwRtv)		goto FINISH;
// 
// 	dwRtv = pRole->GetPetPocket()->RideAfter(pSrcRole);
// 	if (E_Pets_Success != dwRtv)		goto FINISH;
// 
// FINISH:
// 	NET_SIS_mount_invite send;
// 	send.dwDstRoleID = pRole->GetID();
// 	send.dwErrCode = dwRtv;
// 	pSrcRole->SendMessage(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 锁定解锁宠物
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetSetLock(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

 	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_set_lock);
 
 	DWORD dwRtv = E_Pets_Success;

	if (p_receive->bSet)
	{
		dwRtv = GetRole()->GetPetPocket()->LockPet(p_receive->dwPetID, p_receive->n64ItemID);
	}
	else
	{
		dwRtv = GetRole()->GetPetPocket()->UnLockPet(p_receive->dwPetID, p_receive->n64ItemID);
	}

	NET_SIS_pet_set_lock send;
	send.dwPetID = p_receive->dwPetID;
	send.dwErrCode = dwRtv;
	SendMessage(&send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------------------------------
// 宠物食品
//-------------------------------------------------------------------------------------------------------
DWORD PlayerSession::HandlePetFood(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_food);

	NET_SIS_pet_food send;
	send.n64ItemID		= p_receive->n64ItemID;
	send.dwErrCode		= GetRole()->GetPetPocket()->CalledPetFeed(p_receive->n64ItemID, p_receive->dwFoodType);
	
	SendMessage(&send, send.dw_size);

	return 0;
}

// 复活宠物
DWORD PlayerSession::HandleRebornPet(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_reborn_pet);
	
	NET_SIS_reborn_pet send;
	send.dwErrCode = GetRole()->GetPetPocket()->RebornPet(p_receive->dwPetID, p_receive->dw64ItemID);

	SendMessage(&send, send.dw_size);
	return 0;
}

// 派遣宠物
DWORD PlayerSession::HandlePetPaiqian(tag_net_message* pCmd)
{
	
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_paiqian);

	NET_SIS_pet_paiqian send;
	send.dw_pet_id = p_receive->dw_pet_id;
	send.dw_friend_id = p_receive->dw_friend_id;
	send.dw_time = GetCurrentDWORDTime();
	send.dwErrCode = GetRole()->PaiqianPet(p_receive->dw_pet_id, p_receive->dw_friend_id, send.dw_time);

	SendMessage(&send, send.dw_size);

	return 0;
}


// 回收宠物
DWORD PlayerSession::HandlePetReturn(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_return);

	NET_SIS_pet_return send;
	send.dw_pet_id = p_receive->dw_pet_id;
	send.by_type = p_receive->by_type;
	send.dwErrCode = GetRole()->ReturnPet(p_receive->dw_pet_id, p_receive->by_type, send.n_itemNum);
	
	SendMessage(&send, send.dw_size);

	return 0;
}

// 购买爱心值
DWORD PlayerSession::HandleBuyLoveValue(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_buy_love_vaule);
	
	if(!GetRole()->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			GetRole()->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			GetRole()->SendMessage(&send_check, send_check.dw_size);

			GetRole()->set_check_safe_code();
		}
	}

	NET_SIS_buy_love_vaule send;
	send.dwErrCode = GetRole()->BuyLoveValue(p_receive->dw_yuanbao);

	SendMessage(&send, send.dw_size);
	return 0;
}

// 宠物染色
DWORD PlayerSession::HandlePetColor(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_color);

	NET_SIS_pet_color send;
	
	send.dw_error_code = GetRole()->GetPetPocket()->PetColor(p_receive->dwPetID, p_receive->nColor);
	SendMessage(&send, send.dw_size);
	return 0;
}

// 宠物蜕变
DWORD PlayerSession::HandlePetChange(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_change);

	NET_SIS_pet_change send;

	send.dw_error_code = GetRole()->GetPetPocket()->PetChange(p_receive->dwPetID, p_receive->n64ItemID);
	SendMessage(&send, send.dw_size);
	return 0;
}

DWORD PlayerSession::HandlePetaddPoint(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_add_point);

	NET_SIS_pet_add_point send;
	
	for (int i = 0; i < 3; i++)
	{
		if (p_receive->nAttValue[i] != 0)
		{
			send.dw_error_code = GetRole()->GetPetPocket()->PetAddPoint(p_receive->dwPetID, i, p_receive->nAttValue[i]);
		}
	}
	
	SendMessage(&send, send.dw_size);
	return 0;
}

// 宠物融合
DWORD PlayerSession::HandlePetFusion(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;
	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_fusion);


	NET_SIS_pet_fusion send;
	send.dw_error_code = GetRole()->GetPetPocket()->FusionPet(p_receive->dwPetID1, p_receive->dwPetID2, p_receive->dw64ItemID1);
	SendMessage(&send, send.dw_size);
	return 0;
}

// 宠物寄养栏开启
DWORD PlayerSession::HandlePetXiulianSizeChange(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;
	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_xiulian_size_change);
	
	NET_SIS_pet_xiulian_size_change send;
	send.dwErrorCode = GetRole()->PetXiulianSizeChange(p_receive->dwNPCID);
	send.n16NewSize = GetRole()->get_pet_xiulian_size();
	SendMessage(&send, send.dw_size);
	return 0;
}

DWORD PlayerSession::HandlePetXiulian(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;
	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_xiulian);
		
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;

		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	NET_SIS_pet_xiulian send;
	send.dwErrorCode = GetRole()->GetPetPocket()->petXiulian(p_receive->dwPetId, p_receive->dwNPCID, p_receive->dwTimeType, p_receive->dwModeType);
	send.dwPetId = p_receive->dwPetId;
	SendMessage(&send, send.dw_size);
	
	return 0;
}
DWORD PlayerSession::HandlePetXiulianReturn(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole())) return INVALID_VALUE;
	M_trans_pointer(p_receive, pCmd, NET_SIC_pet_xiulian_return);

	NET_SIS_pet_xiulian_return send;
	send.dwErrorCode = GetRole()->GetPetPocket()->petXiulianRetrun(p_receive->dwPetId);
	send.dwPetId = p_receive->dwPetId;
	SendMessage(&send, send.dw_size);

	return 0;
}