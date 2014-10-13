/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
*********************************************************************/
#include "shell.h"
#include "event_queue.h"
#include "output.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Shell::Private
{
public:
    WaylandPointer<wl_shell, wl_shell_destroy> shell;
    EventQueue *queue = nullptr;
};

Shell::Shell(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Shell::~Shell()
{
    release();
}

void Shell::destroy()
{
    if (!d->shell) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->shell.destroy();
}

void Shell::release()
{
    if (!d->shell) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->shell.release();
}

void Shell::setup(wl_shell *shell)
{
    Q_ASSERT(!d->shell);
    Q_ASSERT(shell);
    d->shell.setup(shell);
}

void Shell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *Shell::eventQueue()
{
    return d->queue;
}

ShellSurface *Shell::createSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    ShellSurface *s = new ShellSurface(parent);
    connect(this, &Shell::interfaceAboutToBeReleased, s, &ShellSurface::release);
    connect(this, &Shell::interfaceAboutToBeDestroyed, s, &ShellSurface::destroy);
    auto w = wl_shell_get_shell_surface(d->shell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

ShellSurface *Shell::createSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(surface);
    return createSurface(*surface, parent);
}

bool Shell::isValid() const
{
    return d->shell.isValid();
}

Shell::operator wl_shell*()
{
    return d->shell;
}

Shell::operator wl_shell*() const
{
    return d->shell;
}

class ShellSurface::Private
{
public:
    Private(ShellSurface *q);
    void setup(wl_shell_surface *surface);

    WaylandPointer<wl_shell_surface, wl_shell_surface_destroy> surface;
    QSize size;

private:
    void ping(uint32_t serial);
    static void pingCallback(void *data, struct wl_shell_surface *shellSurface, uint32_t serial);
    static void configureCallback(void *data, struct wl_shell_surface *shellSurface, uint32_t edges, int32_t width, int32_t height);
    static void popupDoneCallback(void *data, struct wl_shell_surface *shellSurface);

    ShellSurface *q;
    static const struct wl_shell_surface_listener s_listener;
};

ShellSurface::Private::Private(ShellSurface *q)
    : q(q)
{
}

void ShellSurface::Private::setup(wl_shell_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
    wl_shell_surface_add_listener(surface, &s_listener, this);
}

ShellSurface::ShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ShellSurface::~ShellSurface()
{
    release();
}

void ShellSurface::release()
{
    d->surface.release();
}

void ShellSurface::destroy()
{
    d->surface.destroy();
}

const struct wl_shell_surface_listener ShellSurface::Private::s_listener = {
    pingCallback,
    configureCallback,
    popupDoneCallback
};

void ShellSurface::Private::configureCallback(void *data, wl_shell_surface *shellSurface, uint32_t edges, int32_t width, int32_t height)
{
    Q_UNUSED(edges)
    auto s = reinterpret_cast<ShellSurface::Private*>(data);
    Q_ASSERT(s->surface == shellSurface);
    s->q->setSize(QSize(width, height));
}

void ShellSurface::Private::pingCallback(void *data, wl_shell_surface *shellSurface, uint32_t serial)
{
    auto s = reinterpret_cast<ShellSurface::Private*>(data);
    Q_ASSERT(s->surface == shellSurface);
    s->ping(serial);
}

void ShellSurface::Private::popupDoneCallback(void *data, wl_shell_surface *shellSurface)
{
    // not needed, we don't have popups
    Q_UNUSED(data)
    Q_UNUSED(shellSurface)
}

void ShellSurface::setup(wl_shell_surface *surface)
{
    d->setup(surface);
}

void ShellSurface::Private::ping(uint32_t serial)
{
    wl_shell_surface_pong(surface, serial);
    emit q->pinged();
}

void ShellSurface::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }
    d->size = size;
    emit sizeChanged(size);
}

void ShellSurface::setFullscreen(Output *output)
{
    Q_ASSERT(isValid());
    wl_shell_surface_set_fullscreen(d->surface, WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, output ? output->output() : nullptr);
}

QSize ShellSurface::size() const
{
    return d->size;
}

bool ShellSurface::isValid() const
{
    return d->surface.isValid();
}

ShellSurface::operator wl_shell_surface*()
{
    return d->surface;
}

ShellSurface::operator wl_shell_surface*() const
{
    return d->surface;
}

}
}
