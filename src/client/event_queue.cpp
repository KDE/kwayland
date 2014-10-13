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
#include "event_queue.h"
#include "connection_thread.h"
#include "wayland_pointer_p.h"

#include <wayland-client.h>

namespace KWayland
{
namespace Client
{

class EventQueue::Private
{
public:
    Private(EventQueue *q);

    wl_display *display = nullptr;
    WaylandPointer<wl_event_queue, wl_event_queue_destroy> queue;

private:
    EventQueue *q;
};

EventQueue::Private::Private(EventQueue *q)
    : q(q)
{
}

EventQueue::EventQueue(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

EventQueue::~EventQueue()
{
    release();
}

void EventQueue::release()
{
    d->queue.release();
    d->display = nullptr;
}

void EventQueue::destroy()
{
    d->queue.destroy();
    d->display = nullptr;
}

bool EventQueue::isValid()
{
    return d->queue.isValid();
}

void EventQueue::setup(wl_display *display)
{
    Q_ASSERT(display);
    Q_ASSERT(!d->display);
    Q_ASSERT(!d->queue);
    d->display = display;
    d->queue.setup(wl_display_create_queue(display));
}

void EventQueue::setup(ConnectionThread *connection)
{
    setup(connection->display());
    connect(connection, &ConnectionThread::eventsRead, this, &EventQueue::dispatch, Qt::QueuedConnection);
}

void EventQueue::dispatch()
{
    if (!d->display || !d->queue) {
        return;
    }
    wl_display_dispatch_queue_pending(d->display, d->queue);
    wl_display_flush(d->display);
}

void EventQueue::addProxy(wl_proxy *proxy)
{
    Q_ASSERT(d->queue);
    wl_proxy_set_queue(proxy, d->queue);
}

EventQueue::operator wl_event_queue*() const
{
    return d->queue;
}

EventQueue::operator wl_event_queue*()
{
    return d->queue;
}

}
}
