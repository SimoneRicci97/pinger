cmake_minimum_required(VERSION 3.5)

set(CC gcc CACHE STRING "Default C compiler")
set(CMAKE_CFLAGS "std=c99 -Wall -pedantic -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE -I./lib -L." STRING "Default gcc flags")

project(pinger VERSION 0.0.1)

configure_file(pinger_config.h.in pinger_config.h)
include_directories(${PROJECT_BINARY_BIN})

#target_include_directories(pinger PUBLIC
#                           "/home/simone/Scrivania/pinger/")

add_library(pinglib lib/ping_list.c lib/ping_list.h lib/pscheck.c lib/pscheck.h)
add_library(utils lib/chout.c lib/chout.h lib/string_utils.c lib/string_utils.h lib/htable.c lib/htable.h lib/configuration.c lib/configuration.h)

add_executable(pinger main.c)

target_link_libraries(pinger PUBLIC pinglib)
target_link_libraries(pinger PUBLIC utils)

target_include_directories(pinger PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/lib"
                          )