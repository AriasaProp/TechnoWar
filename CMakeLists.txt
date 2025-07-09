cmake_minimum_required(VERSION 3.22)
project(
  TechnoWar
  VERSION 0.0.1
	DESCRIPTION "Game"
	LANGUAGES C
)

# Flags
set(CMAKE_BUILD_TYPE MinSizeRel)
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
set(CORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/core")
set(CORE_PATH_INCLUDE "${CORE_PATH}/include")
file(GLOB_RECURSE CORE_SRCS "${CORE_PATH}/src/*.c")
add_library(core STATIC ${CORE_SRCS})
target_include_directories(core PUBLIC ${CORE_PATH_INCLUDE})
target_link_libraries(core PUBLIC m)

## Tools
set(TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")
set(TOOLS_PATH_INCLUDE "${TOOLS_PATH}/include")
file(GLOB_RECURSE TOOLS_SRCS "${TOOLS_PATH}/src/*.c")
add_executable(tools ${TOOLS_SRCS})
target_include_directories(tools PRIVATE ${TOOLS_PATH_INCLUDE} ${CORE_PATH_INCLUDE})
target_link_libraries(tools PRIVATE core)

## Tests
set(TESTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(TESTS_PATH_INCLUDE "${TESTS_PATH}/include")
file(GLOB_RECURSE TESTS_SRCS "${TESTS_PATH}/src/*.c" "${TOOLS_PATH}/src/*.c")
list(REMOVE_ITEM TESTS_SRCS "${TOOLS_PATH}/src/main.c")
add_executable(tests ${TESTS_SRCS})
target_include_directories(tests PRIVATE ${TESTS_PATH_INCLUDE} ${TOOLS_PATH_INCLUDE} ${CORE_PATH_INCLUDE})
target_link_libraries(tests PRIVATE core)

## Android Backend
set(ANDROID_BACKEND_PATH "${CMAKE_CURRENT_SOURCE_DIR}/android/backend")
file(GLOB_RECURSE ANDROID_BACKEND_SRCS "${ANDROID_BACKEND_PATH}/*.c")

add_library(android_backend SHARED ${ANDROID_BACKEND_SRCS})
target_compile_definitions(android_backend PRIVATE 
    ANDROID_STL=c_static
    _IDENTITY_=${BUILD_ID}
)
target_compile_options(android_backend PRIVATE
    -fexceptions -frtti
)
target_include_directories(android_backend PRIVATE ${ANDROID_BACKEND_PATH} ${CORE_PATH_INCLUDE})
target_link_libraries(android_backend PRIVATE core GLESv3 android EGL log m)
target_link_options(android_backend PRIVATE -u ANativeActivity_onCreate)
