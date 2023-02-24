add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.kscreen\")

set(kscreenapplet_SRCS
    kscreenapplet.cpp
    ../common/osdaction.cpp
)

add_library(plasma_applet_kscreen MODULE ${kscreenapplet_SRCS})

target_link_libraries(plasma_applet_kscreen
                      Qt::Qml
                      Qt::DBus
                      KF6::I18n
                      KF6::Plasma
                      KF6::Screen)

install(TARGETS plasma_applet_kscreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)

plasma_install_package(package org.kde.kscreen)
