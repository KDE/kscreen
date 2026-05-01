add_subdirectory(kded)

add_test(NAME kcm_smoketest COMMAND kcmshell6 --smoke-test kcm_kscreen)
set_tests_properties(kcm_smoketest PROPERTIES
    ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
)
