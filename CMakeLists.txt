cmake_minimum_required(VERSION 3.0)

project(KScreen)
set(PROJECT_VERSION "5.16.80")

set(KSCREEN_VERSION ${PROJECT_VERSION})

add_definitions("-DKSCREEN_VERSION=\"${KSCREEN_VERSION}\"")

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.58.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    DBusAddons
    Declarative
    GlobalAccel
    I18n
    IconThemes
    KCMUtils
    Plasma
    XmlGui
)
# WARNING PlasmaQuick provides unversioned CMake config
find_package(KF5 REQUIRED COMPONENTS PlasmaQuick)

set(MIN_LIBKSCREEN_VERSION "5.2.91")
find_package(KF5Screen ${MIN_LIBKSCREEN_VERSION} REQUIRED)

add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(plasmoid)
add_subdirectory(tests)
add_subdirectory(console)

if (${ECM_VERSION} STRGREATER "5.58.0")
    install(FILES kscreen.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kscreen.categories  DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
