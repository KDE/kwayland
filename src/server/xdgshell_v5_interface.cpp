/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "xdgshell_v5_interface_p.h"
#include "xdgshell_interface_p.h"
#include "generic_shell_surface_p.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "output_interface.h"
#include "seat_interface.h"
#include "surface_interface.h"

#include "../compat/wayland-xdg-shell-v5-server-protocol.h"

namespace KWayland
{
namespace Server
{

class XdgShellV5Interface::Private : public XdgShellInterface::Private
{
public:
    Private(XdgShellV5Interface *q, Display *d);

    QVector<XdgSurfaceV5Interface*> surfaces;

private:
    void createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource);
    void createPopup(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, SurfaceInterface *parent, SeatInterface *seat, quint32 serial, const QPoint &pos, wl_resource *parentResource);
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    quint32 ping(XdgShellSurfaceInterface * surface) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    QHash<wl_client *, wl_resource*> resources;

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void useUnstableVersionCallback(wl_client *client, wl_resource *resource, int32_t version);
    static void getXdgSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface);
    static void getXdgPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface, wl_resource * parent, wl_resource * seat, uint32_t serial, int32_t x, int32_t y);
    static void pongCallback(wl_client *client, wl_resource *resource, uint32_t serial);

    XdgShellV5Interface *q;
    static const struct zxdg_shell_v5_interface s_interface;
    static const quint32 s_version;
};

class XdgPopupV5Interface::Private : public XdgShellPopupInterface::Private
{
public:
    Private(XdgPopupV5Interface *q, XdgShellV5Interface *c, SurfaceInterface *surface, wl_resource *parentResource);
    ~Private();

    QRect windowGeometry() const override;
    void commit() override;
    void popupDone() override;

    XdgPopupV5Interface *q_func() {
        return reinterpret_cast<XdgPopupV5Interface *>(q);
    }

private:

    static const struct zxdg_popup_v5_interface s_interface;
};

const quint32 XdgShellV5Interface::Private::s_version = 1;

#ifndef K_DOXYGEN
const struct zxdg_shell_v5_interface XdgShellV5Interface::Private::s_interface = {
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
    // TODO: send protocol error if there are still surfaces mapped
    wl_resource_destroy(resource);
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
        wl_resource_post_error(surface->resource(), ZXDG_SHELL_V5_ERROR_ROLE, "ShellSurface already created");
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
    Q_EMIT q->surfaceCreated(shellSurface);
}

void XdgShellV5Interface::Private::getXdgPopupCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface, wl_resource * parent, wl_resource * seat, uint32_t serial, int32_t x, int32_t y)
{
    auto s = cast(resource);
    s->createPopup(client, wl_resource_get_version(resource), id, SurfaceInterface::get(surface), SurfaceInterface::get(parent), SeatInterface::get(seat), serial, QPoint(x, y), resource);
}

void XdgShellV5Interface::Private::createPopup(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, SurfaceInterface *parent, SeatInterface *seat, quint32 serial, const QPoint &pos, wl_resource *parentResource)
{
    XdgPopupV5Interface *popupSurface = new XdgPopupV5Interface(q, surface, parentResource);
    auto d = popupSurface->d_func();
    d->parent = QPointer<SurfaceInterface>(parent);
    d->anchorRect = QRect(pos, QSize(0,0));
    //default open like a normal popup
    d->anchorEdge = Qt::BottomEdge;
    d->gravity = Qt::TopEdge;
    d->create(display->getConnection(client), version, id);

    //compat
    Q_EMIT q->popupCreated(popupSurface, seat, serial);

    //new system
    Q_EMIT q->xdgPopupCreated(popupSurface);
    Q_EMIT popupSurface->grabRequested(seat, serial);
}

void XdgShellV5Interface::Private::pongCallback(wl_client *client, wl_resource *resource, uint32_t serial)
{
    Q_UNUSED(client)
    auto s = cast(resource);
    auto timerIt = s->pingTimers.find(serial);
    if (timerIt != s->pingTimers.end() && timerIt.value()->isActive()) {
        delete timerIt.value();
        s->pingTimers.erase(timerIt);
        Q_EMIT s->q->pongReceived(serial);
    }
}

XdgShellV5Interface::Private::Private(XdgShellV5Interface *q, Display *d)
    : XdgShellInterface::Private(XdgShellInterfaceVersion::UnstableV5, q, d, &zxdg_shell_v5_interface, s_version)
    , q(q)
{
}

void XdgShellV5Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    auto resource = c->createResource(&zxdg_shell_v5_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    resources[client] = resource;
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
}

void XdgShellV5Interface::Private::unbind(wl_resource *resource)
{
    auto s = cast(resource);
    auto client = wl_resource_get_client(resource);
    s->resources.remove(client);
}

XdgSurfaceV5Interface *XdgShellV5Interface::getSurface(wl_resource *resource)
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

quint32 XdgShellV5Interface::Private::ping(XdgShellSurfaceInterface * surface)
{
    auto client = surface->client()->client();
    //from here we can get the resource bound to our global.

    auto clientXdgShellResource = resources.value(client);
    if (!clientXdgShellResource) {
        return 0;
    }
    const quint32 pingSerial = display->nextSerial();
    zxdg_shell_v5_send_ping(clientXdgShellResource, pingSerial);

    setupTimer(pingSerial);
    return pingSerial;
}

XdgShellV5Interface::Private *XdgShellV5Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

class XdgSurfaceV5Interface::Private : public XdgShellSurfaceInterface::Private
{
public:
    Private(XdgSurfaceV5Interface *q, XdgShellV5Interface *c, SurfaceInterface *surface, wl_resource *parentResource);
    ~Private();

    QRect windowGeometry() const override;
    QSize minimumSize() const override;
    QSize maximumSize() const override;
    void close() override;
    void commit() override;
    quint32 configure(States states, const QSize &size) override;

    XdgSurfaceV5Interface *q_func() {
        return reinterpret_cast<XdgSurfaceV5Interface *>(q);
    }

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

    static const struct zxdg_surface_v5_interface s_interface;

    struct ShellSurfaceState
    {
        QRect windowGeometry;

        bool windowGeometryIsSet = false;
    };

    ShellSurfaceState m_currentState;
    ShellSurfaceState m_pendingState;
};

namespace {
template <>
Qt::Edges edgesToQtEdges(zxdg_surface_v5_resize_edge edges)
{
    Qt::Edges qtEdges;
    switch (edges) {
    case ZXDG_SURFACE_V5_RESIZE_EDGE_TOP:
        qtEdges = Qt::TopEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_BOTTOM:
        qtEdges = Qt::BottomEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_LEFT:
        qtEdges = Qt::LeftEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_TOP_LEFT:
        qtEdges = Qt::TopEdge | Qt::LeftEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_BOTTOM_LEFT:
        qtEdges = Qt::BottomEdge | Qt::LeftEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_RIGHT:
        qtEdges = Qt::RightEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_TOP_RIGHT:
        qtEdges = Qt::TopEdge | Qt::RightEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_BOTTOM_RIGHT:
        qtEdges = Qt::BottomEdge | Qt::RightEdge;
        break;
    case ZXDG_SURFACE_V5_RESIZE_EDGE_NONE:
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    return qtEdges;
}
}

#ifndef K_DOXYGEN
const struct zxdg_surface_v5_interface XdgSurfaceV5Interface::Private::s_interface = {
    resourceDestroyedCallback,
    setParentCallback,
    setTitleCallback,
    setAppIdCallback,
    showWindowMenuCallback,
    moveCallback,
    resizeCallback<zxdg_surface_v5_resize_edge>,
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
    auto parentSurface = static_cast<XdgShellV5Interface*>(s->q->global())->getSurface(parent);
    if (s->parent.data() != parentSurface) {
        s->parent = QPointer<XdgSurfaceV5Interface>(parentSurface);
        Q_EMIT s->q_func()->transientForChanged();
    }
}

void XdgSurfaceV5Interface::Private::showWindowMenuCallback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial, int32_t x, int32_t y)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    Q_EMIT s->q_func()->windowMenuRequested(SeatInterface::get(seat), serial, QPoint(x, y));
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
        Q_EMIT s->q_func()->configureAcknowledged(i);
        if (i == serial) {
            break;
        }
    }
}

void XdgSurfaceV5Interface::Private::setWindowGeometryCallback(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource, -1, "Tried to set invalid xdg-surface geometry");
        return;
    }
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->m_pendingState.windowGeometry = QRect(x, y, width, height);
    s->m_pendingState.windowGeometryIsSet = true;
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
    : XdgShellSurfaceInterface::Private(XdgShellInterfaceVersion::UnstableV5, q, c, surface, parentResource, &zxdg_surface_v5_interface, &s_interface)
{
}

XdgSurfaceV5Interface::Private::~Private() = default;

QRect XdgSurfaceV5Interface::Private::windowGeometry() const
{
    return m_currentState.windowGeometry;
}

QSize XdgSurfaceV5Interface::Private::minimumSize() const
{
    return QSize(0, 0);
}

QSize XdgSurfaceV5Interface::Private::maximumSize() const
{
    return QSize(INT_MAX, INT_MAX);
}

void XdgSurfaceV5Interface::Private::close()
{
    zxdg_surface_v5_send_close(resource);
    client->flush();
}

void XdgSurfaceV5Interface::Private::commit()
{
    const bool windowGeometryChanged = m_pendingState.windowGeometryIsSet;

    if (windowGeometryChanged) {
        m_currentState.windowGeometry = m_pendingState.windowGeometry;
    }

    m_pendingState = ShellSurfaceState{};

    if (windowGeometryChanged) {
        Q_EMIT q_func()->windowGeometryChanged(m_currentState.windowGeometry);
    }
}

quint32 XdgSurfaceV5Interface::Private::configure(States states, const QSize &size)
{
    if (!resource) {
        return 0;
    }
    const quint32 serial = global->display()->nextSerial();
    wl_array state;
    wl_array_init(&state);
    if (states.testFlag(State::Maximized)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = ZXDG_SURFACE_V5_STATE_MAXIMIZED;
    }
    if (states.testFlag(State::Fullscreen)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = ZXDG_SURFACE_V5_STATE_FULLSCREEN;
    }
    if (states.testFlag(State::Resizing)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = ZXDG_SURFACE_V5_STATE_RESIZING;
    }
    if (states.testFlag(State::Activated)) {
        uint32_t *s = reinterpret_cast<uint32_t*>(wl_array_add(&state, sizeof(uint32_t)));
        *s = ZXDG_SURFACE_V5_STATE_ACTIVATED;
    }
    configureSerials << serial;
    zxdg_surface_v5_send_configure(resource, size.width(), size.height(), &state, serial);
    client->flush();
    wl_array_release(&state);

    return serial;
}

#ifndef K_DOXYGEN
const struct zxdg_popup_v5_interface XdgPopupV5Interface::Private::s_interface = {
    resourceDestroyedCallback
};
#endif

XdgPopupV5Interface::Private::Private(XdgPopupV5Interface *q, XdgShellV5Interface *c, SurfaceInterface *surface, wl_resource *parentResource)
    : XdgShellPopupInterface::Private(XdgShellInterfaceVersion::UnstableV5, q, c, surface, parentResource, &zxdg_popup_v5_interface, &s_interface)
{
}

XdgPopupV5Interface::Private::~Private() = default;

QRect XdgPopupV5Interface::Private::windowGeometry() const
{
    return QRect();
}

void XdgPopupV5Interface::Private::commit()
{
}

void XdgPopupV5Interface::Private::popupDone()
{
    if (!resource) {
        return;
    }
    // TODO: dismiss all child popups
    zxdg_popup_v5_send_popup_done(resource);
    client->flush();
}

XdgShellV5Interface::XdgShellV5Interface(Display *display, QObject *parent)
    : XdgShellInterface(new Private(this, display), parent)
{
}

XdgShellV5Interface::~XdgShellV5Interface() = default;

XdgSurfaceV5Interface::XdgSurfaceV5Interface(XdgShellV5Interface *parent, SurfaceInterface *surface, wl_resource *parentResource)
    : KWayland::Server::XdgShellSurfaceInterface(new Private(this, parent, surface, parentResource))
{
}

XdgSurfaceV5Interface::~XdgSurfaceV5Interface() = default;

XdgPopupV5Interface::XdgPopupV5Interface(XdgShellV5Interface *parent, SurfaceInterface *surface, wl_resource *parentResource)
    : XdgShellPopupInterface(new Private(this, parent, surface, parentResource))
{
}

XdgPopupV5Interface::~XdgPopupV5Interface() = default;

XdgPopupV5Interface::Private *XdgPopupV5Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

}
}

