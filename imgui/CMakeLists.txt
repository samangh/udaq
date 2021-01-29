set(TARGET "imgui")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/external/imgui)

find_package(SDL2 REQUIRED)


##
## Source files
##

file(GLOB SRC_FILES
  ${IMGUI_DIR}/*.cpp
  ${IMGUI_DIR}/examples/libs/gl3w/GL/*.c
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp)

add_library(${TARGET} STATIC ${SRC_FILES})

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${IMGUI_DIR}>
    $<BUILD_INTERFACE:${IMGUI_DIR}/examples/libs/gl3w>
  PRIVATE)

target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
    SDL2::SDL2
    SDL2::SDL2main
    "-framework CoreFoundation"
  PRIVATE)

##
## Compiler flags
##

target_compile_features(${TARGET} PRIVATE cxx_std_17)

##
## Version
##

configure_file (
  "${PROJECT_SOURCE_DIR}/cmake/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h")

