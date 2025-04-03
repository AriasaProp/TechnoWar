cmake_minimum_required(VERSION 3.20)
project(tools
		VERSION 0.0.1
		DESCRIPTION "Tools logic"
		LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

set(CORESRC ../core)
add_subdirectory(../core build/core)

file(GLOB_RECURSE SRC "main/src/*.c")
add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "main/include" "${CORESRC}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE core)

#Test
file(GLOB_RECURSE Test_Src "test/src/*.c")
file(GLOB_RECURSE TTest_Src "main/src/stb/.c")
add_executable(${PROJECT_NAME}_test ${Test_Src} ${TTest_Src})
target_include_directories(${PROJECT_NAME}_test PRIVATE "main/include" "test/include" "${CORESRC}/include")
target_link_libraries(${PROJECT_NAME}_test PRIVATE core)

#Cxxdroid-binary: core_test