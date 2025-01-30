cmake_minimum_required(VERSION 3.20)
project(core LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/main")
file(GLOB_RECURSE Src_Files "${SRC}/src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC}/include)
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE BUILD_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_BRANCH=${BUILD_BRANCH})
