cmake_minimum_required(VERSION 4.2)
project(lv220c C)

set(CMAKE_C_STANDARD 11)

find_library(LIBM m)

add_executable(lv220c source/main.c
        source/v220_decoder.c
        source/lmath.c)

target_include_directories(lv220c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lv220c ${LIBM})

install(TARGETS lv220c DESTINATION bin)