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
#ifndef WAYLAND_CONTROL_DATADEVICE_H
#define WAYLAND_CONTROL_DATADEVICE_H

#include "datacontroloffer.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zwlr_data_control_device_v1;

namespace KWayland
{
namespace Client
{
class DataControlSource;

/**
 * @short Wrapper for the zwlr_data_control_device_v1 interface.
 *
 * This class is a convenient wrapper for the zwlr_data_control_device_v1 interface.
 * To create a DataControlDevice call DataControlDeviceManager::getDataControlDevice.
 *
 * @see DataControlDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataControlDevice : public QObject
{
    Q_OBJECT
public:
    explicit DataControlDevice(QObject *parent = nullptr);
    virtual ~DataControlDevice();

    /**
     * Setup this DataControlDevice to manage the @p dataDevice.
     * When using DataControlDeviceManager::createDataControlDevice there is no need to call this
     * method.
     **/
    void setup(zwlr_data_control_device_v1 *dataDevice);
    /**
     * Releases the zwlr_data_control_device_v1 interface.
     * After the interface has been released the DataControlDevice instance is no
     * longer valid and can be setup with another zwlr_data_control_device_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataControlDevice.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwlr_data_control_device_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataControlDevice gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a zwlr_data_control_device_v1.
     **/
    bool isValid() const;

    void setSelection(DataControlSource *source = nullptr);
    void clearSelection();

    DataControlOffer *offeredSelection() const;

    operator zwlr_data_control_device_v1*();
    operator zwlr_data_control_device_v1*() const;

Q_SIGNALS:
    void selectionOffered(KWayland::Client::DataControlOffer*);
    void selectionCleared();
private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
