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
#ifndef COMMON_CONTROL_H
#define COMMON_CONTROL_H

#include <kscreen/types.h>

#include <QVariantMap>

class Control
{
public:
    enum class OutputRetention {
        Undefined = -1,
        Global = 0,
        Individual = 1,
    };

    virtual ~Control() = default;

    static OutputRetention getOutputRetention(const QString &outputId, const QMap<QString, OutputRetention> &retentions);

    virtual QString filePath() = 0;

protected:
    static QString dirPath();
    static OutputRetention convertVariantToOutputRetention(QVariant variant);

private:
    static QString s_dirName;
};

class ControlConfig : public Control
{
public:
    ControlConfig(KScreen::ConfigPtr config);

    QMap<QString, OutputRetention> readInOutputRetentionValues();

    QString filePath() override;
    static QString filePath(const QString &hash);

private:
    KScreen::ConfigPtr m_config;
};

class ControlOutput : public Control
{
public:
    ControlOutput(KScreen::OutputPtr output);

    // TODO: scale auto value

    QString filePath() override;
    static QString filePath(const QString &hash);

private:
    KScreen::OutputPtr m_output;
};

#endif
