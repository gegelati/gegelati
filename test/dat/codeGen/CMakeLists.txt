cmake_minimum_required(VERSION 3.8)
set(PROJECT_NAME CodeGen_GEGELATI)

project(${PROJECT_NAME} LANGUAGES C)

set(SRC ${DIR}/src/)
set(INCLUDE  ${DIR}/src/)
set(BIN ${DIR}/bin/)

include_directories(${INCLUDE})
include_directories(.)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})

message(${SRC})
message(${INCLUDE})
message(${BIN})

set(target OneLeafNoInstruction)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target OneLeaf)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target TwoLeaves)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target ThreeLeaves)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target OneTeamOneLeaf)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target OneTeamTwoLeaves)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target TwoTeamsOneCycle)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target ThreeTeamsOneCycleThreeLeaves)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)

set(target TicTacToeBest_TPG)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)
target_link_libraries(${target} m)

set(target StickGameBest_TPG)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)
target_link_libraries(${target} m)