#project( kcm_kscreen2 )


set(kscreen_configmodule_SRCS
    configmodule.cpp
    qmloutput.cpp
    qmloutputcomponent.cpp
    qmlscreen.cpp
)

ecm_qt_declare_logging_category(kscreen_configmodule_SRCS
                                HEADER debug_p.h
                                IDENTIFIER KSCREEN_KCM
                                CATEGORY_NAME kscreen.kcm
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kcm_kscreen2.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/kcm_kscreen2.desktop
               @ONLY)

add_library(kcm_kscreen2 MODULE ${kscreen_configmodule_SRCS})

target_link_libraries(kcm_kscreen2
    Qt5::DBus
    Qt5::Gui
    Qt5::Quick
    Qt5::Qml
    KF5::Screen
    KF5::Plasma
    KF5::I18n
    KF5::QuickAddons
    )
kcoreaddons_desktop_to_json(kcm_kscreen2 "${CMAKE_CURRENT_BINARY_DIR}/kcm_kscreen2.desktop")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kscreen2.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kcm_kscreen2 DESTINATION ${PLUGIN_INSTALL_DIR}/kcms)
kpackage_install_package(package kcm_kscreen2 kcms)
