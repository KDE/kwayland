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
#ifndef WAYLAND_SHELL_H
#define WAYLAND_SHELL_H

#include <QObject>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_surface;
struct wl_shell;
struct wl_shell_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class ShellSurface;
class Output;
class Surface;

/**
 * @short Wrapper for the wl_shell interface.
 *
 * This class provides a convenient wrapper for the wl_shell interface.
 * It's main purpose is to create a ShellSurface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Shell interface:
 * @code
 * Shell *s = registry->createShell(name, version);
 * @endcode
 *
 * This creates the Shell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * Shell *s = new Shell;
 * s->setup(registry->bindShell(name, version));
 * @endcode
 *
 * The Shell can be used as a drop-in replacement for any wl_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see ShellSurface
 **/
class KWAYLANDCLIENT_EXPORT Shell : public QObject
{
    Q_OBJECT
public:
    explicit Shell(QObject *parent = nullptr);
    virtual ~Shell();

    /**
     * @returns @c true if managing a wl_shell.
     **/
    bool isValid() const;
    /**
     * Releases the wl_shell interface.
     * After the interface has been released the Shell instance is no
     * longer valid and can be setup with another wl_shell interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this Shell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. Once the connection becomes invalid, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_shell interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, shell, &Shell::destroyed);
     * @endcode
     *
     * Right before the data is destroyed, the signal interfaceAboutToBeDestroyed is emitted.
     *
     * @see release
     * @see interfaceAboutToBeDestroyed
     **/
    void destroy();
    /**
     * Setup this Shell to manage the @p shell.
     * When using Registry::createShell there is no need to call this
     * method.
     **/
    void setup(wl_shell *shell);

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a ShellSurface for the given @p surface and sets it up.
     *
     * @param surface The native surface to create the ShellSurface for
     * @param parent The parent to use for the ShellSurface
     * @returns created ShellSurface
     **/
    ShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a ShellSurface for the given @p surface and sets it up.
     *
     * @param surface The Surface to create the ShellSurface for
     * @param parent The parent to use for the ShellSurface
     * @returns created ShellSurface
     **/
    ShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    operator wl_shell*();
    operator wl_shell*() const;

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the wl_shell_surface interface.
 *
 * This class is a convenient wrapper for the wl_shell_surface interface.
 *
 * To create an instance use Shell::createSurface.
 *
 * @see Shell
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT ShellSurface : public QObject
{
    Q_OBJECT
    /**
     * The size of the ShellSurface.
     **/
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
public:
    explicit ShellSurface(QObject *parent);
    virtual ~ShellSurface();

    /**
     * Releases the wl_shell_surface interface.
     * After the interface has been released the ShellSurface instance is no
     * longer valid and can be setup with another wl_shell_surface interface.
     *
     * This method is automatically invoked when the Shell which created this
     * ShellSurface gets released.
     **/
    void release();
    /**
     * Destroys the data hold by this ShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_shell_surface interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Shell which created this
     * ShellSurface gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this ShellSurface to manage the @p surface.
     * There is normally no need to call this method as it's invoked by
     * Shell::createSurface.
     **/
    void setup(wl_shell_surface *surface);
    QSize size() const;
    void setSize(const QSize &size);

    /**
     * Sets the ShellSurface fullscreen on @p output.
     **/
    void setFullscreen(Output *output = nullptr);
    void setMaximized(Output *output = nullptr);
    void setToplevel();

    bool isValid() const;
    operator wl_shell_surface*();
    operator wl_shell_surface*() const;

Q_SIGNALS:
    /**
     * Signal is emitted when the ShellSurface received a ping request.
     * The ShellSurface automatically responds to the ping.
     **/
    void pinged();
    void sizeChanged(const QSize &);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
