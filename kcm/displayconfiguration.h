/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

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

#ifndef DisplayConfiguration_H
#define DisplayConfiguration_H

#include <KCModule>
#include <QDeclarativeListProperty>

class QDeclarativeView;
class QMLOutput;
class QMLOutputView;
class /*KScreen::*/Config;

class DisplayConfiguration : public KCModule
{
    Q_OBJECT
public:
    DisplayConfiguration (QWidget* parent = 0, const QVariantList& args = QVariantList());
    virtual ~DisplayConfiguration();

public Q_SLOTS:
    virtual void load();
    virtual void save();

    static bool x11EventFilter(void* message, long int* result);

private:
    QMLOutputView* getOutputView() const;
    /*KScreen::*/Config *m_config;

    QDeclarativeView* m_declarativeView;
    QList<QMLOutput*> m_monitors;
};

#endif // DisplayConfiguration_H
