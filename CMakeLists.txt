cmake_minimum_required(VERSION 3.6.0)

set(PROJECT_NAME audiodookie)
project(${PROJECT_NAME} C CXX)
set(EXE ${PROJECT_NAME})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
include(GNUInstallDirs)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")

find_package(Wayland REQUIRED Client Cursor Egl)
find_package(WaylandScanner REQUIRED)
find_package(WaylandProtocols 1.13 REQUIRED)
find_package(FFTW)
find_package(PulseAudio)
find_package(Cairo)

include_directories(${FFTW_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS})

set(LIBS ${Wayland_LIBRARIES} ${FFTW_LIBRARIES} ${PulseAudio_LIBRARIES} ${CAIRO_LIBRARIES} rt)

set(DOOKIE_LIB dookie)
set(DOOKIE_SRC
 dookie/context.cpp
 dookie/audiofifo.cpp
 dookie/wayland.cpp
 dookie/analyze.cpp
 dookie/visualize.cpp
 dookie/output.cpp
)


find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
set(PROTO_SRC "")
macro(wayland_protocol protodef protosdir)
  add_custom_command(
    OUTPUT ${protodef}-client-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
    < ${protosdir}/${protodef}.xml > ${CMAKE_CURRENT_BINARY_DIR}/${protodef}-client-protocol.h
    DEPENDS ${protosdir}/${protodef}.xml)
  
  add_custom_command(
    OUTPUT ${protodef}-protocol.c
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} public-code
    < ${protosdir}/${protodef}.xml > ${CMAKE_CURRENT_BINARY_DIR}/${protodef}-protocol.c
    DEPENDS ${protosdir}/${protodef}.xml)
  
  LIST(APPEND PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${protodef}-client-protocol.h")
  LIST(APPEND PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${protodef}-protocol.c")
endmacro()

wayland_protocol(wlr-layer-shell-unstable-v1 ${CMAKE_SOURCE_DIR}/protocols)
wayland_protocol(xdg-shell ${WAYLAND_PROTOCOLS_PKGDATADIR}/stable/xdg-shell)
wayland_protocol(xdg-output-unstable-v1 ${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/xdg-output)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(DOOKIE_PROTO_LIBS ${DOOKIE_LIB}-protocols)

add_library(${DOOKIE_PROTO_LIBS} ${PROTO_SRC})
add_library(${DOOKIE_LIB} ${DOOKIE_SRC})

add_executable(${EXE} main.cpp)

target_link_libraries(${EXE} ${DOOKIE_LIB} ${DOOKIE_PROTO_LIBS} ${LIBS})
