/*
    SPDX-FileCopyrightText: 2026 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KScreen/Config>
#include <KScreen/Mode>

template <typename Func>
struct FunctionPointer
{
};

template <typename Ret, typename... Args>
struct FunctionPointer<Ret (*)(Args...)>
{
    enum { ArgCount = sizeof...(Args) };
};

/**
 * Returns the first Indices values in the specified @a tuple.
 */
template <typename Tuple, std::size_t... Indices>
auto left(Tuple &&tuple, std::index_sequence<Indices...>)
{
    return std::forward_as_tuple(std::get<Indices>(tuple)...);
}

/**
 * Calls the specified @a func with the given @a args. The arguments are applied partially. That
 * is, if the specified function takes no arguments, it will be called without any args; if the
 * function takes one argument, then the first value in the @a args will be passed, and so on.
 */
template <typename Func, typename... Args>
auto wrap(Func func, Args... args)
{
    constexpr size_t argumentCount = FunctionPointer<Func>::ArgCount;
    return std::apply(func, left(std::forward_as_tuple(args...),
                                 std::make_index_sequence<argumentCount>()));
}

void fixupOutputPositions(KScreen::ConfigPtr config);
void snapOutputAfterResize(KScreen::OutputPtr resizedOutput, const QSize &oldSize, KScreen::ConfigPtr config);

int applyConfig(KScreen::ConfigPtr config);
KScreen::OutputPtr outputByQuery(const KScreen::ConfigPtr &config, const QString &query);
KScreen::ModePtr modeByQuery(const KScreen::OutputPtr &output, const QString &query);

std::optional<KScreen::Output::ColorProfileSource> parseColorProfileSource(QStringView input);
std::optional<bool> parseBool(QStringView value);
std::optional<int> parseInt(QStringView input);
std::optional<double> parseDouble(QStringView value);
std::optional<double> parsePercents(QStringView input);

QString printableAutoRotatePolicy(KScreen::Output::AutoRotatePolicy source);
QString printableMode(const KScreen::OutputPtr &output, const KScreen::ModePtr &mode);
QString printableEdrPolicy(KScreen::Output::EdrPolicy source);
QString printableRgbRange(KScreen::Output::RgbRange source);
QString printableVrrPolicy(KScreen::Output::VrrPolicy source);
QString printableColorPowerTradeoff(KScreen::Output::ColorPowerTradeoff source);
QString printableColorProfileSource(KScreen::Output::ColorProfileSource source);
QString printableRotation(KScreen::Output::Rotation rotation);

void showOutputAsText(const KScreen::OutputPtr &output);
QJsonObject outputAsJson(const KScreen::OutputPtr &output);
void showOutputAsJson(const KScreen::OutputPtr &output);
