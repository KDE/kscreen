set(QML_FILES
    main.qml
    Output.qml
    OutputIdentifier.qml
    Tip.qml
)

install(FILES ${QML_FILES} DESTINATION ${KDE_INSTALL_DATADIR}/kcm_kscreen/qml)
