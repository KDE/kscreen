/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "command.h"
#include "util.h"

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Mode>

#define OUTPUT_TRAMPOLINE(func)                                                                                                                                \
    [](const QString &object) -> int {                                                                                                                         \
        auto getConfigOperation = new KScreen::GetConfigOperation();                                                                                           \
        getConfigOperation->exec();                                                                                                                            \
                                                                                                                                                               \
        KScreen::ConfigPtr config = getConfigOperation->config();                                                                                              \
        if (!config) {                                                                                                                                         \
            return 1;                                                                                                                                          \
        }                                                                                                                                                      \
                                                                                                                                                               \
        const KScreen::OutputPtr output = outputByQuery(getConfigOperation->config(), object);                                                                 \
        if (!output) {                                                                                                                                         \
            std::println(std::cerr, "Unknown output: {}", object.toStdString());                                                                               \
            return 1;                                                                                                                                          \
        }                                                                                                                                                      \
                                                                                                                                                               \
        return wrap(func, output, config);                                                                                                                     \
    }

#define OUTPUT_COMMAND(dispatch_func, summary_text, topic_text) \
    OBJECT_COMMAND_WITH_TOPIC(OUTPUT_TRAMPOLINE(dispatch_func), summary_text, topic_text)

#define OUTPUT_COMMAND_WITH_DESCRIPTION(dispatch_func, summary_text, description_text, topic_text) \
    OBJECT_COMMAND_WITH_TOPIC_AND_DESCRIPTION(OUTPUT_TRAMPOLINE(dispatch_func), summary_text, description_text, topic_text)

#define OUTPUT_COMMAND_EXAMPLE(summary_text, example_text) \
    OBJECT_COMMAND_EXAMPLE(summary_text, example_text)

#define OUTPUT_COMMAND_TOGGLE_OPTION(option_name, option_variable, option_summary) \
    OBJECT_COMMAND_TOGGLE_OPTION(option_name, option_variable, option_summary)

#define OUTPUT_COMMAND_POSITIONAL_OPTION(option_format, option_variable) \
    OBJECT_COMMAND_POSITIONAL_OPTION(option_format, option_variable)
