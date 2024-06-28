cmake_minimum_required(VERSION 3.10.2)

project(ext LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

# build cpufeatures as a static lib
#add_library(cpufeatures STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -O3 -Wall -Wextra -Werror \
    -DANDROID_STL=c++_static \
    -D_IDENTITY_=${BUILD_ID} \
    -fexceptions -frtti")
set(CORE_SRC "../core/main")

# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true "main/cpp")
file(GLOB_RECURSE Src_Files "src/main/cpp/**/*.cpp")
add_subdirectory(${CORE_SRC} bin/core)

include_directories(${Include_Dir})
include_directories(${Include_subDir})

add_library(${PROJECT_NAME} SHARED ${Src_Files})

target_link_libraries(${PROJECT_NAME}
# core project library
                       core
# android system library (main)
                       GLESv3 android EGL log
                       )
