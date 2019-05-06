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
#include "control.h"
#include "globals.h"

#include <QFile>
#include <QJsonDocument>
#include <QDir>

#include <kscreen/config.h>
#include <kscreen/output.h>

QString Control::s_dirName = QStringLiteral("control/");

QString Control::dirPath()
{
    return Globals::dirPath() % s_dirName;
}

Control::OutputRetention Control::convertVariantToOutputRetention(QVariant variant)
{
    if (variant.canConvert<int>()) {
        const auto retention = variant.toInt();
        if (retention == (int)OutputRetention::Global) {
            return OutputRetention::Global;
        }
        if (retention == (int)OutputRetention::Individual) {
            return OutputRetention::Individual;
        }
    }
    return OutputRetention::Undefined;
}

ControlConfig::ControlConfig(KScreen::ConfigPtr config)
    : m_config(config)
{
//    qDebug() << "Looking for control file:" << config->connectedOutputsHash();
    QFile file(filePath(config->connectedOutputsHash()));
    if (file.open(QIODevice::ReadOnly)) {
        // This might not be reached, bus this is ok. The control file will
        // eventually be created on first write later on.
        QJsonDocument parser;
        m_info = parser.fromJson(file.readAll()).toVariant().toMap();
    }

    // TODO: use a file watcher in case of changes to the control file while
    //       object exists?

    // As global outputs are indexed by a hash of their edid, which is not unique,
    // to be able to tell apart multiple identical outputs, these need special treatment
    {
    QStringList allIds;
    const auto outputs = config->outputs();
    allIds.reserve(outputs.count());
    for (const KScreen::OutputPtr &output : outputs) {
        const auto outputId = output->hash();
        if (allIds.contains(outputId) && !m_duplicateOutputIds.contains(outputId)) {
            m_duplicateOutputIds << outputId;
        }
        allIds << outputId;
    }
    }

    // TODO: this is same in Output::readInOutputs of the daemon. Combine?

    // TODO: connect to outputs added/removed signals and reevaluate duplicate ids
    //       in case of such a change while object exists?
}

QString ControlConfig::filePath(const QString &hash)
{
    const QString dir = dirPath() % QStringLiteral("configs/");
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return dir % hash;
}

QString ControlConfig::filePath()
{
    if (!m_config) {
        return QString();
    }
    return ControlConfig::filePath(m_config->connectedOutputsHash());
}

bool ControlConfig::infoIsOutput(const QVariantMap &info, const QString &outputId, const QString &outputName) const
{
    const QString outputIdInfo = info[QStringLiteral("id")].toString();
    if (outputIdInfo.isEmpty()) {
        return false;
    }
    if (outputId != outputIdInfo) {
        return false;
    }

    if (!outputName.isEmpty() && m_duplicateOutputIds.contains(outputId)) {
        // We may have identical outputs connected, these will have the same id in the config
        // in order to find the right one, also check the output's name (usually the connector)
        const auto metadata = info[QStringLiteral("metadata")].toMap();
        const auto outputNameInfo = metadata[QStringLiteral("name")].toString();
        if (outputName != outputNameInfo) {
            // was a duplicate id, but info not for this output
            return false;
        }
    }
    return true;
}

Control::OutputRetention ControlConfig::getOutputRetention(const KScreen::OutputPtr &output) const
{
    return getOutputRetention(output->hash(), output->name());
}

Control::OutputRetention ControlConfig::getOutputRetention(const QString &outputId, const QString &outputName) const
{
    const QVariantList outputsInfo = getOutputs();
    for (const auto variantInfo : outputsInfo) {
        const QVariantMap info = variantInfo.toMap();
        if (!infoIsOutput(info, outputId, outputName)) {
            continue;
        }
        return convertVariantToOutputRetention(info[QStringLiteral("retention")]);
    }
    // info for output not found
    return OutputRetention::Undefined;
}

static QVariantMap metadata(const QString &outputName)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = outputName;
    return metadata;
}

void ControlConfig::setOutputRetention(const QString &outputId, const QString &outputName, OutputRetention value)
{
    QList<QVariant>::iterator it;
    QVariantList outputsInfo = getOutputs();

    for (it = outputsInfo.begin(); it != outputsInfo.end(); ++it) {
        QVariantMap outputInfo = (*it).toMap();
        if (!infoIsOutput(outputInfo, outputId, outputName)) {
            continue;
        }
        outputInfo[QStringLiteral("retention")] = (int)value;
        *it = outputInfo;
        setOutputs(outputsInfo);
        return;
    }
    // no entry yet, create one
    QVariantMap outputInfo;
    outputInfo[QStringLiteral("id")] = outputId;
    outputInfo[QStringLiteral("metadata")] = metadata(outputName);
    outputInfo[QStringLiteral("retention")] = (int)value;

    outputsInfo << outputInfo;
    setOutputs(outputsInfo);
}

bool ControlConfig::writeFile()
{
    // write updated data to file
    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly)) {
        // TODO: logging category?
//        qCWarning(KSCREEN_COMMON) << "Failed to open config control file for writing! " << file.errorString();
        return false;
    }
    file.write(QJsonDocument::fromVariant(m_info).toJson());
//    qCDebug(KSCREEN_COMMON) << "Config control saved on: " << file.fileName();
    return true;
}

QVariantList ControlConfig::getOutputs() const
{
    return m_info[QStringLiteral("outputs")].toList();
}

void ControlConfig::setOutputs(QVariantList outputsInfo)
{
    m_info[QStringLiteral("outputs")] = outputsInfo;
}

ControlOutput::ControlOutput(KScreen::OutputPtr output)
    : m_output(output)
{
}

QString ControlOutput::filePath(const QString &hash)
{
    const QString dir = dirPath() % QStringLiteral("outputs/");
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return dir % hash;
}

QString ControlOutput::filePath()
{
    if (!m_output) {
        return QString();
    }
    return ControlOutput::filePath(m_output->hash());
}
