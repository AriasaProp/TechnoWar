cmake_minimum_required(VERSION 3.4.1)

project(core LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
include_directories(${Include_Dir})
file(GLOB_RECURSE Src_Files "*.cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
add_library(${PROJECT_NAME} SHARED ${Src_Files})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
