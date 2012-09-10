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


#include "qmloutputview.h"
#include "qmloutputcomponent.h"
#include "qmloutput.h"

#include <QDeclarativeEngine>
#include <qdeclarativeexpression.h>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <KStandardDirs>
#include <KDebug>
#include <QGraphicsScene>

#include <kscreen/output.h>

Q_DECLARE_METATYPE(QMLOutput*);

QMLOutputView::QMLOutputView():
	QDeclarativeItem()
{
}

QMLOutputView::~QMLOutputView()
{
}

QList<QMLOutput*> QMLOutputView::outputs() const
{
	return m_outputs;
}

void QMLOutputView::addOutput(QDeclarativeEngine *engine, /*KScreen::*/Output* output)
{
	QMLOutputComponent outputComponent(engine);

	QMLOutput *instance = dynamic_cast<QMLOutput*>(outputComponent.createForOutput(output));
	if (!instance) {
		kWarning() << "Failed to add output" << output->name();
		return;
	}

	instance->setParentItem(this);

	/* Root refers to the root object. We need it in order to set drag range */
	instance->setProperty("viewport", this->property("root"));
	connect(instance, SIGNAL(moved()), this, SLOT(outputMoved()));
	connect(instance, SIGNAL(clicked()), this, SLOT(outputClicked()));

	m_outputs << instance;
	instance->setProperty("z", m_outputs.count());

	Q_EMIT outputsChanged();
}

void QMLOutputView::outputClicked()
{
	for (int i = 0; i < m_outputs.count(); i++) {
		QMLOutput *output = m_outputs.at(i);

		/* Find clicked child and move it above all it's siblings */
		if (output == sender()) {
			for (int j = i + 1; j < m_outputs.count(); j++) {
				int z = m_outputs.at(j)->property("z").toInt();
				m_outputs.at(j)->setProperty("z", z - 1);
				m_outputs.at(j)->setProperty("focus", false);
			}
			output->setProperty("z", m_outputs.length());
			output->setProperty("focus", true);
			break;
		}

		output->setProperty("focus", false);
	}
}


void QMLOutputView::outputMoved()
{
	QMLOutput *output = dynamic_cast<QMLOutput*>(sender());

	/* FIXME: output->{x,y}() change even when window is snapped. This must be fixed */
	int x = output->x();
	int y = output->y();
	int width =  output->width();
	int height = output->height();

	Q_FOREACH (QMLOutput *otherOutput, m_outputs) {
		if (otherOutput == output) {
			continue;
		}

		int x2 = otherOutput->x();
		int y2 = otherOutput->y();
		int height2 = otherOutput->height();
		int width2 = otherOutput->width();

		kDebug() << x + width << x2;

		/* @output is snapped to @otherOutput on left and their
		 * upper sides are aligned */
		if ((x + width == x2) && (y < y2 + 15) && (y > y2 - 15)) {
			output->setY(y2);
			return;
		}

		/* @output is snapped to @otherOutput on left and their
		 * bottom sides are aligned */
		if ((x + width == x2) && (y + height < y2 + height2 + 15) && (y + height > y2 + height2 - 15)) {
			output->setY(y2 + height2 - height);
			return;
		}

		/* @output is snapped to @otherOutput on right and their
		 * upper sides are aligned */
		if ((x == x2 + width) && (y < y2 + 15) && (y > y2 - 15)) {
			output->setY(y2);
			return;
		}

		/* @output is snapped to @otherOutput on right and their
		 * bottom sides are aligned */
		if ((x == x2 + width) && (y + height < y2 + height2 + 15) && (y + height > y2 + height2 -15)) {
			output->setY(y2 + height2 - height);
			return;
		}

		/* @output is snapped to @otherOutput on top and their
		 * left sides are aligned */
		if ((y + height == y2) && (x < x2 + 15) && (x > x2 - 15)) {
			output->setX(x2);
			return;
		}

		/* @output is snapped to @otherOutput on top and their
		 * right sides are aligned */
		if ((y + height == y2) && (x + width < x2 + width2 + 15) && (x + width > x2 + width2 - 15)) {
			output->setX(x2 + width2 - width);
			return;
		}

		/* @output is snapped to @otherOutput on bottom and their
		 * left sides are aligned */
		if ((y == y2 + height2) && (x < x2 + 15) && (x > x2 - 15)) {
			output->setX(x2);
			return;
		}

		/* @output is snapped to @otherOutput on bottom and their
		 * right sides are aligned */
		if ((y == y2 + height2) && (x + width < x2 + width2 + 15) && (x + width > x2 + width2 - 15)) {
			output->setX(x2 + width2 - width);
			return;
		}



		/* @output is left of @otherOutput */
		if ((x + width > x2 - 30) && (x + width < x2 + 30) &&
		    (y + height > y2) && (y < y2 + height2)) {

			output->setX(x2 - width);
			return;
		}

		/* @output is right of @otherOutput */
		if ((x > x2 + width2 - 30) && (x < x2 + width2 + 30) &&
		    (y + height > y2) && (y < y2 + height2)) {

			output->setX(x2 + width2);
			return;
		}

		/* @output is above @otherOutput */
		if ((y + height > y2 - 30) && (y + height < y2 + 30) &&
		    (x + width > x2) && (x < x2 + width2)) {

			output->setY(y2 - height);
			return;
		}

		/* @output is below @otherOutput */
		if ((y > y2 + height2 - 30) && (y < y2 + height2 + 30) &&
		    (x + width > x2) && (x < x2 + width2)) {


			output->setY(y2 + height2);
			return;
		}

		/* @output is centered with @otherOutput */
		int centerX = x + (width / 2);
		int centerY = y + (height / 2);
		int centerX2 = x2 + (width2 / 2);
		int centerY2 = y2 + (height2 / 2);
		if ((centerX > centerX2 - 30) && (centerX < centerX2 + 30) &&
		    (centerY > centerY2 - 30) && (centerY < centerY2 + 30)) {

			output->setY(centerY2 - (height / 2));
 			output->setX(centerX2 - (width / 2));
			return;
		}
	}
}

QDeclarativeContext* QMLOutputView::context() const
{
	QList< QGraphicsView* > views;
	QDeclarativeView *view;

	views = scene()->views();
	if (views.count() == 0) {
		kWarning() << "This view is not in any scene!";
		return 0;
	}

	view = dynamic_cast< QDeclarativeView* >(views.at(0));
	return view->rootContext();
}
