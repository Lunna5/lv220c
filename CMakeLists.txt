cmake_minimum_required(VERSION 4.2)
project(lv220c C)

set(CMAKE_C_STANDARD 11)

add_executable(lv220c source/main.c
        source/lv220c.c)

target_include_directories(lv220c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS lv220c DESTINATION bin)