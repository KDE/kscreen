# SPDX-FileCopyrightText: Vlad Zahorodnii <vlad.zahorodnii@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_compile_options(-Wno-missing-designated-field-initializers)
add_compile_options(-Wno-missing-field-initializers)

function(add_verb target)
    foreach(_cmd IN LISTS ARGN)
        get_filename_component(_verb "${_cmd}" NAME_WLE)

        get_filename_component(_object "${_cmd}" DIRECTORY)
        if(_object STREQUAL "command")
            set(_object "")
        else()
            string(REGEX REPLACE "^command/" "" _object "${_object}")
            string(TOUPPER "${_object}" _object)
        endif()

        set(_compile_options "-DVERB_NAME=\\\"${_verb}\\\"")
        if(_object)
            set(_compile_options "${_compile_options} -DOBJECT_NAME=\\\"${_object}\\\"")
        endif()

        set_source_files_properties(${_cmd} PROPERTIES COMPILE_FLAGS "${_compile_options}")
        target_sources(${target} PRIVATE ${_cmd})
    endforeach()
endfunction()

add_executable(kscreenctl
    command.cpp
    main.cpp
    util.cpp
)

add_verb(kscreenctl
    command/identify.cpp
    command/list-outputs.cpp
    command/off.cpp
    command/on.cpp
    command/output/above.cpp
    command/output/add-custom-mode.cpp
    command/output/below.cpp
    command/output/calibrate-hdr.cpp
    command/output/get-name.cpp
    command/output/left-of.cpp
    command/output/list-custom-modes.cpp
    command/output/list-modes.cpp
    command/output/remove-custom-mode.cpp
    command/output/right-of.cpp
    command/output/self.cpp
    command/output/set-abm.cpp
    command/output/set-auto-brightness.cpp
    command/output/set-auto-rotate.cpp
    command/output/set-brightness.cpp
    command/output/set-color-power-tradeoff.cpp
    command/output/set-color-profile-source.cpp
    command/output/set-ddc-ci-allowed.cpp
    command/output/set-dimming.cpp
    command/output/set-edr-policy.cpp
    command/output/set-enabled.cpp
    command/output/set-hdr-and-wcg.cpp
    command/output/set-hdr-color-profile-source.cpp
    command/output/set-hdr-icc-profile.cpp
    command/output/set-hdr.cpp
    command/output/set-icc-profile.cpp
    command/output/set-max-average-brightness-override.cpp
    command/output/set-max-peak-brightness-override.cpp
    command/output/set-maxbpc.cpp
    command/output/set-min-brightness-override.cpp
    command/output/set-mode.cpp
    command/output/set-overscan.cpp
    command/output/set-position.cpp
    command/output/set-primary.cpp
    command/output/set-priority.cpp
    command/output/set-replica-of.cpp
    command/output/set-rgb-range.cpp
    command/output/set-rotation.cpp
    command/output/set-scale.cpp
    command/output/set-sdr-brightness.cpp
    command/output/set-sdr-gamut.cpp
    command/output/set-sharpness.cpp
    command/output/set-vrr-policy.cpp
    command/output/set-wcg.cpp
    command/output/toggle-auto-brightness.cpp
    command/output/toggle-enabled.cpp
    command/output/toggle-hdr-and-wcg.cpp
    command/output/toggle-hdr.cpp
    command/output/toggle-wcg.cpp
)

target_link_libraries(kscreenctl
    PRIVATE
        Qt6::Core
        Qt6::DBus
        KF6::Screen
        KF6::ScreenDpms
)

install(TARGETS kscreenctl ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES shell-completion/fish/kscreenctl.fish DESTINATION ${KDE_INSTALL_DATADIR}/fish/vendor_completions.d/)
install(FILES shell-completion/zsh/_kscreenctl DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
