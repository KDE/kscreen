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
#include "../common/globals.h"

#include "kscreen_daemon_debug.h"
#include "generator.h"

#include <QStringList>
#include <QFile>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QDir>
#include <QLoggingCategory>

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
    output->setScale(info.value(QStringLiteral("scale"), 1).toInt());

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

void Output::readInOutputs(KScreen::OutputList outputs, const QVariantList &outputsInfo, const QMap<QString, Control::OutputRetention> &retentions)
{
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
        const auto retention = Control::getOutputRetention(outputId, retentions);
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
            readIn(output, info, retention);
            break;
        }
        if (!infoFound) {
            // no info in info for this output, try reading in global output info atleast or set some default values

            qCWarning(KSCREEN_KDED) << "\tFailed to find a matching output in the current info data - this means that our info is corrupted"
                                       "or a different device with the same serial number has been connected (very unlikely).";
            if (!readInGlobal(output)) {
                // set some default values instead
                readInGlobalPartFromInfo(output, QVariantMap());
            }
        }
    }
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

bool Output::writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info)
{
    if (!output->isEnabled()) {
        return false;
    }
    const KScreen::ModePtr mode = output->currentMode();
    if (!mode) {
        qWarning() << "CurrentMode is null" << output->name();
        return false;
    }

    info[QStringLiteral("id")] = output->hash();
    info[QStringLiteral("rotation")] = output->rotation();
    info[QStringLiteral("scale")] = output->scale();
    info[QStringLiteral("metadata")] = metadata(output);

    QVariantMap modeInfo;
    modeInfo[QStringLiteral("refresh")] = mode->refreshRate();

    QVariantMap modeSize;
    modeSize[QStringLiteral("width")] = mode->size().width();
    modeSize[QStringLiteral("height")] = mode->size().height();
    modeInfo[QStringLiteral("size")] = modeSize;

    info[QStringLiteral("mode")] = modeInfo;

    return true;
}

void Output::writeGlobal(const KScreen::OutputPtr &output)
{
    // get old values and subsequently override
    QVariantMap info = getGlobalData(output);
    if (!writeGlobalPart(output, info)) {
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
