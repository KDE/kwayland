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
#ifndef KWAYLAND_SERVER_XDG_SHELL_V5_H
#define KWAYLAND_SERVER_XDG_SHELL_V5_H

#include "global.h"
#include "resource.h"

#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{

class Display;
class OutputInterface;
class SeatInterface;
class SurfaceInterface;
class XdgSurfaceV5Interface;
template <typename T>
class GenericShellSurface;

class KWAYLANDSERVER_EXPORT XdgShellV5Interface : public Global
{
    Q_OBJECT
public:
    virtual ~XdgShellV5Interface();

Q_SIGNALS:
    void surfaceCreated(KWayland::Server::XdgSurfaceV5Interface *surface);

private:
    explicit XdgShellV5Interface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

class KWAYLANDSERVER_EXPORT XdgSurfaceV5Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~XdgSurfaceV5Interface();

    /**
     * @return The SurfaceInterface this XdgSurfaceV5Interface got created for.
     **/
    SurfaceInterface *surface() const;

    /**
     * @returns The title of this surface.
     * @see titleChanged
     **/
    QString title() const;
    QByteArray windowClass() const;

    /**
     * Request the client to close the window.
     **/
    void close();

Q_SIGNALS:
    /**
     * Emitted whenever the title changes.
     *
     * @see title
     **/
    void titleChanged(const QString&);
    /**
     * Emitted whenever the window class changes.
     **/
    void windowClassChanged(const QByteArray&);
    /**
     * The surface requested a window move.
     *
     * @param seat The SeatInterface on which the surface requested the move
     * @param serial The serial of the implicit mouse grab which triggered the move
     **/
    void moveRequested(KWayland::Server::SeatInterface *seat, quint32 serial);
    /**
     * The surface requested a window resize.
     *
     * @param seat The SeatInterface on which the surface requested the resize
     * @param serial The serial of the implicit mouse grab which triggered the resize
     * @param edges A hint which edges are involved in the resize
     **/
    void resizeRequested(KWayland::Server::SeatInterface *seat, quint32 serial, Qt::Edges edges);
    void windowMenuRequested(KWayland::Server::SeatInterface *seat, quint32 serial, const QPoint &surfacePos);
    /**
     * The surface requested a change of maximized state.
     * @param maximized whether the window wants to be maximized
     **/
    void maximizedChanged(bool maximized);
    /**
     * The surface requested a change of fullscreen state
     * @param fullscreen whether the window wants to be fullscreen
     * @param output An optional output hint on which the window wants to be fullscreen
     **/
    void fullscreenChanged(bool fullscreen, KWayland::Server::OutputInterface *output);
    /**
     * The surface requested to be minimized.
     **/
    void minimizeRequested();

private:
    explicit XdgSurfaceV5Interface(XdgShellV5Interface *parent, SurfaceInterface *surface, wl_resource *parentResource);
    friend class XdgShellV5Interface;
    friend class GenericShellSurface<XdgSurfaceV5Interface>;

    class Private;
    Private *d_func() const;
};

class KWAYLANDSERVER_EXPORT XdgPopupV5Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~XdgPopupV5Interface();

private:
    explicit XdgPopupV5Interface(XdgShellV5Interface *parent, wl_resource *parentResource);
    friend class XdgShellV5Interface;

    class Private;
    Private *d_func() const;
};


}
}

Q_DECLARE_METATYPE(KWayland::Server::XdgSurfaceV5Interface *)

#endif
