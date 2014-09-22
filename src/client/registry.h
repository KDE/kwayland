/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef WAYLAND_REGISTRY_H
#define WAYLAND_REGISTRY_H

#include <QHash>
#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_compositor;
struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_seat;
struct wl_shell;
struct wl_shm;
struct _wl_fullscreen_shell;

namespace KWayland
{
namespace Client
{

class Compositor;
class ConnectionThread;
class FullscreenShell;
class Output;
class Seat;
class Shell;
class ShmPool;

/**
 * @short Wrapper for the wl_registry interface.
 *
 * The purpose of this class is to manage the wl_registry interface.
 * This class supports some well-known interfaces and can create a
 * wrapper class for those.
 *
 * The main purpose is to emit signals whenever a new interface is
 * added or an existing interface is removed. For the well known interfaces
 * dedicated signals are emitted allowing a user to connect directly to the
 * signal announcing the interface it is interested in.
 *
 * To create and setup the Registry one needs to call create with either a
 * wl_display from an existing Wayland connection or a ConnectionThread instance:
 *
 * @code
 * ConnectionThread *connection; // existing connection
 * Registry registry;
 * registry.create(connection);
 * registry.setup();
 * @endcode
 *
 * The interfaces are announced in an asynchronous way by the Wayland server.
 * To initiate the announcing of the interfaces one needs to call setup.
 **/
class KWAYLANDCLIENT_EXPORT Registry : public QObject
{
    Q_OBJECT
public:
    /**
     * The well-known interfaces this Registry supports.
     * For each of the enum values the Registry is able to create a Wrapper
     * object.
     **/
    enum class Interface {
        Unknown,    ///< Refers to an Unknown interface
        Compositor, ///< Refers to the wl_compositor interface
        Shell,      ///< Refers to the wl_shell interface
        Seat,       ///< Refers to the wl_seat interface
        Shm,        ///< Refers to the wl_shm interface
        Output,     ///< Refers to the wl_output interface
        FullscreenShell ///< Refers to the _wl_fullscreen_shell interface
    };
    explicit Registry(QObject *parent = nullptr);
    virtual ~Registry();

    /**
     * Releases the wl_registry interface.
     * After the interface has been released the Registry instance is no
     * longer valid and can be setup with another wl_registry interface.
     **/
    void release();
    /**
     * Destroys the data hold by this Registry.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_registry interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, registry, &Registry::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Gets the registry from the @p display.
     **/
    void create(wl_display *display);
    /**
     * Gets the registry from the @p connection.
     **/
    void create(ConnectionThread *connection);
    /**
     * Finalizes the setup of the Registry.
     * After calling this method the interfaces will be announced in an asynchronous way.
     * The Registry must have been created when calling this method.
     * @see create
     **/
    void setup();

    /**
     * @returns @c true if managing a wl_registry.
     **/
    bool isValid() const;
    /**
     * @returns @c true if the Registry has an @p interface.
     **/
    bool hasInterface(Interface interface) const;

    /**
     * Binds the wl_compositor with @p name and @p version.
     * If the @p name does not exist or is not for the compositor interface,
     * @c null will be returned.
     *
     * Prefer using createCompositor instead.
     * @see createCompositor
     **/
    wl_compositor *bindCompositor(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_shell with @p name and @p version.
     * If the @p name does not exist or is not for the shell interface,
     * @c null will be returned.
     *
     * Prefer using createShell instead.
     * @see createShell
     **/
    wl_shell *bindShell(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_seat with @p name and @p version.
     * If the @p name does not exist or is not for the seat interface,
     * @c null will be returned.
     *
     * Prefer using createSeat instead.
     * @see createSeat
     **/
    wl_seat *bindSeat(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_shm with @p name and @p version.
     * If the @p name does not exist or is not for the shm interface,
     * @c null will be returned.
     *
     * Prefer using createShmPool instead.
     * @see createShmPool
     **/
    wl_shm *bindShm(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_output with @p name and @p version.
     * If the @p name does not exist or is not for the output interface,
     * @c null will be returned.
     *
     * Prefer using createOutput instead.
     * @see createOutput
     **/
    wl_output *bindOutput(uint32_t name, uint32_t version) const;
    /**
     * Binds the _wl_fullscreen_shell with @p name and @p version.
     * If the @p name does not exist or is not for the fullscreen shell interface,
     * @c null will be returned.
     *
     * Prefer using createFullscreenShell instead.
     * @see createFullscreenShell
     **/
    _wl_fullscreen_shell *bindFullscreenShell(uint32_t name, uint32_t version) const;

    /**
     * Creates a Compositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_compositor interface,
     * the returned Compositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_compositor interface to bind
     * @param version The version or the wl_compositor interface to use
     * @param parent The parent for Compositor
     *
     * @returns The created Compositor.
     **/
    Compositor *createCompositor(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a Seat and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_seat interface,
     * the returned Seat will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_seat interface to bind
     * @param version The version or the wl_seat interface to use
     * @param parent The parent for Seat
     *
     * @returns The created Seat.
     **/
    Shell *createShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a Compositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_compositor interface,
     * the returned Compositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_compositor interface to bind
     * @param version The version or the wl_compositor interface to use
     * @param parent The parent for Compositor
     *
     * @returns The created Compositor.
     **/
    Seat *createSeat(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ShmPool and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_shm interface,
     * the returned ShmPool will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_shm interface to bind
     * @param version The version or the wl_shm interface to use
     * @param parent The parent for ShmPool
     *
     * @returns The created ShmPool.
     **/
    ShmPool *createShmPool(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an Output and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_output interface,
     * the returned Output will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_output interface to bind
     * @param version The version or the wl_output interface to use
     * @param parent The parent for Output
     *
     * @returns The created Output.
     **/
    Output *createOutput(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a FullscreenShell and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the _wl_fullscreen_shell interface,
     * the returned FullscreenShell will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the _wl_fullscreen_shell interface to bind
     * @param version The version or the _wl_fullscreen_shell interface to use
     * @param parent The parent for FullscreenShell
     *
     * @returns The created FullscreenShell.
     **/
    FullscreenShell *createFullscreenShell(quint32 name, quint32 version, QObject *parent = nullptr);

    operator wl_registry*();
    operator wl_registry*() const;
    wl_registry *registry();

Q_SIGNALS:
    /**
     * Emitted whenever a wl_compositor interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void compositorAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void shellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_seat interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void seatAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_shm interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void shmAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_output interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void outputAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a _wl_fullscreen_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void fullscreenShellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_compositor interface gets removed.
     * @param name The name for the removed interface
     **/
    void compositorRemoved(quint32 name);
    /**
     * Emitted whenever a wl_shell interface gets removed.
     * @param name The name for the removed interface
     **/
    void shellRemoved(quint32 name);
    /**
     * Emitted whenever a wl_seat interface gets removed.
     * @param name The name for the removed interface
     **/
    void seatRemoved(quint32 name);
    /**
     * Emitted whenever a wl_shm interface gets removed.
     * @param name The name for the removed interface
     **/
    void shmRemoved(quint32 name);
    /**
     * Emitted whenever a wl_output interface gets removed.
     * @param name The name for the removed interface
     **/
    void outputRemoved(quint32 name);
    /**
     * Emitted whenever a _wl_fullscreen_shell interface gets removed.
     * @param name The name for the removed interface
     **/
    void fullscreenShellRemoved(quint32 name);
    /**
     * Generic announced signal which gets emitted whenever an interface gets
     * announced.
     *
     * This signal is emitted before the dedicated signals are handled. If one
     * wants to know about one of the well-known interfaces use the dedicated
     * signals instead. Especially the bind methods might fail before the dedicated
     * signals are emitted.
     *
     * @param interface The interface (e.g. wl_compositor) which is announced
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void interfaceAnnounced(QByteArray interface, quint32 name, quint32 version);
    /**
     * Generic removal signal which gets emitted whenever an interface gets removed.
     *
     * This signal is emitted after the dedicated signals are handled.
     *
     * @param name The name for the removed interface
     **/
    void interfaceRemoved(quint32 name);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
