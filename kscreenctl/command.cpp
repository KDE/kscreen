/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "command.h"

std::span<const Command> Command::all()
{
    extern const Command __start_COMMAND[];
    extern const Command __stop_COMMAND[];
    return std::span(__start_COMMAND, __stop_COMMAND);
}

std::span<const CommandExample> CommandExample::all()
{
    extern const CommandExample __start_COMMAND_EXAMPLE[];
    extern const CommandExample __stop_COMMAND_EXAMPLE[];
    return std::span(__start_COMMAND_EXAMPLE, __stop_COMMAND_EXAMPLE);
}

std::vector<const CommandExample *> CommandExample::all(const Command *command)
{
    std::vector<const CommandExample *> examples;

    for (const CommandExample &example : all()) {
        if (bool(example.object) != bool(command->object) ||
            (command->object && strcmp(example.object, command->object) != 0)) {
            continue;
        }

        if (strcmp(example.command, command->name) == 0) {
            examples.push_back(&example);
        }
    }

    return examples;
}

std::span<const CommandOption> CommandOption::all()
{
    extern const CommandOption __start_COMMAND_OPTION[];
    extern const CommandOption __stop_COMMAND_OPTION[];
    return std::span(__start_COMMAND_OPTION, __stop_COMMAND_OPTION);
}

std::vector<const CommandOption *> CommandOption::all(const Command *command)
{
    std::vector<const CommandOption *> options;

    for (const CommandOption &option : all()) {
        if (bool(option.object) != bool(command->object) ||
            (command->object && strcmp(option.object, command->object) != 0)) {
            continue;
        }

        if (strcmp(option.command, command->name) == 0) {
            options.push_back(&option);
        }
    }

    return options;
}

bool CommandOption::process(const Command *command, const QStringList &arguments)
{
    const auto options = CommandOption::all(command);
    if (options.empty()) {
        return arguments.isEmpty();
    }

    QStringList positionalArguments;

    for (auto it = arguments.begin(); it != arguments.end(); ++it) {
        if (!it->startsWith("--")) {
            positionalArguments.append(*it);
            continue;
        }

        const CommandOption *option = nullptr;

        const QString optionName = it->mid(2);
        for (const CommandOption *candidate : options) {
            if (optionName == candidate->name) {
                option = candidate;
                break;
            }
        }

        if (!option) {
            std::println(std::cerr, "Unknown argument: {}", it->toStdString());
            return false;
        }

        if (option->format) {
            QString value;

            const int equalsIndex = it->indexOf('=');
            if (equalsIndex == -1) {
                ++it;
                if (it == arguments.end()) {
                    std::println(std::cerr, "Missing value for option {}.", option->name);
                    return false;
                }

                if (it->startsWith("--")) {
                    std::println(std::cerr, "Missing value for option {}.", option->name);
                    return false;
                }

                value = *it;
            } else {
                value = it->mid(equalsIndex + 1).trimmed();
                if (value.isEmpty()) {
                    std::println(std::cerr, "Missing value for option {}.", option->name);
                    return false;
                }
            }

            QString *context = static_cast<QString *>(option->variable);
            *context = value;
        } else {
            bool *context = static_cast<bool *>(option->variable);
            *context = true;
        }
    }

    const CommandOption *positionalOption = nullptr;
    for (const CommandOption *option : options) {
        if (option->positional) {
            positionalOption = option;
            break;
        }
    }

    if (positionalOption) {
        const QByteArrayList parts = QByteArray(positionalOption->format).split(' ');
        if (parts.size() != positionalArguments.size()) {
            if (positionalArguments.size() < parts.size()) {
                std::println(std::cerr, "Too few positional arguments. Expected: {}.", positionalOption->format);
            } else {
                std::println(std::cerr, "Too many positional arguments. Expected: {}.", positionalOption->format);
            }
            return false;
        }

        QStringList *context = static_cast<QStringList *>(positionalOption->variable);
        *context = positionalArguments;
    } else if (!positionalArguments.isEmpty()) {
        std::println(std::cerr, "Unexpected positional argument: {}", positionalArguments[0].toStdString());
        return false;
    }

    return true;
}
