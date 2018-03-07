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

#ifndef RESOLUTIONSLIDER_H
#define RESOLUTIONSLIDER_H

#include <QWidget>
#include <QtCore/QSet>

#include <kscreen/output.h>

class QSlider;
class QLabel;
class QComboBox;

class ResolutionSlider : public QWidget
{
    Q_OBJECT

  public:
    explicit ResolutionSlider(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    virtual ~ResolutionSlider();

    QSize currentResolution() const;

  Q_SIGNALS:
    void resolutionChanged(const QSize &size);

  private Q_SLOTS:
    void slotValueChanged(int);
    void slotOutputModeChanged();

  private:
    KScreen::OutputPtr mOutput;

    QList<QSize> mModes;

    QLabel *mSmallestLabel = nullptr;
    QLabel *mBiggestLabel = nullptr;
    QLabel *mCurrentLabel = nullptr;
    QSlider *mSlider = nullptr;
    QComboBox *mComboBox = nullptr;
};

#endif // RESOLUTIONSLIDER_H
