include_directories(${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

set(console_SRCS
    main.cpp
    console.cpp
)

qt4_automoc(${console_SRCS})
kde4_add_executable(kscreen-console ${console_SRCS})

target_link_libraries(kscreen-console
            ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
            ${LibKScreen_LIBRARY} ${X11_Xrandr_LIB} ${X11_LIBRARIES} ${QJSON_LIBRARIES} qjson)

install(TARGETS kscreen-console ${INSTALL_TARGETS_DEFAULT_ARGS})