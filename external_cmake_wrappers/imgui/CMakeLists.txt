set(TARGET "imgui")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/external/imgui)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

##
## Source files
##

file(COPY ${IMGUI_DIR}/ DESTINATION ${CMAKE_CURRENT_LIST_DIR}/src
  FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.cpp"
    PATTERN "*.c"
    PATTERN "imconfig.h" EXCLUDE)

file(GLOB SRC_FILES
  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/examples/libs/gl3w/GL/*.c
  ${CMAKE_CURRENT_LIST_DIR}/src/backends/imgui_impl_opengl3.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/backends/imgui_impl_sdl.cpp)
list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/src/imgui_demo.cpp)

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

if(TARGET SDL2::SDL2main)
  if(TARGET SDL2::SD2-static)
    if (NOT BUILD_SHARED_LIBS OR NOT TARGET SDL2::SDL2)
      set(${PROJECT}_STATIC_SDL ON)
    endif()
  endif()

  target_link_libraries(${TARGET} PUBLIC
      $<IF:$<BOOL:${PROJECT}_STATIC_SDL>,SDL2::SDL2-static,SDL2::SDL2>
      SDL2::SDL2main)
else()
  target_include_directories(${TARGET} PUBLIC ${SDL2_INCLUDE_DIRS})
  target_link_libraries(${TARGET} PUBLIC ${SDL2_LIBRARIES})
endif()


target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
  PRIVATE
    OpenGL::GL
    "$<$<BOOL:${APPLE}>:-framework CoreFoundation>"
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:${CMAKE_DL_LIBS}>)

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

