/****************************************************************************
Copyright 2016  Sebastian Kügler <sebas@kde.org>

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
#ifndef KWAYLAND_CLIENT_WINDOWMETADATAMAP_H
#define KWAYLAND_CLIENT_WINDOWMETADATAMAP_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_windowmetadatamap;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;

/**
 * @short Wrapper for the org_kde_kwin_windowmetadatamap interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_windowmetadatamap interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the WindowMetadataMap interface:
 * @code
 * WindowMetadataMap *c = registry->createWindowMetadataMap(name, version);
 * @endcode
 *
 * This creates the WindowMetadataMap and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * WindowMetadataMap *c = new WindowMetadataMap;
 * c->setup(registry->bindWindowMetadataMap(name, version));
 * @endcode
 *
 * The WindowMetadataMap can be used as a drop-in replacement for any org_kde_kwin_windowmetadatamap
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT WindowMetadataMap : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new WindowMetadataMap.
     * Note: after constructing the WindowMetadataMap it is not yet valid and one needs
     * to call setup. In order to get a ready to use WindowMetadataMap prefer using
     * Registry::createWindowMetadataMap.
     **/
    explicit WindowMetadataMap(QObject *parent = nullptr);
    virtual ~WindowMetadataMap();

    /**
     * Setup this WindowMetadataMap to manage the @p windowmetadatamap.
     * When using Registry::createWindowMetadataMap there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_windowmetadatamap *windowmetadatamap);
    /**
     * @returns @c true if managing a org_kde_kwin_windowmetadatamap.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_windowmetadatamap interface.
     * After the interface has been released the WindowMetadataMap instance is no
     * longer valid and can be setup with another org_kde_kwin_windowmetadatamap interface.
     **/
    void release();
    /**
     * Destroys the data held by this WindowMetadataMap.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_windowmetadatamap interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, windowmetadatamap, &WindowMetadataMap::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this WindowMetadataMap.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this WindowMetadataMap.
     **/
    EventQueue *eventQueue();

    void registerClient(const QString & serviceName, Surface *surface);

    operator org_kde_kwin_windowmetadatamap*();
    operator org_kde_kwin_windowmetadatamap*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the WindowMetadataMap got created by
     * Registry::createWindowMetadataMap
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};


}
}

#endif
