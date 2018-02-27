#ifndef COSTPRIORITYQ_H_INCLUDE
#define COSTPRIORITYQ_H_INCLUDE
#include <vector>
class c_cost_priorityq
{
private:
	std::vector<float>& m_keys;
	std::vector<unsigned int> m_heap;
	std::vector<unsigned int> m_inv_heap;
	unsigned int m_size;
	unsigned int m_max_size;
public:
	c_cost_priorityq(std::vector<float>& keys_, unsigned int maxsz_);
public:
	inline bool empty() const {return (m_size==0);}
public:
	void insert(unsigned int idx_);
	unsigned int pop();
	void change_priority(unsigned int idx_);
private:
	void _swap(unsigned int a_, unsigned int b_);
	void _reorder_upwards(unsigned int nd_);
	void _reorder_downwards(unsigned int nd_, unsigned int heapsz_);
};
#endif