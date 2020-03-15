cmake_minimum_required(VERSION 3.16)
project(compiler)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE HDRS ${PROJECT_SOURCE_DIR}/src/*.h  )
file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE GEN_HDRS ${PROJECT_SOURCE_DIR}/gen/*.cpp)
file(GLOB_RECURSE GEN_SRCS ${PROJECT_SOURCE_DIR}/gen/*.h)

add_executable(compiler ${HDRS} ${SRCS} ${GEN_HDRS} ${GEN_SRCS})

set(CMAKE_CXX_FLAGS "-O3")

target_link_libraries(compiler antlr4-runtime)

target_include_directories(
        compiler
        PUBLIC
        /usr/include/antlr4-runtime
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/gen
)

get_property(dirs GLOBAL PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(WARNING "dir='${dir}'")
endforeach()
