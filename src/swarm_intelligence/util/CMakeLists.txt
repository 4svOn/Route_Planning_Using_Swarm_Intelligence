cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ${PROJECT_NAME}_SI_util)

add_library(${LIB_NAME} STATIC
    # include/util.hpp
    src/util.cpp
)

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
)
