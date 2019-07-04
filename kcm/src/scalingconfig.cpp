/*
 * Copyright (C) 2015 David Edmundson <davidedmundson@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "scalingconfig.h"

#include <QProcess>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KScreen/Output>

//we want a scale between 1 and 3.0 in intervals of 0.1
//slider can only handle ints so goes 10-30
#define SLIDER_RATIO 10.0

ScalingConfig::ScalingConfig(const KScreen::OutputList &outputList, QWidget* parent):
    QDialog(parent),
    m_outputList(outputList)
{

    ui.setupUi(this);

    ui.warningWidget->setText(i18n("Scaling changes will come into effect after restart"));
    ui.warningWidget->show();


    connect(ui.scaleSlider, &QSlider::valueChanged, ui.previewWidget, [this](qreal value) {
        ui.previewWidget->setScale(value / SLIDER_RATIO);
    });
    connect(ui.scaleSlider, &QSlider::valueChanged, ui.scaleLabel, [this](qreal value) {
        ui.scaleLabel->setText(QString::number(value / SLIDER_RATIO));
    });

    ui.previewWidget->setScale(1);
    ui.scaleLabel->setText(QString::number(1));
    
    load();
}

ScalingConfig::~ScalingConfig()
{
}

void ScalingConfig::load()
{
    //we load UI from a config, as rdb value might not be updated yet
    auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    const qreal dpi = config->group("KScreen").readEntry("ScaleFactor", 1.0);
    
    m_initialScalingFactor = dpi;
    ui.scaleSlider->setValue(dpi * SLIDER_RATIO);
}

void ScalingConfig::accept()
{
    if (qFuzzyCompare(scaleFactor(), m_initialScalingFactor)) {
        QDialog::accept();
        return;
    }
    const qreal scalingFactor = scaleFactor();

    //save to config
    auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    config->group("KScreen").writeEntry("ScaleFactor", scalingFactor);

    //write env var to be used by startkde.sh to populate the QT_SCREEN_SCALE_FACTORS env var
    //we use QT_SCREEN_SCALE_FACTORS as opposed to QT_SCALE_FACTOR as we need to use one that will *NOT* scale fonts according to the scale
    //scaling the fonts makes sense if you don't also set a font DPI, but we *need* to set a font DPI for both PlasmaShell which does it's own thing, and for KDE4/GTK2 applications

    QString screenFactors;
    foreach (const KScreen::OutputPtr &output, m_outputList) {
        screenFactors.append(output->name() + QLatin1Char('=') + QString::number(scalingFactor) + QLatin1Char(';'));
    }
    config->group("KScreen").writeEntry("ScreenScaleFactors", screenFactors);


    KConfig fontConfig(QStringLiteral("kcmfonts"));
    auto fontConfigGroup = fontConfig.group("General");

    if (qFuzzyCompare(scalingFactor, 1.0)) {
        //if dpi is the default (96) remove the entry rather than setting it
        QProcess proc;
        proc.start(QStringLiteral("xrdb -quiet -remove -nocpp"));
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi\n"));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
        fontConfigGroup.writeEntry("forceFontDPI", 0);
    } else {
        QProcess proc;
        proc.start(QStringLiteral("xrdb -quiet -merge -nocpp"));
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi: " + QString::number(scaleDPI()).toLatin1()));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
        fontConfigGroup.writeEntry("forceFontDPI", scaleDPI());
    }


    QDialog::accept();
}

int ScalingConfig::scaleDPI() const
{
    return qRound(scaleFactor() * 96.0);
}

qreal ScalingConfig::scaleFactor() const
{
    return ui.scaleSlider->value() / SLIDER_RATIO;
}

