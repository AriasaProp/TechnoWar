cmake_minimum_required(VERSION 3.22.1)
project(core LANGUAGES C)

# Flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(COMPILE_WARNING_AS_ERROR ON)

# generate variables
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE GIT_COMMAND_RESULT
)

if(NOT GIT_COMMAND_RESULT EQUAL 0)
    error("Could not determine Git branch. Setting to 'unknown'. Git command result: ${GIT_COMMAND_RESULT}")
    exit()
endif()

# Common variable
add_compile_definitions(
    BUILD_BRANCH=${GIT_BRANCH}
    # send message to log / console
    $<IF:$<CONFIG:Debug>,_DEBUG,>
)

## Core
file(GLOB_RECURSE CORE_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(${PROJECT_NAME} STATIC ${CORE_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC m)
