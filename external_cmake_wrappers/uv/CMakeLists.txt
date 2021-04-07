set(TARGET "uv")

# See if vcpkg's libuv is available
find_package(unofficial-libuv CONFIG QUIET)
if (TARGET unofficial::libuv::libuv)
  set(LIBUV_TARGET unofficial::libuv::libuv)
endif()

if(NOT LIBUV_TARGET)
  find_package(LibUV REQUIRED)
  set(LIBUV_TARGET LibUV::LibUV)
endif()

##
## Source files
##

add_library(${TARGET} INTERFACE)

##
## Includes
##

target_link_libraries(${TARGET}
  INTERFACE
    ${LIBUV_TARGET})

