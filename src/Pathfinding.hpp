#pragma once

#include <Simple-Graph/AStar.hpp>
#include "Simple-Utility/container/Vector2d.hpp"
#include "Simple-Utility/container/SortedVector.hpp"

#include <georithm/Vector.hpp>

#include <DynRPG/DynRPG.h>

#include <cassert>
#include <variant>
#include <algorithm>
#include <chrono>
#include <optional>

#undef max		// lol

using Vector2_t = georithm::Vector<int, 2>;
using Path = std::vector<Vector2_t>;

template <class TData>
using IdData = std::tuple<int, TData>;

struct IdLess
{
	template <class TLhs, class TRhs>
	constexpr bool operator ()(const TLhs& lhs, const TRhs& rhs) const noexcept
	{
		return id(lhs) < id(rhs);
	}

private:
	static constexpr int id(int i) noexcept
	{
		return i;
	}

	template <class TData>
	static constexpr int id(const IdData<TData>& data) noexcept
	{
		return std::get<0>(data);
	}
};

template <class TData>
using IdDataSortedVector = sl::container::SortedVector<IdData<TData>, IdLess>;

class PathManager
{
private:
	using PathNode = IdData<Path>;
	
public:
	const Path* find_path(int _id) const noexcept
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

	void clear_path(int _id) noexcept
	{
		if (auto itr = m_Paths.find(_id); itr != std::end(m_Paths))
			m_Paths.erase(itr);
	}
	
	void clear() noexcept
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
	
	void reset_cost(int _terrain_id) noexcept
	{
		if (auto itr = m_CostMap.find(_terrain_id); itr != std::end(m_CostMap))
			m_CostMap.erase(itr);
	}
	
	int get_cost(int _terrain_id) const noexcept
	{
		if (auto itr = m_CostMap.find(_terrain_id); itr != std::end(m_CostMap))
		{
			auto value = std::get<1>(*itr);
			if (value < 0)
				return std::max(1, RPG::system->variables[-value]);
			return std::max(1, value);
		}
		return _terrain_id;
	}
	
	void clear() noexcept
	{
		m_CostMap.clear();
	}

	friend std::ostream& operator <<(std::ostream& _out, const CostCalculator& _obj) noexcept
	{
		auto& data = _obj.m_CostMap;
		_out << std::size(data) << " ";
		std::for_each(std::begin(data), std::end(data),
			[&_out](const auto& _el) { _out << std::get<0>(_el) << " " << std::get<1>(_el) << " "; }	
		);
		return _out;
	}

	friend std::istream& operator >>(std::istream& _in, CostCalculator& _obj) noexcept
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

class EdgeCostCalculator
{
public:
	using CostKey = std::pair<int, int>;

	struct Cost
	{
		CostKey key;
		int cost;
	};

	struct CostKeyLess
	{
		template <class TLhs, class TRhs>
		bool operator ()(const TLhs& lhs, const TRhs& rhs) const noexcept
		{
			auto& lhsKey = key(lhs);
			auto& rhsKey = key(rhs);
			return lhsKey.first < rhsKey.first ||
				(lhsKey.first == rhsKey.first && lhsKey.second < rhsKey.second);
		}

	private:
		static constexpr const CostKey& key(const CostKey& key) noexcept
		{
			return key;
		}

		static constexpr const CostKey& key(const Cost& cost) noexcept
		{
			return cost.key;
		}
	};
	
	void set_cost(int _from_terrain_id, int _to_terrain_id, int _cost)
	{
		if (0 < _cost)
			m_Costs.insert_or_assign(Cost{ { _from_terrain_id, _to_terrain_id }, _cost });
	}

	void set_cost_var(int _from_terrain_id, int _to_terrain_id, int _id)
	{
		if (0 < _id)
			m_Costs.insert_or_assign(Cost{ { _from_terrain_id, _to_terrain_id }, -_id });
	}
	
	void reset_cost(int _from_terrain_id, int _to_terrain_id) noexcept
	{
		if (auto itr = m_Costs.find(CostKey{ _from_terrain_id, _to_terrain_id }); itr != std::end(m_Costs))
			m_Costs.erase(itr);
	}
	
	int get_cost(int _from_terrain_id, int _to_terrain_id) const noexcept
	{
		if (auto itr = m_Costs.find(CostKey{ _from_terrain_id, _to_terrain_id }); itr != std::end(m_Costs))
		{
			auto value = itr->cost;
			if (value < 0)
				return std::max(0, RPG::system->variables[-value]);
			return std::max(0, value);
		}
		return 0;
	}
	
	void clear() noexcept
	{
		m_Costs.clear();
	}

	//friend std::ostream& operator <<(std::ostream& _out, const CostCalculator& _obj)
	//{
	//	auto& data = _obj.m_CostMap;
	//	_out << std::size(data) << " ";
	//	std::for_each(std::begin(data), std::end(data),
	//		[&_out](const auto& _el) { _out << std::get<0>(_el) << " " << std::get<1>(_el) << " "; }	
	//	);
	//	return _out;
	//}

	//friend std::istream& operator >>(std::istream& _in, CostCalculator& _obj)
	//{
	//	auto& data = _obj.m_CostMap;
	//	data.clear();
	//	std::size_t size = 0;
	//	_in >> size;
	//	data.reserve(size);

	//	for (std::size_t i = 0; i < size; ++i)
	//	{
	//		int id;
	//		int val;
	//		_in >> id >> val;
	//		data.insert(IdData<data_type>{id, val});
	//	}
	//	return _in;
	//}
	
private:
	sl::container::SortedVector<Cost, CostKeyLess> m_Costs;
};

inline static CostCalculator globalCostCalculator;
inline static EdgeCostCalculator globalEdgeCostCalculator;

inline bool isValidPos(const Vector2_t& at) noexcept
{
	assert(RPG::map);
	auto& map = *RPG::map;
	return 0 <= at.x() && at.x() < map.getWidth() &&
		0 <= at.y() && at.y() < map.getHeight();
}

struct VectorLess
{
	constexpr bool operator ()(const Vector2_t& lhs, const Vector2_t& rhs) const noexcept
	{
		return lhs.x() < rhs.x() || (lhs.x() == rhs.x() && lhs.y() < rhs.y());
	}
};

using NodeInfo = sl::graph::AStarNodeInfo_t<Vector2_t, int>;
using Node = std::pair<Vector2_t, NodeInfo>;

class Pathfinder
{
public:
	std::optional<int> calc_path(const Vector2_t& destination, RPG::Character& character)
	{
		Vector2_t start{ character.x, character.y };

		auto neighbourSearcher = [&character](const Vector2_t& position, const auto& node, auto callback) noexcept
		{
			for (int i = 0; i < 4; ++i)
			{
				auto to = position;
				switch (i)
				{
				case 0: ++to.x(); break;
				case 1: --to.x(); break;
				case 2: ++to.y(); break;
				case 3: --to.y(); break;
				}
				if (isValidPos(to) && character.isMovePossible(position.x(), position.y(), to.x(), to.y()))
					callback(to);
			}
		};
		
		class PropertyMap
		{
		public:
			using VertexType = Vector2_t;
			using WeightType = int;

			int heuristic(const Vector2_t& vertex, const Vector2_t& destination) const noexcept
			{
				auto dist = abs(vertex - destination);
				return dist.x() + dist.y();
			}

			int edgeWeight(const Vector2_t& from, const Vector2_t& to) const noexcept
			{
				auto fromTerrainId = RPG::map->getTerrainId(RPG::map->getLowerLayerTileId(from.x(), from.y()));
				auto toTerrainId = RPG::map->getTerrainId(RPG::map->getLowerLayerTileId(to.x(), to.y()));
				return globalEdgeCostCalculator.get_cost(fromTerrainId, toTerrainId);
			}

			int nodeWeight(const Vector2_t& vertex) const noexcept
			{
				auto tileId = RPG::map->getLowerLayerTileId(vertex.x(), vertex.y());
				return globalCostCalculator.get_cost(RPG::map->getTerrainId(tileId));
			}
		};

		class StateMap
		{
		public:
			StateMap() :
				m_NodeMap{ static_cast<std::size_t>(RPG::map->getWidth()), static_cast<std::size_t>(RPG::map->getHeight()) }
			{
			}

			auto& operator [](const Vector2_t& at) const noexcept
			{
				return m_NodeMap[at.x()][at.y()];
			}

			auto& operator [](const Vector2_t& at) noexcept
			{
				return m_NodeMap[at.x()][at.y()];
			}
			
		private:
			sl::container::Vector2d<NodeInfo> m_NodeMap;
		};
		StateMap stateMap;
		sl::graph::traverseAStar(start, destination, PropertyMap{}, neighbourSearcher, stateMap);
		
		if (auto path = _extract_path(stateMap, destination))
			return globalPathMgr.insert_path(std::move(*path));
		return std::nullopt;
	}
	
private:
	template <class TClosedList>
	std::optional<Path> _extract_path(const TClosedList& stateMap, const Vector2_t& destination)
	{
		Path path;
		std::optional<Vector2_t> currentPos{ destination };
		while (currentPos)
		{
			path.emplace_back(*currentPos);
			currentPos = stateMap[*currentPos].parent;
		}
		std::reverse(std::begin(path), std::end(path));
		return path;
	}
};
