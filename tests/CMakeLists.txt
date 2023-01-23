include(ECMMarkAsTest)

add_executable(copyClient copyclient.cpp)
target_link_libraries(copyClient KF6::WaylandClient)
ecm_mark_as_test(copyClient)

add_executable(pasteClient pasteclient.cpp)
target_link_libraries(pasteClient Qt${QT_MAJOR_VERSION}::Concurrent KF6::WaylandClient)
ecm_mark_as_test(pasteClient)

add_executable(touchClientTest touchclienttest.cpp)
target_link_libraries(touchClientTest KF6::WaylandClient)

add_executable(panelTest paneltest.cpp)
target_link_libraries(panelTest KF6::WaylandClient)
ecm_mark_as_test(panelTest)

add_executable(qtwayland-integration-test qtwaylandintegrationtest.cpp)
target_link_libraries(qtwayland-integration-test Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui KF6::WaylandClient)
ecm_mark_as_test(qtwayland-integration-test)

add_executable(subsurface-test subsurfacetest.cpp)
target_link_libraries(subsurface-test Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui KF6::WaylandClient)
ecm_mark_as_test(subsurface-test)

add_executable(shadowTest shadowtest.cpp)
target_link_libraries(shadowTest KF6::WaylandClient)
ecm_mark_as_test(shadowTest)


if (TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    add_executable(dpmsTest dpmstest.cpp)
    target_link_libraries(dpmsTest KF6::WaylandClient Qt${QT_MAJOR_VERSION}::Widgets)
    ecm_mark_as_test(dpmsTest)
endif()

add_executable(plasmasurface-test plasmasurfacetest.cpp)
target_link_libraries(plasmasurface-test Qt${QT_MAJOR_VERSION}::Gui KF6::WaylandClient)
ecm_mark_as_test(plasmasurface-test)

add_executable(xdgforeign-test xdgforeigntest.cpp)
target_link_libraries(xdgforeign-test Qt${QT_MAJOR_VERSION}::Gui KF6::WaylandClient)
ecm_mark_as_test(xdgforeign-test)

add_executable(xdg-test xdgtest.cpp)
target_link_libraries(xdg-test Qt${QT_MAJOR_VERSION}::Gui KF6::WaylandClient)
ecm_mark_as_test(xdg-test)

