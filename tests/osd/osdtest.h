/*************************************************************************************
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>                                  *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

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
