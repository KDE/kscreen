# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

# Disable file completions.
complete -c kscreenctl -f

# Suggest available outputs.
function __fish_kde_outputs
    echo active-output
    echo primary-output
    kscreenctl list-outputs --names
end

function __fish_kde_enabled_outputs
    echo none
    echo active-output
    echo primary-output
    kscreenctl list-outputs --names --enabled
end

complete -c kscreenctl -n "__fish_is_nth_token 1" -a "(__fish_kde_outputs)" -d "Output"

# Toplevel commands.
complete -c kscreenctl -n "__fish_is_nth_token 1" -a help -d "Show help information"
complete -c kscreenctl -n "__fish_is_nth_token 1" -a list-outputs -d "List available outputs"
complete -c kscreenctl -n "__fish_is_nth_token 1" -a on -d "Turn on all outputs"
complete -c kscreenctl -n "__fish_is_nth_token 1" -a off -d "Turn off all outputs"
complete -c kscreenctl -n "__fish_is_nth_token 1" -a identify -d "Identify outputs"

# list-outputs
complete -c kscreenctl -n "__fish_seen_subcommand_from list-outputs" -l names -d "Only display connector names"
complete -c kscreenctl -n "__fish_seen_subcommand_from list-outputs" -l enabled -d "Only list enabled outputs"
complete -c kscreenctl -n "__fish_seen_subcommand_from list-outputs" -l disabled -d "Only list disabled outputs"
complete -c kscreenctl -n "__fish_seen_subcommand_from list-outputs" -l json -d "Show output information as JSON"

# output/active-output
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a get-name -d "Get connector name"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-scale -d "Set scale"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-position -d "Set position"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a left-of -d "Place this output to the left of another output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a right-of -d "Place this output to the right of another output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a above -d "Place this output above another output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a below -d "Plasma this output below another output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-enabled -d "Enable or disable the output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a toggle-enabled -d "Toggle the enabled state"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-rotation -d "Change output transform"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-dimming -d "Set dimming, in percents"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-brightness -d "Change brightness, in percents"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-sdr-brightness -d "Change SDR brightness, in nits"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-sdr-gamut -d "Change SDR gamut wideness, in percents"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a add-custom-mode -d "Add custom mode"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a remove-custom-mode -d "Remove custom mode"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a list-custom-modes -d "List custom modes"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-hdr -d "Enable or disable HDR"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a toggle-hdr -d "Toggle HDR"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-wcg -d "Enable or disable wide color gamut"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a toggle-wcg -d "Toggle wide color gamut"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-hdr-and-wcg -d "Enable or disable both HDR and wide color gamut"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a toggle-hdr-and-wcg -d "Toggle both HDR and wide color gamut"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-auto-brightness -d "Enable or disable automatic brightness"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a toggle-auto-brightness -d "Toggle automatic brightness"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-replica-of -d "Mirror or unmirror another output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-icc-profile -d "Set the ICC profile path"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-hdr-icc-profile -d "Set the ICC profile path for the HDR mode"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-color-profile-source -d "Set the color profile source"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-hdr-color-profile-source -d "Set the color profile source for the HDR mode"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-color-power-tradeoff -d "Set the color power tradeoff"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-sharpness -d "Change sharpness, in percents"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-overscan -d "Change overscan, in percents"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-priority -d "Change priority"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-primary -d "Make the output primary"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-vrr-policy -d "Change VRR policy"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-rgb-range -d "Change RGB range"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-edr-policy -d "Change Extended Dynamic Range policy"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-auto-rotate -d "Change auto rotate policy"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-min-brightness-override -d "Change the minimum brightness override"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-max-peak-brightness-override -d "Change the maximum peak brightness override"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-max-average-brightness-override -d "Change the maximum average brightness override"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-ddc-ci-allowed -d "Allow or disallow DDCI CI to be used with this output"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-maxbpc -d "Change maximum bits per color"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-abm -d "Change the adaptive backlight management level"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a list-modes -d "List available modes"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a set-mode -d "Set current mode"
complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -a calibrate-hdr -d "Calibrate HDR"

complete -c kscreenctl -n "__fish_is_nth_token 2 && __fish_seen_subcommand_from (__fish_kde_outputs)" -l json -d "Show output information as JSON"

complete -c kscreenctl -n "__fish_seen_subcommand_from add-custom-mode" -l reduced-blanking -d "Add a custom mode with reduced blanking"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-scale" -a "100% 125% 150% 175% 200% 225% 250% 275% 300%"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-enabled" -a "on off"
complete -c kscreenctl -n "__fish_seen_subcommand_from left-of" -a "(__fish_kde_enabled_outputs)"
complete -c kscreenctl -n "__fish_seen_subcommand_from right-of" -a "(__fish_kde_enabled_outputs)"
complete -c kscreenctl -n "__fish_seen_subcommand_from above" -a "(__fish_kde_enabled_outputs)"
complete -c kscreenctl -n "__fish_seen_subcommand_from below" -a "(__fish_kde_enabled_outputs)"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-rotation" -a "none normal left right inverted flipped 0 90 180 270 flipped90 flipped180 flipped270"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-hdr" -a "on off"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-wcg" -a "on off"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-hdr-and-wcg" -a "on off"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-auto-brightness" -a "on off"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-replica-of" -a "(__fish_kde_enabled_outputs)"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-icc-profile" -F
complete -c kscreenctl -n "__fish_seen_subcommand_from set-hdr-icc-profile" -F
complete -c kscreenctl -n "__fish_seen_subcommand_from set-color-profile-source" -a "sRGB ICC EDID"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-hdr-color-profile-source" -a "ICC EDID"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-color-power-tradeoff" -a "prefer-efficiency prefer-accuracy"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-vrr-policy" -a "never always automatic"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-rgb-range" -a "limited full automatic"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-edr-policy" -a "never always"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-auto-rotate" -a "never in-tablet-mode always"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-ddc-ci-allowed" -a "on off"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-maxbpc" -a "automatic 6 8 10 12 14 16"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-abm" -a "0 1 2 3 4"
complete -c kscreenctl -n "__fish_seen_subcommand_from set-mode" -a "(kscreenctl (__fish_nth_token 1) list-modes)"
