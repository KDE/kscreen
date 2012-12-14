add_definitions("-DTEST_DATA=\\\"${CMAKE_CURRENT_SOURCE_DIR}/configs/\\\"")

macro(KSCREEN_TESTS)
    foreach(_testname ${ARGN})
        qt4_generate_moc(${_testname}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.moc)
        include_directories(${QT_INCLUDES} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${X11_Xrandr_INCLUDE_PATH})
        kde4_add_unit_test(${_testname} ${_testname}.cpp ${_testname}.moc ../kded/generator.cpp)
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kscreen)
        add_test(${_testname} ${CMAKE_CURRENT_BINARY_DIR}/${_testname})
    endforeach(_testname)
endmacro(KSCREEN_TESTS)

KSCREEN_TESTS(testgenerator )