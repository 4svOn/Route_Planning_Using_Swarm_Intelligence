cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ${PROJECT_NAME}_server)

add_library(${LIB_NAME} STATIC
    # include/util.hpp
    src/hello.cpp
)

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(${LIB_NAME} PUBLIC userver::core)

target_link_libraries(${LIB_NAME} PUBLIC ${PROJECT_NAME}_SI_ACO)
target_link_libraries(${LIB_NAME} PUBLIC ${PROJECT_NAME}_proto)
target_link_libraries(${LIB_NAME}  PRIVATE ${PROJECT_NAME}_OSRM)

