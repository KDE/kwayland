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
#include "plasmawindowmodel.h"
#include "event_queue.h"
#include "output.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-plasma-window-management-client-protocol.h>

#include <QTimer>

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
    PlasmaWindow *activeWindow = nullptr;

    void setup(org_kde_plasma_window_management *wm);

private:
    static void showDesktopCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state);
    static void windowCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id);
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
    windowCallback
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

void PlasmaWindowManagement::Private::windowCallback(void *data, org_kde_plasma_window_management *interface, uint32_t id)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private*>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(timer, &QTimer::timeout, wm->q,
        [timer, wm, id] {
            wm->windowCreated(org_kde_plasma_window_management_get_window(wm->wm, id));
            timer->deleteLater();
        }, Qt::QueuedConnection
    );
    timer->start();
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
    QObject::connect(window, &PlasmaWindow::activeChanged, q,
        [this, window] {
            if (window->isActive()) {
                if (activeWindow == window) {
                    return;
                }
                activeWindow = window;
                emit q->activeWindowChanged();
            } else {
                if (activeWindow == window) {
                    activeWindow = nullptr;
                    emit q->activeWindowChanged();
                }
            }
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

PlasmaWindow *PlasmaWindowManagement::activeWindow() const
{
    return d->activeWindow;
}

PlasmaWindowModel *PlasmaWindowManagement::createWindowModel()
{
    return new PlasmaWindowModel(this);
}


class PlasmaWindow::Private
{
public:
    Private(org_kde_plasma_window *window, PlasmaWindow *q);
    WaylandPointer<org_kde_plasma_window, org_kde_plasma_window_destroy> window;
    QString title;
    QString appId;
    quint32 desktop = 0;
    bool active = false;
    bool minimized = false;
    bool maximized = false;
    bool fullscreen = false;
    bool keepAbove = false;
    bool keepBelow = false;
    bool onAllDesktops = false;
    bool demandsAttention = false;
    bool closeable = false;
    bool minimizeable = false;
    bool maximizeable = false;
    bool fullscreenable = false;
    QIcon icon;

private:
    static void titleChangedCallback(void *data, org_kde_plasma_window *window, const char *title);
    static void appIdChangedCallback(void *data, org_kde_plasma_window *window, const char *app_id);
    static void stateChangedCallback(void *data, org_kde_plasma_window *window, uint32_t state);
    static void virtualDesktopChangedCallback(void *data, org_kde_plasma_window *window, int32_t number);
    static void themedIconNameChangedCallback(void *data, org_kde_plasma_window *window, const char *name);
    static void unmappedCallback(void *data, org_kde_plasma_window *window);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setDemandsAttention(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);

    static Private *cast(void *data) {
        return reinterpret_cast<Private*>(data);
    }

    PlasmaWindow *q;

    static struct org_kde_plasma_window_listener s_listener;
};

org_kde_plasma_window_listener PlasmaWindow::Private::s_listener = {
    titleChangedCallback,
    appIdChangedCallback,
    stateChangedCallback,
    virtualDesktopChangedCallback,
    themedIconNameChangedCallback,
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

void PlasmaWindow::Private::stateChangedCallback(void *data, org_kde_plasma_window *window, uint32_t state)
{
    auto p = cast(data);
    Q_UNUSED(window);
    p->setActive(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE);
    p->setMinimized(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED);
    p->setMaximized(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED);
    p->setFullscreen(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN);
    p->setKeepAbove(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE);
    p->setKeepBelow(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW);
    p->setOnAllDesktops(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ON_ALL_DESKTOPS);
    p->setDemandsAttention(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_DEMANDS_ATTENTION);
    p->setCloseable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_CLOSEABLE);
    p->setFullscreenable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREENABLE);
    p->setMaximizeable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZABLE);
    p->setMinimizeable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZABLE);
}

void PlasmaWindow::Private::themedIconNameChangedCallback(void *data, org_kde_plasma_window *window, const char *name)
{
    auto p = cast(data);
    Q_UNUSED(window);
    QIcon icon = QIcon::fromTheme(QString::fromUtf8(name));
    p->icon = icon;
    emit p->q->iconChanged();
}

void PlasmaWindow::Private::setActive(bool set)
{
    if (active == set) {
        return;
    }
    active = set;
    emit q->activeChanged();
}

void PlasmaWindow::Private::setFullscreen(bool set)
{
    if (fullscreen == set) {
        return;
    }
    fullscreen = set;
    emit q->fullscreenChanged();
}

void PlasmaWindow::Private::setKeepAbove(bool set)
{
    if (keepAbove == set) {
        return;
    }
    keepAbove = set;
    emit q->keepAboveChanged();
}

void PlasmaWindow::Private::setKeepBelow(bool set)
{
    if (keepBelow == set) {
        return;
    }
    keepBelow = set;
    emit q->keepBelowChanged();
}

void PlasmaWindow::Private::setMaximized(bool set)
{
    if (maximized == set) {
        return;
    }
    maximized = set;
    emit q->maximizedChanged();
}

void PlasmaWindow::Private::setMinimized(bool set)
{
    if (minimized == set) {
        return;
    }
    minimized = set;
    emit q->minimizedChanged();
}

void PlasmaWindow::Private::setOnAllDesktops(bool set)
{
    if (onAllDesktops == set) {
        return;
    }
    onAllDesktops = set;
    emit q->onAllDesktopsChanged();
}

void PlasmaWindow::Private::setDemandsAttention(bool set)
{
    if (demandsAttention == set) {
        return;
    }
    demandsAttention = set;
    emit q->demandsAttentionChanged();
}

void PlasmaWindow::Private::setCloseable(bool set)
{
    if (closeable == set) {
        return;
    }
    closeable = set;
    emit q->closeableChanged();
}

void PlasmaWindow::Private::setFullscreenable(bool set)
{
    if (fullscreenable == set) {
        return;
    }
    fullscreenable = set;
    emit q->fullscreenableChanged();
}

void PlasmaWindow::Private::setMaximizeable(bool set)
{
    if (maximizeable == set) {
        return;
    }
    maximizeable = set;
    emit q->maximizeableChanged();
}

void PlasmaWindow::Private::setMinimizeable(bool set)
{
    if (minimizeable == set) {
        return;
    }
    minimizeable = set;
    emit q->minimizeableChanged();
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

bool PlasmaWindow::isActive() const
{
    return d->active;
}

bool PlasmaWindow::isFullscreen() const
{
    return d->fullscreen;
}

bool PlasmaWindow::isKeepAbove() const
{
    return d->keepAbove;
}

bool PlasmaWindow::isKeepBelow() const
{
    return d->keepBelow;
}

bool PlasmaWindow::isMaximized() const
{
    return d->maximized;
}

bool PlasmaWindow::isMinimized() const
{
    return d->minimized;
}

bool PlasmaWindow::isOnAllDesktops() const
{
    return d->onAllDesktops;
}

bool PlasmaWindow::isDemandingAttention() const
{
    return d->demandsAttention;
}

bool PlasmaWindow::isCloseable() const
{
    return d->closeable;
}

bool PlasmaWindow::isFullscreenable() const
{
    return d->fullscreenable;
}

bool PlasmaWindow::isMaximizeable() const
{
    return d->maximizeable;
}

bool PlasmaWindow::isMinimizeable() const
{
    return d->minimizeable;
}

QIcon PlasmaWindow::icon() const
{
    return d->icon;
}

void PlasmaWindow::requestActivate()
{
    org_kde_plasma_window_set_state(d->window,
        ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE,
        ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE);
}

void PlasmaWindow::requestClose()
{
    org_kde_plasma_window_close(d->window);
}

void PlasmaWindow::requestVirtualDesktop(quint32 desktop)
{
    org_kde_plasma_window_set_virtual_desktop(d->window, desktop);
}

}
}
