set(QML_FILES
    main.qml
    Output.qml
    OutputControls.qml
    OutputIdentifier.qml
    Tip.qml
)

set(QML_PLASMACOMPONENTS_FILES
    plasmacomponents/Dialog.qml
)

set(QML_PLASMACOMPONENTS_PRIVATE_FILES
    plasmacomponents/private/InlineDialog.qml
    plasmacomponents/private/AppManager.js
)

install(FILES ${QML_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_kscreen/qml)
install(FILES ${QML_PLASMACOMPONENTS_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_kscreen/qml/plasmacomponents)
install(FILES ${QML_PLASMACOMPONENTS_PRIVATE_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_kscreen/qml/plasmacomponents/private)
