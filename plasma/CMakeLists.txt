plasma_install_package(package org.kde.plasma.kscreen)

set(applet_SRCS
    main.cpp
    kscreenapplet.cpp)

install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins.qmltypes 
    ${CMAKE_CURRENT_SOURCE_DIR}/qmldir 
    DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/kscreen)
add_library(plasma_applet_kscreen SHARED ${applet_SRCS})
target_link_libraries(plasma_applet_kscreen
    Qt5::QuickWidgets
    Qt5::DBus
    KF5::Screen
    KF5::Service
    KF5::I18n
)

install(TARGETS plasma_applet_kscreen
        DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/kscreen)
