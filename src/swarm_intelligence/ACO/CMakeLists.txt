cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ${PROJECT_NAME}_SI_ACO)

add_library(${LIB_NAME} STATIC
    # include/ant.hpp
    src/solver.cpp
    src/ant.cpp
    src/util.cpp
)

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
)

target_link_libraries(${LIB_NAME} PUBLIC ${PROJECT_NAME}_base)
target_link_libraries(${LIB_NAME} PUBLIC ${PROJECT_NAME}_SI_common)
