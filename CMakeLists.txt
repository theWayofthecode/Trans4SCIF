cmake_minimum_required(VERSION 3.1)
project(trans4scif)

# Configuration
set (TRANS4SCIF_VERSION_MAJOR 2)
set(TRANS4SCIF_VERSION_MINOR 3)

if (DEFINED ENV{T4S_RECV_BUF_SIZE})
    message("RECV_BUF_SIZE = $ENV{T4S_RECV_BUF_SIZE} * PAGE_SIZE")
    set(RECV_BUF_SIZE "$ENV{T4S_RECV_BUF_SIZE} * PAGE_SIZE")
else ()
    message("RECV_BUF_SIZE = 1024 * PAGE_SIZE")
    set(RECV_BUF_SIZE "1024 * PAGE_SIZE")
endif ()

configure_file (
        "${PROJECT_SOURCE_DIR}/src/trans4scif_config.h.in"
        "${PROJECT_SOURCE_DIR}/src/trans4scif_config.h"
)

include_directories(
        "${PROJECT_BINARY_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
add_subdirectory(test)

# Installation
install(FILES include/trans4scif.h DESTINATION include)
