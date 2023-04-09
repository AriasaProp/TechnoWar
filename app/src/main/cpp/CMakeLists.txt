cmake_minimum_required(VERSION 3.4.3)

project(vktuts)

find_package(game-activity REQUIRED CONFIG)

add_library(${CMAKE_PROJECT_NAME} SHARED
    CreateShaderModule.cpp
    VulkanMain.cpp
    vulkan_wrapper/vulkan_wrapper.cpp
    AndroidMain.cpp
    src/GameActivitySources.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    src
    .
    ${ANDROID_NDK}/sources/android/native_app_glue
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unused-variable -Wno-delete-non-virtual-dtor -DVK_USE_PLATFORM_ANDROID_KHR")

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} \
        -mhard-float -D_NDK_MATH_NO_SOFTFP=1 -mfloat-abi=hard")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
        -Wl,--no-warn-mismatch")
endif()
add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

target_link_libraries(${CMAKE_PROJECT_NAME}
    game-activity::game-activity
    log android shaderc)
