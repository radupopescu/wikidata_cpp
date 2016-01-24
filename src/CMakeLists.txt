include_directories(${PROJECT_SOURCE_DIR}/external ${PROJECT_SOURCE_DIR}/src)

set(lib${PROJECT_NAME_LOWER}_sources data.cpp io.cpp processing.cpp utils.cpp)

add_library(${PROJECT_NAME_LOWER} ${lib${PROJECT_NAME_LOWER}_sources})
