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
#ifndef KWAYLAND_CLIENT_XDGSHELL_P_H
#define KWAYLAND_CLIENT_XDGSHELL_P_H
#include "xdgshell.h"

#include <QSize>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN XdgShell::Private
{
public:
    virtual ~Private();
    virtual void setupV5(xdg_shell *xdgshellv5) {
        Q_UNUSED(xdgshellv5)
    }
    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() const = 0;
    virtual operator xdg_shell*() {
        return nullptr;
    }
    virtual operator xdg_shell*() const {
        return nullptr;
    }
    virtual XdgShellSurface *getXdgSurface(Surface *surface, QObject *parent) = 0;
    virtual XdgShellPopup *getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent) = 0;

    EventQueue *queue = nullptr;

protected:
    Private() = default;
};

class XdgShellUnstableV5 : public XdgShell
{
    Q_OBJECT
public:
    explicit XdgShellUnstableV5(QObject *parent = nullptr);
    virtual ~XdgShellUnstableV5();

private:
    class Private;
};

class Q_DECL_HIDDEN XdgShellSurface::Private
{
public:
    virtual ~Private();
    EventQueue *queue = nullptr;
    QSize size;

    virtual void setupV5(xdg_surface *surface) {
        Q_UNUSED(surface)
    }
    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() const = 0;
    virtual operator xdg_surface*() {
        return nullptr;
    }
    virtual operator xdg_surface*() const {
        return nullptr;
    }

    virtual void setTransientFor(XdgShellSurface *parent) = 0;
    virtual void setTitle(const QString &title) = 0;
    virtual void setAppId(const QByteArray &appId) = 0;
    virtual void showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y) = 0;
    virtual void move(Seat *seat, quint32 serial) = 0;
    virtual void resize(Seat *seat, quint32 serial, Qt::Edges edges) = 0;
    virtual void ackConfigure(quint32 serial) = 0;
    virtual void setMaximized() = 0;
    virtual void unsetMaximized() = 0;
    virtual void setFullscreen(Output *output) = 0;
    virtual void unsetFullscreen() = 0;
    virtual void setMinimized() = 0;

protected:
    Private(XdgShellSurface *q);

    XdgShellSurface *q;
};

class XdgShellSurfaceUnstableV5 : public XdgShellSurface
{
    Q_OBJECT
public:
    virtual ~XdgShellSurfaceUnstableV5();

private:
    explicit XdgShellSurfaceUnstableV5(QObject *parent = nullptr);
    friend class XdgShellUnstableV5;
    class Private;
};

class Q_DECL_HIDDEN XdgShellPopup::Private
{
public:
    Private(XdgShellPopup *q);
    virtual ~Private();

    EventQueue *queue = nullptr;

    virtual void setupV5(xdg_popup *p) {
        Q_UNUSED(p)
    }
    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() const = 0;
    virtual operator xdg_popup*() {
        return nullptr;
    }
    virtual operator xdg_popup*() const {
        return nullptr;
    }

protected:
    XdgShellPopup *q;

private:
};

class XdgShellPopupUnstableV5 : public XdgShellPopup
{
public:
    virtual ~XdgShellPopupUnstableV5();

private:
    explicit XdgShellPopupUnstableV5(QObject *parent = nullptr);
    friend class XdgShellUnstableV5;
    class Private;
};

}
}

#endif
