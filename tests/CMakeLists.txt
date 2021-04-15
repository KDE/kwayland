include(ECMMarkAsTest)

add_executable(copyClient copyclient.cpp)
target_link_libraries(copyClient KF5::WaylandClient)
ecm_mark_as_test(copyClient)

add_executable(pasteClient pasteclient.cpp)
target_link_libraries(pasteClient Qt5::Concurrent KF5::WaylandClient)
ecm_mark_as_test(pasteClient)

if (HAVE_LINUX_INPUT_H)
    add_executable(touchClientTest touchclienttest.cpp)
    target_link_libraries(touchClientTest KF5::WaylandClient)


    add_executable(panelTest paneltest.cpp)
    target_link_libraries(panelTest KF5::WaylandClient)
    ecm_mark_as_test(panelTest)

    add_executable(qtwayland-integration-test qtwaylandintegrationtest.cpp)
    target_link_libraries(qtwayland-integration-test Qt5::Core Qt5::Gui KF5::WaylandClient)
    ecm_mark_as_test(qtwayland-integration-test)

    add_executable(subsurface-test subsurfacetest.cpp)
    target_link_libraries(subsurface-test Qt5::Core Qt5::Gui KF5::WaylandClient)
    ecm_mark_as_test(subsurface-test)
endif()

add_executable(shadowTest shadowtest.cpp)
target_link_libraries(shadowTest KF5::WaylandClient)
ecm_mark_as_test(shadowTest)


if (Qt5Widgets_FOUND)
    add_executable(dpmsTest dpmstest.cpp)
    target_link_libraries(dpmsTest KF5::WaylandClient Qt5::Widgets)
    ecm_mark_as_test(dpmsTest)
endif()

add_executable(plasmasurface-test plasmasurfacetest.cpp)
target_link_libraries(plasmasurface-test Qt5::Gui KF5::WaylandClient)
ecm_mark_as_test(plasmasurface-test)

add_executable(xdgforeign-test xdgforeigntest.cpp)
target_link_libraries(xdgforeign-test Qt5::Gui KF5::WaylandClient)
ecm_mark_as_test(xdgforeign-test)

add_executable(xdg-test xdgtest.cpp)
target_link_libraries(xdg-test Qt5::Gui KF5::WaylandClient)
ecm_mark_as_test(xdg-test)

