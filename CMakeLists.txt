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

project(pinger VERSION 0.0.1)

configure_file(pinger_config.h.in pinger_config.h)
include_directories(${PROJECT_BINARY_BIN})

add_subdirectory(lib/utils)
add_subdirectory(lib/pinger)
add_subdirectory(lib/pthreadpool)

add_executable(pinger main.c)

target_link_libraries(pinger PUBLIC pinglib)
target_link_libraries(pinger PUBLIC utils)
target_link_libraries(pinger PUBLIC pthread)
target_link_libraries(pinger PUBLIC m)
target_link_libraries(pinger PUBLIC pthreadpool)

target_include_directories(pinger PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/lib/utils"
                          "${PROJECT_SOURCE_DIR}/lib/pinger"
                          )