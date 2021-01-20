/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>

#include <kscreen/output.h>

namespace KScreen
{
class Config;
}
class Console : public QObject
{
    Q_OBJECT
public:
    explicit Console(const KScreen::ConfigPtr &config);
    ~Console() override;

public Q_SLOTS:
    void printConfig();
    void printJSONConfig();
    QString typetoString(const KScreen::Output::Type &type) const;
    void printSerializations();
    void monitor();
    void monitorAndPrint();

private:
    KScreen::ConfigPtr m_config;
};

#endif
