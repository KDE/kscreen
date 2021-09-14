/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KSCREEN_OSDTEST_H
#define KSCREEN_OSDTEST_H

#include <QObject>

namespace KScreen
{
class OsdManager;

class OsdTest : public QObject
{
    Q_OBJECT

public:
    explicit OsdTest(QObject *parent = nullptr);
    ~OsdTest() override;

    void setUseDBus(bool yesno);

    void showGenericOsd(const QString &icon, const QString &message);
    void showOutputIdentifiers();
    void showActionSelector();

private:
    OsdManager *getOsdManager();
    OsdManager *m_osdManager = nullptr;
    bool m_useDBus = false;
};

} // namespace

#endif // KSCREEN_OSDTEST_H
