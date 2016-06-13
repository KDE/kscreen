/*
 * Copyright (C) 2015 David Edmundson <davidedmundson@kde.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

#ifndef SCALINGCONFIG_H
#define SCALINGCONFIG_H

#include <QDialog>

#include <KScreen/Types>

#include "ui_scaling.h"

class ScalingConfig : public QDialog
{
    Q_OBJECT
public:
    explicit ScalingConfig(const KScreen::OutputList &outputList, QWidget* parent = 0);
    virtual ~ScalingConfig();
    
protected:
    void accept() Q_DECL_OVERRIDE;
private:
    void load();
    qreal scaleFactor() const;
    qreal scaleDPI() const;
    Ui::Scaling ui;
    qreal m_initialScalingFactor = 1.0;
    KScreen::OutputList m_outputList;
};

#endif // SCALINGCONFIG_H
