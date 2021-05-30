add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

set(kscreen_daemon_SRCS
    daemon.cpp
    config.cpp
    output.cpp
    generator.cpp
    device.cpp
    osd.cpp
    osdmanager.cpp
    osdaction.cpp
    ${CMAKE_SOURCE_DIR}/common/globals.cpp
    ${CMAKE_SOURCE_DIR}/common/control.cpp
    ${CMAKE_SOURCE_DIR}/common/orientation_sensor.cpp
    ${CMAKE_SOURCE_DIR}/common/utils.cpp
)

ecm_qt_declare_logging_category(kscreen_daemon_SRCS HEADER kscreen_daemon_debug.h IDENTIFIER KSCREEN_KDED CATEGORY_NAME kscreen.kded DESCRIPTION "kscreen kded (kscreen)" EXPORT KSCREEN)

qt_add_dbus_interface(kscreen_daemon_SRCS
    org.freedesktop.DBus.Properties.xml
    freedesktop_interface)
qt_add_dbus_adaptor(kscreen_daemon_SRCS
    org.kde.KScreen.xml
    daemon.h
    KScreenDaemon
)

add_library(kscreen MODULE ${kscreen_daemon_SRCS})

target_link_libraries(kscreen Qt::Widgets
                              Qt::DBus
                              Qt::Quick
                              Qt::Sensors
                              KF5::Declarative
                              KF5::Screen
                              KF5::DBusAddons
                              KF5::I18n
                              KF5::XmlGui
                              KF5::GlobalAccel)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/kscreen.desktop
               @ONLY)

kcoreaddons_desktop_to_json(kscreen ${CMAKE_CURRENT_BINARY_DIR}/kscreen.desktop)

install(TARGETS kscreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)

set(QML_FILES
    qml/Osd.qml
    qml/OsdItem.qml
    qml/OsdSelector.qml
    qml/OutputIdentifier.qml
)

install(FILES ${QML_FILES} DESTINATION ${KDE_INSTALL_DATADIR}/kded_kscreen/qml)
