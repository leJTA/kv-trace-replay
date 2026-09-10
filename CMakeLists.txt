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
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config.cpp
)

set(HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/request.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bounded_queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trace_producer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/client_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data_source.hpp
)

add_executable(kv_trace_replay ${SRCS} ${HDRS})

target_link_libraries(kv_trace_replay
    PRIVATE
        httplib
        Boost::program_options
        Threads::Threads
)

# target_link_options(kv_trace_replay PRIVATE -"static")