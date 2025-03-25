add_definitions(-DTRANSLATION_DOMAIN=\"hdrcalibrator\")
qt_policy(SET QTP0001 NEW)

add_executable(hdrcalibrator)
qt_add_qml_module(hdrcalibrator
    URI org.kde.hdrcalibrator
    VERSION 1.0
    QML_FILES
        ui/Main.qml
    RESOURCES
        "ui/images/graz.png"
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
    KF6::I18n
    KF6::I18nQml
    KF6::Screen
    Qt::WaylandClient
    Qt::GuiPrivate
    Qt::QuickControls2
)
target_include_directories(hdrcalibrator PUBLIC ${CMAKE_BINARY_DIR})

install(TARGETS hdrcalibrator ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
