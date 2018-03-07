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

class Widget;
class QTimer;
class QHBoxLayout;
namespace KScreen
{
class ConfigOperation;
}

class KCMKScreen : public KCModule
{
    Q_OBJECT

  public:
    explicit KCMKScreen (QWidget* parent = nullptr, const QVariantList& args = QVariantList());
    virtual ~KCMKScreen();

    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

  public Q_SLOTS:
    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;
    void defaults() Q_DECL_OVERRIDE;
    void changed();

  private:
    void configReady(KScreen::ConfigOperation *op);

    Widget *mKScreenWidget = nullptr;
    bool m_blockChanges = false;
    QHBoxLayout *mMainLayout = nullptr;

};

#endif // DisplayConfiguration_H
