add_executable(kscreen_osd_service main.cpp osdmanager.cpp osd.cpp ../common/osdaction.cpp)

qt_add_dbus_adaptor(DBUS_SRC org.kde.kscreen.osdService.xml osdmanager.h KScreen::OsdManager)
target_sources(kscreen_osd_service PRIVATE ${DBUS_SRC})

target_link_libraries(kscreen_osd_service PRIVATE
    Qt::DBus
    Qt::Quick
    KF5::I18n
    KF5::WindowSystem
    KF5::Screen
    LayerShellQt::Interface
)

set(QML_FILES
    qml/OsdSelector.qml
)

install(FILES ${QML_FILES} DESTINATION ${KDE_INSTALL_DATADIR}/kded_kscreen/qml)

install(TARGETS kscreen_osd_service DESTINATION ${KDE_INSTALL_LIBEXECDIR})

ecm_generate_dbus_service_file(
    NAME org.kde.kscreen.osdService
    EXECUTABLE ${KDE_INSTALL_FULL_LIBEXECDIR}
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

ecm_install_configured_files(INPUT plasma-kscreen-osd.service @ONLY
    DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)
