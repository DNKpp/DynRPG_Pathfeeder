#include "Simple-Graph/Searcher.hpp"
#include "Simple-Utility/container/Vector2d.hpp"
#include "Simple-Utility/container/SortedVector.hpp"

#include "Vector.hpp"

#include <DynRPG/DynRPG.h>
#include <unordered_map>

using Path = std::vector<Vector>;

class PathManager
{
public:
	const Path* find_path(int _id) const
	{
		if (auto itr = m_Paths.find(_id); itr != std::end(m_Paths))
			return &itr->second;
		return nullptr;
	}

	int insert_path(Path _path)
	{
		auto id = m_NextId++;
		m_Paths.emplace(id, std::move(_path));
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
	std::unordered_map<int, Path> m_Paths;
};

inline static PathManager globalPathMgr;

class CostCalculator
{
public:
	void set_cost(int _terrain_id, int _cost)
	{
		m_CostMap.insert_or_assign(_terrain_id, _cost);
	}
	
	void reset_cost(int _terrain_id)
	{
		if (auto itr = m_CostMap.find(_terrain_id); itr != std::end(m_CostMap))
			m_CostMap.erase(itr);
	}
	
	int get_cost(int _terrain_id) const
	{
		if (auto itr = m_CostMap.find(_terrain_id); itr != std::end(m_CostMap))
			return itr->second;
		return _terrain_id;
	}
	
	void clear()
	{
		m_CostMap.clear();
	}

private:
	std::unordered_map<int, int> m_CostMap;
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