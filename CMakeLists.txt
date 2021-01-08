cmake_minimum_required(VERSION 3.13.4)

project(udaq
  VERSION "0.0.1"
  DESCRIPTION "Lab Data Acquisition Sequencer"
  LANGUAGES CXX)

set(NAMESPACE udaq)

# set(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)
# set(CMAKE_VERBOSE_MAKEFILE 1)

##
## Build options
##

# Set this to OFF to build static libraries
option (BUILD_SHARED_LIBS "Build shared libraries" ON)

# Export compile database for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

##
## Install Paths
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR "/usr/lib/cmake/${PROJECT_NAME}")

##
## CMake module paths
##

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/third-party/sanitizers-cmake/cmake")

##
## RPATHS
##

# Set RPATHS in case dependecies are in non-standard locations
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Set RPATH in case the project is installed in non-standard location
list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

list(APPEND CMAKE_INSTALL_RPATH "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")


# Dependencies

find_package(VISA REQUIRED)

##
## Targets/Sources
##

add_subdirectory("src/hello-world")

# Generate target list
get_property(TARGETS GLOBAL PROPERTY ${PROJECT_NAME}_TARGETS)

##
## Compile options
##

# Set the default build type if none specified by user
include(BuildType)

# Enable extra errors for all projects
foreach(t ${TARGETS})
  target_compile_options(${t}
    PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>)
endforeach()

foreach(t ${TARGETS})
  target_compile_features(${t} PRIVATE cxx_std_17)
endforeach()
