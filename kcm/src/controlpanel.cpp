/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "controlpanel.h"


#include <QtGui/QVBoxLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QSlider>
#include <QtGui/QComboBox>

#include <KLocalizedString>
#include <KStandardDirs>
#include <KDebug>

#include <kscreen/output.h>

ControlPanel::ControlPanel(QWidget *parent):
    QWidget(parent)
{
    initUi();
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::initUi()
{
    QFormLayout *formLayout = new QFormLayout(this);

    m_outputNameLabel = new QLabel(this);
    formLayout->addWidget(m_outputNameLabel);

    m_outputTypeLabel = new QLabel(this);
    formLayout->addWidget(m_outputTypeLabel);

    m_enabledCheckBox = new QCheckBox(this);
    formLayout->addRow(i18n("Enabled"), m_enabledCheckBox);
    connect(m_enabledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotEnabledChanged(bool)));

    m_resolutionSlider = new QSlider(Qt::Horizontal, this);
    formLayout->addRow(i18n("Resolution"), m_resolutionSlider);
    connect(m_resolutionSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotResolutionChanged(int)));

    m_rotationCombo = new QComboBox(this);
    m_rotationCombo->addItem(i18n("Normal"), KScreen::Output::None);
    m_rotationCombo->addItem(i18n("90 degrees"), KScreen::Output::Left);
    m_rotationCombo->addItem(i18n("180 degrees"), KScreen::Output::Inverted);
    m_rotationCombo->addItem(i18n("270 degrees"), KScreen::Output::Right);
    formLayout->addRow(i18n("Rotation"), m_rotationCombo);
    connect(m_rotationCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotRotationChanged(int)));
}

void ControlPanel::slotEnabledChanged(bool checked)
{

}

void ControlPanel::slotResolutionChanged(int value)
{

}

void ControlPanel::slotRotationChanged(int index)
{

}

#include "controlpanel.moc"
