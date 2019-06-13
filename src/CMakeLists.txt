add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(tools)

install( FILES org_kde_kwayland.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR} )


if(BUILD_QCH)
    macro(_make_absolute var_name base_path)
        set(_result)
        foreach(_path ${${var_name}})
            if(IS_ABSOLUTE "${_path}")
                list(APPEND _result "${_path}")
            else()
                list(APPEND _result "${base_path}/${_path}")
            endif()
        endforeach()
        set(${var_name} ${_result})
    endmacro()

    _make_absolute(KWaylandClient_APIDOX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/client")
    _make_absolute(KWaylandServer_APIDOX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/server")

    ecm_add_qch(
        KF5Wayland_QCH
        NAME KWayland
        BASE_NAME KF5Wayland
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KWaylandClient_APIDOX_SRCS}
            ${KWaylandServer_APIDOX_SRCS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Gui_QCH
        BLANK_MACROS
            KWAYLANDCLIENT_EXPORT
            KWAYLANDCLIENT_DEPRECATED
            KWAYLANDCLIENT_DEPRECATED_EXPORT
            KWAYLANDSERVER_EXPORT
            KWAYLANDSERVER_DEPRECATED
            KWAYLANDSERVER_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
