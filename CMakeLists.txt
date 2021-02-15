cmake_minimum_required(VERSION 3.13.4)

project(udaq
  VERSION "0.0.1"
  DESCRIPTION "Cross-pltform Data Acquisition Sequencer"
  LANGUAGES C CXX)

set(NAMESPACE udaq)

##
## Build options
##

# Set this to OFF to build static libraries
option (BUILD_SHARED_LIBS "Build shared libraries" ON)

##
## Misc
##

# Export compile database for IDEs, needed for QtCreator
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


##
## CMake module paths
##

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

##
## Packages
##

find_package(VISA REQUIRED)

# Use boost statially if making a static library
if(NOT BUILD_SHARED_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
endif()

# Configure Boost in Windows
if(MSVC)
  # Target Windows 7 and higher
  add_compile_definitions(_WIN32_WINNT=_WIN32_WINNT_WIN7)

  # Boost tries to use auto linking (i.e. #pragma lib in headers) to tell
  # the compiler what to link to. This does not work properly on
  # Widnows/MSVC.
  add_compile_definitions(BOOST_ALL_NO_LIB)
endif()

##
## Targets/Sources
##

add_subdirectory(imgui)

add_subdirectory(udaq)
add_subdirectory(gui)

add_subdirectory(tools/scpi-simulator)
add_subdirectory(tests/find-resource)
add_subdirectory(tests/idn)

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
