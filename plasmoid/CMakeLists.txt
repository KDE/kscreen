add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.kscreen\")

set(kscreenapplet_SRCS
    kscreenapplet.cpp
    ../common/osdaction.cpp
)

plasma_add_applet(org.kde.kscreen
  QML_SOURCES
    package/contents/ui/main.qml
    package/contents/ui/InhibitionHint.qml
    package/contents/ui/PresentationModeItem.qml
    package/contents/ui/ScreenLayoutSelection.qml
  CPP_SOURCES
    kscreenapplet.cpp
    ../common/osdaction.cpp
)

target_link_libraries(org.kde.kscreen PRIVATE
                      Qt::Qml
                      Qt::DBus
                      KF6::I18n
                      Plasma::Plasma
                      KF6::Screen
                      )
