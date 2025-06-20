/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2023 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osdaction.h"
#include "output.h"

#include <KLocalizedString>
#include <KScreen/Config>
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>
#include <QPoint>

using namespace KScreen;

QList<OsdAction> OsdAction::availableActions()
{
    return {
        {SwitchToExternal, i18nd("kscreen_common", "Only use external screen"), QStringLiteral("osd-shutd-laptop")},
        {SwitchToInternal, i18nd("kscreen_common", "Only use built-in screen"), QStringLiteral("osd-shutd-screen")},
        {Clone, i18nd("kscreen_common", "Mirror screens"), QStringLiteral("osd-duplicate")},
        {ExtendLeft, i18nd("kscreen_common", "Extend to left of built-in screen"), QStringLiteral("osd-sbs-left")},
        {ExtendRight, i18nd("kscreen_common", "Extend to right of built-in screen"), QStringLiteral("osd-sbs-sright")},
        {NoAction, i18nd("kscreen_common", "Leave unchanged"), QStringLiteral("dialog-cancel")},
    };
}

KScreen::SetConfigOperation *OsdAction::applyAction(const QSharedPointer<KScreen::Config> &config, Action action)
{
    const ConfigPtr copy = config->clone();
    const OutputList outputs = copy->connectedOutputs();
    if (outputs.size() != 2) {
        return nullptr;
    }

    auto internalIt = std::find_if(outputs.cbegin(), outputs.cend(), [](const auto &output) {
        return output->type() == KScreen::Output::Type::Panel;
    });
    if (internalIt == outputs.end()) {
        internalIt = outputs.begin();
    }
    const OutputPtr &internal = *internalIt;
    const OutputPtr &external = *std::find_if(outputs.cbegin(), outputs.cend(), [&internal](const auto &output) {
        return output != internal;
    });

    if (config->supportedFeatures() & Config::Feature::PerOutputScaling) {
        external->setReplicationSource(0);
        internal->setReplicationSource(0);
    } else {
        if (!internal->isEnabled()) {
            internal->setRotation(::Output::readGlobal(internal).rotation.value_or(Output::Rotation::None));
        }
        if (!external->isEnabled()) {
            external->setRotation(::Output::readGlobal(external).rotation.value_or(Output::Rotation::None));
        }
    }

    switch (action) {
    case KScreen::OsdAction::Action::NoAction:
        return nullptr;
    case KScreen::OsdAction::Action::SwitchToExternal:
        internal->setEnabled(false);
        external->setEnabled(true);
        external->setPos(QPoint());
        break;
    case KScreen::OsdAction::Action::SwitchToInternal:
        internal->setEnabled(true);
        internal->setPos(QPoint());
        external->setEnabled(false);
        break;
    case KScreen::OsdAction::Action::Clone: {
        internal->setEnabled(true);
        external->setEnabled(true);
        if (config->supportedFeatures() & Config::Feature::PerOutputScaling) {
            // on Wayland, KWin deals with modes, we just set the replication source
            external->setReplicationSource(internal->id());
            internal->setReplicationSource(0);
        } else {
            // keep the old code path for Xorg
            internal->setEnabled(true);
            external->setEnabled(true);
            internal->setPos(QPoint());
            external->setPos(QPoint());
            QList<KScreen::ModePtr> commonModes;
            const auto internalModes = internal->modes();
            const auto externalModes = external->modes();
            for (const auto &mode : internalModes) {
                const bool shared = std::any_of(externalModes.begin(), externalModes.end(), [&mode](const auto &otherMode) {
                    return mode->size() == otherMode->size();
                });
                if (shared) {
                    commonModes.push_back(mode);
                }
            }
            if (commonModes.isEmpty()) {
                return nullptr;
            }
            const auto biggestSizeIt = std::max_element(commonModes.begin(), commonModes.end(), [](const auto &left, const auto &right) {
                if (left->size().width() != right->size().width()) {
                    return left->size().width() < right->size().width();
                }
                return left->size().height() < right->size().height();
            });
            const auto biggestSize = (*biggestSizeIt)->size();
            const auto findBestMode = [&biggestSize](const KScreen::OutputPtr &output) {
                auto list = output->modes();
                list.erase(std::remove_if(list.begin(),
                                          list.end(),
                                          [&biggestSize](const auto &mode) {
                                              return mode->size() != biggestSize;
                                          }),
                           list.end());
                return *std::max_element(list.begin(), list.end(), [](const auto &left, const auto &right) {
                    return left->refreshRate() < right->refreshRate();
                });
            };
            internal->setCurrentModeId(findBestMode(internal)->id());
            external->setCurrentModeId(findBestMode(external)->id());
            external->setScale(internal->scale());
        }
        break;
    }
    case KScreen::OsdAction::Action::ExtendRight: {
        internal->setEnabled(true);
        external->setEnabled(true);
        internal->setPos(QPoint());
        ModePtr currentMode = internal->currentMode();
        if (!currentMode) { // When the internal display is not enabled
            const auto internalModesMap = internal->modes();
            Q_ASSERT(!internalModesMap.empty());
            auto bestModeIt = std::max_element(internalModesMap.cbegin(), internalModesMap.cend(), [](const auto &left, const auto &right) {
                const QSize leftSize = left->size();
                const QSize rightSize = right->size();
                return (leftSize.width() < rightSize.width() && leftSize.height() < rightSize.height())
                    || (leftSize == rightSize && left->refreshRate() < right->refreshRate());
            });
            currentMode = bestModeIt.value();
            internal->setCurrentModeId(currentMode->id());
        }
        external->setPos({copy->logicalSizeForOutputInt(*internal).width(), 0});
        break;
    }
    case KScreen::OsdAction::Action::ExtendLeft: {
        internal->setEnabled(true);
        external->setEnabled(true);
        external->setPos(QPoint());
        ModePtr currentMode = external->currentMode();
        if (!currentMode) { // When the external display is not enabled
            const auto externalModesMap = external->modes();
            Q_ASSERT(!externalModesMap.empty());
            auto bestModeIt = std::max_element(externalModesMap.cbegin(), externalModesMap.cend(), [](const auto &left, const auto &right) {
                const QSize leftSize = left->size();
                const QSize rightSize = right->size();
                return (leftSize.width() < rightSize.width() && leftSize.height() < rightSize.height())
                    || (leftSize == rightSize && left->refreshRate() < right->refreshRate());
            });
            currentMode = bestModeIt.value();
            external->setCurrentModeId(currentMode->id());
        }
        internal->setPos({copy->logicalSizeForOutputInt(*external).width(), 0});
        break;
    }
    }
    return new KScreen::SetConfigOperation(copy);
}

#include "moc_osdaction.cpp"
