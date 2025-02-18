cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ${PROJECT_NAME}_proto)
project(${LIB_NAME})

# Находим Protocol Buffers
find_package(Protobuf REQUIRED)

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpp")

# Создаём выходную папку
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}")

# Ищем все .proto файлы
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES}  PROTOC_OUT_DIR ${PROTO_GEN_DIR})
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

message ("PROTO_SRCS = ${PROTO_SRCS}")
message ("PROTO_HDRS = ${PROTO_HDRS}")

add_library(${LIB_NAME} STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(${LIB_NAME} PRIVATE ${PROTO_GEN_DIR})

target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${PROTO_GEN_DIR}/../..>)

target_link_libraries(${LIB_NAME} PUBLIC ${Protobuf_LIBRARIES})
