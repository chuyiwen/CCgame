
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�������÷Ǹ���ˢ�ֵ�ԭ��


// ÿ��ˢ�ֵ�����������
const INT MAX_CREATURE_PER_SSPAWNPOINT	= 50;

//--------------------------------------------------------------------------------
// ��������ˢ�ֵ�ṹ����ͨ��ͼ��
//--------------------------------------------------------------------------------
struct tagSSpawnPointProto
{
	DWORD				dwSpawnPointID;								// ˢ�ֵ�id
	DWORD				dwTypeIDs[MAX_CREATURE_PER_SSPAWNPOINT];	// ����typeid

	tagSSpawnPointProto() { ZeroMemory(this, sizeof(*this)); }
};
