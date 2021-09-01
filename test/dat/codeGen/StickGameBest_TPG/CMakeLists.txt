#doc in ../README.md
cmake_minimum_required(VERSION 3.8)

# This sets the PROJECT_NAME, PROJECT_VERSION as well as other variable
set(PROJECT_NAME CodeGen_GEGELATI)

project(${PROJECT_NAME} LANGUAGES C)

set(SRC ${DIR}/src/)
set(INCLUDE  ${DIR}/src/)
set(BIN ${DIR}/bin/)

include_directories(${INCLUDE})
include_directories(.)

# If DEBUG = 1 the generated will have a verbose execution with more information printed
if (${DEBUG})
    add_definitions(-DDEBUG)
endif ()

if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
	#add libmath during non visual studio builds
	set(CMAKE_EXTRA_LIB m)
endif()

# Control where the executable is placed during the build.
# This is required so the test fixture can execute the compiled binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})

# set the target name
set(target StickGameBest_TPG)
add_executable(${target} ${SRC}${target}.c ${SRC}${target}_program.c main${target}.c)