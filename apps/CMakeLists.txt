include_directories(${PROJECT_SOURCE_DIR}/external ${PROJECT_SOURCE_DIR}/src)

# Example main app
set(main_sources main.cpp)
add_executable(${PROJECT_NAME_LOWER}_main ${main_sources})
target_link_libraries(${PROJECT_NAME_LOWER}_main ${PROJECT_NAME_LOWER} ${Boost_LIBRARIES})

# Example main app
set(main_pipeline_sources main_pipeline.cpp)
add_executable(${PROJECT_NAME_LOWER}_pipeline ${main_pipeline_sources})
target_link_libraries(${PROJECT_NAME_LOWER}_pipeline ${PROJECT_NAME_LOWER} ${Boost_LIBRARIES} ${ZMQPP_LIBRARIES} ${ZEROMQ_LIBRARIES})

