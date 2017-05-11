/****************************************************************************
Copyright 2017  David Edmundson <davidedmundson@kde.org>

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
#include "xdgshell_p.h"
#include "event_queue.h"
#include "output.h"
#include "seat.h"
#include "surface.h"
#include "wayland_pointer_p.h"
#include <wayland-xdg-shell-v6-client-protocol.h>

namespace KWayland
{
namespace Client
{

class XdgShellUnstableV6::Private : public XdgShell::Private
{
public:
    void setupV6(zxdg_shell_v6 *shell) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    XdgShellSurface *getXdgSurface(Surface *surface, QObject *parent) override;
    XdgShellPopup *getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent) override;
    operator zxdg_shell_v6*() override {
        return xdgshellv6;
    }
    operator zxdg_shell_v6*() const override {
        return xdgshellv6;
    }

    WaylandPointer<zxdg_shell_v6, zxdg_shell_v6_destroy> xdgshellv6;
};

void XdgShellUnstableV6::Private::setupV6(zxdg_shell_v6 *shell)
{
    Q_ASSERT(shell);
    Q_ASSERT(!xdgshellv6);
    xdgshellv6.setup(shell);
    //FIXME?
//     xdg_shell_use_unstable_version(xdgshellv6, 6);
}

void XdgShellUnstableV6::Private::release()
{
    xdgshellv6.release();
}

void XdgShellUnstableV6::Private::destroy()
{
    xdgshellv6.destroy();
}

bool XdgShellUnstableV6::Private::isValid() const
{
    return xdgshellv6.isValid();
}

XdgShellSurface *XdgShellUnstableV6::Private::getXdgSurface(Surface *surface, QObject *parent)
{
    //To match API, the XdgShellSurface configusingly wraps the top level, not the surface

    Q_ASSERT(isValid());
    auto ss = zxdg_shell_v6_get_xdg_surface(xdgshellv6, *surface);

    if (!ss) {
        return nullptr;
    }

    auto s = new XdgTopLevelUnstableV6(parent);
    auto toplevel = zxdg_surface_v6_get_toplevel(ss);
    if (queue) {
        queue->addProxy(toplevel);
    }
    s->setup(ss, toplevel);
    return s;
}

XdgShellPopup *XdgShellUnstableV6::Private::getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent)
{
    Q_ASSERT(isValid());
    XdgShellPopup *s = new XdgShellPopupUnstableV6(parent);

    //FIXME
    //the old XDG made popups on the shell
    //v6 makes it on the xdgsurface
    //

//     auto w = zxdg_surface_v6_get_popup(, *parentSurface, *seat, serial, parentPos.x(), parentPos.y());
//     if (queue) {
//         queue->addProxy(w);
//     }
//     s->setup(w);
//     return s;
    return s;
}

XdgShellUnstableV6::XdgShellUnstableV6(QObject *parent)
    : XdgShell(new Private,  parent)
{
}

XdgShellUnstableV6::~XdgShellUnstableV6() = default;


//A top level wraps both xdg_surface_v6 and xdg_top_level    into the public API XdgShelllSurface
class XdgTopLevelUnstableV6::Private : public XdgShellSurface::Private
{
public:
    Private(XdgShellSurface *q);
    WaylandPointer<zxdg_toplevel_v6, zxdg_toplevel_v6_destroy> xdgtoplevelv6;
    WaylandPointer<zxdg_surface_v6, zxdg_surface_v6_destroy> xdgsurfacev6;

    void setupV6(zxdg_surface_v6 *surface, zxdg_toplevel_v6 *toplevel) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    operator zxdg_toplevel_v6*() override {
        return xdgtoplevelv6;
    }
    operator zxdg_toplevel_v6*() const override {
        return xdgtoplevelv6;
    }

    void setTransientFor(XdgShellSurface *parent) override;
    void setTitle(const QString &title) override;
    void setAppId(const QByteArray &appId) override;
    void showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y) override;
    void move(Seat *seat, quint32 serial) override;
    void resize(Seat *seat, quint32 serial, Qt::Edges edges) override;
    void ackConfigure(quint32 serial) override;
    void setMaximized() override;
    void unsetMaximized() override;
    void setFullscreen(Output *output) override;
    void unsetFullscreen() override;
    void setMinimized() override;
    void setMaxSize(const QSize &size) override;
    void setMinSize(const QSize &size) override;

private:
    static void configureCallback(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *state);
    static void closeCallback(void *data, zxdg_toplevel_v6 *xdg_toplevel);
    static void surfaceConfigureCallback(void *data, zxdg_surface_v6 *xdg_surface, uint32_t serial);

     static const struct zxdg_toplevel_v6_listener s_toplevelListener;
     static const struct zxdg_surface_v6_listener s_surfaceListener;
};

const struct zxdg_toplevel_v6_listener XdgTopLevelUnstableV6::Private::s_toplevelListener = {
    configureCallback,
    closeCallback
};

const struct zxdg_surface_v6_listener XdgTopLevelUnstableV6::Private::s_surfaceListener = {
    surfaceConfigureCallback
};

void XdgTopLevelUnstableV6::Private::surfaceConfigureCallback(void *data, struct zxdg_surface_v6 *surface, uint32_t serial)
{

}

void XdgTopLevelUnstableV6::Private::configureCallback(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *state)
{
}

void XdgTopLevelUnstableV6::Private::closeCallback(void *data, zxdg_toplevel_v6 *xdg_toplevel)
{
    auto s = reinterpret_cast<XdgTopLevelUnstableV6::Private*>(data);
    Q_ASSERT(s->xdgtoplevelv6 == xdg_toplevel);
    emit s->q->closeRequested();
}

XdgTopLevelUnstableV6::Private::Private(XdgShellSurface *q)
    : XdgShellSurface::Private(q)
{
}

void XdgTopLevelUnstableV6::Private::setupV6(zxdg_surface_v6 *surface, zxdg_toplevel_v6 *topLevel)
{
    Q_ASSERT(surface);
    Q_ASSERT(!xdgtoplevelv6);
    xdgsurfacev6.setup(surface);
    xdgtoplevelv6.setup(topLevel);
    zxdg_surface_v6_add_listener(xdgsurfacev6, &s_surfaceListener, this);
    zxdg_toplevel_v6_add_listener(xdgtoplevelv6, &s_toplevelListener, this);
}

void XdgTopLevelUnstableV6::Private::release()
{
    xdgtoplevelv6.release();
    xdgsurfacev6.release();
}

void XdgTopLevelUnstableV6::Private::destroy()
{
    xdgtoplevelv6.destroy();
    xdgsurfacev6.destroy();
}

bool XdgTopLevelUnstableV6::Private::isValid() const
{
    return xdgtoplevelv6.isValid() && xdgsurfacev6.isValid();
}

void XdgTopLevelUnstableV6::Private::setTransientFor(XdgShellSurface *parent)
{
    zxdg_toplevel_v6 *parentSurface = nullptr;
    if (parent) {
        parentSurface = *parent;
    }
    zxdg_toplevel_v6_set_parent(xdgtoplevelv6, parentSurface);
}

void XdgTopLevelUnstableV6::Private::setTitle(const QString & title)
{
    zxdg_toplevel_v6_set_title(xdgtoplevelv6, title.toUtf8().constData());
}

void XdgTopLevelUnstableV6::Private::setAppId(const QByteArray & appId)
{
    zxdg_toplevel_v6_set_app_id(xdgtoplevelv6, appId.constData());
}

void XdgTopLevelUnstableV6::Private::showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y)
{
    zxdg_toplevel_v6_show_window_menu(xdgtoplevelv6, *seat, serial, x, y);
}

void XdgTopLevelUnstableV6::Private::move(Seat *seat, quint32 serial)
{
    zxdg_toplevel_v6_move(xdgtoplevelv6, *seat, serial);
}

void XdgTopLevelUnstableV6::Private::resize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    uint wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_NONE;
    if (edges.testFlag(Qt::TopEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::TopEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::TopEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_RIGHT;
        } else if ((edges & ~Qt::TopEdge) == Qt::Edges()) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP;
        }
    } else if (edges.testFlag(Qt::BottomEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::BottomEdge)) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_RIGHT;
        } else if ((edges & ~Qt::BottomEdge) == Qt::Edges()) {
            wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM;
        }
    } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::Edges())) {
        wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_RIGHT;
    } else if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::Edges())) {
        wlEdge = ZXDG_TOPLEVEL_V6_RESIZE_EDGE_LEFT;
    }
    zxdg_toplevel_v6_resize(xdgtoplevelv6, *seat, serial, wlEdge);
}

void XdgTopLevelUnstableV6::Private::ackConfigure(quint32 serial)
{
    zxdg_surface_v6_ack_configure(xdgsurfacev6, serial);
}

void XdgTopLevelUnstableV6::Private::setMaximized()
{
    zxdg_toplevel_v6_set_maximized(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::unsetMaximized()
{
    zxdg_toplevel_v6_unset_maximized(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::setFullscreen(Output *output)
{
    wl_output *o = nullptr;
    if (output) {
        o = *output;
    }
    zxdg_toplevel_v6_set_fullscreen(xdgtoplevelv6, o);
}

void XdgTopLevelUnstableV6::Private::unsetFullscreen()
{
    zxdg_toplevel_v6_unset_fullscreen(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::setMinimized()
{
    zxdg_toplevel_v6_set_minimized(xdgtoplevelv6);
}

void XdgTopLevelUnstableV6::Private::setMaxSize(const QSize &size)
{
    zxdg_toplevel_v6_set_max_size(xdgtoplevelv6, size.width(), size.height());
}

void XdgTopLevelUnstableV6::Private::setMinSize(const QSize &size)
{
    zxdg_toplevel_v6_set_min_size(xdgtoplevelv6, size.width(), size.height());
}

XdgTopLevelUnstableV6::XdgTopLevelUnstableV6(QObject *parent)
    : XdgShellSurface(new Private(this), parent)
{
}

XdgTopLevelUnstableV6::~XdgTopLevelUnstableV6() = default;

class XdgShellPopupUnstableV6::Private : public XdgShellPopup::Private
{
public:
    Private(XdgShellPopup *q);

    void setupV6(zxdg_popup_v6 *p) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    operator zxdg_popup_v6*() override {
        return xdgpopupv6;
    }
    operator zxdg_popup_v6*() const override {
        return xdgpopupv6;
    }
    WaylandPointer<zxdg_popup_v6, zxdg_popup_v6_destroy> xdgpopupv6;

private:
    static void configureCallback(void *data, zxdg_popup_v6 *xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height);
    static void popupDoneCallback(void *data, zxdg_popup_v6 *xdg_popup);
    static const struct zxdg_popup_v6_listener s_listener;
};

const struct zxdg_popup_v6_listener XdgShellPopupUnstableV6::Private::s_listener = {
    configureCallback,
    popupDoneCallback
};

void XdgShellPopupUnstableV6::Private::configureCallback(void *data, zxdg_popup_v6 *xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
    //FIXME
}

void XdgShellPopupUnstableV6::Private::popupDoneCallback(void *data, zxdg_popup_v6 *xdg_popup)
{
    auto s = reinterpret_cast<XdgShellPopupUnstableV6::Private*>(data);
    Q_ASSERT(s->xdgpopupv6 == xdg_popup);
    emit s->q->popupDone();
}

XdgShellPopupUnstableV6::Private::Private(XdgShellPopup *q)
    : XdgShellPopup::Private(q)
{
}

void XdgShellPopupUnstableV6::Private::setupV6(zxdg_popup_v6 *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!xdgpopupv6);
    xdgpopupv6.setup(p);
    zxdg_popup_v6_add_listener(xdgpopupv6, &s_listener, this);
}

void XdgShellPopupUnstableV6::Private::release()
{
    xdgpopupv6.release();
}

void XdgShellPopupUnstableV6::Private::destroy()
{
    xdgpopupv6.destroy();
}

bool XdgShellPopupUnstableV6::Private::isValid() const
{
    return xdgpopupv6.isValid();
}

XdgShellPopupUnstableV6::XdgShellPopupUnstableV6(QObject *parent)
    : XdgShellPopup(new Private(this), parent)
{
}

XdgShellPopupUnstableV6::~XdgShellPopupUnstableV6() = default;

}
}
