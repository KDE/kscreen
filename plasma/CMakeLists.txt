set(applet_SRCS
    kscreenapplet.cpp)

kde4_add_plugin(plasma_applet_kscreen ${applet_SRCS})
target_link_libraries(plasma_applet_kscreen
    ${KDE4_PLASMA_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${LibKScreen_LIBRARY}
)

install(TARGETS plasma_applet_kscreen DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.plasma.kscreen.qml)
install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-kscreen-qml.desktop)
