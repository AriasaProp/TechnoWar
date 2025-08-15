cmake_minimum_required(VERSION 3.22.1)
project(tools LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSION OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(COMPILE_WARNING_AS_ERROR ON)
# rename
set(HERE ${CMAKE_CURRENT_SOURCE_DIR})

# sets output place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${HERE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${HERE}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${HERE}/build)

set(CORE_DIR ${HERE}/../core)
add_subdirectory(${CORE_DIR} ${HERE}/core)

file(GLOB_RECURSE SRCS ${HERE}/src/*.c)

if (DO_TEST)
    enable_testing()
    set(TEST_DIR ${HERE}/test_src)
    list(REMOVE_ITEM SRCS ${HERE}/src/main.c)
    
    # test rect pack
    add_executable(rect_pack ${SRCS}
       ${TEST_DIR}/stb/rect_pack_test.c
    )
    target_link_libraries(rect_pack PRIVATE core)
    target_include_directories(rect_pack PRIVATE
       ${HERE}/include
       ${CORE_DIR}/include
    )
    add_test(NAME "RECT PACK" COMMAND rect_pack)
    
    # test decoder
    add_executable(decoder ${SRCS}
       ${TEST_DIR}/decoder_test.c
    )
    target_link_libraries(decoder PRIVATE core)
    target_include_directories(decoder PRIVATE
       ${HERE}/include
       ${CORE_DIR}/include
    )
    file(GLOB IMGS ${HERE}/tests_data/*.png)
    add_test(
       NAME "Decoder Test"
       COMMAND decoder ${IMGS}
       WORKING_DIRECTORY ${HERE}
    )
    
    add_executable(fasttest ${TEST_DIR}/fasttest.c)
    add_test(
      NAME "Fast Test"
      COMMAND fasttest
    )
    
else()
   # build tools
   add_executable(${PROJECT_NAME} ${SRCS})
   target_include_directories(${PROJECT_NAME} PRIVATE
       ${HERE}/include
       ${CORE_DIR}/include
   )
   target_link_libraries(${PROJECT_NAME} PRIVATE core)
endif ()