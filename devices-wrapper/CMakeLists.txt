set(TARGET "devices-wrapper")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

#find_package(Libuv REQUIRED)

##
## Source files
##

file(GLOB_RECURSE SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# This MUST be a shared library, as otherwise you can't export from it
# See https://devblogs.microsoft.com/oldnewthing/20140321-00/?p=1433
add_library(${TARGET} SHARED ${SRC_FILES})

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE
  )

target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
  PRIVATE
    devices
    common)

target_compile_definitions(${TARGET} PRIVATE DEVICES_WRAPPER_BUILDING_DLL)

##
## Compiler flags
##

target_compile_features(${TARGET} PRIVATE cxx_std_17)

##
## Version
##

configure_file (
  "${PROJECT_SOURCE_DIR}/cmake/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  )

