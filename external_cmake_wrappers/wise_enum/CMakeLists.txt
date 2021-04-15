set(TARGET "wise_enum")

set(DEST ${CMAKE_CURRENT_LIST_DIR}/include/wise_enum)
file(GLOB FILES ${PROJECT_SOURCE_DIR}/external/wise_enum/*.h)

##
## Source files
##

# Create target for just copying some files

add_custom_target(${TARGET}_files )
add_custom_command(TARGET ${TARGET}_files PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILES} ${DEST})

setup_ide_folders(TARGET ${TARGET}_files DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create interface target that depends on those files
add_library(${TARGET} INTERFACE)
add_dependencies(${TARGET}
    ${TARGET}_files
)

##
## Includes
##

target_include_directories(${TARGET} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)


