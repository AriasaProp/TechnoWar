cmake_minimum_required(VERSION 3.10.2)

project(ext LANGUAGES CXX)

find_package(Vulkan REQUIRED)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -O3 -Wall -Wextra -Werror \
    -DANDROID_STL=c++_static \
    -DVK_USE_PLATFORM_ANDROID_KHR \
    -DVK_NO_PROTOTYPES \
    -fexceptions -frtti")
set(CORE_SRC "../../../../core")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp")
add_subdirectory(${CORE_SRC} bin/core)

include_directories(${Include_Dir})
include_directories(${Include_subDir})
#include_directories(${Vulkan_INCLUDE_DIRS})
add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_link_libraries(${PROJECT_NAME} core GLESv3 android EGL log vulkan)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)


