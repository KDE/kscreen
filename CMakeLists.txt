cmake_minimum_required(VERSION 3.16)

project(KScreen)
set(PROJECT_VERSION "6.4.3")
set(KSCREEN_VERSION ${PROJECT_VERSION})

add_definitions("-DKSCREEN_VERSION=\"${KSCREEN_VERSION}\"")

set(PROJECT_DEP_VERSION "6.4.3")
set(QT_MIN_VERSION "6.8.0")
set(KF6_MIN_VERSION "6.14.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMConfiguredInstall)
include(ECMGenerateDBusServiceFile)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)
include(ECMQmlModule)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Gui Test Sensors QuickControls2)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Config
    DBusAddons
    I18n
    KCMUtils
    Svg
    XmlGui
    Crash
)
if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
    find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()

find_package(KF6Screen ${PROJECT_DEP_VERSION} REQUIRED)
find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)
find_package(PlasmaQuick ${PROJECT_DEP_VERSION} REQUIRED)

find_package(LayerShellQt)
set_package_properties(LayerShellQt PROPERTIES
    DESCRIPTION "Layer shell Qt bindings"
    URL "https://invent.kde.org/plasma/layer-shell-qt"
    PURPOSE "Required for the screen selection osd"
    TYPE REQUIRED
)

find_package(Qt6WaylandClient COMPONENTS Private)
find_package(WaylandProtocols 1.41)
set_package_properties(WaylandProtocols PROPERTIES
    TYPE REQUIRED
    PURPOSE "Collection of Wayland protocols that add functionality not available in the Wayland core protocol"
    URL "https://gitlab.freedesktop.org/wayland/wayland-protocols/"
)

option(WITH_X11 "Enable X11 support" ON)

if(WITH_X11)
    find_package(X11)
    set_package_properties(X11 PROPERTIES
        DESCRIPTION "X11 libraries"
        URL "https://www.x.org"
        PURPOSE "Required for touchscreen XInput mapping to internal display"
        TYPE REQUIRED
    )
    find_package(X11_XCB REQUIRED)
    find_package(XCB REQUIRED COMPONENTS XCB ATOM)
endif()

configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

ecm_set_disabled_deprecation_versions(QT 6.8.0
    KF 6.12.0
)

add_subdirectory(common)
add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(osd)
add_subdirectory(plasmoid)
add_subdirectory(tests)
add_subdirectory(console)
add_subdirectory(hdrcalibrator)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ecm_qt_install_logging_categories(EXPORT KSCREEN FILE kscreen.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
