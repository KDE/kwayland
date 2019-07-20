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
#ifndef WAYLAND_CONTROL_DATASOURCE_H
#define WAYLAND_CONTROL_DATASOURCE_H

#include "buffer.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zwlr_data_control_source_v1;
class QMimeType;

namespace KWayland
{
namespace Client
{


/**
 * @short Wrapper for the wl_data_source interface.
 *
 * This class is a convenient wrapper for the wl_data_source interface.
 * To create a DataControlSource call DataControlDeviceManager::createDataControlSource.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataControlSource : public QObject
{
    Q_OBJECT
public:
    explicit DataControlSource(QObject *parent = nullptr);
    virtual ~DataControlSource();

    /**
     * Setup this DataControlSource to manage the @p dataSource.
     * When using DataDeviceManager::createDataControlSource there is no need to call this
     * method.
     **/
    void setup(zwlr_data_control_source_v1 *dataSource);
    /**
     * Releases the wl_data_source interface.
     * After the interface has been released the DataControlSource instance is no
     * longer valid and can be setup with another wl_data_source interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataControlSource.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_source interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataControlSource gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_source.
     **/
    bool isValid() const;

    void offer(const QString &mimeType);
    void offer(const QMimeType &mimeType);

    operator zwlr_data_control_source_v1*();
    operator zwlr_data_control_source_v1*() const;

Q_SIGNALS:
    /**
     * Request for data from the client. Send the data as the
     * specified @p mimeType over the passed file descriptor @p fd, then close
     * it.
     **/
    void sendDataRequested(const QString &mimeType, qint32 fd);
    /**
     * This DataControlSource has been replaced by another DataControlSource.
     * The client should clean up and destroy this DataControlSource.
     **/
    void cancelled();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
