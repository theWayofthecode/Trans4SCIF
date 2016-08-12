set(TEST_SOURCE_FILES
        test_trans4scif.cpp
        test_util.cpp
        test_ctl_messages.cpp
        test_scifnode.cpp
        test_rmarecords.cpp
        test_hbsocket.cpp
        main.cpp)

include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(main_test ${TEST_SOURCE_FILES})
target_link_libraries(main_test trans4scif scif pthread)
