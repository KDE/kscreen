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

QString Control::s_dirName = QStringLiteral("control/");

QString Control::dirPath()
{
    return Globals::dirPath() % s_dirName;
}

QString Control::outputFilePath(const QString &hash)
{
    const QString dir = dirPath() % QStringLiteral("outputs/");
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return dir % hash;
}

QString Control::configFilePath(const QString &hash)
{
    const QString dir = dirPath() % QStringLiteral("configs/");
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return dir % hash;
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

QMap<QString, Control::OutputRetention> Control::readInOutputRetentionValues(const QString &configId)
{
//    qDebug() << "Looking for control file:" << configId;
    QFile file(configFilePath(configId));
    if (!file.open(QIODevice::ReadOnly)) {
        // TODO: have a logging category
//        qCDebug(KSCREEN_COMMON) << "Failed to open file" << file.fileName();
        return QMap<QString, Control::OutputRetention>();
    }

    QJsonDocument parser;
    const QVariantMap controlInfo = parser.fromJson(file.readAll()).toVariant().toMap();
    const QVariantList outputsInfo = controlInfo[QStringLiteral("outputs")].toList();
    QMap<QString, Control::OutputRetention> retentions;

    for (const auto variantInfo : outputsInfo) {
        const QVariantMap info = variantInfo.toMap();

        // TODO: this does not yet consider the output name (i.e. connector). Necessary?
        const QString outputHash = info[QStringLiteral("id")].toString();
        if (outputHash.isEmpty()) {
            continue;
        }
        retentions[outputHash] = convertVariantToOutputRetention(info[QStringLiteral("retention")]);
    }

    return retentions;
}

Control::OutputRetention Control::getOutputRetention(const QString &outputId, const QMap<QString, Control::OutputRetention> &retentions)
{
    if (retentions.contains(outputId)) {
        return retentions[outputId];
    }
    // info for output not found
    return OutputRetention::Undefined;
}
