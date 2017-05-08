/****************************************************************************
Copyright 2017  David Edmundson <davidedmundson@kde.org>

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
#include "xdgshell_v6_interface_p.h"
#include "xdgshell_interface_p.h"
#include "generic_shell_surface_p.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "output_interface.h"
#include "seat_interface.h"
#include "surface_interface.h"

#include <wayland-xdg-shell-v6-server-protocol.h>

namespace KWayland
{
namespace Server
{

class XdgShellV6Interface::Private : public XdgShellInterface::Private
{
public:
    Private(XdgShellV6Interface *q, Display *d);

    QVector<XdgSurfaceV6Interface*> surfaces;

private:
    void createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource);
    void createPopup(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, SurfaceInterface *parent, SeatInterface *seat, quint32 serial, const QPoint &pos, wl_resource *parentResource);
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void createPositionerCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void getXdgSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface);
    static void pongCallback(wl_client *client, wl_resource *resource, uint32_t serial);

    XdgShellV6Interface *q;
    static const struct zxdg_shell_v6_interface s_interface;
    static const quint32 s_version;
};

class XdgPopupV6Interface::Private : public XdgShellPopupInterface::Private
{
public:
    Private(XdgPopupV6Interface *q, XdgShellV6Interface *c, SurfaceInterface *surface, wl_resource *parentResource);
    ~Private();

    void popupDone() override;

    XdgPopupV6Interface *q_func() {
        return reinterpret_cast<XdgPopupV6Interface *>(q);
    }

private:

    static const struct zxdg_popup_v6_interface s_interface;
};

const quint32 XdgShellV6Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_shell_v6_interface XdgShellV6Interface::Private::s_interface = {
    destroyCallback,
    createPositionerCallback,
    getXdgSurfaceCallback,
    pongCallback
};
#endif

void XdgShellV6Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    // TODO: send protocol error if there are still surfaces mapped
    wl_resource_destroy(resource);
}

void XdgShellV6Interface::Private::createPositionerCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    qDebug() << "creating positioner - not implemented";
    //FIXME
}

void XdgShellV6Interface::Private::getXdgSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface)
{
    auto s = cast(resource);
    s->createSurface(client, wl_resource_get_version(resource), id, SurfaceInterface::get(surface), resource);
}

void XdgShellV6Interface::Private::createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource)
{
    auto it = std::find_if(surfaces.constBegin(), surfaces.constEnd(),
        [surface](XdgSurfaceV6Interface *s) {
            return surface == s->surface();
        }
    );
    if (it != surfaces.constEnd()) {
        wl_resource_post_error(surface->resource(), ZXDG_SHELL_V6_ERROR_ROLE, "ShellSurface already created");
        return;
    }
    XdgSurfaceV6Interface *shellSurface = new XdgSurfaceV6Interface(q, surface, parentResource);
    surfaces << shellSurface;
    QObject::connect(shellSurface, &XdgSurfaceV6Interface::destroyed, q,
        [this, shellSurface] {
            surfaces.removeAll(shellSurface);
        }
    );
    shellSurface->d->create(display->getConnection(client), version, id);
    emit q->surfaceCreated(shellSurface);
}

void XdgShellV6Interface::Private::pongCallback(wl_client *client, wl_resource *resource, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
    // TODO: implement
}

XdgShellV6Interface::Private::Private(XdgShellV6Interface *q, Display *d)
    : XdgShellInterface::Private(XdgShellInterfaceVersion::UnstableV6, q, d, &zxdg_shell_v6_interface, 1)
    , q(q)
{
}

void XdgShellV6Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zxdg_shell_v6_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track, yes we need to track to be able to ping!
}

void XdgShellV6Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

XdgSurfaceV6Interface *XdgShellV6Interface::getSurface(wl_resource *resource)
{
    if (!resource) {
        return nullptr;
    }
    Q_D();
    auto it = std::find_if(d->surfaces.constBegin(), d->surfaces.constEnd(),
                           [resource] (XdgSurfaceV6Interface *surface) {
                               return surface->resource() == resource;
                            }
                          );
    if (it != d->surfaces.constEnd()) {
        return *it;
    }
    return nullptr;
}

XdgShellV6Interface::Private *XdgShellV6Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

class XdgSurfaceV6Interface::Private : public XdgShellSurfaceInterface::Private
{
public:
    Private(XdgSurfaceV6Interface* q, XdgShellV6Interface* c, SurfaceInterface* surface, wl_resource* parentResource);
    ~Private();

    void close() override;
    quint32 configure(States states, const QSize &size) override;

    XdgSurfaceV6Interface *q_func() {
        return reinterpret_cast<XdgSurfaceV6Interface *>(q);
    }

private:
    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void getTopLevelCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void getPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *parent, wl_resource *positioner);
    static void ackConfigureCallback(wl_client *client, wl_resource *resource, uint32_t serial);
    static void setWindowGeometryCallback(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
    static void setMaximizedCallback(wl_client *client, wl_resource *resource);
    static const struct zxdg_surface_v6_interface s_interface;
};

// namespace {
// template <>
// Qt::Edges edgesToQtEdges(zxdg_toplevel_v6_resize_edge edges)
// {
//     Qt::Edges qtEdges;
//     switch (edges) {
//     case XDG_SURFACE_RESIZE_EDGE_TOP:
//         qtEdges = Qt::TopEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_BOTTOM:
//         qtEdges = Qt::BottomEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_LEFT:
//         qtEdges = Qt::LeftEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_TOP_LEFT:
//         qtEdges = Qt::TopEdge | Qt::LeftEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT:
//         qtEdges = Qt::BottomEdge | Qt::LeftEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_RIGHT:
//         qtEdges = Qt::RightEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT:
//         qtEdges = Qt::TopEdge | Qt::RightEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT:
//         qtEdges = Qt::BottomEdge | Qt::RightEdge;
//         break;
//     case XDG_SURFACE_RESIZE_EDGE_NONE:
//         break;
//     default:
//         Q_UNREACHABLE();
//         break;
//     }
//     return qtEdges;
// }
// }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_surface_v6_interface XdgSurfaceV6Interface::Private::s_interface = {
    destroyCallback,
    getTopLevelCallback,
    getPopupCallback,
    setWindowGeometryCallback,
    ackConfigureCallback
};
#endif

void XdgSurfaceV6Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{}

void XdgSurfaceV6Interface::Private::getTopLevelCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    qDebug() << "get top level - not implemented";

}

void XdgSurfaceV6Interface::Private::getPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *parent, wl_resource *positioner)
{
    qDebug() << "get popup - not implemented";
}

void XdgSurfaceV6Interface::Private::ackConfigureCallback(wl_client *client, wl_resource *resource, uint32_t serial)
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

void XdgSurfaceV6Interface::Private::setWindowGeometryCallback(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    // TODO: implement
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void XdgSurfaceV6Interface::Private::setMaximizedCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->q_func()->maximizedChanged(true);
}

XdgSurfaceV6Interface::Private::Private(XdgSurfaceV6Interface *q, XdgShellV6Interface *c, SurfaceInterface *surface, wl_resource *parentResource)
    : XdgShellSurfaceInterface::Private(XdgShellInterfaceVersion::UnstableV6, q, c, surface, parentResource, &zxdg_surface_v6_interface, &s_interface)
{
}

XdgSurfaceV6Interface::Private::~Private() = default;

void XdgSurfaceV6Interface::Private::close()
{
//     zxdg_surface_send_close(resource);
    client->flush();
}

quint32 XdgSurfaceV6Interface::Private::configure(States states, const QSize &size)
{
    if (!resource) {
        return 0;
    }
//     const quint32 serial = global->display()->nextSerial();
//     wl_array state;
//     wl_array_init(&state);
//     if (states.testFlag(State::Maximized)) {
//         uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
//         *s = XDG_SURFACE_STATE_MAXIMIZED;
//     }
//     if (states.testFlag(State::Fullscreen)) {
//         uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
//         *s = XDG_SURFACE_STATE_FULLSCREEN;
//     }
//     if (states.testFlag(State::Resizing)) {
//         uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
//         *s = XDG_SURFACE_STATE_RESIZING;
//     }
//     if (states.testFlag(State::Activated)) {
//         uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
//         *s = XDG_SURFACE_STATE_ACTIVATED;
//     }
//     configureSerials << serial;
//     xdg_surface_send_configure(resource, size.width(), size.height(), &state, serial);
//     client->flush();
//     wl_array_release(&state);
//  return serial;

    return 0;
}

class XdgTopLevelV6Interface::Private : public XdgShellSurfaceInterface::Private
{
public:
    Private(XdgTopLevelV6Interface* q, XdgShellV6Interface* c, SurfaceInterface* surface, wl_resource* parentResource);
    ~Private();

    void close() override {};
    quint32 configure(States states, const QSize &size) override {
        qDebug() << "nope";
        return 0;
    };

    XdgTopLevelV6Interface *q_func() {
        return reinterpret_cast<XdgTopLevelV6Interface*>(q);
    }

private:
//     static void destroyCallback(wl_client *client, wl_resource *resource);
//     static void getTopLevelCallback(wl_client *client, wl_resource *resource, uint32_t id);
//     static void getPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *parent, wl_resource *positioner);
//     static void ackConfigureCallback(wl_client *client, wl_resource *resource, uint32_t serial);
//     static void setWindowGeometryCallback(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
//     static void setMaximizedCallback(wl_client *client, wl_resource *resource);
    static const struct zxdg_toplevel_v6_interface s_interface;
};

const struct zxdg_toplevel_v6_interface XdgTopLevelV6Interface::Private::s_interface = {
};

XdgTopLevelV6Interface::Private::Private(XdgTopLevelV6Interface *q, XdgShellV6Interface *c, SurfaceInterface *surface, wl_resource *parentResource)
    : XdgShellSurfaceInterface::Private(XdgShellInterfaceVersion::UnstableV6, q, c, surface, parentResource, &zxdg_surface_v6_interface, &s_interface)
{
}

XdgTopLevelV6Interface::Private::~Private() = default;



#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_popup_v6_interface XdgPopupV6Interface::Private::s_interface = {
    resourceDestroyedCallback
};
#endif

XdgPopupV6Interface::Private::Private(XdgPopupV6Interface *q, XdgShellV6Interface *c, SurfaceInterface *surface, wl_resource *parentResource)
    : XdgShellPopupInterface::Private(XdgShellInterfaceVersion::UnstableV6, q, c, surface, parentResource, &zxdg_popup_v6_interface, &s_interface)
{
}

XdgPopupV6Interface::Private::~Private() = default;


void XdgPopupV6Interface::Private::popupDone()
{
    if (!resource) {
        return;
    }
    // TODO: dismiss all child popups
    zxdg_popup_v6_send_popup_done(resource);
    client->flush();
}

XdgShellV6Interface::XdgShellV6Interface(Display *display, QObject *parent)
    : XdgShellInterface(new Private(this, display), parent)
{
}

XdgShellV6Interface::~XdgShellV6Interface() = default;

XdgSurfaceV6Interface::XdgSurfaceV6Interface(XdgShellV6Interface *parent, SurfaceInterface *surface, wl_resource *parentResource)
    : KWayland::Server::XdgShellSurfaceInterface(new Private(this, parent, surface, parentResource))
{
}

XdgSurfaceV6Interface::~XdgSurfaceV6Interface() = default;


XdgTopLevelV6Interface::XdgTopLevelV6Interface(XdgShellV6Interface *parent, SurfaceInterface *surface, wl_resource *parentResource)
    : KWayland::Server::XdgShellSurfaceInterface(new Private(this, parent, surface, parentResource))
{
}

XdgTopLevelV6Interface::~XdgTopLevelV6Interface() = default;

XdgPopupV6Interface::XdgPopupV6Interface(XdgShellV6Interface *parent, SurfaceInterface *surface, wl_resource *parentResource)
    : XdgShellPopupInterface(new Private(this, parent, surface, parentResource))
{
}


XdgPopupV6Interface::~XdgPopupV6Interface() = default;

XdgPopupV6Interface::Private *XdgPopupV6Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

}
}

