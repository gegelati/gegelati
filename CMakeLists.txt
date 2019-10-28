cmake_minimum_required(VERSION 3.15)

# *******************************************
# ************* CMake Content ***************
# *******************************************
# This CMake create a workspace containing the following projects
#
# Programs
#  - runTest
#  - tpglib

# *******************************************
# ************ Project Naming ***************
# *******************************************
set (PROJECT_NAME tpg)
set (LIB_NAME tpglib)
set (PROJECT_VERSION "0.0.0")


project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

# *******************************************
# ************ Configurations ***************
# *******************************************

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# IDE dependent config
if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
	MESSAGE("Add definition for ${CMAKE_GENERATOR}")
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

# Add definition for relative path into project
add_definitions( -DPROJECT_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set to 1 to activate debug compilation (0 for release)
if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
	set(DEBUG 0)

	if(${DEBUG})
			MESSAGE("Generate Debug project")
			set(CMAKE_BUILD_TYPE Debug)
			set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
			set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -pg -Wall")
	else()
			MESSAGE("Generate Release project")
			set(CMAKE_BUILD_TYPE Release)
			set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")
	endif()
	#add libmath during non visual studio builds
	set(CMAKE_EXTRA_LIB m)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxywizardProject.txt)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/doc)

    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
        WORKING_DIRECTORY ${DOXYGEN_OUT}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)


# *******************************************
# **************** tpglib * ****************
# *******************************************
set(include_dir ./include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(
	GLOB_RECURSE
	source_files
	./src/*.cpp
)

file(
	GLOB_RECURSE
	header_files
	./include/*.h
)


add_library(${LIB_NAME} SHARED)

target_sources(${LIB_NAME} PUBLIC ${source_files})

set_target_properties(${LIB_NAME}
PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# Let's set compiler-specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # G++
    target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra)	
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# Install in dedicated subfolder
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/${LIB_NAME}-${PROJECT_VERSION})
    # MSVC
    target_compile_options(${LIB_NAME} PRIVATE /EHsc /MTd /W2 /c)
    # Set the DLLEXPORT variable to export symbols
    target_compile_definitions(${LIB_NAME} PRIVATE WIN_EXPORT)
	# Under MSVC, we set CMAKE_DEBUG_POSTFIX to "d" to add a trailing "d" to library
	# built in debug mode. In this Windows user can compile, build and install the
	# library in both Release and Debug configuration avoiding naming clashes in the
	# installation directories.
	set(CMAKE_DEBUG_POSTFIX "d")
	
	set_target_properties(${LIB_NAME}
	PROPERTIES
		DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	)
endif()

install(TARGETS ${LIB_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)
	
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include" # source directory
		DESTINATION "." # Destination is CMAKE_BINARY_DIR
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
)

# *******************************************
# **************** runTest.exe  *************
# *******************************************

# Download and unpack googletest at configure time
# from https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project
configure_file(test/CMakeLists.txt.in googletest-download/CMakeLists.txt)
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

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
# Link runTests with what we want to test and the GTest and pthread library
file(GLOB_RECURSE
     test_files
     ./test/*.cpp
	 ./test/*.h)

add_executable(runTests ${test_files})
target_link_libraries(runTests gtest_main ${LIB_NAME})
add_test(NAME tpgTest COMMAND runTests)