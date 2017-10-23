/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QFrame>

#include <kscreen/output.h>

class QVBoxLayout;
class OutputConfig;
class UnifiedOutputConfig;

class QLabel;
class QCheckBox;
class QSlider;
class QComboBox;

class ControlPanel : public QFrame
{
    Q_OBJECT

  public:
    explicit ControlPanel(QWidget *parent = nullptr);
    virtual ~ControlPanel();

    void setConfig(const KScreen::ConfigPtr &config);

    void setUnifiedOutput(const KScreen::OutputPtr &output);

  public Q_SLOTS:
    void activateOutput(const KScreen::OutputPtr &output);

  Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void addOutput(const KScreen::OutputPtr &output);
    void removeOutput(int outputId);

  private:
    KScreen::ConfigPtr mConfig;
    QList<OutputConfig*> mOutputConfigs;

    QVBoxLayout *mLayout;
    UnifiedOutputConfig *mUnifiedOutputCfg;
};

#endif // CONTROLPANEL_H
