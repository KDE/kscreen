/*
    SPDX-FileCopyrightText: 2025 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "hdrhelper.h"

#include <QQuickWindow>
#include <qpa/qplatformwindow_p.h>

ColorManagementGlobal::ColorManagementGlobal()
    : QWaylandClientExtensionTemplate<ColorManagementGlobal>(1)
{
    initialize();
    connect(this, &ColorManagementGlobal::activeChanged, this, [this]() {
        if (!isActive()) {
            wp_color_manager_v1_destroy(object());
        }
    });
}

ColorManagementSurface::ColorManagementSurface(::wp_color_management_surface_v1 *obj)
    : QtWayland::wp_color_management_surface_v1(obj)
{
}

ColorManagementSurface::~ColorManagementSurface()
{
    wp_color_management_surface_v1_destroy(object());
}

PendingImageDescription::PendingImageDescription(ColorManagementSurface *surface, ::wp_image_description_v1 *descr)
    : QtWayland::wp_image_description_v1(descr)
    , m_surface(surface)
{
}

PendingImageDescription::~PendingImageDescription()
{
    wp_image_description_v1_destroy(object());
}

void PendingImageDescription::wp_image_description_v1_ready([[maybe_unused]] uint32_t identity)
{
    if (m_surface) {
        wp_color_management_surface_v1_set_image_description(m_surface->object(), object(), WP_COLOR_MANAGER_V1_RENDER_INTENT_RELATIVE_BPC);
    }
    delete this;
}

HdrHelper::HdrHelper()
    : m_global(std::make_unique<ColorManagementGlobal>())
{
}

HdrHelper::~HdrHelper()
{
}

void HdrHelper::setHdrParameters(QQuickWindow *window, uint32_t referenceLuminance, uint32_t maximumLuminance)
{
    if (!m_surfaces.contains(window)) {
        connect(window, &QObject::destroyed, this, [this, window]() {
            m_surfaces.erase(window);
        });
    }
    auto &params = m_surfaces[window];
    params.referenceLuminance = referenceLuminance;
    params.maximumLuminance = maximumLuminance;

    auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow || !waylandWindow->surface()) {
        window->installEventFilter(this);
        return;
    }
    setImageDescription(window);
}

void HdrHelper::setImageDescription(QQuickWindow *window)
{
    const auto initWayland = [this, window]() {
        auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
        auto &params = m_surfaces[window];
        if (!m_global->isActive()) {
            params.surface.reset();
            return;
        }
        if (!params.surface) {
            params.surface = std::make_unique<ColorManagementSurface>(m_global->get_surface(waylandWindow->surface()));
        }
        auto creator = m_global->create_parametric_creator();
        wp_image_description_creator_params_v1_set_primaries_named(creator, QtWayland::wp_color_manager_v1::primaries_srgb);
        wp_image_description_creator_params_v1_set_tf_named(creator, QtWayland::wp_color_manager_v1::transfer_function_gamma22);
        wp_image_description_creator_params_v1_set_luminances(creator, 0, params.maximumLuminance, params.referenceLuminance);
        wp_image_description_creator_params_v1_set_mastering_luminance(creator, 0, params.maximumLuminance);
        new PendingImageDescription(params.surface.get(), wp_image_description_creator_params_v1_create(creator));
    };
    if (m_global->isActive()) {
        initWayland();
    }
    connect(m_global.get(), &ColorManagementGlobal::activeChanged, this, initWayland);
}

bool HdrHelper::eventFilter(QObject *watched, QEvent *event)
{
    auto window = qobject_cast<QQuickWindow *>(watched);
    if (!window) {
        return false;
    }
    if (event->type() == QEvent::PlatformSurface) {
        auto surfaceEvent = static_cast<QPlatformSurfaceEvent *>(event);
        if (surfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            setImageDescription(window);
        }
    }
    return false;
}

#include "moc_hdrhelper.cpp"
