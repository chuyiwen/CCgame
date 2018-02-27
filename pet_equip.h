/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//����װ��

#pragma once

#include "../../common/WorldDefine/container_define.h"
#include "../../common/WorldDefine/pet_equip_define.h"
#include "container_template.h"
#include "time_limit_mgr.h"

class PetSoul;
class PetSkill;
struct tagItem;

const INT PET_EQUIP_UPDATE_TIME = 60 * TICK_PER_SECOND;

typedef	Container<tagItem> PetEquipContainer;

//-------------------------------------------------------------------------
// ����װ����
//-------------------------------------------------------------------------
class PetEquipBar: public PetEquipContainer
{
public:
	//---------------------------------------------------------------------
	// ����������
	//---------------------------------------------------------------------
	PetEquipBar(PetSoul* pSoul);
	~PetEquipBar(){}

	//---------------------------------------------------------------------
	// ���װ��
	//---------------------------------------------------------------------
	tagItem*		GetFromRoleBag(INT64 n64ItemID);
	tagItem*		GetFromPet(INT64 n64ItemID);
	VOID			FillClientEquipData(tagPetEquipMsgInfo* pMsgInfo, tagItem* pItem);
	BOOL			HasEquip();
	BOOL			IsEquipExist(INT64 n64Id);

	//---------------------------------------------------------------------
	// ��װ��жװ
	//---------------------------------------------------------------------
	DWORD			Equip(INT64 n64ItemID, INT8 n8DstPos, BOOL bSend = FALSE);
	DWORD			UnEquip(INT64 n64ItemID, INT16 n16DstPos, BOOL bSend = FALSE);
	DWORD			SwapEquipPos(INT64 n64ItemID, INT8 n8DstPos, BOOL bSend = FALSE);

	//---------------------------------------------------------------------
	// ����
	//---------------------------------------------------------------------
	VOID			AddEquip( tagItem* p2Equip, INT16 n16DstPos, BOOL bOnLoad = FALSE);
	VOID			Update();
	VOID			SaveToDB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);

private:
	//---------------------------------------------------------------------
	// ����װ��Ч��
	//---------------------------------------------------------------------
	VOID			CalcEffect(PetSoul* pSoul, DWORD dwPetEquipTypeID, BOOL bEquip = TRUE, BOOL bSend = FALSE );
	VOID			DisablePetSkill(std::list<PetSkill*> &listSkill);
	VOID			EnablePetSkill(std::list<PetSkill*> &listSkill);

	//---------------------------------------------------------------------
	// ���ݿ����
	//---------------------------------------------------------------------
	VOID			InsertDB( tagItem* p2Equip );
	VOID			RemoveDB( tagItem* p2UnEquip );

	//---------------------------------------------------------------------
	// ����
	//---------------------------------------------------------------------
	const tagPetEquipProto* GetPetEquipProto( tagItem* pItem );

private:
	TimeLimitMgr<INT64>	m_TimeLimitMgr;	// ʱ�޹�����
	PetSoul*		m_pSoul;			// �������
};