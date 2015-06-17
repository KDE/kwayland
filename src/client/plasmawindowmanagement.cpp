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
    QList<PlasmaWindow*> windows;

    void setup(org_kde_plasma_window_management *wm);

private:
    static void showDesktopCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state);
    static void windowCreatedCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, org_kde_plasma_window *id);
    void setShowDesktop(bool set);
    void windowCreated(org_kde_plasma_window *id);

    static struct org_kde_plasma_window_management_listener s_listener;
    PlasmaWindowManagement *q;
};

PlasmaWindowManagement::Private::Private(PlasmaWindowManagement *q)
    : q(q)
{
}

org_kde_plasma_window_management_listener PlasmaWindowManagement::Private::s_listener = {
    showDesktopCallback,
    windowCreatedCallback
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

void PlasmaWindowManagement::Private::windowCreatedCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, org_kde_plasma_window *id)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private*>(data);
    Q_ASSERT(wm->wm == org_kde_plasma_window_management);
    wm->windowCreated(id);
}

void PlasmaWindowManagement::Private::windowCreated(org_kde_plasma_window *id)
{
    if (queue) {
        queue->addProxy(id);
    }
    PlasmaWindow *window = new PlasmaWindow(q, id);
    windows << window;
    QObject::connect(window, &QObject::destroyed, q,
        [this, window] {
            windows.removeAll(window);
        }
    );
    emit q->windowCreated(window);
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

QList< PlasmaWindow* > PlasmaWindowManagement::windows() const
{
    return d->windows;
}

class PlasmaWindow::Private
{
public:
    Private(org_kde_plasma_window *window, PlasmaWindow *q);
    WaylandPointer<org_kde_plasma_window, org_kde_plasma_window_destroy> window;
    QString title;
    QString appId;
    quint32 desktop = 0;

private:
    static void titleChangedCallback(void *data, org_kde_plasma_window *window, const char *title);
    static void appIdChangedCallback(void *data, org_kde_plasma_window *window, const char *app_id);
    static void virtualDesktopChangedCallback(void *data, org_kde_plasma_window *window, int32_t number);
    static void unmappedCallback(void *data, org_kde_plasma_window *window);

    static Private *cast(void *data) {
        return reinterpret_cast<Private*>(data);
    }

    PlasmaWindow *q;

    static struct org_kde_plasma_window_listener s_listener;
};

org_kde_plasma_window_listener PlasmaWindow::Private::s_listener = {
    titleChangedCallback,
    appIdChangedCallback,
    virtualDesktopChangedCallback,
    unmappedCallback
};

void PlasmaWindow::Private::titleChangedCallback(void *data, org_kde_plasma_window *window, const char *title)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString t = QString::fromUtf8(title);
    if (p->title == t) {
        return;
    }
    p->title = t;
    emit p->q->titleChanged();
}

void PlasmaWindow::Private::appIdChangedCallback(void *data, org_kde_plasma_window *window, const char *appId)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString s = QString::fromUtf8(appId);
    if (s == p->appId) {
        return;
    }
    p->appId = s;
    emit p->q->appIdChanged();
}

void PlasmaWindow::Private::virtualDesktopChangedCallback(void *data, org_kde_plasma_window *window, int32_t number)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (p->desktop == number) {
        return;
    }
    p->desktop = number;
    emit p->q->virtualDesktopChanged();
}

void PlasmaWindow::Private::unmappedCallback(void *data, org_kde_plasma_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    emit p->q->unmapped();
    p->q->deleteLater();
}

PlasmaWindow::Private::Private(org_kde_plasma_window *w, PlasmaWindow *q)
    : q(q)
{
    window.setup(w);
    org_kde_plasma_window_add_listener(w, &s_listener, this);
}

PlasmaWindow::PlasmaWindow(PlasmaWindowManagement *parent, org_kde_plasma_window *window)
    : QObject(parent)
    , d(new Private(window, this))
{
}

PlasmaWindow::~PlasmaWindow()
{
    release();
}

void PlasmaWindow::destroy()
{
    d->window.destroy();
}

void PlasmaWindow::release()
{
    d->window.release();
}

bool PlasmaWindow::isValid() const
{
    return d->window.isValid();
}

PlasmaWindow::operator org_kde_plasma_window*() const
{
    return d->window;
}

PlasmaWindow::operator org_kde_plasma_window*()
{
    return d->window;
}

QString PlasmaWindow::appId() const
{
    return d->appId;
}

QString PlasmaWindow::title() const
{
    return d->title;
}

quint32 PlasmaWindow::virtualDesktop() const
{
    return d->desktop;
}

}
}
