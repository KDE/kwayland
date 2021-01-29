remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

# needed to access QPA
include_directories(SYSTEM ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

set(CLIENT_LIB_SRCS
    appmenu.cpp
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
    idleinhibit.cpp
    keyboard.cpp
    keystate.cpp
    remote_access.cpp
    outputconfiguration.cpp
    outputmanagement.cpp
    outputdevice.cpp
    output.cpp
    pointer.cpp
    pointerconstraints.cpp
    pointergestures.cpp
    plasmashell.cpp
    plasmavirtualdesktop.cpp
    plasmawindowmanagement.cpp
    plasmawindowmodel.cpp
    region.cpp
    registry.cpp
    relativepointer.cpp
    seat.cpp
    server_decoration.cpp
    server_decoration_palette.cpp
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
    xdgdecoration.cpp
    xdgshell.cpp
    xdgforeign_v2.cpp
    xdgforeign.cpp
    xdgshell_v5.cpp
    xdgshell_v6.cpp
    xdgshell_stable.cpp
    xdgoutput.cpp
    ../compat/wayland-xdg-shell-v5-protocol.c
)

ecm_qt_declare_logging_category(CLIENT_LIB_SRCS
    HEADER logging.h
    IDENTIFIER KWAYLAND_CLIENT
    CATEGORY_NAME kf.wayland.client
    OLD_CATEGORY_NAMES kwayland-client
    DEFAULT_SEVERITY Critical
    DESCRIPTION "KWayland Client Library"
    EXPORT KWAYLAND
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
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/plasma-virtual-desktop.xml
    BASENAME plasma-virtual-desktop
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/plasma-window-management.xml
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
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-shell/xdg-shell-unstable-v6.xml
    BASENAME xdg-shell-v6
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml
    BASENAME relativepointer-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/pointer-gestures/pointer-gestures-unstable-v1.xml
    BASENAME pointer-gestures-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
    BASENAME pointer-constraints-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-foreign/xdg-foreign-unstable-v2.xml
    BASENAME xdg-foreign-unstable-v2
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml
    BASENAME idle-inhibit-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/appmenu.xml
    BASENAME appmenu
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/server-decoration-palette.xml
    BASENAME server-decoration-palette
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-output/xdg-output-unstable-v1.xml
    BASENAME xdg-output-unstable-v1
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
    BASENAME xdg-shell
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
    BASENAME xdg-decoration-unstable-v1
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/keystate.xml
    BASENAME keystate
)

set(CLIENT_GENERATED_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-fullscreen-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-output-management-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-org_kde_kwin_outputdevice-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-plasma-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-plasma-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-plasma-window-management-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-idle-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-fake-input-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-shadow-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-blur-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-contrast-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-slide-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-dpms-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-server-decoration-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-server-decoration-palette-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-text-input-v0-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-text-input-v2-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-shell-v6-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-relativepointer-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-pointer-gestures-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-pointer-constraints-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-foreign-unstable-v2-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-idle-inhibit-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-output-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-decoration-unstable-v1-client-protocol.h
)

set_source_files_properties(${CLIENT_GENERATED_FILES} PROPERTIES SKIP_AUTOMOC ON)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${KWayland_SOURCE_DIR}/src/client/protocols/remote-access.xml
    BASENAME remote-access
)

add_library(KF5WaylandClient ${CLIENT_LIB_SRCS})
add_library(KF5::WaylandClient ALIAS KF5WaylandClient)
ecm_generate_export_header(KF5WaylandClient
    BASE_NAME
        KWaylandClient
    EXPORT_FILE_NAME
        KWayland/Client/kwaylandclient_export.h
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.49 5.50 5.52 5.53 5.73
)
# TODO: add support for EXCLUDE_DEPRECATED_BEFORE_AND_AT to all KWayland libs
# needs fixing of undeprecated API being still implemented using own deprecated API


target_include_directories(KF5WaylandClient INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KWayland/Client>")

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
  appmenu.h
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
  idleinhibit.h
  keyboard.h
  keystate.h
  remote_access.h
  outputconfiguration.h
  outputmanagement.h
  outputdevice.h
  output.h
  pointer.h
  pointerconstraints.h
  plasmashell.h
  plasmavirtualdesktop.h
  plasmawindowmanagement.h
  plasmawindowmodel.h
  pointergestures.h
  region.h
  registry.h
  relativepointer.h
  seat.h
  server_decoration.h
  server_decoration_palette.h
  shadow.h
  shell.h
  shm_pool.h
  slide.h
  subcompositor.h
  subsurface.h
  surface.h
  touch.h
  textinput.h
  xdgdecoration.h
  xdgshell.h
  xdgforeign.h
  xdgoutput.h
)

install(FILES
  ${CLIENT_LIB_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KWayland/Client COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KWaylandClient_APIDOX_SRCS ${CLIENT_LIB_HEADERS} PARENT_SCOPE)
set(KWaylandClient_APIDOX_BUILD_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KWaylandClient LIB_NAME KF5WaylandClient DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5})
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
