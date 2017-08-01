set(kscreenplugin_SRCS
    kscreendoctor.cpp
    kscreenplugin.cpp
)

add_library(kscreenplugin SHARED ${kscreenplugin_SRCS})
target_link_libraries(kscreenplugin
    Qt5::Core
    Qt5::Qml
    KF5::Screen
    )

install(TARGETS kscreenplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kscreendoctor)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kscreendoctor)
