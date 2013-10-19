include_directories(${CMAKE_CURRENT_SOURCE_DIR}/declarative
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kcm_kscreen_SRCS
    declarative/iconbutton.cpp
    declarative/qmloutput.cpp
    declarative/qmloutputcomponent.cpp
    declarative/qmlscreen.cpp

    collapsablebutton.cpp
    controlpanel.cpp
    outputconfig.cpp
    profilesmodel.cpp
    resolutionslider.cpp
    unifiedoutputconfig.cpp
    utils.cpp
    widget.cpp
)

set(kscreen_dbus ${CMAKE_SOURCE_DIR}/dbus/org.kde.KScreen.xml)
set_source_files_properties(${kscreen_dbus} PROPERTIES INCLUDE "${CMAKE_SOURCE_DIR}/dbus/types.h" )
qt4_add_dbus_interface(kcm_kscreen_SRCS ${kscreen_dbus} kscreeninterface)
qt4_automoc(${kcm_kscreen_SRCS})

kde4_add_plugin(kcm_kscreen kcm_kscreen.cpp ${kcm_kscreen_SRCS})

target_link_libraries(kcm_kscreen
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${LibKScreen_LIBRARY})

install(TARGETS kcm_kscreen DESTINATION ${PLUGIN_INSTALL_DIR} )


kde4_add_executable(kcm_testapp kcm_testapp.cpp ${kcm_kscreen_SRCS})
set_target_properties(kcm_testapp PROPERTIES COMPILE_FLAGS "-DQT_DECLARATIVE_DEBUG")
target_link_libraries(kcm_testapp
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_PLASMA_LIBS}
        ${LibKScreen_LIBRARY})
