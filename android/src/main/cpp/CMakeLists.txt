cmake_minimum_required(VERSION 3.6.0)
 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o3 -std=gnu++11 -Wall -Werror")
set(CORE_SRC "../../../../core")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp")
add_subdirectory(${CORE_SRC} bin/core)

include_directories(${Include_Dir})
include_directories(${Include_subDir})
add_library(ext SHARED ${Src_Files})

target_link_libraries(ext core GLESv3 android EGL log)

