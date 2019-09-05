include_directories(
    ${CMAKE_BINARY_DIR}/kded
)

add_executable(osdtest main.cpp
                       osdtest.cpp
                       ../../kded/osd.cpp
                       ../../kded/osdmanager.cpp
                       ../../kded/osdaction.cpp
                       ../../common/utils.cpp
)

target_link_libraries(osdtest Qt5::Core
                              Qt5::DBus
                              Qt5::Quick
                              Qt5::Qml
                              KF5::Screen
                              KF5::I18n
                              KF5::Declarative
)

add_test(NAME kscreen-kded-osdtest COMMAND osdtest)
ecm_mark_as_test(osdtest)
