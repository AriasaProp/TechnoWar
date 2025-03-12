cmake_minimum_required(VERSION 3.20)
project(core 
		VERSION 0.0.1
		DESCRIPTION "Main game logic"
		LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/main")
file(GLOB_RECURSE Src_Files "${SRC}/src/*.c")

add_library(${PROJECT_NAME} STATIC ${Src_Files})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC m)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE BUILD_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_BRANCH=${BUILD_BRANCH})

#Test
set(TSRC ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(GLOB_RECURSE Test_Src "${TSRC}/src/*.c")
add_executable(${PROJECT_NAME}_test ${Test_Src})
target_compile_definitions(${PROJECT_NAME}_test PRIVATE NDEBUG=1)

target_include_directories(${PROJECT_NAME}_test PRIVATE "${SRC}/include" "${TSRC}/include")
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})

