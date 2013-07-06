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


#include "qmloutput.h"
#include <kscreen/output.h>
#include <KDebug>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringBuilder>

Q_DECLARE_METATYPE(KScreen::Mode*)

bool operator>(const QSize &sizeA, const QSize &sizeB)
{
    return ((sizeA.width() > sizeB.width()) && (sizeA.height() > sizeB.height()));
}

QMLOutput::QMLOutput():
    QDeclarativeItem(),
    m_output(0),
    m_cloneOf(0),
    m_modesModel(new QStandardItemModel(this))
{

}

QMLOutput::~QMLOutput()
{

}

void QMLOutput::setOutput(KScreen::Output* output)
{
    m_output = output;

    QList<KScreen::Mode*> modes = m_output->modes().values();
    Q_FOREACH (KScreen::Mode *mode, modes) {
        QList<QStandardItem*> items = m_modesModel->findItems(mode->name(), Qt::MatchExactly, 0);
        if (items.isEmpty()) {
            QStandardItem *item = new QStandardItem(mode->name());
            item->setData(mode->size(), QMLOutput::SizeRole);

            m_modesModel->appendRow(item);
            items << item;
        }

        QStandardItem *modeItem = new QStandardItem(QString::number(mode->refreshRate(), 'f', 1) % QLatin1String("Hz"));
        modeItem->setData(mode->refreshRate(), QMLOutput::RefreshRateRole);
        modeItem->setData(mode->id(), QMLOutput::ModeIdRole);
        modeItem->setData(QVariant::fromValue(mode), QMLOutput::ModeRole);

        QStandardItem *item = items.first();
        item->appendRow(modeItem);
    }

    connect(output, SIGNAL(clonesChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(currentModeIdChanged()), SIGNAL(currentOutputSizeChanged()));
    connect(output, SIGNAL(currentModeIdChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(isEnabledChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(isPrimaryChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(outputChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(posChanged()), SIGNAL(changed()));
    connect(output, SIGNAL(rotationChanged()), SIGNAL(changed()));

    Q_EMIT outputChanged();
}

KScreen::Output* QMLOutput::output() const
{
    return m_output;
}

void QMLOutput::setCloneOf(QMLOutput* other)
{
    m_cloneOf = other;

    Q_EMIT cloneOfChanged();
}

QMLOutput* QMLOutput::cloneOf() const
{
    return m_cloneOf;
}

QAbstractItemModel* QMLOutput::modesModel()
{
    return m_modesModel;
}

int QMLOutput::currentOutputHeight() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::Mode *mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }

    return mode->size().height();
}

int QMLOutput::currentOutputWidth() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::Mode *mode = m_output->currentMode();
    if (!mode) {
        if (m_output->isConnected()) {
            mode = bestMode();
            m_output->setCurrentModeId(mode->id());
        } else {
            return 1000;
        }
    }

    return mode->size().width();
}

int QMLOutput::outputX() const
{
    return m_output->pos().x();
}

void QMLOutput::setOutputX(int x)
{
    QPoint pos = m_output->pos();
    pos.setX(x);
    m_output->setPos(pos);
}

int QMLOutput::outputY() const
{
    return m_output->pos().y();
}

void QMLOutput::setOutputY(int y)
{
    QPoint pos = m_output->pos();
    pos.setY(y);
    m_output->setPos(pos);
}


float QMLOutput::displayScale() const
{
    return (1.0 / 6.0);
}

KScreen::Mode* QMLOutput::bestMode() const
{
    if (!m_output) {
        return 0;
    }

    KScreen::ModeList modes = m_output->modes();
    KScreen::Mode *bestMode = 0;
    Q_FOREACH (KScreen::Mode *mode, modes) {
        if (!bestMode || (mode->size() > bestMode->size())) {
            bestMode = mode;
        }
    }

    return bestMode;
}

#include "qmloutput.moc"
