cmake_minimum_required (VERSION 3.4.1)

if (MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /WX /EHsc /nologo /Zc:wchar_t /Zc:forScope /Zc:inline /GS /D \"_UNICODE\" /D \"UNICODE\"")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /MTd /D \"_DEBUG\"")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /MT /D \"NDEBUG\"")
else()
    set(CMAKE_CXX_FLAGS "-std=c++1z -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ../../bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../../bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ../../lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ../../lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ../../lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ../../lib)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)
endif()

set(CMAKE_DEBUG_POSTFIX "d")

project (DesktopApp)

set(CORE_SRC "../core")

file(GLOB_RECURSE Include_subDir LIST_DIRECTORIES true ${CORE_SRC})
file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp")

add_subdirectory(${CORE_SRC} bin/core)

add_executable(DesktopApp ${Src_Files})

target_include_directories(ext PRIVATE ${Include_Dir} ${Include_subDir})
target_link_libraries(DesktopApp core)