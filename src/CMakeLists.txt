set(SOURCE_FILES
        circbuf.cpp
        ctl_messages.cpp
        rmawindow.cpp
        scifnode.cpp
        hbsocket.cpp
        virt_circbuf.cpp
        trans4scif.cpp)

add_library(trans4scif ${SOURCE_FILES})
