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
#include "plasmawindowmanagement_interface.h"
#include "global_p.h"
#include "resource_p.h"
#include "display.h"

#include <QList>
#include <QVector>

#include <wayland-server.h>
#include <wayland-plasma-window-management-server-protocol.h>

namespace KWayland
{
namespace Server
{

class PlasmaWindowManagementInterface::Private : public Global::Private
{
public:
    Private(PlasmaWindowManagementInterface *q, Display *d);
    void sendShowingDesktopState();

    ShowingDesktopState state = ShowingDesktopState::Disabled;
    QVector<wl_resource*> resources;
    QList<PlasmaWindowInterface*> windows;
    quint32 windowIdCounter = 0;

private:
    static void unbind(wl_resource *resource);
    static void showDesktopCallback(wl_client *client, wl_resource *resource, uint32_t state);
    static void getWindowCallback(wl_client *client, wl_resource *resource, uint32_t id, uint32_t internalWindowId);

    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void sendShowingDesktopState(wl_resource *r);

    PlasmaWindowManagementInterface *q;
    static const struct org_kde_plasma_window_management_interface s_interface;
    static const quint32 s_version;
};

class PlasmaWindowInterface::Private
{
public:
    Private(PlasmaWindowManagementInterface *wm, PlasmaWindowInterface *q);
    ~Private();

    void createResource(wl_resource *parent, uint32_t id);
    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setThemedIconName(const QString &iconName);
    void setVirtualDesktop(quint32 desktop);
    void unmap();
    void setState(org_kde_plasma_window_management_state flag, bool set);

    struct WindowResource {
        wl_resource *resource;
        wl_listener *destroyListener;
    };
    QList<WindowResource> resources;
    quint32 windowId = 0;

private:
    static void unbind(wl_resource *resource);
    static void destroyListenerCallback(wl_listener *listener, void *data);
    static void setStateCallback(wl_client *client, wl_resource *resource, uint32_t flags, uint32_t state);
    static void setVirtualDesktopCallback(wl_client *client, wl_resource *resource, uint32_t number);
    static void closeCallback(wl_client *client, wl_resource *resource);
    static Private *cast(wl_resource *resource) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    }

    PlasmaWindowInterface *q;
    PlasmaWindowManagementInterface *wm;
    QString m_title;
    QString m_appId;
    QString m_themedIconName;
    quint32 m_virtualDesktop = 0;
    quint32 m_state = 0;
    wl_listener listener;
    static const struct org_kde_plasma_window_interface s_interface;
};

const quint32 PlasmaWindowManagementInterface::Private::s_version = 1;

PlasmaWindowManagementInterface::Private::Private(PlasmaWindowManagementInterface *q, Display *d)
    : Global::Private(d, &org_kde_plasma_window_management_interface, s_version)
    , q(q)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_plasma_window_management_interface PlasmaWindowManagementInterface::Private::s_interface = {
    showDesktopCallback,
    getWindowCallback
};
#endif

void PlasmaWindowManagementInterface::Private::sendShowingDesktopState()
{
    for (wl_resource *r : resources) {
        sendShowingDesktopState(r);
    }
}

void PlasmaWindowManagementInterface::Private::sendShowingDesktopState(wl_resource *r)
{
    uint32_t s = 0;
    switch (state) {
    case ShowingDesktopState::Enabled:
        s = ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED;
        break;
    case ShowingDesktopState::Disabled:
        s = ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    org_kde_plasma_window_management_send_show_desktop_changed(r, s);
}

void PlasmaWindowManagementInterface::Private::showDesktopCallback(wl_client *client, wl_resource *resource, uint32_t state)
{
    Q_UNUSED(client)
    ShowingDesktopState s = ShowingDesktopState::Disabled;
    switch (state) {
    case ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED:
        s = ShowingDesktopState::Enabled;
        break;
    case ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED:
    default:
        s = ShowingDesktopState::Disabled;
        break;
    }
    emit reinterpret_cast<Private*>(wl_resource_get_user_data(resource))->q->requestChangeShowingDesktop(s);
}

void PlasmaWindowManagementInterface::Private::getWindowCallback(wl_client *client, wl_resource *resource, uint32_t id, uint32_t internalWindowId)
{
    auto p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    auto it = std::find_if(p->windows.constBegin(), p->windows.constEnd(),
        [internalWindowId] (PlasmaWindowInterface *window) {
            return window->d->windowId == internalWindowId;
        }
    );
    if (it == p->windows.constEnd()) {
        ClientConnection *c = p->q->display()->getConnection(client);
        wl_resource *r = c->createResource(&org_kde_plasma_window_interface, wl_resource_get_version(resource), id);
        if (!r) {
            return;
        }
        org_kde_plasma_window_send_unmapped(r);
        wl_resource_destroy(r);
        return;
    }
    (*it)->d->createResource(resource, id);
}

PlasmaWindowManagementInterface::PlasmaWindowManagementInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

PlasmaWindowManagementInterface::~PlasmaWindowManagementInterface() = default;

void PlasmaWindowManagementInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *shell = c->createResource(&org_kde_plasma_window_management_interface, qMin(version, s_version), id);
    if (!shell) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(shell, &s_interface, this, unbind);
    resources << shell;
    for (auto it = windows.constBegin(); it != windows.constEnd(); ++it) {
        org_kde_plasma_window_management_send_window(shell, (*it)->d->windowId);
    }
}

void PlasmaWindowManagementInterface::Private::unbind(wl_resource *resource)
{
    auto wm = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    wm->resources.removeAll(resource);
}

void PlasmaWindowManagementInterface::setShowingDesktopState(PlasmaWindowManagementInterface::ShowingDesktopState state)
{
    Q_D();
    if (d->state == state) {
        return;
    }
    d->state = state;
    d->sendShowingDesktopState();
}

PlasmaWindowManagementInterface::Private *PlasmaWindowManagementInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

PlasmaWindowInterface *PlasmaWindowManagementInterface::createWindow(QObject *parent)
{
    Q_D();
    PlasmaWindowInterface *window = new PlasmaWindowInterface(this, parent);
    // TODO: improve window ids so that it cannot wrap around
    window->d->windowId = ++d->windowIdCounter;
    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_window_management_send_window(*it, window->d->windowId);
    }
    d->windows << window;
    connect(window, &QObject::destroyed, this,
        [this, window] {
            Q_D();
            d->windows.removeAll(window);
        }
    );
    return window;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_plasma_window_interface PlasmaWindowInterface::Private::s_interface = {
    setStateCallback,
    setVirtualDesktopCallback,
    closeCallback
};
#endif

PlasmaWindowInterface::Private::Private(PlasmaWindowManagementInterface *wm, PlasmaWindowInterface *q)
    : q(q)
    , wm(wm)
{
}

PlasmaWindowInterface::Private::~Private()
{
    // need to copy, as destroy goes through the destroy listener and modifies the list as we iterate
    const auto c = resources;
    for (const auto &r : c) {
        org_kde_plasma_window_send_unmapped(r.resource);
        wl_resource_destroy(r.resource);
    }
}

void PlasmaWindowInterface::Private::unbind(wl_resource *resource)
{
    Private *p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    auto it = p->resources.begin();
    while (it != p->resources.end()) {
        if ((*it).resource == resource) {
            wl_list_remove(&(*it).destroyListener->link);
            delete (*it).destroyListener;
            it = p->resources.erase(it);
        } else {
            it++;
        }
    }
}

void PlasmaWindowInterface::Private::destroyListenerCallback(wl_listener *listener, void *data)
{
    Q_UNUSED(listener);
    Private::unbind(reinterpret_cast<wl_resource*>(data));
}

void PlasmaWindowInterface::Private::createResource(wl_resource *parent, uint32_t id)
{
    ClientConnection *c = wm->display()->getConnection(wl_resource_get_client(parent));
    wl_resource *resource = c->createResource(&org_kde_plasma_window_interface, wl_resource_get_version(parent), id);
    if (!resource) {
        return;
    }
    WindowResource r;
    r.resource = resource;
    r.destroyListener = new wl_listener;
    r.destroyListener->notify = destroyListenerCallback;
    r.destroyListener->link.prev = nullptr;
    r.destroyListener->link.next = nullptr;
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    wl_resource_add_destroy_listener(resource, r.destroyListener);
    resources << r;

    org_kde_plasma_window_send_virtual_desktop_changed(resource, m_virtualDesktop);
    if (!m_appId.isEmpty()) {
        org_kde_plasma_window_send_app_id_changed(resource, m_appId.toUtf8().constData());
    }
    if (!m_title.isEmpty()) {
        org_kde_plasma_window_send_title_changed(resource, m_title.toUtf8().constData());
    }
    org_kde_plasma_window_send_state_changed(resource, m_state);
    org_kde_plasma_window_send_themed_icon_name_changed(resource, m_themedIconName.toUtf8().constData());
    c->flush();
}

void PlasmaWindowInterface::Private::setAppId(const QString &appId)
{
    if (m_appId == appId) {
        return;
    }
    m_appId = appId;
    const QByteArray utf8 = m_appId.toUtf8();
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        org_kde_plasma_window_send_app_id_changed((*it).resource, utf8.constData());
    }
}

void PlasmaWindowInterface::Private::setThemedIconName(const QString &iconName)
{
    if (m_themedIconName == iconName) {
        return;
    }
    m_themedIconName = iconName;
    const QByteArray utf8 = m_themedIconName.toUtf8();
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        org_kde_plasma_window_send_themed_icon_name_changed((*it).resource, utf8.constData());
    }
}

void PlasmaWindowInterface::Private::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    const QByteArray utf8 = m_title.toUtf8();
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        org_kde_plasma_window_send_title_changed((*it).resource, utf8.constData());
    }
}

void PlasmaWindowInterface::Private::setVirtualDesktop(quint32 desktop)
{
    if (m_virtualDesktop == desktop) {
        return;
    }
    m_virtualDesktop = desktop;
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        org_kde_plasma_window_send_virtual_desktop_changed((*it).resource, m_virtualDesktop);
    }
}

void PlasmaWindowInterface::Private::unmap()
{
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        org_kde_plasma_window_send_unmapped((*it).resource);
        wl_resource_destroy((*it).resource);
    }
}

void PlasmaWindowInterface::Private::setState(org_kde_plasma_window_management_state flag, bool set)
{
    quint32 newState = m_state;
    if (set) {
        newState |= flag;
    } else {
        newState &= ~flag;
    }
    if (newState == m_state) {
        return;
    }
    m_state = newState;
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        org_kde_plasma_window_send_state_changed((*it).resource, m_state);
    }
}

void PlasmaWindowInterface::Private::closeCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Private *p = cast(resource);
    emit p->q->closeRequested();
}

void PlasmaWindowInterface::Private::setVirtualDesktopCallback(wl_client *client, wl_resource *resource, uint32_t number)
{
    Q_UNUSED(client)
    Private *p = cast(resource);
    emit p->q->virtualDesktopRequested(number);
}

void PlasmaWindowInterface::Private::setStateCallback(wl_client *client, wl_resource *resource, uint32_t flags, uint32_t state)
{
    Q_UNUSED(client)
    Private *p = cast(resource);
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE) {
        emit p->q->activeRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED) {
        emit p->q->minimizedRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED) {
        emit p->q->maximizedRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN) {
        emit p->q->fullscreenRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE) {
        emit p->q->keepAboveRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW) {
        emit p->q->keepBelowRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_DEMANDS_ATTENTION) {
        emit p->q->demandsAttentionRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_DEMANDS_ATTENTION);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_CLOSEABLE) {
        emit p->q->closeableRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_CLOSEABLE);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZABLE) {
        emit p->q->minimizeableRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZABLE);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZABLE) {
        emit p->q->maximizeableRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZABLE);
    }
    if (flags & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREENABLE) {
        emit p->q->fullscreenableRequested(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREENABLE);
    }
}

PlasmaWindowInterface::PlasmaWindowInterface(PlasmaWindowManagementInterface *wm, QObject *parent)
    : QObject(parent)
    , d(new Private(wm, this))
{
}

PlasmaWindowInterface::~PlasmaWindowInterface() = default;

void PlasmaWindowInterface::setAppId(const QString &appId)
{
    d->setAppId(appId);
}

void PlasmaWindowInterface::setTitle(const QString &title)
{
    d->setTitle(title);
}

void PlasmaWindowInterface::setVirtualDesktop(quint32 desktop)
{
    d->setVirtualDesktop(desktop);
}

void PlasmaWindowInterface::unmap()
{
    d->unmap();
}

void PlasmaWindowInterface::setActive(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE, set);
}

void PlasmaWindowInterface::setFullscreen(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN, set);
}

void PlasmaWindowInterface::setKeepAbove(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE, set);
}

void PlasmaWindowInterface::setKeepBelow(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW, set);
}

void PlasmaWindowInterface::setMaximized(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED, set);
}

void PlasmaWindowInterface::setMinimized(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED, set);
}

void PlasmaWindowInterface::setOnAllDesktops(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ON_ALL_DESKTOPS, set);
}

void PlasmaWindowInterface::setDemandsAttention(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_DEMANDS_ATTENTION, set);
}

void PlasmaWindowInterface::setCloseable(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_CLOSEABLE, set);
}

void PlasmaWindowInterface::setFullscreenable(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREENABLE, set);
}

void PlasmaWindowInterface::setMaximizeable(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZABLE, set);
}

void PlasmaWindowInterface::setMinimizeable(bool set)
{
    d->setState(ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZABLE, set);
}

void PlasmaWindowInterface::setThemedIconName(const QString &iconName)
{
    d->setThemedIconName(iconName);
}

}
}
