project(KScreen)
set(PROJECT_VERSION "5.2.1")

cmake_minimum_required(VERSION 2.8.12)

set(KSCREEN_VERSION ${PROJECT_VERSION})

add_definitions("-DKSCREEN_VERSION=\"${KSCREEN_VERSION}\"")

find_package(ECM 1.5.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED COMPONENTS QuickWidgets X11Extras Test)
find_package(KF5 REQUIRED COMPONENTS Screen DBusAddons ConfigWidgets I18n XmlGui GlobalAccel)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(FeatureSummary)


add_subdirectory(icons)
add_subdirectory(kcm)
#add_subdirectory(plasma)
add_subdirectory(kded)
add_subdirectory(tests)
add_subdirectory(console)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
