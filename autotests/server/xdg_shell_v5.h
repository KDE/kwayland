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
#ifndef KWAYLAND_CLIENT_XDG_SHELL_V5_H
#define KWAYLAND_CLIENT_XDG_SHELL_V5_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct xdg_shell;
struct xdg_surface;
struct xdg_popup;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Output;
class Surface;
class Seat;
class XdgPopupV5;
class XdgSurfaceV5;

/**
 * @short Wrapper for the xdg_shell interface.
 *
 * This class provides a convenient wrapper for the xdg_shell interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the XdgShellV5 interface:
 * @code
 * XdgShellV5 *c = registry->createXdgShellV5(name, version);
 * @endcode
 *
 * This creates the XdgShellV5 and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * XdgShellV5 *c = new XdgShellV5;
 * c->setup(registry->bindXdgShellV5(name, version));
 * @endcode
 *
 * The XdgShellV5 can be used as a drop-in replacement for any xdg_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT XdgShellV5 : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new XdgShellV5.
     * Note: after constructing the XdgShellV5 it is not yet valid and one needs
     * to call setup. In order to get a ready to use XdgShellV5 prefer using
     * Registry::createXdgShellV5.
     **/
    explicit XdgShellV5(QObject *parent = nullptr);
    virtual ~XdgShellV5();

    /**
     * Setup this XdgShellV5 to manage the @p xdgshellv5.
     * When using Registry::createXdgShellV5 there is no need to call this
     * method.
     **/
    void setup(xdg_shell *xdgshellv5);
    /**
     * @returns @c true if managing a xdg_shell.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_shell interface.
     * After the interface has been released the XdgShellV5 instance is no
     * longer valid and can be setup with another xdg_shell interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShellV5.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_shell interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgshellv5, &XdgShellV5::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this XdgShellV5.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this XdgShellV5.
     **/
    EventQueue *eventQueue();

    void useUnstableVersion(qint32 version);

    XdgSurfaceV5 *getXdgSurface(Surface *surface, QObject *parent = nullptr);

    XdgPopupV5 *getXdgPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent = nullptr);

    void pong(quint32 serial);

    operator xdg_shell*();
    operator xdg_shell*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the XdgShellV5 got created by
     * Registry::createXdgShellV5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgSurfaceV5 : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgSurfaceV5();
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
     * Setup this XdgSurfaceV5 to manage the @p xdgsurfacev5.
     * When using XdgShellV5::createXdgSurfaceV5 there is no need to call this
     * method.
     **/
    void setup(xdg_surface *xdgsurfacev5);
    /**
     * @returns @c true if managing a xdg_surface.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_surface interface.
     * After the interface has been released the XdgSurfaceV5 instance is no
     * longer valid and can be setup with another xdg_surface interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgSurfaceV5.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_surface interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgsurfacev5, &XdgSurfaceV5::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue();

    void setTransientFor(XdgSurfaceV5 *parent);

    void setTitle(const QString & title);

    void setAppId(const QByteArray & appId);

    void showWindowMenu(Seat *seat, quint32 serial, qint32 x, qint32 y);

    void move(Seat *seat, quint32 serial);

    void resize(Seat *seat, quint32 serial, quint32 edges);

    void ackConfigure(quint32 serial);

    void setWindowGeometry(qint32 x, qint32 y, qint32 width, qint32 height);

    void setMaximized();

    void unsetMaximized();

    void setFullscreen(Output *output);

    void unsetFullscreen();

    void setMinimized();

    operator xdg_surface*();
    operator xdg_surface*() const;

Q_SIGNALS:
    void closeRequested();
    void configureRequested(const QSize &size, KWayland::Client::XdgSurfaceV5::States states, quint32 serial);

private:
    friend class XdgShellV5;
    explicit XdgSurfaceV5(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgPopupV5 : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgPopupV5();

    /**
     * Setup this XdgPopupV5 to manage the @p xdgpopupv5.
     * When using XdgShellV5::createXdgPopupV5 there is no need to call this
     * method.
     **/
    void setup(xdg_popup *xdgpopupv5);
    /**
     * @returns @c true if managing a xdg_popup.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_popup interface.
     * After the interface has been released the XdgPopupV5 instance is no
     * longer valid and can be setup with another xdg_popup interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgPopupV5.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_popup interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgpopupv5, &XdgPopupV5::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue();

    operator xdg_popup*();
    operator xdg_popup*() const;

Q_SIGNALS:
    void popupDone();

private:
    friend class XdgShellV5;
    explicit XdgPopupV5(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};


}
}

Q_DECLARE_METATYPE(KWayland::Client::XdgSurfaceV5::State)
Q_DECLARE_METATYPE(KWayland::Client::XdgSurfaceV5::States)

#endif
