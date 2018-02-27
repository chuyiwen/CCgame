#include "StdAfx.h"
#include "assert.h"
#include "AStarSearch.h"
#include "SparseGraph.h"

const unsigned int INVALID_NODE_INDEX = 0xffffffff;

c_sparse_graph::c_sparse_graph(unsigned int width_,unsigned int height_, unsigned int numcellx_, unsigned int numcelly_)
	: m_next_node_index(0) ,
	m_cellw((float)width_ / (float)m_num_cellx) , 
	m_cellh((float)height_ / (float)m_num_celly) , 
	m_num_cellx(numcellx_) , 
	m_num_celly(numcelly_) , 
	m_start(0) , 
	m_end(0)
{
	float midx = m_cellw/2;
	float midy = m_cellh/2;
	for (unsigned int row=0; row<m_num_celly; ++row)
	{
		for (unsigned int col=0; col<m_num_cellx; ++col)
		{
			s_nav_node n;
			n.index = m_next_node_index;
			n.pos = pathNode(midx + (col*m_cellw),midy + (row*m_cellh));
			add_node(n);
		}
	}
	for (unsigned int row=0; row<m_num_celly; ++row)
	{
		for (unsigned int col=0; col<m_num_cellx; ++col)
		{
			_add_neighbours(row, col, numcellx_, numcelly_);
		}
	}
}
//--------------------------------------------------------
void c_sparse_graph::brush(unsigned int x , unsigned int y, e_brush_type bt_)
{
	unsigned int idx = y*m_num_cellx+x;
	if (bt_ == BT_OBSTACLE)
		remove_node(idx);
	else
	{
		if (!_is_node_present(idx))
		{
			unsigned int y = idx / m_num_celly;
			unsigned int x = idx - (y*m_num_celly);
			s_nav_node nd;
			nd.pos = pathNode(x*m_cellw + m_cellw/2.f, y*m_cellh+m_cellh/2.f);
			nd.index = idx;
			_add_neighbours(y, x, m_num_cellx, m_num_celly);
		}
		float cost;
		switch(bt_)
		{
		case BT_NORMAL:cost = 1.f;break;
		case BT_MUD:cost = 1.5f;break;
		case BT_WATER:cost = 2.f;break;
		default:assert(0);cost = 0.f;break;
		}
		_weight_edges(idx, cost);                            
	}
}
//--------------------------------------------------------
bool c_sparse_graph::go(const pathNode& st_, const pathNode& et_, std::list<pathNode>& path)
{
	unsigned int sx = (unsigned int)((float)(st_.x())/m_cellw);  
	unsigned int sy = (unsigned int)((float)(st_.y())/m_cellh); 
	unsigned int ex = (unsigned int)((float)(et_.x())/m_cellw);  
	unsigned int ey = (unsigned int)((float)(et_.y())/m_cellh); 
	unsigned int start = sy*m_num_cellx+sx;
	unsigned int end = ey*m_num_cellx+ex; 
	if(get_node(end).index == INVALID_NODE_INDEX)
		end = _find_nearest_pos(st_, et_);

	if(end == INVALID_NODE_INDEX) 
		return false;
	
	c_astar_search as(this , start , end);
	std::list<unsigned int> lst = as.get_path();
	_smooth_path(lst);
	FOREACH(TYPEOF(std::list<unsigned int>)::const_iterator, iter, lst)
	{			
		path.push_back(get_node(*iter).pos);
	}
	
	return true;
}
//--------------------------------------------------------
unsigned int c_sparse_graph::num_nodes() const
{
	return m_nodes.size();
}
//--------------------------------------------------------
unsigned int c_sparse_graph::num_edges() const
{
	unsigned int tot = 0;
	FOREACH(TYPEOF(std::vector<std::list<s_nav_edge> >)::const_iterator , iter , m_edges)
		tot += iter->size();
	return tot;
}
//--------------------------------------------------------
const c_sparse_graph::s_nav_node& c_sparse_graph::get_node(unsigned int idx_) const
{
	assert((idx_<m_nodes.size()) &&(idx_ >=0));
	return m_nodes[idx_];
}
//--------------------------------------------------------
c_sparse_graph::s_nav_node& c_sparse_graph::get_node(unsigned int idx_)
{
	assert((idx_ < m_nodes.size()) &&(idx_ >=0));
	return m_nodes[idx_];
}
//--------------------------------------------------------
const c_sparse_graph::s_nav_edge& c_sparse_graph::get_edge(unsigned int s_, unsigned int e_) const
{
	assert((s_ < m_nodes.size()) && (s_ >=0) && m_nodes[s_].index != INVALID_NODE_INDEX);
	assert((e_ < m_nodes.size()) && (e_ >=0) && m_nodes[e_].index != INVALID_NODE_INDEX);
	FOREACH(TYPEOF(std::list<s_nav_edge>)::const_iterator , iter , m_edges[s_])
	{
		if (iter->end == e_) 
			return *iter;
	}
	assert(0);
	return m_edges.front().front();
}
//--------------------------------------------------------
c_sparse_graph::s_nav_edge& c_sparse_graph::get_edge(unsigned int s_, unsigned int e_)
{
	assert((s_ < m_nodes.size()) && (s_ >=0) && m_nodes[s_].index != INVALID_NODE_INDEX);
	assert((e_ < m_nodes.size()) && (e_ >=0) && m_nodes[e_].index != INVALID_NODE_INDEX);
	FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , iter, m_edges[s_])
	{
		if (iter->end == e_)
			return *iter;
	}
	assert(0);
	return m_edges.front().front();
}
//--------------------------------------------------------
unsigned int c_sparse_graph::add_node(const c_sparse_graph::s_nav_node& node_)
{
	if (node_.index < m_nodes.size())
	{
		assert(m_nodes[node_.index].index == INVALID_NODE_INDEX);
		m_nodes[node_.index] = node_;
		return m_next_node_index;
	}
	else
	{
		assert(node_.index == m_next_node_index);
		m_nodes.push_back(node_);
		m_edges.push_back(std::list<s_nav_edge>());
		return m_next_node_index++;
	}
}
//--------------------------------------------------------
void c_sparse_graph::remove_node(unsigned int node_)                                   
{
	assert(node_ < m_nodes.size());
	m_nodes[node_].index = INVALID_NODE_INDEX;
	FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , curedge , m_edges[node_])
	{
		FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , iter , m_edges[curedge->end])
		{
			if (iter->end == node_)
			{
				m_edges[curedge->end].erase(iter);
				break;
			}
		}
	}
	m_edges[node_].clear();
}
//--------------------------------------------------------
void c_sparse_graph::add_edge(const c_sparse_graph::s_nav_edge& edge_)
{
	assert((edge_.start < m_next_node_index) && (edge_.end < m_next_node_index));
	if((m_nodes[edge_.end].index != INVALID_NODE_INDEX) && (m_nodes[edge_.start].index != INVALID_NODE_INDEX))
	{
		if (_unique_edge(edge_.start, edge_.end))
			m_edges[edge_.start].push_back(edge_);
		if (_unique_edge(edge_.end, edge_.start))
		{
			s_nav_edge newedge = edge_;
			newedge.end = edge_.start;
			newedge.start = edge_.end;
			m_edges[edge_.end].push_back(newedge);
		}
	}
}
//--------------------------------------------------------
void c_sparse_graph::remove_edge(unsigned int from_, unsigned int to_)
{
	assert((from_ < m_nodes.size()) && (to_ < m_nodes.size()));
	FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , iter , m_edges[to_])
	{
		if (iter->end == from_)
		{
			m_edges[to_].erase(iter);
			iter++;
			break;
		}
	}
	FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , iter , m_edges[from_])
	{
		if (iter->end == to_)
		{
			m_edges[from_].erase(iter);
			iter++;
			break;
		}
	}
}
//--------------------------------------------------------
bool c_sparse_graph::_is_node_present(unsigned int nd_) const
{
	if ((nd_ >= m_nodes.size() || (m_nodes[nd_].index == INVALID_NODE_INDEX)))
	{
		return false;
	}
	else return true;
}
//--------------------------------------------------------
bool c_sparse_graph::_is_edge_present(unsigned int s_, unsigned int e_)const
{
	if (_is_node_present(s_) && _is_node_present(s_))
	{
		FOREACH(TYPEOF(std::list<s_nav_edge>)::const_iterator , iter , m_edges[s_])
		{
			if (iter->end == e_) 
				return true;
		}
		return false;
	}
	else 
		return false;
}
//--------------------------------------------------------
bool c_sparse_graph::_unique_edge(unsigned int s_, unsigned int e_)const
{
	FOREACH(TYPEOF(std::list<s_nav_edge>)::const_iterator , iter , m_edges[s_])
	{
		if (iter->end == e_)
			return false;
	}
	return true;
}
//--------------------------------------------------------
void c_sparse_graph::_add_neighbours(unsigned int row_, unsigned int col_, unsigned int numcellx_, unsigned int numcelly_)
{   
	for(int i=-1; i<2; ++i)
	{
		for(int j=-1; j<2; ++j)
		{
			int nodex = col_+j;
			int nodey = row_+i;
			if ( (i == 0) && (j==0) )
				continue;
			if (!((nodex < 0) || (nodex >= numcellx_) || (nodey < 0) || (nodey >= numcelly_)))
			{
				pathNode posnode = get_node(row_*numcellx_+col_).pos;
				pathNode posneighbour = get_node(nodey*numcellx_+nodex).pos;
				float dist = pt_distance(posnode , posneighbour);
				s_nav_edge newedge;
				newedge.start = row_*numcellx_+col_;
				newedge.end = nodey*numcellx_+nodex;
				newedge.cost = dist;
				add_edge(newedge);
				newedge.start = nodey*numcellx_+nodex;
				newedge.end = row_*numcellx_+col_;
				newedge.cost = dist;
				add_edge(newedge);
			}
		}
	}
}
//--------------------------------------------------------
void c_sparse_graph::_weight_edges(unsigned int nd_, float weight_)
{
	FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , pE , m_edges[nd_])
	{
		float dist = pt_distance(get_node(pE->start).pos,get_node(pE->end).pos);
		_set_edge_cost(pE->start, pE->end, dist * weight_);
		_set_edge_cost(pE->end, pE->start, dist * weight_);
	}
}
//--------------------------------------------------------
void c_sparse_graph::_set_edge_cost(unsigned int s_, unsigned int e_, float cost_)
{
	assert((s_ < m_nodes.size()) && (e_ < m_nodes.size()));
	FOREACH(TYPEOF(std::list<s_nav_edge>)::iterator , iter , m_edges[s_])
	{
		if (iter->end == e_)
		{
			iter->cost = cost_;
			break;
		}
	}
}
//--------------------------------------------------------
unsigned int c_sparse_graph::_find_nearest_pos( const pathNode& st_, const pathNode& et_ ) const
{
	float x = st_.x() - et_.x();
	float y = st_.y() - et_.y();
	float len = sqrt(x*x + y*y);
	x = x/len;
	y = y/len;
	for(int i = 0 ; i < len / TILE_SCALE ; ++i)
	{
		float dx = et_.x() + x*i * TILE_SCALE;
		float dy = et_.y() + y*i * TILE_SCALE;
		unsigned int ex = (unsigned int)((float)(dx)/m_cellw);  
		unsigned int ey = (unsigned int)((float)(dy)/m_cellh); 
		unsigned int idx = ey*m_num_cellx+ex; 
		s_nav_node nd = get_node(idx);
		if(nd.index!=INVALID_NODE_INDEX)
		{
			return nd.index;
		}
	}
	return INVALID_NODE_INDEX;
}

void c_sparse_graph::smooth_path(float curx, float cury, std::list<pathNode>& path)
{
	if(path.size()<3)
		return;
	
	pathNode curNode;
	curNode._x = curx;
	curNode._y = cury;
	
	pathNode tempNode;
	std::list<pathNode>::iterator et = path.end();
	et--;
	FOREACH(TYPEOF(std::list<pathNode>)::iterator , iter , path)
	{
		if(iter == path.begin())
			continue;
		if(iter == et)
			continue;
		std::list<pathNode>::iterator tmp = iter;
		if (can_walk_between(curNode, *(tmp++), tempNode))
		{
			tmp = iter;
			iter++;
			path.erase(tmp);
		}
	}
}

//--------------------------------------------------------
void c_sparse_graph::_smooth_path(std::list<unsigned int>& path_)
{
	if(path_.size()<3)
		return;
	std::list<unsigned int>::iterator et = path_.end();
	et--;
	FOREACH(TYPEOF(std::list<unsigned int>)::iterator , iter , path_)
	{
		if(iter == path_.begin())
			continue;
		if(iter == et)
			continue;
		std::list<unsigned int>::iterator tmp = iter;
		if (_can_walk_between(*(tmp--), *(tmp++)))
		{
			tmp = iter;
			iter++;
			path_.erase(tmp);
		}
	}
}

bool c_sparse_graph::can_walk_between(const pathNode& p1, const pathNode& p2, pathNode& out)
{
	pathNode vec(p2.x() - p1.x() , p2.y() - p1.y());
	float len = sqrt(vec.x()*vec.x() + vec.y()*vec.y());
	vec.x()/=len;
	vec.y()/=len;
	for(unsigned int i = 0 ; i < (unsigned int)(len/TILE_SCALE) ; ++i)
	{
		float dx = p1.x() + vec.x()*i*TILE_SCALE;
		float dy = p1.y() + vec.y()*i*TILE_SCALE;
		s_nav_node nd = get_node((unsigned int)(dy/m_cellh)*m_num_cellx+(unsigned int)(dx/m_cellw));
		if(nd.index==INVALID_NODE_INDEX)
			return false;
		else
		{
			out._x = dx;
			out._y = dy;
		}
	}
	return true;
}

//--------------------------------------------------------
bool c_sparse_graph::_can_walk_between(unsigned int p1_, unsigned int p2_)
{
	pathNode p1 = get_node(p1_).pos;
	pathNode p2 = get_node(p2_).pos;
	pathNode out;
	return can_walk_between(p1, p2, out);
}
