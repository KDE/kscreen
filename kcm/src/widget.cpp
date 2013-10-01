/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "widget.h"
#include "controlpanel.h"

#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeEngine>

#include <QtGui/QVBoxLayout>
#include <QtGui/QSplitter>
#include <QtGui/QLabel>

#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "declarative/iconbutton.h"
#include "utils.h"

#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/mode.h>
#include <kscreen/config.h>
#include <QtCore/QDir>
#include <KLocalizedString>
#include <KComboBox>
#include <KPushButton>

Widget::Widget(QWidget *parent):
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *hbox = new QHBoxLayout;
    layout->addLayout(hbox);


    mPrimaryCombo = new KComboBox(this);
    mPrimaryCombo->setSizeAdjustPolicy(QComboBox::QComboBox::AdjustToContents);
    hbox->addWidget(new QLabel(i18n("Primary display:")));
    hbox->addWidget(mPrimaryCombo);

    hbox->addStretch();

    mProfilesCombo = new KComboBox(this);
    mProfilesCombo->addItem(i18n("Default Profile:"));
    mProfilesCombo->setEnabled(false);
    hbox->addWidget(new QLabel(i18n("Active profile")));
    hbox->addWidget(mProfilesCombo);


    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    m_declarativeView = new QDeclarativeView(this);
    m_declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    splitter->addWidget(m_declarativeView);
    splitter->setStretchFactor(0, 1);
    loadQml();

    m_controlPanel = new ControlPanel(mConfig, this);
    splitter->addWidget(m_controlPanel);

    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        connect(output, SIGNAL(isConnectedChanged()), this, SLOT(slotOutputConnectedChanged()));
        connect(output, SIGNAL(isEnabledChanged()), this, SLOT(slotOutputEnabledChanged()));
        connect(output, SIGNAL(isPrimaryChanged()), this, SLOT(slotOutputPrimaryChanged()));

        if (!output->isConnected()) {
            continue;
        }

        mPrimaryCombo->addItem(Utils::outputName(output), output->id());
        if (output->isPrimary()) {
            mPrimaryCombo->setCurrentIndex(mPrimaryCombo->count() - 1);
        }
    }

    connect(mPrimaryCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotPrimaryChanged(int)));

    hbox = new QHBoxLayout;
    layout->addLayout(hbox);

    mUnifyButton = new KPushButton(i18n("Unify outputs"), this);
    connect(mUnifyButton, SIGNAL(clicked(bool)), this, SLOT(slotUnifyOutputs()));
    hbox->addWidget(mUnifyButton);
}

Widget::~Widget()
{
}

void Widget::loadQml()
{
    qmlRegisterType<QMLOutput>("org.kde.kscreen", 1, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 1, 0, "QMLScreen");
    qmlRegisterType<IconButton>("org.kde.kscreen", 1, 0, "IconButton");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    QDir dir = QDir::current();
    dir.cdUp();
    dir.cdUp();
    dir.cd("kcm/qml");
    QDir::setCurrent(dir.path());

    const QString file = QDir::currentPath() + "/main.qml";
    m_declarativeView->engine()->addImportPath(QLatin1String("/usr/lib64/kde4/imports/"));
    m_declarativeView->setSource(QUrl::fromLocalFile(file));

    QGraphicsObject *rootObject = m_declarativeView->rootObject();
    QMLScreen *screen = rootObject->findChild<QMLScreen*>(QLatin1String("outputView"));
    if (!screen) {
        return;
    }

    connect(screen, SIGNAL(focusedOutputChanged(QMLOutput*)),
            this, SLOT(slotFocusedOutputChanged(QMLOutput*)));

    mConfig = screen->config();
}

void Widget::slotFocusedOutputChanged(QMLOutput *output)
{
    m_controlPanel->activateOutput(output->output());
}

void Widget::slotOutputPrimaryChanged()
{
    const int id = qobject_cast<KScreen::Output*>(sender())->id();
    const int index = mPrimaryCombo->findData(id);
    mPrimaryCombo->blockSignals(true);
    mPrimaryCombo->setCurrentIndex(index);
    mPrimaryCombo->blockSignals(false);
}

void Widget::slotPrimaryChanged(int index)
{
    const int id = mPrimaryCombo->itemData(index).toInt();
    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        output->blockSignals(true);
        output->setPrimary(output->id() == id);
        output->blockSignals(false);
    }
}

void Widget::slotOutputConnectedChanged()
{
    KScreen::Output *output = qobject_cast<KScreen::Output*>(sender());
    if (output->isConnected()) {
        mPrimaryCombo->addItem(Utils::outputName(output), output->id());
        if (output->isPrimary()) {
            mPrimaryCombo->setCurrentIndex(mPrimaryCombo->count() - 1);
        }
    } else {
        const int index = mPrimaryCombo->findData(output->id());
        mPrimaryCombo->removeItem(index);
    }
}

void Widget::slotOutputEnabledChanged()
{
    int enabledOutputsCnt = 0;
    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCnt;
        }

        if (enabledOutputsCnt > 1) {
            break;
        }
    }

    mUnifyButton->setEnabled(enabledOutputsCnt > 1);
}

void Widget::slotUnifyOutputs()
{
    KScreen::Output *base = 0;
    QList<int> clones;
    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }

        if (base == 0) {
            base = output;
            continue;
        }

        clones << output->id();
        output->setPos(QPoint(0, 0));
    }

    base->setClones(clones);
}


#include "widget.moc"
