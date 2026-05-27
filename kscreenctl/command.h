/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QStringList>

#include <iostream>
#include <print>
#include <vector>

using namespace Qt::StringLiterals;

/**
 * The Command type represents an operation that can be performed. There are two types of commands:
 *
 * - standalone commands, e.g. list-outputs;
 * - and commands that have an object/output associated with them, which they operate on, for
 * example, OUTPUT set-scale.
 *
 * All available commands are stored in the COMMAND section, which is then traversed when processing
 * command line arguments or displaying a help message.
 */
struct Command
{
    /**
     * List all available commands.
     */
    static std::span<const Command> all();

    /**
     * The associated object. If it's a standalone command, the object will be @c null.
     */
    const char *object;

    /**
     * The command name.
     */
    const char *name;

    /**
     * Short command description.
     */
    const char *summary;

    /**
     * Long command description.
     */
    const char *description;

    /**
     * Command group. It can be used to group similar commands in help output.
     */
    const char *topic;

    int (*dispatch)();
    int (*dispatchObject)(const QString &object);
};

#define COMMAND_ATTRIBUTE __attribute__((used, retain, section("COMMAND"), aligned(sizeof(void *))))

#define COMMAND(dispatch_func, summary_text)                                                                                                                   \
    COMMAND_ATTRIBUTE                                                                                                                                          \
    static const Command command = {                                                                                                                           \
        .name = VERB_NAME,                                                                                                                                     \
        .summary = summary_text,                                                                                                                               \
        .dispatch = dispatch_func,                                                                                                                             \
    };

#define COMMAND_WITH_DESCRIPTION(dispatch_func, summary_text, description_text)                                                                                \
    COMMAND_ATTRIBUTE                                                                                                                                          \
    static const Command command = {                                                                                                                           \
        .name = VERB_NAME,                                                                                                                                     \
        .summary = summary_text,                                                                                                                               \
        .description = description_text,                                                                                                                       \
        .dispatch = dispatch_func,                                                                                                                             \
    };

#define OBJECT_COMMAND(dispatch_func, summary_text)                                                                                                            \
    COMMAND_ATTRIBUTE                                                                                                                                          \
    static const Command command = {                                                                                                                           \
        .object = OBJECT_NAME,                                                                                                                                 \
        .name = VERB_NAME,                                                                                                                                     \
        .summary = summary_text,                                                                                                                               \
        .dispatchObject = dispatch_func,                                                                                                                       \
    };

#define OBJECT_COMMAND_WITH_TOPIC(dispatch_func, summary_text, topic_text)                                                                                     \
    COMMAND_ATTRIBUTE                                                                                                                                          \
    static const Command command = {                                                                                                                           \
        .object = OBJECT_NAME,                                                                                                                                 \
        .name = VERB_NAME,                                                                                                                                     \
        .summary = summary_text,                                                                                                                               \
        .topic = topic_text,                                                                                                                                   \
        .dispatchObject = dispatch_func,                                                                                                                       \
    };

#define OBJECT_COMMAND_WITH_TOPIC_AND_DESCRIPTION(dispatch_func, summary_text, description_text, topic_text)                                                   \
    COMMAND_ATTRIBUTE                                                                                                                                          \
    static const Command command = {                                                                                                                           \
        .object = OBJECT_NAME,                                                                                                                                 \
        .name = VERB_NAME,                                                                                                                                     \
        .summary = summary_text,                                                                                                                               \
        .description = description_text,                                                                                                                       \
        .topic = topic_text,                                                                                                                                   \
        .dispatchObject = dispatch_func,                                                                                                                       \
    };

/**
 * The CommandExample type represents a usage example of a command.
 */
struct CommandExample
{
    /**
     * List all available command examples.
     */
    static std::span<const CommandExample> all();

    /**
     * List command examples for the specified @a command.
     */
    static std::vector<const CommandExample *> all(const Command *command);

    /**
     * The object that the command is associated with. Can be null.
     */
    const char *object;

    /**
     * The command name.
     */
    const char *command;

    /**
     * Short example description.
     */
    const char *summary;

    /**
     * Example usage.
     */
    const char *example;
};

// The CONCATENATE_RAW helper exists to force macro expansion. Without it, ## will literally join
// __COUNTER__ with the preceeding token, e.g. foo__COUNTER__ instead of foo0.
#define CONCATENATE_RAW(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_RAW(x, y)

#define COMMAND_EXAMPLE_ATTRIBUTE __attribute__((used, retain, section("COMMAND_EXAMPLE"), aligned(sizeof(void *))))

#define COMMAND_EXAMPLE(summary_text, example_text)                                                                                                            \
    COMMAND_EXAMPLE_ATTRIBUTE                                                                                                                                  \
    static const CommandExample CONCATENATE(commandExample_, __COUNTER__) = {                                                                                  \
        .command = VERB_NAME,                                                                                                                                  \
        .summary = summary_text,                                                                                                                               \
        .example = example_text,                                                                                                                               \
    };

#define OBJECT_COMMAND_EXAMPLE(summary_text, example_text)                                                                                                     \
    COMMAND_EXAMPLE_ATTRIBUTE                                                                                                                                  \
    static const CommandExample CONCATENATE(commandExample_, __COUNTER__) = {                                                                                  \
        .object = OBJECT_NAME,                                                                                                                                 \
        .command = VERB_NAME,                                                                                                                                  \
        .summary = summary_text,                                                                                                                               \
        .example = example_text,                                                                                                                               \
    };

/**
 * The CommandOption type represents an option that a Command accepts. There are several command types:
 *
 * - simple toggle switches. They have the "--option" format;
 * - value options. They have the "--option value" or "--option=value" format;
 * - positional arguments: these are options that don't start with "--". For example, they can be
 *   used to specify the position in the "set-position X Y" command.
 *
 */
struct CommandOption
{
    /**
     * List all registered command options.
     */
    static std::span<const CommandOption> all();

    /**
     * List all options for the specified @a command.
     */
    static std::vector<const CommandOption *> all(const Command *command);

    /**
     * Process the command line @a arguments for the specified @a command. It returns @c true
     * if the command line arguments have been processed successfully, otherwise returns @c false.
     */
    static bool process(const Command *command, const QStringList &arguments);

    /**
     * The object that the command is associated with it. Can be @c null.
     */
    const char *object;

    /**
     * The command name.
     */
    const char *command;

    /**
     * The option name.
     */
    const char *name;

    /**
     * The value format.
     *
     * For example, if there is a "set-position" that takes two values x and y, then the format will
     * be "X Y". The format is used to display the help message and validate command line arguments.
     *
     * The format can be @c null for simple toggle options.
     */
    const char *format;

    /**
     * Short command option description.
     */
    const char *summary;

    /**
     * Where the value will be stored.
     */
    void *variable = nullptr;

    /**
     * Whether this is a positional option or a --regular option.
     */
    bool positional = false;
};

#define COMMAND_OPTION_ATTRIBUTE __attribute__((used, retain, section("COMMAND_OPTION"), aligned(sizeof(void *))))

#define COMMAND_TOGGLE_OPTION(option_name, option_variable, option_summary)                                                                                    \
    COMMAND_OPTION_ATTRIBUTE                                                                                                                                   \
    static const CommandOption CONCATENATE(commandOption_, __COUNTER__) = {                                                                                    \
        .command = VERB_NAME,                                                                                                                                  \
        .name = option_name,                                                                                                                                   \
        .summary = option_summary,                                                                                                                             \
        .variable = option_variable,                                                                                                                           \
    };

#define COMMAND_POSITIONAL_OPTION(option_format, option_variable)                                                                                              \
    COMMAND_OPTION_ATTRIBUTE                                                                                                                                   \
    static const CommandOption CONCATENATE(commandOption_, __COUNTER__) = {                                                                                    \
        .command = VERB_NAME,                                                                                                                                  \
        .format = option_format,                                                                                                                               \
        .variable = option_variable,                                                                                                                           \
        .positional = true,                                                                                                                                    \
    };

#define OBJECT_COMMAND_TOGGLE_OPTION(option_name, option_variable, option_summary)                                                                             \
    COMMAND_OPTION_ATTRIBUTE                                                                                                                                   \
    static const CommandOption CONCATENATE(commandOption_, __COUNTER__) = {                                                                                    \
        .object = OBJECT_NAME,                                                                                                                                 \
        .command = VERB_NAME,                                                                                                                                  \
        .name = option_name,                                                                                                                                   \
        .summary = option_summary,                                                                                                                             \
        .variable = option_variable,                                                                                                                           \
    };

#define OBJECT_COMMAND_POSITIONAL_OPTION(option_format, option_variable)                                                                                       \
    COMMAND_OPTION_ATTRIBUTE                                                                                                                                   \
    static const CommandOption CONCATENATE(commandOption_, __COUNTER__) = {                                                                                    \
        .object = OBJECT_NAME,                                                                                                                                 \
        .command = VERB_NAME,                                                                                                                                  \
        .format = option_format,                                                                                                                               \
        .variable = option_variable,                                                                                                                           \
        .positional = true,                                                                                                                                    \
    };
