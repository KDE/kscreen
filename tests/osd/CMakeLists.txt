include_directories(${CMAKE_SOURCE_DIR}/kcm/src)

add_executable(osdtest main.cpp osdtest.cpp ../../kded/osd.cpp ../../kded/osdmanager.cpp ../../kcm/src/utils.cpp)
target_link_libraries(osdtest Qt5::Core
                              Qt5::DBus
                              Qt5::Quick
                              Qt5::Qml
                              KF5::Screen
                              KF5::PlasmaQuick
                              KF5::I18n
                              KF5::Declarative
                              KF5::IconThemes
                              KF5::QuickAddons
)

add_test(kscreen-kded-osdtest osdtest)
ecm_mark_as_test(osdtest)
