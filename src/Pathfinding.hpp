#include "Simple-Graph/Searcher.hpp"
#include "Simple-Utility/container/Vector2d.hpp"
#include "Simple-Utility/container/SortedVector.hpp"

#include "Vector.hpp"

#include <DynRPG/DynRPG.h>

#include <cassert>
#include <variant>
#include <algorithm>

#undef max		// lol

using Path = std::vector<Vector>;

template <class TData>
using IdData = std::tuple<int, TData>;

struct IdLess
{
	template <class TData>
	bool operator ()(const IdData<TData>& _lhs, const IdData<TData>& _rhs) const
	{
		return std::get<0>(_lhs) < std::get<0>(_rhs);
	}

	template <class TData>
	bool operator ()(const IdData<TData>& _lhs, int _rhs) const
	{
		return std::get<0>(_lhs) < _rhs;
	}

	template <class TData>
	bool operator ()(int _lhs, const IdData<TData>& _rhs) const
	{
		return _lhs < std::get<0>(_rhs);
	}
};

template <class TData>
using IdDataSortedVector = sl::container::SortedVector<IdData<TData>, IdLess>;

class PathManager
{
private:
	using PathNode = IdData<Path>;
	
public:
	const Path* find_path(int _id) const
	{
		if (auto itr = m_Paths.find(_id); itr != std::end(m_Paths))
			return &std::get<Path>(*itr);
		return nullptr;
	}

	int insert_path(Path _path)
	{
		auto id = m_NextId++;
		m_Paths.insert(PathNode{ id, std::move(_path) });
		return id;
	}

	void clear_path(int _id)
	{
		if (auto itr = m_Paths.find(_id); itr != std::end(m_Paths))
			m_Paths.erase(itr);
	}
	
	void clear()
	{
		m_Paths.clear();
		m_NextId = 1;
	}

private:
	int m_NextId = 1;
	IdDataSortedVector<Path> m_Paths;
};

inline static PathManager globalPathMgr;

class CostCalculator
{
public:
	using data_type = int;
	void set_cost(int _terrain_id, int _cost)
	{
		if (0 < _cost)
			m_CostMap.insert_or_assign(IdData<data_type>{_terrain_id, _cost});
	}

	void set_cost_var(int _terrain_id, int _id)
	{
		if (0 < _id)
			m_CostMap.insert_or_assign(IdData<data_type>{_terrain_id, -_id});
	}
	
	void reset_cost(int _terrain_id)
	{
		if (auto itr = m_CostMap.find(_terrain_id); itr != std::end(m_CostMap))
			m_CostMap.erase(itr);
	}
	
	int get_cost(int _terrain_id) const
	{
		if (auto itr = m_CostMap.find(_terrain_id); itr != std::end(m_CostMap))
		{
			auto value = std::get<1>(*itr);
			if (value < 0)
				return std::max(0, RPG::system->variables[-value]);
			return value;
		}
		return _terrain_id;
	}
	
	void clear()
	{
		m_CostMap.clear();
	}

	friend std::ostream& operator <<(std::ostream& _out, const CostCalculator& _obj)
	{
		auto& data = _obj.m_CostMap;
		_out << std::size(data) << " ";
		std::for_each(std::begin(data), std::end(data),
			[&_out](const auto& _el) { _out << std::get<0>(_el) << " " << std::get<1>(_el) << " "; }	
		);
		return _out;
	}

	friend std::istream& operator >>(std::istream& _in, CostCalculator& _obj)
	{
		auto& data = _obj.m_CostMap;
		data.clear();
		std::size_t size = 0;
		_in >> size;
		data.reserve(size);

		for (std::size_t i = 0; i < size; ++i)
		{
			int id;
			int val;
			_in >> id >> val;
			data.insert(IdData<data_type>{id, val});
		}
		return _in;
	}
	
private:
	IdDataSortedVector<data_type> m_CostMap;
};

inline static CostCalculator globalCostCalculator;

bool is_valid_pos(const Vector& _at)
{
	assert(RPG::map);
	auto& map = *RPG::map;
	return 0 <= _at.x && _at.x < map.getWidth() &&
		0 <= _at.y && _at.y < map.getHeight();
}

template <class TSearcher>
class NodeMap
{
public:
	using vertex_type = typename TSearcher::VertexType;
	using value_type = typename TSearcher::NodeDataType;
	using node_type = typename TSearcher::NodeType;

	explicit NodeMap(int _width, int _height) :
		m_Map(_width, _height)
	{
	}

	template <class TNodeCompare>
	void insert(const node_type& _node, TNodeCompare&& _nodeComp)
	{
		auto& at = m_Map[_node.vertex.x][_node.vertex.y];
		if (!at)
			at = _node.data;
		else if (_nodeComp(_node.data, *at))
		{
			at = _node.data;
			++m_NodeCount;
		}
	}

	/*template <class TNodeCompare>
	node_type take_node(TNodeCompare&& _nodeComp)
	{
		auto itr = std::min_element(std::begin(m_Nodes), std::end(m_Nodes),
			[&_nodeComp](const auto& _lhs, const auto& _rhs) { return _nodeComp(_lhs.second, _rhs.second); }
		);
		assert(itr != std::end(m_Nodes));
		auto node = std::move(*itr);
		m_Nodes.erase(itr);
		return { node.first, node.second };
	}*/

	const value_type* find(const vertex_type& _key) const
	{
		auto& at = m_Map[_key.x][_key.y];
		if (at)
			return &*at;
		return nullptr;
	}

	bool contains(const vertex_type& _key) const
	{
		return m_Map[_key.x][_key.y].has_value();
	}

	bool empty() const
	{
		return m_NodeCount == 0;
	}

private:
	int m_NodeCount = 0;
	sl::container::Vector2d<std::optional<value_type>> m_Map;
};

template <class TSearcher>
class NodeList
{
public:
	using vertex_type = typename TSearcher::VertexType;
	using value_type = typename TSearcher::NodeDataType;
	using node_type = typename TSearcher::NodeType;

	template <class TNodeCompare>
	void insert(const node_type& _node, TNodeCompare&& _nodeComp)
	{
		if (auto itr = m_Nodes.find(_node); itr != std::end(m_Nodes))
		{
			if (_nodeComp(_node.data, itr->data))
				itr->data = _node.data;
		}
		else
			m_Nodes.insert(_node);
	}

	template <class TNodeCompare>
	node_type take_node(TNodeCompare&& _nodeComp)
	{
		auto itr = std::min_element(std::begin(m_Nodes), std::end(m_Nodes),
			[&_nodeComp](const auto& _lhs, const auto& _rhs) { return _nodeComp(_lhs.data, _rhs.data); }
		);
		assert(itr != std::end(m_Nodes));
		auto node = *itr;
		m_Nodes.erase(itr);
		return node;
	}

	/*const NodeDataType* find(const VertexType& _key) const
	{
		auto& at = m_Map[_node.vertex.x][_node.vertex.y];
		if (at)
			return &*at;
		return nullptr;
	}*/

	/*bool contains(const vertex_type& _key) const
	{
		return m_Map[_node.vertex.x][_node.vertex.y];
	}*/

	bool empty() const
	{
		return std::empty(m_Nodes);
	}

private:
	struct NodeVertexLess
	{
		bool operator ()(const node_type& _lhs, const node_type& _rhs) const
		{
			return _lhs.vertex.x < _rhs.vertex.x ||
				(_lhs.vertex.x == _rhs.vertex.x && _lhs.vertex.y < _rhs.vertex.y);
		}
	};
	
	int m_NodeCount = 0;
	sl::container::SortedVector<node_type, NodeVertexLess> m_Nodes;
};

class Pathfinder
{
public:
	std::optional<int> calc_path(const Vector& _end, RPG::Character& _character)
	{
		Vector start{ _character.x, _character.y };

		struct VectorLess
		{
			bool operator ()(const Vector& _lhs, const Vector& _rhs) const
			{
				return _lhs.x < _rhs.x || (_lhs.x == _rhs.x && _lhs.y < _rhs.y);
			}
		};

		auto costCalculator = [](const Vector& _prevPos, const Vector& _pos)
		{
			auto tileId = RPG::map->getLowerLayerTileId(_pos.x, _pos.y);
			return globalCostCalculator.get_cost(RPG::map->getTerrainId(tileId));
		};
		
		auto heuristicCalculator = [](const Vector& _prevPos, const Vector& _pos)
		{
			auto diff = _pos - _prevPos;
			return diff.x + diff.y;
		};
		auto searcher = sl::graph::make_astar_searcher<Vector>(costCalculator, heuristicCalculator);
		using Searcher_t = decltype(searcher);
		//using Map_t = std::map<typename Searcher_t::VertexType, typename Searcher_t::NodeDataType, VectorLess>;


		auto neighbourSearcher = [&_character](const auto& _node, auto&& _callback)
		{
			for (int i = 0; i < 4; ++i)
			{
				Vector dir{ 0, 0 };
				switch (i)
				{
				case 0: dir.x = 1; break;
				case 1: dir.x = -1; break;
				case 2: dir.y = 1; break;
				case 3: dir.y = -1; break;
				}
				auto at = _node.vertex + dir;
				if (is_valid_pos(at) && _character.isMovePossible(_node.vertex.x, _node.vertex.y, at.x, at.y))
					_callback(at);
			}
		};

		if (auto path = sl::graph::find_path(start, _end, neighbourSearcher, searcher, NodeList<Searcher_t>(), NodeMap<Searcher_t>(RPG::map->getWidth(), RPG::map->getHeight())))
			return globalPathMgr.insert_path(std::move(*path));
		return std::nullopt;
	}
	
private:
	
};