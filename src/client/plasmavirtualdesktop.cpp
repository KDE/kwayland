/****************************************************************************
Copyright 2018  Marco Martin <notmart@gmail.com>

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
#include "plasmavirtualdesktop.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <wayland-org_kde_plasma_virtual_desktop-client-protocol.h>

namespace KWayland
{
namespace Client
{

class PlasmaVirtualDesktopManagement::Private
{
public:
    Private() = default;

    void setup(org_kde_plasma_virtual_desktop_management *arg);

    WaylandPointer<org_kde_plasma_virtual_desktop_management, org_kde_plasma_virtual_desktop_management_destroy> plasmavirtualdesktopmanagement;
    EventQueue *queue = nullptr;

private:
    static void addedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id);
    static void removedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id);
    static void layoutCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, uint32_t rows, uint32_t columns);
    static void doneCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management);

    static const org_kde_plasma_virtual_desktop_management_listener s_listener;
};

const org_kde_plasma_virtual_desktop_management_listener PlasmaVirtualDesktopManagement::Private::s_listener = {
    addedCallback,
    removedCallback,
    layoutCallback,
    doneCallback
};

void PlasmaVirtualDesktopManagement::Private::addedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    Q_UNUSED(id)
    // TODO: implement
}

void PlasmaVirtualDesktopManagement::Private::removedCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, const char *id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    Q_UNUSED(id)
    // TODO: implement
}

void PlasmaVirtualDesktopManagement::Private::layoutCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management, uint32_t rows, uint32_t columns)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    Q_UNUSED(rows)
    Q_UNUSED(columns)
    // TODO: implement
}

void PlasmaVirtualDesktopManagement::Private::doneCallback(void *data, org_kde_plasma_virtual_desktop_management *org_kde_plasma_virtual_desktop_management)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktopManagement::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktopmanagement == org_kde_plasma_virtual_desktop_management);
    // TODO: implement
}

PlasmaVirtualDesktopManagement::PlasmaVirtualDesktopManagement(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void PlasmaVirtualDesktopManagement::Private::setup(org_kde_plasma_virtual_desktop_management *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!plasmavirtualdesktopmanagement);
    plasmavirtualdesktopmanagement.setup(arg);
    org_kde_plasma_virtual_desktop_management_add_listener(plasmavirtualdesktopmanagement, &s_listener, this);
}

PlasmaVirtualDesktopManagement::~PlasmaVirtualDesktopManagement()
{
    release();
}

void PlasmaVirtualDesktopManagement::setup(org_kde_plasma_virtual_desktop_management *plasmavirtualdesktopmanagement)
{
    d->setup(plasmavirtualdesktopmanagement);
}

void PlasmaVirtualDesktopManagement::release()
{
    d->plasmavirtualdesktopmanagement.release();
}

void PlasmaVirtualDesktopManagement::destroy()
{
    d->plasmavirtualdesktopmanagement.destroy();
}

PlasmaVirtualDesktopManagement::operator org_kde_plasma_virtual_desktop_management*() {
    return d->plasmavirtualdesktopmanagement;
}

PlasmaVirtualDesktopManagement::operator org_kde_plasma_virtual_desktop_management*() const {
    return d->plasmavirtualdesktopmanagement;
}

bool PlasmaVirtualDesktopManagement::isValid() const
{
    return d->plasmavirtualdesktopmanagement.isValid();
}

void PlasmaVirtualDesktopManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaVirtualDesktopManagement::eventQueue()
{
    return d->queue;
}

PlasmaVirtualDesktop *PlasmaVirtualDesktopManagement::getVirtualDesktop(const QString &id, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new PlasmaVirtualDesktop(parent);
    auto w = org_kde_plasma_virtual_desktop_management_get_virtual_desktop(d->plasmavirtualdesktopmanagement, id.toUtf8());
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class PlasmaVirtualDesktop::Private
{
public:
    Private(PlasmaVirtualDesktop *q);

    void setup(org_kde_plasma_virtual_desktop *arg);

    WaylandPointer<org_kde_plasma_virtual_desktop, org_kde_plasma_virtual_desktop_destroy> plasmavirtualdesktop;

private:
    PlasmaVirtualDesktop *q;

private:
    static void idCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * id);
    static void nameCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * name);
    static void layout_positionCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, uint32_t row, uint32_t column);
    static void activatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);
    static void deactivatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);
    static void doneCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);
    static void removedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop);

    static const org_kde_plasma_virtual_desktop_listener s_listener;
};

const org_kde_plasma_virtual_desktop_listener PlasmaVirtualDesktop::Private::s_listener = {
    idCallback,
    nameCallback,
    layout_positionCallback,
    activatedCallback,
    deactivatedCallback,
    doneCallback,
    removedCallback
};

void PlasmaVirtualDesktop::Private::idCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * id)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    Q_UNUSED(id)
    // TODO: implement
}

void PlasmaVirtualDesktop::Private::nameCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, const char * name)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    Q_UNUSED(name)
    // TODO: implement
}

void PlasmaVirtualDesktop::Private::layout_positionCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop, uint32_t row, uint32_t column)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    Q_UNUSED(row)
    Q_UNUSED(column)
    // TODO: implement
}

void PlasmaVirtualDesktop::Private::activatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    // TODO: implement
}

void PlasmaVirtualDesktop::Private::deactivatedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    // TODO: implement
}

void PlasmaVirtualDesktop::Private::doneCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    // TODO: implement
}

void PlasmaVirtualDesktop::Private::removedCallback(void *data, org_kde_plasma_virtual_desktop *org_kde_plasma_virtual_desktop)
{
    auto p = reinterpret_cast<PlasmaVirtualDesktop::Private*>(data);
    Q_ASSERT(p->plasmavirtualdesktop == org_kde_plasma_virtual_desktop);
    // TODO: implement
}

PlasmaVirtualDesktop::Private::Private(PlasmaVirtualDesktop *q)
    : q(q)
{
}

PlasmaVirtualDesktop::PlasmaVirtualDesktop(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void PlasmaVirtualDesktop::Private::setup(org_kde_plasma_virtual_desktop *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!plasmavirtualdesktop);
    plasmavirtualdesktop.setup(arg);
    org_kde_plasma_virtual_desktop_add_listener(plasmavirtualdesktop, &s_listener, this);
}

PlasmaVirtualDesktop::~PlasmaVirtualDesktop()
{
    release();
}

void PlasmaVirtualDesktop::setup(org_kde_plasma_virtual_desktop *plasmavirtualdesktop)
{
    d->setup(plasmavirtualdesktop);
}

void PlasmaVirtualDesktop::release()
{
    d->plasmavirtualdesktop.release();
}

void PlasmaVirtualDesktop::destroy()
{
    d->plasmavirtualdesktop.destroy();
}

PlasmaVirtualDesktop::operator org_kde_plasma_virtual_desktop*() {
    return d->plasmavirtualdesktop;
}

PlasmaVirtualDesktop::operator org_kde_plasma_virtual_desktop*() const {
    return d->plasmavirtualdesktop;
}

bool PlasmaVirtualDesktop::isValid() const
{
    return d->plasmavirtualdesktop.isValid();
}

void PlasmaVirtualDesktop::activate()
{
    Q_ASSERT(isValid());
    org_kde_plasma_virtual_desktop_activate(d->plasmavirtualdesktop);
}


}
}

