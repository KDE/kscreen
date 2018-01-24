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

#include <QWidget>
#include <QVariantMap>

#include <kscreen/config.h>

class ProfilesModel;
class QMLOutput;
class QMLScreen;
class ControlPanel;
class PrimaryOutputCombo;

class QPushButton;
class QComboBox;

class QQuickView;

namespace KScreen
{
class ConfigOperation;
}

class Widget : public QWidget
{
    Q_OBJECT

  public:
    explicit Widget(QWidget *parent = nullptr);
    virtual ~Widget();

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::ConfigPtr currentConfig() const;

  protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

  Q_SIGNALS:
    void changed();

  private Q_SLOTS:
    void slotFocusedOutputChanged(QMLOutput *output);

    void slotOutputEnabledChanged();

    void slotUnifyOutputs();
    void slotProfileChanged(int index);

    void slotProfilesAboutToUpdate();
    void slotProfilesUpdated();

    void slotIdentifyButtonClicked(bool checked = true);
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    void clearOutputIdentifiers();

  private:
    void loadQml();
    void initPrimaryCombo();

    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);

  private:
    QMLScreen *mScreen = nullptr;
    KScreen::ConfigPtr mConfig;
    KScreen::ConfigPtr mPrevConfig;

    QQuickView *mDeclarativeView = nullptr;
    ControlPanel *mControlPanel = nullptr;

    ProfilesModel *mProfilesModel = nullptr;
    PrimaryOutputCombo *mPrimaryCombo = nullptr;
    QComboBox *mProfilesCombo = nullptr;

    QPushButton *mScaleAllOutputsButton = nullptr;
    QPushButton *mUnifyButton = nullptr;
    QPushButton *mSaveProfileButton = nullptr;

    QList<QQuickView*> mOutputIdentifiers;
    QTimer *mOutputTimer = nullptr;

};

#endif // WIDGET_H
