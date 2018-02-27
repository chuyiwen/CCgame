#ifndef SPARSEGRAPH_H_INCLUDE
#define SPARSEGRAPH_H_INCLUDE
#include "Requisites.h"
#include <vector>
#include <list>
#include <math.h>
struct pathNode
{
	float _x;
	float _y;
	pathNode()
	{
		_x = 0.f;
		_y = 0.f;
	}
	pathNode(float x_ , float y_)
	{
		_x = x_;
		_y = y_;
	}
	float x() const {return _x;}
	float y() const {return _y;}
	float& x() {return _x;}
	float& y() {return _y;}
};
enum e_brush_type
{
	BT_NORMAL = 0,
	BT_OBSTACLE = 1,
	BT_WATER = 2,
	BT_MUD = 3
};


inline float pt_distance( const pathNode& p1 , const pathNode& p2)
{
	return sqrt((p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y()));
}
class c_sparse_graph
{
	friend class c_astar_search;
public:
	struct s_nav_node
	{
		pathNode pos;
		unsigned int index;
	};
	struct s_nav_edge
	{
		unsigned int start;
		unsigned int end;
		float cost;
	};
private:
	std::vector<std::list<s_nav_edge> > m_edges;
	std::vector<s_nav_node> m_nodes;
	unsigned int m_next_node_index;
	unsigned int m_num_cellx;
	unsigned int m_num_celly;
	float m_cellw;
	float m_cellh;
	unsigned int m_start;
	unsigned int m_end;
public:
	c_sparse_graph(unsigned int width_,unsigned int height_, unsigned int numcellx_, unsigned int numcelly_);
public:
	void brush(unsigned int x_ , unsigned int y_, e_brush_type bt_);
	bool go(const pathNode& st_, const pathNode& et_, std::list<pathNode>& path);
	unsigned int num_nodes() const;
	unsigned int num_edges() const;
	const c_sparse_graph::s_nav_node& get_node(unsigned int idx_) const;
	c_sparse_graph::s_nav_node& get_node(unsigned int idx_);
	const c_sparse_graph::s_nav_edge& get_edge(unsigned int from_, unsigned int to_) const;
	c_sparse_graph::s_nav_edge& get_edge(unsigned int s_, unsigned int e_);
	unsigned int add_node(const s_nav_node& node_);
	void remove_node(unsigned int node_);
	void add_edge(const s_nav_edge& edge_);
	void remove_edge(unsigned int s_, unsigned int e_);
	bool can_walk_between(const pathNode& p1, const pathNode& p2, pathNode& out);
	void smooth_path(float curx, float cury, std::list<pathNode>& path);
private:
	bool _is_node_present(unsigned int nd_) const;
	bool _is_edge_present(unsigned int s_, unsigned int e_) const;
	bool _unique_edge(unsigned int s_, unsigned int e_)const;
	void _add_neighbours(unsigned int row_, unsigned int col_, unsigned int numcellx_, unsigned int numcelly_);
	void _weight_edges(unsigned int nd_, float weight_);
	void _set_edge_cost(unsigned int s_, unsigned int e_, float cost_);
	unsigned int _find_nearest_pos(const pathNode& st_, const pathNode& et_) const;
	void _smooth_path(std::list<unsigned int>& path_);
	bool _can_walk_between(unsigned int p1_, unsigned int p2_);
};
#endif