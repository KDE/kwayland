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

#include <memory>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_compositor;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Region;
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
     * Creates a Compositor for the used QGuiApplication.
     * This is an integration feature for QtWayland. On non-wayland platforms this method returns
     * @c nullptr.
     *
     * The returned Compositor will be fully setup, which means it manages a wl_compositor.
     * When the created Compositor gets destroyed the managed wl_compositor won't be disconnected
     * as that's managed by Qt.
     * @since 5.4
     **/
    static Compositor *fromApplication(QObject *parent = nullptr);

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
     * Destroys the data held by this Compositor.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_compositor interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, compositor, &Compositor::destroy);
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

    /**
     * Creates and setup a new Region with @p parent.
     * @param parent The parent to pass to the Region.
     * @returns The new created Region
     **/
    Region *createRegion(QObject *parent = nullptr);
    /**
     * Creates and setup a new Region with @p parent.
     *
     * The @p region is directly added to the created Region.
     * @param parent The parent to pass to the Region.
     * @param region The region to install on the newly created Region
     * @returns The new created Region
     **/
    Region *createRegion(const QRegion &region, QObject *parent);
    /**
     * Creates and setup a new Region with @p region installed.
     *
     * This overloaded convenience method is intended to be used in the
     * case that the Region is only needed to setup e.g. input region on
     * a Surface and is afterwards no longer needed. Setting the input
     * region has copy semantics and the Region can be destroyed afterwards.
     * This allows to simplify setting the input region to:
     *
     * @code
     * Surface *s = compositor->createSurface();
     * s->setInputRegion(compositor->createRegion(QRegion(0, 0, 10, 10)).get());
     * @endcode
     *
     * @param region The region to install on the newly created Region
     * @returns The new created Region
     **/
    std::unique_ptr<Region> createRegion(const QRegion &region);

    operator wl_compositor*();
    operator wl_compositor*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createCompositor
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
