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

class QMLOutput;
class QMLScreen;
class ControlPanel;
class ControlConfig;

class QQuickView;

namespace KScreen
{
class ConfigOperation;
}

namespace Ui
{
class KScreenWidget;
}

class Widget : public QWidget
{
    Q_OBJECT

  public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::ConfigPtr currentConfig() const;

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  Q_SIGNALS:
    void changed();
    void saveControls();

  private Q_SLOTS:
    void slotFocusedOutputChanged(QMLOutput *output);

    void slotOutputEnabledChanged();
    void slotOutputConnectedChanged();

    void slotUnifyOutputs();

    void slotIdentifyButtonClicked(bool checked = true);
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    void clearOutputIdentifiers();

    void outputAdded(const KScreen::OutputPtr &output);
    void outputRemoved(int outputId);
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);

  private:
    void loadQml();
    void resetPrimaryCombo();
    void addOutputToPrimaryCombo(const KScreen::OutputPtr &output);

    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);

  private:
    Ui::KScreenWidget *ui;
    QMLScreen *mScreen = nullptr;
    KScreen::ConfigPtr mConfig = nullptr;
    KScreen::ConfigPtr mPrevConfig = nullptr;

    ControlPanel *mControlPanel = nullptr;

    QList<QQuickView*> mOutputIdentifiers;
    QTimer *mOutputTimer = nullptr;
};

#endif // WIDGET_H
