add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.kscreen\")

plasma_add_applet(org.kde.kscreen
  QML_SOURCES
    main.qml
    InhibitionHint.qml
    PresentationModeItem.qml
    ScreenLayoutSelection.qml
  CPP_SOURCES
    kscreenapplet.cpp
)

target_link_libraries(org.kde.kscreen PRIVATE
                      Qt::Qml
                      KF6::I18n
                      Plasma::Plasma
                      KF6::Screen
                      kscreen_common
                      )
