#doc in ../README.md
cmake_minimum_required(VERSION 3.8)
set(PROJECT_NAME CodeGen_GEGELATI)

project(${PROJECT_NAME} LANGUAGES C)

set(SRC ${DIR}/src/)
set(INCLUDE  ${DIR}/src/)
set(BIN ${DIR}/bin/)

include_directories(${INCLUDE})
include_directories(.)

if (${DEBUG})
    add_definitions(-DDEBUG)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})

set(target OneLeaf)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)