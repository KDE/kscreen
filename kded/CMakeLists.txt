add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

kcoreaddons_add_plugin(kscreen INSTALL_NAMESPACE "kf6/kded")

target_sources(kscreen PRIVATE
    daemon.cpp daemon.h
    config.cpp
    generator.cpp generator.h
    device.cpp device.h
)

qt_add_dbus_interface(dbus_SRCS
    org.freedesktop.DBus.Properties.xml
    freedesktop_interface)
qt_add_dbus_interface(dbus_SRCS
    ../osd/org.kde.kscreen.osdService.xml
    osdservice_interface)

target_sources(kscreen PRIVATE ${dbus_SRCS})

target_link_libraries(kscreen PRIVATE
                              kscreen_common
                              Qt::Widgets
                              Qt::DBus
                              Qt::Quick
                              KF6::CoreAddons
                              KF6::Screen
                              KF6::DBusAddons
                              KF6::I18n
                              KF6::XmlGui
)

if(WITH_X11)
    target_link_libraries(kscreen PRIVATE X11::X11 X11::Xi X11::XCB XCB::ATOM)
endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/kscreen.json
               @ONLY)
