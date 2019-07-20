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
#ifndef WAYLAND_CONTROL_DATAOFFER_H
#define WAYLAND_CONTROL_DATAOFFER_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

#include "datacontroldevicemanager.h"

struct zwlr_data_control_offer_v1;

class QMimeType;

namespace KWayland
{
namespace Client
{
class DataControlDevice;

/**
 * @short Wrapper for the zwlr_data_control_offer_v1 interface.
 *
 * This class is a convenient wrapper for the zwlr_data_control_offer_v1 interface.
 * The DataControlOffer gets created by DataControlDevice.
 *
 * @see DataControlOfferManager
 **/
class KWAYLANDCLIENT_EXPORT DataControlOffer : public QObject
{
    Q_OBJECT
public:
    virtual ~DataControlOffer();

    /**
     * Releases the zwlr_data_control_offer_v1 interface.
     * After the interface has been released the DataControlOffer instance is no
     * longer valid and can be setup with another zwlr_data_control_offer_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataControlOffer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwlr_data_control_offer_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataControlOffer gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a zwlr_data_control_offer_v1.
     **/
    bool isValid() const;

    QList<QMimeType> offeredMimeTypes() const;

    void receive(const QMimeType &mimeType, qint32 fd);
    void receive(const QString &mimeType, qint32 fd);

    operator zwlr_data_control_offer_v1*();
    operator zwlr_data_control_offer_v1*() const;

Q_SIGNALS:
    void mimeTypeOffered(const QString&);

private:
    friend class DataControlDevice;
    explicit DataControlOffer(DataControlDevice *parent, zwlr_data_control_offer_v1 *dataOffer);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::DataControlOffer*)

#endif
