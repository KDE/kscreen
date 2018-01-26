find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                    )

set(kscreenplugin_SRCS
    kscreendoctor.cpp
    kscreenplugin.cpp
    xtouchscreen.cpp
)

add_library(kscreenplugin SHARED ${kscreenplugin_SRCS})
target_link_libraries(kscreenplugin
                        Qt5::Core
                        Qt5::Qml
                        Qt5::Sensors
                        KF5::Screen
                        ${X11_X11_LIB}
                        ${X11_Xinput_LIB}
                    )

install(TARGETS kscreenplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kscreendoctor)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kscreendoctor)
