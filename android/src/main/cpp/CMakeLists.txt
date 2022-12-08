cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o3 -std=gnu++11 -Wall -Werror")
set(CORE_SRC "../../../../core")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp *.vert *.frag")

add_subdirectory(${CORE_SRC} bin/core)

add_library(ext SHARED ${Src_Files})

target_include_directories(ext PRIVATE ${Include_Dir} ${Include_subDir} ${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(ext native_app_glue core GLESv3 android EGL log)

