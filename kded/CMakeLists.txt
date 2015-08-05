add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

QT5_ADD_RESOURCES(qtsourceview_RC_SRCS images/image.qrc)

set(kscreen_daemon_SRCS
    debug.cpp
    daemon.cpp
    serializer.cpp
    generator.cpp
    device.cpp
    osdwidget.cpp
    ${qtsourceview_RC_SRCS}
)

qt5_add_dbus_interface(kscreen_daemon_SRCS
    org.freedesktop.DBus.Properties.xml
    freedesktop_interface)
qt5_add_dbus_adaptor(kscreen_daemon_SRCS
    org.kde.KScreen.xml
    daemon.h
    KScreenDaemon
)

set(kscreen_daemon_LIBS
    Qt5::DBus
    KF5::Screen
    KF5::DBusAddons
    KF5::I18n
    KF5::XmlGui
    KF5::GlobalAccel
    KF5::Service
    KF5::IconThemes
)

add_library(kded_kscreen MODULE ${kscreen_daemon_SRCS})
target_link_libraries(kded_kscreen ${kscreen_daemon_LIBS})

## Test executable for OSD Widget, not installed
add_executable(osdwidget_test osdwidget_test.cpp ${kscreen_daemon_SRCS})
target_link_libraries(osdwidget_test ${kscreen_daemon_LIBS})
##

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/kscreen.desktop
               @ONLY)


install(TARGETS kded_kscreen DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
