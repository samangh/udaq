cmake_minimum_required(VERSION 3.13.4)

# Allows for setting MSVC static runtime
cmake_policy(SET CMP0091 NEW)

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
option (BUILD_TESTS "Build tests and related tools" OFF)

if (BUILD_SHARED_LIBS)
  set(USE_STATIC_LIBS_DEFAULT OFF)
else()
  set(USE_STATIC_LIBS_DEFAULT ON)
endif()

option (USE_STATIC_LIBS "Use external static libs if possible" ${USE_STATIC_LIBS_DEFAULT})

if(MSVC)
  # Note: will require SDL2 to be compiled with FORCE_STATIC_VCRT=ON
  # We only use this on MSVC
  option(USE_STATIC_RUNTIME "Statically link against the C++ runtime" OFF)
endif()

option (LABVIEW_SUPPORT "Build with LabVIEW support" OFF)

##
## CMake module paths
##

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external/sanitizers-cmake/cmake")

##
## Compile support
##

# Export compile database for IDEs, needed for QtCreator
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##
## Import functions
##

include(setup_ide_folders)
include(get_standard_library_name)

# Check for name of standard library, use by common
include(CheckCXXSourceCompiles)
get_standard_library_name(STANDARD_LIBRARY)

##
## Packages
##

find_package(Sanitizers REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(LibUV REQUIRED)

find_package(wampcc REQUIRED)
if (TARGET wampcc::wampcc_static AND USE_STATIC_LIBS)
  set_target_properties(wampcc::wampcc_static PROPERTIES IMPORTED_GLOBAL TRUE)
  add_library(wampcc ALIAS wampcc::wampcc_static)
else()
  set_target_properties(wampcc::wampcc_shared PROPERTIES IMPORTED_GLOBAL TRUE)
  add_library(wampcc ALIAS wampcc::wampcc_shared)
endif()

# Use boost statially if making a static library
if(NOT BUILD_SHARED_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
endif()

if (USE_STATIC_RUNTIME)
  set(Boost_USE_STATIC_RUNTIME ON)
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

add_subdirectory(external/cli11)
add_subdirectory(external_cmake_wrappers/uv)
add_subdirectory(external_cmake_wrappers/wise_enum)
add_subdirectory(helpers/visa)

add_subdirectory(common)
add_subdirectory(imgui)

add_subdirectory(devices/safibra)

add_subdirectory(wrappers/c)
if(LABVIEW_SUPPORT)
  add_subdirectory(wrappers/labview-c)
endif()

add_subdirectory(udaq)
add_subdirectory(gui)

if(BUILD_TESTS)
  add_subdirectory(tools/scpi-simulator)
  add_subdirectory(tools/safibra-simulator)
  add_subdirectory(tests/find-resource)
  add_subdirectory(tests/idn)
  add_subdirectory(tests/device-wrapper)
  #add_subdirectory(test)
endif()

# Generate target list
get_property(TARGETS GLOBAL PROPERTY ${PROJECT_NAME}_TARGETS)

##
## Compile options
##

foreach(t ${TARGETS})
  # Enable extra errors for all projects
  target_compile_options(${t}
    PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
     $<$<CXX_COMPILER_ID:MSVC>:/permissive->)

  # Link against MSVC statically if enabled
  if (USE_STATIC_RUNTIME AND MSVC)
    set_property(TARGET ${t} PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()

  add_sanitizers(${t})
endforeach()

foreach(t ${TARGETS})
  target_compile_features(${t} PRIVATE cxx_std_17)
endforeach()
