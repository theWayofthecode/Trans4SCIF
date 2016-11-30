set(SOURCE_FILES
        ctl_messages.cpp
        rmawindow.cpp
        mmapmem.cpp
        rmarecordsreader.cpp
        rmarecordswriter.cpp
        scifnode.cpp
        hbsocket.cpp
        trans4scif.cpp)

add_library(trans4scif SHARED ${SOURCE_FILES})

set(TRANS4SCIF_VERSION ${TRANS4SCIF_VERSION_MAJOR}.${TRANS4SCIF_VERSION_MINOR})

set_target_properties(trans4scif PROPERTIES VERSION ${TRANS4SCIF_VERSION}
        SOVERSION ${TRANS4SCIF_VERSION})



target_link_libraries(trans4scif pthread)
# Installation

install(TARGETS trans4scif LIBRARY DESTINATION ${INSTALL_PATH})
