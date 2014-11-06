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
#ifndef WAYLAND_DATADEVICE_H
#define WAYLAND_DATADEVICE_H

#include "buffer.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_data_device;

namespace KWayland
{
namespace Client
{
class DataSource;
class Surface;

/**
 * @short Wrapper for the wl_data_device interface.
 *
 * This class is a convenient wrapper for the wl_data_device interface.
 * To create a DataDevice call DataDeviceManager::getDataDevice.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataDevice : public QObject
{
    Q_OBJECT
public:
    explicit DataDevice(QObject *parent = nullptr);
    virtual ~DataDevice();

    /**
     * Setup this DataDevice to manage the @p dataDevice.
     * When using DataDeviceManager::createDataDevice there is no need to call this
     * method.
     **/
    void setup(wl_data_device *dataDevice);
    /**
     * Releases the wl_data_device interface.
     * After the interface has been released the DataDevice instance is no
     * longer valid and can be setup with another wl_data_device interface.
     **/
    void release();
    /**
     * Destroys the data hold by this DataDevice.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_data_device interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, source, &DataDevice::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_device.
     **/
    bool isValid() const;

    void startDrag(quint32 serial, DataSource *source, Surface *origin, Surface *icon = nullptr);
    void startDragInternally(quint32 serial, Surface *origin, Surface *icon = nullptr);

    void setSelection(quint32 serial, DataSource *source = nullptr);
    void clearSelection(quint32 serial);

    operator wl_data_device*();
    operator wl_data_device*() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
