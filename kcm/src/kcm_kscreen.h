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

#ifndef KCM_KSCREEN_H
#define KCM_KSCREEN_H

#include <KCModule>
#include <QDeclarativeListProperty>

class QDeclarativeView;
class QTimer;
class ControlPanel;

namespace KScreen {
class Config;
}

class KCMKScreen : public KCModule
{
    Q_OBJECT
public:
    explicit KCMKScreen (QWidget* parent = 0, const QVariantList& args = QVariantList());
    virtual ~KCMKScreen();

public Q_SLOTS:
    virtual void load();
    virtual void save();

    static bool x11EventFilter(void* message, long int* result);

private Q_SLOTS:
    void identifyOutputs();
    void clearOutputIdentifiers();
    void moveMouse(int dX, int dY);

    void outputMousePressed();
    void outputMouseReleased();

private:
    KScreen::Config *m_config;

    QDeclarativeView *m_declarativeView;
    ControlPanel *m_controlPanel;

    QList<QWidget*> m_outputIdentifiers;
    QTimer *m_outputTimer;
};

#endif // DisplayConfiguration_H
