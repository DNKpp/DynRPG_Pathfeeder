//#define AUTO_DLLMAIN
//#define CUSTOM_DLLMAIN
//#define _GLIBCXX_USE_CXX11_ABI 0

// ToDo: Clear the globalPathManager object on map change

#include <DynRPG/DynRPG.h>
#include <string_view>
#include <stdexcept>
#include <charconv>
#include <locale>
#include <sstream>

#include "Vector.hpp"
#include "Pathfinding.hpp"

struct RPGVariable
{
	static int* get_ptr(int _index)
	{
		if (0 < _index)
		// || RPG::system->variables.size <= _index)
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
		if (0 < _index)
			//|| RPG::system->switches.size <= _index)
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
		/* It seems, the current DynRPG version already parses the values out of the token. So this is obsolete now?
		case RPG::PARAM_TOKEN:
			return lookup_value<int>(std::begin(_param.text), std::find(std::begin(_param.text), std::end(_param.text), 0));*/
		default:
			throw std::runtime_error("Invalid argument.");
		}
	}

	static RPG::Character* get_character(const RPG::ParsedCommentParameter& _param)
	{
		switch (_param.type)
		{
		case RPG::PARAM_NUMBER:
			return RPG::map->events[_param.number];
		case RPG::PARAM_TOKEN:
			return lookup_value<RPG::Character*>(std::begin(_param.text), std::find(std::begin(_param.text), std::end(_param.text), 0)).value_or(nullptr);
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
		std::string_view token(&*_itr, std::distance(_itr, _end));
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
		else if constexpr (std::is_same_v<RPG::Character*, std::remove_cv_t<TType>>)
		{
			std::string tokenStr(std::size(token), 0);
			std::transform(std::begin(token), std::end(token), std::begin(tokenStr),
				[](char _c){ return std::tolower(_c, std::locale()); }
			);

			if (tokenStr == "hero")
				return RPG::hero;
			if (tokenStr == "ship")
				return RPG::vehicleShip;
			if (tokenStr == "airship")
				return RPG::vehicleAirship;
			if (tokenStr == "skiff")
				return RPG::vehicleSkiff;

			// seems to be not necessary; let's just wait for the number
			/*if (tokenStr[0] == 'n')
			{
				if (auto eventId = lookup_value<int>(std::begin(tokenStr) + 1, std::end(tokenStr)))
					return RPG::map->events[*eventId];
			}*/
		}

		return std::nullopt;		
	}
};

void cmd_find_path(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto& outSuccess = RPGSwitch::get(Param::get_integer(params[4]).value());
	outSuccess = false;

	if (auto target = Param::get_character(params[0]))
	{
		Pathfinder p;
		auto x = Param::get_integer(params[1]).value();
		auto y = Param::get_integer(params[2]).value();
		auto& outId = RPGVariable::get(Param::get_integer(params[3]).value());
		if (auto optId = p.calc_path({ x, y }, *target))
		{
			outId = *optId;
			outSuccess = true;
		}
	}
}

void cmd_get_path_length(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto& outSuccess = RPGSwitch::get(Param::get_integer(params[2]).value());
	outSuccess = false;

	auto id = Param::get_integer(params[0]).value();
	auto& outLength = RPGVariable::get(Param::get_integer(params[1]).value());
	

	if (auto pathPtr = globalPathMgr.find_path(id))
	{
		outLength = std::size(*pathPtr);
		outSuccess = true;
	}
}

void cmd_get_path_vertex(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = Param::get_integer(params[0]).value();
	auto index = Param::get_integer(params[1]).value();
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
	auto id = Param::get_integer(params[0]).value();

	globalPathMgr.clear_path(id);
}

void cmd_set_terrain_cost(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = Param::get_integer(params[0]).value();
	auto cost = Param::get_integer(params[1]).value();

	globalCostCalculator.set_cost(id, cost);
}

void cmd_set_terrain_cost_var(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = Param::get_integer(params[0]).value();
	auto var = Param::get_integer(params[1]).value();

	globalCostCalculator.set_cost_var(id, var);
}

void cmd_get_terrain_cost(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = Param::get_integer(params[0]).value();
	auto& outCost = RPGVariable::get(Param::get_integer(params[1]).value());

	outCost = globalCostCalculator.get_cost(id);
}

void cmd_reset_terrain_cost(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	auto id = Param::get_integer(params[0]).value();

	globalCostCalculator.reset_cost(id);
}

void cmd_clear_terrain_costs(const char* _text, const RPG::ParsedCommentData* _parsedData)
{
	auto& params = _parsedData->parameters;
	globalCostCalculator.clear();
}

bool onComment(const char* _text, const RPG::ParsedCommentData* _parsedData, RPG::EventScriptLine* _nextScriptLine,
	RPG::EventScriptData* _scriptData, int _eventId, int _pageId, int _lineId, int* _nextLineId)
{
	std::string_view cmd{ _parsedData->command };
	if (cmd == "pathfeeder_find_path")
	{
		cmd_find_path(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_get_path_length")
	{
		cmd_get_path_length(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_get_path_vertex")
	{
		cmd_get_path_vertex(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_clear_path")
	{
		cmd_clear_path(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_set_terrain_cost")
	{
		cmd_set_terrain_cost(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_set_terrain_cost_var")
	{
		cmd_set_terrain_cost_var(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_reset_terrain_cost")
	{
		cmd_reset_terrain_cost(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_get_terrain_cost")
	{
		cmd_get_terrain_cost(_text, _parsedData);
		return false;
	}
	else if (cmd == "pathfeeder_clear_terrain_costs")
	{
		cmd_clear_terrain_costs(_text, _parsedData);
		return false;
	}
	return true;
}

bool onEventCommand(RPG::EventScriptLine* _scriptLine, RPG::EventScriptData* _scriptData, int _eventId, int _pageId, int _lineId, int* _nextLineId)
{
	switch (_scriptLine->command)
	{
	case RPG::EVCMD_TELEPORT:
	{
		/*
		 * 0 = mapId
		 * 1 = x
		 * 2 = y
		 * 3 = facing (0 = retain)
		 */
		auto& params = _scriptLine->parameters;
		if (RPG::Map::properties->id != params[0])
			globalPathMgr.clear();
	}
	default: break;
	}
	return true;
}

/*template <class T>
class ArrayView
{
public:
	using value_type				= T;
	using size_type					= std::size_t;
    using difference_type			= std::ptrdiff_t;
    using reference					= T&;
    using const_reference			= const T&;
    using pointer					= T*;
    using const_pointer				= const T*;
    using iterator					= typename TContainer::iterator;
    using const_iterator			= typename TContainer::const_iterator;
    using reverse_iterator			= typename TContainer::reverse_iterator;
    using const_reverse_iterator	= typename TContainer::const_reverse_iterator;
	
	ArrayView(T* _dataPtr, int _length) :
		m_DataPtr{ _dataPtr },
		m_Length{ _length }
	{
	}

    reference operator[](size_type n)
	{
		assert(m_Length <= _at);
		return m_DataPtr[_at];	
	}
	
    constexpr const_reference operator[](size_type n) const;
    constexpr const_reference at(size_type n) const;
    constexpr reference       at(size_type n);
    constexpr reference       front();
    constexpr const_reference front() const;
    constexpr reference       back();
    constexpr const_reference back() const;


private:
	T* m_DataPtr;
	int m_Length;
};*/

void onLoadGame(int _id, char* _data, int _length)
{
	// thanks to https://stackoverflow.com/a/1449527/4691843
	struct OneShotReadBuf : public std::streambuf
	{
	    OneShotReadBuf(char* s, std::size_t n)
	    {
	        setg(s, s, s + n);
	    }
	};
	OneShotReadBuf buffer(_data, _length);
	std::istream in(&buffer);
	in >>globalCostCalculator;

	globalPathMgr.clear();
}

void onSaveGame(int id, void __cdecl(*savePluginData)(char*data, int length))
{
	std::ostringstream out(std::ios_base::out | std::ios_base::binary);
	out << globalCostCalculator;
	savePluginData(out.str().data(), std::size(out.str()));
}

void onNewGame()
{
	globalCostCalculator.clear();
	globalPathMgr.clear();
}
