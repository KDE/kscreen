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

#include "modeselectionwidget.h"

#include <QListView>
#include <QGridLayout>
#include <QPainter>
#include <QGraphicsProxyWidget>
#include <kscreen/output.h>
#include <KDebug>
#include <QRectF>
#include <sys/socket.h>

#include "qmloutput.h"
#include "modesproxymodel.h"
#include "resolutionsortmodel.h"

ModeSelectionWidget::ModeSelectionWidget(QDeclarativeItem *parent):
    QDeclarativeItem(parent),
    m_output(0),
    m_refreshRatesModel(new ModesProxyModel(this)),
    m_resolutionsModel(new ResolutionSortModel(this))
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    rootWidget = new QWidget();

    m_resolutionsView = new QListView(rootWidget);
    m_resolutionsView->setModel(m_resolutionsModel);
    m_resolutionsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_resolutionsView, SIGNAL(clicked(QModelIndex)), SLOT(resolutionChanged(QModelIndex)));
    connect(m_resolutionsView, SIGNAL(doubleClicked(QModelIndex)), SLOT(acceptMode(QModelIndex)));

    m_refreshRatesView = new QListView(rootWidget);
    m_refreshRatesView->setModel(m_refreshRatesModel);
    m_refreshRatesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_refreshRatesView, SIGNAL(clicked(QModelIndex)), SLOT(refreshRateChanged()));
    connect(m_refreshRatesView, SIGNAL(doubleClicked(QModelIndex)), SLOT(acceptMode(QModelIndex)));

    QGridLayout *mainLayout = new QGridLayout(rootWidget);
    mainLayout->addWidget(m_resolutionsView, 0, 0);
    mainLayout->addWidget(m_refreshRatesView, 0, 1);
    rootWidget->setLayout(mainLayout);
    rootWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rootWidget->setMaximumSize(300, 250);
    rootWidget->setAttribute(Qt::WA_TranslucentBackground);

    m_proxyWidget = new QGraphicsProxyWidget(this);
    m_proxyWidget->setWidget(rootWidget);

    /* FIXME: Shouldn't this be automatic? */
    setWidth(m_proxyWidget->geometry().width());
    setHeight(m_proxyWidget->geometry().height());
}

ModeSelectionWidget::~ModeSelectionWidget()
{

}

void ModeSelectionWidget::setOutput(QMLOutput *output)
{
    m_output = output;

    if (!output) {
        return;
    }

    m_resolutionsModel->setSourceModel(output->modesModel());
    m_resolutionsModel->sort(0, Qt::DescendingOrder);

    m_refreshRatesModel->setSourceModel(m_resolutionsModel);

    KScreen::Mode *currentMode = m_output->output()->currentMode();
    if (!currentMode) {
        return;
    }

    for (int i = 0; i < m_resolutionsModel->rowCount(); i++) {
        QSize size = m_resolutionsModel->index(i, 0).data(QMLOutput::SizeRole).toSize();

        if (size == currentMode->size()) {
            QModelIndex index = m_resolutionsModel->index(i, 0);
            m_resolutionsView->setCurrentIndex(index);
            resolutionChanged(index);
            break;
        }
    }
}

QMLOutput *ModeSelectionWidget::output() const
{
    return m_output;
}

void ModeSelectionWidget::resolutionChanged(const QModelIndex &index)
{
    m_refreshRatesModel->setSourceModelCurrentRow(index.row());

    if (!m_refreshRatesView->currentIndex().isValid()) {
        m_refreshRatesView->setCurrentIndex(m_refreshRatesModel->index(0, 0));
    } else {
        refreshRateChanged();
    }
}

void ModeSelectionWidget::acceptMode(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (sender() == m_refreshRatesView) {
        refreshRateChanged();
        Q_EMIT accepted();
    } else if (sender() == m_resolutionsView) {
        resolutionChanged(index);
        Q_EMIT accepted();
    }
}


void ModeSelectionWidget::refreshRateChanged()
{
    if (!m_output) {
        return;
    }

    QModelIndex proxyModelIndex = m_resolutionsModel->index(m_resolutionsView->currentIndex().row(), 0);
    QModelIndex parentIndex = m_resolutionsModel->mapToSource(proxyModelIndex);
    QModelIndex modelIndex = m_refreshRatesView->model()->index(m_refreshRatesView->currentIndex().row(), 0, parentIndex);

    QString modeId = m_refreshRatesView->model()->data(modelIndex, QMLOutput::ModeIdRole).toString();
    if (modeId == QLatin1String("-1")) {
        QModelIndex proxyModelIndex = m_resolutionsModel->index(m_resolutionsView->currentIndex().row(), 0);
        QModelIndex parentIndex = m_resolutionsModel->mapToSource(proxyModelIndex);
        modeId = m_output->modesModel()->index(0, 0, parentIndex).data(QMLOutput::ModeIdRole).toString();
    }

    if (modeId == QLatin1String("0")) {
        return;
    }

    m_output->output()->setCurrentModeId(modeId);
    m_refreshRatesView->repaint();
}

#include "modeselectionwidget.moc"
