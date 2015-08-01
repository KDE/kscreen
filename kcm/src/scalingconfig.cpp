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
#include <QDebug>
#include <KSharedConfig>
#include <KConfigGroup>

//we want a scale between 1 and 3.0 in intervals of 0.1
//slider can only handle ints so goes 10-30
#define SLIDER_RATIO 10.0

ScalingConfig::ScalingConfig(QWidget* parent):
    QDialog(parent)
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
    
    load();
}

ScalingConfig::~ScalingConfig()
{
}

void ScalingConfig::load()
{
    //we load UI from a config, as rdb value might not be updated yet
    auto config = KSharedConfig::openConfig("kdeglobals");
    const qreal dpi = config->group("KScreen").readEntry("ScaleFactor", 1.0);
    
    m_initialScalingFactor = dpi;
    ui.scaleSlider->setValue(dpi * SLIDER_RATIO);
}

void ScalingConfig::accept()
{
    if (scaleFactor() == m_initialScalingFactor) {
        QDialog::accept();
        return;
    }
    const qreal scalingFactor = scaleFactor();
    
    //save to config
    //note this is also used by startkde.sh to populate the QT_DEVICE_PIXEL_RATIO env var
    auto config = KSharedConfig::openConfig("kdeglobals");
    config->group("KScreen").writeEntry("ScaleFactor", scalingFactor);   
        
    if (qFuzzyCompare(scalingFactor, 1.0)) {
        //if dpi is the default (96) remove the entry rather than setting it
        QProcess proc;
        proc.start("xrdb -quiet -remove -nocpp");
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi\n"));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
    } else {
        QProcess proc;
        proc.start("xrdb -quiet -merge -nocpp");
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi: " + QString::number(scaleDPI()).toLatin1()));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
    }
    QDialog::accept();
}

qreal ScalingConfig::scaleDPI() const
{
    return scaleFactor() * 96.0;
}

qreal ScalingConfig::scaleFactor() const
{
    return ui.scaleSlider->value() / SLIDER_RATIO;
}
