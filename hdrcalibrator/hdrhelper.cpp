/*
    SPDX-FileCopyrightText: 2025 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "hdrhelper.h"

#include <QQuickWindow>
#include <qpa/qplatformwindow_p.h>
#include <wayland-client-protocol.h>

ColorManagementGlobal::ColorManagementGlobal()
    : QWaylandClientExtensionTemplate<ColorManagementGlobal, &QtWayland::wp_color_manager_v1::destroy>(1)
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

PendingImageDescription::PendingImageDescription(QQuickWindow *window,
                                                 ColorManagementSurface *surface,
                                                 ::wp_image_description_v1 *descr,
                                                 HdrHelper::RenderIntent renderIntent)
    : QtWayland::wp_image_description_v1(descr)
    , m_window(window)
    , m_surface(surface)
    , m_renderIntent(renderIntent)
{
}

PendingImageDescription::~PendingImageDescription()
{
    wp_image_description_v1_destroy(object());
}

void PendingImageDescription::wp_image_description_v1_ready([[maybe_unused]] uint32_t identity)
{
    if (m_window && m_surface) {
        if (m_renderIntent == HdrHelper::RenderIntent::Perceptual) {
            wp_color_management_surface_v1_set_image_description(m_surface->object(), object(), WP_COLOR_MANAGER_V1_RENDER_INTENT_PERCEPTUAL);
        } else {
            wp_color_management_surface_v1_set_image_description(m_surface->object(), object(), WP_COLOR_MANAGER_V1_RENDER_INTENT_RELATIVE_BPC);
        }
        m_window->requestUpdate();
    }
    delete this;
}

HdrHelper::HdrHelper()
    : m_global(std::make_unique<ColorManagementGlobal>())
{
    connect(m_global.get(), &ColorManagementGlobal::activeChanged, this, [this]() {
        for (const auto &[window, params] : m_surfaces) {
            setImageDescription(window);
        }
    });
}

HdrHelper::~HdrHelper()
{
}

void HdrHelper::setHdrParameters(QQuickWindow *window, Colorspace colorspace, uint32_t referenceLuminance, uint32_t maximumLuminance, RenderIntent renderIntent)
{
    if (!m_surfaces.contains(window)) {
        connect(window, &QObject::destroyed, this, [this, window]() {
            m_surfaces.erase(window);
        });
    }
    auto &params = m_surfaces[window];
    params.colorspace = colorspace;
    params.renderIntent = renderIntent;
    params.referenceLuminance = referenceLuminance;
    params.maximumLuminance = maximumLuminance;

    const auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow || !waylandWindow->surface()) {
        window->installEventFilter(this);
        return;
    }
    setImageDescription(window);
}

void HdrHelper::setImageDescription(QQuickWindow *window)
{
    auto &params = m_surfaces[window];
    if (!m_global->isActive()) {
        params.surface.reset();
        return;
    }
    const auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow || !waylandWindow->surface()) {
        qDebug() << "Wayland window or surface don't exist yet";
        return;
    }
    if (!params.surface) {
        params.surface = std::make_unique<ColorManagementSurface>(m_global->get_surface(waylandWindow->surface()));
    }
    auto creator = m_global->create_parametric_creator();
    if (params.colorspace == Colorspace::BT2020PQ) {
        wp_image_description_creator_params_v1_set_primaries_named(creator, QtWayland::wp_color_manager_v1::primaries_bt2020);
        wp_image_description_creator_params_v1_set_tf_named(creator, QtWayland::wp_color_manager_v1::transfer_function_st2084_pq);
    } else {
        wp_image_description_creator_params_v1_set_primaries_named(creator, QtWayland::wp_color_manager_v1::primaries_srgb);
        wp_image_description_creator_params_v1_set_tf_named(creator, QtWayland::wp_color_manager_v1::transfer_function_ext_linear);
    }
    wp_image_description_creator_params_v1_set_luminances(creator, 0, params.maximumLuminance, params.referenceLuminance);
    wp_image_description_creator_params_v1_set_mastering_luminance(creator, 0, params.maximumLuminance);
    new PendingImageDescription(window, params.surface.get(), wp_image_description_creator_params_v1_create(creator), params.renderIntent);
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
