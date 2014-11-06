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
#include "datadevice.h"
#include "dataoffer.h"
#include "datasource.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class DataDevice::Private
{
public:
    explicit Private(DataDevice *q);
    void setup(wl_data_device *d);

    WaylandPointer<wl_data_device, wl_data_device_destroy> device;
    QScopedPointer<DataOffer> selectionOffer;

private:
    void dataOffer(wl_data_offer *id);
    void selection(wl_data_offer *id);
    static void dataOfferCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id);
    static void enterCallback(void *data, wl_data_device *dataDevice, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id);
    static void leaveCallback(void *data, wl_data_device *dataDevice);
    static void motionCallback(void *data, wl_data_device *dataDevice, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void dropCallback(void *data, wl_data_device *dataDevice);
    static void selectionCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id);

    static const struct wl_data_device_listener s_listener;

    DataDevice *q;
    DataOffer *lastOffer = nullptr;
};

const wl_data_device_listener DataDevice::Private::s_listener = {
    dataOfferCallback,
    enterCallback,
    leaveCallback,
    motionCallback,
    dropCallback,
    selectionCallback
};

void DataDevice::Private::dataOfferCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->device == dataDevice);
    d->dataOffer(id);
}

void DataDevice::Private::dataOffer(wl_data_offer *id)
{
    Q_ASSERT(!lastOffer);
    lastOffer = new DataOffer(q, id);
    Q_ASSERT(lastOffer->isValid());
}

void DataDevice::Private::enterCallback(void *data, wl_data_device *dataDevice, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id)
{
    Q_UNUSED(data)
    Q_UNUSED(dataDevice)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(id)
}

void DataDevice::Private::leaveCallback(void *data, wl_data_device *dataDevice)
{
    Q_UNUSED(data)
    Q_UNUSED(dataDevice)
}

void DataDevice::Private::motionCallback(void *data, wl_data_device *dataDevice, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(data)
    Q_UNUSED(dataDevice)
    Q_UNUSED(time)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void DataDevice::Private::dropCallback(void *data, wl_data_device *dataDevice)
{
    Q_UNUSED(data)
    Q_UNUSED(dataDevice)
}

void DataDevice::Private::selectionCallback(void *data, wl_data_device *dataDevice, wl_data_offer *id)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->device == dataDevice);
    d->selection(id);
}

void DataDevice::Private::selection(wl_data_offer *id)
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

DataDevice::Private::Private(DataDevice *q)
    : q(q)
{
}

void DataDevice::Private::setup(wl_data_device *d)
{
    Q_ASSERT(d);
    Q_ASSERT(!device.isValid());
    device.setup(d);
    wl_data_device_add_listener(device, &s_listener, this);
}

DataDevice::DataDevice(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataDevice::~DataDevice()
{
    release();
}

void DataDevice::destroy()
{
    d->device.destroy();
}

void DataDevice::release()
{
    d->device.release();
}

bool DataDevice::isValid() const
{
    return d->device.isValid();
}

void DataDevice::setup(wl_data_device *dataDevice)
{
    d->setup(dataDevice);
}

void DataDevice::startDragInternally(quint32 serial, Surface *origin, Surface *icon)
{
    startDrag(serial, nullptr, origin, icon);
}

static wl_data_source *dataSource(const DataSource *source)
{
    if (!source) {
        return nullptr;
    }
    return *source;
}

void DataDevice::startDrag(quint32 serial, DataSource *source, Surface *origin, Surface *icon)
{
    wl_data_device_start_drag(d->device,
                              dataSource(source),
                              *origin,
                              icon ? (wl_surface*)*icon : nullptr,
                              serial);
}

void DataDevice::setSelection(quint32 serial, DataSource *source)
{
    wl_data_device_set_selection(d->device, dataSource(source), serial);
}

void DataDevice::clearSelection(quint32 serial)
{
    setSelection(serial);
}

DataOffer *DataDevice::offeredSelection() const
{
    return d->selectionOffer.data();
}

DataDevice::operator wl_data_device*()
{
    return d->device;
}

DataDevice::operator wl_data_device*() const
{
    return d->device;
}

}
}
