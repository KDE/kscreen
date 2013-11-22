set(applet_SRCS
    kscreenapplet.cpp)

kde4_add_plugin(plasma_applet_kscreen ${applet_SRCS})
target_link_libraries(plasma_applet_kscreen
    ${KDE4_PLASMA_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${LibKScreen_LIBRARY}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plasma-applet-kscreen.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/plasma-applet-kscreen.desktop
               @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package/metadata.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/package/metadata.desktop
               @ONLY)


install(TARGETS plasma_applet_kscreen
        DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-applet-kscreen.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(DIRECTORY package/contents/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.plasma.kscreen.qml/contents)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/package/metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.plasma.kscreen.qml)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-kscreen-qml.desktop)
