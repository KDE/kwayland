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

#include <QSize>

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

    /**
     * @returns The XdgSurfaceV5Interface for the @p native resource.
     **/
    XdgSurfaceV5Interface *get(wl_resource *native);

Q_SIGNALS:
    void surfaceCreated(KWayland::Server::XdgSurfaceV5Interface *surface);

private:
    explicit XdgShellV5Interface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
    Private *d_func() const;
};

class KWAYLANDSERVER_EXPORT XdgSurfaceV5Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~XdgSurfaceV5Interface();

    /**
     * States the Surface can be in
     **/
    enum class State {
        /**
         * The Surface is maximized.
         **/
        Maximized  = 1 << 0,
        /**
         * The Surface is fullscreen.
         **/
        Fullscreen = 1 << 1,
        /**
         * The Surface is currently being resized by the Compositor.
         **/
        Resizing   = 1 << 2,
        /**
         * The Surface is considered active. Does not imply keyboard focus.
         **/
        Activated  = 1 << 3
    };
    Q_DECLARE_FLAGS(States, State)

    /**
     * Sends a configure event to the Surface.
     * This tells the Surface the current @p states it is in and the @p size it should have.
     * If @p size has with and height at @c 0, the Surface can choose the size.
     *
     * The Surface acknowledges the configure event with @link{configureAcknowledged}.
     *
     * @param states The states the surface is in
     * @param size The requested size
     * @returns The serial of the configure event
     * @see configureAcknowledged
     * @see isConfigurePending
     **/
    quint32 configure(States states, const QSize &size = QSize(0, 0));

    /**
     * @returns @c true if there is a not yet acknowledged configure event.
     * @see configure
     * @see configureAcknowledged
     **/
    bool isConfigurePending() const;

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
     * @returns Whether this Surface is a transient for another Surface, that is it has a parent.
     * @see transientFor
     **/
    bool isTransient() const;
    /**
     * @returns the parent surface if the surface is a transient for another surface
     * @see isTransient
     **/
    QPointer<XdgSurfaceV5Interface> transientFor() const;

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
    /**
     * A configure event with @p serial got acknowledged.
     * @see configure
     **/
    void configureAcknowledged(quint32 serial);
    /**
     * Emitted whenever the parent surface changes.
     * @see isTransient
     * @see transientFor
     **/
    void transientForChanged();

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
