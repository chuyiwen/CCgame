/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
/**
*	@file		pet.h
*	@author		lc
*	@date		2010/6/30	initial
*	@version	0.0.1.0
*	@brief		宠物合体
*/

#ifndef _PET_HETI_H_
#define _PET_HETI_H_

#include "role.h"
#include "pet_pocket.h"
#include "pet_soul.h"


class PetHeti
{
public:

	//取消某个玩家的合体状态
	static VOID cancelHeti(Role* pTarget)
	{
		DWORD dwTargetPetID; 
		Role* pTargetPetMaster = pTarget->GetTargetPet(dwTargetPetID);

		pTarget->SetTargetPet(NULL, INVALID_VALUE);

		if (VALID_POINT(pTargetPetMaster))
		{
			PetSoul* pPetSoul = pTargetPetMaster->GetPetPocket()->GetPetSoul(dwTargetPetID);
			if (VALID_POINT(pPetSoul))
			{
				pPetSoul->SetHeti(FALSE, NULL);
			}
		}
		
	}
	
	//取消某个玩家所有宠物的合体状态
	static VOID cancelBeHeTiPet(Role* pMaster)
	{
		PetPocket* pPetPocket = pMaster->GetPetPocket();

		DWORD dwPetID[MAX_PETSOUL_NUM];
		INT nPetNum;
		pPetPocket->GetAllPetID(dwPetID, nPetNum);
		for (int i = 0; i < nPetNum; i++)
		{
			PetSoul* pPetSoul = pPetPocket->GetPetSoul(dwPetID[i]);
			if (VALID_POINT(pPetSoul))
			{
				Role* pTargetTeti = pPetSoul->GetHeti();
				if (pTargetTeti == pMaster)
					continue;

				if (VALID_POINT(pTargetTeti))
				{
					pTargetTeti->RemoveBuff(HETI_BUFF, TRUE);
				}
				//pPetSoul->SetHeti(FALSE, NULL);
			}
			
		}
	}
};
#endif