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
#include <QWidget>

#include <kscreen/output.h>

class QCheckBox;
class ResolutionSlider;
class QLabel;


class OutputConfig : public QWidget
{
    Q_OBJECT

  public:
    explicit OutputConfig(QWidget *parent);
    explicit OutputConfig(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    ~OutputConfig() override;

    virtual void setOutput(const KScreen::OutputPtr &output);
    KScreen::OutputPtr output() const;

    void setTitle(const QString &title);
    void setShowScaleOption(bool showScaleOption);
    bool showScaleOption() const;

  protected Q_SLOTS:
    void slotResolutionChanged(const QSize &size);
    void slotRotationChanged(int index);
    void slotRefreshRateChanged(int index);
    void slotScaleChanged(int index);

  Q_SIGNALS:
    void changed();

  protected:
    virtual void initUi();

  protected:
    QLabel *mTitle = nullptr;
    KScreen::OutputPtr mOutput;
    QCheckBox *mEnabled = nullptr;
    ResolutionSlider *mResolution = nullptr;
    QComboBox *mRotation = nullptr;
    QComboBox *mScale = nullptr;
    QComboBox *mRefreshRate = nullptr;
    bool mShowScaleOption  = false;
};

#endif // OUTPUTCONFIG_H
