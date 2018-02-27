/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

// 彩票机

#ifndef _LOTTERY_H_
#define _LOTTERY_H_

#include <map>
#include "..\common\WorldDefine\gp_mall_define.h"

// 彩票种类
enum E_LOTTERY_TYPE
{
	ELT_A,//A代表玩家可免费进行的抽奖
	ELT_B,//B代表需消耗道具或是元宝（非绑定的）的抽奖

	ELT_NUM
};

#define LOTTERY_COUNT 2

class Role;
struct s_lottery_save;
class lottery
{
public:
	lottery();
	~lottery() {}
	
	BOOL Init();

	void reset(E_LOTTERY_TYPE elt);

	//gx modify 2013.6.3 
	DWORD getLottery(Role* pRole, BYTE& byType, DWORD& dwItme,BYTE& byNum);
	//end

	VOID sendLotteryNum(Role* pRole);

	//const std::list<tagPrizeRole>& getPrizeRoleList(BYTE byType) { return m_listPrizeRole[byType]; }

	VOID	Log(Role* pRole, DWORD dwItemID, DWORD dwType, DWORD dwIndex);

	VOID	ChangeNumber(DWORD dwType, DWORD dwNumber);
private:
	//std::map<DWORD,DWORD> m_mapLottery[LOTTERY_COUNT];
	std::map<DWORD,tagLotteryProto*> m_mapLottery[LOTTERY_COUNT];//gx modify 2013.6.26
	DWORD dwMaxNumber[LOTTERY_COUNT];

	//DWORD dwPrizeItem[2][LOTTERY_COUNT];
	std::map<DWORD,tagLotteryProto*> m_mapFristPrize;//记录付费抽奖中的天命大奖

	// 头奖获得者
	//std::list<tagPrizeRole> m_listPrizeRole[LOTTERY_COUNT];

};

extern lottery	g_lottery;
#endif