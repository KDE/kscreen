/*
    SPDX-FileCopyrightText: 2025 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>
#include <QQmlApplicationEngine>
#include <QTimer>

class HdrCalibrator : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString outputName READ outputName CONSTANT);
    Q_PROPERTY(qreal peakBrightness READ peakBrightness CONSTANT);
    Q_PROPERTY(qreal peakBrightnessOverride READ peakBrightnessOverride WRITE setPeakBrightnessOverride NOTIFY peakBrightnessOverrideChanged);
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged);
    Q_PROPERTY(qreal sdrBrightness READ sdrBrightness WRITE setSdrBrightness NOTIFY sdrBrightnessChanged);

public:
    explicit HdrCalibrator();

    Q_INVOKABLE void applyConfig();
    // ensures that pending operations are finished before quitting
    Q_INVOKABLE void safeQuit();

    QString outputName() const;
    qreal peakBrightness() const;

    qreal peakBrightnessOverride() const;
    void setPeakBrightnessOverride(qreal override);

    qreal brightness() const;
    void setBrightness(qreal brightness);

    qreal sdrBrightness() const;
    void setSdrBrightness(qreal brightness);

    void setOutput(const KScreen::ConfigPtr &config, const KScreen::OutputPtr &output);

Q_SIGNALS:
    void peakBrightnessOverrideChanged();
    void brightnessChanged();
    void sdrBrightnessChanged();

private:
    void setOpFinished();

    KScreen::SetConfigOperation *m_setOp = nullptr;
    bool m_configDirty = false;
    bool m_quitRequested = false;
    KScreen::OutputPtr m_output;
    KScreen::ConfigPtr m_config;
    QTimer m_compressor;
};
