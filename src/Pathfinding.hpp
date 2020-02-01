#include "Simple-Graph/algorithm.hpp"
#include "Simple-Utility/container/Vector2d.hpp"
#include "Simple-Utility/container/SortedVector.hpp"

#include "Vector.hpp"

#include <DynRPG/DynRPG.h>

#include <cassert>
#include <variant>
#include <algorithm>
#include <chrono>

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

		auto costCalculator = [](const Vector& _pos)
		{
			auto tileId = RPG::map->getLowerLayerTileId(_pos.x, _pos.y);
			//return globalCostCalculator.get_cost(RPG::map->getTerrainId(tileId));
			return RPG::map->getTerrainId(tileId);
		};
		
		auto heuristicCalculator = [&_end](const Vector& _prevPos, const Vector& _pos)
		{
			auto diff = _end - _pos;
			return std::abs(diff.x) + std::abs(diff.y);
		};

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

		class TableVisitationTracker
		{
		public:
			TableVisitationTracker(std::size_t _width, std::size_t _height) :
				m_Tracker{ _width, _height }
			{
			}
			
			decltype(auto) operator [](const Vector& _at)
			{
				return m_Tracker[_at.x][_at.y];
			}
			
		private:
			sl::container::Vector2d<bool> m_Tracker;
		};

		using Node = sl::graph::AStarNode<Vector, int>;
		struct NodeVectorLess
		{
			bool operator ()(const Node& _lhs, const Node& _rhs) const
			{
				return VectorLess{}(_lhs.vertex, _rhs.vertex);
			}

			bool operator ()(const Node& _lhs, const Vector& _rhs) const
			{
				return VectorLess{}(_lhs.vertex, _rhs);
			}

			bool operator ()(const Vector& _lhs, const Node& _rhs) const
			{
				return VectorLess{}(_lhs, _rhs.vertex);
			}
		};
		
		sl::container::SortedVector<Node, NodeVectorLess> closedList;
		sl::graph::traverse_astar(start, _end, neighbourSearcher,
			TableVisitationTracker{ static_cast<std::size_t>(RPG::map->getWidth()), static_cast<std::size_t>(RPG::map->getHeight()) },
			heuristicCalculator, costCalculator, sl::graph::ConstWeight<0>{},
			[&closedList](const Node& _node)
			{
				closedList.insert(_node);
			}
		);
		
		if (auto path = _extract_path(closedList, _end))
			return globalPathMgr.insert_path(std::move(*path));
		return std::nullopt;
	}
	
private:
	template <class TClosedList>
	std::optional<Path> _extract_path(const TClosedList& _closedList, Vector _end)
	{
		Path path;
		auto itr = _closedList.find(_end);
		if (itr == std::end(_closedList))
			return std::nullopt;

		path.emplace_back(itr->vertex);
		while (itr->parent)
		{
			path.emplace_back(*itr->parent);
			itr = _closedList.find(*itr->parent);
		}
		std::reverse(std::begin(path), std::end(path));
		return path;
	}
};
