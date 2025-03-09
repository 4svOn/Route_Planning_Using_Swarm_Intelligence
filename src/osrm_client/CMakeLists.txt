cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ${PROJECT_NAME}_OSRM)

add_library(${LIB_NAME} STATIC
    # include/util.hpp
    src/client.cpp
    src/request.cpp
    src/response.cpp
    src/util.cpp
)

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(${LIB_NAME} PUBLIC userver::core)

target_link_libraries(${LIB_NAME} PUBLIC ${PROJECT_NAME}_base)



# add_executable(${PROJECT_NAME}_unittest
#     # src/response.cpp
#     ut/response_ut.cpp
# )
# target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${LIB_NAME} userver::utest)
# add_google_tests(${LIB_NAME}_unittest)
