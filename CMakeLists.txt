cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.240.0") # handled by release scripts
project(KWayland VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.240.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMPoQmTools)
include(ECMAddQch)
include(CMakeFindFrameworks)
include(CMakePackageConfigHelpers)
include(CheckIncludeFile)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control how much of deprecated API is built [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kwayland_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/client/kwayland_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KWAYLAND
                        VERSION_HEADER "${kwayland_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6WaylandConfigVersion.cmake"
                        SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Private)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Concurrent)
find_package(Qt6WaylandClient ${REQUIRED_QT_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS Private)

find_package(Wayland 1.15 COMPONENTS Client)
set_package_properties(Wayland PROPERTIES
                       TYPE REQUIRED
                      )

find_package(WaylandScanner)

find_package(WaylandProtocols 1.15)
set_package_properties(WaylandProtocols PROPERTIES TYPE REQUIRED)

find_package(EGL)
set_package_properties(EGL PROPERTIES TYPE REQUIRED)

find_package(PlasmaWaylandProtocols 1.9.0 CONFIG)
set_package_properties(PlasmaWaylandProtocols PROPERTIES TYPE REQUIRED)

# adjusting CMAKE_C_FLAGS to get wayland protocols to compile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

ecm_set_disabled_deprecation_versions(
    QT 6.4.0
)

check_cxx_source_compiles("
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    const int size = 10;
    int fd = memfd_create(\"test\", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    ftruncate(fd, size);
    fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    mmap(nullptr, size, PROT_WRITE, MAP_SHARED, fd, 0);
    return 0;
}" HAVE_MEMFD)

# Subdirectories
ecm_install_po_files_as_qm(po)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Wayland")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Wayland_QCH
        FILE KF6WaylandQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6WaylandQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6WaylandConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6WaylandConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6WaylandConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6WaylandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF6WaylandTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6WaylandTargets.cmake NAMESPACE KF6:: )


install(FILES ${kwayland_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KWayland COMPONENT Devel)

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF6WaylandClient
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KWayland
      DEPS Qt6Gui
    INSTALL)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
