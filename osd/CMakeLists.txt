include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

QT5_ADD_RESOURCES(qtsourceview_RC_SRCS images/image.qrc)

add_executable(kscreen-osd 
    main.cpp 
    osdwidget.cpp
    qtsingleapplication/qtsingleapplication.cpp
    qtsingleapplication/qtlocalpeer.cpp
    ${qtsourceview_RC_SRCS})

target_link_libraries(kscreen-osd
            Qt5::Core
            Qt5::Widgets
            Qt5::Network
            KF5::I18n
            KF5::Screen
            KF5::Service)

install(TARGETS kscreen-osd ${INSTALL_TARGETS_DEFAULT_ARGS})
