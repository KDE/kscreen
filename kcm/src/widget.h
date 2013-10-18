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

#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>

class QMLScreen;
class KPushButton;
class KComboBox;
namespace KScreen
{
class Config;
}

class ControlPanel;
class QDeclarativeView;
class QMLOutput;

class Widget : public QWidget
{
    Q_OBJECT

  public:
    explicit Widget(QWidget *parent = 0);
    virtual ~Widget();

    KScreen::Config* currentConfig() const;

  private Q_SLOTS:
    void slotFocusedOutputChanged(QMLOutput *output);

    void slotPrimaryChanged(int index);
    void slotOutputPrimaryChanged();
    void slotOutputConnectedChanged();
    void slotOutputEnabledChanged();

    void slotUnifyOutputs();
  private:
    void loadQml();

  private:
    QMLScreen *mScreen;
    KScreen::Config *mConfig;
    KScreen::Config *mPrevConfig;

    QDeclarativeView *m_declarativeView;
    ControlPanel *m_controlPanel;

    KComboBox *mPrimaryCombo;
    KComboBox *mProfilesCombo;

    KPushButton *mUnifyButton;
    KPushButton *mSaveProfileButton;

};

#endif // WIDGET_H
