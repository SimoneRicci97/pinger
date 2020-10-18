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

add_executable(test_pthreadpool pthreadpool_test.c)

target_link_libraries(test_pthreadpool PUBLIC pthread)
target_link_libraries(test_pthreadpool PUBLIC pthreadpool)

target_include_directories(test_pthreadpool PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/lib/pthreadpool"
                          )