cmake_minimum_required(VERSION 3.16)
project(compiler)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE HDRS ${PROJECT_SOURCE_DIR}/src/*.h  )
file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE GEN_HDRS ${PROJECT_SOURCE_DIR}/gen/*.cpp)
file(GLOB_RECURSE GEN_SRCS ${PROJECT_SOURCE_DIR}/gen/*.h)

include_directories(
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/misc
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/atn
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/dfa
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/tree
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/support
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/libs/
)

add_executable(compiler ${HDRS} ${SRCS} ${GEN_HDRS} ${GEN_SRCS})

target_link_libraries(compiler "${PROJECT_SOURCE_DIR}/libs/libantlr4-runtime.so")