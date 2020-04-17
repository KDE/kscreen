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
#ifndef KDED_CONFIG_H
#define KDED_CONFIG_H

#include <kscreen/config.h>

#include <QOrientationReading>

#include <memory>

class ControlConfig;

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(KScreen::ConfigPtr config, QObject *parent = nullptr);
    ~Config() = default;

    QString id() const;

    bool fileExists() const;
    std::unique_ptr<Config> readFile();
    std::unique_ptr<Config> readOpenLidFile();
    bool writeFile();
    bool writeOpenLidFile();

    KScreen::ConfigPtr data() const {
        return m_data;
    }

    void activateControlWatching();
    bool autoRotationRequested() const;
    void setDeviceOrientation(QOrientationReading::Orientation orientation);
    bool getAutoRotate() const;
    void setAutoRotate(bool value);
    void log();

    void setValidityFlags(KScreen::Config::ValidityFlags flags) {
        m_validityFlags = flags;
    }

    bool canBeApplied() const;

Q_SIGNALS:
    void controlChanged();

private:
    friend class TestConfig;

    QString filePath();
    std::unique_ptr<Config> readFile(const QString &fileName);
    bool writeFile(const QString &filePath);

    bool canBeApplied(KScreen::ConfigPtr config) const;

    KScreen::ConfigPtr m_data;
    KScreen::Config::ValidityFlags m_validityFlags;
    ControlConfig *m_control;

    static QString s_configsDirName;
    static QString s_fixedConfigFileName;

    static QString configsDirPath();
};

#endif
