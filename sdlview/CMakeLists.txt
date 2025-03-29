cmake_minimum_required(VERSION 3.10)

project(sdlview
		VERSION 0.0.1
		DESCRIPTION "Window logic"
		LANGUAGES C)

find_package(SDL2 REQUIRED)
add_definitions(-DGLES2)
file(GLOB_RECURSE SRC "src/*.c")
add_executable(${PROJECT_NAME} ${SRC})
include_directories(${PROJECT_NAME} include)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})


