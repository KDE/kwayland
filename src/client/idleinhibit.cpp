/****************************************************************************
Copyright 2017  Martin Flöser <mgraesslin@kde.org>

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
#include "idleinhibit.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-idle-inhibit-unstable-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN IdleInhibitManager::Private
{
public:
    Private() = default;

    void setup(zwp_idle_inhibit_manager_v1 *arg);

    WaylandPointer<zwp_idle_inhibit_manager_v1, zwp_idle_inhibit_manager_v1_destroy> idleinhibitmanager;
    EventQueue *queue = nullptr;
};

IdleInhibitManager::IdleInhibitManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void IdleInhibitManager::Private::setup(zwp_idle_inhibit_manager_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!idleinhibitmanager);
    idleinhibitmanager.setup(arg);
}

IdleInhibitManager::~IdleInhibitManager()
{
    release();
}

void IdleInhibitManager::setup(zwp_idle_inhibit_manager_v1 *idleinhibitmanager)
{
    d->setup(idleinhibitmanager);
}

void IdleInhibitManager::release()
{
    d->idleinhibitmanager.release();
}

void IdleInhibitManager::destroy()
{
    d->idleinhibitmanager.destroy();
}

IdleInhibitManager::operator zwp_idle_inhibit_manager_v1*() {
    return d->idleinhibitmanager;
}

IdleInhibitManager::operator zwp_idle_inhibit_manager_v1*() const {
    return d->idleinhibitmanager;
}

bool IdleInhibitManager::isValid() const
{
    return d->idleinhibitmanager.isValid();
}

void IdleInhibitManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *IdleInhibitManager::eventQueue()
{
    return d->queue;
}

IdleInhibitor *IdleInhibitManager::createInhibitor(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new IdleInhibitor(parent);
    auto w = zwp_idle_inhibit_manager_v1_create_inhibitor(d->idleinhibitmanager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN IdleInhibitor::Private
{
public:
    Private(IdleInhibitor *q);

    void setup(zwp_idle_inhibitor_v1 *arg);

    WaylandPointer<zwp_idle_inhibitor_v1, zwp_idle_inhibitor_v1_destroy> idleinhibitor;

private:
    IdleInhibitor *q;
};

IdleInhibitor::Private::Private(IdleInhibitor *q)
    : q(q)
{
}

IdleInhibitor::IdleInhibitor(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void IdleInhibitor::Private::setup(zwp_idle_inhibitor_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!idleinhibitor);
    idleinhibitor.setup(arg);
}

IdleInhibitor::~IdleInhibitor()
{
    release();
}

void IdleInhibitor::setup(zwp_idle_inhibitor_v1 *idleinhibitor)
{
    d->setup(idleinhibitor);
}

void IdleInhibitor::release()
{
    d->idleinhibitor.release();
}

void IdleInhibitor::destroy()
{
    d->idleinhibitor.destroy();
}

IdleInhibitor::operator zwp_idle_inhibitor_v1*() {
    return d->idleinhibitor;
}

IdleInhibitor::operator zwp_idle_inhibitor_v1*() const {
    return d->idleinhibitor;
}

bool IdleInhibitor::isValid() const
{
    return d->idleinhibitor.isValid();
}

}
}
