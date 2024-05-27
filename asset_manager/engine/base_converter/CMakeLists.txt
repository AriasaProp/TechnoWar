cmake_minimum_required (VERSION 3.10.2)

project(base_converter LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

# Build the executable program
if(MSVC)
	add_executable(${PROJECT_NAME} WIN32 "*.hpp" "*.cpp")
else()
	add_executable(${PROJECT_NAME} "*.hpp" "*.cpp")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

