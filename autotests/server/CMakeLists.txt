########################################################
# Test WaylandServerDisplay
########################################################
set( testWaylandServerDisplay_SRCS
        test_display.cpp
    )
add_executable(testWaylandServerDisplay ${testWaylandServerDisplay_SRCS})
target_link_libraries( testWaylandServerDisplay Qt6::Test Qt6::Gui KF6::WaylandServer Wayland::Server)
add_test(NAME kwayland-testWaylandServerDisplay COMMAND testWaylandServerDisplay)
ecm_mark_as_test(testWaylandServerDisplay)

########################################################
# Test WaylandServerSeat
########################################################
set( testWaylandServerSeat_SRCS
        test_seat.cpp
    )
add_executable(testWaylandServerSeat ${testWaylandServerSeat_SRCS})
target_link_libraries( testWaylandServerSeat Qt6::Test Qt6::Gui KF6::WaylandServer Wayland::Server)
add_test(NAME kwayland-testWaylandServerSeat COMMAND testWaylandServerSeat)
ecm_mark_as_test(testWaylandServerSeat)

########################################################
# QtSurfaceExtenstion Helper
########################################################
add_executable(surfaceExtensionHelper surfaceextension_helper.cpp)
target_link_libraries( surfaceExtensionHelper Qt6::Gui)
ecm_mark_as_test(surfaceExtensionHelper)

########################################################
# Test QtSurfaceExtenstion
########################################################
set( testQtSurfaceExtension_SRCS
        test_qt_surface_extension.cpp
    )
add_executable(testQtSurfaceExtension ${testQtSurfaceExtension_SRCS})
target_link_libraries( testQtSurfaceExtension Qt6::Test Qt6::Gui KF6::WaylandServer)
add_test(NAME kwayland-testQtSurfaceExtension COMMAND testQtSurfaceExtension)
ecm_mark_as_test(testQtSurfaceExtension)

########################################################
# Test No XDG_RUNTIME_DIR
########################################################
add_executable(testNoXdgRuntimeDir test_no_xdg_runtime_dir.cpp)
target_link_libraries( testNoXdgRuntimeDir Qt6::Test KF6::WaylandServer)
add_test(NAME kwayland-testNoXdgRuntimeDir COMMAND testNoXdgRuntimeDir)
ecm_mark_as_test(testNoXdgRuntimeDir)

########################################################
# Test Tablet Interface
########################################################
add_executable(testTabletInterface)

if (TARGET Qt6::WaylandClient)
    qt_generate_wayland_protocol_client_sources(testTabletInterface
        FILES ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml
    )
else()
    ecm_add_qtwayland_client_protocol(TABLET_SRCS
        PROTOCOL ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml
        BASENAME tablet-unstable-v2
    )
endif()

target_sources(testTabletInterface PRIVATE test_tablet_interface.cpp ${TABLET_SRCS})
target_link_libraries( testTabletInterface Qt6::Test KF6::WaylandServer KF6::WaylandClient Wayland::Client)
add_test(NAME kwayland-testTabletInterface COMMAND testTabletInterface)
ecm_mark_as_test(testTabletInterface)
