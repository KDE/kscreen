/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "kcm.h"

#include "../common/control.h"
#include "../common/orientation_sensor.h"
#include "config_handler.h"
#include "globalscalesettings.h"
#include "kcm_screen_debug.h"
#include "kwincompositing_setting.h"

#include <kscreen/config.h>
#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/log.h>
#include <kscreen/mode.h>
#include <kscreen/output.h>
#include <kscreen/setconfigoperation.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QProcess>
#include <QSortFilterProxyModel>
#include <QTimer>

K_PLUGIN_CLASS_WITH_JSON(KCMKScreen, "kcm_kscreen.json")

using namespace KScreen;
class ScreenSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ScreenSortProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
    }
};

KCMKScreen::KCMKScreen(QObject *parent, const KPluginMetaData &data)
    : KQuickManagedConfigModule(parent, data)
{
    qmlRegisterUncreatableType<OutputModel>("org.kde.private.kcm.kscreen", 1, 0, "OutputModel", QStringLiteral("For enums"));
    qmlRegisterType<KScreen::Output>("org.kde.private.kcm.kscreen", 1, 0, "Output");
    qmlRegisterUncreatableType<KCMKScreen>("org.kde.private.kcm.kscreen", 1, 0, "KCMKScreen", QStringLiteral("For InvalidConfig enum"));
    Log::instance();

    setButtons(Apply);

    m_outputProxyModel = new ScreenSortProxyModel(this);

    m_loadCompressor = new QTimer(this);
    m_loadCompressor->setInterval(1000);
    m_loadCompressor->setSingleShot(true);
    connect(m_loadCompressor, &QTimer::timeout, this, &KCMKScreen::load);

    m_orientationSensor = new OrientationSensor(this);
    connect(m_orientationSensor, &OrientationSensor::availableChanged, this, &KCMKScreen::orientationSensorAvailableChanged);

    connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged, this, &KCMKScreen::updateFromBackend);

    registerSettings(GlobalScaleSettings::self());
    connect(GlobalScaleSettings::self(), &GlobalScaleSettings::scaleFactorChanged, this, &KCMKScreen::globalScaleChanged);

    registerSettings(KWinCompositingSetting::self());
    connect(KWinCompositingSetting::self(), &KWinCompositingSetting::allowTearingChanged, this, &KCMKScreen::tearingAllowedChanged);
}

void KCMKScreen::configReady(ConfigOperation *op)
{
    qCDebug(KSCREEN_KCM) << "Reading in config now.";
    if (op->hasError()) {
        m_configHandler.reset();
        m_configNeedsSave = false;
        settingsChanged();
        Q_EMIT backendError();
        return;
    }

    KScreen::ConfigPtr config = qobject_cast<GetConfigOperation *>(op)->config();
    const bool autoRotationSupported = config->supportedFeatures() & (KScreen::Config::Feature::AutoRotation | KScreen::Config::Feature::TabletMode);
    m_orientationSensor->setEnabled(autoRotationSupported);

    m_configHandler->setConfig(config);
    Q_EMIT multipleScreensAvailableChanged();

    setBackendReady(true);
    checkConfig();
    Q_EMIT perOutputScalingChanged();
    Q_EMIT xwaylandClientsScaleSupportedChanged();
    Q_EMIT tearingSupportedChanged();
    Q_EMIT primaryOutputSupportedChanged();
    Q_EMIT outputReplicationSupportedChanged();
    Q_EMIT tabletModeAvailableChanged();
    Q_EMIT autoRotationSupportedChanged();
}

void KCMKScreen::save()
{
    doSave();
}

void KCMKScreen::revertSettings()
{
    if (!m_configHandler || !m_configHandler->config()) {
        return;
    }
    if (!m_settingsReverted) {
        m_configHandler->revertConfig();
        m_settingsReverted = true;
        doSave();
        load(); // reload the configuration
        Q_EMIT settingsReverted();
        m_stopUpdatesFromBackend = false;
    }
}

void KCMKScreen::requestReboot()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.LogoutPrompt"),
                                                      QStringLiteral("/LogoutPrompt"),
                                                      QStringLiteral("org.kde.LogoutPrompt"),
                                                      QStringLiteral("promptReboot"));
    QDBusConnection::sessionBus().asyncCall(msg);
}

void KCMKScreen::setStopUpdatesFromBackend(bool value)
{
    m_stopUpdatesFromBackend = value;
}

void KCMKScreen::updateFromBackend()
{
    if (needsSave() || m_stopUpdatesFromBackend) {
        return;
    }

    m_loadCompressor->start();
}

void KCMKScreen::doSave()
{
    if (!m_configHandler || !m_configHandler->config()) {
        Q_EMIT errorOnSave(i18n("Implementation error"));
        return;
    }

    const auto outputs = m_configHandler->config()->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        KScreen::ModePtr mode = output->currentMode();
        qCDebug(KSCREEN_KCM) << output->name() << output->id() << output.data() << "\n"
                             << "\tConnected:" << output->isConnected() << "\n"
                             << "\tEnabled:" << output->isEnabled() << "\n"
                             << "\tPriority:" << output->priority() << "\n"
                             << "\tRotation:" << output->rotation() << "\n"
                             << "\tMode:" << (mode ? mode->name() : QStringLiteral("unknown")) << "@" << (mode ? mode->refreshRate() : 0.0) << "Hz"
                             << "\n"
                             << "\tPosition:" << output->pos().x() << "x" << output->pos().y() << "\n"
                             << "\tScale:" << (perOutputScaling() ? QString::number(output->scale()) : QStringLiteral("global")) << "\n"
                             << "\tReplicates:" << (output->replicationSource() == 0 ? "no" : "yes");
    }

    auto config = m_configHandler->config();

    if (!Config::canBeApplied(config)) {
        Q_EMIT errorOnSave(i18n("Implementation error"));
        m_configHandler->checkNeedsSave();
        return;
    }

    const bool globalScaleChanged = GlobalScaleSettings::self()->isSaveNeeded();
    KQuickManagedConfigModule::save();
    if (globalScaleChanged) {
        exportGlobalScale();
    }

    m_configHandler->prepareForSave();

    m_configHandler->writeControl();

    // Store the current config, apply settings. Block until operation is
    // completed, otherwise ConfigModule might terminate before we get to
    // execute the Operation.
    auto *op = new SetConfigOperation(config);
    m_stopUpdatesFromBackend = true;
    if (!op->exec()) {
        Q_EMIT errorOnSave(op->errorString());
        return;
    }

    // exec() opens a nested eventloop that may have unset m_configHandler if (e.g.)
    // outputs changed during saving. https://bugs.kde.org/show_bug.cgi?id=466960
    if (!m_configHandler || !m_configHandler->config()) {
        Q_EMIT errorOnSave(i18n("Outputs changed while trying to apply settings"));
        return;
    }

    const auto updateInitialData = [this]() {
        if (!m_configHandler || !m_configHandler->config()) {
            return;
        }
        m_configHandler->updateInitialData();

        if (!m_settingsReverted && m_configHandler->shouldTestNewSettings()) {
            Q_EMIT showRevertWarning();
        } else {
            m_settingsReverted = false;
            m_stopUpdatesFromBackend = false;
        }
    };

    if (m_configHandler->config()->supportedFeatures() & (KScreen::Config::Feature::SynchronousOutputChanges)) {
        updateInitialData();
    } else {
        // The 1000ms is a legacy value tested to work for randr having
        // enough time to change configuration.
        QTimer::singleShot(1000, this, updateInitialData);
    }

    if (m_needsKwinConfigReload) {
        m_needsKwinConfigReload = false;
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }
}

bool KCMKScreen::backendReady() const
{
    return m_backendReady;
}

void KCMKScreen::setBackendReady(bool ready)
{
    if (m_backendReady == ready) {
        return;
    }
    m_backendReady = ready;
    Q_EMIT backendReadyChanged();
}

QAbstractItemModel *KCMKScreen::outputModel() const
{
    return m_outputProxyModel;
}

void KCMKScreen::identifyOutputs()
{
    const QString name = QStringLiteral("org.kde.KWin");
    const QString interface = QStringLiteral("org.kde.KWin.Effect.OutputLocator1");
    const QString path = QStringLiteral("/org/kde/KWin/Effect/OutputLocator1");
    auto message = QDBusMessage::createMethodCall(name, path, interface, QStringLiteral("show"));
    QDBusConnection::sessionBus().send(message);
}

QSize KCMKScreen::normalizeScreen() const
{
    if (!m_configHandler) {
        return QSize();
    }
    return m_configHandler->normalizeScreen();
}

bool KCMKScreen::screenNormalized() const
{
    return m_screenNormalized;
}

bool KCMKScreen::perOutputScaling() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    return m_configHandler->config()->supportedFeatures().testFlag(Config::Feature::PerOutputScaling);
}

bool KCMKScreen::primaryOutputSupported() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    return m_configHandler->config()->supportedFeatures().testFlag(Config::Feature::PrimaryDisplay);
}

bool KCMKScreen::outputReplicationSupported() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    return m_configHandler->config()->supportedFeatures().testFlag(Config::Feature::OutputReplication);
}

bool KCMKScreen::autoRotationSupported() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    return m_configHandler->config()->supportedFeatures() & (KScreen::Config::Feature::AutoRotation | KScreen::Config::Feature::TabletMode);
}

bool KCMKScreen::orientationSensorAvailable() const
{
    return m_orientationSensor->available();
}

bool KCMKScreen::tabletModeAvailable() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    return m_configHandler->config()->tabletModeAvailable();
}

void KCMKScreen::setScreenNormalized(bool normalized)
{
    if (m_screenNormalized == normalized) {
        return;
    }
    m_screenNormalized = normalized;
    Q_EMIT screenNormalizedChanged();
}

void KCMKScreen::defaults()
{
    qCDebug(KSCREEN_KCM) << "Applying defaults.";
    load();
}

void KCMKScreen::load()
{
    qCDebug(KSCREEN_KCM) << "About to read in config.";

    KQuickManagedConfigModule::load();

    setBackendReady(false);
    m_configNeedsSave = false;
    settingsChanged();
    if (!screenNormalized()) {
        Q_EMIT screenNormalizedChanged();
    }

    // Don't pull away the outputModel under QML's feet
    // signal its disappearance first before deleting and replacing it.
    // We take the m_config pointer so outputModel() will return null,
    // gracefully cleaning up the QML side and only then we will delete it.
    auto *oldConfig = m_configHandler.release();
    if (oldConfig) {
        m_outputProxyModel->setSourceModel(nullptr);
        delete oldConfig;
    }

    m_configHandler.reset(new ConfigHandler(this));
    m_outputProxyModel->setSourceModel(m_configHandler->outputModel());

    Q_EMIT perOutputScalingChanged();
    Q_EMIT xwaylandClientsScaleSupportedChanged();
    Q_EMIT tearingSupportedChanged();
    Q_EMIT tearingAllowedChanged();

    connect(m_configHandler.get(), &ConfigHandler::outputModelChanged, this, [this]() {
        m_outputProxyModel->setSourceModel(m_configHandler->outputModel());
    });
    connect(m_configHandler.get(), &ConfigHandler::outputConnect, this, [this](bool connected) {
        Q_EMIT outputConnect(connected);
        setBackendReady(false);

        // Reload settings delayed such that daemon can update output values.
        m_loadCompressor->start();
    });
    connect(m_configHandler.get(), &ConfigHandler::screenNormalizationUpdate, this, &KCMKScreen::setScreenNormalized);

    // This is a queued connection so that we can fire the event from
    // within the save() call in case it failed.
    connect(m_configHandler.get(), &ConfigHandler::needsSaveChecked, this, &KCMKScreen::continueNeedsSaveCheck, Qt::QueuedConnection);

    connect(m_configHandler.get(), &ConfigHandler::changed, this, &KCMKScreen::changed);

    connect(new GetConfigOperation(), &GetConfigOperation::finished, this, &KCMKScreen::configReady);

    Q_EMIT changed();
}

void KCMKScreen::checkConfig()
{
    if (!m_configHandler || !m_configHandler->config()) {
        return;
    }

    const auto outputs = m_configHandler->config()->outputs();
    std::vector<OutputPtr> enabledOutputs;
    std::copy_if(outputs.cbegin(), outputs.cend(), std::back_inserter(enabledOutputs), std::mem_fn(&Output::isEnabled));
    if (enabledOutputs.empty()) {
        Q_EMIT invalidConfig(NoEnabledOutputs);
        m_configNeedsSave = false;
    }
    auto rectsTouch = [](const QRect &rect, const QRect &other) {
        return rect.left() <= other.left() + other.width() && other.left() <= rect.left() + rect.width() && rect.top() <= other.top() + other.height()
            && other.top() <= rect.top() + rect.height();
    };
    auto doesNotTouchAnyOther = [&enabledOutputs, &rectsTouch](const OutputPtr &output) {
        return std::none_of(enabledOutputs.cbegin(), enabledOutputs.cend(), [&output, &rectsTouch](const OutputPtr &other) {
            return other != output && rectsTouch(output->geometry(), other->geometry());
        });
    };
    if (enabledOutputs.size() > 1 && std::any_of(enabledOutputs.cbegin(), enabledOutputs.cend(), doesNotTouchAnyOther)) {
        Q_EMIT invalidConfig(ConfigHasGaps);
        m_configNeedsSave = false;
    }
}

void KCMKScreen::continueNeedsSaveCheck(bool needs)
{
    m_configNeedsSave = needs;

    if (needs) {
        checkConfig();
    }

    settingsChanged();
}

bool KCMKScreen::isSaveNeeded() const
{
    return m_configNeedsSave;
}

void KCMKScreen::exportGlobalScale()
{
    // Write env var to be used by session startup scripts to populate the QT_SCREEN_SCALE_FACTORS
    // env var.
    // We use QT_SCREEN_SCALE_FACTORS as opposed to QT_SCALE_FACTOR as we need to use one that will
    // NOT scale fonts according to the scale.
    // Scaling the fonts makes sense if you don't also set a font DPI, but we NEED to set a font
    // DPI for both PlasmaShell which does it's own thing, and for KDE4/GTK2 applications.
    QString screenFactors;
    const auto outputs = m_configHandler->config()->outputs();
    for (const auto &output : outputs) {
        screenFactors.append(output->name() + QLatin1Char('=') + QString::number(globalScale()) + QLatin1Char(';'));
    }
    auto config = KSharedConfig::openConfig("kdeglobals");
    config->group("KScreen").writeEntry("ScreenScaleFactors", screenFactors);
    config->sync();

    KConfig fontConfig(QStringLiteral("kcmfonts"));
    auto fontConfigGroup = fontConfig.group("General");

    if (qFuzzyCompare(globalScale(), 1.0)) {
        // if dpi is the default (96) remove the entry rather than setting it
        QProcess queryProc;
        queryProc.start(QStringLiteral("xrdb"), {QStringLiteral("-query")});
        if (queryProc.waitForFinished()) {
            QByteArray db = queryProc.readAllStandardOutput();
            int idx1 = 0;
            while (idx1 < db.size()) {
                int idx2 = db.indexOf('\n', idx1);
                if (idx2 == -1) {
                    idx2 = db.size() - 1;
                }
                const auto entry = QByteArray::fromRawData(db.constData() + idx1, idx2 - idx1 + 1);
                if (entry.startsWith("Xft.dpi:")) {
                    db.remove(idx1, entry.size());
                } else {
                    idx1 = idx2 + 1;
                }
            }

            QProcess loadProc;
            loadProc.start(QStringLiteral("xrdb"), {QStringLiteral("-quiet"), QStringLiteral("-load"), QStringLiteral("-nocpp")});
            if (loadProc.waitForStarted()) {
                loadProc.write(db);
                loadProc.closeWriteChannel();
                loadProc.waitForFinished();
            }
        }
        fontConfigGroup.writeEntry("forceFontDPI", 0, KConfig::Notify);
    } else {
        const int scaleDpi = qRound(globalScale() * 96.0);
        QProcess proc;
        proc.start(QStringLiteral("xrdb"), {QStringLiteral("-quiet"), QStringLiteral("-merge"), QStringLiteral("-nocpp")});
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi: ") + QByteArray::number(scaleDpi));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
        fontConfigGroup.writeEntry("forceFontDPI", scaleDpi, KConfig::Notify);
    }

    Q_EMIT globalScaleWritten();
}

qreal KCMKScreen::globalScale() const
{
    return GlobalScaleSettings::self()->scaleFactor();
}

void KCMKScreen::setGlobalScale(qreal scale)
{
    GlobalScaleSettings::self()->setScaleFactor(scale);
    Q_EMIT changed();
}

bool KCMKScreen::xwaylandClientsScale() const
{
    return GlobalScaleSettings::self()->xwaylandClientsScale();
}

void KCMKScreen::setXwaylandClientsScale(bool scale)
{
    GlobalScaleSettings::self()->setXwaylandClientsScale(scale);
    Q_EMIT changed();
}

bool KCMKScreen::xwaylandClientsScaleSupported() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    return m_configHandler->config()->supportedFeatures().testFlag(Config::Feature::XwaylandScales);
}

void KCMKScreen::setAllowTearing(bool allow)
{
    if (KWinCompositingSetting::self()->allowTearing() == allow) {
        return;
    }
    m_needsKwinConfigReload = true;
    KWinCompositingSetting::self()->setAllowTearing(allow);
    Q_EMIT changed();
}

bool KCMKScreen::allowTearing() const
{
    return KWinCompositingSetting::self()->allowTearing();
}

bool KCMKScreen::tearingSupported() const
{
    if (!m_configHandler || !m_configHandler->config()) {
        return false;
    }
    // == is Wayland
    return m_configHandler->config()->supportedFeatures().testFlag(Config::Feature::XwaylandScales);
}

bool KCMKScreen::multipleScreensAvailable() const
{
    return m_outputProxyModel->rowCount() > 1;
}

void KCMKScreen::startHdrCalibrator(const QString &outputName)
{
    QProcess::startDetached("hdrcalibrator", {outputName});
}

#include "kcm.moc"

#include "moc_kcm.cpp"
