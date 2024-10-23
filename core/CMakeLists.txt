cmake_minimum_required(VERSION 3.10.2)

project(core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

MACRO (TODAY BUILD_DATE)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${BUILD_DATE})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${BUILD_DATE} "${${BUILD_DATE}}")
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${BUILD_DATE})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${BUILD_DATE} "${${BUILD_DATE}}")
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${BUILD_DATE} "No Date Implemented")
    ENDIF (WIN32)
ENDMACRO (TODAY)

TODAY(BUILD_DATE)
add_definitions(-DBUILD_DATE=${BUILD_DATE})

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE BUILD_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DBUILD_BRANCH=${BUILD_BRANCH})

file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true "main")
include_directories(${Include_Dir})
file(GLOB_RECURSE Src_Files "main/*.cpp")

add_library(${PROJECT_NAME} SHARED ${Src_Files})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

#target_compile_options(${PROJECT_NAME} PRIVATE -DBuild_Date=${Build_Date})
#target_compile_definitions(${PROJECT_NAME} PRIVATE Build_Date=${Build_Date})
