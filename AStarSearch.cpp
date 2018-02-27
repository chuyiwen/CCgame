#include "StdAfx.h"
#include "CostPriorityQ.h"
#include "AStarSearch.h"
c_astar_search::c_astar_search(c_sparse_graph* graph_, unsigned int start_, unsigned int end_)
	:m_graph(graph_),                 
	m_source(start_),
	m_target(end_)
{
	m_shortest_path_tree.resize(graph_->num_nodes());
	m_search_frontier.resize(graph_->num_nodes());
	//for(unsigned int i = 0 ; i < graph_->num_nodes() ; ++i)
	//{
	//	m_gcosts.push_back(0.0);
	//	m_fcosts.push_back(0.0);
	//}
	m_gcosts.resize(graph_->num_nodes(), 0.0);
	m_fcosts.resize(graph_->num_nodes(), 0.0);
	_search();   
}
//--------------------------------------------------------
std::list<unsigned int> c_astar_search::get_path() const
{
	std::list<unsigned int> path;
	if (m_target < 0)
		return path;
	unsigned int nd = m_target;
	path.push_front(nd);	
	while ((nd != m_source) && (m_shortest_path_tree[nd] != 0))
	{
		nd = m_shortest_path_tree[nd]->start;
		path.push_front(nd);
	}
	path.pop_front();
	return path;
}
//--------------------------------------------------------
void c_astar_search::_search()
{
	c_cost_priorityq pq(m_fcosts, m_graph->num_nodes());
	pq.insert(m_source);
	while(!pq.empty())
	{
		unsigned int nextclosestnode = pq.pop();
		m_shortest_path_tree[nextclosestnode] = m_search_frontier[nextclosestnode];
		if (nextclosestnode == m_target) 
			return;
		for(std::list<c_sparse_graph::s_nav_edge>::iterator pE = m_graph->m_edges[nextclosestnode].begin() ; 
			pE!= m_graph->m_edges[nextclosestnode].end() ; ++pE)
		{          
			float hcost = pt_distance(m_graph->get_node(m_target).pos, m_graph->get_node(pE->end).pos);
			float gcost = m_gcosts[nextclosestnode] + pE->cost;
			if (m_search_frontier[pE->end] == NULL)
			{
				m_fcosts[pE->end] = gcost + hcost;
				m_gcosts[pE->end] = gcost;
				pq.insert(pE->end);
				m_search_frontier[pE->end] = &(*pE);
			}
			else if ((gcost < m_gcosts[pE->end]) && (m_shortest_path_tree[pE->end]==NULL))
			{
				m_fcosts[pE->end] = gcost + hcost;
				m_gcosts[pE->end] = gcost;
				pq.change_priority(pE->end);
				m_search_frontier[pE->end] = &(*pE);
			}
		}
	}
}