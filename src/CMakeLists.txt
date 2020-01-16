cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project(Pathfeeder)

add_library(Pathfeeder SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp"
)

set_property(TARGET Pathfeeder
    PROPERTY CXX_STANDARD 17
)

target_link_directories(Pathfeeder
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/DynRPG_0_32/sdk/lib"
)

TARGET_LINK_LIBRARIES(Pathfeeder
	PUBLIC debug DynRPGd
	PUBLIC optimized DynRPG
)

target_include_directories(Pathfeeder
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/Simple-Graph/include"
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/Simple-Utility/include"
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/DynRPG_0_32/sdk/include"
)

add_custom_command(TARGET Pathfeeder
	COMMAND ${CMAKE_COMMAND} -E copy
		${Pathfinder_BINARY_DIR}Pathfeeder.dll
		${CMAKE_CURRENT_SOURCE_DIR}/../TestGame/DynPlugins
	COMMAND ${CMAKE_COMMAND} -E copy
		${Pathfinder_BINARY_DIR}Pathfeeder.dll
		${CMAKE_CURRENT_SOURCE_DIR}/../ExampleGame/DynPlugins
)
