set(kscreen_daemon_SRCS
    daemon.cpp
)

kde4_add_plugin(kded_kscreen
    ${kscreen_daemon_SRCS}
)

target_link_libraries(kded_kscreen
    ${KDE4_KDECORE_LIBS}
)

install(TARGETS kded_kscreen DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

set(daemon_testapp_SRCS
    main.cpp
    testapp.cpp
    serializer.cpp
    generator.cpp
)

kde4_add_executable(kscreen_daemon_testapp ${daemon_testapp_SRCS})
target_link_libraries(kscreen_daemon_testapp ${QT_QTCORE_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${KDE4_KDECORE_LIBS} ${LibKScreen_LIBRARY} ${QJSON_LIBRARIES} qjson)