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
#include "shell_interface.h"
#include "global_p.h"
#include "resource_p.h"
#include "display.h"
#include "surface_interface.h"

#include <QTimer>

#include <wayland-server.h>

namespace KWayland
{
namespace Server
{

static const quint32 s_version = 1;

class ShellInterface::Private : public Global::Private
{
public:
    Private(ShellInterface *q, Display *d);

    QList<ShellSurfaceInterface*> surfaces;

private:
    static void createSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface);
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource);

    ShellInterface *q;
    static const struct wl_shell_interface s_interface;
};

ShellInterface::Private::Private(ShellInterface *q, Display *d)
    : Global::Private(d, &wl_shell_interface, s_version)
    , q(q)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct wl_shell_interface ShellInterface::Private::s_interface = {
    createSurfaceCallback
};
#endif


class ShellSurfaceInterface::Private : public Resource::Private
{
public:
    Private(ShellSurfaceInterface *q, ShellInterface *shell, SurfaceInterface *surface, wl_resource *parentResource);
    void ping();

    SurfaceInterface *surface;
    QString title;
    QByteArray windowClass;
    QScopedPointer<QTimer> pingTimer;
    quint32 pingSerial = 0;
    enum class WindowMode {
        Fullscreen,
        Toplevel,
        Maximized
    };
    WindowMode windowMode = WindowMode::Toplevel;
    void setWindowMode(WindowMode newWindowMode);

private:
    // interface callbacks
    static void pongCallback(wl_client *client, wl_resource *resource, uint32_t serial);
    static void moveCallback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial);
    static void resizeCallback(wl_client *client, wl_resource *resource, wl_resource *seat,
                               uint32_t serial, uint32_t edges);
    static void setToplevelCallback(wl_client *client, wl_resource *resource);
    static void setTransientCallback(wl_client *client, wl_resource *resource, wl_resource *parent,
                                     int32_t x, int32_t y, uint32_t flags);
    static void setFullscreenCallback(wl_client *client, wl_resource *resource, uint32_t method,
                                      uint32_t framerate, wl_resource *output);
    static void setPopupCalback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial,
                                wl_resource *parent, int32_t x, int32_t y, uint32_t flags);
    static void setMaximizedCallback(wl_client *client, wl_resource *resource, wl_resource *output);
    static void setTitleCallback(wl_client *client, wl_resource *resource, const char *title);
    static void setClassCallback(wl_client *client, wl_resource *resource, const char *class_);

    void setTitle(const QString &title);
    void setWindowClass(const QByteArray &windowClass);
    void pong(quint32 serial);
    ShellSurfaceInterface *q_func() {
        return reinterpret_cast<ShellSurfaceInterface *>(q);
    }

    static const struct wl_shell_surface_interface s_interface;
};

ShellInterface::ShellInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

ShellInterface::~ShellInterface() = default;

void ShellInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *shell = c->createResource(&wl_shell_interface, qMin(version, s_version), id);
    if (!shell) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(shell, &s_interface, this, nullptr);
}

void ShellInterface::Private::createSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface)
{
    auto s = reinterpret_cast<ShellInterface::Private*>(wl_resource_get_user_data(resource));
    s->createSurface(client, wl_resource_get_version(resource), id, SurfaceInterface::get(surface), resource);
}

void ShellInterface::Private::createSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource)
{
    auto it = std::find_if(surfaces.constBegin(), surfaces.constEnd(),
        [surface](ShellSurfaceInterface *s) {
            return surface == s->surface();
        }
    );
    if (it != surfaces.constEnd()) {
        wl_resource_post_error(surface->resource(), WL_DISPLAY_ERROR_INVALID_OBJECT, "ShellSurface already created");
        return;
    }
    ShellSurfaceInterface *shellSurface = new ShellSurfaceInterface(q, surface, parentResource);
    surfaces << shellSurface;
    QObject::connect(shellSurface, &ShellSurfaceInterface::destroyed, q,
        [this, shellSurface] {
            surfaces.removeAll(shellSurface);
        }
    );
    shellSurface->d->create(display->getConnection(client), version, id);
    emit q->surfaceCreated(shellSurface);
}

/*********************************
 * ShellSurfaceInterface
 *********************************/
ShellSurfaceInterface::Private::Private(ShellSurfaceInterface *q, ShellInterface *shell, SurfaceInterface *surface, wl_resource *parentResource)
    : Resource::Private(q, shell, parentResource, &wl_shell_surface_interface, &s_interface)
    , surface(surface)
    , pingTimer(new QTimer)
{
    pingTimer->setSingleShot(true);
    pingTimer->setInterval(1000);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct wl_shell_surface_interface ShellSurfaceInterface::Private::s_interface = {
    pongCallback,
    moveCallback,
    resizeCallback,
    setToplevelCallback,
    setTransientCallback,
    setFullscreenCallback,
    setPopupCalback,
    setMaximizedCallback,
    setTitleCallback,
    setClassCallback
};
#endif

ShellSurfaceInterface::ShellSurfaceInterface(ShellInterface *shell, SurfaceInterface *parent, wl_resource *parentResource)
    : Resource(new Private(this, shell, parent, parentResource), parent)
{
    Q_D();
    connect(d->pingTimer.data(), &QTimer::timeout, this, &ShellSurfaceInterface::pingTimeout);
}

ShellSurfaceInterface::~ShellSurfaceInterface() = default;

void ShellSurfaceInterface::Private::pongCallback(wl_client *client, wl_resource *resource, uint32_t serial)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->pong(serial);
}

void ShellSurfaceInterface::Private::pong(quint32 serial)
{
    if (pingTimer->isActive() && serial == pingSerial) {
        pingTimer->stop();
        Q_Q(ShellSurfaceInterface);
        emit q->pongReceived();
    }
}

void ShellSurfaceInterface::ping()
{
    Q_D();
    d->ping();
}

void ShellSurfaceInterface::Private::ping()
{
    if (pingTimer->isActive()) {
        return;
    }
    pingSerial = global->display()->nextSerial();
    wl_shell_surface_send_ping(resource, pingSerial);
    client->flush();
    pingTimer->start();
}

void ShellSurfaceInterface::setPingTimeout(uint msec)
{
    Q_D();
    d->pingTimer->setInterval(msec);
}

bool ShellSurfaceInterface::isPinged() const
{
    Q_D();
    return d->pingTimer->isActive();
}

void ShellSurfaceInterface::requestSize(const QSize &size)
{
    Q_D();
    // TODO: what about the edges?
    wl_shell_surface_send_configure(d->resource, 0, size.width(), size.height());
    d->client->flush();
}

void ShellSurfaceInterface::Private::moveCallback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    // TODO: implement
}

void ShellSurfaceInterface::Private::resizeCallback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial, uint32_t edges)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    Q_UNUSED(edges)
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    // TODO: implement
}

void ShellSurfaceInterface::Private::setToplevelCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->setWindowMode(WindowMode::Toplevel);
}

void ShellSurfaceInterface::Private::setTransientCallback(wl_client *client, wl_resource *resource, wl_resource *parent,
                                                 int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(parent)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(flags)
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    // TODO: implement
}

void ShellSurfaceInterface::Private::setFullscreenCallback(wl_client *client, wl_resource *resource, uint32_t method,
                                                  uint32_t framerate, wl_resource *output)
{
    Q_UNUSED(method)
    Q_UNUSED(framerate)
    Q_UNUSED(output)
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    // TODO: add method, framerate and output
    s->setWindowMode(WindowMode::Fullscreen);
}

void ShellSurfaceInterface::Private::setWindowMode(WindowMode newWindowMode)
{
    if (windowMode == newWindowMode) {
        return;
    }
    const WindowMode oldWindowMode = windowMode;
    windowMode = newWindowMode;
    Q_Q(ShellSurfaceInterface);
    if (oldWindowMode == WindowMode::Fullscreen || newWindowMode == WindowMode::Fullscreen) {
        emit q->fullscreenChanged(windowMode == WindowMode::Fullscreen);
    }
    if (oldWindowMode == WindowMode::Toplevel || newWindowMode == WindowMode::Toplevel) {
        emit q->toplevelChanged(windowMode == WindowMode::Toplevel);
    }
    if (oldWindowMode == WindowMode::Maximized || newWindowMode == WindowMode::Maximized) {
        emit q->maximizedChanged(windowMode == WindowMode::Maximized);
    }
}

void ShellSurfaceInterface::Private::setPopupCalback(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial,
                                            wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    Q_UNUSED(parent)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(flags)
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    // TODO: implement
}

void ShellSurfaceInterface::Private::setMaximizedCallback(wl_client *client, wl_resource *resource, wl_resource *output)
{
    Q_UNUSED(output)
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->setWindowMode(WindowMode::Maximized);
}

void ShellSurfaceInterface::Private::setTitleCallback(wl_client *client, wl_resource *resource, const char *title)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->setTitle(QString::fromUtf8(title));
}

void ShellSurfaceInterface::Private::setTitle(const QString &t)
{
    if (title == t) {
        return;
    }
    title = t;
    Q_Q(ShellSurfaceInterface);
    emit q->titleChanged(title);
}

void ShellSurfaceInterface::Private::setClassCallback(wl_client *client, wl_resource *resource, const char *class_)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);
    s->setWindowClass(QByteArray(class_));
}

void ShellSurfaceInterface::Private::setWindowClass(const QByteArray &wc)
{
    if (windowClass == wc) {
        return;
    }
    windowClass = wc;
    Q_Q(ShellSurfaceInterface);
    emit q->windowClassChanged(windowClass);
}

SurfaceInterface *ShellSurfaceInterface::surface() const {
    Q_D();
    return d->surface;
}

ShellInterface *ShellSurfaceInterface::shell() const {
    Q_D();
    return reinterpret_cast<ShellInterface*>(d->global);
}

QString ShellSurfaceInterface::title() const {
    Q_D();
    return d->title;
}

QByteArray ShellSurfaceInterface::windowClass() const {
    Q_D();
    return d->windowClass;
}

bool ShellSurfaceInterface::isFullscreen() const {
    Q_D();
    return d->windowMode == Private::WindowMode::Fullscreen;
}

bool ShellSurfaceInterface::isToplevel() const {
    Q_D();
    return d->windowMode == Private::WindowMode::Toplevel;
}

bool ShellSurfaceInterface::isMaximized() const {
    Q_D();
    return d->windowMode == Private::WindowMode::Maximized;
}

ShellSurfaceInterface::Private *ShellSurfaceInterface::d_func() const
{
    return reinterpret_cast<ShellSurfaceInterface::Private*>(d.data());
}

}
}
