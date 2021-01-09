cmake_minimum_required(VERSION 3.13.4)

project(udaq
  VERSION "0.0.1"
  DESCRIPTION "Cross-pltform Data Acquisition Sequencer"
  LANGUAGES CXX)

set(NAMESPACE udaq)

##
## Build options
##

# Set this to OFF to build static libraries
option (BUILD_SHARED_LIBS "Build shared libraries" ON)

##
## CMake module paths
##

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

##
## Packages
##

find_package(VISA REQUIRED)

##
## Targets/Sources
##

add_subdirectory("src/tests/find-resource")
add_subdirectory("src/tests/idn")

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
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
     $<$<CXX_COMPILER_ID:MSVC>:/W4>)
endforeach()

foreach(t ${TARGETS})
  target_compile_features(${t} PRIVATE cxx_std_17)
endforeach()

##
## Misc
##

# Export compile database for IDEs, needed for QtCreator
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

