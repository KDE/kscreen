include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/configmodule
                    ${CMAKE_BINARY_DIR}/configmodule
)

macro(ADD_KDED_TEST testname)
    set(test_SRCS
        ${testname}.cpp
        ${CMAKE_SOURCE_DIR}/configmodule/configmodule.cpp
        ${CMAKE_SOURCE_DIR}/configmodule/modeselector.cpp
        ${CMAKE_SOURCE_DIR}/configmodule/qmloutput.cpp
        ${CMAKE_SOURCE_DIR}/configmodule/qmloutputcomponent.cpp
        ${CMAKE_SOURCE_DIR}/configmodule/qmlscreen.cpp
#         # These are needed for the config generator to work
#         ${CMAKE_SOURCE_DIR}/kded/generator.cpp
#         ${CMAKE_SOURCE_DIR}/kded/device.cpp
        ${CMAKE_BINARY_DIR}/configmodule/debug_p.cpp
#         ${CMAKE_SOURCE_DIR}/kded/serializer.cpp
    )

    qt5_add_dbus_interface(test_SRCS
        ${CMAKE_SOURCE_DIR}/kded/org.freedesktop.DBus.Properties.xml
        freedesktop_interface
    )

    add_executable(${testname} ${test_SRCS})
    add_dependencies(${testname} kscreen) # make sure the dbus interfaces are generated
    target_compile_definitions(${testname} PRIVATE "-DTEST_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/\"")
    target_link_libraries(${testname} Qt5::Test Qt5::DBus Qt5::Gui Qt5::Qml Qt5::Quick KF5::CoreAddons KF5::I18n KF5::QuickAddons KF5::Screen)
    add_test(kscreen-kded-${testname} ${testname})
    ecm_mark_as_test(${testname})
endmacro()

add_kded_test(testconfigmodule)
