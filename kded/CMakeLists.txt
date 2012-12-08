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

