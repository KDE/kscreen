/*
 * Copyright (C) 2015 David Edmundson <davidedmundson@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QLabel>

class PreviewWidget : public QLabel
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent=0);
    ~PreviewWidget();
    void setScale(qreal scale);
public Q_SLOTS:
    void updatePixmapCache();
private:
    qreal pointSizeToPixelSize(qreal pointSize) const;
    qreal m_scale;
    QWidget *m_internalPreview;
};

#endif // PREVIEWWIDGET_H
