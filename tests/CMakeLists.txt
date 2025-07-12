cmake_minimum_required(VERSION 3.22.1)
project(tests LANGUAGES C)

# Flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(COMPILE_WARNING_AS_ERROR ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core ${CMAKE_CURRENT_SOURCE_DIR}/bin/core)

## Tests
file(GLOB_RECURSE SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/src/*.c
)
list(REMOVE_ITEM SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/src/main.c
)
add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE core)
