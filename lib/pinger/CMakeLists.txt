cmake_minimum_required(VERSION 3.5)

set(CC gcc CACHE STRING "Default C compiler")
set(CMAKE_CFLAGS "-I./lib -L." STRING "Default gcc flags")
set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall)
add_compile_options(-pedantic)
add_compile_options(-g)
add_compile_options(-O3)
add_compile_options(-DMAKE_VALGRIND_HAPPY)
add_compile_options(-D_GNU_SOURCE)

add_library(pinglib ping_list.c ping_list.h pscheck.c pscheck.h job.c job.h pinger_signal.c pinger_signal.h alarm_handler.h pinger_report.c pinger_report.h pinger_archive.c pinger_archive.h)

target_include_directories(pinglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pinglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../utils)