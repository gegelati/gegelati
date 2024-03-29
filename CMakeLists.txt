# Minimum version for CXX17 support.
cmake_minimum_required(VERSION 3.8)

# This CMakeLists was freely adapted by Karol Desnos from
# https://github.com/robotology/how-to-export-cpp-library
# Which is available under the following license :
#
# The MIT License (MIT)
#
# Copyright (c) 2014 Silvio Traversaro, Claudio Fantacci
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


# This sets the PROJECT_NAME, PROJECT_VERSION as well as other variable
# listed here: https://cmake.org/cmake/help/latest/command/project.html.
# We use this name to export all the files such that is then possible to use
# find_package(<libName>) in third party projects.
# LANGUAGES specifies which languages your project supports. To have a broad
# support with external libraries, it is good practice to list both CXX and C.
# Infact, some legacy Find<package>.cmake files that require the C language to
# be enabled and thus using only CXX may cause issues. Note that by default when
# LANGUAGES is not specified CMake enables both CXX and C.
set(PROJECT_NAME GEGELATI)

project(${PROJECT_NAME}
        LANGUAGES CXX
        VERSION 1.3.1)
string(TOLOWER ${PROJECT_NAME} project_name)
set(LIB_SOURCE_DIRECTORY ${project_name}lib)

# Propagate version number
add_definitions(-DGEGELATI_VERSION="${PROJECT_VERSION}")

# If CODE_GEN = ON gegelati is compile with the codeGen, if CODE_GEN = OFF the
# codeGen is disabled
option(CODE_GEN "Compile GEGELATI with the code generation feature." ON)

if(CODE_GEN)
    add_definitions(-DCODE_GENERATION)
endif()


# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# Control where libraries and executables are placed during the build.
# With the following settings executables are placed in <the top level of the
# build tree>/bin and libraries/archives in <top level of the build tree>/lib.
# This is particularly useful to run ctests on libraries built on Windows
# machines: tests, which are executables, are placed in the same folders of
# dlls, which are treated as executables as well, so that they can properly
# find the libraries to run. This is a because of missing RPATH on Windows.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Compiler-specific settings
if(MSVC)
	# Under MSVC, we set CMAKE_DEBUG_POSTFIX to "d" to add a trailing "d" to library
	# built in debug mode. In this Windows user can compile, build and install the
	# library in both Release and Debug configuration avoiding naming clashes in the
	# installation directories.
    set(CMAKE_DEBUG_POSTFIX "d")
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
	# Install in dedicated subfolder
	set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/${LIB_SOURCE_DIRECTORY}-${PROJECT_VERSION})
elseif(MINGW)
    # Under MinGW, we set CMAKE_DEBUG_POSTFIX to "d" to add a trailing "d" to library
	# built in debug mode. In this Windows user can compile, build and install the
	# library in both Release and Debug configuration avoiding naming clashes in the
	# installation directories.
    set(CMAKE_DEBUG_POSTFIX "d")
    # Install in dedicated subfolder
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/${LIB_SOURCE_DIRECTORY}-${PROJECT_VERSION})
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Require gcc 7 at least.
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        message(FATAL_ERROR "GCC version cannot be lower than 7.0 to build this lib!")
    endif()
    # Add definition for inttype support in Mingw
    add_definitions(-D__STDC_FORMAT_MACROS)
endif()

# Build position independent code.
# Position Independent Code (PIC) is commonly used for shared libraries so that
# the same shared library code can be loaded in each program address space in a
# location where it will not overlap with any other uses of such memory.
# In particular, this option avoids problems occurring when a process wants to
# load more than one shared library at the same virtual address.
# Since shared libraries cannot predict where other shared libraries could be
# loaded, this is an unavoidable problem with the traditional shared library
# concept.
# Generating position-independent code is often the default behavior for most
# modern compilers.
# Moreover linking a static library that is not built with PIC from a shared
# library will fail on some compiler/architecture combinations.
# Further details on PIC can be found here:
# https://eli.thegreenplace.net/2011/11/03/position-independent-code-pic-in-shared-libraries/
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable C and C++ compiler extensions.
# C/CXX_EXTENSIONS are ON by default to allow the compilers to use extended
# variants of the C/CXX language.
# However, this could expose cross-platform bugs in user code or in the headers
# of third-party dependencies and thus it is strongly suggested to turn
# extensions off.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# We use
# - InstallBasicPackageFiles (http://robotology.github.io/ycm/gh-pages/v0.8/module/InstallBasicPackageFiles.html)
# - AddUninstallTarget (http://robotology.github.io/ycm/gh-pages/v0.8/module/AddUninstallTarget.html)
# - AddInstallRPATHSupport (http://robotology.github.io/ycm/gh-pages/v0.8/module/AddInstallRPATHSupport.html)
# from YCM. Files are under the cmake subdirectory.
# See https://github.com/robotology/ycm/
# If you don't want to ship these files with your code (recommended), you can
# instead depend on YCM.
# In this case replace the following line with
# find_package(YCM REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

### Options
# Build test related commands?
option(BUILD_TESTING "Create tests using CMake" ON)
if(BUILD_TESTING)
    enable_testing()
endif()

# Enable RPATH support for installed binaries and libraries
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
                          INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
                          USE_LINK_PATH)

# Encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()

### Compile- and install-related commands.
add_subdirectory(${LIB_SOURCE_DIRECTORY})

# Create and install CMake configuration files for your project that are
# necessary to for other projects to call find_package(LibTemplateCMake).
#
# Note that it is extremely important to use exactly the project name while
# installing configuration files (you can use PROJECT_NAME variable to avoid
# any possible error). This is required to allow find_package() to properly
# look for the installed library in system path, in particular in Windows when
# the installation is performed in the default path.
#
# install_basic_package_files() comes with many input parameters to customize
# the configuration files. The parameters used in the following call provide
# basic versions of CMake configuration files.
# See install_basic_package_files() documentation found in ./cmake folder.
#
# Note that if your library depends from other libraries, you are probably
# required to used the install_basic_package_files() DEPENDENCIES option.
include(InstallBasicPackageFiles)
install_basic_package_files(${PROJECT_NAME}
                            VERSION ${${PROJECT_NAME}_VERSION}
                            COMPATIBILITY AnyNewerVersion
                            VARS_PREFIX ${PROJECT_NAME}
                            NO_CHECK_REQUIRED_COMPONENTS_MACRO)
# Add the uninstall target
include(AddUninstallTarget)

# Add integration tests (unit tests for each library should be in each sublibrary directory).
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

if(NOT SKIP_DOXYGEN_BUILD)
    # Add targets related to doxygen documention generation
    add_subdirectory(doc)
endif()
