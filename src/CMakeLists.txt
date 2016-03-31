set(SOURCE_FILES
        circbuf.cpp
        ctl_messages.cpp
        rmawindow.cpp
        scifnode.cpp
        hbsocket.cpp
        virt_circbuf.cpp
        trans4scif.cpp)

set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS  "-finstrument-functions -g")

add_library(trans4scif SHARED ${SOURCE_FILES})

set(TRANS4SCIF_VERSION ${TRANS4SCIF_VERSION_MAJOR}.${TRANS4SCIF_VERSION_MINOR})
set_target_properties(trans4scif PROPERTIES VERSION ${TRANS4SCIF_VERSION}
        SOVERSION ${TRANS4SCIF_VERSION})
# Installation
install(TARGETS trans4scif
        LIBRARY DESTINATION lib)
