include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_BINARY_DIR}
)

macro(ADD_KDED_TEST testname)
    set(test_SRCS
        ${testname}.cpp
        ${CMAKE_SOURCE_DIR}/kded/generator.cpp
        ${CMAKE_SOURCE_DIR}/kded/device.cpp
        ${CMAKE_SOURCE_DIR}/kded/serializer.cpp
        #${CMAKE_SOURCE_DIR}/kded/daemon.cpp
    )
    ecm_qt_declare_logging_category(test_SRCS HEADER kscreen_daemon_debug.h IDENTIFIER KSCREEN_KDED CATEGORY_NAME kscreen.kded)

    qt5_add_dbus_interface(test_SRCS
        ${CMAKE_SOURCE_DIR}/kded/org.freedesktop.DBus.Properties.xml
        freedesktop_interface
    )

    add_executable(${testname} ${test_SRCS})
    add_dependencies(${testname} kscreen) # make sure the dbus interfaces are generated
    target_compile_definitions(${testname} PRIVATE "-DTEST_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/\"")
    target_link_libraries(${testname} Qt5::Test Qt5::DBus Qt5::Gui KF5::Screen)
    add_test(kscreen-kded-${testname} ${testname})
    ecm_mark_as_test(${testname})
endmacro()

add_kded_test(testgenerator)
add_kded_test(serializertest)
#add_kded_test(testdaemon)
