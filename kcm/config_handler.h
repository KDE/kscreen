/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

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
#pragma once

#include "../common/control.h"

#include <kscreen/config.h>

#include <memory>

class OutputModel;

class ConfigHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConfigHandler(QObject *parent = nullptr);
    ~ConfigHandler() override = default;

    void setConfig(KScreen::ConfigPtr config);
    void updateInitialData();

    OutputModel *outputModel() const
    {
        return m_outputs;
    }

    QSize normalizeScreen();

    KScreen::ConfigPtr config() const
    {
        return m_config;
    }

    KScreen::ConfigPtr initialConfig() const
    {
        return m_initialConfig;
    }

    void revertConfig()
    {
        m_config = m_previousConfig->clone();
    }

    int retention() const;
    void setRetention(int retention);

    qreal scale(const KScreen::OutputPtr &output) const;
    void setScale(KScreen::OutputPtr &output, qreal scale);

    KScreen::OutputPtr replicationSource(const KScreen::OutputPtr &output) const;
    void setReplicationSource(KScreen::OutputPtr &output, const KScreen::OutputPtr &source);

    bool autoRotate(const KScreen::OutputPtr &output) const;
    void setAutoRotate(KScreen::OutputPtr &output, bool autoRotate);
    bool autoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const;
    void setAutoRotateOnlyInTabletMode(KScreen::OutputPtr &output, bool value);

    uint32_t overscan(const KScreen::OutputPtr &output) const;
    void setOverscan(const KScreen::OutputPtr &output, uint32_t value);

    KScreen::Output::VrrPolicy vrrPolicy(const KScreen::OutputPtr &output) const;
    void setVrrPolicy(const KScreen::OutputPtr &output, KScreen::Output::VrrPolicy value);

    void writeControl();

    void checkNeedsSave();
    bool shouldTestNewSettings();

Q_SIGNALS:
    void outputModelChanged();
    void changed();
    void screenNormalizationUpdate(bool normalized);
    void needsSaveChecked(bool need);
    void retentionChanged();
    void outputConnect(bool connected);

private:
    void checkScreenNormalization();
    QSize screenSize() const;
    Control::OutputRetention getRetention() const;
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);
    void initOutput(const KScreen::OutputPtr &output);
    void resetScale(const KScreen::OutputPtr &output);
    /**
     * @brief checkSaveandTestCommon - compairs common config changes that would make the config dirty and needed to have the config checked when applied.
     * @param isSaveCheck - True  if your checking to see if the changes should request a save.
     *                      False if you want to check if you should test the config when applied.
     * @return true, if you should check for a save or test the new configuration
     */
    bool checkSaveandTestCommon(bool isSaveCheck);

    KScreen::ConfigPtr m_config = nullptr;
    KScreen::ConfigPtr m_initialConfig;
    KScreen::ConfigPtr m_previousConfig = nullptr;
    OutputModel *m_outputs = nullptr;

    std::unique_ptr<ControlConfig> m_control;
    std::unique_ptr<ControlConfig> m_initialControl;
    Control::OutputRetention m_initialRetention = Control::OutputRetention::Undefined;
    QSize m_lastNormalizedScreenSize;
};
