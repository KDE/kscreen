set(QML_FILES
	main.qml
	IconButton.qml
	OutputView.qml
	Output.qml
	OutputIdentifier.qml
	OutputControls.qml

	OutputView.js
)

install(FILES ${QML_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_displayconfiguration/qml)