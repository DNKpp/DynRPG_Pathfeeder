cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/")
message(NOTICE "dependencies dir: ${DEPENDENCIES_DIR}")

project(Pathfeeder)

add_library(
	Pathfeeder
	SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)

add_subdirectory("${DEPENDENCIES_DIR}/georithm" "${CMAKE_BINARY_DIR}/georithm")
add_subdirectory("${DEPENDENCIES_DIR}/Simple-Graph" "${CMAKE_BINARY_DIR}/Simple-Graph")
add_subdirectory("${DEPENDENCIES_DIR}/Simple-Utility" "${CMAKE_BINARY_DIR}/Simple-Utility")

target_compile_features(
	Pathfeeder
	INTERFACE
	cxx_std_20
)

target_link_directories(
	Pathfeeder
    PUBLIC "${DEPENDENCIES_DIR}/DynRPG_0_32/sdk/lib"
)

TARGET_LINK_LIBRARIES(
	Pathfeeder
	PUBLIC debug DynRPGd
	PUBLIC optimized DynRPG
	PUBLIC georithm
	PUBLIC simple_graph
	PUBLIC simple_utility
)

target_include_directories(
	Pathfeeder
	PUBLIC "${DEPENDENCIES_DIR}/DynRPG_0_32/sdk/include"
)

add_custom_command(TARGET Pathfeeder
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/Pathfeeder.dll
		${CMAKE_CURRENT_SOURCE_DIR}/TestGame/DynPlugins
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_BINARY_DIR}/Pathfeeder.dll
		${CMAKE_CURRENT_SOURCE_DIR}/ExampleGame/DynPlugins
)