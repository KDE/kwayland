cmake_minimum_required(VERSION 2.8.12)
set(PROJECT_VERSION "5.1.90")
set(PROJECT_VERSION_MAJOR 5)

project(KWAYLAND)

# ECM setup
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakeFindFrameworks)

include(ECMPoQmTools)

set(KF5_VERSION ${PROJECT_VERSION})

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KWAYLAND
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwayland_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfigVersion.cmake"
                        SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION 5.3.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)

find_package(Wayland 1.3 COMPONENTS Client Server)
set_package_properties(Wayland PROPERTIES
                       TYPE REQUIRED
                      )

find_package(WaylandScanner)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

# adjusting CMAKE_C_FLAGS to get wayland protocols to compile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

# Subdirectories
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Wayland")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5WaylandConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5WaylandTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5WaylandTargets.cmake NAMESPACE KF5:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwayland_version.h
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
