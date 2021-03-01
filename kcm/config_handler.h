/*
SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/
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

    void writeControl();

    void checkNeedsSave();

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

    KScreen::ConfigPtr m_config = nullptr;
    KScreen::ConfigPtr m_initialConfig;
    OutputModel *m_outputs = nullptr;

    std::unique_ptr<ControlConfig> m_control;
    std::unique_ptr<ControlConfig> m_initialControl;
    Control::OutputRetention m_initialRetention = Control::OutputRetention::Undefined;
    QSize m_lastNormalizedScreenSize;
};
