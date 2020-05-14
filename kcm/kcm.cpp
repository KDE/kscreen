/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "kcm.h"

#include "config_handler.h"
#include "kcm_screen_debug.h"
#include "output_identifier.h"
#include "output_model.h"
#include "../common/control.h"
#include "../common/orientation_sensor.h"

#include <kscreen/config.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/log.h>
#include <kscreen/output.h>
#include <kscreen/setconfigoperation.h>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KSharedConfig>

#include <QTimer>

K_PLUGIN_FACTORY_WITH_JSON(KCMDisplayConfigurationFactory, "kcm_kscreen.json",
                           registerPlugin<KCMKScreen>();)

using namespace KScreen;

KCMKScreen::KCMKScreen(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
{
    qmlRegisterType<OutputModel>();
    qmlRegisterType<KScreen::Output>("org.kde.private.kcm.kscreen",
                                     1, 0,
                                     "Output");
    qmlRegisterUncreatableType<Control>("org.kde.private.kcm.kscreen",
                                        1, 0, "Control",
            QStringLiteral("Provides only the OutputRetention enum class"));
    Log::instance();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_kscreen"),
                                       i18n("Display Configuration"),
                                       QStringLiteral(KSCREEN_VERSION),
                                       i18n("Manage and configure monitors and displays"),
                                       KAboutLicense::GPL,
                                       i18n("Copyright © 2019 Roman Gilg"));
    about->addAuthor(i18n("Roman Gilg"),
                    i18n("Developer"),
                    QStringLiteral("subdiff@gmail.com"));
    setAboutData(about);
    setButtons(Apply);

    m_loadCompressor = new QTimer(this);
    m_loadCompressor->setInterval(1000);
    m_loadCompressor->setSingleShot(true);
    connect (m_loadCompressor, &QTimer::timeout, this, &KCMKScreen::load);

    m_orientationSensor = new OrientationSensor(this);
    connect(m_orientationSensor, &OrientationSensor::availableChanged,
            this, &KCMKScreen::orientationSensorAvailableChanged);
}

void KCMKScreen::configReady(ConfigOperation *op)
{
    qCDebug(KSCREEN_KCM) << "Reading in config now.";
    if (op->hasError()) {
        m_config.reset();
        Q_EMIT backendError();
        return;
    }

    KScreen::ConfigPtr config = qobject_cast<GetConfigOperation*>(op)->config();
    const bool autoRotationSupported =
            config->supportedFeatures() & (KScreen::Config::Feature::AutoRotation
                                           | KScreen::Config::Feature::TabletMode);
    m_orientationSensor->setEnabled(autoRotationSupported);

    m_config->setConfig(config);
    setBackendReady(true);
    Q_EMIT perOutputScalingChanged();
    Q_EMIT primaryOutputSupportedChanged();
    Q_EMIT outputReplicationSupportedChanged();
    Q_EMIT tabletModeAvailableChanged();
    Q_EMIT autoRotationSupportedChanged();
}

void KCMKScreen::forceSave()
{
    doSave(true);
}

void KCMKScreen::save()
{
    doSave(false);
}

void KCMKScreen::doSave(bool force)
{
    if (!m_config) {
        Q_EMIT errorOnSave();
        return;
    }

    auto config = m_config->config();

    bool atLeastOneEnabledOutput = false;
    for (const KScreen::OutputPtr &output : config->outputs()) {
        KScreen::ModePtr mode = output->currentMode();

        atLeastOneEnabledOutput |= output->isEnabled();

        qCDebug(KSCREEN_KCM) << output->name() << output->id()
                             << output.data() << "\n"
                << "	Connected:" << output->isConnected() << "\n"
                << "	Enabled:" << output->isEnabled() << "\n"
                << "	Primary:" << output->isPrimary() << "\n"
                << "	Rotation:" << output->rotation() << "\n"
                << "	Mode:"
                << (mode ? mode->name() : QStringLiteral("unknown"))
                << "@" << (mode ? mode->refreshRate() : 0.0) << "Hz" << "\n"
                << "    Position:" << output->pos().x() << "x" << output->pos().y() << "\n"
                << "    Scale:" << (perOutputScaling() ? QString::number(output->scale()) :
                                                         QStringLiteral("global")) << "\n"
                << "    Replicates:" << (output->replicationSource() == 0 ? "no" : "yes");
    }

    if (!atLeastOneEnabledOutput && !force) {
        Q_EMIT dangerousSave();
        m_config->checkNeedsSave();
        return;
    }

    if (!Config::canBeApplied(config)) {
        Q_EMIT errorOnSave();
        m_config->checkNeedsSave();
        return;
    }

    if (!perOutputScaling()) {
        writeGlobalScale();
    }
    m_config->writeControl();

    // Store the current config, apply settings. Block until operation is
    // completed, otherwise ConfigModule might terminate before we get to
    // execute the Operation.
    auto *op = new SetConfigOperation(config);
    op->exec();

    // The 1000ms is a legacy value tested to work for randr having
    // enough time to change configuration.
    QTimer::singleShot(1000, this,
        [this] () {
            if (!m_config) {
                setNeedsSave(false);
                return;
            }
            m_config->updateInitialData();
        }
    );
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

OutputModel* KCMKScreen::outputModel() const
{
    if (!m_config) {
        return nullptr;
    }
    return m_config->outputModel();
}

void KCMKScreen::identifyOutputs()
{
    if (!m_config || !m_config->initialConfig() || m_outputIdentifier) {
        return;
    }
    m_outputIdentifier.reset(new OutputIdentifier(m_config->initialConfig(), this));
    connect(m_outputIdentifier.get(), &OutputIdentifier::identifiersFinished,
            this, [this]() {
        m_outputIdentifier.reset();
    });

}

QSize KCMKScreen::normalizeScreen() const
{
    if (!m_config) {
        return QSize();
    }
    return m_config->normalizeScreen();
}

bool KCMKScreen::screenNormalized() const
{
    return m_screenNormalized;
}

bool KCMKScreen::perOutputScaling() const
{
    if (!m_config || !m_config->config()) {
        return false;
    }
    return m_config->config()->supportedFeatures().testFlag(Config::Feature::
                                                            PerOutputScaling);
}

bool KCMKScreen::primaryOutputSupported() const
{
    if (!m_config || !m_config->config()) {
        return false;
    }
    return m_config->config()->supportedFeatures().testFlag(Config::Feature::
                                                            PrimaryDisplay);
}

bool KCMKScreen::outputReplicationSupported() const
{
    if (!m_config || !m_config->config()) {
        return false;
    }
    return m_config->config()->supportedFeatures().testFlag(Config::Feature::
                                                            OutputReplication);
}

bool KCMKScreen::autoRotationSupported() const
{
    if (!m_config || !m_config->config()) {
        return false;
    }
    return m_config->config()->supportedFeatures() & (KScreen::Config::Feature::AutoRotation
                                                      | KScreen::Config::Feature::TabletMode);
}

bool KCMKScreen::orientationSensorAvailable() const
{
    return m_orientationSensor->available();
}

bool KCMKScreen::tabletModeAvailable() const
{
    if (!m_config || !m_config->config()) {
        return false;
    }
    return m_config->config()->tabletModeAvailable();
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

    setBackendReady(false);
    setNeedsSave(false);
    if (!screenNormalized()) {
        Q_EMIT screenNormalizedChanged();
    }
    fetchGlobalScale();

    // Don't pull away the outputModel under QML's feet
    // signal its disappearance first before deleting and replacing it.
    // We take the m_config pointer so outputModel() will return null,
    // gracefully cleaning up the QML side and only then we will delete it.
    auto *oldConfig = m_config.release();
    if (oldConfig) {
        emit outputModelChanged();
        delete oldConfig;
    }

    m_config.reset(new ConfigHandler(this));
    Q_EMIT perOutputScalingChanged();
    connect (m_config.get(), &ConfigHandler::outputModelChanged,
             this, &KCMKScreen::outputModelChanged);
    connect (m_config.get(), &ConfigHandler::outputConnect,
             this, [this](bool connected) {

        Q_EMIT outputConnect(connected);
        setBackendReady(false);

        // Reload settings delayed such that daemon can update output values.
        m_loadCompressor->start();
    });
    connect (m_config.get(), &ConfigHandler::screenNormalizationUpdate,
             this, &KCMKScreen::setScreenNormalized);
    connect (m_config.get(), &ConfigHandler::retentionChanged,
             this, &KCMKScreen::outputRetentionChanged);

    // This is a queued connection so that we can fire the event from
    // within the save() call in case it failed.
    connect (m_config.get(), &ConfigHandler::needsSaveChecked,
             this, &KCMKScreen::continueNeedsSaveCheck, Qt::QueuedConnection);

    connect (m_config.get(), &ConfigHandler::changed,
             this, &KCMKScreen::changed);

    connect(new GetConfigOperation(), &GetConfigOperation::finished,
            this, &KCMKScreen::configReady);

    Q_EMIT changed();
}

void KCMKScreen::continueNeedsSaveCheck(bool needs)
{
    if (needs || m_globalScale != m_initialGlobalScale) {
        setNeedsSave(true);
    } else {
        setNeedsSave(false);
    }
}

void KCMKScreen::fetchGlobalScale()
{
    const auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    const qreal scale = config->group("KScreen").readEntry("ScaleFactor", 1.0);
    m_initialGlobalScale = scale;
    setGlobalScale(scale);
}

void KCMKScreen::writeGlobalScale()
{
    if (qFuzzyCompare(m_initialGlobalScale, m_globalScale)) {
        return;
    }
    auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    config->group("KScreen").writeEntry("ScaleFactor", m_globalScale);

    // Write env var to be used by session startup scripts to populate the QT_SCREEN_SCALE_FACTORS
    // env var.
    // We use QT_SCREEN_SCALE_FACTORS as opposed to QT_SCALE_FACTOR as we need to use one that will
    // NOT scale fonts according to the scale.
    // Scaling the fonts makes sense if you don't also set a font DPI, but we NEED to set a font
    // DPI for both PlasmaShell which does it's own thing, and for KDE4/GTK2 applications.
    QString screenFactors;
    for (const auto &output: m_config->config()->outputs()) {
        screenFactors.append(output->name() + QLatin1Char('=') + QString::number(m_globalScale) + QLatin1Char(';'));
    }
    config->group("KScreen").writeEntry("ScreenScaleFactors", screenFactors);

    KConfig fontConfig(QStringLiteral("kcmfonts"));
    auto fontConfigGroup = fontConfig.group("General");

    if (qFuzzyCompare(m_globalScale, 1.0)) {
        //if dpi is the default (96) remove the entry rather than setting it
        QProcess proc;
        proc.start(QStringLiteral("xrdb -quiet -remove -nocpp"));
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi\n"));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
        fontConfigGroup.writeEntry("forceFontDPI", 0);
    } else {
        const int scaleDpi = qRound(m_globalScale * 96.0);
        QProcess proc;
        proc.start(QStringLiteral("xrdb -quiet -merge -nocpp"));
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi: " + QString::number(scaleDpi).toLatin1()));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
        fontConfigGroup.writeEntry("forceFontDPI", scaleDpi);
    }

    m_initialGlobalScale = m_globalScale;
    Q_EMIT globalScaleWritten();
}

qreal KCMKScreen::globalScale() const
{
    return m_globalScale;
}

void KCMKScreen::setGlobalScale(qreal scale)
{
    if (qFuzzyCompare(m_globalScale, scale)) {
        return;
    }
    m_globalScale = scale;
    if (m_config) {
        m_config->checkNeedsSave();
    } else {
        continueNeedsSaveCheck(false);
    }
    Q_EMIT changed();
    Q_EMIT globalScaleChanged();
}

int KCMKScreen::outputRetention() const
{
    if (!m_config) {
        return -1;
    }
    return m_config->retention();
}

void KCMKScreen::setOutputRetention(int retention)
{
    if (!m_config) {
        return;
    }
    m_config->setRetention(retention);
}


#include "kcm.moc"
