add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kscreen\")

set(kcm_kscreen_SRCS
    config_handler.cpp
    kcm.cpp
    output_identifier.cpp
    output_model.cpp
    ${CMAKE_SOURCE_DIR}/common/utils.cpp
    ${CMAKE_SOURCE_DIR}/common/control.cpp
    ${CMAKE_SOURCE_DIR}/common/globals.cpp
    ${CMAKE_SOURCE_DIR}/common/orientation_sensor.cpp
)

ecm_qt_declare_logging_category(kcm_kscreen_SRCS
    HEADER
        kcm_screen_debug.h
    IDENTIFIER
        KSCREEN_KCM
    CATEGORY_NAME
        kscreen.kcm
)

add_library(kcm_kscreen MODULE ${kcm_kscreen_SRCS})

target_link_libraries(kcm_kscreen
    Qt5::Sensors
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::Declarative
    KF5::I18n
    KF5::PlasmaQuick
    KF5::QuickAddons
    KF5::Screen
)

kcoreaddons_desktop_to_json(kcm_kscreen "kcm_kscreen.desktop" SERVICE_TYPES kcmodule.desktop)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES kcm_kscreen.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(TARGETS kcm_kscreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_kscreen kcms)
