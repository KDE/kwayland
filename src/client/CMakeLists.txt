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
    disabledoutput.cpp
    fakeinput.cpp
    fullscreen_shell.cpp
    idle.cpp
    keyboard.cpp
    output_management.cpp
    logging.cpp
    output.cpp
    pointer.cpp
    plasmashell.cpp
    plasmawindowmanagement.cpp
    plasmawindowmodel.cpp
    region.cpp
    registry.cpp
    seat.cpp
    shadow.cpp
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

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/output-management.xml
    BASENAME org_kde_kwin_output_management
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/plasma-shell.xml
    BASENAME plasma-shell
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/plasma-window-management.xml
    BASENAME plasma-window-management
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/idle.xml
    BASENAME idle
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/fake-input.xml
    BASENAME fake-input
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWAYLAND_SOURCE_DIR}/src/client/protocols/shadow.xml
    BASENAME shadow
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
  disabledoutput.h
  fakeinput.h
  fullscreen_shell.h
  idle.h
  keyboard.h
  output_management.h
  output.h
  pointer.h
  plasmashell.h
  plasmawindowmanagement.h
  plasmawindowmodel.h
  region.h
  registry.h
  seat.h
  shadow.h
  shell.h
  shm_pool.h
  subcompositor.h
  subsurface.h
  surface.h
  touch.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KWayland/Client COMPONENT Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KWaylandClient LIB_NAME KF5WaylandClient DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5})
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
