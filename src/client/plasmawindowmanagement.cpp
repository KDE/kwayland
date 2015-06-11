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
#include "plasmawindowmanagement.h"
#include "event_queue.h"
#include "output.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-plasma-window-management-client-protocol.h>

namespace KWayland
{
namespace Client
{

class PlasmaWindowManagement::Private
{
public:
    Private(PlasmaWindowManagement *q);
    WaylandPointer<org_kde_plasma_window_management, org_kde_plasma_window_management_destroy> wm;
    EventQueue *queue = nullptr;
    bool showingDesktop = false;

    void setup(org_kde_plasma_window_management *wm);

private:
    static void showDesktopCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state);
    void setShowDesktop(bool set);

    static struct org_kde_plasma_window_management_listener s_listener;
    PlasmaWindowManagement *q;
};

PlasmaWindowManagement::Private::Private(PlasmaWindowManagement *q)
    : q(q)
{
}

org_kde_plasma_window_management_listener PlasmaWindowManagement::Private::s_listener = {
    showDesktopCallback
};

void PlasmaWindowManagement::Private::setup(org_kde_plasma_window_management *windowManagement)
{
    Q_ASSERT(!wm);
    Q_ASSERT(windowManagement);
    wm.setup(windowManagement);
    org_kde_plasma_window_management_add_listener(windowManagement, &s_listener, this);
}

void PlasmaWindowManagement::Private::showDesktopCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private*>(data);
    Q_ASSERT(wm->wm == org_kde_plasma_window_management);
    switch (state) {
    case ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED:
        wm->setShowDesktop(true);
        break;
    case ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED:
        wm->setShowDesktop(false);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

void PlasmaWindowManagement::Private::setShowDesktop(bool set)
{
    if (showingDesktop == set) {
        return;
    }
    showingDesktop = set;
    emit q->showingDesktopChanged(showingDesktop);
}

PlasmaWindowManagement::PlasmaWindowManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PlasmaWindowManagement::~PlasmaWindowManagement()
{
    release();
}

void PlasmaWindowManagement::destroy()
{
    if (!d->wm) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->wm.destroy();
}

void PlasmaWindowManagement::release()
{
    if (!d->wm) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->wm.release();
}

void PlasmaWindowManagement::setup(org_kde_plasma_window_management *wm)
{
    d->setup(wm);
}

void PlasmaWindowManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaWindowManagement::eventQueue()
{
    return d->queue;
}

bool PlasmaWindowManagement::isValid() const
{
    return d->wm.isValid();
}

PlasmaWindowManagement::operator org_kde_plasma_window_management*()
{
    return d->wm;
}

PlasmaWindowManagement::operator org_kde_plasma_window_management*() const
{
    return d->wm;
}

void PlasmaWindowManagement::hideDesktop()
{
    setShowingDesktop(false);
}

void PlasmaWindowManagement::showDesktop()
{
    setShowingDesktop(true);
}

void PlasmaWindowManagement::setShowingDesktop(bool show)
{
    org_kde_plasma_window_management_show_desktop(d->wm, show ? ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED : ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED);
}

bool PlasmaWindowManagement::isShowingDesktop() const
{
    return d->showingDesktop;
}

}
}
