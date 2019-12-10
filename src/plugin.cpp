//#define AUTO_DLLMAIN
//#define CUSTOM_DLLMAIN
//#define _GLIBCXX_USE_CXX11_ABI 0

#include "Simple-Graph/Searcher.hpp"

#include <DynRPG/DynRPG.h>
#include <cassert>
#include <iostream>
#include <string_view>

#include "Vector.hpp"

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
	int calc_path(const Vector& _end)
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
		auto searcher = sl::graph::make_breadth_first_searcher<Vector>();
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
		auto path = sl::graph::find_path(start, _end, neighbourSearcher, searcher, NodeMap(Map_t(VectorLess())), NodeMap(Map_t(VectorLess())));
		if (path)
			return std::size(*path);
		return -1;
	}
	
private:
	
};

// called on comment
bool onComment(const char* _text, const RPG::ParsedCommentData* _parsedData, RPG::EventScriptLine* _nextScriptLine,
	RPG::EventScriptData* _scriptData, int _eventId, int _pageId, int _lineId, int* _nextLineId)
{
	std::string_view cmd{ _parsedData->command };
	if (cmd == "find_path")
	{
		auto& params = _parsedData->parameters;
		Pathfinder p;
		auto length = p.calc_path({ 20, 20 });
		RPG::variables[1] = length;
		return false;
	}
	return true;
}
