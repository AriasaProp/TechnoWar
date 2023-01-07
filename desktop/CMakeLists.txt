cmake_minimum_required (VERSION 3.4.1)
project (DesktopApp)

set(CORE_SRC "../../../../core")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp")

add_subdirectory(${CORE_SRC} bin/core)

add_executable(DesktopApp ${Src_Files})

target_include_directories(ext PRIVATE ${Include_Dir} ${Include_subDir})
target_link_libraries(DesktopApp core)