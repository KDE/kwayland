/****************************************************************************
Copyright 2016  Martin Gräßlin <mgraesslin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/
#include "xdg_shell_v5_interface.h"
#include "generic_shell_surface_p.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "output_interface.h"
#include "seat_interface.h"
#include "surface_interface.h"

#include <wayland-xdg-shell-v5-server-protocol.h>

namespace KWayland
{
namespace Server
{

class XdgShellV5Interface::Private : public Global::Private
{
public:
    Private(XdgShellV5Interface *q, Display *d);

    QVector<XdgSurfaceV5Interface*> surfaces;

private:
    void createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource);
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void useUnstableVersionCallback(wl_client *client, wl_resource *resource, int32_t version);
    static void getXdgSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface);
    static void getXdgPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface, wl_resource * parent, wl_resource * seat, uint32_t serial, int32_t x, int32_t y);
    static void pongCallback(wl_client *client, wl_resource *resource, uint32_t serial);

    XdgShellV5Interface *q;
    static const struct xdg_shell_interface s_interface;
    static const quint32 s_version;
};

const quint32 XdgShellV5Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct xdg_shell_interface XdgShellV5Interface::Private::s_interface = {
    destroyCallback,
    useUnstableVersionCallback,
    getXdgSurfaceCallback,
    getXdgPopupCallback,
    pongCallback
};
#endif

void XdgShellV5Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    // TODO: implement
}

void XdgShellV5Interface::Private::useUnstableVersionCallback(wl_client *client, wl_resource *resource, int32_t version)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(version)
    // TODO: implement
}

void XdgShellV5Interface::Private::getXdgSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface)
{
    auto s = cast(resource);
    s->createSurface(client, wl_resource_get_version(resource), id, SurfaceInterface::get(surface), resource);
}

void XdgShellV5Interface::Private::createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource)
{
    auto it = std::find_if(surfaces.constBegin(), surfaces.constEnd(),
        [surface](XdgSurfaceV5Interface *s) {
            return surface == s->surface();
        }
    );
    if (it != surfaces.constEnd()) {
        wl_resource_post_error(surface->resource(), XDG_SHELL_ERROR_ROLE, "ShellSurface already created");
        return;
    }
    XdgSurfaceV5Interface *shellSurface = new XdgSurfaceV5Interface(q, surface, parentResource);
    surfaces << shellSurface;
    QObject::connect(shellSurface, &XdgSurfaceV5Interface::destroyed, q,
        [this, shellSurface] {
            surfaces.removeAll(shellSurface);
        }
    );
    shellSurface->d->create(display->getConnection(client), version, id);
    emit q->surfaceCreated(shellSurface);
}

void XdgShellV5Interface::Private::getXdgPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface, wl_resource * parent, wl_resource * seat, uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(id)
    Q_UNUSED(surface)
    Q_UNUSED(parent)
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    Q_UNUSED(x)
    Q_UNUSED(y)
    // TODO: implement
}

void XdgShellV5Interface::Private::pongCallback(wl_client *client, wl_resource *resource, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
    // TODO: implement
}

XdgShellV5Interface::Private::Private(XdgShellV5Interface *q, Display *d)
    : Global::Private(d, &xdg_shell_interface, s_version)
    , q(q)
{
}

void XdgShellV5Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&xdg_shell_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track, yes we need to track to be able to ping!
}

void XdgShellV5Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

XdgSurfaceV5Interface *XdgShellV5Interface::get(wl_resource *resource)
{
    if (!resource) {
        return nullptr;
    }
    Q_D();
    auto it = std::find_if(d->surfaces.constBegin(), d->surfaces.constEnd(),
                           [resource] (XdgSurfaceV5Interface *surface) {
                               return surface->resource() == resource;
                            }
                          );
    if (it != d->surfaces.constEnd()) {
        return *it;
    }
    return nullptr;
}

XdgShellV5Interface::Private *XdgShellV5Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

class XdgSurfaceV5Interface::Private : public Resource::Private, public GenericShellSurface<XdgSurfaceV5Interface>
{
public:
    Private(XdgSurfaceV5Interface *q, XdgShellV5Interface *c, SurfaceInterface *surface, wl_resource *parentResource);
    ~Private();

    XdgSurfaceV5Interface *q_func() {
        return reinterpret_cast<XdgSurfaceV5Interface *>(q);
    }

    QVector<quint32> configureSerials;
    QPointer<XdgSurfaceV5Interface> parent;

private:
    static void setParentCallback(wl_client *client, wl_resource *resource, wl_resource * parent);
    static void showWindowMenuCallback(wl_client *client, wl_resource *resource, wl_resource * seat, uint32_t serial, int32_t x, int32_t y);
    static void ackConfigureCallback(wl_client *client, wl_resource *resource, uint32_t serial);
    static void setWindowGeometryCallback(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
    static void setMaximizedCallback(wl_client *client, wl_resource *resource);
    static void unsetMaximizedCallback(wl_client *client, wl_resource *resource);
    static void setFullscreenCallback(wl_client *client, wl_resource *resource, wl_resource * output);
    static void unsetFullscreenCallback(wl_client *client, wl_resource *resource);
    static void setMinimizedCallback(wl_client *client, wl_resource *resource);

    static const struct xdg_surface_interface s_interface;
};

namespace {
template <>
Qt::Edges edgesToQtEdges(xdg_surface_resize_edge edges)
{
    Qt::Edges qtEdges;
    switch (edges) {
    case XDG_SURFACE_RESIZE_EDGE_TOP:
        qtEdges = Qt::TopEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_BOTTOM:
        qtEdges = Qt::BottomEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_LEFT:
        qtEdges = Qt::LeftEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_TOP_LEFT:
        qtEdges = Qt::TopEdge | Qt::LeftEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT:
        qtEdges = Qt::BottomEdge | Qt::LeftEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_RIGHT:
        qtEdges = Qt::RightEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT:
        qtEdges = Qt::TopEdge | Qt::RightEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT:
        qtEdges = Qt::BottomEdge | Qt::RightEdge;
        break;
    case XDG_SURFACE_RESIZE_EDGE_NONE:
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    return qtEdges;
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct xdg_surface_interface XdgSurfaceV5Interface::Private::s_interface = {
    destroyCallback,
    setParentCallback,
    setTitleCallback,
    setAppIdCallback,
    showWindowMenuCallback,
    moveCallback,
    resizeCallback<xdg_surface_resize_edge>,
    ackConfigureCallback,
    setWindowGeometryCallback,
    setMaximizedCallback,
    unsetMaximizedCallback,
    setFullscreenCallback,
    unsetFullscreenCallback,
    setMinimizedCallback
};
#endif

void XdgSurfaceV5Interface::Private::setParentCallback(wl_client *client, wl_resource *resource, wl_resource *parent)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    auto parentSurface = static_cast<XdgShellV5Interface*>(s->q->global())->get(parent);
    if (s->parent.data() != parentSurface) {
        s->parent = QPointer<XdgSurfaceV5Interface>(parentSurface);
        emit s->q_func()->transientForChanged();
    }
}

void XdgSurfaceV5Interface::Private::showWindowMenuCallback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial, int32_t x, int32_t y)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    emit s->q_func()->windowMenuRequested(SeatInterface::get(seat), serial, QPoint(x, y));
}

void XdgSurfaceV5Interface::Private::ackConfigureCallback(wl_client *client, wl_resource *resource, uint32_t serial)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    if (!s->configureSerials.contains(serial)) {
        // TODO: send error?
        return;
    }
    while (!s->configureSerials.isEmpty()) {
        quint32 i = s->configureSerials.takeFirst();
        emit s->q_func()->configureAcknowledged(i);
        if (i == serial) {
            break;
        }
    }
}

void XdgSurfaceV5Interface::Private::setWindowGeometryCallback(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    // TODO: implement
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void XdgSurfaceV5Interface::Private::setMaximizedCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->q_func()->maximizedChanged(true);
}

void XdgSurfaceV5Interface::Private::unsetMaximizedCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->q_func()->maximizedChanged(false);
}

void XdgSurfaceV5Interface::Private::setFullscreenCallback(wl_client *client, wl_resource *resource, wl_resource *output)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    OutputInterface *o = nullptr;
    if (output) {
        o = OutputInterface::get(output);
    }
    s->q_func()->fullscreenChanged(true, o);
}

void XdgSurfaceV5Interface::Private::unsetFullscreenCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->q_func()->fullscreenChanged(false, nullptr);
}

void XdgSurfaceV5Interface::Private::setMinimizedCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->q_func()->minimizeRequested();
}

XdgSurfaceV5Interface::Private::Private(XdgSurfaceV5Interface *q, XdgShellV5Interface *c, SurfaceInterface *surface, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &xdg_surface_interface, &s_interface)
    , GenericShellSurface<XdgSurfaceV5Interface>(q, surface)
{
}

XdgSurfaceV5Interface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}
class XdgPopupV5Interface::Private : public Resource::Private
{
public:
    Private(XdgPopupV5Interface *q, XdgShellV5Interface *c, wl_resource *parentResource);
    ~Private();

private:
    static void destroyCallback(wl_client *client, wl_resource *resource);

    XdgPopupV5Interface *q_func() {
        return reinterpret_cast<XdgPopupV5Interface *>(q);
    }

    static const struct xdg_popup_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct xdg_popup_interface XdgPopupV5Interface::Private::s_interface = {
    destroyCallback
};
#endif

void XdgPopupV5Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client);
    Private *p = cast<Private>(resource);
    wl_resource_destroy(resource);
    p->q->deleteLater();
}

XdgPopupV5Interface::Private::Private(XdgPopupV5Interface *q, XdgShellV5Interface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &xdg_popup_interface, &s_interface)
{
}

XdgPopupV5Interface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

XdgShellV5Interface::XdgShellV5Interface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

XdgShellV5Interface::~XdgShellV5Interface() = default;

XdgSurfaceV5Interface::XdgSurfaceV5Interface(XdgShellV5Interface *parent, SurfaceInterface *surface, wl_resource *parentResource)
    : Resource(new Private(this, parent, surface, parentResource), surface)
{
}

XdgSurfaceV5Interface::~XdgSurfaceV5Interface() = default;

SurfaceInterface *XdgSurfaceV5Interface::surface() const
{
    Q_D();
    return d->surface;
}

QString XdgSurfaceV5Interface::title() const
{
    Q_D();
    return d->title;
}

QByteArray XdgSurfaceV5Interface::windowClass() const
{
    Q_D();
    return d->windowClass;
}

void XdgSurfaceV5Interface::close()
{
    Q_D();
    xdg_surface_send_close(d->resource);
    d->client->flush();
}

quint32 XdgSurfaceV5Interface::configure(States states, const QSize &size)
{
    Q_D();
    if (!d->resource) {
        return 0;
    }
    const quint32 serial = d->global->display()->nextSerial();
    wl_array state;
    wl_array_init(&state);
    if (states.testFlag(State::Maximized)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = XDG_SURFACE_STATE_MAXIMIZED;
    }
    if (states.testFlag(State::Fullscreen)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = XDG_SURFACE_STATE_FULLSCREEN;
    }
    if (states.testFlag(State::Resizing)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = XDG_SURFACE_STATE_RESIZING;
    }
    if (states.testFlag(State::Activated)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = XDG_SURFACE_STATE_ACTIVATED;
    }
    d->configureSerials << serial;
    xdg_surface_send_configure(d->resource, size.width(), size.height(), &state, serial);
    d->client->flush();
    wl_array_release(&state);

    return serial;
}

bool XdgSurfaceV5Interface::isConfigurePending() const
{
    Q_D();
    return !d->configureSerials.isEmpty();
}

bool XdgSurfaceV5Interface::isTransient() const
{
    Q_D();
    return !d->parent.isNull();
}

QPointer<XdgSurfaceV5Interface> XdgSurfaceV5Interface::transientFor() const
{
    Q_D();
    return d->parent;
}

XdgSurfaceV5Interface::Private *XdgSurfaceV5Interface::d_func() const
{
    return reinterpret_cast<XdgSurfaceV5Interface::Private*>(d.data());
}

XdgPopupV5Interface::~XdgPopupV5Interface() = default;

}
}

