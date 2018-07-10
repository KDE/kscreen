cmake_minimum_required(VERSION 3.0)

project(KScreen)
set(PROJECT_VERSION "5.13.80")

set(KSCREEN_VERSION ${PROJECT_VERSION})

add_definitions("-DKSCREEN_VERSION=\"${KSCREEN_VERSION}\"")

set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.42.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS QuickWidgets Test)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    DBusAddons
    Config
    ConfigWidgets
    I18n
    XmlGui
    GlobalAccel
    WidgetsAddons
    Declarative
    IconThemes
    Plasma
)
# WARNING PlasmaQuick provides unversioned CMake config
find_package(KF5 REQUIRED COMPONENTS PlasmaQuick)

set(MIN_LIBKSCREEN_VERSION "5.2.91")
find_package(KF5Screen ${MIN_LIBKSCREEN_VERSION} REQUIRED)

add_subdirectory(icons)
add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(tests)
add_subdirectory(console)

install( FILES kscreen.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
