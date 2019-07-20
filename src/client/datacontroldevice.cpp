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
#include "datacontroldevice.h"
#include "datacontroloffer.h"
#include "datacontrolsource.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPointer>
// Wayland
#include <wayland-data-control-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DataControlDevice::Private
{
public:
    explicit Private(DataControlDevice *q);
    void setup(zwlr_data_control_device_v1 *d);

    WaylandPointer<zwlr_data_control_device_v1, zwlr_data_control_device_v1_destroy> device;
    QScopedPointer<DataControlOffer> selectionOffer;

private:
    void dataOffer(zwlr_data_control_offer_v1 *id);
    void selection(zwlr_data_control_offer_v1 *id);
    static void dataOfferCallback(void *data, zwlr_data_control_device_v1 *dataDevice, zwlr_data_control_offer_v1 *id);
    static void selectionCallback(void *data, zwlr_data_control_device_v1 *dataDevice, zwlr_data_control_offer_v1 *id);

    static const struct zwlr_data_control_device_v1_listener s_listener;

    DataControlDevice *q;
    DataControlOffer *lastOffer = nullptr;
};

const zwlr_data_control_device_v1_listener DataControlDevice::Private::s_listener = {
    dataOfferCallback,
    selectionCallback,
};

void DataControlDevice::Private::dataOfferCallback(void *data, zwlr_data_control_device_v1 *dataDevice, zwlr_data_control_offer_v1 *id)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->device == dataDevice);
    d->dataOffer(id);
}

void DataControlDevice::Private::dataOffer(zwlr_data_control_offer_v1 *id)
{
    Q_ASSERT(!lastOffer);
    lastOffer = new DataControlOffer(q, id);
    Q_ASSERT(lastOffer->isValid());
}

void DataControlDevice::Private::selectionCallback(void *data, zwlr_data_control_device_v1 *dataDevice, zwlr_data_control_offer_v1 *id)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->device == dataDevice);
    d->selection(id);
}

void DataControlDevice::Private::selection(zwlr_data_control_offer_v1 *id)
{
    if (!id) {
        selectionOffer.reset();
        emit q->selectionCleared();
        return;
    }
    Q_ASSERT(*lastOffer == id);
    selectionOffer.reset(lastOffer);
    lastOffer = nullptr;
    emit q->selectionOffered(selectionOffer.data());
}

DataControlDevice::Private::Private(DataControlDevice *q)
    : q(q)
{
}

void DataControlDevice::Private::setup(zwlr_data_control_device_v1 *d)
{
    Q_ASSERT(d);
    Q_ASSERT(!device.isValid());
    device.setup(d);
    zwlr_data_control_device_v1_add_listener(device, &s_listener, this);
}

DataControlDevice::DataControlDevice(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataControlDevice::~DataControlDevice()
{
    release();
}

void DataControlDevice::destroy()
{
    d->device.destroy();
}

void DataControlDevice::release()
{
    d->device.release();
}

bool DataControlDevice::isValid() const
{
    return d->device.isValid();
}

void DataControlDevice::setup(zwlr_data_control_device_v1 *dataDevice)
{
    d->setup(dataDevice);
}

namespace {
static zwlr_data_control_source_v1 *dataSource(const DataControlSource *source)
{
    if (!source) {
        return nullptr;
    }
    return *source;
}
}

void DataControlDevice::setSelection(DataControlSource *source)
{
    zwlr_data_control_device_v1_set_selection(d->device, dataSource(source));
}

void DataControlDevice::clearSelection()
{
    setSelection();
}

DataControlOffer *DataControlDevice::offeredSelection() const
{
    return d->selectionOffer.data();
}


DataControlDevice::operator zwlr_data_control_device_v1*()
{
    return d->device;
}

DataControlDevice::operator zwlr_data_control_device_v1*() const
{
    return d->device;
}

}
}
