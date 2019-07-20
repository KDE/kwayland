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
#include "datacontroldevice_interface.h"
#include "datacontroldevicemanager_interface.h"
#include "datacontroloffer_interface_p.h"
#include "datacontrolsource_interface.h"
#include "display.h"
#include "resource_p.h"
#include "pointer_interface.h"
#include "seat_interface.h"
#include "surface_interface.h"
// Wayland
#include <wayland-data-control-v1-server-protocol.h>

namespace KWayland
{
namespace Server
{

class DataControlDeviceInterface::Private : public Resource::Private
{
public:
    Private(SeatInterface *seat, DataControlDeviceInterface *q, DataControlDeviceManagerInterface *manager, wl_resource *parentResource);
    ~Private();

    DataControlOfferInterface *createDataOffer(DataControlSourceInterface *source);

    SeatInterface *seat;
    DataControlSourceInterface *source = nullptr;

    DataControlSourceInterface *selection = nullptr;
    QMetaObject::Connection selectionUnboundConnection;
    QMetaObject::Connection selectionDestroyedConnection;

private:
    DataControlDeviceInterface *q_func() {
        return reinterpret_cast<DataControlDeviceInterface*>(q);
    }
    void setSelection(DataControlSourceInterface *dataSource);
    static void setSelectionCallback(wl_client *client, wl_resource *resource, wl_resource *source);

    static const struct zwlr_data_control_device_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_device_v1_interface DataControlDeviceInterface::Private::s_interface = {
    setSelectionCallback,
    resourceDestroyedCallback
};
#endif

DataControlDeviceInterface::Private::Private(SeatInterface *seat, DataControlDeviceInterface *q, DataControlDeviceManagerInterface *manager, wl_resource *parentResource)
    : Resource::Private(q, manager, parentResource, &zwlr_data_control_device_v1_interface, &s_interface)
    , seat(seat)
{
}

DataControlDeviceInterface::Private::~Private() = default;

void DataControlDeviceInterface::Private::setSelectionCallback(wl_client *client, wl_resource *resource, wl_resource *source)
{
    Q_UNUSED(client)
    cast<Private>(resource)->setSelection(DataControlSourceInterface::get(source));
}

void DataControlDeviceInterface::Private::setSelection(DataControlSourceInterface *dataSource)
{
    if (selection == dataSource) {
        return;
    }
    Q_Q(DataControlDeviceInterface);
    QObject::disconnect(selectionUnboundConnection);
    QObject::disconnect(selectionDestroyedConnection);
    if (selection) {
        selection->cancel();
    }
    selection = dataSource;
    if (selection) {
        auto clearSelection = [this] {
            setSelection(nullptr);
        };
        selectionUnboundConnection = QObject::connect(selection, &Resource::unbound, q, clearSelection);
        selectionDestroyedConnection = QObject::connect(selection, &QObject::destroyed, q, clearSelection);
        emit q->selectionChanged(selection);
    } else {
        selectionUnboundConnection = QMetaObject::Connection();
        selectionDestroyedConnection = QMetaObject::Connection();
        emit q->selectionCleared();
    }
}

DataControlOfferInterface *DataControlDeviceInterface::Private::createDataOffer(DataControlSourceInterface *source)
{
    if (!resource) {
        return nullptr;
    }
    if (!source) {
        // a data offer can only exist together with a source
        return nullptr;
    }
    Q_Q(DataControlDeviceInterface);
    DataControlOfferInterface *offer = new DataControlOfferInterface(source, q, resource);
    auto c = q->global()->display()->getConnection(wl_resource_get_client(resource));
    offer->create(c, wl_resource_get_version(resource), 0);
    if (!offer->resource()) {
        // TODO: send error?
        delete offer;
        return nullptr;
    }
    zwlr_data_control_device_v1_send_data_offer(resource, offer->resource());
    offer->sendAllOffers();
    return offer;
}

DataControlDeviceInterface::DataControlDeviceInterface(SeatInterface *seat, DataControlDeviceManagerInterface *parent, wl_resource *parentResource)
    : Resource(new Private(seat, this, parent, parentResource))
{
}

DataControlDeviceInterface::~DataControlDeviceInterface() = default;

SeatInterface *DataControlDeviceInterface::seat() const
{
    Q_D();
    return d->seat;
}

DataControlSourceInterface *DataControlDeviceInterface::selection() const
{
    Q_D();
    return d->selection;
}

void DataControlDeviceInterface::sendSelection(DataControlDeviceInterface *other)
{
    Q_D();
    auto otherSelection = other->selection();
    if (!otherSelection) {
        sendClearSelection();
        return;
    }
    auto r = d->createDataOffer(otherSelection);
    if (!r) {
        return;
    }
    if (!d->resource) {
        return;
    }
    zwlr_data_control_device_v1_send_selection(d->resource, r->resource());
}

void DataControlDeviceInterface::sendClearSelection()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    zwlr_data_control_device_v1_send_selection(d->resource, nullptr);
}

DataControlDeviceInterface::Private *DataControlDeviceInterface::d_func() const
{
    return reinterpret_cast<DataControlDeviceInterface::Private*>(d.data());
}

}
}
