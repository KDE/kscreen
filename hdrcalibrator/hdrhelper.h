/*
    SPDX-FileCopyrightText: 2025 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QWaylandClientExtensionTemplate>
#include <qwayland-color-management-v1.h>
#include <unordered_map>

class QQuickWindow;
class QEvent;
class ColorManagementGlobal;
class ColorManagementSurface;

class HdrHelper : public QObject
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
public:
    explicit HdrHelper();
    ~HdrHelper();

    enum class Colorspace {
        BT709Linear,
        BT2020PQ,
    };
    Q_ENUM(Colorspace);
    enum class RenderIntent {
        Perceptual,
        RelativeColorimetricBPC,
    };
    Q_ENUM(RenderIntent);

    Q_INVOKABLE void
    setHdrParameters(QQuickWindow *window, Colorspace colorspace, uint32_t referenceLuminance, uint32_t maximumLuminance, RenderIntent renderIntent);

private:
    void setImageDescription(QQuickWindow *window);
    bool eventFilter(QObject *watched, QEvent *event) override;

    const std::unique_ptr<ColorManagementGlobal> m_global;
    struct WindowParams {
        std::unique_ptr<ColorManagementSurface> surface;
        Colorspace colorspace;
        RenderIntent renderIntent;
        uint32_t referenceLuminance;
        uint32_t maximumLuminance;
    };
    std::unordered_map<QQuickWindow *, WindowParams> m_surfaces;
};

class ColorManagementGlobal : public QWaylandClientExtensionTemplate<ColorManagementGlobal, &QtWayland::wp_color_manager_v1::destroy>,
                              public QtWayland::wp_color_manager_v1
{
public:
    explicit ColorManagementGlobal();
};

class ColorManagementSurface : public QObject, public QtWayland::wp_color_management_surface_v1
{
    Q_OBJECT
public:
    explicit ColorManagementSurface(::wp_color_management_surface_v1 *obj);
    ~ColorManagementSurface() override;

    void setImageDescription(::wp_image_description_v1 *descr);
};

class PendingImageDescription : public QtWayland::wp_image_description_v1
{
public:
    explicit PendingImageDescription(QQuickWindow *window,
                                     ColorManagementSurface *surface,
                                     ::wp_image_description_v1 *descr,
                                     HdrHelper::RenderIntent renderIntent);
    ~PendingImageDescription();

    void wp_image_description_v1_ready(uint32_t identity) override;

    QPointer<QQuickWindow> m_window;
    QPointer<ColorManagementSurface> m_surface;
    HdrHelper::RenderIntent m_renderIntent;
};
