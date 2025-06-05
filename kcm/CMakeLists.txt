add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kscreen\")

kcmutils_add_qml_kcm(kcm_kscreen)

target_sources(kcm_kscreen PRIVATE
    config_handler.cpp config_handler.h
    kcm.cpp kcm.h
    output_model.cpp output_model.h
    ${kwincompositing_SRC}
)

ecm_qt_declare_logging_category(kcm_kscreen
    HEADER
        kcm_screen_debug.h
    IDENTIFIER
        KSCREEN_KCM
    CATEGORY_NAME
        kscreen.kcm
     DESCRIPTION "kscreen kcm (kscreen)" EXPORT KSCREEN
)

kconfig_add_kcfg_files(kcm_kscreen GENERATE_MOC globalscalesettings.kcfgc)
kconfig_add_kcfg_files(kcm_kscreen GENERATE_MOC kwincompositing_setting.kcfgc)
target_link_libraries(kcm_kscreen PRIVATE
    kscreen_common
    Qt::DBus
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    Plasma::PlasmaQuick
    KF6::KCMUtilsQuick
    KF6::Screen
)
