project(kscreen)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
find_package(LibKScreen 0.0.72 REQUIRED)
find_package(QJSON 0.8 REQUIRED)

include(MacroLibrary)
include(KDE4Defaults)

include_directories(
    ${QT_INCLUDES}
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QJSON_INCLUDE_DIR}
    ${LibKScreen_INCLUDE_DIR}
)

add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(tests)
add_subdirectory(console)
