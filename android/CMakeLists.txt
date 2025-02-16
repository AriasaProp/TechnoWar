cmake_minimum_required(VERSION 3.20)

project(ext LANGUAGES C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -O3 -Wall -Wextra -Werror \
    -DANDROID_STL=c_static \
    -D_IDENTITY_=${BUILD_ID} \
    -fexceptions -frtti")
    
set(CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../core")
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main/ext/")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
file(GLOB_RECURSE Src_Files "${SRC}/*.c")

add_subdirectory(${CORE_SRC} "${CMAKE_CURRENT_SOURCE_DIR}/bin/core")

add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_include_directories(${PROJECT_NAME} PRIVATE "${CORE_SRC}/main/include")
target_link_libraries(${PROJECT_NAME} core GLESv3 android EGL log m)
