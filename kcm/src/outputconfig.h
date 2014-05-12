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

#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QGroupBox>
#include <QComboBox>

class CollapsableButton;
class QCheckBox;
class ResolutionSlider;
class QLabel;

namespace KScreen
{
class Output;
}

class OutputConfig : public QGroupBox
{
    Q_OBJECT

  public:
    explicit OutputConfig(QWidget *parent);
    explicit OutputConfig(KScreen::Output *output, QWidget *parent = 0);
    virtual ~OutputConfig();

    virtual void setOutput(KScreen::Output *output);
    KScreen::Output* output() const;

  protected Q_SLOTS:
    void slotOutputConnectedChanged();
    void slotOutputEnabledChanged();
    void slotOutputRotationChanged();

    void slotEnabledChanged(bool checked);
    void slotResolutionChanged(const QSize &size);
    void slotRotationChanged(int index);
    void slotRefreshRateChanged(int index);

  Q_SIGNALS:
    void changed();

  protected:
    virtual void initUi();

  protected:
    KScreen::Output *mOutput;
    QCheckBox *mEnabled;
    ResolutionSlider *mResolution;
    QComboBox *mRotation;
    QComboBox *mRefreshRate;
};

#endif // OUTPUTCONFIG_H
