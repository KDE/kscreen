ecm_add_tests(osdactions.cpp LINK_LIBRARIES Qt6::Test Qt6::DBus KF6::Screen kscreen_common)
target_compile_definitions(osdactions PRIVATE "-DTEST_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/\"")

add_test(NAME kcm_smoketest COMMAND kcmshell6 --smoke-test kcm_kscreen)
set_tests_properties(kcm_smoketest PROPERTIES
    ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
)
