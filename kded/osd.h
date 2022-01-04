/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSCREEN_OSD_H
#define KSCREEN_OSD_H

#include <QObject>
#include <QRect>
#include <QString>

#include <KScreen/Output>

#include "osdmanager.h"

namespace KDeclarative
{
class QmlObject;
}

class QTimer;

namespace KScreen
{
class Osd : public QObject
{
    Q_OBJECT

public:
    explicit Osd(const OutputPtr &output, QObject *parent = nullptr);
    ~Osd() override;

    void showGenericOsd(const QString &icon, const QString &text);
    void showOutputIdentifier(const KScreen::OutputPtr &output, bool shouldShouldSerialNumber);
    void showActionSelector();
    void hideOsd();

Q_SIGNALS:
    void osdActionSelected(OsdAction::Action action);

private Q_SLOTS:
    void onOsdActionSelected(int action);
    void onOutputAvailabilityChanged();

private:
    bool initOsd();

    KScreen::OutputPtr m_output;
    QRect m_outputGeometry;
    KDeclarative::QmlObject *m_osdActionSelector = nullptr;
    QTimer *m_osdTimer = nullptr;
    int m_timeout = 0;
};

} // ns

#endif // KSCREEN_OSD_H
