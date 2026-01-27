add_definitions(-DTRANSLATION_DOMAIN=\"hdrcalibrator\")
qt_policy(SET QTP0001 NEW)

add_executable(hdrcalibrator)
ecm_add_qml_module(hdrcalibrator URI org.kde.hdrcalibrator GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)

ecm_target_qml_sources(hdrcalibrator
    SOURCES
        ui/Main.qml
    RESOURCES
        ui/images/graz.avif
        ui/images/plasma-symbolic.svg
)

qt6_generate_wayland_protocol_client_sources(hdrcalibrator
    FILES ${WaylandProtocols_DATADIR}/staging/color-management/color-management-v1.xml
)

target_sources(hdrcalibrator PUBLIC
    hdrcalibrator.cpp
    hdrhelper.cpp
)

target_link_libraries(hdrcalibrator PUBLIC
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::Crash
    KF6::I18n
    KF6::I18nQml
    KF6::Screen
    Qt::Qml
    Qt::WaylandClient
    Qt::GuiPrivate
    Qt::QuickControls2
    Qt::DBus
)
target_include_directories(hdrcalibrator PUBLIC ${CMAKE_BINARY_DIR})

ecm_qt_declare_logging_category(hdrcalibrator
    HEADER
        hdrcalibrator_debug.h
    IDENTIFIER
        HDRCALIBRATOR
    CATEGORY_NAME
        hdrcalibrator
     DESCRIPTION "hdr calibrator" EXPORT KSCREEN
)

ecm_finalize_qml_module(hdrcalibrator)
install(TARGETS hdrcalibrator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
