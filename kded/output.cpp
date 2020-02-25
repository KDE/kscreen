/********************************************************************
Copyright 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "output.h"
#include "config.h"

#include "kscreen_daemon_debug.h"
#include "generator.h"

#include <QStringList>
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QLoggingCategory>
#include <QRect>

#include <kscreen/output.h>
#include <kscreen/edid.h>

QString Output::s_dirName = QStringLiteral("outputs/");

QString Output::dirPath()
{
    return Globals::dirPath() % s_dirName;
}

QString Output::globalFileName(const QString &hash)
{
    const auto dir = dirPath();
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return dir % hash;
}

void Output::readInGlobalPartFromInfo(KScreen::OutputPtr output, const QVariantMap &info)
{
    output->setRotation(static_cast<KScreen::Output::Rotation>(info.value(QStringLiteral("rotation"), 1).toInt()));

    bool scaleOk;
    const qreal scale = info.value(QStringLiteral("scale"), 1.).toDouble(&scaleOk);
    if (scaleOk) {
        output->setScale(scale);
    }

    const QVariantMap modeInfo = info[QStringLiteral("mode")].toMap();
    const QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
    const QSize size = QSize(modeSize[QStringLiteral("width")].toInt(), modeSize[QStringLiteral("height")].toInt());

    qCDebug(KSCREEN_KDED) << "Finding a mode for" << size << "@" << modeInfo[QStringLiteral("refresh")].toFloat();

    KScreen::ModeList modes = output->modes();
    KScreen::ModePtr matchingMode;
    for(const KScreen::ModePtr &mode : modes) {
        if (mode->size() != size) {
            continue;
        }
        if (!qFuzzyCompare(mode->refreshRate(), modeInfo[QStringLiteral("refresh")].toFloat())) {
            continue;
        }

        qCDebug(KSCREEN_KDED) << "\tFound: " << mode->id() << " " << mode->size() << "@" << mode->refreshRate();
        matchingMode = mode;
        break;
    }

    if (!matchingMode) {
        qCWarning(KSCREEN_KDED) << "\tFailed to find a matching mode - this means that our config is corrupted"
                                   "or a different device with the same serial number has been connected (very unlikely)."
                                   "Falling back to preferred modes.";
        matchingMode = output->preferredMode();
    }
    if (!matchingMode) {
        qCWarning(KSCREEN_KDED) << "\tFailed to get a preferred mode, falling back to biggest mode.";
        matchingMode = Generator::biggestMode(modes);
    }
    if (!matchingMode) {
        qCWarning(KSCREEN_KDED) << "\tFailed to get biggest mode. Which means there are no modes. Turning off the screen.";
        output->setEnabled(false);
        return;
    }

    output->setCurrentModeId(matchingMode->id());
}

QVariantMap Output::getGlobalData(KScreen::OutputPtr output)
{
    QFile file(globalFileName(output->hashMd5()));
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KSCREEN_KDED) << "Failed to open file" << file.fileName();
        return QVariantMap();
    }
    QJsonDocument parser;
    return parser.fromJson(file.readAll()).toVariant().toMap();
}

bool Output::readInGlobal(KScreen::OutputPtr output)
{
    const QVariantMap info = getGlobalData(output);
    if (info.empty()) {
        // if info is empty, the global file does not exists, or is in an unreadable state
        return false;
    }
    readInGlobalPartFromInfo(output, info);
    return true;
}

KScreen::Output::Rotation orientationToRotation(QOrientationReading::Orientation orientation,
                                                KScreen::Output::Rotation fallback)
{
    using Orientation = QOrientationReading::Orientation;

    switch (orientation) {
    case Orientation::TopUp:
        return KScreen::Output::Rotation::None;
    case Orientation::TopDown:
        return KScreen::Output::Rotation::Inverted;
    case Orientation::LeftUp:
        return KScreen::Output::Rotation::Right;
    case Orientation::RightUp:
        return KScreen::Output::Rotation::Left;
    case Orientation::Undefined:
    case Orientation::FaceUp:
    case Orientation::FaceDown:
        return fallback;
    default:
        Q_UNREACHABLE();
    }
}

bool Output::updateOrientation(KScreen::OutputPtr &output,
                               QOrientationReading::Orientation orientation)
{
    if (output->type() != KScreen::Output::Type::Panel) {
        return false;
    }
    const auto currentRotation = output->rotation();
    const auto rotation = orientationToRotation(orientation, currentRotation);
    if (rotation == currentRotation) {
        return true;
    }
    output->setRotation(rotation);
    return true;
}

// TODO: move this into the Layouter class.
void Output::adjustPositions(KScreen::ConfigPtr config, const QVariantList &outputsInfo)
{
    typedef QPair<int, QPoint> Out;

    KScreen::OutputList outputs = config->outputs();
    QVector<Out> sortedOutputs; // <id, pos>
    for (const KScreen::OutputPtr &output : outputs) {
        sortedOutputs.append(Out(output->id(), output->pos()));
    }

    // go from left to right, top to bottom
    std::sort(sortedOutputs.begin(), sortedOutputs.end(), [](const Out &o1, const Out &o2) {
        const int x1 = o1.second.x();
        const int x2 = o2.second.x();
        return x1 < x2 || (x1 == x2 && o1.second.y() < o2.second.y());
    });

    for (int cnt = 1; cnt < sortedOutputs.length(); cnt++) {
        auto getOutputInfoProperties = [outputsInfo](KScreen::OutputPtr output, QRect &geo) -> bool {
            if (!output) {
                return false;
            }
            const auto hash = output->hash();

            auto it = std::find_if(outputsInfo.begin(), outputsInfo.end(),
                [hash](QVariant v) {
                    const QVariantMap info = v.toMap();
                    return info[QStringLiteral("id")].toString() == hash;
                }
            );
            if (it == outputsInfo.end()) {
                return false;
            }

            auto isPortrait = [](const QVariant &info) {
                bool ok;
                const int rot = info.toInt(&ok);
                if (!ok) {
                    return false;
                }
                return rot & KScreen::Output::Rotation::Left ||
                        rot & KScreen::Output::Rotation::Right;
            };

            const QVariantMap outputInfo = it->toMap();

            const QVariantMap posInfo = outputInfo[QStringLiteral("pos")].toMap();
            const QVariant scaleInfo = outputInfo[QStringLiteral("scale")];
            const QVariantMap modeInfo = outputInfo[QStringLiteral("mode")].toMap();
            const QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
            const bool portrait = isPortrait(outputInfo[QStringLiteral("rotation")]);

            if (posInfo.isEmpty() || modeSize.isEmpty() || !scaleInfo.canConvert<int>()) {
                return false;
            }

            const qreal scale = scaleInfo.toDouble();
            if (scale <= 0) {
                return false;
            }
            const QPoint pos = QPoint(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
            QSize size = QSize(modeSize[QStringLiteral("width")].toInt() / scale,
                               modeSize[QStringLiteral("height")].toInt() / scale);
            if (portrait) {
                size.transpose();
            }
            geo = QRect(pos, size);

            return true;
        };

        // it's guaranteed that we find the following values in the QMap
        KScreen::OutputPtr prevPtr = outputs.find(sortedOutputs[cnt - 1].first).value();
        KScreen::OutputPtr curPtr = outputs.find(sortedOutputs[cnt].first).value();

        QRect prevInfoGeo, curInfoGeo;
        if (!getOutputInfoProperties(prevPtr, prevInfoGeo) ||
                !getOutputInfoProperties(curPtr, curInfoGeo)) {
            // no info found, nothing can be adjusted for the next output
            continue;
        }

        const QRect prevGeo = prevPtr->geometry();
        const QRect curGeo = curPtr->geometry();

        // the old difference between previous and current output read from the config file
        const int xInfoDiff = curInfoGeo.x() - (prevInfoGeo.x() + prevInfoGeo.width());

        // the proposed new difference
        const int prevRight = prevGeo.x() + prevGeo.width();
        const int xCorrected = prevRight + prevGeo.width() * xInfoDiff / (double)prevInfoGeo.width();
        const int xDiff = curGeo.x() - prevRight;

        // In the following calculate the y-correction. This is more involved since we
        // differentiate between overlapping and non-overlapping pairs and align either
        // top to top/bottom or bottom to top/bottom
        const bool yOverlap = prevInfoGeo.y() + prevInfoGeo.height() > curInfoGeo.y() &&
                prevInfoGeo.y() < curInfoGeo.y() + curInfoGeo.height();

        // these values determine which horizontal edge of previous output we align with
        const int topToTopDiffAbs = qAbs(prevInfoGeo.y() - curInfoGeo.y());
        const int topToBottomDiffAbs = qAbs(prevInfoGeo.y() - curInfoGeo.y() - curInfoGeo.height());
        const int bottomToBottomDiffAbs = qAbs(prevInfoGeo.y() + prevInfoGeo.height() - curInfoGeo.y() - curInfoGeo.height());
        const int bottomToTopDiffAbs = qAbs(prevInfoGeo.y() + prevInfoGeo.height() - curInfoGeo.y());

        const bool yTopAligned = (topToTopDiffAbs < bottomToBottomDiffAbs && topToTopDiffAbs <= bottomToTopDiffAbs) ||
                topToBottomDiffAbs < bottomToBottomDiffAbs;

        int yInfoDiff = curInfoGeo.y() - prevInfoGeo.y();
        int yDiff = curGeo.y() - prevGeo.y();
        int yCorrected;

        if (yTopAligned) {
            // align to previous top
            if (!yOverlap) {
                // align previous top with current bottom
                yInfoDiff += curInfoGeo.height();
                yDiff += curGeo.height();
            }
            // When we align with previous top we are interested in the changes to the
            // current geometry and not in the ones of the previous one.
            const double yInfoRel = yInfoDiff / (double)curInfoGeo.height();
            yCorrected = prevGeo.y() + yInfoRel * curGeo.height();
        } else {
            // align previous bottom...
            yInfoDiff -= prevInfoGeo.height();
            yDiff -= prevGeo.height();
            yCorrected = prevGeo.y() + prevGeo.height();

            if (yOverlap) {
                // ... with current bottom
                yInfoDiff += curInfoGeo.height();
                yDiff += curGeo.height();
                yCorrected -= curGeo.height();
            } // ... else with current top

            // When we align with previous bottom we are interested in changes to the
            // previous geometry.
            const double yInfoRel = yInfoDiff / (double)prevInfoGeo.height();
            yCorrected += yInfoRel * prevGeo.height();
        }

        const int x = xDiff == xInfoDiff ? curGeo.x() : xCorrected;
        const int y = yDiff == yInfoDiff ? curGeo.y() : yCorrected;
        curPtr->setPos(QPoint(x, y));
    }
}

void Output::readIn(KScreen::OutputPtr output, const QVariantMap &info, Control::OutputRetention retention)
{
    const QVariantMap posInfo = info[QStringLiteral("pos")].toMap();
    QPoint point(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
    output->setPos(point);
    output->setPrimary(info[QStringLiteral("primary")].toBool());
    output->setEnabled(info[QStringLiteral("enabled")].toBool());

    if (retention != Control::OutputRetention::Individual && readInGlobal(output)) {
        // output data read from global output file
        return;
    }
    // output data read directly from info
    readInGlobalPartFromInfo(output, info);
}

void Output::readInOutputs(KScreen::ConfigPtr config, const QVariantList &outputsInfo)
{
    KScreen::OutputList outputs = config->outputs();
    ControlConfig control(config);
    // As global outputs are indexed by a hash of their edid, which is not unique,
    // to be able to tell apart multiple identical outputs, these need special treatment
    QStringList duplicateIds;
    {
    QStringList allIds;
    allIds.reserve(outputs.count());
    for (const KScreen::OutputPtr &output : outputs) {
        const auto outputId = output->hash();
        if (allIds.contains(outputId) && !duplicateIds.contains(outputId)) {
            duplicateIds << outputId;
        }
        allIds << outputId;
    }
    allIds.clear();
    }

    for (KScreen::OutputPtr output : outputs) {
        if (!output->isConnected()) {
            output->setEnabled(false);
            continue;
        }
        const auto outputId = output->hash();
        bool infoFound = false;
        for (const auto &variantInfo : outputsInfo) {
            const QVariantMap info = variantInfo.toMap();
            if (outputId != info[QStringLiteral("id")].toString()) {
                continue;
            }
            if (!output->name().isEmpty() && duplicateIds.contains(outputId)) {
                // We may have identical outputs connected, these will have the same id in the config
                // in order to find the right one, also check the output's name (usually the connector)
                const auto metadata = info[QStringLiteral("metadata")].toMap();
                const auto outputName = metadata[QStringLiteral("name")].toString();
                if (output->name() != outputName) {
                    // was a duplicate id, but info not for this output
                    continue;
                }
            }
            infoFound = true;
            readIn(output, info, control.getOutputRetention(output));
            break;
        }
        if (!infoFound) {
            // no info in info for this output, try reading in global output info at least or set some default values

            qCWarning(KSCREEN_KDED) << "\tFailed to find a matching output in the current info data - this means that our info is corrupted"
                                       "or a different device with the same serial number has been connected (very unlikely).";
            if (!readInGlobal(output)) {
                // set some default values instead
                readInGlobalPartFromInfo(output, QVariantMap());
            }
        }
    }

    for (KScreen::OutputPtr output : outputs) {
        auto replicationSource = control.getReplicationSource(output);
        if (replicationSource) {
            output->setPos(replicationSource->pos());
            output->setLogicalSize(replicationSource->logicalSize());
        } else {
            output->setLogicalSize(QSizeF());
        }
    }

    // TODO: this does not work at the moment with logical size replication. Deactivate for now.
    // correct positional config regressions on global output data changes
#if 0
    adjustPositions(config, outputsInfo);
#endif
}

static QVariantMap metadata(const KScreen::OutputPtr &output)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = output->name();
    if (!output->edid() || !output->edid()->isValid()) {
        return metadata;
    }

    metadata[QStringLiteral("fullname")] = output->edid()->deviceId();
    return metadata;
}

bool Output::writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info,
                             const KScreen::OutputPtr &fallback)
{

    info[QStringLiteral("id")] = output->hash();
    info[QStringLiteral("metadata")] = metadata(output);
    info[QStringLiteral("rotation")] = output->rotation();

    // Round scale to four digits
    info[QStringLiteral("scale")] = int(output->scale() * 10000 + 0.5) / 10000.;

    QVariantMap modeInfo;
    float refreshRate = -1.;
    QSize modeSize;
    if (output->currentMode() && output->isEnabled()) {
        refreshRate = output->currentMode()->refreshRate();
        modeSize = output->currentMode()->size();
    } else if (fallback && fallback->currentMode()) {
        refreshRate = fallback->currentMode()->refreshRate();
        modeSize = fallback->currentMode()->size();
    }

    if (refreshRate < 0 || !modeSize.isValid()) {
        return false;
    }

    modeInfo[QStringLiteral("refresh")] = refreshRate;

    QVariantMap modeSizeMap;
    modeSizeMap[QStringLiteral("width")] = modeSize.width();
    modeSizeMap[QStringLiteral("height")] = modeSize.height();
    modeInfo[QStringLiteral("size")] = modeSizeMap;

    info[QStringLiteral("mode")] = modeInfo;

    return true;
}

void Output::writeGlobal(const KScreen::OutputPtr &output)
{
    // get old values and subsequently override
    QVariantMap info = getGlobalData(output);
    if (!writeGlobalPart(output, info, nullptr)) {
        return;
    }

    QFile file(globalFileName(output->hashMd5()));
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(KSCREEN_KDED) << "Failed to open global output file for writing! " << file.errorString();
        return;
    }

    file.write(QJsonDocument::fromVariant(info).toJson());
    return;
}
