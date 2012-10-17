#  KScreen_FOUND - system has KScreen
#  KScreen_INCLUDE_DIRS - the KScreen include directories
#  KScreen_LIBRARIES - link these to use KScreen

find_package(PkgConfig)
pkg_check_modules(PC_LIBKSCREEN QUIET kscreen)
set(KSCREEN_DEFINITIONS ${PC_KSCREEN_CFLAGS_OTHER})

find_path(KSCREEN_INCLUDE_DIR kscreen/provider.h
          HINTS ${PC_KSCREEN_INCLUDEDIR} ${PC_KSCREEN_INCLUDE_DIRS}
          PATH_SUFFIXES kscreen )

find_library(KSCREEN_LIBRARY NAMES kscreen libkscreen
             HINTS ${PC_KSCREEN_LIBDIR} ${PC_KSCREEN_LIBRARY_DIRS} )

set(KSCREEN_LIBRARIES ${KSCREEN_LIBRARY} )
set(KSCREEN_INCLUDE_DIRS ${KSCREEN_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(KScreen DEFAULT_MSG
                                  KSCREEN_LIBRARY KSCREEN_INCLUDE_DIR)

mark_as_advanced(KSCREEN_INCLUDE_DIR KSCREEN_LIBRARY )
