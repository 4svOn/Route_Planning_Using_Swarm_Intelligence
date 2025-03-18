cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ${PROJECT_NAME}_base)

add_library(${LIB_NAME} STATIC
    # include/util.hpp
    src/coordinates.cpp
    src/random.cpp
    src/ranges.cpp
)

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(${LIB_NAME} PUBLIC ${PROJECT_NAME}_proto)
