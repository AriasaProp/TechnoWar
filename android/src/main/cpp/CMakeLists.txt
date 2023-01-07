cmake_minimum_required(VERSION 3.6.0)

project(ext LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o3 -Wall -Werror")
set(CORE_SRC "../../../../core")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp")
add_subdirectory(${CORE_SRC} bin/core)

include_directories(${Include_Dir})
include_directories(${Include_subDir})
add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_link_libraries(${PROJECT_NAME} core GLESv3 android EGL log)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

