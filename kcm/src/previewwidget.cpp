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

#include "previewwidget.h"

#include <QPainter>

#include "ui_stylepreview.h"


PreviewWidget::PreviewWidget(QWidget *parent):
    QLabel(parent),
    m_scale(1.0),
    m_internalPreview(new QWidget) // deliberately no parent, we don't want it to have a screen
{
    Ui::StylePreview ui;
    ui.setupUi(m_internalPreview);
}

PreviewWidget::~PreviewWidget()
{
    delete m_internalPreview;
}

void PreviewWidget::setScale(qreal scale)
{
    m_scale = scale;

    QFont font;
    //take the user's configured point size, and convert it to a pixel size for preview

    font.setPixelSize(pointSizeToPixelSize(font.pointSize()));
    m_internalPreview->setFont(font);

    //as we are a hidden widget, we need to force a repaint to update the size hint properly
    updatePixmapCache();
    m_internalPreview->resize(sizeHint());
    m_internalPreview->adjustSize();

    QPixmap preview = updatePixmapCache();
    setPixmap(preview);
}

qreal PreviewWidget::pointSizeToPixelSize(qreal pointSize) const
{
    //point size is in how many 72ths of an inch it should be, default DPI is 96
    qreal pixelSize = pointSize * 96.0 / 72.0;

    //scale by our new factor
    pixelSize *= m_scale;

    return pixelSize / m_scale; //as we are now dealing with pixels it will be scaled up in the paint(), so it needs dividing here
}

QPixmap PreviewWidget::updatePixmapCache()
{
   QPixmap pixmap(m_internalPreview ->sizeHint() * m_scale);
   pixmap.setDevicePixelRatio(m_scale);
   QPainter p(&pixmap);
   m_internalPreview ->render(&p);

   //render back at whatever the native DPR of the KCM is
   pixmap.setDevicePixelRatio(devicePixelRatioF());

   return pixmap;
}


