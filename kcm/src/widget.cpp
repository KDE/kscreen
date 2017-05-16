/*
 * Copyright (C) 2013  Daniel Vr??til <dvratil@redhat.com>
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
#include "primaryoutputcombo.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QtCore/qglobal.h>

#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "utils.h"
#include "scalingconfig.h"

#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/mode.h>
#include <kscreen/config.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/configmonitor.h>

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

    mDeclarativeView = new QQuickView();
    QWidget *container = QWidget::createWindowContainer(mDeclarativeView, this);
    mDeclarativeView->setResizeMode(QQuickView::SizeRootObjectToView);
    mDeclarativeView->setMinimumHeight(280);
    container->setMinimumHeight(280);
    splitter->addWidget(container);

    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    splitter->setStretchFactor(1, 1);
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QVBoxLayout *vbox = new QVBoxLayout(widget);
    const int topMargin = style()->pixelMetric(QStyle::PM_LayoutTopMargin, 0, this);
    vbox->setContentsMargins(0, topMargin, 0, 0);
    widget->setLayout(vbox);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    mPrimaryCombo = new PrimaryOutputCombo(this);
    connect(mPrimaryCombo, &PrimaryOutputCombo::changed,
            this, &Widget::changed);
    hbox->addWidget(new QLabel(i18n("Primary display:")));
    hbox->addWidget(mPrimaryCombo);

    hbox->addStretch();

#ifdef WITH_PROFILES
    mProfilesModel = new ProfilesModel(this);

    connect(mProfilesModel, &ProfilesModel::modelUpdated()),
            this, &Widget::slotProfilesUpdated);
    mProfilesCombo = new QComboBox(this);
    mProfilesCombo->setModel(mProfilesModel);
    mProfilesCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    hbox->addWidget(new QLabel(i18n("Active profile")));
    hbox->addWidget(mProfilesCombo);
#endif

    mControlPanel = new ControlPanel(this);
    connect(mControlPanel, &ControlPanel::changed,
            this, &Widget::changed);
    vbox->addWidget(mControlPanel);

    mUnifyButton = new QPushButton(i18n("Unify Outputs"), this);
    connect(mUnifyButton, &QPushButton::released,
            [this]{
                slotUnifyOutputs();
            });

    vbox->addWidget(mUnifyButton);

    auto setScaleButton = new QPushButton(i18n("Scale Display"), this);
    connect(setScaleButton, &QPushButton::released,
            [this] {
                QPointer<ScalingConfig> dialog = new ScalingConfig(mConfig->outputs(), this);
                dialog->exec();
                delete dialog;
            });
    
    vbox->addWidget(setScaleButton);

    mOutputTimer = new QTimer(this);
    connect(mOutputTimer, &QTimer::timeout,
            this, &Widget::clearOutputIdentifiers);

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


void Widget::setConfig(const KScreen::ConfigPtr &config)
{
    if (mConfig) {
        KScreen::ConfigMonitor::instance()->removeConfig(mConfig);
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            output->disconnect(this);
        }
    }

    mConfig = config;
    KScreen::ConfigMonitor::instance()->addConfig(mConfig);

    mScreen->setConfig(mConfig);
    mControlPanel->setConfig(mConfig);
    mPrimaryCombo->setConfig(mConfig);
    mUnifyButton->setEnabled(mConfig->outputs().count() > 1);

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        connect(output.data(), &KScreen::Output::isEnabledChanged,
                this, &Widget::slotOutputEnabledChanged);
        connect(output.data(), &KScreen::Output::posChanged,
                this, &Widget::changed);
    }

    // Select the primary (or only) output by default
    QMLOutput *qmlOutput = mScreen->primaryOutput();
    if (qmlOutput) {
        mScreen->setActiveOutput(qmlOutput);
    } else {
        if (mScreen->outputs().count() > 0) {
            mScreen->setActiveOutput(mScreen->outputs()[0]);
        }
    }

    slotOutputEnabledChanged();
}

KScreen::ConfigPtr Widget::currentConfig() const
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
    mDeclarativeView->setSource(QUrl::fromLocalFile(file));

    QQuickItem* rootObject = mDeclarativeView->rootObject();
    mScreen = rootObject->findChild<QMLScreen*>(QStringLiteral("outputView"));
    if (!mScreen) {
        return;
    }
    mScreen->setEngine(mDeclarativeView->engine());

    connect(mScreen, &QMLScreen::focusedOutputChanged,
            this, &Widget::slotFocusedOutputChanged);
    connect(rootObject->findChild<QObject*>(QStringLiteral("identifyButton")), SIGNAL(clicked()),
            this, SLOT(slotIdentifyButtonClicked()));
}

void Widget::slotFocusedOutputChanged(QMLOutput *output)
{
    mControlPanel->activateOutput(output->outputPtr());
}

void Widget::slotOutputEnabledChanged()
{
    int enabledOutputsCnt = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
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
        mPrevConfig.clear();

        mPrimaryCombo->setEnabled(true);
        mUnifyButton->setText(i18n("Unify Outputs"));
    } else {
        // Clone the current config, so that we can restore it in case user
        // breaks the cloning
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
        mControlPanel->setUnifiedOutput(base->outputPtr());

        mUnifyButton->setText(i18n("Break Unified Outputs"));
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
#else
    Q_UNUSED(index)
#endif
}

// FIXME: Copy-pasted from KDED's Serializer::findOutput()
KScreen::OutputPtr Widget::findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        const QString outputId = (output->edid() && output->edid()->isValid()) ? output->edid()->hash() : output->name();
        if (outputId != info[QStringLiteral("id")].toString()) {
            continue;
        }

        QVariantMap posInfo = info[QStringLiteral("pos")].toMap();
        QPoint point(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
        output->setPos(point);
        output->setPrimary(info[QStringLiteral("primary")].toBool());
        output->setEnabled(info[QStringLiteral("enabled")].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info[QStringLiteral("rotation")].toInt()));

        QVariantMap modeInfo = info[QStringLiteral("mode")].toMap();
        QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
        QSize size(modeSize[QStringLiteral("width")].toInt(), modeSize[QStringLiteral("height")].toInt());

        const KScreen::ModeList modes = output->modes();
        Q_FOREACH(const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate()) != modeInfo[QStringLiteral("refresh")].toString()) {
                continue;
            }

            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

    return KScreen::OutputPtr();
}

void Widget::slotProfilesAboutToUpdate()
{
#ifdef WITH_PROFILES
    disconnect(mProfilesCombo, &QComboBox::currentIndexChanged,
               this, &Widget::slotProfileChanged);
#endif
}

void Widget::slotProfilesUpdated()
{
#ifdef WITH_PROFILES
    connect(mProfilesCombo, &QComboBox::currentIndexChanged,
            this, &Widget::slotProfileChanged);

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

void Widget::slotIdentifyButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &Widget::slotIdentifyOutputs);
}

void Widget::slotIdentifyOutputs(KScreen::ConfigOperation *op)
{
    if (op->hasError()) {
        return;
    }

    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    const QString qmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral(QML_PATH "OutputIdentifier.qml"));

    mOutputTimer->stop();
    clearOutputIdentifiers();

    /* Obtain the current active configuration from KScreen */
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (!output->isConnected() || !output->currentMode()) {
            continue;
        }

        const KScreen::ModePtr mode = output->currentMode();

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
