cmake_minimum_required(VERSION 3.4.1)

project(ext LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)

set(CORE_SRC "../../../../core")
file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
add_subdirectory(${CORE_SRC} bin/core)

file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
include_directories(${Include_Dir} ${Include_subDir})
file(GLOB_RECURSE Src_Files "*.cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
add_library(${PROJECT_NAME} SHARED ${Src_Files})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
# include external libraries from core and ndk extras
target_link_libraries(${PROJECT_NAME} PRIVATE core GLESv3 android EGL log m)

