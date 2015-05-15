remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

# needed to access QPA
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

set(CLIENT_LIB_SRCS
    buffer.cpp
    compositor.cpp
    connection_thread.cpp
    event_queue.cpp
    datadevice.cpp
    datadevicemanager.cpp
    dataoffer.cpp
    datasource.cpp
    fullscreen_shell.cpp
    keyboard.cpp
    logging.cpp
    output.cpp
    pointer.cpp
    region.cpp
    registry.cpp
    seat.cpp
    shell.cpp
    shm_pool.cpp
    subcompositor.cpp
    subsurface.cpp
    surface.cpp
    touch.cpp
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/fullscreen-shell.xml
    BASENAME fullscreen-shell
)

add_library(KF5WaylandClient ${CLIENT_LIB_SRCS})
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
  event_queue.h
  datadevice.h
  datadevicemanager.h
  dataoffer.h
  datasource.h
  fullscreen_shell.h
  keyboard.h
  output.h
  pointer.h
  region.h
  registry.h
  seat.h
  shell.h
  shm_pool.h
  subcompositor.h
  subsurface.h
  surface.h
  touch.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KWayland/Client COMPONENT Devel
)
