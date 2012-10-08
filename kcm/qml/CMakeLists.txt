set(QML_FILES
	main.qml
	IconButton.qml
	ModeListView.qml
	OutputView.qml
	Output.qml
	OutputIdentifier.qml
	OutputControls.qml
	VirtualScreen.qml
)

install(FILES ${QML_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_displayconfiguration/qml)