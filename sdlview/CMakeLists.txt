cmake_minimum_required(VERSION 3.20)
project(sdlview 
		VERSION 0.0.1
		DESCRIPTION "SDL sample logic"
		LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

find_package(SDL2 REQUIRED)
set(CORE_SRC ../core)
add_subdirectory(${CORE_SRC} build/core)

file(GLOB_RECURSE SRCS src/*.c)
add_definitions(-DNDEBUG)
add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE include ${CORE_SRC}/include)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} core)