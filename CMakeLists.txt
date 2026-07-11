cmake_minimum_required(VERSION 3.15)
project(lv220c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(lv220c_core STATIC
    source/lmath.c
    source/v220_decoder.c
    source/map_transformer.c
    source/lun_file.c
    include/lmath.h
    include/v220_decoder.h
    include/map_transformer.h
    include/lun_file.h
)

target_include_directories(lv220c_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(lv220c_core PRIVATE /W4)
else()
    target_compile_options(lv220c_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(NOT MSVC)
    target_link_libraries(lv220c_core PRIVATE m)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(lv220c_core PRIVATE rt)
endif()

add_executable(lv220c source/main.c)
target_link_libraries(lv220c PRIVATE lv220c_core)

install(TARGETS lv220c DESTINATION bin)

enable_testing()

add_executable(test_csg source/test/test_csg.c)
target_link_libraries(test_csg PRIVATE lv220c_core)

add_test(NAME CSG_Math_Test COMMAND test_csg)