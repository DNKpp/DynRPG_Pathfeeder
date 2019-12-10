cmake_minimum_required (VERSION 3.8)

# THIS HAS TO COME BEFORE THE PROJECT LINE
#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")
# THIS HAS TO COME BEFORE THE PROJECT LINE

project(DynRPG_Pathfinder LANGUAGES C CXX)

#enable_testing()

add_subdirectory("src")
