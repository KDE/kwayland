remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/wayland_protocols/)
add_custom_command(
    OUTPUT
        ${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.h
    COMMAND
        ${WAYLAND_SCANNER_EXECUTABLE} client-header <${KWAYLAND_SOURCE_DIR}/src/client/protocols/fullscreen-shell.xml >${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.h
    DEPENDS ${CMAKE_BINARY_DIR}/wayland_protocols/
    WORKING_DIRECTORY
        ${CMAKE_BINARY_DIR}/wayland_protocols
)
add_custom_command(
    OUTPUT
        ${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.c
    COMMAND
        ${WAYLAND_SCANNER_EXECUTABLE} code <${KWAYLAND_SOURCE_DIR}/src/client/protocols/fullscreen-shell.xml >${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.c
    DEPENDS ${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.h
    WORKING_DIRECTORY
        ${CMAKE_BINARY_DIR}/wayland_protocols
)
add_custom_target(wayland-client-fullscreen-shell DEPENDS ${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.c)
include_directories(${CMAKE_BINARY_DIR}/wayland_protocols/)
set_source_files_properties(${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.c GENERATED)

set(CLIENT_LIB_SRCS
    ${CMAKE_BINARY_DIR}/wayland_protocols/wayland-client-fullscreen-shell.c
    buffer.cpp
    compositor.cpp
    connection_thread.cpp
    fullscreen_shell.cpp
    keyboard.cpp
    output.cpp
    pointer.cpp
    registry.cpp
    seat.cpp
    shell.cpp
    shm_pool.cpp
    surface.cpp
)

add_library(KF5WaylandClient ${CLIENT_LIB_SRCS})
add_dependencies(KF5WaylandClient wayland-client-fullscreen-shell)
generate_export_header(KF5WaylandClient
    BASE_NAME
        KWaylandClient
    EXPORT_FILE_NAME
        KWayland/Client/kwaylandclient_export.h
)
add_library(KF5::WaylandClient ALIAS KF5WaylandClient)

target_include_directories(KF5WaylandClient INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KWayland/Client>")

target_link_libraries(KF5WaylandClient
    PUBLIC Qt5::Gui
    PRIVATE Wayland::Client
)

if(IS_ABSOLUTE "${KF5_INCLUDE_INSTALL_DIR}")
  target_include_directories(KF5WaylandClient INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}>" )
else()
  target_include_directories(KF5WaylandClient INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${KF5_INCLUDE_INSTALL_DIR}>" )
endif()

set_target_properties(KF5WaylandClient PROPERTIES VERSION   ${KWAYLAND_VERSION_STRING}
                                                 SOVERSION ${KWAYLAND_SOVERSION}
                                                 EXPORT_NAME WaylandClient
)

install(TARGETS KF5WaylandClient EXPORT KF5WaylandTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/KWayland/Client/kwaylandclient_export.h
  buffer.h
  compositor.h
  connection_thread.h
  fullscreen_shell.h
  keyboard.h
  output.h
  pointer.h
  registry.h
  seat.h
  shell.h
  shm_pool.h
  surface.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KWayland/Client COMPONENT Devel
)
