/****************************************************************************
Copyright 2016  Martin Gräßlin <mgraesslin@kde.org>

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
#include <wayland-xdg-shell-v5-client-protocol.h>

namespace KWayland
{
namespace Client
{

class XdgShellUnstableV5::Private : public XdgShell::Private
{
public:
    void setupV5(xdg_shell *shell) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    XdgShellSurface *getXdgSurface(Surface *surface, QObject *parent) override;
    XdgShellPopup *getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent) override;
    operator xdg_shell*() override {
        return xdgshellv5;
    }
    operator xdg_shell*() const override {
        return xdgshellv5;
    }

    WaylandPointer<xdg_shell, xdg_shell_destroy> xdgshellv5;
};

void XdgShellUnstableV5::Private::setupV5(xdg_shell *shell)
{
    Q_ASSERT(shell);
    Q_ASSERT(!xdgshellv5);
    xdgshellv5.setup(shell);
    xdg_shell_use_unstable_version(xdgshellv5, 5);
}

void XdgShellUnstableV5::Private::release()
{
    xdgshellv5.release();
}

void XdgShellUnstableV5::Private::destroy()
{
    xdgshellv5.destroy();
}

bool XdgShellUnstableV5::Private::isValid() const
{
    return xdgshellv5.isValid();
}

XdgShellSurface *XdgShellUnstableV5::Private::getXdgSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    XdgShellSurface *s = new XdgShellSurfaceUnstableV5(parent);
    auto w = xdg_shell_get_xdg_surface(xdgshellv5, *surface);
    if (queue) {
        queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

XdgShellPopup *XdgShellUnstableV5::Private::getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent)
{
    Q_ASSERT(isValid());
    XdgShellPopup *s = new XdgShellPopupUnstableV5(parent);
    auto w = xdg_shell_get_xdg_popup(xdgshellv5, *surface, *parentSurface, *seat, serial, parentPos.x(), parentPos.y());
    if (queue) {
        queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

XdgShellUnstableV5::XdgShellUnstableV5(QObject *parent)
    : XdgShell(new Private,  parent)
{
}

XdgShellUnstableV5::~XdgShellUnstableV5() = default;

class XdgShellSurfaceUnstableV5::Private : public XdgShellSurface::Private
{
public:
    Private(XdgShellSurface *q);
    WaylandPointer<xdg_surface, xdg_surface_destroy> xdgsurfacev5;

    void setupV5(xdg_surface *surface) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    operator xdg_surface*() override {
        return xdgsurfacev5;
    }
    operator xdg_surface*() const override {
        return xdgsurfacev5;
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

private:
    static void configureCallback(void *data, xdg_surface *xdg_surface, int32_t width, int32_t height, wl_array *states, uint32_t serial);
    static void closeCallback(void *data, xdg_surface *xdg_surface);

    static const struct xdg_surface_listener s_listener;
};

const struct xdg_surface_listener XdgShellSurfaceUnstableV5::Private::s_listener = {
    configureCallback,
    closeCallback
};

void XdgShellSurfaceUnstableV5::Private::configureCallback(void *data, xdg_surface *xdg_surface, int32_t width, int32_t height, wl_array *wlStates, uint32_t serial)
{
    auto s = reinterpret_cast<XdgShellSurfaceUnstableV5::Private*>(data);
    Q_ASSERT(s->xdgsurfacev5 == xdg_surface);
    uint32_t *state = reinterpret_cast<uint32_t*>(wlStates->data);
    size_t numStates = wlStates->size / sizeof(uint32_t);
    States states;
    for (size_t i = 0; i < numStates; i++) {
        switch (state[i]) {
        case XDG_SURFACE_STATE_MAXIMIZED:
            states = states | XdgShellSurface::State::Maximized;
            break;
        case XDG_SURFACE_STATE_FULLSCREEN:
            states = states | XdgShellSurface::State::Fullscreen;
            break;
        case XDG_SURFACE_STATE_RESIZING:
            states = states | XdgShellSurface::State::Resizing;
            break;
        case XDG_SURFACE_STATE_ACTIVATED:
            states = states | XdgShellSurface::State::Activated;
            break;
        }
    }
    const QSize size = QSize(width, height);
    emit s->q->configureRequested(size, states, serial);
    if (!size.isNull()) {
        s->q->setSize(size);
    }
}

void XdgShellSurfaceUnstableV5::Private::closeCallback(void *data, xdg_surface *xdg_surface)
{
    auto s = reinterpret_cast<XdgShellSurfaceUnstableV5::Private*>(data);
    Q_ASSERT(s->xdgsurfacev5 == xdg_surface);
    emit s->q->closeRequested();
}

XdgShellSurfaceUnstableV5::Private::Private(XdgShellSurface *q)
    : XdgShellSurface::Private(q)
{
}

void XdgShellSurfaceUnstableV5::Private::setupV5(xdg_surface *surface)
{
    Q_ASSERT(surface);
    Q_ASSERT(!xdgsurfacev5);
    xdgsurfacev5.setup(surface);
    xdg_surface_add_listener(xdgsurfacev5, &s_listener, this);
}

void XdgShellSurfaceUnstableV5::Private::release()
{
    xdgsurfacev5.release();
}

void XdgShellSurfaceUnstableV5::Private::destroy()
{
    xdgsurfacev5.destroy();
}

bool XdgShellSurfaceUnstableV5::Private::isValid() const
{
    return xdgsurfacev5.isValid();
}


void XdgShellSurfaceUnstableV5::Private::setTransientFor(XdgShellSurface *parent)
{
    xdg_surface *parentSurface = nullptr;
    if (parent) {
        parentSurface = *parent;
    }
    xdg_surface_set_parent(xdgsurfacev5, parentSurface);
}

void XdgShellSurfaceUnstableV5::Private::setTitle(const QString & title)
{
    xdg_surface_set_title(xdgsurfacev5, title.toUtf8().constData());
}

void XdgShellSurfaceUnstableV5::Private::setAppId(const QByteArray & appId)
{
    xdg_surface_set_app_id(xdgsurfacev5, appId.constData());
}

void XdgShellSurfaceUnstableV5::Private::showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y)
{
    xdg_surface_show_window_menu(xdgsurfacev5, *seat, serial, x, y);
}

void XdgShellSurfaceUnstableV5::Private::move(Seat *seat, quint32 serial)
{
    xdg_surface_move(xdgsurfacev5, *seat, serial);
}

void XdgShellSurfaceUnstableV5::Private::resize(Seat *seat, quint32 serial, Qt::Edges edges)
{
    uint wlEdge = XDG_SURFACE_RESIZE_EDGE_NONE;
    if (edges.testFlag(Qt::TopEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::TopEdge)) {
            wlEdge = XDG_SURFACE_RESIZE_EDGE_TOP_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::TopEdge)) {
            wlEdge = XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT;
        } else if ((edges & ~Qt::TopEdge) == Qt::Edges()) {
            wlEdge = XDG_SURFACE_RESIZE_EDGE_TOP;
        }
    } else if (edges.testFlag(Qt::BottomEdge)) {
        if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::BottomEdge)) {
            wlEdge = XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT;
        } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::BottomEdge)) {
            wlEdge = XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT;
        } else if ((edges & ~Qt::BottomEdge) == Qt::Edges()) {
            wlEdge = XDG_SURFACE_RESIZE_EDGE_BOTTOM;
        }
    } else if (edges.testFlag(Qt::RightEdge) && ((edges & ~Qt::RightEdge) == Qt::Edges())) {
        wlEdge = XDG_SURFACE_RESIZE_EDGE_RIGHT;
    } else if (edges.testFlag(Qt::LeftEdge) && ((edges & ~Qt::LeftEdge) == Qt::Edges())) {
        wlEdge = XDG_SURFACE_RESIZE_EDGE_LEFT;
    }
    xdg_surface_resize(xdgsurfacev5, *seat, serial, wlEdge);
}

void XdgShellSurfaceUnstableV5::Private::ackConfigure(quint32 serial)
{
    xdg_surface_ack_configure(xdgsurfacev5, serial);
}

void XdgShellSurfaceUnstableV5::Private::setMaximized()
{
    xdg_surface_set_maximized(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::unsetMaximized()
{
    xdg_surface_unset_maximized(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::setFullscreen(Output *output)
{
    wl_output *o = nullptr;
    if (output) {
        o = *output;
    }
    xdg_surface_set_fullscreen(xdgsurfacev5, o);
}

void XdgShellSurfaceUnstableV5::Private::unsetFullscreen()
{
    xdg_surface_unset_fullscreen(xdgsurfacev5);
}

void XdgShellSurfaceUnstableV5::Private::setMinimized()
{
    xdg_surface_set_minimized(xdgsurfacev5);
}

XdgShellSurfaceUnstableV5::XdgShellSurfaceUnstableV5(QObject *parent)
    : XdgShellSurface(new Private(this), parent)
{
}

XdgShellSurfaceUnstableV5::~XdgShellSurfaceUnstableV5() = default;

class XdgShellPopupUnstableV5::Private : public XdgShellPopup::Private
{
public:
    Private(XdgShellPopup *q);

    void setupV5(xdg_popup *p) override;
    void release() override;
    void destroy() override;
    bool isValid() const override;
    operator xdg_popup*() override {
        return xdgpopupv5;
    }
    operator xdg_popup*() const override {
        return xdgpopupv5;
    }
    WaylandPointer<xdg_popup, xdg_popup_destroy> xdgpopupv5;

private:
    static void popupDoneCallback(void *data, xdg_popup *xdg_popup);
    static const struct xdg_popup_listener s_listener;
};

const struct xdg_popup_listener XdgShellPopupUnstableV5::Private::s_listener = {
    popupDoneCallback
};

void XdgShellPopupUnstableV5::Private::popupDoneCallback(void *data, xdg_popup *xdg_popup)
{
    auto s = reinterpret_cast<XdgShellPopupUnstableV5::Private*>(data);
    Q_ASSERT(s->xdgpopupv5 == xdg_popup);
    emit s->q->popupDone();
}

XdgShellPopupUnstableV5::Private::Private(XdgShellPopup *q)
    : XdgShellPopup::Private(q)
{
}

void XdgShellPopupUnstableV5::Private::setupV5(xdg_popup *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!xdgpopupv5);
    xdgpopupv5.setup(p);
    xdg_popup_add_listener(xdgpopupv5, &s_listener, this);
}

void XdgShellPopupUnstableV5::Private::release()
{
    xdgpopupv5.release();
}

void XdgShellPopupUnstableV5::Private::destroy()
{
    xdgpopupv5.destroy();
}

bool XdgShellPopupUnstableV5::Private::isValid() const
{
    return xdgpopupv5.isValid();
}

XdgShellPopupUnstableV5::XdgShellPopupUnstableV5(QObject *parent)
    : XdgShellPopup(new Private(this), parent)
{
}

XdgShellPopupUnstableV5::~XdgShellPopupUnstableV5() = default;

}
}
