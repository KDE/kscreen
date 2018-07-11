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

#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QtGlobal>

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

#include <QDir>
#include <QStandardPaths>
#include <KLocalizedString>
#include <QComboBox>
#include <QPushButton>
#include <QQuickView>
#include <QQuickWidget>

#define QML_PATH "kcm_kscreen/qml/"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    qRegisterMetaType<QQuickView*>();

    setMinimumHeight(550);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    mDeclarativeView = new QQuickWidget();
    mDeclarativeView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mDeclarativeView->setMinimumHeight(280);
    splitter->addWidget(mDeclarativeView);

    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    splitter->setStretchFactor(1, 1);
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    QVBoxLayout *vbox = new QVBoxLayout(widget);
    const int topMargin = style()->pixelMetric(QStyle::PM_LayoutTopMargin, nullptr, this);
    vbox->setContentsMargins(0, topMargin, 0, 0);
    widget->setLayout(vbox);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    mPrimaryCombo = new QComboBox(this);
    mPrimaryCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mPrimaryCombo->addItem(i18n("No Primary Output"));
    connect(mPrimaryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Widget::primaryOutputSelected);
    mPrimaryLabel = new QLabel(i18n("Primary display:"));
    hbox->addWidget(mPrimaryLabel);
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

    mScaleAllOutputsButton = new QPushButton(i18n("Scale Display"), this);
    connect(mScaleAllOutputsButton, &QPushButton::released,
            [this] {
                QPointer<ScalingConfig> dialog = new ScalingConfig(mConfig->outputs(), this);
                dialog->exec();
                delete dialog;
            });

    vbox->addWidget(mScaleAllOutputsButton);


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
    mUnifyButton->setEnabled(mConfig->outputs().count() > 1);
    mScaleAllOutputsButton->setVisible(!mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));

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

void Widget::resetPrimaryCombo()
{
    bool isPrimaryDisplaySupported = mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PrimaryDisplay);
    mPrimaryLabel->setVisible(isPrimaryDisplaySupported);
    mPrimaryCombo->setVisible(isPrimaryDisplaySupported);

    // Don't emit currentIndexChanged when resetting
    bool blocked = mPrimaryCombo->blockSignals(true);
    mPrimaryCombo->clear();
    mPrimaryCombo->addItem(i18n("No Primary Output"));
    mPrimaryCombo->blockSignals(blocked);

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
    mPrimaryCombo->addItem(Utils::outputName(output), output->id());
    if (output->isPrimary()) {
        Q_ASSERT(mConfig);
        int lastIndex = mPrimaryCombo->count() - 1;
        mPrimaryCombo->setCurrentIndex(lastIndex);
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
    mUnifyButton->setEnabled(enabledOutputsCount > 1);
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

        mPrimaryCombo->setEnabled(true);
        mUnifyButton->setText(i18n("Unify Outputs"));
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
    for (KScreen::Output: output, outputList) {
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

    const int index = mPrimaryCombo->findData(outputId);
    if (index == -1) {
        return;
    }

    if (index == mPrimaryCombo->currentIndex()) {
        // We'll get the actual primary update signal eventually
        // Don't emit currentIndexChanged
        const bool blocked = mPrimaryCombo->blockSignals(true);
        mPrimaryCombo->setCurrentIndex(0);
        mPrimaryCombo->blockSignals(blocked);
    }
    mPrimaryCombo->removeItem(index);
}

void Widget::primaryOutputSelected(int index)
{
    if (!mConfig) {
        return;
    }

    const KScreen::OutputPtr newPrimary = index == 0 ? KScreen::OutputPtr() : mConfig->output(mPrimaryCombo->itemData(index).toInt());
    if (newPrimary == mConfig->primaryOutput()) {
        return;
    }

    mConfig->setPrimaryOutput(newPrimary);
    Q_EMIT changed();
}

void Widget::primaryOutputChanged(const KScreen::OutputPtr &output)
{
    Q_ASSERT(mConfig);
    int index = output.isNull() ? 0 : mPrimaryCombo->findData(output->id());
    if (index == -1 || index == mPrimaryCombo->currentIndex()) {
        return;
    }
    mPrimaryCombo->setCurrentIndex(index);
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
