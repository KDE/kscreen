include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kscreen_daemon_SRCS
    daemon.cpp
    serializer.cpp
    generator.cpp
    device.cpp
)

qt4_add_dbus_interface(kscreen_daemon_SRCS
    org.freedesktop.DBus.Properties.xml
    freedesktop_interface)
qt4_add_dbus_adaptor(kscreen_daemon_SRCS
    org.kde.KScreen.xml
    daemon.h
    KScreenDaemon
)

kde4_add_plugin(kded_kscreen
    ${kscreen_daemon_SRCS}
)

target_link_libraries(kded_kscreen
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${LibKScreen_LIBRARY} ${QJSON_LIBRARIES} qjson
)

install(TARGETS kded_kscreen DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)