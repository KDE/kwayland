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

struct zxdg_shell_v6;
struct zxdg_toplevel_v6;
struct zxdg_surface_v6;
struct zxdg_popup_v6;
struct zxdg_position_v6;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Output;
class Surface;
class Seat;
class XdgShellPopup;
class XdgShellSurface;




class XdgPositioner
{
    enum class Constraint {
        SlideX = 1 << 0,
        SlideY = 1 << 1,
        FlipX = 1 << 2,
        FlipY = 1 << 3,
        ResizeX = 1 << 4,
        ResizeY = 1 << 5
    };

    Q_DECLARE_FLAGS(Constraints, Constraint)

public:
    XdgPositioner(const QSize &initialSize, const QRect &anchor);

    Qt::Edges anchorEdge() const;
    void setAnchorEdge(Qt::Edges edge);

    Qt::Edges gravity() const;
    void setGravity(Qt::Edges edge);

    QRect anchor() const;
    void setAnchor(const QRect &anchor);

    QSize initialSize() const;
    void setInitialSize(const QSize &size);

    Constraints constraints() const;
    void setConstraint(Constraints constaints);

    QPoint anchorOffset() const;
    void setAnchorOffset(const QPoint &offset);

private:
    class Private;
    QScopedPointer<Private> d; //Dave QSharedPointer?
};

/**
 * @short Wrapper for the xdg_shell interface.
 *
 * This class provides a convenient wrapper for the xdg_shell interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the XdgShell interface:
 * @code
 * XdgShell *c = registry->createXdgShell(name, version);
 * @endcode
 *
 * This creates the XdgShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * XdgShell *c = new XdgShell;
 * c->setup(registry->bindXdgShell(name, version));
 * @endcode
 *
 * The XdgShell can be used as a drop-in replacement for any xdg_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.25
 **/
class KWAYLANDCLIENT_EXPORT XdgShell : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgShell();

    /**
     * Setup this XdgShell to manage the @p xdgshellv5.
     * When using Registry::createXdgShell there is no need to call this
     * method.
     **/
    void setup(xdg_shell *xdgshellv5);

    void setup(zxdg_shell_v6 *xdgshellv6);

    /**
     * @returns @c true if managing a xdg_shell.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_shell interface.
     * After the interface has been released the XdgShell instance is no
     * longer valid and can be setup with another xdg_shell interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_shell interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgshellv5, &XdgShell::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this XdgShell.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this XdgShell.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a new XdgShellSurface for the given @p surface.
     **/
    XdgShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    /**
     * Creates a new XdgShellPopup for the given @p surface on top of @p parentSurface.
     **/
    XdgShellPopup *createPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent = nullptr);

    //DAVE - we need XdgV6ShellSurface for parent surface, it's easy to go get surface from shell, but not this way round. Suggest to deprecate this method, instead replace with these two - (could be one method if I do my template trickery)

    XdgShellPopup *createPopup(Surface *surface, XdgShellSurface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent = nullptr);
//     XdgShellPopup *createPopup(Surface *surface, XdgShellPopup *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent = nullptr);


    operator xdg_shell*();
    operator xdg_shell*() const;
    operator zxdg_shell_v6*();
    operator zxdg_shell_v6*() const;


Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the XdgShell got created by
     * Registry::createXdgShell
     **/
    void removed();

protected:
    /**
     * Creates a new XdgShell.
     * Note: after constructing the XdgShell it is not yet valid and one needs
     * to call setup. In order to get a ready to use XdgShell prefer using
     * Registry::createXdgShell.
     **/
    class Private;
    explicit XdgShell(Private *p, QObject *parent = nullptr);

private:
    QScopedPointer<Private> d;
};

/**
 *
 * @since 5.25
 **/
class KWAYLANDCLIENT_EXPORT XdgShellSurface : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgShellSurface();
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
     * Setup this XdgShellSurface to manage the @p xdgsurfacev5.
     * When using XdgShell::createXdgShellSurface there is no need to call this
     * method.
     **/
    void setup(xdg_surface *xdgsurfacev5);

    void setup(zxdg_surface_v6 *xdgsurfacev6, zxdg_toplevel_v6 *toplevel);

    /**
     * @returns @c true if managing a xdg_surface.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_surface interface.
     * After the interface has been released the XdgShellSurface instance is no
     * longer valid and can be setup with another xdg_surface interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_surface interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgsurfacev5, &XdgShellSurface::destroy);
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

    /**
     * The currently configured size.
     * @see sizeChanged
     * @see setSize
     **/
    QSize size() const;

    /**
     * Sets the size for the XdgShellSurface to @p size.
     * This is mostly an internal information. The actual size of the XdgShellSurface is
     * determined by the size of the Buffer attached to the XdgShellSurface's Surface.
     *
     * @param size The new size to be used for the XdgShellSurface
     * @see size
     * @see sizeChanged
     **/
    void setSize(const QSize &size);

    /**
     * Set this XdgShellSurface as transient for @p parent.
     **/
    void setTransientFor(XdgShellSurface *parent);

    /**
     * Sets the window title of this XdgShellSurface to @p title.
     **/
    void setTitle(const QString &title);

    /**
     * Set an application identifier for the surface.
     **/
    void setAppId(const QByteArray &appId);

    /**
     * Requests to show the window menu at @p pos in surface coordinates.
     **/
    void requestShowWindowMenu(Seat *seat, quint32 serial, const QPoint &pos);

    /**
     * Requests a move on the given @p seat after the pointer button press with the given @p serial.
     *
     * @param seat The seat on which to move the window
     * @param serial The serial of the pointer button press which should trigger the move
     **/
    void requestMove(Seat *seat, quint32 serial);

    /**
     * Requests a resize on the given @p seat after the pointer button press with the given @p serial.
     *
     * @param seat The seat on which to resize the window
     * @param serial The serial of the pointer button press which should trigger the resize
     * @param edges A hint for the compositor to set e.g. an appropriate cursor image
     **/
    void requestResize(Seat *seat, quint32 serial, Qt::Edges edges);

    /**
     * When a configure event is received, if a client commits the
     * Surface in response to the configure event, then the client
     * must make an ackConfigure request sometime before the commit
     * request, passing along the @p serial of the configure event.
     * @see configureRequested
     **/
    void ackConfigure(quint32 serial);

    /**
     * Request to set this XdgShellSurface to be maximized if @p set is @c true.
     * If @p set is @c false it requests to unset the maximized state - if set.
     *
     * @param set Whether the XdgShellSurface should be maximized
     **/
    void setMaximized(bool set);

    /**
     * Request to set this XdgShellSurface as fullscreen on @p output.
     * If @p set is @c true the Surface should be set to fullscreen, otherwise restore
     * from fullscreen state.
     *
     * @param set Whether the Surface should be fullscreen or not
     * @param output Optional output as hint to the compositor where the Surface should be put
     **/
    void setFullscreen(bool set, Output *output = nullptr);

    /**
     * Request to the compositor to minimize this XdgShellSurface.
     **/
    void requestMinimize();

    void setMaxSize(const QSize &size);
    void setMinSize(const QSize &size);

    operator xdg_surface*();
    operator xdg_surface*() const;

    operator zxdg_surface_v6*();
    operator zxdg_surface_v6*() const;
    operator zxdg_toplevel_v6*();
    operator zxdg_toplevel_v6*() const;

Q_SIGNALS:
    /**
     * The compositor requested to close this window.
     **/
    void closeRequested();
    /**
     * The compositor sent a configure with the new @p size and the @p states.
     * Before the next commit of the surface the @p serial needs to be passed to ackConfigure.
     **/
    void configureRequested(const QSize &size, KWayland::Client::XdgShellSurface::States states, quint32 serial);

    /**
     * Emitted whenever the size of the XdgShellSurface changes by e.g. receiving a configure request.
     *
     * @see configureRequested
     * @see size
     * @see setSize
     **/
    void sizeChanged(const QSize &);

protected:
    class Private;
    explicit XdgShellSurface(Private *p, QObject *parent = nullptr);

private:
    QScopedPointer<Private> d;
};

/**
 * A XdgShellPopup is a short-lived, temporary surface that can be
 * used to implement menus. It takes an explicit grab on the surface
 * that will be dismissed when the user dismisses the popup. This can
 * be done by the user clicking outside the surface, using the keyboard,
 * or even locking the screen through closing the lid or a timeout.
 * @since 5.25
 **/
class KWAYLANDCLIENT_EXPORT XdgShellPopup : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgShellPopup();

    /**
     * Setup this XdgShellPopup to manage the @p xdgpopupv5.
     * When using XdgShell::createXdgShellPopup there is no need to call this
     * method.
     **/
    void setup(xdg_popup *xdgpopupv5);

    /**
     * Setup this XdgShellPopup to manage the @p xdgpopupv6
     * When using XdgShell::createXdgShellPopup there is no need to call this
     * method.
     **/
    void setup(zxdg_surface_v6 *xdgsurfacev6, zxdg_popup_v6 *xdgpopup6);

    /**
     * @returns @c true if managing an xdg_popup.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_popup interface.
     * After the interface has been released the XdgShellPopup instance is no
     * longer valid and can be setup with another xdg_popup interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShellPopup.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_popup interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgpopupv5, &XdgShellPopup::destroy);
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
    operator zxdg_popup_v6*();
    operator zxdg_popup_v6*() const;


Q_SIGNALS:
    /**
     * This signal is emitted when a XdgShellPopup is dismissed by the
     * compositor. The user should delete this instance at this point.
     **/
    void popupDone();

    /**
     *
     **/
    void configureRequested(const QRect &relativePosition, quint32 serial);


protected:
    class Private;
    explicit XdgShellPopup(Private *p, QObject *parent = nullptr);

private:
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::XdgShellSurface::State)
Q_DECLARE_METATYPE(KWayland::Client::XdgShellSurface::States)

#endif
