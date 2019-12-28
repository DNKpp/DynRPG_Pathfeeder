cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project(Pathfinder)

add_library(Pathfinder SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp"
)

set_property(TARGET Pathfinder
    PROPERTY CXX_STANDARD 17
)

target_link_directories(Pathfinder
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/DynRPG_0_32/sdk/lib"
)

TARGET_LINK_LIBRARIES(Pathfinder
	PUBLIC DynRPG
)

target_include_directories(Pathfinder
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/Simple-Graph/include"
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/DynRPG_0_32/sdk/include"
)

add_custom_command(TARGET Pathfinder
	COMMAND ${CMAKE_COMMAND} -E copy
		${Pathfinder_BINARY_DIR}/Pathfinder.dll
		${CMAKE_CURRENT_SOURCE_DIR}/../TestGame/DynPlugins
)
