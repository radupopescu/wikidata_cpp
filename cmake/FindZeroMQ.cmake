find_path(
    ZEROMQ_INCLUDE_DIRS
    NAMES zmq.h
    HINTS ${ZEROMQ_INCLUDE_DIRS}
)

find_library(
    ZEROMQ_LIBRARIES
    NAMES zmq
    HINTS ${ZEROMQ_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    ZEROMQ
    DEFAULT_MSG
    ZEROMQ_LIBRARIES
    ZEROMQ_INCLUDE_DIRS
)

if(ZEROMQ_FOUND)
    mark_as_advanced(ZEROMQ_LIBRARIES ZEROMQ_INCLUDE_DIRS)
endif()