set(TEST_TARGET_NAME runTests)

# Download and unpack googletest at configure time
# from https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

if(CODE_GEN)
  add_definitions(-DBIN_DIR_PATH="${CMAKE_BINARY_DIR}")
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src ${CMAKE_BINARY_DIR}/obj ${CMAKE_BINARY_DIR}/buildCodeGen)
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

file(
	GLOB_RECURSE
	${TEST_TARGET_NAME}_SRC
	*.cpp
	*.h
)

# Add definition for relative path into project
# Path to dat folder needed for tests
add_definitions( -DTESTS_DAT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/dat/")

# Build for coverage when in debug
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage -fprofile-arcs -ftest-coverage")
endif()

add_executable(${TEST_TARGET_NAME} ${${TEST_TARGET_NAME}_SRC})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_link_libraries(${TEST_TARGET_NAME} gtest_main ${PROJECT_NAME}::${PROJECT_NAME} -latomic)
else()
	target_link_libraries(${TEST_TARGET_NAME} gtest_main ${PROJECT_NAME}::${PROJECT_NAME})
endif()

# Add a test to the project to be run by ctest.
# See https://cmake.org/cmake/help/latest/command/add_test.html
# See https://cmake.org/cmake/help/latest/manual/ctest.1.html
# COMMAND tag specifies the test command-line. If it is an executable target
# created by add_executable(), it will automatically be replaced by the location
# of the executable created at build time.
add_test(NAME TPGTest
         COMMAND ${TEST_TARGET_NAME})
         #WORKING_DIRECTORY $<TARGET_FILE_DIR:${TEST_TARGET_NAME}>)
