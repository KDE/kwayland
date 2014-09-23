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
#ifndef WAYLAND_COMPOSITOR_H
#define WAYLAND_COMPOSITOR_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_compositor;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;

/**
 * @short Wrapper for the wl_compositor interface.
 *
 * This class provides a convenient wrapper for the wl_compositor interface.
 * It's main purpose is to create a Surface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Compositor interface:
 * @code
 * Compositor *c = registry->createCompositor(name, version);
 * @endcode
 *
 * This creates the Compositor and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * Compositor *c = new Compositor;
 * c->setup(registry->bindCompositor(name, version));
 * @endcode
 *
 * The Compositor can be used as a drop-in replacement for any wl_compositor
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT Compositor : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new Compositor.
     * Note: after constructing the Compositor it is not yet valid and one needs
     * to call setup. In order to get a ready to use Compositor prefer using
     * Registry::createCompositor.
     **/
    explicit Compositor(QObject *parent = nullptr);
    virtual ~Compositor();

    /**
     * @returns @c true if managing a wl_compositor.
     **/
    bool isValid() const;
    /**
     * Setup this Compositor to manage the @p compositor.
     * When using Registry::createCompositor there is no need to call this
     * method.
     **/
    void setup(wl_compositor *compositor);
    /**
     * Releases the wl_compositor interface.
     * After the interface has been released the Compositor instance is no
     * longer valid and can be setup with another wl_compositor interface.
     **/
    void release();
    /**
     * Destroys the data hold by this Compositor.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_compositor interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, compositor, &Compositor::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates and setup a new Surface with @p parent.
     * @param parent The parent to pass to the Surface.
     * @returns The new created Surface
     **/
    Surface *createSurface(QObject *parent = nullptr);

    operator wl_compositor*();
    operator wl_compositor*() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
