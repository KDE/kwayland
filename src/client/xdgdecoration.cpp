/****************************************************************************
Copyright 2018  David Edmundson <kde@davidedmundson.co.uk>

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
#include "xdgdecoration.h"

#include "event_queue.h"
#include "wayland_pointer_p.h"
#include "xdgshell.h"

#include <QDebug>

#include "wayland-xdg-decoration-unstable-v1-client-protocol.h"

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN XdgDecorationManager::Private
{
public:
    Private() = default;

    void setup(zxdg_decoration_manager_v1 *arg);

    WaylandPointer<zxdg_decoration_manager_v1, zxdg_decoration_manager_v1_destroy> xdgdecorationmanager;
    EventQueue *queue = nullptr;
};

XdgDecorationManager::XdgDecorationManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void XdgDecorationManager::Private::setup(zxdg_decoration_manager_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!xdgdecorationmanager);
    xdgdecorationmanager.setup(arg);
}

XdgDecorationManager::~XdgDecorationManager()
{
    release();
}

void XdgDecorationManager::setup(zxdg_decoration_manager_v1 *xdgdecorationmanager)
{
    d->setup(xdgdecorationmanager);
}

void XdgDecorationManager::release()
{
    d->xdgdecorationmanager.release();
}

void XdgDecorationManager::destroy()
{
    d->xdgdecorationmanager.destroy();
}

XdgDecorationManager::operator zxdg_decoration_manager_v1*() {
    return d->xdgdecorationmanager;
}

XdgDecorationManager::operator zxdg_decoration_manager_v1*() const {
    return d->xdgdecorationmanager;
}

bool XdgDecorationManager::isValid() const
{
    return d->xdgdecorationmanager.isValid();
}

void XdgDecorationManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgDecorationManager::eventQueue()
{
    return d->queue;
}

XdgDecoration *XdgDecorationManager::getToplevelDecoration(XdgShellSurface *toplevel, QObject *parent)
{
    Q_ASSERT(isValid());
    xdg_toplevel *toplevel_resource = *toplevel;
    if (!toplevel_resource) { //i.e using XDGShellV5
        qWarning() << "Trying to create an XdgDecoration without an XDGShell stable toplevel object";
        return nullptr;
    }
    auto p = new XdgDecoration(parent);
    auto w = zxdg_decoration_manager_v1_get_toplevel_decoration(d->xdgdecorationmanager, toplevel_resource);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN XdgDecoration::Private
{
public:
    Private(XdgDecoration *q);

    void setup(zxdg_toplevel_decoration_v1 *arg);

    WaylandPointer<zxdg_toplevel_decoration_v1, zxdg_toplevel_decoration_v1_destroy> xdgdecoration;

    XdgDecoration::Mode m_mode = XdgDecoration::Mode::ClientSide;
private:
    XdgDecoration *q;

private:
    static void configureCallback(void *data, zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode);

    static const zxdg_toplevel_decoration_v1_listener s_listener;
};

const zxdg_toplevel_decoration_v1_listener XdgDecoration::Private::s_listener = {
    configureCallback
};

void XdgDecoration::Private::configureCallback(void *data, zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t m)
{
    auto p = reinterpret_cast<XdgDecoration::Private*>(data);
    Q_ASSERT(p->xdgdecoration == zxdg_toplevel_decoration_v1);
    switch (m) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
        p->m_mode = XdgDecoration::Mode::ClientSide;
        break;
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
        p->m_mode = XdgDecoration::Mode::ServerSide;
        break;
    }
    emit p->q->modeChanged(p->m_mode);
}

XdgDecoration::Private::Private(XdgDecoration *q)
    : q(q)
{
}

XdgDecoration::XdgDecoration(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void XdgDecoration::Private::setup(zxdg_toplevel_decoration_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!xdgdecoration);
    xdgdecoration.setup(arg);
    zxdg_toplevel_decoration_v1_add_listener(xdgdecoration, &s_listener, this);
}

XdgDecoration::~XdgDecoration()
{
    release();
}

void XdgDecoration::setup(zxdg_toplevel_decoration_v1 *xdgdecoration)
{
    d->setup(xdgdecoration);
}

void XdgDecoration::release()
{
    d->xdgdecoration.release();
}

void XdgDecoration::destroy()
{
    d->xdgdecoration.destroy();
}

XdgDecoration::operator zxdg_toplevel_decoration_v1*() {
    return d->xdgdecoration;
}

XdgDecoration::operator zxdg_toplevel_decoration_v1*() const {
    return d->xdgdecoration;
}

bool XdgDecoration::isValid() const
{
    return d->xdgdecoration.isValid();
}

void XdgDecoration::setMode(XdgDecoration::Mode mode)
{
    Q_ASSERT(isValid());
    uint32_t mode_raw;
    switch (mode) {
    case XdgDecoration::Mode::ClientSide:
        mode_raw = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
        break;
    default:
        mode_raw = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
        break;
    }
    zxdg_toplevel_decoration_v1_set_mode(d->xdgdecoration, mode_raw);
}

void XdgDecoration::unsetMode()
{
    Q_ASSERT(isValid());
    zxdg_toplevel_decoration_v1_unset_mode(d->xdgdecoration);
}

XdgDecoration::Mode XdgDecoration::mode() const
{
    return d->m_mode;
}


}
}

