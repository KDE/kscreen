set(kcm_kscreen_SRCS
    kcm_kscreen.cpp

    qmlcursor.cpp
    qmloutputcomponent.cpp
    qmloutput.cpp
    modesproxymodel.cpp
    modeselectionwidget.cpp
    resolutionsortmodel.cpp

    # Copied from kde-runtime/plasma/declarativeimports/plasmaextracomponents
    # as a dependency of qml/plasmacomponents/private/InlineDialog.qml
    fallbackcomponent.cpp
)

qt4_automoc(${kcm_kscreen_SRCS})

kde4_add_plugin(kcm_kscreen ${kcm_kscreen_SRCS})

target_link_libraries(kcm_kscreen
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${LibKScreen_LIBRARY})

install(TARGETS kcm_kscreen DESTINATION ${PLUGIN_INSTALL_DIR} )

kde4_add_executable(kcm_testapp
        kcm_testapp.cpp
        ${kcm_kscreen_SRCS})
set_target_properties(kcm_testapp PROPERTIES COMPILE_FLAGS "-DQT_DECLARATIVE_DEBUG")
target_link_libraries(kcm_testapp
        ${QT_QTDECLARATIVE_LIBRARY}
        ${KDE4_KUTILS_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_PLASMA_LIBS}
        ${LibKScreen_LIBRARY})
