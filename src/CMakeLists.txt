set(SOURCE_FILES
        circbuf.cpp
        ctl_messages.cpp
        rmawindow.cpp
        scifnode.cpp
        hbsocket.cpp
        virt_circbuf.cpp)

add_library(trans4scif ${SOURCE_FILES})
