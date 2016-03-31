cmake_minimum_required(VERSION 3.1)
project(trans4scif)

# The version number.
set (TRANS4SCIF_VERSION_MAJOR 0)
set (TRANS4SCIF_VERSION_MINOR 2)

configure_file (
        "${PROJECT_SOURCE_DIR}/include/trans4scif_config.h.in"
        "${PROJECT_BINARY_DIR}/include/trans4scif_config.h"
)

include_directories(
        "${PROJECT_BINARY_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

add_subdirectory(src)
add_subdirectory(test)

# Testing
enable_testing()
add_test(version test/main_test "Test version")
add_test(util test/main_test [util])
add_test(ctl_messages test/main_test [ctl_messages])
add_test(scif_node test/main_test [scifnode])
add_test(circbuf test/main_test [virt_circbuf], [circbuf])
add_test(hbsocket test/main_test [hbsocket])
add_test(trans4scif test/main_test [trans4scif])

# Installation
install(FILES include/trans4scif.h DESTINATION include)