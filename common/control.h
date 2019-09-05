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

#include <QObject>
#include <QVariantMap>

class Control
{
    Q_GADGET
public:
    enum class OutputRetention {
        Undefined = -1,
        Global = 0,
        Individual = 1,
    };
    Q_ENUM(OutputRetention)


    virtual ~Control() = default;

    virtual QString filePath() = 0;

protected:
    static QString dirPath();
    static OutputRetention convertVariantToOutputRetention(QVariant variant);

private:
    static QString s_dirName;
};

class ControlConfig : public Control
{
    Q_GADGET
public:
    ControlConfig(KScreen::ConfigPtr config);

    OutputRetention getOutputRetention(const KScreen::OutputPtr &output) const;
    OutputRetention getOutputRetention(const QString &outputId, const QString &outputName) const;
    void setOutputRetention(const KScreen::OutputPtr &output, OutputRetention value);
    void setOutputRetention(const QString &outputId, const QString &outputName, OutputRetention value);

    bool writeFile();

    QString filePath() override;
    static QString filePath(const QString &hash);

private:
    QVariantList getOutputs() const;
    void setOutputs(QVariantList outputsInfo);
    bool infoIsOutput(const QVariantMap &info, const QString &outputId, const QString &outputName) const;

    KScreen::ConfigPtr m_config;
    QVariantMap m_info;
    QStringList m_duplicateOutputIds;
};

class ControlOutput : public Control
{
    Q_GADGET
public:
    ControlOutput(KScreen::OutputPtr output);

    // TODO: scale auto value

    QString filePath() override;
    static QString filePath(const QString &hash);

private:
    KScreen::OutputPtr m_output;
};

#endif
