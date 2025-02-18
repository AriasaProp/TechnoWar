cmake_minimum_required(VERSION 3.20)

project(android_opengles LANGUAGES C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -O3 -Wall -Wextra -Werror \
    -DANDROID_STL=c_static \
    -fexceptions -frtti")
    
file(GLOB_RECURSE Src_Files "*.c")

add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_include_directories(${PROJECT_NAME} PRIVATE ".")
target_link_libraries(${PROJECT_NAME} GLESv3 android EGL)
