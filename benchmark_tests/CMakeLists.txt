cmake_minimum_required(VERSION 3.14)
project(Route_Planning_Using_Swarm_Intelligence_Benchmark_Tests CXX)

set(CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} main.cpp)

# Add these include directories before adding subdirectories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../proto
    ${CMAKE_CURRENT_BINARY_DIR}/proto_build
)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/swarm_intelligence/ACO
    ${CMAKE_CURRENT_BINARY_DIR}/ACO_build
)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/swarm_intelligence/PSO
    ${CMAKE_CURRENT_BINARY_DIR}/PSO_build
)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/swarm_intelligence/common
    ${CMAKE_CURRENT_BINARY_DIR}/common_build
)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/base
    ${CMAKE_CURRENT_BINARY_DIR}/base_build
)


target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/proto_build/cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Route_Planning_Using_Swarm_Intelligence_Benchmark_Tests_SI_ACO
    Route_Planning_Using_Swarm_Intelligence_Benchmark_Tests_SI_PSO
    Route_Planning_Using_Swarm_Intelligence_Benchmark_Tests_SI_common
    Route_Planning_Using_Swarm_Intelligence_Benchmark_Tests_proto
    Route_Planning_Using_Swarm_Intelligence_Benchmark_Tests_base

)
