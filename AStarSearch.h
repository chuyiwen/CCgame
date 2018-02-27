#ifndef ASTARSEARCH_H_INCLUDE
#define ASTARSEARCH_H_INCLUDE
#include "SparseGraph.h"
class c_astar_search
{
private:
	c_sparse_graph* m_graph;
	std::vector<float> m_gcosts; 
	std::vector<float> m_fcosts;
	std::vector<c_sparse_graph::s_nav_edge*> m_shortest_path_tree;
	std::vector<c_sparse_graph::s_nav_edge*> m_search_frontier;
	unsigned int m_source;
	unsigned int m_target;
public:
	c_astar_search(c_sparse_graph* graph_, unsigned int start_, unsigned int end_);
public:
	std::list<unsigned int> get_path()const;
private:
	void _search();
};
#endif