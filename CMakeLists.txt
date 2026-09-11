cmake_minimum_required(VERSION 3.30)

project(kv_trace_replay LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

# dependencies
include(FetchContent)
FetchContent_Declare(
   httplib
   GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
   GIT_TAG v0.54.1
)
FetchContent_MakeAvailable(httplib)

# build options
if (NOT CMAKE_BUILD_TYPE)
   message(STATUS "CMAKE_BUILD_TYPE not specified. Defaulting to 'Release'.")
   set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
)

set(HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/request.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bounded_queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_producer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_source.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_player.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trace_error.hpp
)

add_executable(kv_trace_replay ${SRCS} ${HDRS})

target_link_libraries(kv_trace_replay
    PRIVATE
        httplib
        Boost::program_options
        Threads::Threads
        memcached
)

# target_link_options(kv_trace_replay PRIVATE -"static")