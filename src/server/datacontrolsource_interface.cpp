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
#include "datacontrolsource_interface.h"
#include "datacontroldevicemanager_interface.h"
#include "clientconnection.h"
#include "resource_p.h"
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

class DataControlSourceInterface::Private : public Resource::Private
{
public:
    Private(DataControlSourceInterface *q, DataControlDeviceManagerInterface *parent, wl_resource *parentResource);
    ~Private();

    QStringList mimeTypes;

private:
    DataControlSourceInterface *q_func() {
        return reinterpret_cast<DataControlSourceInterface *>(q);
    }
    void offer(const QString &mimeType);

    static void offerCallback(wl_client *client, wl_resource *resource, const char *mimeType);
    static void setActionsCallback(wl_client *client, wl_resource *resource, uint32_t dnd_actions);

    const static struct zwlr_data_control_source_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_source_v1_interface DataControlSourceInterface::Private::s_interface = {
    offerCallback,
    resourceDestroyedCallback,
};
#endif

DataControlSourceInterface::Private::Private(DataControlSourceInterface *q, DataControlDeviceManagerInterface *parent, wl_resource *parentResource)
    : Resource::Private(q, parent, parentResource, &zwlr_data_control_source_v1_interface, &s_interface)
{
}

DataControlSourceInterface::Private::~Private() = default;

void DataControlSourceInterface::Private::offerCallback(wl_client *client, wl_resource *resource, const char *mimeType)
{
    Q_UNUSED(client)
    cast<Private>(resource)->offer(QString::fromUtf8(mimeType));
}

void DataControlSourceInterface::Private::offer(const QString &mimeType)
{
    mimeTypes << mimeType;
    Q_Q(DataControlSourceInterface);
    emit q->mimeTypeOffered(mimeType);
}

DataControlSourceInterface::DataControlSourceInterface(DataControlDeviceManagerInterface *parent, wl_resource *parentResource)
    : Resource(new Private(this, parent, parentResource))
{
}

DataControlSourceInterface::~DataControlSourceInterface() = default;

void DataControlSourceInterface::requestData(const QString &mimeType, qint32 fd)
{
    Q_D();
    // TODO: does this require a sanity check on the possible mimeType?
    if (d->resource) {
        zwlr_data_control_source_v1_send_send(d->resource, mimeType.toUtf8().constData(), int32_t(fd));
    }
    close(fd);
}

void DataControlSourceInterface::cancel()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    zwlr_data_control_source_v1_send_cancelled(d->resource);
    client()->flush();
}

QStringList DataControlSourceInterface::mimeTypes() const
{
    Q_D();
    return d->mimeTypes;
}

DataControlSourceInterface *DataControlSourceInterface::get(wl_resource *native)
{
    return Private::get<DataControlSourceInterface>(native);
}

DataControlSourceInterface::Private *DataControlSourceInterface::d_func() const
{
    return reinterpret_cast<DataControlSourceInterface::Private*>(d.data());
}

}
}
