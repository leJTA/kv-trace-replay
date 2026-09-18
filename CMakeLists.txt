cmake_minimum_required(VERSION 3.30)

project(kv_trace_replay LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

# dependencies
include(FetchContent)
FetchContent_Declare(
   httplib
   GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
   GIT_TAG v0.56.0
)
FetchContent_MakeAvailable(httplib)
FetchContent_Declare(
   hdr_histogram
   GIT_REPOSITORY https://github.com/HdrHistogram/HdrHistogram_c.git
   GIT_TAG 0.11.10
)
FetchContent_MakeAvailable(hdr_histogram)

find_package(RocksDB REQUIRED)

# build options
if (NOT CMAKE_BUILD_TYPE)
   message(STATUS "CMAKE_BUILD_TYPE not specified. Defaulting to 'Release'.")
   set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

set(TRACE_REPLAY_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_replay.m.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/request.cpp
)
set(TRACE_REPLAY_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/request.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bounded_queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_producer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sender_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_source.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_player.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_error.hpp
)

set(TRACE_PRELOAD_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_preload.m.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/request.cpp
)
set(TRACE_PRELOAD_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/request.hpp
)

set(TRACE_CLIENT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_client.m.cpp
)

add_executable(kv_trace_replay ${TRACE_REPLAY_SRCS} ${TRACE_REPLAY_HDRS})
add_executable(kv_trace_preload ${TRACE_PRELOAD_SRCS} ${TRACE_PRELOAD_HDRS})
add_executable(kv_trace_client ${TRACE_CLIENT_SRCS})

target_link_libraries(kv_trace_replay
    PRIVATE
        httplib
        Boost::program_options
        Threads::Threads
        hdr_histogram::hdr_histogram_static
)

target_link_libraries(kv_trace_preload
    PRIVATE
        Boost::program_options
        RocksDB::rocksdb
)

target_link_libraries(kv_trace_client
    PRIVATE
        httplib
        Boost::program_options
        RocksDB::rocksdb
        memcached
        hdr_histogram::hdr_histogram_static
)

# target_link_options(kv_trace_replay PRIVATE -"static")