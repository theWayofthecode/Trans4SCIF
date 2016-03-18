cmake_minimum_required(VERSION 3.1)
project(trans4scif)

# The version number.
set (TRANS4SCIF_VERSION_MAJOR 0)
set (TRANS4SCIF_VERSION_MINOR 1)

configure_file (
        "${PROJECT_SOURCE_DIR}/include/trans4scif_config.hpp.in"
        "${PROJECT_BINARY_DIR}/include/trans4scif_config.hpp"
)

include_directories(
        "${PROJECT_BINARY_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
add_subdirectory(test)

enable_testing()
add_test(version test/main_test)
add_test(util test/main_test [util])
add_test(ctl_messages test/main_test [ctl_messages])