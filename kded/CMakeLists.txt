add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

set(kscreen_daemon_SRCS
    debug.cpp
    daemon.cpp
    serializer.cpp
    generator.cpp
    device.cpp
)

qt5_add_dbus_interface(kscreen_daemon_SRCS
    org.freedesktop.DBus.Properties.xml
    freedesktop_interface)
qt5_add_dbus_adaptor(kscreen_daemon_SRCS
    org.kde.KScreen.xml
    daemon.h
    KScreenDaemon
)

add_library(kscreen MODULE ${kscreen_daemon_SRCS})

target_link_libraries(kscreen Qt5::Widgets Qt5::DBus KF5::Screen KF5::DBusAddons KF5::I18n KF5::XmlGui KF5::GlobalAccel)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/kscreen.desktop
               @ONLY)

kcoreaddons_desktop_to_json(kscreen ${CMAKE_CURRENT_BINARY_DIR}/kscreen.desktop)

install(TARGETS kscreen DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kded)
