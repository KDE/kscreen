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

Control::OutputRetention ControlConfig::getOutputRetention(const KScreen::OutputPtr &output) const
{
    return getOutputRetention(output->hash(), output->name());
}

Control::OutputRetention ControlConfig::getOutputRetention(const QString &outputId, const QString &outputName) const
{
    const QVariantList outputsInfo = m_info[QStringLiteral("outputs")].toList();
    for (const auto variantInfo : outputsInfo) {
        const QVariantMap info = variantInfo.toMap();

        const QString outputIdInfo = info[QStringLiteral("id")].toString();
        if (outputIdInfo.isEmpty()) {
            continue;
        }
        if (outputId != outputIdInfo) {
            continue;
        }

        if (!outputName.isEmpty() && m_duplicateOutputIds.contains(outputId)) {
            // We may have identical outputs connected, these will have the same id in the config
            // in order to find the right one, also check the output's name (usually the connector)
            const auto metadata = info[QStringLiteral("metadata")].toMap();
            const auto outputNameInfo = metadata[QStringLiteral("name")].toString();
            if (outputName != outputNameInfo) {
                // was a duplicate id, but info not for this output
                continue;
            }
        }
        return convertVariantToOutputRetention(info[QStringLiteral("retention")]);
    }
    // info for output not found
    return OutputRetention::Undefined;
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
