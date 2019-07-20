/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2019  David Edmundson <davidedmundson@kde.org>

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
#ifndef WAYLAND_CONTROL_DATA_DEVICE_MANAGER_H
#define WAYLAND_CONTROL_DATA_DEVICE_MANAGER_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zwlr_data_control_manager_v1;

namespace KWayland
{
namespace Client
{

class EventQueue;
class DataControlDevice;
class DataControlSource;
class Seat;

/**
 * @short Wrapper for the zwlr_data_control_manager_v1 interface.
 *
 * This class provides a convenient wrapper for the zwlr_data_control_manager_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the DataControlDeviceManager interface:
 * @code
 * DataControlDeviceManager *m = registry->createDataControlDeviceManager(name, version);
 * @endcode
 *
 * This creates the DataControlDeviceManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DataControlDeviceManager *m = new DataControlDeviceManager;
 * m->setup(registry->bindDataControlDeviceManager(name, version));
 * @endcode
 *
 * The DataControlDeviceManager can be used as a drop-in replacement for any zwlr_data_control_manager_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT DataControlDeviceManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new Compositor.
     * Note: after constructing the Compositor it is not yet valid and one needs
     * to call setup. In order to get a ready to use Compositor prefer using
     * Registry::createCompositor.
     **/
    explicit DataControlDeviceManager(QObject *parent = nullptr);
    virtual ~DataControlDeviceManager();

    /**
     * @returns @c true if managing a zwlr_data_control_manager_v1.
     **/
    bool isValid() const;
    /**
     * Setup this DataControlDeviceManager to manage the @p manager.
     * When using Registry::createDataControlDeviceManager there is no need to call this
     * method.
     **/
    void setup(zwlr_data_control_manager_v1 *manager);
    /**
     * Releases the zwlr_data_control_manager_v1 interface.
     * After the interface has been released the DataControlDeviceManager instance is no
     * longer valid and can be setup with another zwlr_data_control_manager_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataControlDeviceManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwlr_data_control_manager_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataControlDeviceManager gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a DataSource.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a DataSource.
     **/
    EventQueue *eventQueue();

    DataControlSource *createDataSource(QObject *parent = nullptr);

    DataControlDevice *getDataDevice(Seat *seat, QObject *parent = nullptr);

    operator zwlr_data_control_manager_v1*();
    operator zwlr_data_control_manager_v1*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createDataControlDeviceManager
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
