/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include "idle_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "seat_interface.h"

#include <QTimer>

#include <wayland-server.h>
#include <wayland-idle-server-protocol.h>

namespace KWayland
{
namespace Server
{

static const quint32 s_version = 1;

class IdleInterface::Private : public Global::Private
{
public:
    Private(IdleInterface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    static void getIdleTimeoutCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat, uint32_t timeout);

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    IdleInterface *q;
    static const struct org_kde_kwin_idle_interface s_interface;
};

class IdleTimeoutInterface::Private : public Resource::Private
{
public:
    Private(SeatInterface *seat, IdleTimeoutInterface *q, IdleInterface *manager, wl_resource *parentResource);
    ~Private();
    void setup(quint32 timeout);

    SeatInterface *seat;
    QTimer *timer = nullptr;

private:
    static void releaseCallback(wl_client *client, wl_resource *resource);
    static void simulateUserActivityCallback(wl_client *client, wl_resource *resource);
    IdleTimeoutInterface *q_func() {
        return reinterpret_cast<IdleTimeoutInterface*>(q);
    }
    static const struct org_kde_kwin_idle_timeout_interface s_interface;
};

const struct org_kde_kwin_idle_interface IdleInterface::Private::s_interface = {
    getIdleTimeoutCallback
};

IdleInterface::Private::Private(IdleInterface *q, Display *d)
    : Global::Private(d, &org_kde_kwin_idle_interface, s_version)
    , q(q)
{
}

void IdleInterface::Private::getIdleTimeoutCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat, uint32_t timeout)
{
    Private *p = cast(resource);
    SeatInterface *s = SeatInterface::get(seat);
    Q_ASSERT(s);
    IdleTimeoutInterface *idleTimeout = new IdleTimeoutInterface(s, p->q, resource);
    idleTimeout->create(p->display->getConnection(client), wl_resource_get_version(resource), id);
    if (!idleTimeout->resource()) {
        wl_resource_post_no_memory(resource);
        delete idleTimeout;
        return;
    }
    idleTimeout->d_func()->setup(timeout);
}

void IdleInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&org_kde_kwin_idle_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void IdleInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
}

IdleInterface::IdleInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

IdleInterface::~IdleInterface() = default;

const struct org_kde_kwin_idle_timeout_interface IdleTimeoutInterface::Private::s_interface = {
    releaseCallback,
    simulateUserActivityCallback
};

IdleTimeoutInterface::Private::Private(SeatInterface *seat, IdleTimeoutInterface *q, IdleInterface *manager, wl_resource *parentResource)
    : Resource::Private(q, manager, parentResource, &org_kde_kwin_idle_timeout_interface, &s_interface)
    , seat(seat)
{
}

IdleTimeoutInterface::Private::~Private() = default;

void IdleTimeoutInterface::Private::releaseCallback(wl_client* client, wl_resource* resource)
{
    Q_UNUSED(client);
    Private *p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    wl_resource_destroy(resource);
    p->q->deleteLater();
}

void IdleTimeoutInterface::Private::simulateUserActivityCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client);
    Private *p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    if (!p->timer) {
        // not yet configured
        return;
    }
    if (!p->timer->isActive() && p->resource) {
        org_kde_kwin_idle_timeout_send_resumed(p->resource);
    }
    p->timer->start();
}

void IdleTimeoutInterface::Private::setup(quint32 timeout)
{
    if (timer) {
        return;
    }
    timer = new QTimer(q);
    timer->setSingleShot(true);
    timer->setInterval(timeout);
    QObject::connect(timer, &QTimer::timeout, q,
        [this] {
            if (resource) {
                org_kde_kwin_idle_timeout_send_idle(resource);
            }
        }
    );
    timer->start();
}

IdleTimeoutInterface::IdleTimeoutInterface(SeatInterface *seat, IdleInterface *parent, wl_resource *parentResource)
    : Resource(new Private(seat, this, parent, parentResource))
{
    connect(seat, &SeatInterface::timestampChanged, this,
        [this] {
            Q_D();
            if (!d->timer) {
                // not yet configured
                return;
            }
            if (!d->timer->isActive() && d->resource) {
                org_kde_kwin_idle_timeout_send_resumed(d->resource);
            }
            d->timer->start();
        }
    );
}

IdleTimeoutInterface::~IdleTimeoutInterface() = default;

IdleTimeoutInterface::Private *IdleTimeoutInterface::d_func() const
{
    return reinterpret_cast<IdleTimeoutInterface::Private*>(d.data());
}

}
}
