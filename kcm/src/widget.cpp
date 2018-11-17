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

#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QtGlobal>

#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "utils.h"
#include "scalingconfig.h"
#include "../../common/control.h"

#include <kscreen/output.h>
#include <kscreen/edid.h>
#include <kscreen/mode.h>
#include <kscreen/config.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/configmonitor.h>

#include <QDir>
#include <QStandardPaths>
#include <KLocalizedString>
#include <QComboBox>
#include <QPushButton>
#include <QQuickView>
#include <QQuickWidget>

#include "ui_kscreen_widget.h"

#define QML_PATH "kcm_kscreen/qml/"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KScreenWidget())
{
    qRegisterMetaType<QQuickView*>();

    ui->setupUi(this);
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    connect(ui->primaryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Widget::primaryOutputSelected);

    mControlPanel = new ControlPanel(this);
    connect(mControlPanel, &ControlPanel::changed,
            this, &Widget::changed);
    ui->controlPanelLayout->addWidget(mControlPanel);

    connect(ui->unifyButton, &QPushButton::released,
            [this]{
                slotUnifyOutputs();
            });

    connect(ui->scaleAllOutputsButton, &QPushButton::released,
            [this] {
                QPointer<ScalingConfig> dialog = new ScalingConfig(mConfig->outputs(), this);
                dialog->exec();
                delete dialog;
            });
    connect(this, &Widget::saveControls, mControlPanel, &ControlPanel::save);

    mOutputTimer = new QTimer(this);
    connect(mOutputTimer, &QTimer::timeout,
            this, &Widget::clearOutputIdentifiers);

    loadQml();
}

Widget::~Widget()
{
    clearOutputIdentifiers();
    delete ui;
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
        mConfig->disconnect(this);
    }

    mConfig = config;
    KScreen::ConfigMonitor::instance()->addConfig(mConfig);
    resetPrimaryCombo();
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, &Widget::outputAdded);
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, &Widget::outputRemoved);
    connect(mConfig.data(), &KScreen::Config::primaryOutputChanged,
            this, &Widget::primaryOutputChanged);

    mScreen->setConfig(mConfig);
    mControlPanel->setConfig(mConfig);
    ui->unifyButton->setEnabled(mConfig->outputs().count() > 1);
    ui->scaleAllOutputsButton->setVisible(!mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        outputAdded(output);
    }

    // Select the primary (or only) output by default
    QMLOutput *qmlOutput = mScreen->primaryOutput();
    if (qmlOutput) {
        mScreen->setActiveOutput(qmlOutput);
    } else {
        if (!mScreen->outputs().isEmpty()) {
            mScreen->setActiveOutput(mScreen->outputs().at(0));
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
    ui->quickWidget->setSource(QUrl::fromLocalFile(file));

    QQuickItem* rootObject = ui->quickWidget->rootObject();
    mScreen = rootObject->findChild<QMLScreen*>(QStringLiteral("outputView"));
    if (!mScreen) {
        return;
    }
    connect(mScreen, &QMLScreen::focusedOutputChanged,
            this, &Widget::slotFocusedOutputChanged);
    connect(rootObject->findChild<QObject*>(QStringLiteral("identifyButton")), SIGNAL(clicked()),
            this, SLOT(slotIdentifyButtonClicked()));
}

void Widget::resetPrimaryCombo()
{
    bool isPrimaryDisplaySupported = mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PrimaryDisplay);
    ui->primaryLabel->setVisible(isPrimaryDisplaySupported);
    ui->primaryCombo->setVisible(isPrimaryDisplaySupported);

    // Don't emit currentIndexChanged when resetting
    bool blocked = ui->primaryCombo->blockSignals(true);
    ui->primaryCombo->clear();
    ui->primaryCombo->addItem(i18n("No Primary Output"));
    ui->primaryCombo->blockSignals(blocked);

    if (!mConfig) {
        return;
    }

    for (auto &output: mConfig->outputs()) {
        addOutputToPrimaryCombo(output);
    }
}

void Widget::addOutputToPrimaryCombo(const KScreen::OutputPtr &output)
{
    if (!output->isConnected() || !output->isEnabled()) {
        return;
    }
    ui->primaryCombo->addItem(Utils::outputName(output), output->id());
    if (output->isPrimary()) {
        Q_ASSERT(mConfig);
        int lastIndex = ui->primaryCombo->count() - 1;
        ui->primaryCombo->setCurrentIndex(lastIndex);
    }
}

void Widget::slotFocusedOutputChanged(QMLOutput *output)
{
    mControlPanel->activateOutput(output->outputPtr());
}

void Widget::slotOutputEnabledChanged()
{
    resetPrimaryCombo();

    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
        if (enabledOutputsCount > 1) {
            break;
        }
    }
    ui->unifyButton->setEnabled(enabledOutputsCount > 1);
}

void Widget::slotOutputConnectedChanged()
{
    resetPrimaryCombo();
}

void Widget::slotUnifyOutputs()
{
    QMLOutput *base = mScreen->primaryOutput();
    QList<int> clones;

    if (!base) {
        for (QMLOutput *output: mScreen->outputs()) {
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

        ui->primaryCombo->setEnabled(true);
        ui->unifyButton->setText(i18n("Unify Outputs"));
    } else {
        // Clone the current config, so that we can restore it in case user
        // breaks the cloning
        mPrevConfig = mConfig->clone();

        for (QMLOutput *output: mScreen->outputs()) {
            if (!output->output()->isConnected()) {
                continue;
            }

            if (!output->output()->isEnabled()) {
                output->setVisible(false);
                continue;
            }

            if (!base) {
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

        ui->primaryCombo->setEnabled(false);
        mControlPanel->setUnifiedOutput(base->outputPtr());
        ui->unifyButton->setText(i18n("Break Unified Outputs"));
    }

    Q_EMIT changed();
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

void Widget::clearOutputIdentifiers()
{
    mOutputTimer->stop();
    qDeleteAll(mOutputIdentifiers);
    mOutputIdentifiers.clear();
}

void Widget::outputAdded(const KScreen::OutputPtr &output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &Widget::slotOutputConnectedChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged,
            this, &Widget::slotOutputEnabledChanged);
    connect(output.data(), &KScreen::Output::posChanged,
            this, &Widget::changed);

    addOutputToPrimaryCombo(output);
}

void Widget::outputRemoved(int outputId)
{
    KScreen::OutputPtr output = mConfig->output(outputId);
    if (!output.isNull()) {
        output->disconnect(this);
    }

    const int index = ui->primaryCombo->findData(outputId);
    if (index == -1) {
        return;
    }

    if (index == ui->primaryCombo->currentIndex()) {
        // We'll get the actual primary update signal eventually
        // Don't emit currentIndexChanged
        const bool blocked = ui->primaryCombo->blockSignals(true);
        ui->primaryCombo->setCurrentIndex(0);
        ui->primaryCombo->blockSignals(blocked);
    }
    ui->primaryCombo->removeItem(index);
}

void Widget::primaryOutputSelected(int index)
{
    if (!mConfig) {
        return;
    }

    const KScreen::OutputPtr newPrimary = index == 0 ? KScreen::OutputPtr() : mConfig->output(ui->primaryCombo->itemData(index).toInt());
    if (newPrimary == mConfig->primaryOutput()) {
        return;
    }

    mConfig->setPrimaryOutput(newPrimary);
    Q_EMIT changed();
}

void Widget::primaryOutputChanged(const KScreen::OutputPtr &output)
{
    Q_ASSERT(mConfig);
    int index = output.isNull() ? 0 : ui->primaryCombo->findData(output->id());
    if (index == -1 || index == ui->primaryCombo->currentIndex()) {
        return;
    }
    ui->primaryCombo->setCurrentIndex(index);
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

        QSize deviceSize, logicalSize;
        if (output->isHorizontal()) {
            deviceSize = mode->size();
        } else {
            deviceSize = QSize(mode->size().height(), mode->size().width());
        }
        if (config->supportedFeatures() & KScreen::Config::Feature::PerOutputScaling) {
            // no scale adjustment needed on Wayland
            logicalSize = deviceSize;
        } else {
            logicalSize = deviceSize / devicePixelRatioF();
        }

        rootObj->setProperty("outputName", Utils::outputName(output));
        rootObj->setProperty("modeName", Utils::sizeToString(deviceSize));
        view->setProperty("screenSize", QRect(output->pos(), logicalSize));
        mOutputIdentifiers << view;
    }

    for (QQuickView *view: mOutputIdentifiers) {
        view->show();
    }

    mOutputTimer->start(2500);
}
