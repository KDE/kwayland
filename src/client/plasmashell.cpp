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
#include "plasmashell.h"
#include "event_queue.h"
#include "output.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-plasma-shell-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN PlasmaShell::Private
{
public:
    WaylandPointer<org_kde_plasma_shell, org_kde_plasma_shell_destroy> shell;
    EventQueue *queue = nullptr;
};

class Q_DECL_HIDDEN PlasmaShellSurface::Private
{
public:
    Private(PlasmaShellSurface *q);
    ~Private();
    void setup(org_kde_plasma_surface *surface);

    WaylandPointer<org_kde_plasma_surface, org_kde_plasma_surface_destroy> surface;
    QSize size;
    QPointer<Surface> parentSurface;
    PlasmaShellSurface::Role role;

    static PlasmaShellSurface *get(Surface *surface);

private:
    static void autoHidingPanelHiddenCallback(void *data, org_kde_plasma_surface *org_kde_plasma_surface);
    static void autoHidingPanelShownCallback(void *data, org_kde_plasma_surface *org_kde_plasma_surface);

    PlasmaShellSurface *q;
    static QVector<Private*> s_surfaces;
    static const org_kde_plasma_surface_listener s_listener;
};

QVector<PlasmaShellSurface::Private*> PlasmaShellSurface::Private::s_surfaces;

PlasmaShell::PlasmaShell(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

PlasmaShell::~PlasmaShell()
{
    release();
}

void PlasmaShell::destroy()
{
    if (!d->shell) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->shell.destroy();
}

void PlasmaShell::release()
{
    if (!d->shell) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->shell.release();
}

void PlasmaShell::setup(org_kde_plasma_shell *shell)
{
    Q_ASSERT(!d->shell);
    Q_ASSERT(shell);
    d->shell.setup(shell);
}

void PlasmaShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaShell::eventQueue()
{
    return d->queue;
}

PlasmaShellSurface *PlasmaShell::createSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto kwS = Surface::get(surface);
    if (kwS) {
        if (auto s = PlasmaShellSurface::Private::get(kwS)) {
            return s;
        }
    }
    PlasmaShellSurface *s = new PlasmaShellSurface(parent);
    connect(this, &PlasmaShell::interfaceAboutToBeReleased, s, &PlasmaShellSurface::release);
    connect(this, &PlasmaShell::interfaceAboutToBeDestroyed, s, &PlasmaShellSurface::destroy);
    auto w = org_kde_plasma_shell_get_surface(d->shell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    s->d->parentSurface = QPointer<Surface>(kwS);
    return s;
}

PlasmaShellSurface *PlasmaShell::createSurface(Surface *surface, QObject *parent)
{
    return createSurface(*surface, parent);
}

bool PlasmaShell::isValid() const
{
    return d->shell.isValid();
}

PlasmaShell::operator org_kde_plasma_shell*()
{
    return d->shell;
}

PlasmaShell::operator org_kde_plasma_shell*() const
{
    return d->shell;
}

PlasmaShellSurface::Private::Private(PlasmaShellSurface *q)
    : role(PlasmaShellSurface::Role::Normal),
      q(q)
{
    s_surfaces << this;
}

PlasmaShellSurface::Private::~Private()
{
    s_surfaces.removeAll(this);
}

PlasmaShellSurface *PlasmaShellSurface::Private::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_surfaces.constBegin(); it != s_surfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == surface) {
            return (*it)->q;
        }
    }
    return nullptr;
}

void PlasmaShellSurface::Private::setup(org_kde_plasma_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
    org_kde_plasma_surface_add_listener(surface, &s_listener, this);
}

const org_kde_plasma_surface_listener PlasmaShellSurface::Private::s_listener = {
    autoHidingPanelHiddenCallback,
    autoHidingPanelShownCallback
};

void PlasmaShellSurface::Private::autoHidingPanelHiddenCallback(void *data, org_kde_plasma_surface *org_kde_plasma_surface)
{
    auto p = reinterpret_cast<PlasmaShellSurface::Private*>(data);
    Q_ASSERT(p->surface == org_kde_plasma_surface);
    emit p->q->autoHidePanelHidden();
}

void PlasmaShellSurface::Private::autoHidingPanelShownCallback(void *data, org_kde_plasma_surface *org_kde_plasma_surface)
{
    auto p = reinterpret_cast<PlasmaShellSurface::Private*>(data);
    Q_ASSERT(p->surface == org_kde_plasma_surface);
    emit p->q->autoHidePanelShown();
}

PlasmaShellSurface::PlasmaShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PlasmaShellSurface::~PlasmaShellSurface()
{
    release();
}

void PlasmaShellSurface::release()
{
    d->surface.release();
}

void PlasmaShellSurface::destroy()
{
    d->surface.destroy();
}

void PlasmaShellSurface::setup(org_kde_plasma_surface *surface)
{
    d->setup(surface);
}

PlasmaShellSurface *PlasmaShellSurface::get(Surface *surface)
{
    if (auto s = PlasmaShellSurface::Private::get(surface)) {
        return s;
    }

    return nullptr;
}

bool PlasmaShellSurface::isValid() const
{
    return d->surface.isValid();
}

PlasmaShellSurface::operator org_kde_plasma_surface*()
{
    return d->surface;
}

PlasmaShellSurface::operator org_kde_plasma_surface*() const
{
    return d->surface;
}

void PlasmaShellSurface::setPosition(const QPoint& point)
{
    Q_ASSERT(isValid());
    org_kde_plasma_surface_set_position(d->surface, point.x(), point.y());
}

void PlasmaShellSurface::setRole(PlasmaShellSurface::Role role)
{
    Q_ASSERT(isValid());
    uint32_t wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_NORMAL;
    switch (role) {
    case Role::Normal:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_NORMAL;
        break;
    case Role::Desktop:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_DESKTOP;
        break;
    case Role::Panel:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_PANEL;
        break;
    case Role::OnScreenDisplay:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_ONSCREENDISPLAY;
        break;
    case Role::Notification:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_NOTIFICATION;
        break;
    case Role::ToolTip:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_TOOLTIP;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    org_kde_plasma_surface_set_role(d->surface, wlRole);
    d->role = role;
}

PlasmaShellSurface::Role PlasmaShellSurface::role() const
{
    return d->role;
}

void PlasmaShellSurface::setPanelBehavior(PlasmaShellSurface::PanelBehavior behavior)
{
    Q_ASSERT(isValid());
    uint32_t wlRole = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_ALWAYS_VISIBLE;
    switch (behavior) {
    case PanelBehavior::AlwaysVisible:
        wlRole = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_ALWAYS_VISIBLE;
        break;
    case PanelBehavior::AutoHide:
        wlRole = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_AUTO_HIDE;
        break;
    case PanelBehavior::WindowsCanCover:
        wlRole = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_WINDOWS_CAN_COVER;
        break;
    case PanelBehavior::WindowsGoBelow:
        wlRole = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_WINDOWS_GO_BELOW;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    org_kde_plasma_surface_set_panel_behavior(d->surface, wlRole);
}

void PlasmaShellSurface::setSkipTaskbar(bool skip)
{
    org_kde_plasma_surface_set_skip_taskbar(d->surface, skip);
}

void PlasmaShellSurface::setSkipSwitcher(bool skip)
{
    org_kde_plasma_surface_set_skip_switcher(d->surface, skip);
}

void PlasmaShellSurface::requestHideAutoHidingPanel()
{
    org_kde_plasma_surface_panel_auto_hide_hide(d->surface);
}

void PlasmaShellSurface::requestShowAutoHidingPanel()
{
    org_kde_plasma_surface_panel_auto_hide_show(d->surface);
}

void PlasmaShellSurface::setPanelTakesFocus(bool takesFocus)
{
    org_kde_plasma_surface_set_panel_takes_focus(d->surface, takesFocus);
}

}
}
