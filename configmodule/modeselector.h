/*
    Copyright 2016 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef KSCREEN_MODESELECTOR_H
#define KSCREEN_MODESELECTOR_H

#include <QObject>
#include <QtQml>
#include <kscreen/mode.h>

namespace KScreen {

class ModeSelector : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KScreen::Output* output READ output NOTIFY outputChanged)

    Q_PROPERTY(QQmlListProperty<KScreen::Mode> modes READ modes NOTIFY modesChanged)

    Q_PROPERTY(QStringList modeSizes READ modeSizes NOTIFY modesChanged)
    Q_PROPERTY(QStringList refreshRatesLabels READ refreshRatesLabels NOTIFY refreshRatesChanged)
    Q_PROPERTY(QList<qreal> refreshRates READ refreshRates NOTIFY refreshRatesChanged)
    Q_PROPERTY(KScreen::Mode* selectedMode READ selectedMode NOTIFY selectedModeChanged)

    Q_PROPERTY(QString modeLabelMin READ modeLabelMin NOTIFY modesChanged)
    Q_PROPERTY(QString modeLabelMax READ modeLabelMax NOTIFY modesChanged)
    Q_PROPERTY(QString refreshLabelMin READ refreshLabelMin NOTIFY refreshRatesChanged)
    Q_PROPERTY(QString refreshLabelMax READ refreshLabelMax NOTIFY refreshRatesChanged)


  public:
    explicit ModeSelector(QObject *parent = 0);
    virtual ~ModeSelector();

    KScreen::OutputPtr outputPtr() const;
    void setOutputPtr(const KScreen::OutputPtr &output);

    KScreen::Output* output() const;
    QQmlListProperty<KScreen::Mode> modes();
    QStringList modeSizes();
    QStringList refreshRatesLabels() const;
    QList<qreal> refreshRates();
    KScreen::Mode* selectedMode() const;

    Q_INVOKABLE void setSelectedResolutionIndex(int index);
    Q_INVOKABLE void setSelectedRefreshRate(int index);
    Q_INVOKABLE int currentModeIndex() const;
    Q_INVOKABLE int preferredRefreshIndexForSizeIndex(int index);

    QString modeLabelMin() const;
    QString modeLabelMax() const;

    QString refreshLabelMin() const;
    QString refreshLabelMax() const;

  Q_SIGNALS:
    void outputChanged();
    void modesChanged();
    void refreshRatesChanged();
    void selectedModeChanged();

  private:
    friend class TestConfigModule;
    void updateModes();
    void updateSelectedMode();

    KScreen::OutputPtr m_output;
    QList<KScreen::Mode*> m_modes;
    QStringList m_modeSizes;
    QHash<QString, QList<qreal>> m_refreshRatesTable;
    QString m_selectedModeId;
    QString m_selectedModeSize;
    int m_currentModeIndex = 0;
    qreal m_selectedRefreshRate = 0;
    KScreen::ModePtr m_initialMode;

    QString m_modeLabelMin;
    QString m_modeLabelMax;
    QString m_refreshLabelMin;
    QString m_refreshLabelMax;
};

};

#endif // KSCREEN_MODESELECTOR_H

