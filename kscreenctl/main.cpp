/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QGuiApplication>

#include "command.h"

class HelpStreambuf : public std::streambuf
{
public:
    explicit HelpStreambuf(const std::ostream &system)
        : m_system(system.rdbuf())
    {
    }

    void setMinLeft(int_type left)
    {
        m_minLeft = left;
        m_overshot = m_minLeft && m_cursor && m_cursor > m_minLeft;
    }

    void setMaxRight(int_type right)
    {
        m_maxRight = right;
    }

private:
    int_type overflow(int_type character) override
    {
        if (traits_type::eq_int_type(traits_type::eof(), character)) {
            return traits_type::not_eof(character);
        }

        if (character == '\n') {
            m_buffer += character;
            const int_type result = m_system->sputn(m_buffer.data(), m_buffer.size());

            m_buffer.clear();
            m_cursor = 0;
            return result;
        }

        if (m_overshot) {
            m_buffer += '\n';
            m_system->sputn(m_buffer.data(), m_buffer.size());

            m_buffer.clear();
            m_cursor = 0;
            m_overshot = false;
        }

        if (m_cursor < m_minLeft) {
            const int_type n = m_minLeft - m_cursor;
            m_buffer.append(n, ' ');
            m_cursor += n;
        }

        m_buffer += character;

        // Avoid bumping the cursor for UTF-8 continuation bytes.
        m_cursor += (character & 0xc0) != 0x80;

        if (m_maxRight && m_cursor >= m_maxRight) {
            if (std::isspace(m_buffer.back())) {
                int_type lastChar = -1;
                for (size_t i = 0; i < m_buffer.size(); ++i) {
                    if (!std::isspace(m_buffer[i])) {
                        lastChar = i;
                    }
                }

                if (lastChar != -1) {
                    const int_type newlineIndex = lastChar + 1;
                    m_buffer[newlineIndex] = '\n';
                    m_system->sputn(m_buffer.data(), newlineIndex + 1);
                }

                m_buffer.clear();
                m_cursor = 0;
            }
        }

        return character;
    }

    std::streambuf *m_system;
    std::basic_string<char_type> m_buffer;
    int_type m_minLeft = 0;
    int_type m_maxRight = 0;
    int_type m_cursor = 0;
    bool m_overshot = false;
};

class HelpStream : public std::ostream
{
public:
    HelpStream(const std::ostream &stream)
        : std::ostream(&m_buffer)
        , m_buffer(stream)
    {
    }

    void setMinLeft(int left)
    {
        m_buffer.setMinLeft(left);
    }

    void setMaxRight(int right)
    {
        m_buffer.setMaxRight(right);
    }

private:
    HelpStreambuf m_buffer;
};

static void showHelp()
{
    const std::string applicationName = QCoreApplication::applicationName().toStdString();
    HelpStream help(std::cerr);

    std::println(help, "Usage: {} COMMAND …\n", applicationName);
    std::println(help, "Utility to change the output configuration.\n");

    std::println(help, "Commands:");

    const size_t padding = 30;
    const size_t maxWidth = 80;
    const size_t indent = 3;

    for (const Command &command : Command::all()) {
        if (command.object && command.name != "self"_ba) {
            continue;
        }

        help.setMinLeft(indent);
        help.setMaxRight(0);
        if (command.object) {
            std::print(help, "{}", command.object);
        } else {
            std::print(help, "{}", command.name);
        }

        help.setMinLeft(indent + padding);
        help.setMaxRight(maxWidth);
        std::print(help, "{}\n", command.summary);
    }

    std::println(help);

    for (const Command &command : Command::all()) {
        if (command.object) {
            continue;
        }

        std::vector<const CommandOption *> options = CommandOption::all(&command);

        const CommandOption *positional = nullptr;
        for (const CommandOption *option : options) {
            if (option->positional) {
                positional = option;
                break;
            }
        }

        help.setMinLeft(0);
        help.setMaxRight(0);
        std::println(help, "\x1b[1m{}: {}\x1b[22m", command.name, command.summary);

        help.setMinLeft(indent);
        help.setMaxRight(0);
        std::print(help, "Usage: {} {}", applicationName, command.name);
        if (!options.empty()) {
            std::print(help, " [OPTIONS…]");
        }
        if (positional) {
            const QByteArrayList parts = QByteArray::fromRawData(positional->format, strlen(positional->format)).split(' ');
            for (const QByteArray &part : parts) {
                std::print(help, " {}…", part.data());
            }
        }
        std::print(help, "\n\n");

        if (command.description) {
            help.setMinLeft(indent);
            help.setMaxRight(maxWidth);
            std::println(help, "{}\n", command.description);
        }

        const auto examples = CommandExample::all(&command);
        if (!examples.empty()) {
            help.setMinLeft(0);
            help.setMaxRight(0);
            std::println(help, "\x1b[4mExamples:\x1b[24m\n");

            for (const CommandExample *example : examples) {
                help.setMinLeft(indent);
                help.setMaxRight(maxWidth);
                std::println(help, "{}\n", example->summary);

                help.setMinLeft(2 * indent);
                help.setMaxRight(0);
                std::println(help, "{} {} {}\n", applicationName, example->command, example->example);
            }
        }

        if (!options.empty()) {
            help.setMinLeft(0);
            help.setMaxRight(0);
            std::println(help, "\x1b[4mOptions:\x1b[24m");

            for (const CommandOption *option : std::as_const(options)) {
                help.setMinLeft(indent);
                help.setMaxRight(0);
                if (!option->format) {
                    std::print(help, "--{}", option->name);
                } else {
                    std::print(help, "--{}={}…", option->name, option->format);
                }

                help.setMinLeft(indent + padding);
                help.setMaxRight(maxWidth);
                std::print(help, "{}\n", option->summary);
            }

            std::println(help);
        }
    }

    std::vector<const Command *> objectCommands;
    const Command *selfCommand = nullptr;
    for (const Command &command : Command::all()) {
        if (command.object) {
            objectCommands.push_back(&command);

            if (command.name == "self"_ba) {
                selfCommand = &command;
            }
        }
    }

    help.setMinLeft(0);
    help.setMaxRight(0);
    std::println(help, "\x1b[1m{}: {}\x1b[22m", selfCommand->object, selfCommand->summary);

    help.setMinLeft(indent);
    help.setMaxRight(0);
    std::println(help, "Usage: {} {} COMMAND… [OPTIONS…]\n", applicationName, selfCommand->object);

    if (selfCommand->description) {
        help.setMinLeft(indent);
        help.setMaxRight(maxWidth);
        std::println(help, "{}\n", selfCommand->description);
    }

    QList<const CommandOption *> options;
    for (const CommandOption &option : CommandOption::all()) {
        if (!option.object || option.positional) {
            continue;
        }
        options.append(&option);
    }

    if (!options.empty()) {
        help.setMinLeft(0);
        help.setMaxRight(0);
        std::println(help, "\x1b[4mOptions:\x1b[24m");

        for (const CommandOption *option : std::as_const(options)) {
            help.setMinLeft(indent);
            help.setMaxRight(0);
            if (!option->format) {
                std::print(help, "--{}", option->name);
            } else {
                std::print(help, "--{}={}…", option->name, option->format);
            }

            help.setMinLeft(indent + padding);
            help.setMaxRight(maxWidth);
            std::print(help, "{}\n", option->summary);
        }

        std::println(help);
    }

    QMultiMap<QString, const Command *> topics;
    QMultiMap<QString, const CommandExample *> topicExamples;
    for (const Command *command : objectCommands) {
        if (command->name != "self"_ba) {
            topics.insert(command->topic, command);
        }

        const auto examples = CommandExample::all(command);
        for (const CommandExample *example : examples) {
            topicExamples.insert(command->topic, example);
        }
    }

    const QList<QString> sortedTopics = topics.uniqueKeys();
    for (const QString &topic : sortedTopics) {
        QList<const Command *> commands = topics.values(topic);
        std::sort(commands.begin(), commands.end(), [](const auto &a, const auto &b) {
            return strcmp(a->name, b->name) < 0;
        });

        QList<const CommandExample *> commandExamples = topicExamples.values(topic);
        std::sort(commandExamples.begin(), commandExamples.end(), [](const auto &a, const auto &b) {
            return strcmp(a->command, b->command) < 0;
        });

        help.setMinLeft(0);
        help.setMaxRight(0);
        std::println(help, "\x1b[4m{} Commands:\x1b[24m", topic.toStdString());
        for (const Command *command : commands) {
            const auto options = CommandOption::all(command);

            const CommandOption *positional = nullptr;
            for (const CommandOption *option : options) {
                if (option->positional) {
                    positional = option;
                    break;
                }
            }

            help.setMinLeft(indent);
            help.setMaxRight(0);
            std::print(help, "{}", command->name);
            if (positional) {
                const QByteArrayList parts = QByteArray::fromRawData(positional->format, strlen(positional->format)).split(' ');
                for (const QByteArray &part : parts) {
                    std::print(help, " {}…", part.data());
                }
            }

            help.setMinLeft(indent + padding);
            help.setMaxRight(maxWidth);
            std::print(help, "{}\n", command->summary);
        }

        std::println(help);

        if (!commandExamples.isEmpty()) {
            help.setMinLeft(0);
            help.setMaxRight(0);
            std::println(help, "\x1b[4m{} Command Examples:\x1b[24m\n", topic.toStdString());

            for (const CommandExample *example : commandExamples) {
                help.setMinLeft(indent);
                help.setMaxRight(maxWidth);
                std::println(help, "{}\n", example->summary);

                help.setMinLeft(2 * indent);
                help.setMaxRight(0);
                std::println(help, "{} {}\n", applicationName, example->example);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const QStringList arguments = app.arguments().sliced(1);
    if (!arguments.isEmpty()) {
        if (arguments[0] == "help" || arguments.contains("--help")) {
            showHelp();
            return 0;
        }

        for (const Command &command : Command::all()) {
            if (command.name == arguments[0]) {
                if (command.dispatch) {
                    if (!CommandOption::process(&command, arguments.sliced(1))) {
                        return 1;
                    }
                    return command.dispatch();
                }
            }
        }

        if (arguments.size() >= 2) {
            const QString object = arguments[0];
            const QString verb = arguments[1];

            for (const Command &command : Command::all()) {
                if (command.dispatchObject && command.name == verb) {
                    if (!CommandOption::process(&command, arguments.sliced(2))) {
                        return 1;
                    }
                    return command.dispatchObject(object);
                }
            }
        }

        for (const Command &command : Command::all()) {
            if (command.dispatchObject && command.name == "self"_ba) {
                if (!CommandOption::process(&command, arguments.sliced(1))) {
                    return 1;
                }
                return command.dispatchObject(arguments[0]);
            }
        }
    }

    showHelp();
    return 1;
}
