project(KScreen)
set(PROJECT_VERSION "5.3.90")

cmake_minimum_required(VERSION 2.8.12)

set(KSCREEN_VERSION ${PROJECT_VERSION})

add_definitions("-DKSCREEN_VERSION=\"${KSCREEN_VERSION}\"")

find_package(ECM 1.6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED COMPONENTS QuickWidgets X11Extras Test)
find_package(KF5 REQUIRED COMPONENTS DBusAddons ConfigWidgets I18n XmlGui GlobalAccel)

set(MIN_LIBKSCREEN_VERSION "5.2.91")
find_package(KF5Screen ${MIN_LIBKSCREEN_VERSION} REQUIRED)

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
