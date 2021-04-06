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
option (BUILD_TESTS "Build tests and related tools" OFF)

if(MSVC)
  # Note: will require SDL2 to be compiled with FORCE_STATIC_VCRT=ON
  # We only use this on MSVC
  option(USE_STATIC_RUNTIME "Statically link against the C++ runtime" OFF)
endif()

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

find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(VISA REQUIRED)

find_package(unofficial-libuv CONFIG REQUIRED)
set(uv_selected unofficial::libuv::libuv)

# find_package(wampcc REQUIRED)

# if (BUILD_SHARED_LIBS)
#   add_library(wampcc_selected ALIAS wampcc::wampcc_shared)
# else()
#   add_library(wampcc_selected ALIAS wampcc::wampcc_static)
# endif()

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

add_subdirectory(external_cmake_wrappers/implot)
add_subdirectory(external_cmake_wrappers/imgui)
add_subdirectory(external_cmake_wrappers/wise_enum)
add_subdirectory(external_cmake_wrappers/imgui-filebrowser)

add_subdirectory(helpers/imgui)

add_subdirectory(common)
add_subdirectory(devices)

add_subdirectory(udaq)
add_subdirectory(gui)

if(BUILD_TESTS)
  add_subdirectory(tools/scpi-simulator)
  add_subdirectory(tools/safibra-simulator)
  add_subdirectory(tests/find-resource)
  add_subdirectory(tests/idn)
  add_subdirectory(test)
endif()

# Generate target list
get_property(TARGETS GLOBAL PROPERTY ${PROJECT_NAME}_TARGETS)

##
## Compile options
##

# Set the default build type if none specified by user
include(BuildType)

foreach(t ${TARGETS})

  # Enable extra errors for all projects
  target_compile_options(${t}
    PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
     $<$<CXX_COMPILER_ID:MSVC>:/W4>)

  # Link against MSVC statically if enabled
  if (USE_STATIC_RUNTIME AND MSVC)
    # The MSVC_RUNTIME_LIBRARY option does work properly (even when the
    # CMP0091 option is set), so let's do this manually
    set_property(TARGET ${t} PROPERTY MSVC_RUNTIME_LIBRARY "")
    target_compile_options(${t} PRIVATE "/MT$<$<CONFIG:Debug>:d>")
  endif()
endforeach()

foreach(t ${TARGETS})
  target_compile_features(${t} PRIVATE cxx_std_17)
endforeach()
