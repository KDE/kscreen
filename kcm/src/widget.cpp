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
#ifdef WITH_PROFILES
#include "profilesmodel.h"
#endif

#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QtCore/qglobal.h>

#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "utils.h"

#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/mode.h>
#include <kscreen/config.h>

#include <QtCore/QDir>
#include <QStandardPaths>
#include <KLocalizedString>
#include <QComboBox>
#include <QPushButton>
#include <QQuickView>

#define QML_PATH "kcm_kscreen/qml/"

Widget::Widget(QWidget *parent):
    QWidget(parent),
    mScreen(0),
    mConfig(0),
    mPrevConfig(0)
{
    qRegisterMetaType<QQuickView*>();

    setMinimumHeight(550);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    m_declarativeView = new QQuickView();
    QWidget *container = QWidget::createWindowContainer(m_declarativeView, this);
    m_declarativeView->setResizeMode(QQuickView::SizeRootObjectToView);
    m_declarativeView->setMinimumHeight(280);
    container->setMinimumHeight(280);
    splitter->addWidget(container);

    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *vbox = new QVBoxLayout(widget);
    widget->setLayout(vbox);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    mPrimaryCombo = new QComboBox(this);
    mPrimaryCombo->setSizeAdjustPolicy(QComboBox::QComboBox::AdjustToContents);
    mPrimaryCombo->addItem(i18n("No primary screen"));
    connect(mPrimaryCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotPrimaryChanged(int)));

    hbox->addWidget(new QLabel(i18n("Primary display:")));
    hbox->addWidget(mPrimaryCombo);

    hbox->addStretch();

#ifdef WITH_PROFILES
    mProfilesModel = new ProfilesModel(this);

    connect(mProfilesModel, SIGNAL(modelUpdated()),
            this, SLOT(slotProfilesUpdated()));
    mProfilesCombo = new QComboBox(this);
    mProfilesCombo->setModel(mProfilesModel);
    mProfilesCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    hbox->addWidget(new QLabel(i18n("Active profile")));
    hbox->addWidget(mProfilesCombo);
#endif

    m_controlPanel = new ControlPanel(this);
    connect(m_controlPanel, SIGNAL(changed()), this, SIGNAL(changed()));
    vbox->addWidget(m_controlPanel);

    mUnifyButton = new QPushButton(i18n("Unify outputs"), this);
    connect(mUnifyButton, SIGNAL(clicked(bool)), this, SLOT(slotUnifyOutputs()));
    vbox->addWidget(mUnifyButton);

    mOutputTimer = new QTimer(this);
    connect(mOutputTimer, SIGNAL(timeout()), SLOT(clearOutputIdentifiers()));

    loadQml();
}

Widget::~Widget()
{
    clearOutputIdentifiers();
}

bool Widget::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::Resize) {
        if (mOutputIdentifiers.contains(qobject_cast<QQuickView*>(object))) {
            QResizeEvent *e = static_cast<QResizeEvent*>(event);
            const QRect screenSize = object->property("screenSize").toRect();
            QRect geometry(QPoint(0, 0), e->size());
            geometry.moveCenter(screenSize.center());
            static_cast<QQuickView*>(object)->setGeometry(geometry);
            // Pass the event further
        }
    }

    return QObject::eventFilter(object, event);
}


void Widget::setConfig(KScreen::Config *config)
{
    if (mConfig) {
        Q_FOREACH (KScreen::Output *output, mConfig->outputs()) {
            disconnect(output, SIGNAL(isConnectedChanged()), this, SLOT(slotOutputConnectedChanged()));
            disconnect(output, SIGNAL(isEnabledChanged()), this, SLOT(slotOutputEnabledChanged()));
            disconnect(output, SIGNAL(isPrimaryChanged()), this, SLOT(slotOutputPrimaryChanged()));
            disconnect(output, SIGNAL(posChanged()), this, SIGNAL(changed()));
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
        connect(output, SIGNAL(posChanged()), this, SIGNAL(changed()));
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

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    //const QString file = QDir::currentPath() + "/main.qml";
    const QString file = QStandardPaths::locate(QStandardPaths::QStandardPaths::GenericDataLocation, QStringLiteral("kcm_kscreen/qml/main.qml"));
    m_declarativeView->setSource(QUrl::fromLocalFile(file));

    QQuickItem* rootObject = m_declarativeView->rootObject();
    mScreen = rootObject->findChild<QMLScreen*>(QLatin1String("outputView"));
    if (!mScreen) {
        return;
    }
    mScreen->setEngine(m_declarativeView->engine());

    connect(mScreen, SIGNAL(focusedOutputChanged(QMLOutput*)),
            this, SLOT(slotFocusedOutputChanged(QMLOutput*)));
    connect(rootObject->findChild<QObject*>("identifyButton"), SIGNAL(clicked()),
            this, SLOT(slotIdentifyOutputs()));


#ifndef WITH_PROFILES
    setConfig(KScreen::Config::current());
#endif
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
                output->setVisible(false);
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
                output->setVisible(false);
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

void Widget::slotProfileChanged(int index)
{
#ifdef WITH_PROFILES
    const QVariantMap profile = mProfilesCombo->itemData(index, ProfilesModel::ProfileRole).toMap();
    const QVariantList outputs = profile[QLatin1String("outputs")].toList();

    // FIXME: Copy-pasted from KDED's Serializer::config()
    KScreen::Config *config = KScreen::Config::current();
    KScreen::OutputList outputList = config->outputs();
    Q_FOREACH(KScreen::Output * output, outputList) {
        if (!output->isConnected() && output->isEnabled()) {
            output->setEnabled(false);
        }
    }

    KScreen::Config *outputsConfig = config->clone();
    Q_FOREACH(const QVariant & info, outputs) {
        KScreen::Output *output = findOutput(outputsConfig, info.toMap());
        if (!output) {
            continue;
        }

        delete outputList.take(output->id());
        outputList.insert(output->id(), output);
    }

    config->setOutputs(outputList);

    setConfig(config);
#endif
}

// FIXME: Copy-pasted from KDED's Serializer::findOutput()
KScreen::Output *Widget::findOutput(KScreen::Config *config, const QVariantMap &info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::Output * output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        const QString outputId = (output->edid() && output->edid()->isValid()) ? output->edid()->hash() : output->name();
        if (outputId != info["id"].toString()) {
            continue;
        }

        QVariantMap posInfo = info["pos"].toMap();
        QPoint point(posInfo["x"].toInt(), posInfo["y"].toInt());
        output->setPos(point);
        output->setPrimary(info["primary"].toBool());
        output->setEnabled(info["enabled"].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info["rotation"].toInt()));

        QVariantMap modeInfo = info["mode"].toMap();
        QVariantMap modeSize = modeInfo["size"].toMap();
        QSize size(modeSize["width"].toInt(), modeSize["height"].toInt());

        KScreen::ModeList modes = output->modes();
        Q_FOREACH(KScreen::Mode * mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate()) != modeInfo["refresh"].toString()) {
                continue;
            }

            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

    return 0;
}

void Widget::slotProfilesAboutToUpdate()
{
#ifdef WITH_PROFILES
    disconnect(mProfilesCombo, SIGNAL(currentIndexChanged(int)),
               this, SLOT(slotProfileChanged(int)));
#endif
}


void Widget::slotProfilesUpdated()
{
#ifdef WITH_PROFILES
    connect(mProfilesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotProfileChanged(int)));

    const int index = mProfilesModel->activeProfileIndex();
    mProfilesCombo->setCurrentIndex(index);
#endif
}


void Widget::clearOutputIdentifiers()
{
    mOutputTimer->stop();
    qDeleteAll(mOutputIdentifiers);
    mOutputIdentifiers.clear();
}

void Widget::slotIdentifyOutputs()
{
    const QString qmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String(QML_PATH "OutputIdentifier.qml"));

    mOutputTimer->stop();
    clearOutputIdentifiers();

    /* Obtain the current active configuration from KScreen */
    KScreen::OutputList outputs = KScreen::Config::current()->outputs();
    Q_FOREACH (KScreen::Output *output, outputs) {
        if (!output->isConnected() || !output->currentMode()) {
            continue;
        }

        KScreen::Mode *mode = output->currentMode();

        QQuickView *view = new QQuickView();

        view->setFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
        view->setResizeMode(QQuickView::SizeViewToRootObject);
        view->setSource(QUrl::fromLocalFile(qmlPath));
        view->installEventFilter(this);

        QQuickItem *rootObj = view->rootObject();
        if (!rootObj) {
            qWarning() << "Failed to obtain root item";
            continue;
        }
        QSize realSize;
        if (output->isHorizontal()) {
            realSize = mode->size();
        } else {
            realSize = QSize(mode->size().height(), mode->size().width());
        }
        rootObj->setProperty("outputName", Utils::outputName(output));
        rootObj->setProperty("modeName", Utils::sizeToString(realSize));
        view->setProperty("screenSize", QRect(output->pos(), realSize));
        mOutputIdentifiers << view;
    }

    Q_FOREACH (QQuickView *view, mOutputIdentifiers) {
        view->show();
    }

    mOutputTimer->start(2500);
}

#include "widget.moc"
