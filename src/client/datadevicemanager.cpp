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
#include "datadevicemanager.h"
#include "datasource.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class DataDeviceManager::Private
{
public:
    WaylandPointer<wl_data_device_manager, wl_data_device_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

DataDeviceManager::DataDeviceManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

DataDeviceManager::~DataDeviceManager()
{
    release();
}

void DataDeviceManager::release()
{
    d->manager.release();
}

void DataDeviceManager::destroy()
{
    d->manager.destroy();
}

bool DataDeviceManager::isValid() const
{
    return d->manager.isValid();
}

void DataDeviceManager::setup(wl_data_device_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *DataDeviceManager::eventQueue()
{
    return d->queue;
}

void DataDeviceManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

DataSource *DataDeviceManager::createDataSource(QObject *parent)
{
    Q_ASSERT(isValid());
    DataSource *s = new DataSource(parent);
    auto w = wl_data_device_manager_create_data_source(d->manager);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

DataDeviceManager::operator wl_data_device_manager*() const
{
    return d->manager;
}

DataDeviceManager::operator wl_data_device_manager*()
{
    return d->manager;
}

}
}
