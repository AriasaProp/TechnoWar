cmake_minimum_required (VERSION 3.10.2)

project(Converter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

if (MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /O2 /Oi /MT /EHsc /nologo /Zc:wchar_t /Zc:forScope /Zc:inline /GS /D \"_UNICODE\" /D \"UNICODE\"")
else()
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3")
endif()

file(GLOB_RECURSE Include_Dir LIST_DIRECTORIES true ".")
file(GLOB_RECURSE Src_Files "*.cpp")

include_directories(${Include_Dir})

if(MSVC)
	add_executable(${PROJECT_NAME} WIN32 main.cpp Src_Files)
else()
	add_executable(${PROJECT_NAME} main.cpp Src_Files)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)