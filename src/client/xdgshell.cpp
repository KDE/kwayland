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
#include "xdgshell_p.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"
#include "seat.h"
#include "surface.h"
#include "output.h"
#include <wayland-xdg-shell-v5-client-protocol.h>

namespace KWayland
{
namespace Client
{

XdgShell::Private::~Private() = default;

XdgShell::XdgShell(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgShell::~XdgShell()
{
    release();
}

void XdgShell::setup(xdg_shell *xdgshellv5)
{
    d->setupV5(xdgshellv5);
}

void XdgShell::setup(zxdg_shell_v6 *xdgshellv6)
{
    d->setupV6(xdgshellv6);
}


void XdgShell::release()
{
    d->release();
}

void XdgShell::destroy()
{
    d->destroy();
}

void XdgShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgShell::eventQueue()
{
    return d->queue;
}

XdgShell::operator xdg_shell*() {
    return *(d.data());
}

XdgShell::operator xdg_shell*() const {
    return *(d.data());
}

XdgShell::operator zxdg_shell_v6*() {
    return *(d.data());
}

XdgShell::operator zxdg_shell_v6*() const {
    return *(d.data());
}

bool XdgShell::isValid() const
{
    return d->isValid();
}

XdgShellSurface *XdgShell::createSurface(Surface *surface, QObject *parent)
{
    return d->getXdgSurface(surface, parent);
}

XdgShellPopup *XdgShell::createPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent)
{
    return d->getXdgPopup(surface, parentSurface, seat, serial, parentPos, parent);
}

XdgShellSurface::Private::Private(XdgShellSurface *q)
    : q(q)
{
}

XdgShellSurface::Private::~Private() = default;

XdgShellSurface::XdgShellSurface(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgShellSurface::~XdgShellSurface()
{
    release();
}

void XdgShellSurface::setup(xdg_surface *xdgsurfacev5)
{
    d->setupV5(xdgsurfacev5);
}

void XdgShellSurface::setup(zxdg_toplevel_v6 *xdgsurfacev6)
{
    d->setupV6(xdgsurfacev6);
}

void XdgShellSurface::release()
{
    d->release();
}

void XdgShellSurface::destroy()
{
    d->destroy();
}

void XdgShellSurface::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgShellSurface::eventQueue()
{
    return d->queue;
}

XdgShellSurface::operator xdg_surface*() {
    return *(d.data());
}

XdgShellSurface::operator xdg_surface*() const {
    return *(d.data());
}

XdgShellSurface::operator zxdg_toplevel_v6*() {
    return *(d.data());
}

XdgShellSurface::operator zxdg_toplevel_v6*() const {
    return *(d.data());
}

bool XdgShellSurface::isValid() const
{
    return d->isValid();
}

void XdgShellSurface::setTransientFor(XdgShellSurface *parent)
{
    d->setTransientFor(parent);
}

void XdgShellSurface::setTitle(const QString &title)
{
    d->setTitle(title);
}

void XdgShellSurface::setAppId(const QByteArray &appId)
{
    d->setAppId(appId);
}

void XdgShellSurface::requestShowWindowMenu(Seat *seat, quint32 serial, const QPoint &pos)
{
    d->showWindowMenu(seat, serial, pos.x(), pos.y());
}

void XdgShellSurface::requestMove(Seat *seat, quint32 serial)
{
    d->move(seat, serial);
}

void XdgShellSurface::requestResize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    d->resize(seat, serial, edges);
}

void XdgShellSurface::ackConfigure(quint32 serial)
{
    d->ackConfigure(serial);
}

void XdgShellSurface::setMaximized(bool set)
{
    if (set) {
        d->setMaximized();
    } else {
        d->unsetMaximized();
    }
}

void XdgShellSurface::setFullscreen(bool set, Output *output)
{
    if (set) {
        d->setFullscreen(output);
    } else {
        d->unsetFullscreen();
    }
}

void XdgShellSurface::requestMinimize()
{
    d->setMinimized();
}

void XdgShellSurface::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }
    d->size = size;
    emit sizeChanged(size);
}

QSize XdgShellSurface::size() const
{
    return d->size;
}

XdgShellPopup::Private::~Private() = default;


XdgShellPopup::Private::Private(XdgShellPopup *q)
    : q(q)
{
}

XdgShellPopup::XdgShellPopup(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgShellPopup::~XdgShellPopup()
{
    release();
}

void XdgShellPopup::setup(xdg_popup *xdgpopupv5)
{
    d->setupV5(xdgpopupv5);
}

void XdgShellPopup::release()
{
    d->release();
}

void XdgShellPopup::destroy()
{
    d->destroy();
}

void XdgShellPopup::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgShellPopup::eventQueue()
{
    return d->queue;
}

XdgShellPopup::operator xdg_popup*() {
    return *(d.data());
}

XdgShellPopup::operator xdg_popup*() const {
    return *(d.data());
}

XdgShellPopup::operator zxdg_popup_v6*() {
    return *(d.data());
}

XdgShellPopup::operator zxdg_popup_v6*() const {
    return *(d.data());
}

bool XdgShellPopup::isValid() const
{
    return d->isValid();
}

}
}

