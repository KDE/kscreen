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
	${CMAKE_CURRENT_BINARY_DIR}
	${KSCREEN_INCLUDE_DIR}
)

install(FILES displayconfiguration.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

add_subdirectory(src)
add_subdirectory(qml)
