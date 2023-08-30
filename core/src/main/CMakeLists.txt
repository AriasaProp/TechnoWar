cmake_minimum_required(VERSION 3.10.2)

project(core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

execute_process(COMMAND date "+%d/%m/%Y" OUTPUT_VARIABLE Build_Date OUTPUT_STRIP_TRAILING_WHITESPACE)
message("Release date is: ${Build_Date}")
add_definitions(-DBuild_Date=${Build_Date})

file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
include_directories(${Include_Dir})
file(GLOB_RECURSE Src_Files "*.cpp")

add_library(${PROJECT_NAME} SHARED ${Src_Files})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

#target_compile_options(${PROJECT_NAME} PRIVATE -DBuild_Date=${Build_Date})
#target_compile_definitions(${PROJECT_NAME} PRIVATE Build_Date=${Build_Date})
