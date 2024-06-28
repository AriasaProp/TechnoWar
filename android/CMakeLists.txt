cmake_minimum_required(VERSION 3.10.2)

project(ext LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -O3 -Wall -Wextra -Werror \
    -DANDROID_STL=c++_static \
    -D_IDENTITY_=${BUILD_ID} \
    -fexceptions -frtti")
set(CORE_SRC "../core/main")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true "src/main/cpp")
file(GLOB_RECURSE Src_Files "src/main/cpp/**/*.cpp")
add_subdirectory(${CORE_SRC} bin/core)

include_directories(${Include_Dir})
include_directories(${Include_subDir})
add_library(${PROJECT_NAME} SHARED ${Src_Files})
target_link_libraries(${PROJECT_NAME} core GLESv3 android EGL log)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)


# Add a custom target to compile GLSL shaders
# Add custom commands to compile each GLSL file
# add_custom_target(Shaders)
# file(GLOB_RECURSE SHADER_SOURCES "shaders/*.glsl")
# foreach(SHADER_SOURCE ${SHADER_SOURCES})
#    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WE)
#    set(SHADER_OUTPUT "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
#    add_custom_command(
#        TARGET Shaders
#        COMMAND glslc -o ${SHADER_OUTPUT} ${SHADER_SOURCE}
#        DEPENDS ${SHADER_SOURCE}
#     )
# endforeach()