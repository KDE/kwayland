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
#ifndef KWAYLAND_CLIENT_XDGFOREIGN_V1_H
#define KWAYLAND_CLIENT_XDGFOREIGN_V1_H

#include "surface.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zxdg_exporter_v1;
struct zxdg_importer_v1;
struct zxdg_exported_v1;
struct zxdg_imported_v1;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;

/**
 * @short Wrapper for the zxdg_exporter_v1 interface.
 *
 * This class provides a convenient wrapper for the zxdg_exporter_v1 interface.
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
 * The  can be used as a drop-in replacement for any zxdg_exporter_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT XdgExporterUnstableV1 : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new .
     * Note: after constructing the  it is not yet valid and one needs
     * to call setup. In order to get a ready to use  prefer using
     * Registry::create.
     **/
    explicit XdgExporterUnstableV1(QObject *parent = nullptr);
    virtual ~XdgExporterUnstableV1();

    /**
     * Setup this  to manage the @p .
     * When using Registry::create there is no need to call this
     * method.
     **/
    void setup(zxdg_exporter_v1 *);
    /**
     * @returns @c true if managing a zxdg_exporter_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_exporter_v1 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_exporter_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_exporter_v1 interface
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

    zxdg_exported_v1 *exportSurface(Surface *surface, QObject *parent = nullptr);

    operator zxdg_exporter_v1*();
    operator zxdg_exporter_v1*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the  got created by
     * Registry::create
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the zxdg_importer_v1 interface.
 *
 * This class provides a convenient wrapper for the zxdg_importer_v1 interface.
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
 * The  can be used as a drop-in replacement for any zxdg_importer_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT XdgImporterUnstableV1 : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new .
     * Note: after constructing the  it is not yet valid and one needs
     * to call setup. In order to get a ready to use  prefer using
     * Registry::create.
     **/
    explicit XdgImporterUnstableV1(QObject *parent = nullptr);
    virtual ~XdgImporterUnstableV1();

    /**
     * Setup this  to manage the @p .
     * When using Registry::create there is no need to call this
     * method.
     **/
    void setup(zxdg_importer_v1 *);
    /**
     * @returns @c true if managing a zxdg_importer_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_importer_v1 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_importer_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_importer_v1 interface
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

    zxdg_imported_v1 *import(const QString & handle, QObject *parent = nullptr);

    operator zxdg_importer_v1*();
    operator zxdg_importer_v1*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the  got created by
     * Registry::create
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgExportedUnstableV1 : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgExportedUnstableV1();

    /**
     * Setup this  to manage the @p .
     * When using ::create there is no need to call this
     * method.
     **/
    void setup(zxdg_exported_v1 *);
    /**
     * @returns @c true if managing a zxdg_exported_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_exported_v1 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_exported_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_exported_v1 interface
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

    operator zxdg_exported_v1*();
    operator zxdg_exported_v1*() const;

private:
    friend class XdgExporterUnstableV1;
    explicit XdgExportedUnstableV1(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgImportedUnstableV1 : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgImportedUnstableV1();

    /**
     * Setup this  to manage the @p .
     * When using ::create there is no need to call this
     * method.
     **/
    void setup(zxdg_imported_v1 *);
    /**
     * @returns @c true if managing a zxdg_imported_v1.
     **/
    bool isValid() const;
    /**
     * Releases the zxdg_imported_v1 interface.
     * After the interface has been released the  instance is no
     * longer valid and can be setup with another zxdg_imported_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this .
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zxdg_imported_v1 interface
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

    void setParentOf(Surface *surface);

    operator zxdg_imported_v1*();
    operator zxdg_imported_v1*() const;

private:
    friend class XdgImporterUnstableV1;
    explicit XdgImportedUnstableV1(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};


}
}

#endif
