//#define AUTO_DLLMAIN
//#define CUSTOM_DLLMAIN
//#define _GLIBCXX_USE_CXX11_ABI 0

// ToDo: Clear the globalPathManager object on map change

#include "Simple-Graph/Searcher.hpp"

#include <DynRPG/DynRPG.h>
#include <cassert>
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <charconv>
#include <locale>
#include <unordered_map>

#include "Vector.hpp"

bool is_valid_pos(const Vector& _at)
{
	assert(RPG::map);
	auto& map = *RPG::map;
	return 0 <= _at.x && _at.x < map.getWidth() &&
		0 <= _at.y && _at.y < map.getHeight();
}

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
		m_NextId = 1;
		m_Paths.clear();
	}

private:
	int m_NextId = 1;
	std::unordered_map<int, Path> m_Paths;
};

static inline PathManager globalPathMgr;

class Pathfinder
{
public:
	std::optional<int> calc_path(const Vector& _end)
	{
		auto& character = *RPG::hero;
		Vector start{ character.x, character.y };

		struct VectorLess
		{
			bool operator ()(const Vector& _lhs, const Vector& _rhs) const
			{
				return _lhs.x < _rhs.x || (_lhs.x == _rhs.x && _lhs.y < _rhs.y);
			}
		};

		//auto searcher = sl::graph::make_breadth_first_searcher<Vector>();

		auto costCalculator = [](const Vector& _prevPos, const Vector& _pos) { return 1; }; // ToDo: calculate cost from terrain type
		auto heuristicCalculator = [](const Vector& _prevPos, const Vector& _pos)
		{
			auto diff = _pos - _prevPos;
			return diff.x + diff.y;
		};
		auto searcher = sl::graph::make_astar_searcher<Vector>(costCalculator, heuristicCalculator);
		using Searcher_t = decltype(searcher);
		using Map_t = std::map<typename Searcher_t::VertexType, typename Searcher_t::NodeDataType, VectorLess>;
		using NodeMap = sl::graph::NodeMap<Map_t>;


		auto neighbourSearcher = [&character](const auto& _node, auto&& _callback)
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
				if (is_valid_pos(at) && character.isMovePossible(_node.vertex.x, _node.vertex.y, at.x, at.y))
					_callback(at);
			}
		};

		if (auto path = sl::graph::find_path(start, _end, neighbourSearcher, searcher, NodeMap(Map_t(VectorLess())), NodeMap(Map_t(VectorLess()))))
			return globalPathMgr.insert_path(std::move(*path));
		return std::nullopt;
	}
	
private:
	
};

struct RPGVariable
{
	static int* get_ptr(int _index)
	{
		if (_index < 0 || RPG::system->variables.size <= _index)
			return &RPG::system->variables[_index];
		return nullptr;
	}

	static int& get(int _index)
	{
		if (auto ptr = get_ptr(_index))
			return *ptr;
		throw std::runtime_error("RPG::Variable index out of bounds.");
	}
};

struct RPGSwitch
{
	static bool* get_ptr(int _index)
	{
		if (_index < 0 || RPG::system->switches.size <= _index)
			return &RPG::system->switches[_index];
		return nullptr;
	}

	static bool& get(int _index)
	{
		if (auto ptr = get_ptr(_index))
			return *ptr;
		throw std::runtime_error("RPG::Switch index out of bounds.");
	}
};

struct Param
{
	static std::optional<int> get_integer(const RPG::ParsedCommentParameter& _param)
	{
		switch (_param.type)
		{
		case RPG::PARAM_NUMBER:
			return get_from_number<int>(_param);
		case RPG::PARAM_TOKEN:
			return lookup_value<int>(std::begin(_param.text), std::end(_param.text));
		default:
			throw std::runtime_error("Invalid argument.");
		}
	}

private:
	template <class TType>
	static TType get_from_number(const RPG::ParsedCommentParameter& _param)
	{
		return static_cast<TType>(_param.number);
	}

	template <class TType, class TIterator>
	static std::optional<TType> lookup_value(TIterator _itr, const TIterator& _end)
	{
		std::string_view token(_itr, std::distance(_itr, _end));
		if (std::empty(token))
			return std::nullopt;

		if constexpr (std::is_same_v<int, TType>)
		{
			auto valBegin = std::find_if_not(std::rbegin(token), std::rend(token), [](char _c) { return '0' <= _c && _c <= '9'; }).base();
			if (valBegin == std::end(token))
				return std::nullopt;

			int curIndex = 0;
			auto result = std::from_chars(&*valBegin, token.data() + token.size(), curIndex);
		    if (result.ec == std::errc::invalid_argument)
		        return std::nullopt;

			try
			{
				for (auto itr = std::begin(token); itr != valBegin; ++itr)
				{
					if (std::tolower(*itr, std::locale()) != 'v')
						return std::nullopt;
					
					curIndex = RPGVariable::get(curIndex);
				}
				return RPGVariable::get(curIndex);
			}
			catch (const std::runtime_error& _e)
			{
			}
			return std::nullopt;
		}
		else if constexpr (std::is_same_v<bool, TType>)
		{
			if (std::tolower(token[0], std::locale()) != 's')
				return std::nullopt;

			try
			{
				if (auto index = lookup_value<int>(_itr + 1, _end))
					return RPGSwitch::get(index);
			}
			catch (const std::runtime_error& _e)
			{
			}
			return std::nullopt;
		}

		return std::nullopt;		
	}
};

void cmd_find_path(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	Pathfinder p;
	
	auto& outId = RPGVariable::get(Param::get_integer(params[0]).value());
	auto& outSuccess = RPGSwitch::get(Param::get_integer(params[1]).value());
	if (auto optId = p.calc_path({ 20, 20 }))
	{
		outId = *optId;
		outSuccess = true;
	}
	else
		outSuccess = false;
}

void cmd_get_path_length(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = Param::get_integer(params[0]).value();
	auto& outLength = RPGVariable::get(Param::get_integer(params[1]).value());
	auto& outSuccess = RPGSwitch::get(Param::get_integer(params[2]).value());

	if (auto pathPtr = globalPathMgr.find_path(id))
	{
		outLength = std::size(*pathPtr);
		outSuccess = true;
	}
	else
		outSuccess = false;
}

void cmd_get_path_vertex(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = params[0].number;
	auto index = params[1].number;
	auto& outX = RPGVariable::get(Param::get_integer(params[2]).value());
	auto& outY = RPGVariable::get(Param::get_integer(params[3]).value());
	auto& outSuccess = RPGSwitch::get(Param::get_integer(params[4]).value());

	if (auto pathPtr = globalPathMgr.find_path(id))
	{
		auto& path = *pathPtr;
		auto vertex = path[static_cast<std::size_t>(index)];
		outX = vertex.x;
		outY = vertex.y;
		outSuccess = true;
	}
	else
		outSuccess = false;
}

void cmd_clear_path(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = params[0].number;

	globalPathMgr.clear_path(id);
}

// called on comment
bool onComment(const char* _text, const RPG::ParsedCommentData* _parsedData, RPG::EventScriptLine* _nextScriptLine,
	RPG::EventScriptData* _scriptData, int _eventId, int _pageId, int _lineId, int* _nextLineId)
{
	std::string_view cmd{ _parsedData->command };
	if (cmd == "find_path")
	{
		cmd_find_path(_text, _parsedData);
		return false;
	}
	else if (cmd == "get_path_length")
	{
		cmd_get_path_length(_text, _parsedData);
		return false;
	}
	else if (cmd == "get_path_vertex")
	{
		cmd_get_path_vertex(_text, _parsedData);
		return false;
	}
	else if (cmd == "clear_path")
	{
		cmd_clear_path(_text, _parsedData);
		return false;
	}
	return true;
}
