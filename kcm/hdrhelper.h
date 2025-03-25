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

class ColorManagementGlobal : public QWaylandClientExtensionTemplate<ColorManagementGlobal>, public QtWayland::wp_color_manager_v1
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
    explicit PendingImageDescription(ColorManagementSurface *surface, ::wp_image_description_v1 *descr);
    ~PendingImageDescription();

    void wp_image_description_v1_ready(uint32_t identity) override;

    QPointer<ColorManagementSurface> m_surface;
};

class HdrHelper : public QObject
{
    Q_OBJECT
public:
    explicit HdrHelper();
    ~HdrHelper();

    void setHdrParameters(QQuickWindow *window, uint32_t referenceLuminance, uint32_t maximumLuminance);

private:
    void setImageDescription(QQuickWindow *window);
    bool eventFilter(QObject *watched, QEvent *event) override;

    const std::unique_ptr<ColorManagementGlobal> m_global;
    struct WindowParams {
        std::unique_ptr<ColorManagementSurface> surface;
        uint32_t referenceLuminance;
        uint32_t maximumLuminance;
    };
    std::unordered_map<QQuickWindow *, WindowParams> m_surfaces;
};
