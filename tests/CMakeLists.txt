add_definitions("-DTEST_DATA=\\\"${CMAKE_CURRENT_SOURCE_DIR}/configs/\\\"")
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${X11_Xrandr_INCLUDE_PATH})

qt4_generate_moc(testgenerator.cpp ${CMAKE_CURRENT_BINARY_DIR}/testgenerator.moc)

set(testgenerator_SRCS
    testgenerator.cpp
    testgenerator.moc
    ../kded/generator.cpp
    ../kded/device.cpp)

qt4_add_dbus_interface(testgenerator_SRCS
    ../kded/org.freedesktop.DBus.Properties.xml
    upower_interface)

kde4_add_unit_test(testgenerator ${testgenerator_SRCS})

target_link_libraries(testgenerator ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kscreen)

add_test(testgenerator ${CMAKE_CURRENT_BINARY_DIR}/testgenerator)