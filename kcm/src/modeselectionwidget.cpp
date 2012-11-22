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
#include <kscreen/output.h>
#include <KDebug>

#include "qmloutput.h"
#include "modesproxymodel.h"
#include "resolutionsortmodel.h"

ModeSelectionWidget::ModeSelectionWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QGraphicsProxyWidget(parent, wFlags),
    m_output(0)
{
    m_resolutionsModel = new ResolutionSortModel(this);

    m_resolutionsView = new QListView();
    m_resolutionsView->setModel(m_resolutionsModel);
    connect(m_resolutionsView, SIGNAL(clicked(QModelIndex)), SLOT(resolutionChanged(QModelIndex)));

    m_refreshRatesModel = new ModesProxyModel(this);

    m_refreshRatesView = new QListView();
    m_refreshRatesView->setModel(m_refreshRatesModel);
    connect(m_resolutionsView, SIGNAL(clicked(QModelIndex)), SLOT(refreshRateChanged()));

    QWidget *rootWidget = new QWidget();

    QGridLayout *mainLayout = new QGridLayout(rootWidget);
    mainLayout->addWidget(m_resolutionsView, 0, 0);
    mainLayout->addWidget(m_refreshRatesView, 0, 1);
    rootWidget->setLayout(mainLayout);

    setWidget(rootWidget);
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

    KScreen::Mode *currentMode = m_output->output()->mode(m_output->output()->currentMode());
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

void ModeSelectionWidget::refreshRateChanged()
{
    if (!m_output) {
        return;
    }

    QModelIndex proxyModelIndex = m_resolutionsModel->index(m_resolutionsView->currentIndex().row(), 0);
    QModelIndex parentIndex = m_resolutionsModel->mapToSource(proxyModelIndex);
    QModelIndex modelIndex = m_refreshRatesView->model()->index(m_refreshRatesView->currentIndex().row(), 0, parentIndex);

    int modeId = m_refreshRatesView->model()->data(modelIndex, QMLOutput::ModeIdRole).toInt();
    if (modeId == -1) {
        QModelIndex proxyModelIndex = m_resolutionsModel->index(m_resolutionsView->currentIndex().row(), 0);
        QModelIndex parentIndex = m_resolutionsModel->mapToSource(proxyModelIndex);
        modeId = m_output->modesModel()->index(0, 0, parentIndex).data(QMLOutput::ModeIdRole).toInt();
    }

    if (modeId == 0) {
        return;
    }

    m_output->output()->setCurrentMode(modeId);
    m_refreshRatesView->repaint();
}

#include "modeselectionwidget.moc"
