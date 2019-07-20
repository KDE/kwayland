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
#include "datacontroldevicemanager.h"
#include "datacontroldevice.h"
#include "datacontrolsource.h"
#include "event_queue.h"
#include "seat.h"
#include "wayland_pointer_p.h"

#include <wayland-data-control-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DataControlDeviceManager::Private
{
public:
    WaylandPointer<zwlr_data_control_manager_v1, zwlr_data_control_manager_v1_destroy> manager;
    EventQueue *queue = nullptr;
};

DataControlDeviceManager::DataControlDeviceManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

DataControlDeviceManager::~DataControlDeviceManager()
{
    release();
}

void DataControlDeviceManager::release()
{
    d->manager.release();
}

void DataControlDeviceManager::destroy()
{
    d->manager.destroy();
}

bool DataControlDeviceManager::isValid() const
{
    return d->manager.isValid();
}

void DataControlDeviceManager::setup(zwlr_data_control_manager_v1 *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *DataControlDeviceManager::eventQueue()
{
    return d->queue;
}

void DataControlDeviceManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

DataControlSource *DataControlDeviceManager::createDataSource(QObject *parent)
{
    Q_ASSERT(isValid());
    DataControlSource *s = new DataControlSource(parent);
    auto w = zwlr_data_control_manager_v1_create_data_source(d->manager);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

DataControlDevice *DataControlDeviceManager::getDataDevice(Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);
    DataControlDevice *device = new DataControlDevice(parent);
    auto w = zwlr_data_control_manager_v1_get_data_device(d->manager, *seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    device->setup(w);
    return device;
}

DataControlDeviceManager::operator zwlr_data_control_manager_v1*() const
{
    return d->manager;
}

DataControlDeviceManager::operator zwlr_data_control_manager_v1*()
{
    return d->manager;
}

}
}
