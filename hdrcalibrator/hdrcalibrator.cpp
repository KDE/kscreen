
#include "hdrcalibrator.h"

#include <KLocalizedQmlContext>
#include <QGuiApplication>

HdrCalibrator::HdrCalibrator()
{
    m_compressor.setSingleShot(true);
    m_compressor.setInterval(10);
    connect(&m_compressor, &QTimer::timeout, this, &HdrCalibrator::doApplyConfig);
}

void HdrCalibrator::applyConfig()
{
    if (!m_compressor.isActive()) {
        m_compressor.start();
    }
}

void HdrCalibrator::doApplyConfig()
{
    if (m_setOp) {
        connect(m_setOp, &KScreen::SetConfigOperation::finished, this, &HdrCalibrator::applyConfig, Qt::UniqueConnection);
        return;
    }
    m_setOp = new KScreen::SetConfigOperation(m_config);
    connect(m_setOp, &KScreen::SetConfigOperation::finished, this, &HdrCalibrator::setOpFinished);
}

void HdrCalibrator::setOpFinished()
{
    m_setOp = nullptr;
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
        qWarning() << "need the output name as the argument!";
        return -1;
    }
    char *outputName = argv[1];

    QGuiApplication app(argc, argv);

    // first, get the output
    // we can't do anything until we have it, so this just blocks
    auto getOp = std::make_unique<KScreen::GetConfigOperation>();
    if (!getOp->exec()) {
        qWarning() << "failed to get a config!";
        return -1;
    }
    const auto outputs = getOp->config()->outputs();
    const auto output = std::ranges::find_if(outputs, [outputName](const auto &output) {
        return output->name() == outputName;
    });
    if (output == outputs.end()) {
        qWarning() << "couldn't find output" << outputName;
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
