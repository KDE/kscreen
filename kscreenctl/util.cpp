/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "util.h"

#include <KScreen/SetConfigOperation>

#include <QCollator>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRect>

#include <iostream>

using namespace Qt::StringLiterals;

const static QString green = u"\033[01;32m"_s;
const static QString red = u"\033[01;31m"_s;
const static QString yellow = u"\033[01;33m"_s;
const static QString blue = u"\033[01;34m"_s;
const static QString bold = u"\033[01;39m"_s;
const static QString cr = u"\033[0;0m"_s;

KScreen::OutputPtr outputByQuery(const KScreen::ConfigPtr &config, const QString &query)
{
    if (query == "primary-output") {
        return config->primaryOutput();
    }

    QString name = query;
    if (name == "active-output") {
        const QDBusReply<QString> reply = QDBusConnection::sessionBus().call(
            QDBusMessage::createMethodCall(u"org.kde.KWin"_s,
                                           u"/KWin"_s,
                                           u"org.kde.KWin"_s,
                                           u"activeOutputName"_s)
        );
        if (!reply.isValid()) {
            std::println(std::cerr, "Failed to determine active output: {}", reply.error().message().toStdString());
            return nullptr;
        }

        name = reply.value();
    }

    const auto outputs = config->outputs();
    for (const auto &output : outputs) {
        if (output->name() == name || output->uuid() == name) {
            return output;
        }
    }

    bool isInteger;
    const int id = name.toInt(&isInteger);
    if (!isInteger) {
        return nullptr;
    }

    for (const auto &output : outputs) {
        if (output->id() == id) {
            return output;
        }
    }

    return nullptr;
}

KScreen::ModePtr modeByQuery(const KScreen::OutputPtr &output, const QString &query)
{
    const auto availableModes = output->modes();

    for (const auto &mode : availableModes) {
        if (mode->id() == query) {
            return mode;
        }
    }

    for (const auto &mode : availableModes) {
        if (printableMode(output, mode) == query) {
            return mode;
        }
    }

    const int xIndex = query.indexOf('x');
    if (xIndex == -1) {
        return nullptr;
    }

    const int atIndex = query.indexOf('@', xIndex);

    // Try to match the mode by widthxheight.
    if (atIndex == -1) {
        const std::optional<int> width = parseInt(query.left(xIndex));
        if (!width) {
            return nullptr;
        }

        const std::optional<int> height = parseInt(query.mid(xIndex + 1));
        if (!height) {
            return nullptr;
        }

        KScreen::ModePtr mode;
        for (const auto &candidate : availableModes) {
            if (candidate->size() != QSize(*width, *height)) {
                continue;
            }
            if (!mode || candidate->refreshRate() > mode->refreshRate()) {
                mode = candidate;
            }
        }

        return mode;
    }

    // Try to match the mode by widthxheight@refreshRate.
    const std::optional<int> width = parseInt(query.left(xIndex));
    if (!width) {
        return nullptr;
    }

    const std::optional<int> height = parseInt(query.mid(xIndex + 1, atIndex - xIndex - 1));
    if (!height) {
        return nullptr;
    }

    QString refreshRatePart = query.mid(atIndex + 1);
    if (refreshRatePart.endsWith("Hz")) {
        refreshRatePart.chop(2);
    }

    const std::optional<double> refreshRate = parseDouble(refreshRatePart);
    if (!refreshRate) {
        return nullptr;
    }

    KScreen::ModePtr mode;
    for (const auto &candidate : availableModes) {
        if (candidate->size() != QSize(*width, *height)) {
            continue;
        }
        if (std::abs(candidate->refreshRate() - *refreshRate) < 0.01) {
            mode = candidate;
        }
    }

    return mode;
}

QString printableRotation(KScreen::Output::Rotation rotation)
{
    static const QHash<KScreen::Output::Rotation, QString> rotationMap {
        { KScreen::Output::None, u"none"_s },
        { KScreen::Output::None, u"normal"_s },
        { KScreen::Output::Left, u"90"_s },
        { KScreen::Output::Right, u"270"_s },
        { KScreen::Output::Inverted, u"180"_s },
        { KScreen::Output::Flipped, u"flipped"_s },
        { KScreen::Output::Flipped90, u"flipped90"_s },
        { KScreen::Output::Flipped180, u"flipped180"_s },
        { KScreen::Output::Flipped270, u"flipped270"_s },
    };

    return rotationMap.value(rotation);
}

std::optional<KScreen::Output::ColorProfileSource> parseColorProfileSource(QStringView input)
{
    if (input == "EDID") {
        return KScreen::Output::ColorProfileSource::EDID;
    } else if (input == "ICC") {
        return KScreen::Output::ColorProfileSource::ICC;
    } else if (input == "sRGB") {
        return KScreen::Output::ColorProfileSource::sRGB;
    }
    return std::nullopt;
}

std::optional<bool> parseBool(QStringView value)
{
    if (value == "on" || value == "true") {
        return true;
    } else if (value == "off" || value == "false") {
        return false;
    }

    return std::nullopt;
}

std::optional<int> parseInt(QStringView input)
{
    bool ok;
    const int number = input.toInt(&ok);
    if (ok) {
        return number;
    } else {
        return std::nullopt;
    }
}

std::optional<double> parseDouble(QStringView value)
{
    bool ok;
    const double number = value.toDouble(&ok);
    if (ok) {
        return number;
    } else {
        return std::nullopt;
    }
}

std::optional<double> parsePercents(QStringView input)
{
    if (input.endsWith('%')) {
        input.chop(1);
    }

    return parseDouble(input).transform([](double value) {
        return value / 100;
    });
}

QString printableColorProfileSource(KScreen::Output::ColorProfileSource source)
{
    switch (source) {
    case KScreen::Output::ColorProfileSource::EDID:
        return u"EDID"_s;
    case KScreen::Output::ColorProfileSource::ICC:
        return u"ICC"_s;
    case KScreen::Output::ColorProfileSource::sRGB:
        return u"sRGB"_s;
    }

    Q_UNREACHABLE_RETURN(u"sRGB"_s);
}

QString printableColorPowerTradeoff(KScreen::Output::ColorPowerTradeoff source)
{
    switch (source) {
    case KScreen::Output::ColorPowerTradeoff::PreferAccuracy:
        return u"prefer-accuracy"_s;
    case KScreen::Output::ColorPowerTradeoff::PreferEfficiency:
        return u"prefer-efficiency"_s;
    }

    Q_UNREACHABLE_RETURN(u"prefer-efficiency"_s);
}

QString printableVrrPolicy(KScreen::Output::VrrPolicy source)
{
    switch (source) {
    case KScreen::Output::VrrPolicy::Never:
        return u"never"_s;
    case KScreen::Output::VrrPolicy::Always:
        return u"always"_s;
    case KScreen::Output::VrrPolicy::Automatic:
        return u"automatic"_s;
    }

    Q_UNREACHABLE_RETURN(u"automatic"_s);
}

QString printableRgbRange(KScreen::Output::RgbRange source)
{
    switch (source) {
    case KScreen::Output::RgbRange::Automatic:
        return u"automatic"_s;
    case KScreen::Output::RgbRange::Full:
        return u"full"_s;
    case KScreen::Output::RgbRange::Limited:
        return u"limited"_s;
    }

    Q_UNREACHABLE_RETURN(u"automatic"_s);
}

QString printableEdrPolicy(KScreen::Output::EdrPolicy source)
{
    switch (source) {
    case KScreen::Output::EdrPolicy::Never:
        return u"never"_s;
    case KScreen::Output::EdrPolicy::Always:
        return u"always"_s;
    }

    Q_UNREACHABLE_RETURN(u"never"_s);
}

QString printableAutoRotatePolicy(KScreen::Output::AutoRotatePolicy source)
{
    switch (source) {
    case KScreen::Output::AutoRotatePolicy::Never:
        return u"never"_s;
    case KScreen::Output::AutoRotatePolicy::InTabletMode:
        return u"in-tablet-mode"_s;
    case KScreen::Output::AutoRotatePolicy::Always:
        return u"always"_s;
    }

    Q_UNREACHABLE_RETURN(u"in-tablet-mode"_s);
}

QString printableMode(const KScreen::OutputPtr &output, const KScreen::ModePtr &mode)
{
    QString name = u"%1:%2x%3@%4"_s
                    .arg(mode->id(),
                         QString::number(mode->size().width()),
                         QString::number(mode->size().height()),
                         QString::number(mode->refreshRate(), 'f', 2));

    if (mode == output->currentMode()) {
        name += '*'_L1;
    }
    if (mode == output->preferredMode()) {
        name += '!'_L1;
    }

    return name;
}

int applyConfig(KScreen::ConfigPtr config)
{
    auto operation = new KScreen::SetConfigOperation(config);
    if (operation->exec()) {
        return 0;
    } else {
        std::println(std::cerr, "Failed to apply output configuration: {}", operation->errorString().toStdString());
        return 1;
    }
}

void fixupOutputPositions(KScreen::ConfigPtr config)
{
    const auto outputs = config->connectedOutputs();
    if (outputs.isEmpty()) {
        return;
    }

    std::optional<QPoint> topLeft;
    for (const auto &output : outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        if (!topLeft) {
            topLeft = output->pos();
        } else {
            topLeft = QPoint(std::min(topLeft->x(), output->pos().x()),
                             std::min(topLeft->y(), output->pos().y()));
        }
    }

    if (!topLeft) {
        return;
    }

    for (const auto &output : outputs) {
        if (output->isEnabled()) {
            output->setPos(output->pos() - *topLeft);
        }
    }
}

void snapOutputAfterResize(KScreen::OutputPtr resizedOutput, const QSize &oldSize, KScreen::ConfigPtr config)
{
    const QPoint oldCenter(resizedOutput->pos().x() + oldSize.width() / 2,
                           resizedOutput->pos().y() + oldSize.height() / 2);

    const QPoint offset(resizedOutput->explicitLogicalSizeInt().width() - oldSize.width(),
                        resizedOutput->explicitLogicalSizeInt().height() - oldSize.height());

    const auto outputs = config->connectedOutputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        if (output->id() == resizedOutput->id()) {
            continue;
        }

        QPoint translation;
        if (output->pos().x() >= oldCenter.x()) {
            translation.setX(offset.x());
        }
        if (output->pos().y() >= oldCenter.y()) {
            translation.setY(offset.y());
        }

        if (!translation.isNull()) {
            output->setPos(output->pos() + translation);
        }
    }

    fixupOutputPositions(config);
}

void showOutputAsText(const KScreen::OutputPtr &output)
{
    QHash<KScreen::Output::Type, QString> typeString;
    typeString[KScreen::Output::Unknown] = u"Unknown"_s;
    typeString[KScreen::Output::VGA] = u"VGA"_s;
    typeString[KScreen::Output::DVI] = u"DVI"_s;
    typeString[KScreen::Output::DVII] = u"DVII"_s;
    typeString[KScreen::Output::DVIA] = u"DVIA"_s;
    typeString[KScreen::Output::DVID] = u"DVID"_s;
    typeString[KScreen::Output::HDMI] = u"HDMI"_s;
    typeString[KScreen::Output::Panel] = u"Panel"_s;
    typeString[KScreen::Output::TV] = u"TV"_s;
    typeString[KScreen::Output::TVComposite] = u"TVComposite"_s;
    typeString[KScreen::Output::TVSVideo] = u"TVSVideo"_s;
    typeString[KScreen::Output::TVComponent] = u"TVComponent"_s;
    typeString[KScreen::Output::TVSCART] = u"TVSCART"_s;
    typeString[KScreen::Output::TVC4] = u"TVC4"_s;
    typeString[KScreen::Output::DisplayPort] = u"DisplayPort"_s;

    QCollator collator;
    collator.setNumericMode(true);

    static QTextStream cout(stdout);
    static QTextStream cerr(stderr);

    cout << green << "Output: " << cr << output->id() << " " << output->name() << " " << output->uuid() << Qt::endl;
    cout << "\t" << (output->isEnabled() ? green + u"enabled"_s : red + u"disabled"_s) << cr << Qt::endl;
    cout << "\t" << (output->isConnected() ? green + u"connected"_s : red + u"disconnected"_s) << cr << Qt::endl;
    cout << "\t" << (output->isEnabled() ? green : red) + u"priority "_s << output->priority() << cr << Qt::endl;
    auto _type = typeString[output->type()];
    cout << "\t" << yellow << (_type.isEmpty() ? u"UnmappedOutputType"_s : _type) << cr << Qt::endl;
    cout << "\t" << yellow << "replication source:" << cr << output->replicationSource() << Qt::endl;
    cout << "\t" << blue << "Modes: " << cr;

    const auto modes = output->modes();
    auto modeKeys = modes.keys();
    std::sort(modeKeys.begin(), modeKeys.end(), collator);

    for (const auto &key : modeKeys) {
        auto mode = *modes.find(key);

        auto name = u"%1x%2@%3"_s
                        .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(mode->refreshRate(), 'f', 2));
        if (mode == output->currentMode()) {
            name = green + name + '*'_L1 + cr;
        }
        if (mode == output->preferredMode()) {
            name = name + '!'_L1;
        }
        cout << " " << mode->id() << ":" << name << " ";
    }
    cout << Qt::endl;

    cout << yellow << "\tCustom modes:" << cr;
    const auto customModes = output->customModes();
    if (customModes.empty()) {
        cout << " None" << Qt::endl;
    } else {
        cout << Qt::endl;
        for (const auto &[info, index] : std::views::zip(customModes, std::views::iota(0))) {
            cout << "\t\t" << index << ": ";
            cout << std::format("{}x{}@{:.2f}", info.size.width(), info.size.height(), info.refreshRate).c_str();
            if (info.flags & KScreen::ModeInfo::Flag::ReducedBlanking) {
                cout << " (reduced blanking)";
            }
            cout << Qt::endl;
        }
    }

    const auto g = output->geometry();
    cout << yellow << "\tGeometry: " << cr << g.x() << "," << g.y() << " " << g.width() << "x" << g.height() << Qt::endl;
    cout << yellow << "\tScale: " << cr << output->scale() << Qt::endl;
    cout << yellow << "\tRotation: " << cr << output->rotation() << Qt::endl;
    cout << yellow << "\tOverscan: " << cr << output->overscan() << Qt::endl;
    cout << yellow << "\tVrr: ";
    if (output->capabilities() & KScreen::Output::Capability::Vrr) {
        switch (output->vrrPolicy()) {
        case KScreen::Output::VrrPolicy::Never:
            cout << cr << "Never" << Qt::endl;
            break;
        case KScreen::Output::VrrPolicy::Automatic:
            cout << cr << "Automatic" << Qt::endl;
            break;
        case KScreen::Output::VrrPolicy::Always:
            cout << cr << "Always" << Qt::endl;
        }
    } else {
        cout << cr << "incapable" << Qt::endl;
    }
    cout << yellow << "\tRgbRange: ";
    if (output->capabilities() & KScreen::Output::Capability::RgbRange) {
        switch (output->rgbRange()) {
        case KScreen::Output::RgbRange::Automatic:
            cout << cr << "Automatic" << Qt::endl;
            break;
        case KScreen::Output::RgbRange::Full:
            cout << cr << "Full" << Qt::endl;
            break;
        case KScreen::Output::RgbRange::Limited:
            cout << cr << "Limited" << Qt::endl;
        }
    } else {
        cout << cr << "unknown" << Qt::endl;
    }
    cout << yellow << "\tHDR: ";
    if (output->capabilities() & KScreen::Output::Capability::HighDynamicRange) {
        if (output->isHdrEnabled()) {
            cout << cr << "enabled" << Qt::endl;
            cout << yellow << "\t\tSDR brightness: " << cr << output->sdrBrightness() << " nits" << Qt::endl;
            cout << yellow << "\t\tSDR gamut wideness: " << cr << std::round(output->sdrGamutWideness() * 100) << "%" << Qt::endl;
            if (output->maxPeakBrightness() == 0) {
                cout << yellow << "\t\tPeak brightness: " << cr << "unknown";
            } else {
                cout << yellow << "\t\tPeak brightness: " << cr << output->maxPeakBrightness() << " nits";
            }
            if (const auto used = output->maxPeakBrightnessOverride()) {
                cout << yellow << ", overridden with: " << cr << *used << " nits";
            }
            cout << Qt::endl;
            if (output->maxAverageBrightness() == 0) {
                cout << yellow << "\t\tMax average brightness: " << cr << "unknown";
            } else {
                cout << yellow << "\t\tMax average brightness: " << cr << output->maxAverageBrightness() << " nits";
            }
            if (const auto used = output->maxAverageBrightnessOverride()) {
                cout << yellow << ", overridden with: " << cr << *used << " nits";
            }
            cout << Qt::endl;
            cout << yellow << "\t\tMin brightness: " << cr << output->minBrightness() << " nits";
            if (const auto used = output->minBrightnessOverride()) {
                cout << yellow << ", overridden with: " << cr << (*used) / 10'000.0 << " nits";
            }
            cout << Qt::endl;

            cout << yellow << "\t\tHDR color profile source: ";
            if (output->capabilities() & KScreen::Output::Capability::HdrIccProfile) {
                cout << cr;
                switch (output->hdrColorProfileSource()) {
                case KScreen::Output::ColorProfileSource::sRGB:
                    cout << "sRGB";
                    break;
                case KScreen::Output::ColorProfileSource::ICC:
                    cout << "ICC";
                    break;
                case KScreen::Output::ColorProfileSource::EDID:
                    cout << "EDID";
                    break;
                }
                cout << Qt::endl;

                cout << yellow << "\t\tHDR ICC profile: ";
                if (!output->hdrIccProfilePath().isEmpty()) {
                    cout << cr << output->hdrIccProfilePath() << Qt::endl;
                } else {
                    cout << cr << "none" << Qt::endl;
                }
            } else {
                cout << cr << "incapable" << Qt::endl;
            }
        } else {
            cout << cr << "disabled" << Qt::endl;
        }
    } else {
        cout << cr << "incapable" << Qt::endl;
    }
    cout << yellow << "\tWide Color Gamut: ";
    if (output->capabilities() & KScreen::Output::Capability::WideColorGamut) {
        if (output->isWcgEnabled()) {
            cout << cr << "enabled" << Qt::endl;
        } else {
            cout << cr << "disabled" << Qt::endl;
        }
    } else {
        cout << cr << "incapable" << Qt::endl;
    }
    cout << yellow << "\tICC profile: ";
    if (output->capabilities() & KScreen::Output::Capability::IccProfile) {
        if (!output->iccProfilePath().isEmpty()) {
            cout << cr << output->iccProfilePath() << Qt::endl;
        } else {
            cout << cr << "none" << Qt::endl;
        }
    } else {
        cout << cr << "incapable" << Qt::endl;
    }
    cout << yellow << "\tColor profile source: ";
    if (output->capabilities() & (KScreen::Output::Capabilities(KScreen::Output::Capability::IccProfile) | KScreen::Output::Capability::BuiltInColorProfile)) {
        cout << cr;
        switch (output->colorProfileSource()) {
        case KScreen::Output::ColorProfileSource::sRGB:
            cout << "sRGB";
            break;
        case KScreen::Output::ColorProfileSource::ICC:
            cout << "ICC";
            break;
        case KScreen::Output::ColorProfileSource::EDID:
            cout << "EDID";
            break;
        }
        cout << Qt::endl << yellow << "\tColor power preference: " << cr;
        switch (output->colorPowerPreference()) {
        case KScreen::Output::ColorPowerTradeoff::PreferEfficiency:
            cout << "prefer efficiency and performance";
            break;
        case KScreen::Output::ColorPowerTradeoff::PreferAccuracy:
            cout << "prefer accuracy";
            break;
        }
        cout << Qt::endl;
    } else {
        cout << cr << "incapable" << Qt::endl;
    }
    cout << yellow << "\tBrightness control: ";
    if (output->capabilities() & KScreen::Output::Capability::BrightnessControl) {
        cout << cr << "supported, set to " << std::round(output->brightness() * 100) << "%"
             << " and dimming to " << std::round(output->dimming() * 100) << "%" << Qt::endl;
    } else {
        cout << cr << "unsupported" << Qt::endl;
    }
    if (output->capabilities() & KScreen::Output::Capability::DdcCi) {
        cout << yellow << "\tDDC/CI: ";
        cout << cr << (output->ddcCiAllowed() ? "allowed" : "disallowed") << Qt::endl;
    }
    cout << yellow << "\tColor resolution: ";
    if (output->capabilities() & KScreen::Output::Capability::MaxBitsPerColor) {
        cout << cr;
        if (output->maxBitsPerColor() != 0) {
            cout << output->maxBitsPerColor() << " bits per color";
        } else {
            uint32_t autoBpc = 0;
            if (output->colorPowerPreference() == KScreen::Output::ColorPowerTradeoff::PreferEfficiency) {
                autoBpc = 10;
            } else {
                autoBpc = 16;
            }
            if (output->automaticMaxBitsPerColorLimit()) {
                autoBpc = std::min(autoBpc, output->automaticMaxBitsPerColorLimit());
            }
            cout << "automatic (" << autoBpc << ")";
        }
        cout << ", range: [" << output->bitsPerColorRange().min << "; " << output->bitsPerColorRange().max << "] bits per color" << Qt::endl;
    } else {
        cout << "unknown" << Qt::endl;
    }
    cout << yellow << "\tAllow EDR: ";
    if (output->capabilities() & KScreen::Output::Capability::ExtendedDynamicRange) {
        cout << cr;
        switch (output->edrPolicy()) {
        case KScreen::Output::EdrPolicy::Never:
            cout << "never";
            break;
        case KScreen::Output::EdrPolicy::Always:
            cout << "always";
            break;
        }
        cout << Qt::endl;
    } else {
        cout << cr << "unsupported" << Qt::endl;
    }
    cout << yellow << "\tSharpness control: ";
    if (output->capabilities() & KScreen::Output::Capability::SharpnessControl) {
        cout << cr << "supported, set to " << std::round(output->sharpness() * 100) << "%"
             << Qt::endl;
    } else {
        cout << cr << "unsupported" << Qt::endl;
    }
    cout << yellow << "\tAutomatic brightness: ";
    if (output->capabilities() & KScreen::Output::Capability::AutomaticBrightness) {
        cout << cr << "supported, " << (output->automaticBrightness() ? "enabled" : "disabled") << Qt::endl;
    } else {
        cout << cr << "unsupported" << Qt::endl;
    }
    cout << yellow << "\tAuto Rotate Policy: ";
    if (output->capabilities() & KScreen::Output::Capability::AutoRotation) {
        switch (output->autoRotatePolicy()) {
        case KScreen::Output::AutoRotatePolicy::Never:
            cout << cr << "never" << Qt::endl;
            break;
        case KScreen::Output::AutoRotatePolicy::InTabletMode:
            cout << cr << "inTabletMode" << Qt::endl;
            break;
        case KScreen::Output::AutoRotatePolicy::Always:
            cout << cr << "always" << Qt::endl;
        }
    } else {
        cout << cr << "incapable" << Qt::endl;
    }

    cout << yellow << "\tAdaptive backlight modulation: ";
    if (output->capabilities() & KScreen::Output::Capability::AbmLevel) {
        cout << cr << "supported, set to " << output->abmLevel() << Qt::endl;
    } else {
        cout << cr << "unsupported" << Qt::endl;
    }
}

QJsonObject outputAsJson(const KScreen::OutputPtr &output)
{
    const auto jsonMode = [&output](const KScreen::ModePtr &mode) -> QJsonValue {
        return QJsonObject {
            { u"id"_s, mode->id(), },
            {
                u"resolution"_s, QJsonObject {
                    { u"width"_s, mode->size().width() },
                    { u"height"_s, mode->size().height() },
                }
            },
            { u"refreshRate"_s, mode->refreshRate() },
            { u"current"_s, output->currentMode() == mode },
            { u"preferred"_s, output->preferredMode() == mode },
        };
    };

    QJsonObject status = {
        { u"name"_s, output->name() },
        { u"uuid"_s, output->uuid() },
        { u"enabled"_s, output->isEnabled() },
        { u"connected"_s, output->isConnected() },
        { u"scale"_s, output->scale() },
        {
            u"position"_s, QJsonObject {
                { u"x"_s, output->pos().x() },
                { u"y"_s, output->pos().y() },
            }
        },
        { u"rotation"_s, printableRotation(output->rotation()) },
        { u"dimming"_s, output->dimming() },
        { u"brightness"_s, output->brightness() },
        { u"sdrBrightness"_s, int(output->sdrBrightness()) },
        { u"sdrGamut"_s, output->sdrGamutWideness() },
        { u"hdr"_s, output->isHdrEnabled() },
        { u"wcg"_s, output->isWcgEnabled() },
        { u"iccProfilePath"_s, output->iccProfilePath() },
        { u"hdrIccProfilePath"_s, output->hdrIccProfilePath() },
        { u"colorProfileSource"_s, printableColorProfileSource(output->colorProfileSource()) },
        { u"hdrColorProfileSource"_s, printableColorProfileSource(output->hdrColorProfileSource()) },
        { u"colorPowerTradeoff"_s, printableColorPowerTradeoff(output->colorPowerPreference()) },
        { u"autoBrightness"_s, output->automaticBrightness() },
        { u"replicaOf"_s, output->replicationSource() },
        { u"sharpness"_s, std::round(100 * output->sharpness()) },
        { u"overscan"_s, int(output->overscan()) },
        { u"priority"_s, int(output->priority()) },
        { u"primary"_s, output->priority() == 1 },
        { u"vrrPolicy"_s, printableVrrPolicy(output->vrrPolicy()) },
        { u"rgbRange"_s, printableRgbRange(output->rgbRange()) },
        { u"autoRotate"_s, printableAutoRotatePolicy(output->autoRotatePolicy()) },
        { u"ddcCiAllowed"_s, output->ddcCiAllowed() },
        { u"maxbpc"_s, int(output->maxBitsPerColor()) },
        { u"abm"_s, int(output->abmLevel()) },
        { u"mode"_s, jsonMode(output->currentMode()) },
        {
            u"modes"_s,
            [&output, &jsonMode]() -> QJsonValue {
                QJsonArray ret;
                for (const auto modes = output->modes(); const auto &mode : modes) {
                    ret.append(jsonMode(mode));
                }
                return ret;
            }()
        },
        {
            u"customModes"_s,
            [&output]() -> QJsonValue {
                QJsonArray ret;
                for (const auto modes = output->customModes(); const auto &mode : modes) {
                    ret.append(QJsonObject {
                        {
                            u"resolution"_s, QJsonObject {
                                { u"width"_s, mode.size.width() },
                                { u"height"_s, mode.size.height() },
                            }
                        },
                        { u"refreshRate"_s, mode.refreshRate },
                    });
                }
                return ret;
            }()
        },
        { u"edrPolicy"_s, printableEdrPolicy(output->edrPolicy()) },
        {
            u"capabilities"_s,
            [&output]() -> QJsonValue {
                const struct {
                    KScreen::Output::Capability capability;
                    QString name;
                } mappings[] = {
                    { KScreen::Output::Capability::Overscan, u"overscan"_s },
                    { KScreen::Output::Capability::Vrr, u"vrr"_s },
                    { KScreen::Output::Capability::RgbRange, u"rgb-range"_s },
                    { KScreen::Output::Capability::HighDynamicRange, u"hdr"_s },
                    { KScreen::Output::Capability::WideColorGamut, u"wcg"_s },
                    { KScreen::Output::Capability::AutoRotation, u"auto-rotate"_s },
                    { KScreen::Output::Capability::IccProfile, u"icc"_s },
                    { KScreen::Output::Capability::BrightnessControl, u"brightness"_s },
                    { KScreen::Output::Capability::BuiltInColorProfile, u"builtin-color-profile"_s },
                    { KScreen::Output::Capability::DdcCi, u"ddc-ci"_s },
                    { KScreen::Output::Capability::MaxBitsPerColor, u"maxbpc"_s },
                    { KScreen::Output::Capability::ExtendedDynamicRange, u"edr"_s },
                    { KScreen::Output::Capability::SharpnessControl, u"sharpness"_s },
                    { KScreen::Output::Capability::CustomModes, u"custom-modes"_s },
                    { KScreen::Output::Capability::AutomaticBrightness, u"auto-brightness"_s },
                    { KScreen::Output::Capability::HdrIccProfile, u"hdr-icc"_s },
                    { KScreen::Output::Capability::AbmLevel, u"abm"_s },
                };

                const auto capabilities = output->capabilities();
                QJsonArray ret;
                for (const auto &mapping : mappings) {
                    if (capabilities & mapping.capability) {
                        ret.append(mapping.name);
                    }
                }

                return ret;
            }()
        }
    };

    if (const auto value = output->minBrightnessOverride()) {
        status.insert(u"minBrightnessOverride"_s, *value);
    }

    if (const auto value = output->maxPeakBrightnessOverride()) {
        status.insert(u"maxPeakBrightnessOverride"_s, *value);
    }

    if (const auto value = output->maxAverageBrightnessOverride()) {
        status.insert(u"maxAverageBrightnessOverride"_s, *value);
    }

    return status;
}

void showOutputAsJson(const KScreen::OutputPtr &output)
{
    std::print(std::cout, "{}", QJsonDocument(outputAsJson(output)).toJson().toStdString());
}
