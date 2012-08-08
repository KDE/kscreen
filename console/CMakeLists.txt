include_directories(${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

set(console_SRCS
    main.cpp
)
add_executable(kscreen-console ${console_SRCS})

target_link_libraries(kscreen-console
                      ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kscreen caca cucul)

install(TARGETS kscreen-console ${INSTALL_TARGETS_DEFAULT_ARGS})