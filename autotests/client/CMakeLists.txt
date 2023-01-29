########################################################
# Test WaylandFullscreenShell
########################################################
if(Wayland_VERSION VERSION_GREATER "1.4.0")
  find_program(WESTON_EXECUTABLE weston DOC "Path to the weston executable.")
  if(WESTON_EXECUTABLE)
    set( testWaylandFullscreenShell_SRCS
            test_wayland_fullscreen_shell.cpp
        )
    add_executable(testWaylandFullscreenShell ${testWaylandFullscreenShell_SRCS})
    target_link_libraries( testWaylandFullscreenShell Qt6::Test KF6::WaylandClient Wayland::Client)
    add_test(NAME kwayland-testWaylandFullscreenShell COMMAND testWaylandFullscreenShell)
    ecm_mark_as_test(testWaylandFullscreenShell)
  else()
    message(STATUS "The weston executable was not found. Some autotests will not be executed.")
  endif()
endif()
