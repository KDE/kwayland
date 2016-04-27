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
#include "xdg_shell_v5.h"
#include "event_queue.h"
#include "../../src/client/wayland_pointer_p.h"
#include "../../src/client/seat.h"
#include "../../src/client/surface.h"
#include "../../src/client/output.h"
#include <wayland-xdg-shell-v5-client-protocol.h>

namespace KWayland
{
namespace Client
{

class XdgShellV5::Private
{
public:
    Private() = default;

    WaylandPointer<xdg_shell, xdg_shell_destroy> xdgshellv5;
    EventQueue *queue = nullptr;
};

XdgShellV5::XdgShellV5(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

XdgShellV5::~XdgShellV5()
{
    release();
}

void XdgShellV5::setup(xdg_shell *xdgshellv5)
{
    Q_ASSERT(xdgshellv5);
    Q_ASSERT(!d->xdgshellv5);
    d->xdgshellv5.setup(xdgshellv5);
}

void XdgShellV5::release()
{
    d->xdgshellv5.release();
}

void XdgShellV5::destroy()
{
    d->xdgshellv5.destroy();
}

void XdgShellV5::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgShellV5::eventQueue()
{
    return d->queue;
}

XdgShellV5::operator xdg_shell*() {
    return d->xdgshellv5;
}

XdgShellV5::operator xdg_shell*() const {
    return d->xdgshellv5;
}

bool XdgShellV5::isValid() const
{
    return d->xdgshellv5.isValid();
}

void XdgShellV5::useUnstableVersion(qint32 version)
{
    Q_UNUSED(version)
}

XdgSurfaceV5 *XdgShellV5::getXdgSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    XdgSurfaceV5 *s = new XdgSurfaceV5(parent);
    auto w = xdg_shell_get_xdg_surface(d->xdgshellv5, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

XdgPopupV5 *XdgShellV5::getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, qint32 x, qint32 y, QObject *parent)
{
    return nullptr;
}

void XdgShellV5::pong(quint32 serial)
{
    Q_UNUSED(serial)
}

class XdgSurfaceV5::Private
{
public:
    Private() = default;

    WaylandPointer<xdg_surface, xdg_surface_destroy> xdgsurfacev5;
    EventQueue *queue = nullptr;
};

XdgSurfaceV5::XdgSurfaceV5(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

XdgSurfaceV5::~XdgSurfaceV5()
{
    release();
}

void XdgSurfaceV5::setup(xdg_surface *xdgsurfacev5)
{
    Q_ASSERT(xdgsurfacev5);
    Q_ASSERT(!d->xdgsurfacev5);
    d->xdgsurfacev5.setup(xdgsurfacev5);
}

void XdgSurfaceV5::release()
{
    d->xdgsurfacev5.release();
}

void XdgSurfaceV5::destroy()
{
    d->xdgsurfacev5.destroy();
}

void XdgSurfaceV5::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgSurfaceV5::eventQueue()
{
    return d->queue;
}

XdgSurfaceV5::operator xdg_surface*() {
    return d->xdgsurfacev5;
}

XdgSurfaceV5::operator xdg_surface*() const {
    return d->xdgsurfacev5;
}

bool XdgSurfaceV5::isValid() const
{
    return d->xdgsurfacev5.isValid();
}

void XdgSurfaceV5::setTransientFor(XdgSurfaceV5 *parent)
{
    xdg_surface *parentSurface = nullptr;
    if (parent) {
        parentSurface = *parent;
    }
    xdg_surface_set_parent(d->xdgsurfacev5, parentSurface);
}

void XdgSurfaceV5::setTitle(const QString & title)
{
    xdg_surface_set_title(d->xdgsurfacev5, title.toUtf8().constData());
}

void XdgSurfaceV5::setAppId(const QByteArray & appId)
{
    xdg_surface_set_app_id(d->xdgsurfacev5, appId.constData());
}

void XdgSurfaceV5::showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y)
{
    xdg_surface_show_window_menu(d->xdgsurfacev5, *seat, serial, x, y);
}

void XdgSurfaceV5::move(Seat *seat, quint32 serial)
{
    xdg_surface_move(d->xdgsurfacev5, *seat, serial);
}

void XdgSurfaceV5::resize(Seat *seat, quint32 serial, quint32 edges)
{
    xdg_surface_resize(d->xdgsurfacev5, *seat, serial, edges);
}

void XdgSurfaceV5::ackConfigure(quint32 serial)
{
}

void XdgSurfaceV5::setWindowGeometry(qint32 x, qint32 y, qint32 width, qint32 height)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void XdgSurfaceV5::setMaximized()
{
    xdg_surface_set_maximized(d->xdgsurfacev5);
}

void XdgSurfaceV5::unsetMaximized()
{
    xdg_surface_unset_maximized(d->xdgsurfacev5);
}

void XdgSurfaceV5::setFullscreen(Output *output)
{
    wl_output *o = nullptr;
    if (output) {
        o = *output;
    }
    xdg_surface_set_fullscreen(d->xdgsurfacev5, o);
}

void XdgSurfaceV5::unsetFullscreen()
{
    xdg_surface_unset_fullscreen(d->xdgsurfacev5);
}

void XdgSurfaceV5::setMinimized()
{
    xdg_surface_set_minimized(d->xdgsurfacev5);
}

class XdgPopupV5::Private
{
public:
    Private() = default;

    WaylandPointer<xdg_popup, xdg_popup_destroy> xdgpopupv5;
    EventQueue *queue = nullptr;
};

XdgPopupV5::XdgPopupV5(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

XdgPopupV5::~XdgPopupV5()
{
    release();
}

void XdgPopupV5::setup(xdg_popup *xdgpopupv5)
{
    Q_ASSERT(xdgpopupv5);
    Q_ASSERT(!d->xdgpopupv5);
    d->xdgpopupv5.setup(xdgpopupv5);
}

void XdgPopupV5::release()
{
    d->xdgpopupv5.release();
}

void XdgPopupV5::destroy()
{
    d->xdgpopupv5.destroy();
}

void XdgPopupV5::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgPopupV5::eventQueue()
{
    return d->queue;
}

XdgPopupV5::operator xdg_popup*() {
    return d->xdgpopupv5;
}

XdgPopupV5::operator xdg_popup*() const {
    return d->xdgpopupv5;
}

bool XdgPopupV5::isValid() const
{
    return d->xdgpopupv5.isValid();
}

}
}

