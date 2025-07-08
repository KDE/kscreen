/*
    SPDX-FileCopyrightText: 2025 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "hdrcalibrator.h"
#include "hdrcalibrator_debug.h"

#include <KAboutData>
#include <KCrash>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <QGuiApplication>

using namespace Qt::StringLiterals;

HdrCalibrator::HdrCalibrator()
{
    m_compressor.setSingleShot(true);
    m_compressor.setInterval(10);
    connect(&m_compressor, &QTimer::timeout, this, &HdrCalibrator::applyConfig);
}

void HdrCalibrator::applyConfig()
{
    if (m_setOp) {
        m_configDirty = true;
        return;
    }
    m_configDirty = false;
    // clone makes sure that modifying the output (but not applying the config)
    // won't override the settings we just changed
    m_setOp = new KScreen::SetConfigOperation(m_config->clone());
    connect(m_setOp, &KScreen::SetConfigOperation::finished, this, &HdrCalibrator::setOpFinished);
}

void HdrCalibrator::safeQuit()
{
    if (m_setOp || m_compressor.isActive()) {
        m_quitRequested = true;
    } else {
        QCoreApplication::quit();
    }
}

void HdrCalibrator::setOpFinished()
{
    m_setOp = nullptr;
    if (m_configDirty) {
        m_compressor.start();
    } else if (m_quitRequested) {
        QCoreApplication::quit();
    }
}

QString HdrCalibrator::outputName() const
{
    return m_output->name();
}

qreal HdrCalibrator::peakBrightness() const
{
    return m_output->maxPeakBrightness();
}

qreal HdrCalibrator::peakBrightnessOverride() const
{
    return m_output->maxPeakBrightnessOverride().value_or(0);
}

void HdrCalibrator::setPeakBrightnessOverride(qreal override)
{
    m_output->setMaxPeakBrightnessOverride(override);
}

qreal HdrCalibrator::brightness() const
{
    return m_output->brightness();
}

void HdrCalibrator::setBrightness(qreal brightness)
{
    m_output->setBrightness(brightness);
}

qreal HdrCalibrator::sdrBrightness() const
{
    return m_output->sdrBrightness();
}

void HdrCalibrator::setSdrBrightness(qreal brightness)
{
    m_output->setSdrBrightness(brightness);
}

void HdrCalibrator::setOutput(const KScreen::ConfigPtr &config, const KScreen::OutputPtr &output)
{
    m_config = config;
    m_output = output;
    connect(output.get(), &KScreen::Output::brightnessChanged, this, &HdrCalibrator::brightnessChanged);
    connect(output.get(), &KScreen::Output::maxPeakBrightnessOverrideChanged, this, &HdrCalibrator::peakBrightnessOverrideChanged);
    connect(output.get(), &KScreen::Output::brightnessChanged, this, &HdrCalibrator::brightnessChanged);
    connect(output.get(), &KScreen::Output::sdrBrightnessChanged, this, &HdrCalibrator::sdrBrightnessChanged);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        qCWarning(HDRCALIBRATOR) << "need the output name as the argument!";
        return -1;
    }
    char *outputName = argv[1];

    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    KAboutData aboutData(u"KScreen"_s,
                         i18nc("@title:window", "HDR Calibration"), // this is title:window for cherry picking reasons and can be changed in the future
                         QLatin1String(KSCREEN_VERSION),
                         QString() /* description */,
                         KAboutLicense::GPL_V2);

    KAboutData::setApplicationData(aboutData);
    KCrash::initialize();

    // first, get the output
    // we can't do anything until we have it, so this just blocks
    auto getOp = std::make_unique<KScreen::GetConfigOperation>();
    if (!getOp->exec()) {
        qCWarning(HDRCALIBRATOR) << "failed to get a config!";
        return -1;
    }
    const auto outputs = getOp->config()->outputs();
    const auto output = std::ranges::find_if(outputs, [outputName](const auto &output) {
        return output->name() == outputName;
    });
    if (output == outputs.end()) {
        qCWarning(HDRCALIBRATOR) << "couldn't find output" << outputName;
        return -1;
    }

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);

    auto hdrCalibrator = engine.singletonInstance<HdrCalibrator *>("org.kde.hdrcalibrator", "HdrCalibrator");
    hdrCalibrator->setOutput(getOp->config(), *output);
    getOp.reset();

    qmlRegisterType<KScreen::Output>("org.kde.private.kcm.kscreen", 1, 0, "Output");
    engine.loadFromModule("org.kde.hdrcalibrator", "Main");
    return app.exec();
}

#include "moc_hdrcalibrator.cpp"
