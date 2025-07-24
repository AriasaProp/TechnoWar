cmake_minimum_required(VERSION 3.22.1)

project(android-native LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(COMPILE_WARNING_AS_ERROR ON)

add_compile_definitions(
    ANDROID_STL=c_static
    _IDENTITY_=${BUILD_ID}
    $<IF:$<CONFIG:Debug>,_DEBUG,>
)
add_compile_options(
    -fexceptions
    -frtti
)
add_link_options(
    -u ANativeActivity_onCreate
)

set(CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../core")
file(GLOB_RECURSE Src_Files "${CMAKE_CURRENT_SOURCE_DIR}/c/*.c")

add_subdirectory(${CORE_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/bin/core)

add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/c
    ${CORE_SRC}/include
)
target_link_libraries(${PROJECT_NAME} core android log m)
