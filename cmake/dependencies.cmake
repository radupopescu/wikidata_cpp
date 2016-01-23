# Find dependencies

# Boost iostreams
find_package(Boost REQUIRED iostreams)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Threads (need to do this explicitly on Linux?)
if (UNIX AND NOT APPLE)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)
    if(CMAKE_USE_PTHREADS_INIT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()
endif()

