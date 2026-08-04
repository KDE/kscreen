/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command/output/cmd.h"

#include <expected>

// copied from drm_mode.h.
// There's more flags, but libxcvt doesn't support
// them, so we probably don't need them either
#define DRM_MODE_FLAG_PHSYNC (1 << 0)
#define DRM_MODE_FLAG_NHSYNC (1 << 1)
#define DRM_MODE_FLAG_PVSYNC (1 << 2)
#define DRM_MODE_FLAG_NVSYNC (1 << 3)
#define DRM_MODE_FLAG_INTERLACE (1 << 4)
#define DRM_MODE_FLAG_DBLSCAN (1 << 5)

static bool reducedBlanking = false;
OUTPUT_COMMAND_TOGGLE_OPTION(
    "reduced-blanking",
    &reducedBlanking,
    "Add a custom mode with reduced blanking"
)

static QStringList arguments;
OUTPUT_COMMAND_POSITIONAL_OPTION(
    "MODE;CLOCK HDISPLAY HSYNC_START HSYNC_END HTOTAL VDISPLAY VSYNC_START VSYNC_END VTOTAL FLAG*",
    &arguments
)

static std::expected<KScreen::ModeInfo, std::string> tryBasicModeline()
{
    Q_ASSERT(!arguments.isEmpty());

    const QString modeline = arguments[0];
    const auto failure = [&modeline]() {
        return std::unexpected(std::format("Invalid argument: {}, expecting `width`x`height`@`refresh_rate`", modeline.toStdString()));
    };

    const int xIndex = modeline.indexOf('x');
    if (xIndex == -1) {
        return failure();
    }

    const int atIndex = modeline.indexOf('@', xIndex);
    if (atIndex == -1) {
        return failure();
    }

    const std::optional<int> width = parseInt(modeline.left(xIndex));
    if (!width) {
        return failure();
    }

    const std::optional<int> height = parseInt(modeline.mid(xIndex + 1, atIndex - xIndex - 1));
    if (!height) {
        return failure();
    }

    QString refreshRatePart = modeline.mid(atIndex + 1);
    if (refreshRatePart.endsWith("Hz")) {
        refreshRatePart.chop(2);
    }

    const std::optional<double> refreshRate = parseDouble(refreshRatePart);
    if (!refreshRate) {
        return failure();
    }

    KScreen::ModeInfo::Flags flags = KScreen::ModeInfo::Flag::Custom;
    if (reducedBlanking) {
        flags |= KScreen::ModeInfo::Flag::ReducedBlanking;
    }

    return KScreen::ModeInfo {
        .size = QSize(*width, *height),
        .refreshRate = float(*refreshRate),
        .flags = flags,
    };
}

static uint32_t refreshRateFromMode(const KScreen::Cvt &mode)
{
    uint64_t refreshRate = (mode.clock * 1000000LL / mode.htotal + mode.vtotal / 2) / mode.vtotal;
    if (mode.flags & DRM_MODE_FLAG_INTERLACE) {
        refreshRate *= 2;
    }
    if (mode.flags & DRM_MODE_FLAG_DBLSCAN) {
        refreshRate /= 2;
    }
    if (mode.vscan > 1) {
        refreshRate /= mode.vscan;
    }
    return refreshRate;
}

static std::optional<std::expected<KScreen::ModeInfo, std::string>> tryCvtModeline()
{
    if (arguments.size() < 9) {
        return std::nullopt;
    }

    const auto failure = [](const QString &failedArgument) {
        return std::unexpected(std::format("Failed to parse argument: {}", failedArgument.toStdString()));
    };

    bool ok = false;
    KScreen::Cvt cvt{};
    cvt.clock = std::round(arguments[0].toDouble(&ok) * 1000);
    if (!ok) {
        return failure(arguments[0]);
    }
    cvt.hdisplay = arguments[1].toUInt(&ok);
    if (!ok) {
        return failure(arguments[1]);
    }
    cvt.hsyncStart = arguments[2].toUInt(&ok);
    if (!ok) {
        return failure(arguments[2]);
    }
    cvt.hsyncEnd = arguments[3].toUInt(&ok);
    if (!ok) {
        return failure(arguments[3]);
    }
    cvt.htotal = arguments[4].toUInt(&ok);
    if (!ok) {
        return failure(arguments[4]);
    }
    cvt.hskew = 0;

    cvt.vdisplay = arguments[5].toUInt(&ok);
    if (!ok) {
        return failure(arguments[5]);
    }
    cvt.vsyncStart = arguments[6].toUInt(&ok);
    if (!ok) {
        return failure(arguments[6]);
    }
    cvt.vsyncEnd = arguments[7].toUInt(&ok);
    if (!ok) {
        return failure(arguments[7]);
    }
    cvt.vtotal = arguments[8].toUInt(&ok);
    if (!ok) {
        return failure(arguments[8]);
    }
    cvt.vscan = 1;

    cvt.flags = 0;
    for (int i = 9; i < arguments.size(); i++) {
        if (arguments[i] == "+hsync") {
            cvt.flags |= DRM_MODE_FLAG_PHSYNC;
        } else if (arguments[i] == "-hsync") {
            cvt.flags |= DRM_MODE_FLAG_NHSYNC;
        } else if (arguments[i] == "+vsync") {
            cvt.flags |= DRM_MODE_FLAG_PVSYNC;
        } else if (arguments[i] == "-vsync") {
            cvt.flags |= DRM_MODE_FLAG_NVSYNC;
        } else if (arguments[i] == "interlace") {
            cvt.flags |= DRM_MODE_FLAG_INTERLACE;
        } else {
            return failure(arguments[i]);
        }
    }

    return KScreen::ModeInfo{
        .size = QSize(cvt.hdisplay, cvt.vdisplay),
        .refreshRate = refreshRateFromMode(cvt) / 1000.0f,
        .flags = KScreen::ModeInfo::Flag::Custom,
        .cvt = cvt,
    };
}

template<typename T, typename Func>
static T valueOr(std::optional<T> &&optional, Func &&defaultValue)
{
    if (optional) {
        return optional.value();
    } else {
        return defaultValue();
    }
}

static int run(KScreen::OutputPtr output, KScreen::ConfigPtr config)
{
    const auto modeInfo = valueOr(tryCvtModeline(), tryBasicModeline);
    if (!modeInfo) {
        std::println(std::cerr, "{}", modeInfo.error());
        return 1;
    }

    auto modes = output->customModes();
    if (modes.contains(*modeInfo)) {
        return 0;
    }

    modes.append(*modeInfo);
    output->setCustomModes(modes);

    return applyConfig(config);
}

OUTPUT_COMMAND(run, "Add a custom mode. A custom mode can be specified using either the resolution and the refresh rate or CVT timings. Possible FLAG values: +hsync, -hsync, +vsync, -vsync, interlace", "Modes")

OUTPUT_COMMAND_EXAMPLE("Add a custom mode with reduced blanking",
                       "active-output add-custom-mode --reduced-blanking 1920x1080@60.0Hz")

OUTPUT_COMMAND_EXAMPLE("Add a custom mode specified by CVT timings",
                       "active-output add-custom-mode 712.75 3840 4160 4576 5312 2160 2163 2168 2237 -hsync +vsync")
