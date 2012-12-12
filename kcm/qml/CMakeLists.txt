set(QML_FILES
	main.qml
	IconButton.qml
	ModeSelectionDialog.qml
	OutputView.qml
	Output.qml
	OutputIdentifier.qml
	OutputControls.qml
)

install(FILES ${QML_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_displayconfiguration/qml)