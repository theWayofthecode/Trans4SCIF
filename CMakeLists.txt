cmake_minimum_required(VERSION 3.3)
project(trans4scif)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/circbuf.cpp
    src/circbuf.hpp
    src/constants.hpp
    src/ctl_messages.cpp
    src/ctl_messages.hpp
    src/rmawindow.cpp
    src/rmawindow.hpp
    src/scifepd.hpp
    src/scifnode.cpp
    src/scifnode.hpp
    src/test.c
    src/trans4node.cpp
    src/trans4node.hpp
    src/util.hpp
    src/virt_circbuf.cpp
    src/virt_circbuf.hpp)

add_executable(trans4scif ${SOURCE_FILES})