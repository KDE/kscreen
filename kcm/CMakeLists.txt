project(kcm_displayconfiguration)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
find_package(KScreen REQUIRED)

include(MacroLibrary)
include(KDE4Defaults)

include_directories(
	${QT_INCLUDES}
	${KDE4_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR})

set(kcm_displayconfiguration_SRCS
  	qmloutputcomponent.cpp
   	qmloutputview.cpp
  	qmloutput.cpp
	displayconfiguration.cpp)

set(QML_FILES
	qml/main.qml
	qml/OutputView.qml
    	qml/Output.qml)

qt4_automoc(${kcm_displayconfiguration_SRCS})


kde4_add_plugin(kcm_displayconfiguration
	${kcm_displayconfiguration_SRCS})

target_link_libraries(kcm_displayconfiguration
	${QT_QTDECLARATIVE_LIBRARY}
	${KDE4_KDEUI_LIBS}
	${KSCREEN_LIBRARY})

install(TARGETS kcm_displayconfiguration DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES displayconfiguration.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES ${QML_FILES} DESTINATION ${DATA_INSTALL_DIR}/kcm_displayconfiguration/qml)