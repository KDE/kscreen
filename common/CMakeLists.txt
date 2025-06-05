add_library(kscreen_common OBJECT control.cpp globals.cpp osdaction.cpp  output.cpp  utils.cpp)
ecm_qt_declare_logging_category(kscreen_common HEADER kscreen_daemon_debug.h IDENTIFIER KSCREEN_KDED CATEGORY_NAME kscreen.kded DESCRIPTION "kscreen kded (kscreen)" EXPORT KSCREEN)
set_property(TARGET kscreen_common PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(kscreen_common PRIVATE Qt::Core KF6::Screen KF6::CoreAddons KF6::I18n)
target_include_directories(kscreen_common PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")

