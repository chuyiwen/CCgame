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
 *	@file		stall
 *	@author		mwh
 *	@date		2011/02/25	initial
 *	@version	0.0.1.0
 *	@brief		��̯
*/

#include "StdAfx.h"

#include "../../common/WorldDefine/stall_define.h"
#include "../../common/WorldDefine/filter.h"
#include "../../common/WorldDefine/protocol_common_errorcode.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../../common/WorldDefine/stall_protocol.h"

#include "stall.h"
#include "role.h"
#include "item_mgr.h"
#include "item_creator.h"

// ��̯��Ʒ�ṹ
struct local_stall_goods
{
	INT64	n64_serial;			// �����۵���Ʒ&װ��64λID
	INT64	n64UnitPrice;		// ����

	local_stall_goods() { n64_serial = INVALID_VALUE; n64UnitPrice = 0; }
};

//
stall::stall(Role *p_role, const s_role_data_save* p_data)
{
	ASSERT(VALID_POINT(p_role) && VALID_POINT(p_data));

	p_role_			= p_role;
	b_vip_			= FALSE;

	goods_number_	= 0;

	sz_title_		= NULL;
	sz_ad_			= NULL;
	p_goods_	= NULL;
}

stall::~stall()
{
	destroy(FALSE);
}


// ��̯
DWORD stall::init( )
{

	DWORD dw_error_code = can_stall();
	if(dw_error_code != E_Success) return dw_error_code;
	
	goods_number_	= 0;

	// �����ڴ�
	p_goods_ = new local_stall_goods[STALL_MAX_DISPLAY];
	if(!VALID_POINT(p_goods_))return INVALID_VALUE;

	// ����״̬
	p_role_->SetRoleState(ERS_StallSet, FALSE);

	check_vip( );

	return E_Success;
}

// ��̯
DWORD stall::destroy(BOOL bSendMsg)
{
	// �Ƿ���̯
	if(!p_role_->IsInRoleStateAny(ERS_Stall | ERS_StallSet))
		return E_Stall_Role_Pull;
	
	// ���ð�̯״̬
	if(p_role_->IsInRoleState(ERS_Stall))
		p_role_->UnsetRoleState(ERS_Stall, bSendMsg);
	
	if(p_role_->IsInRoleState(ERS_StallSet))
		// ȡ����̯����״̬������Ҫ����Χ��ҷ�����Ϣ
		p_role_->UnsetRoleState(ERS_StallSet, FALSE);

	b_vip_ = FALSE;

	// ������Ʒʹ��״̬
	if(VALID_POINT(p_goods_))
	{
		tagItem *pGoods;
		for(INT i=0; i<STALL_MAX_DISPLAY; ++i)
		{
			if(p_goods_[i].n64_serial != INVALID_VALUE)
			{
				pGoods = p_role_->GetItemMgr().GetBagItem(p_goods_[i].n64_serial);
				if(VALID_POINT(pGoods)) pGoods->SetUsing(FALSE);
			}
		}
	}

	tagStallMessageMem* p_msg = NULL; 
	messages_.reset_iterator( );
	while( messages_.find_next(p_msg))
		SAFE_DELETE( p_msg );
	messages_.clear( );

	goods_number_	= 0;

	// �ͷ��ڴ�
	SAFE_DELETE_ARRAY(sz_title_);
	SAFE_DELETE_ARRAY(sz_ad_ );
	SAFE_DELETE_ARRAY(p_goods_);

	return E_Success;
}

// �㲥���
VOID stall::update()
{
	if (!p_role_->IsInRoleState(ERS_Stall)) return;
}

// ���ñ���
DWORD stall::set_title(LPCTSTR csz_str)
{
	if(!p_role_->IsInRoleState(ERS_StallSet))
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}
	
	INT n_title_len = AttRes::GetInstance()->GetVariableLen().nStallTitleMax + 1;
	
	//gx modify 2013.6.25 �����ڿͻ���ִ�У������������ٴδ���
	/*DWORD dw_error_code = Filter::CheckName(csz_str, n_title_len - 1);
	if(dw_error_code != E_Success) return dw_error_code;*/

	if( !VALID_POINT(sz_title_) ) sz_title_ = new TCHAR[n_title_len];

	ZeroMemory(sz_title_, n_title_len * sizeof(TCHAR));
	_tcsncpy(sz_title_, csz_str, n_title_len - 1);

	return E_Success;
}

// ���
DWORD stall::set_ad( LPCTSTR csz_ad )
{
	if(!p_role_->IsInRoleState(ERS_StallSet))
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}

	DWORD dw_error_code = Filter::CheckName(csz_ad, STALL_AD_CHAR_MAX - 1);
	if(dw_error_code != E_Success) return dw_error_code;


	if( !VALID_POINT(sz_ad_) )
		sz_ad_ = new TCHAR[STALL_AD_CHAR_MAX];

	ZeroMemory( sz_ad_, sizeof(TCHAR)*STALL_AD_CHAR_MAX);
	_tcsncpy( sz_ad_, csz_ad, STALL_AD_CHAR_MAX - 1 );

	return E_Success;
}

// ��Ʒ�ϼ�
DWORD stall::set_goods(const INT64 n64_serial, const INT64 n64_price, const BYTE index)
{
	if(!p_role_->IsInRoleStateAny(ERS_Stall | ERS_StallSet))
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}

	// ��Ǯ�����Ϸ����ж�
	if(n64_price < STALL_MIN_SOLD_SLIVER ||
	   n64_price > 999999)//gx modify
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}
	
	// �±��ж�
	if( !check_index( index) )
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}

	// ��λ�ж�
	if(p_goods_[index].n64_serial != INVALID_VALUE)
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}
	
	// ȡ����Ʒ
	tagItem *p_goods = p_role_->GetItemMgr().GetBagItem(n64_serial);
	if(!VALID_POINT(p_goods)) return E_Stall_ItemNotFound_InBag;

	// �Ƿ��Ѵ���ʹ��״̬
	if(p_goods->bUsing) return E_Stall_Goods_InUsed;

	// �ж���Ʒ�Ƿ���Գ���
	if(!p_role_->GetItemMgr().CanExchange(*p_goods))
		return E_Stall_ItemCanNot_Sold;

	if(!MIsEquipment(p_goods->dw_data_id))
	{
		if(!VALID_POINT(p_goods->pProtoType) || p_goods->nUseTimes)
		{
			return E_Stall_ItemCanNot_Sold;
		}
	}

	// ���ð�̯��Ʒ
	p_goods_[index].n64_serial = n64_serial;
	p_goods_[index].n64UnitPrice = n64_price;

	p_goods->SetUsing(TRUE);

	// ����
	++goods_number_;

	return E_Success;
}

// ��Ʒ�¼�
DWORD stall::unset_goods(const BYTE index)
{
	if(!p_role_->IsInRoleStateAny(ERS_Stall | ERS_StallSet))
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}

	// �±��ж�
	if( !check_index( index ))
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}

	// ��λ�ж�
	if( !VALID_POINT(p_goods_[index].n64_serial) )
		// �Ƿ���Ϣ������Ʒ�Ѿ�����
		return E_Stall_Goods_BeSold;

	// �����Ʒ������ʹ�ñ�־
	tagItem *p_goods = p_role_->GetItemMgr().GetBagItem(p_goods_[index].n64_serial);
	if(VALID_POINT(p_goods)) p_goods->SetUsing(FALSE);
	
	// �¼���Ʒ
	unset(index);

	return E_Success;
}

// ���̯λ����
DWORD stall::get_title(OUT LPTSTR sz_str)
{
	// �Ƿ���̯
	if(!p_role_->IsInRoleState(ERS_Stall))
		return E_Stall_Role_Pull;

	if(VALID_POINT(sz_title_) )
	{
		_tcsncpy(sz_str, sz_title_, STALL_MAX_TITLE_NUM - 1);
		sz_str[STALL_MAX_TITLE_NUM - 1] = _T('\0');
	}
	else sz_str[0] = _T('\0');

	return E_Success;
}

// ���̯λ���
DWORD stall::get_ad(OUT LPTSTR sz_ad)
{
	// �Ƿ���̯
	if(!p_role_->IsInRoleState(ERS_Stall))
		return E_Stall_Role_Pull;

	if(VALID_POINT(sz_ad_) )
	{
		_tcsncpy(sz_ad, sz_ad_, STALL_AD_CHAR_MAX - 1);
		sz_ad[STALL_MAX_TITLE_NUM - 1] = _T('\0');
	}
	else sz_ad[0] = _T('\0');

	return E_Success;
}

// �������
DWORD stall::get_history_chat( Role* p_dest )
{
	if( !VALID_POINT(p_dest) ) return INVALID_VALUE;

	INT32 n_msg_size = sizeof(NET_SIS_stall_history_chat) + messages_.size( ) * StallMessageRealSize(STALL_MESSAGE_CHAR_MAX);
	CREATE_MSG( p_send, n_msg_size,  NET_SIS_stall_history_chat);

	p_send->dwStallRoleID = p_role_->GetID( );
	p_send->byNumber = messages_.size( );
	p_send->dw_size = sizeof(NET_SIS_stall_history_chat) - sizeof(tagStallMessage);

	tagStallMessage* p_start = p_send->stMessage;
	tagStallMessageMem* p_msg = NULL; 
	
	messages_.reset_iterator( );
	while( messages_.find_next(p_msg))
	{
		INT32 n_len = _tcslen(p_msg->cMessage);
		INT32 n_size = StallMessageRealSize( n_len );
		get_fast_code()->memory_copy( p_start->cMessage, p_msg->cMessage, n_len * sizeof(TCHAR));
		p_start->cMessage[n_len] = (TCHAR)0;
		p_start->dwSender = p_msg->dwSender;
		p_start->dw_time = p_msg->dw_time;
		p_start->byCharSize = (n_len + 1) * sizeof(TCHAR);
		p_start = (tagStallMessage*)( (char*)p_start + n_size );
		p_send->dw_size += n_size;
	}

	p_dest->SendMessage(p_send, p_send->dw_size);

	return E_Success;
}
// ���̯λָ��λ����Ʒ
DWORD stall::get_goods(BYTE index, OUT LPVOID p_data, OUT INT &goods_size)
{
	goods_size	= 0;

	if(!VALID_POINT(p_goods_[index].n64_serial) )
		return E_Stall_Goods_BeSold;

	M_trans_pointer(pCur, p_data, tagMsgStallGoods);
	pCur->n64UnitPrice	= p_goods_[index].n64UnitPrice;
	pCur->byIndex		= index;

	tagItem *pGoods = p_role_->GetItemMgr().GetBagItem(p_goods_[index].n64_serial);
	if(!VALID_POINT(pGoods))
	{
		// û���ڱ������ҵ�(���ܱ����ʹ�á�������)
		unset(index);
		return E_Stall_Goods_BeSold;
	}

	if(MIsEquipment(pGoods->dw_data_id))
	{
		pCur->byItem	= 0;
		get_fast_code()->memory_copy(pCur->byData, pGoods, SIZE_EQUIP);
		goods_size = STALLEQUIPSIZE;
	}
	else
	{
		pCur->byItem	= 1;
		get_fast_code()->memory_copy(pCur->byData, pGoods, SIZE_ITEM);
		goods_size = STALLITEMSIZE;
	}

	return E_Success;
}

// ���̯λ������Ʒ
DWORD stall::get_goods(OUT LPVOID p_data, OUT BYTE &goods_num, OUT INT &goods_size)
{
	goods_num	= 0;
	goods_size	= 0;
	
	// �Ƿ���̯
	if(!p_role_->IsInRoleState(ERS_Stall))
		return E_Stall_Role_Pull;

	INT8 n8_number = 0;
	M_trans_pointer(p_start, p_data, BYTE);

	INT good_size_temp = 0;
	for(INT i=0; i<STALL_MAX_DISPLAY; ++i)
	{
		if(get_goods(i, p_start, good_size_temp) != E_Success)
		{
			continue;
		}

		p_start += good_size_temp;
		goods_size += good_size_temp;

		++n8_number;
		if(goods_number_ == n8_number) break;
	}

	goods_num	= goods_number_;
	return E_Success;
}

// ����������
BYTE stall::get_max_index( )
{
	if( b_vip_ ) return STALL_VIP_MAX_DISPLAY;
	return STALL_BASIC_MAX_DISPLAY;
}
// ������Ʒ
DWORD stall::sell_goods(Role *p_role, INT64 n64_price, INT64 n64_serial, BYTE index, INT16 number, OUT INT16 &remain)
{
	// �±��ж�
	if( !check_index( index ) || number <= 0)
	{
		// �Ƿ���Ϣ�Ż�ִ�е��˴�
		ASSERT(0);
		return INVALID_VALUE;
	}
	
	// �Ƿ���̯
	if(!p_role_->IsInRoleState(ERS_Stall))
	{
		return E_Stall_Role_Pull;
	}
	
	// ��λ�ж�
	if(INVALID_VALUE == p_goods_[index].n64_serial)
	{
		return E_Stall_Goods_BeSold;
	}

	// �����Ʒ
	if(n64_serial != p_goods_[index].n64_serial)
	{
		return E_Stall_Goods_Refresh;
	}

	// ����ۼ�
	if(p_goods_[index].n64UnitPrice != n64_price)
	{
		return E_Stall_Goods_Refresh_Price;
	}

	// ��һ�õĽ�Ǯ
	INT64 n64_total		= n64_price * number;

	// ����򷽽�Ǯ
	if(p_role->GetCurMgr().GetBaiBaoYuanBao() < n64_total)//gx modify 2013.6.26 ���Ľ�Ҹĳ�Ԫ��
	{
		return E_Stall_CustomerMoney_NotEnough;
	}

	// ����򷽱���
	if(p_role->GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Stall_CustomerBag_NoFreeSpace;
	}

	tagItem *p_goods = p_role_->GetItemMgr().GetBagItem(n64_serial);
	if(!VALID_POINT(p_goods))
	{
		unset(index);
		return E_Stall_Goods_NotInBag;
	}
	
	// �����Ʒ����
	if(p_goods->n16Num < number)
	{
		return E_Stall_GoodsNum_NotEnough;
	}

	// ���ô�������
	remain = p_goods->n16Num - number;
	
	// ���� -- �۳���Ʒ�ͽ�Ǯ
	DWORD dw_error_code;
	tagItem *p_item = NULL;
	if(number == p_goods->n16Num)
	{
		p_item = p_goods;

		unset(index);
		dw_error_code = p_role_->GetItemMgr().TakeOutFromBag(p_item->n64_serial, elcid_stall_besold, TRUE);
	}
	else
	{
		// �����µĶ���Ʒ
		p_item = ItemCreator::Create(*p_goods, number);
		dw_error_code = p_role_->GetItemMgr().DelFromBag(n64_serial, (DWORD)elcid_stall_besold, number);
	}

	if(dw_error_code != E_Success)
	{
		// ���ϲ��ж�����©����ʱ���ܻ����ڴ�й©
		ASSERT(0);
		return dw_error_code;
	}

	if(!p_role->GetCurMgr().DecBaiBaoYuanBao(n64_total, (DWORD)elcid_stall_buy))
	{
		// �Ƿ�黹��Ʒ//??
		
		// ���ϲ��ж�����©����ʱ���ܻ����ڴ�й©
		ASSERT(0);
		return dw_error_code;
	}

	ASSERT( VALID_POINT(p_item->pProtoType) );
	if( VALID_POINT(p_item->pProtoType) )
	{//���߰�̯�ߣ���Ʒ������Ϣ
		NET_SIS_stall_buy_message send;
		send.dw_role_id = p_role->GetID( );
		send.dw_time = GetCurrentDWORDTime( );
		send.dw_data_id = p_item->pProtoType->dw_data_id;
		send.n16Number	= number;
		send.n64_price = n64_price;//gx add 2013.6.25
		p_role_->SendMessage(&send, send.dw_size );
	}

	// ����ʹ��״̬
	p_item->SetUsing(FALSE);

	// ���� -- �����Ʒ�ͽ�Ǯ
	p_role_->GetCurMgr().IncBaiBaoYuanBao(n64_total, (DWORD)elcid_stall_besold);
	p_role->GetItemMgr().Add2Bag(p_item, (DWORD)elcid_stall_buy, TRUE);
	
	p_role->GetAchievementMgr().UpdateAchievementCriteria(eta_stall_buy, 1);
	return E_Success;
}

// �¼���Ʒ
VOID stall::unset(const BYTE index)
{
	ASSERT(index < STALL_MAX_DISPLAY);
	
	p_goods_[index].n64_serial = INVALID_VALUE;
	p_goods_[index].n64UnitPrice = 0;

	// ����
	--goods_number_;
}

// ��ʼ��̯�����㲥״̬
DWORD stall::set_finish()
{ 
	// �Ƿ��Ѵ����̯״̬
	if(p_role_->IsInRoleState(ERS_Stall)) return INVALID_VALUE;
	
	// �Ƿ��ڰ�̯����״̬
	if(!p_role_->IsInRoleState(ERS_StallSet)) return INVALID_VALUE;

	if(empty()) return E_Stall_Goods_Empty;

	
	// ������� -- �ٴ�ȷ���Ƿ��ڰ�̯����
	if(!p_role_->IsInRoleState(ERS_StallArea))
		return E_Stall_Area_NotAllowed;

	// ��������״̬
	p_role_->SetRoleState(ERS_Stall);
	p_role_->UnsetRoleState(ERS_StallSet, FALSE);
	
	p_role_->GetAchievementMgr().UpdateAchievementCriteria(eta_stall_start, 1);
	return E_Success;
}


// ̯λ������Ʒ��Ҫ������ڴ�
INT32 stall::goods_memory_size() const
{
	return goods_number_ * STALLEQUIPSIZE;
}

// �����ݿ�
VOID stall::save_to_db( s_role_data_save* p_save )
{
	if (!VALID_POINT(p_save))	return;

	p_save->by_stall_level_		= model_level_;
	p_save->n_stall_daily_exp_	= 0;
	p_save->n_stall_cur_exp_	= 0;
	p_save->dw_last_stall_time_	= 0;
}
// ������
VOID stall::save_chat( DWORD dw_sender, LPVOID p_data, size_t len )
{
	tagStallMessageMem* p_msg = NULL;
	if( messages_.size( ) >= STALL_MESSAGE_SAVE)
	{
		p_msg = messages_.front( );
		messages_.erase( p_msg );
	}
	else
		p_msg = new tagStallMessageMem;

	ASSERT( VALID_POINT( p_msg ) );

	if( len > 0 )
		get_fast_code()->memory_copy( p_msg->cMessage,  p_data, len * sizeof(TCHAR) );
	p_msg->cMessage[len] = (TCHAR)0;
	p_msg->dwSender = dw_sender;
	p_msg->dw_time = GetCurrentDWORDTime( );

	messages_.push_back( p_msg );
}

// �ж��Ƿ���԰�̯
DWORD stall::can_stall()
{
	// �Ѵ��ڰ�̯���̯����״̬
	if(p_role_->IsInRoleStateAny(ERS_Stall | ERS_StallSet))
	{
		return E_Stall_Role_InStall;
	}

	// �Ƿ�ս״̬ gx add ˫��״̬������
	if(p_role_->IsInRoleStateAny(ERS_Combat | ERS_PK | ERS_PVP | ERS_Prictice | ERS_Fishing | ERS_Carry | ERS_ComPractice))
	{
		return E_Stall_RoleState_NotPermit;
	}

	// �ȼ�>=35
	if(p_role_->get_level() < STALL_MIN_ROLE_LEVEL)
	{
		return E_Stall_Role_Level_2Low;
	}
	
	// �������
	if(!p_role_->IsInRoleState(ERS_StallArea))
	{
		return E_Stall_Area_NotAllowed;
	}

	return E_Success;
}

// ��֤����
BOOL stall::check_index( BYTE index )
{
	if( b_vip_ ) return index < STALL_VIP_MAX_DISPLAY;
	return index < STALL_BASIC_MAX_DISPLAY;
}

// ��֤VIP
VOID stall::check_vip( )
{
	model_level_ = ESM_Wood;
	b_vip_ = p_role_->GetItemMgr( ).GetBagSameItemCount( STALL_VIP_ITEM_TYPE ) > 0;
	if( b_vip_ ) model_level_ = ESM_GOLD;
}
