#include "StdAfx.h"
#include "assert.h"
#include "CostPriorityQ.h"
c_cost_priorityq::c_cost_priorityq(std::vector<float>& keys_, unsigned int maxsz_) 
	:m_keys(keys_),
	m_max_size(maxsz_),
	m_size(0)
{
	//for(unsigned int i = 0 ; i < maxsz_ + 1 ; ++i)
	//{
	//	m_heap.push_back(0);
	//	m_inv_heap.push_back(0);
	//}
	m_heap.resize(maxsz_ + 1, 0);
	m_inv_heap.resize(maxsz_ + 1, 0);
}
//--------------------------------------------------------
void c_cost_priorityq::insert(unsigned int idx_)
{
	assert(m_size+1 <= m_max_size);
	++m_size;
	m_heap[m_size] = idx_;
	m_inv_heap[idx_] = m_size;
	_reorder_upwards(m_size);
}
//--------------------------------------------------------
unsigned int c_cost_priorityq::pop()
{
	_swap(1, m_size);
	_reorder_downwards(1, m_size-1);
	return m_heap[m_size--];
}
//--------------------------------------------------------
void c_cost_priorityq::change_priority(unsigned int idx_)
{
	_reorder_upwards(m_inv_heap[idx_]);
}
//--------------------------------------------------------
void c_cost_priorityq::_swap(unsigned int a_, unsigned int b_)
{
	unsigned int temp = m_heap[a_]; m_heap[a_] = m_heap[b_]; m_heap[b_] = temp;
	m_inv_heap[m_heap[a_]] = a_; m_inv_heap[m_heap[b_]] = b_;
}
//--------------------------------------------------------
void c_cost_priorityq::_reorder_upwards(unsigned int nd_)
{
	while((nd_>1) && (m_keys[m_heap[nd_/2]] > m_keys[m_heap[nd_]]))
	{      
		_swap(nd_/2, nd_);
		nd_ /= 2;
	}
}
//--------------------------------------------------------
void c_cost_priorityq::_reorder_downwards(unsigned int nd_, unsigned int heapsz_)
{
	while (2*nd_ <= heapsz_)
	{
		unsigned int child = 2 * nd_;
		if ((child < heapsz_) && (m_keys[m_heap[child]] > m_keys[m_heap[child+1]]))
			++child;
		if (m_keys[m_heap[nd_]] > m_keys[m_heap[child]])
		{
			_swap(child, nd_);
			nd_ = child;
		}
		else
		{
			break;
		}
	}
}