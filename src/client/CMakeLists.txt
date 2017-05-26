remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

# needed to access QPA
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

set(CLIENT_LIB_SRCS
    buffer.cpp
    blur.cpp
    compositor.cpp
    connection_thread.cpp
    contrast.cpp
    slide.cpp
    event_queue.cpp
    datadevice.cpp
    datadevicemanager.cpp
    dataoffer.cpp
    datasource.cpp
    dpms.cpp
    fakeinput.cpp
    fullscreen_shell.cpp
    idle.cpp
    keyboard.cpp
    outputconfiguration.cpp
    outputmanagement.cpp
    outputdevice.cpp
    logging.cpp
    output.cpp
    pointer.cpp
    pointerconstraints.cpp
    pointergestures.cpp
    plasmashell.cpp
    plasmawindowmanagement.cpp
    plasmawindowmodel.cpp
    region.cpp
    registry.cpp
    relativepointer.cpp
    seat.cpp
    server_decoration.cpp
    shadow.cpp
    shell.cpp
    shm_pool.cpp
    subcompositor.cpp
    subsurface.cpp
    surface.cpp
    touch.cpp
    textinput.cpp
    textinput_v0.cpp
    textinput_v2.cpp
    xdgshell.cpp
    xdgshell_v5.cpp
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/fullscreen-shell.xml
    BASENAME fullscreen-shell
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/output-management.xml
    BASENAME output-management
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/outputdevice.xml
    BASENAME org_kde_kwin_outputdevice
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/plasma-shell.xml
    BASENAME plasma-shell
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/plasma-window-management.xml
    BASENAME plasma-window-management
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/idle.xml
    BASENAME idle
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/fake-input.xml
    BASENAME fake-input
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/shadow.xml
    BASENAME shadow
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/blur.xml
    BASENAME blur
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/contrast.xml
    BASENAME contrast
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/slide.xml
    BASENAME slide
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/dpms.xml
    BASENAME dpms
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/server-decoration.xml
    BASENAME server-decoration
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/text-input.xml
    BASENAME text-input-v0
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/text-input-unstable-v2.xml
    BASENAME text-input-v2
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/xdg-shell-unstable-v5.xml
    BASENAME xdg-shell-v5
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/relative-pointer-unstable-v1.xml
    BASENAME relativepointer-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/pointer-gestures-unstable-v1.xml
    BASENAME pointer-gestures-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/pointer-constraints-unstable-v1.xml
    BASENAME pointer-constraints-unstable-v1
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
        Qt5::Concurrent
)

set_target_properties(KF5WaylandClient PROPERTIES VERSION   ${KWAYLAND_VERSION_STRING}
                                                 SOVERSION ${KWAYLAND_SOVERSION}
                                                 EXPORT_NAME WaylandClient
)

install(TARGETS KF5WaylandClient EXPORT KF5WaylandTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

set(CLIENT_LIB_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/KWayland/Client/kwaylandclient_export.h
  blur.h
  buffer.h
  compositor.h
  connection_thread.h
  contrast.h
  event_queue.h
  datadevice.h
  datadevicemanager.h
  dataoffer.h
  datasource.h
  dpms.h
  fakeinput.h
  fullscreen_shell.h
  idle.h
  keyboard.h
  outputconfiguration.h
  outputmanagement.h
  outputdevice.h
  output.h
  pointer.h
  pointerconstraints.h
  plasmashell.h
  plasmawindowmanagement.h
  plasmawindowmodel.h
  pointergestures.h
  region.h
  registry.h
  relativepointer.h
  seat.h
  server_decoration.h
  shadow.h
  shell.h
  shm_pool.h
  slide.h
  subcompositor.h
  subsurface.h
  surface.h
  touch.h
  textinput.h
  xdgshell.h
)

install(FILES
  ${CLIENT_LIB_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KWayland/Client COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KWaylandClient_APIDOX_SRCS ${CLIENT_LIB_HEADERS} PARENT_SCOPE)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KWaylandClient LIB_NAME KF5WaylandClient DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5})
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
