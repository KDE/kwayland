/****************************************************************************
Copyright 2017  Marco Martin <notmart@gmail.com>

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
#ifndef KWAYLAND_CLIENT_XDGFOREIGN_H
#define KWAYLAND_CLIENT_XDGFOREIGN_H

#include "surface.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zxdg_exporter_v2;
struct zxdg_importer_v2;
struct zxdg_exported_v2;
struct zxdg_imported_v2;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class XdgExported;
class XdgImported;

/**
 * @short Wrapper for the zxdg_exporter_v2 interface.
 *
 * This class provides a convenient wrapper for the zxdg_exporter_v2 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the  interface:
 * @code
 *  *c = registry->create(name, version);
 * @endcode
 *
 * This creates the  and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 *  *c = new ;
 * c->setup(registry->bind(name, version));
 * @endcode
 *
 * The  can be used as a drop-in replacement for any zxdg_exporter_v2
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT XdgExporter : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgExporter();

    /**
     * Setup this  to manage the @p .
     * When using Registry::create there is no need to call this
     * method.
     **/
    void setup(zxdg_exporter_v2 *);
    /**
     * @returns @c true if managing a zxdg_exporter_v2.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_exporter_v2 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_exporter_v2 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_exporter_v2 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, , &::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this .
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this .
     **/
    EventQueue *eventQueue();

    /**
     * The export request exports the passed surface so that it can later be
     * imported via XdgImporter::importTopLevel.
     * A surface may be exported multiple times, and each exported handle may
     * be used to create an XdgImported multiple times.
     * @param surface the surface which we want to export an handle.
     * @param parent the parent in the QObject's hierarchy of the new XdgExported
     */
    XdgExported *exportTopLevel(Surface *surface, QObject *parent = nullptr);

    operator zxdg_exporter_v2*();
    operator zxdg_exporter_v2*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the  got created by
     * Registry::create
     **/
    void removed();

protected:
    class Private;
    explicit XdgExporter(Private *p, QObject *parent = nullptr);
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the zxdg_importer_v2 interface.
 *
 * This class provides a convenient wrapper for the zxdg_importer_v2 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the  interface:
 * @code
 *  *c = registry->create(name, version);
 * @endcode
 *
 * This creates the  and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 *  *c = new ;
 * c->setup(registry->bind(name, version));
 * @endcode
 *
 * The  can be used as a drop-in replacement for any zxdg_importer_v2
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT XdgImporter : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgImporter();

    /**
     * Setup this  to manage the @p .
     * When using Registry::create there is no need to call this
     * method.
     **/
    void setup(zxdg_importer_v2 *);
    /**
     * @returns @c true if managing a zxdg_importer_v2.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_importer_v2 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_importer_v2 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_importer_v2 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, , &::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this .
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this .
     **/
    EventQueue *eventQueue();

    /**
     * Imports a surface from any client given a handle
     * retrieved by exporting said surface using XdgExporter::exportTopLevel.
     * When called, a new XdgImported object will be created.
     * This new object represents the imported surface, and the importing
     * client can manipulate its relationship using it.
     *
     * @param handle the unique handle that represent an exported toplevel surface.
     *               it has to have been generated by the XdgExporter by either this
     *               or some other process (which would have communicated the handle
     *               in some way, such as command line or a DBus call)
     * @param parent the parent in the QObject's hierarchy of the new XdgImported
     */
    XdgImported *importTopLevel(const QString & handle, QObject *parent = nullptr);

    operator zxdg_importer_v2*();
    operator zxdg_importer_v2*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the  got created by
     * Registry::create
     **/
    void removed();

protected:
    class Private;
    explicit XdgImporter(Private *p, QObject *parent = nullptr);
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgExported : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgExported();

    /**
     * Setup this  to manage the @p .
     * When using ::create there is no need to call this
     * method.
     **/
    void setup(zxdg_exported_v2 *);
    /**
     * @returns @c true if managing a zxdg_exported_v2.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_exported_v2 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_exported_v2 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_exported_v2 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, , &::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * @returns The unique handle corresponding tho this exported surface.
     * Any process can import this toplevel surface provided they know this
     * unique string.
     */
    QString handle() const;

    operator zxdg_exported_v2*();
    operator zxdg_exported_v2*() const;

Q_SIGNALS:
    /**
     * Emitted when the exported window is fully initialized.
     * the handle will be valid at this point
     **/
    void done();

protected:
    friend class XdgExporter;
    class Private;
    explicit XdgExported(Private *p, QObject *parent = nullptr);
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgImported : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgImported();

    /**
     * Setup this  to manage the @p .
     * When using ::create there is no need to call this
     * method.
     **/
    void setup(zxdg_imported_v2 *);
    /**
     * @returns @c true if managing a zxdg_imported_v2.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_imported_v2 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_imported_v2 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_imported_v2 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, , &::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Set the imported surface as the parent of some surface of the client.
     * The passed surface must be a toplevel xdg_surface.
     * Calling this function sets up a surface to surface relation with the same
     * stacking and positioning semantics as XdgShellSurface::setTransientFor
     *
     * @param surface the child surface, which must belong to this process.
     */
    void setParentOf(Surface *surface);

    operator zxdg_imported_v2*();
    operator zxdg_imported_v2*() const;

Q_SIGNALS:
    /**
     * Emitted when the imported surface is not valid anymore,
     * for instance because it's no longer exported on the other end
     */
    void importedDestroyed();

protected:
    friend class XdgImporter;
    class Private;
    explicit XdgImported(Private *p, QObject *parent = nullptr);
    QScopedPointer<Private> d;
};


}
}

#endif
