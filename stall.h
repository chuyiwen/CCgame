/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/
#ifndef __STALL_H__

#define __STALL_H__

/**
 *	@file		stall
 *	@author		mwh
 *	@date		2011/02/25	initial
 *	@version	0.0.1.0
 *	@brief		摆摊
*/
class Role;

struct s_role_data_save;
struct local_stall_goods;
struct tagStallChatMessage;

class stall
{
public:
	stall(Role *p_role, const s_role_data_save* p_data);
	~stall();

public:
	DWORD init( );
	DWORD destroy(BOOL bSendMsg = TRUE);
	VOID  update();
public:
	DWORD set_title(LPCTSTR csz_str);
	DWORD set_ad( LPCTSTR csz_ad );
	DWORD set_goods(const INT64 n64_serial, const INT64 n64_price, const BYTE index);
	DWORD unset_goods(const BYTE index);
	DWORD set_finish();
public:
	DWORD get_title(OUT LPTSTR sz_str);
	DWORD get_ad(OUT LPTSTR sz_ad);
	DWORD get_history_chat(Role* p_dest);
	DWORD get_goods(BYTE index, OUT LPVOID p_data, OUT INT &goods_size);
	DWORD get_goods(OUT LPVOID p_data, OUT BYTE &goods_num, OUT INT &goods_size);
	DWORD sell_goods(Role *p_role, INT64 n64_price, INT64 n64_serial, BYTE index, INT16 number, OUT INT16 &remain);
	INT32 goods_memory_size() const;
	BYTE  get_modelevel()	const { return model_level_; }
	BYTE get_max_index( );

public:
	DWORD gain_exp(INT32 nExp){ return 0; };
	BOOL  empty() const { return 0 >= goods_number_; }
	VOID check_vip( );
public:
	VOID  save_to_db(s_role_data_save* p_save);
	VOID  save_chat( DWORD dw_sender, LPVOID p_data, size_t len );
private:
	DWORD can_stall();
	VOID  unset(const BYTE index);
	BOOL  check_index( BYTE byIndex );
private:

	// 摊主
 	Role *p_role_;
	// 摊位模型等级
	BYTE model_level_;	
	// VIP
	bool b_vip_;

	// 摊位商品数
	INT8 goods_number_;	
	// 标题
	TCHAR *sz_title_;	
	// 广告
	TCHAR *sz_ad_;			

	// 摊位商品
	local_stall_goods	*p_goods_;		

	// 向客户端传输时的物品结构大小
	// tagMsgStallGoods为tagItem数据时
	static const INT STALLITEMSIZE = sizeof(tagMsgStallGoods) - 1 + SIZE_ITEM;		
	// tagMsgStallGoods为tagEquip
	static const INT STALLEQUIPSIZE = sizeof(tagMsgStallGoods) - 1 + SIZE_EQUIP;		

	package_list<tagStallMessageMem*> messages_;
	typedef package_list<tagStallMessageMem*>::list_iter SMSGITER;
};

#endif //__STALL_H__