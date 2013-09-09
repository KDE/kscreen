set(QML_FILES
    main.qml
    Output.qml
    OutputControls.qml
    OutputIdentifier.qml
    Tip.qml
)

install(FILES ${QML_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_kscreen/qml)
