/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command.h"
#include "util.h"

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static bool enabled = false;
COMMAND_TOGGLE_OPTION("enabled",
                      &enabled,
                      "Only list enabled outputs")

static bool disabled = false;
COMMAND_TOGGLE_OPTION("disabled",
                      &disabled,
                      "Only list disabled outputs")

static bool names = false;
COMMAND_TOGGLE_OPTION("names",
                      &names,
                      "Only display output names")

static bool json = false;
COMMAND_TOGGLE_OPTION("json",
                      &json,
                      "Display output information as JSON")

static int run()
{
    auto getConfigOperation = new KScreen::GetConfigOperation();
    getConfigOperation->exec();

    KScreen::ConfigPtr config = getConfigOperation->config();
    if (!config) {
        return 1;
    }

    auto outputs = config->connectedOutputs();
    if (enabled) {
        outputs.removeIf([](const auto &output) {
            return !(*output)->isEnabled();
        });
    } else if (disabled) {
        outputs.removeIf([](const auto &output) {
            return (*output)->isEnabled();
        });
    }

    if (names) {
        if (json) {
            QJsonArray array;
            for (const auto &output : outputs) {
                array.append(output->name());
            }
            std::println(std::cout, "{}", QJsonDocument(array).toJson().toStdString());
        } else {
            for (const auto &output : outputs) {
                std::println(std::cout, "{}", output->name().toStdString());
            }
        }
    } else {
        if (json) {
            QJsonArray array;
            for (const auto &output : outputs) {
                array.append(outputAsJson(output));
            }
            std::println(std::cout, "{}", QJsonDocument(array).toJson().toStdString());
        } else {
            for (const auto &output : outputs) {
                showOutputAsText(output);
            }
        }
    }

    return 0;
}

COMMAND(run, "List all connected outputs")

COMMAND_EXAMPLE("List all available outputs", "")

COMMAND_EXAMPLE("List all enabled outputs", "--enabled")
