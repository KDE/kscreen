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
#include <QtCore/qglobal.h>

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
#include <KDebug>

Widget::Widget(QWidget *parent):
    QWidget(parent),
    mScreen(0),
    mConfig(0),
    mPrevConfig(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    m_declarativeView = new QDeclarativeView(this);
    m_declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    splitter->addWidget(m_declarativeView);
    splitter->setStretchFactor(0, 1);
    loadQml();

    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);

    QVBoxLayout *vbox = new QVBoxLayout(widget);
    widget->setLayout(vbox);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    mPrimaryCombo = new KComboBox(this);
    mPrimaryCombo->setSizeAdjustPolicy(QComboBox::QComboBox::AdjustToContents);
    mPrimaryCombo->addItem(i18n("No primary screen"));
    connect(mPrimaryCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotPrimaryChanged(int)));

    hbox->addWidget(new QLabel(i18n("Primary display:")));
    hbox->addWidget(mPrimaryCombo);

    hbox->addStretch();

    mProfilesCombo = new KComboBox(this);
    mProfilesCombo->addItem(i18n("Default Profile:"));
    mProfilesCombo->setEnabled(false);
    hbox->addWidget(new QLabel(i18n("Active profile")));
    hbox->addWidget(mProfilesCombo);


    m_controlPanel = new ControlPanel(this);
    connect(m_controlPanel, SIGNAL(changed()), this, SIGNAL(changed()));
    vbox->addWidget(m_controlPanel);

    mUnifyButton = new KPushButton(i18n("Unify outputs"), this);
    connect(mUnifyButton, SIGNAL(clicked(bool)), this, SLOT(slotUnifyOutputs()));
    vbox->addWidget(mUnifyButton);
}

Widget::~Widget()
{
}

void Widget::setConfig(KScreen::Config *config)
{
    if (mConfig) {
        Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
            disconnect(output, SIGNAL(isConnectedChanged()), this, SLOT(slotOutputConnectedChanged()));
            disconnect(output, SIGNAL(isEnabledChanged()), this, SLOT(slotOutputEnabledChanged()));
            disconnect(output, SIGNAL(isPrimaryChanged()), this, SLOT(slotOutputPrimaryChanged()));
        }

        delete mConfig;
    }

    mConfig = config;
    mScreen->setConfig(mConfig);
    m_controlPanel->setConfig(mConfig);
    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        connect(output, SIGNAL(isConnectedChanged()), this, SLOT(slotOutputConnectedChanged()));
        connect(output, SIGNAL(isEnabledChanged()), this, SLOT(slotOutputEnabledChanged()));
        connect(output, SIGNAL(isPrimaryChanged()), this, SLOT(slotOutputPrimaryChanged()));
    }

    initPrimaryCombo();
}

KScreen::Config *Widget::currentConfig() const
{
    return mConfig;
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
    mScreen = rootObject->findChild<QMLScreen*>(QLatin1String("outputView"));
    if (!mScreen) {
        return;
    }

    connect(mScreen, SIGNAL(focusedOutputChanged(QMLOutput*)),
            this, SLOT(slotFocusedOutputChanged(QMLOutput*)));
    connect(mScreen, SIGNAL(focusedOutputChanged(QMLOutput*)),
            this, SIGNAL(changed()));
}

void Widget::initPrimaryCombo()
{
    mPrimaryCombo->blockSignals(true);
    mPrimaryCombo->clear();
    mPrimaryCombo->addItem(i18n("No primary output"));

    Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }

        mPrimaryCombo->addItem(Utils::outputName(output), output->id());
        if (output->isPrimary()) {
            mPrimaryCombo->setCurrentIndex(mPrimaryCombo->count() - 1);
        }
    }
    mPrimaryCombo->blockSignals(false);
}


void Widget::slotFocusedOutputChanged(QMLOutput *output)
{
    m_controlPanel->activateOutput(output->output());
}

void Widget::slotOutputPrimaryChanged()
{
    const int id = qobject_cast<KScreen::Output*>(sender())->id();
    const int index = mPrimaryCombo->findData(id);
    if (index == -1) { // No primary
        mPrimaryCombo->setCurrentIndex(0);
        return;
    }
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

    Q_EMIT changed();
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

    KScreen::Output *output = qobject_cast<KScreen::Output*>(sender());
    if (output->isEnabled()) {
        mPrimaryCombo->addItem(Utils::outputName(output), output->id());
    } else {
        const int index = mPrimaryCombo->findData(output->id());
        mPrimaryCombo->removeItem(index);
    }

    mUnifyButton->setEnabled(enabledOutputsCnt > 1);
}

void Widget::slotUnifyOutputs()
{
    QMLOutput *base = mScreen->primaryOutput();
    QList<int> clones;

    if (!base) {
        Q_FOREACH (QMLOutput *output, mScreen->outputs()) {
            if (output->output()->isConnected() && output->output()->isEnabled()) {
                base = output;
                break;
            }
        }

        if (!base) {
            // WTF?
            return;
        }
    }

    if (base->isCloneMode()) {
        setConfig(mPrevConfig);
        mPrevConfig = 0;

        mPrimaryCombo->setEnabled(true);
        mUnifyButton->setText(i18n("Unify Outputs"));
    } else {
        // Clone the current config, so that we can restore it in case user
        // breaks the cloning
        if (mPrevConfig) {
            delete mPrevConfig;
        }

        mPrevConfig = mConfig->clone();

        Q_FOREACH (QMLOutput *output, mScreen->outputs()) {
            if (!output->output()->isConnected()) {
                continue;
            }

            if (!output->output()->isEnabled()) {
                output->hide();
                continue;
            }

            if (base == 0) {
                base = output;
            }

            output->setOutputX(0);
            output->setOutputY(0);
            output->output()->setPos(QPoint(0, 0));
            output->output()->setClones(QList<int>());

            if (base != output) {
                clones << output->output()->id();
                output->setCloneOf(base);
                output->hide();
            }
        }

        base->output()->setClones(clones);
        base->setIsCloneMode(true);

        mScreen->updateOutputsPlacement();

        mPrimaryCombo->setEnabled(false);
        m_controlPanel->setUnifiedOutput(base->output());

        mUnifyButton->setText(i18n("Break unified outputs"));
    }

    Q_EMIT changed();
}


#include "widget.moc"
