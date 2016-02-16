include_directories(${PROJECT_SOURCE_DIR}/external ${PROJECT_SOURCE_DIR}/src)

# Example main app
set(main_sources main.cpp)
add_executable(${PROJECT_NAME_LOWER}_main ${main_sources})
target_link_libraries(${PROJECT_NAME_LOWER}_main ${PROJECT_NAME_LOWER} ${Boost_LIBRARIES})

# Pipeline playground
set(pipeline_sources pipeline.cpp)
add_executable(pipeline ${pipeline_sources})
target_link_libraries(pipeline ${PROJECT_NAME_LOWER} ${Boost_LIBRARIES} ${ZMQPP_LIBRARIES} ${ZEROMQ_LIBRARIES})
