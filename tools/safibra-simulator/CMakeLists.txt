set(TARGET "safibra-simulator")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

##
## Source files
##

file(GLOB_RECURSE SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${TARGET} ${SRC_FILES})

find_package(Boost COMPONENTS thread REQUIRED)

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE
    VISA
    ${CMAKE_CURRENT_BINARY_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/src
  )

target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
  PRIVATE
    Boost::thread
    VISA
    common
    CLI11::CLI11
    $<$<BOOL:${WIN32}>:Winmm>
    )

setup_ide_folders(TARGET ${TARGET} DIR ${CMAKE_CURRENT_SOURCE_DIR})

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

