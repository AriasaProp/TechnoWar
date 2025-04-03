cmake_minimum_required(VERSION 3.20)
project(core 
		VERSION 0.0.1
		DESCRIPTION "Main game logic"
		LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

file(GLOB_RECURSE Src "src/*.c")

add_library(${PROJECT_NAME} STATIC ${Src})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC m)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE BUILD_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_BRANCH=${BUILD_BRANCH})
