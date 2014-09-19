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
#include "registry.h"
#include "compositor.h"
#include "connection_thread.h"
#include "fullscreen_shell.h"
#include "output.h"
#include "seat.h"
#include "shell.h"
#include "shm_pool.h"
// Qt
#include <QDebug>
// wayland
#include <wayland-client-protocol.h>
#include <wayland-client-fullscreen-shell.h>

namespace KWayland
{
namespace Client
{

class Registry::Private
{
public:
    Private(Registry *q);
    void setup();
    bool hasInterface(Interface interface) const;
    template <typename T>
    T *bind(Interface interface, uint32_t name, uint32_t version) const;

    wl_registry *registry = nullptr;

private:
    void handleAnnounce(uint32_t name, const char *interface, uint32_t version);
    void handleRemove(uint32_t name);
    static void globalAnnounce(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void globalRemove(void *data, struct wl_registry *registry, uint32_t name);

    Registry *q;
    struct InterfaceData {
        Interface interface;
        uint32_t name;
        uint32_t version;
    };
    QList<InterfaceData> m_interfaces;
    static const struct wl_registry_listener s_registryListener;
};

Registry::Private::Private(Registry *q)
    : q(q)
{
}

void Registry::Private::setup()
{
    wl_registry_add_listener(registry, &s_registryListener, this);
}

Registry::Registry(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Registry::~Registry()
{
    release();
}

void Registry::release()
{
    if (d->registry) {
        wl_registry_destroy(d->registry);
        d->registry = nullptr;
    }
}

void Registry::destroy()
{
    if (d->registry) {
        free(d->registry);
        d->registry = nullptr;
    }
}

void Registry::create(wl_display *display)
{
    Q_ASSERT(display);
    Q_ASSERT(!isValid());
    d->registry = wl_display_get_registry(display);
}

void Registry::create(ConnectionThread *connection)
{
    create(connection->display());
}

void Registry::setup()
{
    Q_ASSERT(isValid());
    d->setup();
}

const struct wl_registry_listener Registry::Private::s_registryListener = {
    globalAnnounce,
    globalRemove
};

void Registry::Private::globalAnnounce(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto r = reinterpret_cast<Registry::Private*>(data);
    Q_ASSERT(registry == r->registry);
    r->handleAnnounce(name, interface, version);
}

void Registry::Private::globalRemove(void *data, wl_registry *registry, uint32_t name)
{
    auto r = reinterpret_cast<Registry::Private*>(data);
    Q_ASSERT(registry == r->registry);
    r->handleRemove(name);
}

static Registry::Interface nameToInterface(const char *interface)
{
    if (strcmp(interface, "wl_compositor") == 0) {
        return Registry::Interface::Compositor;
    } else if (strcmp(interface, "wl_shell") == 0) {
        return Registry::Interface::Shell;
    } else if (strcmp(interface, "wl_seat") == 0) {
        return Registry::Interface::Seat;
    } else if (strcmp(interface, "wl_shm") == 0) {
        return Registry::Interface::Shm;
    } else if (strcmp(interface, "wl_output") == 0) {
        return Registry::Interface::Output;
    } else if (strcmp(interface, "_wl_fullscreen_shell") == 0) {
        return Registry::Interface::FullscreenShell;
    }
    return Registry::Interface::Unknown;
}

void Registry::Private::handleAnnounce(uint32_t name, const char *interface, uint32_t version)
{
    Interface i = nameToInterface(interface);
    emit q->interfaceAnnounced(QByteArray(interface), name, version);
    if (i == Interface::Unknown) {
        qDebug() << "Unknown interface announced: " << interface << "/" << name << "/" << version;
        return;
    }
    qDebug() << "Wayland Interface: " << interface << "/" << name << "/" << version;
    m_interfaces.append({i, name, version});
    switch (i) {
    case Interface::Compositor:
        emit q->compositorAnnounced(name, version);
        break;
    case Interface::Shell:
        emit q->shellAnnounced(name, version);
        break;
    case Interface::Output:
        emit q->outputAnnounced(name, version);
        break;
    case Interface::Seat:
        emit q->seatAnnounced(name, version);
        break;
    case Interface::Shm:
        emit q->shmAnnounced(name, version);
        break;
    case Interface::FullscreenShell:
        emit q->fullscreenShellAnnounced(name, version);
        break;
    case Interface::Unknown:
    default:
        // nothing
        break;
    }
}

void Registry::Private::handleRemove(uint32_t name)
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(),
        [name](const InterfaceData &data) {
            return data.name == name;
        }
    );
    if (it != m_interfaces.end()) {
        InterfaceData data = *(it);
        m_interfaces.erase(it);
        switch (data.interface) {
        case Interface::Compositor:
            emit q->compositorRemoved(data.name);
            break;
        case Interface::Output:
            emit q->outputRemoved(data.name);
            break;
        case Interface::Seat:
            emit q->seatRemoved(data.name);
            break;
        case Interface::Shell:
            emit q->shellRemoved(data.name);
            break;
        case Interface::Shm:
            emit q->shmRemoved(data.name);
            break;
        case Interface::FullscreenShell:
            emit q->fullscreenShellRemoved(data.name);
            break;
        case Interface::Unknown:
        default:
            // nothing
            break;
        }
    }
    emit q->interfaceRemoved(name);
}

bool Registry::Private::hasInterface(Registry::Interface interface) const
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(),
        [interface](const InterfaceData &data) {
            return data.interface == interface;
        }
    );
    return it != m_interfaces.end();
}

bool Registry::hasInterface(Registry::Interface interface) const
{
    return d->hasInterface(interface);
}

wl_compositor *Registry::bindCompositor(uint32_t name, uint32_t version) const
{
    return d->bind<wl_compositor>(Interface::Compositor, name, version);
}

wl_output *Registry::bindOutput(uint32_t name, uint32_t version) const
{
    return d->bind<wl_output>(Interface::Output, name, version);
}

wl_seat *Registry::bindSeat(uint32_t name, uint32_t version) const
{
    return d->bind<wl_seat>(Interface::Seat, name, version);
}

wl_shell *Registry::bindShell(uint32_t name, uint32_t version) const
{
    return d->bind<wl_shell>(Interface::Shell, name, version);
}

wl_shm *Registry::bindShm(uint32_t name, uint32_t version) const
{
    return d->bind<wl_shm>(Interface::Shm, name, version);
}

_wl_fullscreen_shell *Registry::bindFullscreenShell(uint32_t name, uint32_t version) const
{
    return d->bind<_wl_fullscreen_shell>(Interface::FullscreenShell, name, version);
}

Compositor *Registry::createCompositor(quint32 name, quint32 version, QObject *parent)
{
    Compositor *c = new Compositor(parent);
    c->setup(bindCompositor(name, version));
    return c;
}

FullscreenShell *Registry::createFullscreenShell(quint32 name, quint32 version, QObject *parent)
{
    FullscreenShell *s = new FullscreenShell(parent);
    s->setup(bindFullscreenShell(name, version));
    return s;
}

Output *Registry::createOutput(quint32 name, quint32 version, QObject *parent)
{
    Output *o = new Output(parent);
    o->setup(bindOutput(name, version));
    return o;
}

Seat *Registry::createSeat(quint32 name, quint32 version, QObject *parent)
{
    Seat *s = new Seat(parent);
    s->setup(bindSeat(name, version));
    return s;
}

Shell *Registry::createShell(quint32 name, quint32 version, QObject *parent)
{
    Shell *s = new Shell(parent);
    s->setup(bindShell(name, version));
    return s;
}

ShmPool *Registry::createShmPool(quint32 name, quint32 version, QObject *parent)
{
    ShmPool *s = new ShmPool(parent);
    s->setup(bindShm(name, version));
    return s;
}

static const wl_interface *wlInterface(Registry::Interface interface)
{
    switch (interface) {
    case Registry::Interface::Compositor:
        return &wl_compositor_interface;
    case Registry::Interface::Output:
        return &wl_output_interface;
    case Registry::Interface::Seat:
        return &wl_seat_interface;
    case Registry::Interface::Shell:
        return &wl_shell_interface;
    case Registry::Interface::Shm:
        return &wl_shm_interface;
    case Registry::Interface::FullscreenShell:
        return &_wl_fullscreen_shell_interface;
    case Registry::Interface::Unknown:
    default:
        return nullptr;
    }
}

template <typename T>
T *Registry::Private::bind(Registry::Interface interface, uint32_t name, uint32_t version) const
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(), [=](const InterfaceData &data) {
        return data.interface == interface && data.name == name && data.version >= version;
    });
    if (it == m_interfaces.end()) {
        qDebug() << "Don't have interface " << int(interface) << "with name " << name << "and minimum version" << version;
        return nullptr;
    }
    return reinterpret_cast<T*>(wl_registry_bind(registry, name, wlInterface(interface), version));
}

bool Registry::isValid() const
{
    return d->registry != nullptr;
}

wl_registry *Registry::registry()
{
    return d->registry;
}

Registry::operator wl_registry*() const
{
    return d->registry;
}

Registry::operator wl_registry*()
{
    return d->registry;
}

}
}
