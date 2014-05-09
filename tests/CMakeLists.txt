include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/../ ${X11_Xrandr_INCLUDE_PATH})

set(testgenerator_SRCS
    testgenerator.cpp
    ../kded/generator.cpp
    ../kded/device.cpp)

qt5_add_dbus_interface(testgenerator_SRCS
    ../kded/org.freedesktop.DBus.Properties.xml
    upower_interface)

add_executable(testgenerator ${testgenerator_SRCS})
target_compile_definitions(testgenerator PRIVATE "-DTEST_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/configs/\"")

target_link_libraries(testgenerator Qt5::Test Qt5::DBus Qt5::Gui KF5::Screen)

add_test(kscreen-testgenerator testgenerator)
ecm_mark_as_test(testgenerator)
