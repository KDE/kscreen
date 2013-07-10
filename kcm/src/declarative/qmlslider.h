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

#ifndef QMLSLIDER_H
#define QMLSLIDER_H

#include <QGraphicsProxyWidget>
#include <QDeclarativeListProperty>

class QSlider;
namespace KScreen {
class Output;
class Mode;
}


class QMLSlider : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(KScreen::Output *output
               READ output
               WRITE setOutput
               NOTIFY outputChanged)

    Q_PROPERTY(QDeclarativeListProperty<KScreen::Mode> modes
               READ modes
               NOTIFY modesChanged)

  public:
    explicit QMLSlider(QGraphicsItem *parent = 0);
    virtual ~QMLSlider();

    KScreen::Output *output() const;
    void setOutput(KScreen::Output *output);

    QDeclarativeListProperty<KScreen::Mode> modes();

  Q_SIGNALS:
    void outputChanged();
    void modesChanged();

  private Q_SLOTS:
    void slotSliderMoved();

  private:
    QSlider *m_slider;
    KScreen::Output *m_output;

    QList<KScreen::Mode*> m_modes;

};

#endif // QMLSLIDER_H
