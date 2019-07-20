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
#include "datacontroloffer_interface_p.h"
#include "datacontroldevice_interface.h"
#include "datacontrolsource_interface.h"
// Qt
#include <QStringList>
// Wayland
#include <wayland-data-control-v1-server-protocol.h>
// system
#include <unistd.h>

namespace KWayland
{
namespace Server
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_offer_v1_interface DataControlOfferInterface::Private::s_interface = {
    receiveCallback,
    resourceDestroyedCallback,
};
#endif

DataControlOfferInterface::Private::Private(DataControlSourceInterface *source, DataControlDeviceInterface *parentInterface, DataControlOfferInterface *q, wl_resource *parentResource)
    : Resource::Private(q, nullptr, parentResource, &zwlr_data_control_offer_v1_interface, &s_interface)
    , source(source)
    , dataDevice(parentInterface)
{
    // TODO: connect to new selections
}

DataControlOfferInterface::Private::~Private() = default;

void DataControlOfferInterface::Private::receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd)
{
    Q_UNUSED(client)
    cast<Private>(resource)->receive(QString::fromUtf8(mimeType), fd);
}

void DataControlOfferInterface::Private::receive(const QString &mimeType, qint32 fd)
{
    if (!source) {
        close(fd);
        return;
    }
    source->requestData(mimeType, fd);
}

DataControlOfferInterface::DataControlOfferInterface(DataControlSourceInterface *source, DataControlDeviceInterface *parentInterface, wl_resource *parentResource)
    : Resource(new Private(source, parentInterface, this, parentResource))
{
    Q_ASSERT(source);
    connect(source, &DataControlSourceInterface::mimeTypeOffered, this,
        [this](const QString &mimeType) {
            Q_D();
            if (!d->resource) {
                return;
            }
            zwlr_data_control_offer_v1_send_offer(d->resource, mimeType.toUtf8().constData());
        }
    );
    QObject::connect(source, &QObject::destroyed, this,
        [this] {
            Q_D();
            d->source = nullptr;
        }
    );
}

DataControlOfferInterface::~DataControlOfferInterface() = default;

void DataControlOfferInterface::sendAllOffers()
{
    Q_D();
    for (const QString &mimeType : d->source->mimeTypes()) {
        zwlr_data_control_offer_v1_send_offer(d->resource, mimeType.toUtf8().constData());
    }
}

DataControlOfferInterface::Private *DataControlOfferInterface::d_func() const
{
    return reinterpret_cast<DataControlOfferInterface::Private*>(d.data());
}

}
}
