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

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtGui/QScrollArea>

class QVBoxLayout;
class OutputConfig;
class UnifiedOutputConfig;

class QLabel;
class QCheckBox;
class QSlider;
class QComboBox;

namespace KScreen {
class Config;
class Output;
}

class ControlPanel : public QScrollArea
{
    Q_OBJECT

  public:
    explicit ControlPanel(QWidget *parent = 0);
    virtual ~ControlPanel();

    void setConfig(KScreen::Config *config);

    void setUnifiedOutput(KScreen::Output *output);

  public Q_SLOTS:
    void activateOutput(KScreen::Output *output);

  Q_SIGNALS:
    void changed();

  private:
    KScreen::Config *mConfig;
    QList<OutputConfig*> mOutputConfigs;

    QVBoxLayout *mLayout;
    UnifiedOutputConfig *mUnifiedOutputCfg;
};

#endif // CONTROLPANEL_H
