cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.92.0") # handled by release scripts
project(KWayland VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.92.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(FeatureSummary)
include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(CMakeFindFrameworks)
include(ECMQtDeclareLoggingCategory)

include(ECMPoQmTools)
include(ECMAddQch)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control how much of deprecated API is built [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT VARIABLE_PREFIX KWAYLAND
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwayland_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfigVersion.cmake"
                        SOVERSION 5)

# Dependencies
set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION}Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Private)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Concurrent WaylandClient)

# With Qt6 use qt_generate_wayland_protocol_{client,server}_sources instead of
# ecm_add_qtwayland_{client,server}_protocol (from ECM's QtWaylandScanner)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6WaylandCompositor CONFIG REQUIRED)
else()
    find_package(QtWaylandScanner REQUIRED)
endif()

find_package(Wayland 1.15 COMPONENTS Client Server)
set_package_properties(Wayland PROPERTIES
                       TYPE REQUIRED
                      )

find_package(WaylandScanner)

find_package(WaylandProtocols 1.15)
set_package_properties(WaylandProtocols PROPERTIES TYPE REQUIRED)

find_package(EGL)
set_package_properties(EGL PROPERTIES TYPE REQUIRED)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(CheckIncludeFile)

find_package(PlasmaWaylandProtocols 1.4.0 CONFIG)
set_package_properties(PlasmaWaylandProtocols PROPERTIES TYPE REQUIRED)

# adjusting CMAKE_C_FLAGS to get wayland protocols to compile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f02)
# Subdirectories
ecm_install_po_files_as_qm(po)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Wayland")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Wayland_QCH
        FILE KF5WaylandQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5WaylandQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5WaylandConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5WaylandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5WaylandTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5WaylandTargets.cmake NAMESPACE KF5:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwayland_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KWayland COMPONENT Devel)

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF5WaylandClient
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}
      DEPS Qt${QT_MAJOR_VERSION}Gui
    INSTALL)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
