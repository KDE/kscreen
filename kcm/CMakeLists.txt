add_subdirectory(src)
add_subdirectory(qml)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kcm_kscreen.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/kcm_kscreen.desktop
               @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
