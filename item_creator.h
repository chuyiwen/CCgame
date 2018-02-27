/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ʒ��װ��������

#pragma once

#include "mutex.h"

struct tagEquip;
struct tagEquipProto;
struct s_ime_effect;
//-----------------------------------------------------------------------------
class ItemCreator
{
	friend class GMCommandMgr;

public:
	ItemCreator();
	~ItemCreator();

public:
	// �������ݿ��ȡ�����ݴ�����Ʒ
	static tagItem* CreateItemByData(PVOID pData);
	static tagEquip* CreateEquipByData(PVOID pData);
	// ����������Ʒ�����µĶ���Ʒ
	static tagItem* Create(const tagItem &item, INT16 n16Num);
	
	// ����δ������Ʒ&װ��
	static tagItem* Create(EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, INT16 n16Num = 1, BOOL bBind = FALSE, 
							DWORD dwCreator = INVALID_VALUE, INT16 n16QltyModPct = 0, INT16 n16QltyModPctEx = 0, INT16 n16PotValPct = 10000);

	// ����δ�����䱦(��Ʒ&װ��)
	static tagItem* CreateTreasure(DWORD dwNameID, EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, INT16 n16Num = 1, 
							DWORD dwCreator = INVALID_VALUE, INT16 n16QltyModPct = 0, INT16 n16QltyModPctEx = 0);

	// ����ָ��Ʒ������Ʒ&װ��
	static tagItem* CreateEx(EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, 
							INT16 n16Num = 1, EItemQuality eQlty = EIQ_Null, BOOL bBind = FALSE, DWORD dwCreator = INVALID_VALUE, 
							const s_ime_effect *pIMEffect = NULL, BOOL bRandAtt = 1)
	{
		tagItem *pNewItem = Create(eCreateMode, dwCreateID, dw_data_id, n16Num, bBind);
		if(!VALID_POINT(pNewItem))
		{
			return NULL;
		}

		if(MIsEquipment(dw_data_id)/* && eQlty > EIQ_Start && eQlty < EIQ_End*/)
		{
			IdentifyEquip((tagEquip*)pNewItem, eQlty, bRandAtt);
		}

		return pNewItem;
	}

	// ����ָ��Ʒ�����䱦(��Ʒ&װ��)
	static tagItem* CreateTreasureEx(DWORD dwNameID, EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, 
		INT16 n16Num = 1, EItemQuality eQlty = EIQ_Null, DWORD dwCreator = INVALID_VALUE, 
		const s_ime_effect *pIMEffect = NULL);


	// ������Ʒװ�����
	static VOID SetItemSerial(INT64 n64Max, INT64 n64Min) { m_n64MaxSerial = n64Max; m_n64MinSerial = n64Min; }

	// ��������Ψһ��(ע��Ҫ����������)
	static VOID CreateItemSerial(INT64 &n64NewSerial);

public:
	static EItemQuality IdentifyEquip(IN OUT tagEquip *pEquip, 
						EItemQuality eQlty = EIQ_Null, BOOL bRandAtt = TRUE);
	static VOID CreateEquipBindAtt(tagEquip* pEquip, FLOAT fParam);
	static VOID RemoveEquipBindAtt(tagEquip* pEquip);
	static VOID ReattEquip(tagEquip* pEquip, BYTE byindex);
	static VOID ReattEquip(tagEquip* pEquip);
	static INT32 GenEquipQlty(DWORD dw_data_id, BYTE byQuality, INT nAddPro, INT nSkillLevel);

	static ERoleAttribute WRA2ERA(WeaponRoleAtt eWeaponRoleAtt);
	static ERoleAttribute ARA2ERA(ArmorRoleAtt eArmorRoleAtt);
	static ERoleAttribute DRA2ERA(DecorationRoleAtt	eDecorationRoleAtt);
	static ERoleAttribute SPA2ERA(ShiPinRoleAtt	eShipinRoleAtt);

	static WeaponRoleAtt		ERA2WRA(ERoleAttribute eWeaponRoleAtt);
	static ArmorRoleAtt			ERA2ARA(ERoleAttribute eArmorRoleAtt);
	static DecorationRoleAtt	ERA2DRA(ERoleAttribute	eDecorationRoleAtt);
	static ShiPinRoleAtt		ERA2SPA(ERoleAttribute	eRoleAtt);

	static EquipAddAtt ERA2EAA(ERoleAttribute eRoleAttribut);
	static ERoleAttribute EAA2ERA(EquipAddAtt eRoleAttribut);

	static INT GetAttParam(ERoleAttribute eAtt);

private: 	
	// �����Ϸ������ϲ���ú�����֤
	static VOID InitItem(tagItem &item, EItemCreateMode eCreateMode, const tagItemProto *pProto, DWORD dwCreateID, INT64 n64_serial, INT16 n16Num, DWORD dwCreator, DWORD dwCreateTime);
	static VOID InitEquipSpec(tagEquipSpec &equipSpec, const tagEquipProto* pProto, BOOL bHoldNum = TRUE);

	static INT32 GenBaseEquipQlty(DWORD dw_data_id);
	//static INT32 ModEquipQltyByProduce(const tagEquip *pEquip, INT32 nQuality);

	
	static VOID CreateEquipQltyRel(OUT tagEquip *pEquip, const tagEquipProto *pEquipProto, EItemQuality eQlty, BOOL bRandAtt);
	static BOOL CreateFashionQltyRel(OUT tagEquip *pEquip, const tagEquipProto *pEquipProto, 
									EItemQuality eQlty, const s_ime_effect *pIMEffect = NULL);
	
	

	//static VOID ProcEquipAttBySpecAtt(tagEquip *pEquip);

	static BOOL IsGMItemNoInit(tagItem* pTmpItem);
	static BOOL InitGMItem(tagItem* pTmpItem);

	static VOID	 SetWeaponAtt(std::vector<int>& vec);
	static VOID	 SetDecorationAtt(std::vector<int>& vec);
	static VOID	 SetArmorAtt(std::vector<int>& vec);

	static VOID	 RemoveAtt(std::vector<int>& vec, int nType);
private:
	static INT64			m_n64MaxSerial;
	static INT64			m_n64MinSerial;
	static Mutex			m_Mutex;
};